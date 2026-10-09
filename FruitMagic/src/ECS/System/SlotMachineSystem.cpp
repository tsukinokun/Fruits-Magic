//----------------------------------------------------------------------------
//! @file   SlotMachineSystem.cpp
//! @brief  ルーレット（3リールのスロット）を表示するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/SlotMachineSystem.hpp>

#include <FruitMagic/ECS/Component/FruitIconComponent.hpp>
#include <FruitMagic/ECS/Component/SlotMachineComponents.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/Game/FruitIcon.hpp>
#include <FruitMagic/Game/GameState.hpp>
#include <FruitMagic/Game/JackpotConfig.hpp>
#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/RouletteConfig.hpp>
#include <FruitMagic/Game/RouletteState.hpp>
#include <FruitMagic/Game/UiConfig.hpp>

#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ScreenModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/UI/UICanvas.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Window.hpp>
#include <Tsukino/EngineIntegration/EngineContext.hpp>

#include <algorithm>
#include <cmath>
#include <random>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        constexpr float kPi = 3.14159265358979323846f;

        //--------------------------------------------------------------
        //! 少し行き過ぎて戻る動き（0〜1 → 0〜1。行き過ぎの強さ overshoot）を返します。
        //! @param  [in] t         進み具合（0〜1）
        //! @param  [in] overshoot 行き過ぎの強さ（0 なら行き過ぎない）
        //! @return 動きの進み具合
        //--------------------------------------------------------------
        float EaseOutBack(float t, float overshoot) {
            const float u = std::clamp(t, 0.0f, 1.0f) - 1.0f;
            return 1.0f + (overshoot + 1.0f) * u * u * u + overshoot * u * u;
        }

        //--------------------------------------------------------------
        //! 値を長さ length で折り返し、-length/2〜length/2 に収めます（リールの絵柄の縦の位置）。
        //! @param  [in] value  値
        //! @param  [in] length 折り返す長さ
        //! @return 折り返した値
        //--------------------------------------------------------------
        float WrapCentered(float value, float length) {
            float v = std::fmod(value, length);
            if(v < 0.0f)
                v += length;
            if(v >= length * 0.5f)
                v -= length;
            return v;
        }

        //--------------------------------------------------------------
        //! リールの並びを作ります。使える絵柄を列ごとに違う順に並べ、絵柄の置き台の数まで繰り返します。
        //! @param  [in] symbols 使える絵柄（果物の添字と kCoinSymbol）
        //! @param  [in] reel    列（並べ方を列ごとに変える）
        //! @param  [in] count   並びの長さ（置き台の数）
        //! @return 並び
        //--------------------------------------------------------------
        std::vector<int> BuildStrip(std::vector<int> symbols, int reel, size_t count) {
            std::vector<int> strip;
            if(symbols.empty() || count == 0)
                return strip;
            std::mt19937 rng(static_cast<unsigned int>(20261008 + reel * 7919));    // 列ごとに決まった並び（毎回同じ）
            std::shuffle(symbols.begin(), symbols.end(), rng);
            for(size_t i = 0; i < count; ++i)
                strip.push_back(symbols[i % symbols.size()]);
            return strip;
        }

        //--------------------------------------------------------------
        //! ワールドの点を、画面 UI の座標にします（メインのカメラで映した位置）。
        //! @param  [in]  registry レジストリ
        //! @param  [in]  world    ワールドの点
        //! @param  [out] ui       UI の座標
        //! @return 映せたら true（メインのカメラが無い・カメラの後ろなら false）
        //--------------------------------------------------------------
        bool ProjectToUI(Tsukino::ECS::Registry& registry, const hlslpp::float3& world, hlslpp::float2& ui) {
            Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
            if(!ctx || !ctx->window)
                return false;

            bool                        found = false;
            Tsukino::Core::Math::matrix viewProj;
            registry.View<Tsukino::BuiltIn::ECS::CameraComponent>().each([&](Tsukino::ECS::Entity, Tsukino::BuiltIn::ECS::CameraComponent& camera) {
                if(!found && camera.isPrimary) {
                    viewProj = camera.viewProjMatrix;
                    found    = true;
                }
            });
            if(!found)
                return false;

            const hlslpp::float4 clip = hlslpp::mul(hlslpp::float4(world, 1.0f), static_cast<const hlslpp::float4x4&>(viewProj));
            if(float(clip.w) <= 0.0f)
                return false;
            const float          ndcX  = float(clip.x) / float(clip.w);
            const float          ndcY  = float(clip.y) / float(clip.w);
            const hlslpp::float2 pixel((ndcX * 0.5f + 0.5f) * static_cast<float>(ctx->window->GetWidth()),
                                       (0.5f - ndcY * 0.5f) * static_cast<float>(ctx->window->GetHeight()));
            ui = Tsukino::BuiltIn::ECS::GetUICanvas(registry).ToUI(pixel);
            return true;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! スロットの表示を進めます。
    //----------------------------------------------------------------------------
    void SlotMachineSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        if(!registry.HasContext<RouletteState>() || !registry.HasContext<FruitCatalog>())
            return;

        m_time += deltaTime;
        const RouletteState& state    = registry.GetContext<RouletteState>();
        const FruitCatalog&  catalog  = registry.GetContext<FruitCatalog>();
        const UiConfig&      ui       = GetUiConfig(registry);
        const int            level    = registry.HasContext<GameState>() ? registry.GetContext<GameState>().treeLevel : 0;

        const bool jackpotShown = state.phase == RoulettePhase::JackpotSpin || state.phase == RoulettePhase::JackpotResult;
        const bool spinning     = state.phase == RoulettePhase::Spinning || state.phase == RoulettePhase::JackpotSpin;

        // 縁の光り方（[ジャックポットか][列]。0〜1）
        float flashLevel[2][kReelCount] = {};

        //--------------------------------------------------------------
        // リール
        //--------------------------------------------------------------
        registry.View<SlotReelComponent>().each([&](Tsukino::ECS::Entity, SlotReelComponent& reel) {
            const SlotLayout& layout = reel.jackpot ? ui.jackpotSlot : ui.slot;
            const bool        mine   = (state.jackpotReels == reel.jackpot);    // 今の回転がこのスロットのものか
            // 画面（図鑑など）を開いていても隠さない。絵柄の重ね順は画面の板より奥なので、板の下に透けて見える
            const bool        shown  = reel.jackpot ? jackpotShown : true;
            const float       pitch  = layout.symbolPitch;
            if(reel.icons.empty())
                return;

            //--------------------------------------------------------------
            // 並び。ふだんのスロットは今出せる果物とコイン、ジャックポットは当たりの果物とコイン。
            // 果樹の段階が上がったら、止まっている間に作り直す
            //--------------------------------------------------------------
            if(reel.strip.empty() || (!reel.jackpot && reel.stripLevel != level && reel.motion == SlotReelMotion::Stopped)) {
                std::vector<int> symbols;
                if(reel.jackpot) {
                    const JackpotConfig jackpot = registry.HasContext<JackpotConfig>() ? registry.GetContext<JackpotConfig>() : JackpotConfig{};
                    const int           fruit   = catalog.FindIndex(jackpot.symbolFruit);
                    symbols                     = {fruit >= 0 ? fruit : 0, kCoinSymbol};
                } else {
                    symbols = catalog.SpawnableFruits(level);
                    symbols.push_back(kCoinSymbol);
                }
                reel.strip       = BuildStrip(symbols, reel.reel + (reel.jackpot ? kReelCount : 0), reel.icons.size());
                reel.stripLevel  = level;
                reel.variantIcon = -1;
                for(size_t i = 0; i < reel.icons.size() && i < reel.strip.size(); ++i)
                    SetFruitIcon(registry, reel.icons[i], reel.strip[i], 0, false, layout.symbolSize);
            }
            if(reel.strip.empty())
                return;
            const float stripLength = pitch * static_cast<float>(reel.strip.size());

            //--------------------------------------------------------------
            // 新しい回転: 回し始める。前の回転で色違いの色にした絵柄は通常に戻す
            //--------------------------------------------------------------
            if(mine && spinning && reel.spinId != state.spinId) {
                reel.spinId = state.spinId;
                reel.motion = SlotReelMotion::Spinning;
                reel.speed  = layout.spinSpeed;
                if(reel.variantIcon >= 0) {
                    SetFruitIcon(registry, reel.icons[reel.variantIcon], reel.strip[reel.variantIcon], 0, false, layout.symbolSize);
                    reel.variantIcon = -1;
                }
            }

            //--------------------------------------------------------------
            // 回す。止める時刻の少し前から、止める絵柄が窓の真ん中に来るように減速する
            //--------------------------------------------------------------
            const float stopTime = state.reelStopTimes[reel.reel];
            if(reel.motion == SlotReelMotion::Spinning) {
                // リーチの3列目はゆっくり（ドキドキ）
                const bool  reachSlow = state.reach && reel.reel == kReelCount - 1 && state.reelsStopped >= kReelCount - 1;
                const float target    = reachSlow ? layout.reachSpeed : layout.spinSpeed;
                reel.speed += (target - reel.speed) * std::min(1.0f, deltaTime * 8.0f);
                reel.scroll += reel.speed * deltaTime;

                if(mine && state.spinTime >= stopTime - layout.stopSeconds) {
                    // 止める絵柄の候補のうち、ちょうどよい距離（速さを落としながら届く距離）で止まるものを選ぶ
                    const int   symbol  = state.reelSymbols[reel.reel];
                    const float minDist = reel.speed * layout.stopSeconds * 0.5f;
                    int         best    = -1;
                    float       bestD   = 0.0f;
                    for(size_t i = 0; i < reel.strip.size(); ++i) {
                        if(reel.strip[i] != symbol)
                            continue;
                        // 絵柄 i が真ん中に来る（i*pitch + scroll が stripLength の倍数）までの距離
                        float d = std::fmod(-(static_cast<float>(i) * pitch) - reel.scroll, stripLength);
                        if(d < 0.0f)
                            d += stripLength;
                        while(d < minDist)
                            d += stripLength;
                        if(best < 0 || d < bestD) {
                            best  = static_cast<int>(i);
                            bestD = d;
                        }
                    }
                    if(best < 0) {
                        // 並びに無い絵柄（果樹の段階が回転中に変わったなど）: 先頭を差し替えて、そこで止める
                        best           = 0;
                        reel.strip[0]  = symbol;
                        SetFruitIcon(registry, reel.icons[0], symbol, 0, false, layout.symbolSize);
                        bestD = std::fmod(-reel.scroll, stripLength);
                        if(bestD < 0.0f)
                            bestD += stripLength;
                        while(bestD < minDist)
                            bestD += stripLength;
                    }
                    reel.motion       = SlotReelMotion::Stopping;
                    reel.stopFrom     = reel.scroll;
                    reel.stopDistance = bestD;
                    reel.stopStart    = std::min(state.spinTime, stopTime - layout.stopSeconds);
                    reel.variantIcon  = best;    // 止まった後に色違いの色にするかもしれない絵柄（色にしなければ戻す処理が空振りするだけ）
                }
            }
            if(reel.motion == SlotReelMotion::Stopping) {
                // 回転が終わって段階が進んでいたら、止まりきったことにする
                const bool  sameSpin = (reel.spinId == state.spinId) && spinning;
                const float u        = sameSpin ? (state.spinTime - reel.stopStart) / layout.stopSeconds : 1.0f;
                reel.scroll          = reel.stopFrom + reel.stopDistance * EaseOutBack(u, layout.overshoot);
                if(u >= 1.0f) {
                    reel.scroll = reel.stopFrom + reel.stopDistance;
                    reel.motion = SlotReelMotion::Stopped;
                    reel.speed  = 0.0f;
                    reel.flash  = ui.slotFlashSeconds;

                    // 当たりの果物の色違い・金色は、止まった絵柄をその色にする
                    const int symbol = state.reelSymbols[reel.reel];
                    if(state.reelVariant > 0 && symbol >= 0 && reel.variantIcon >= 0)
                        SetFruitIcon(registry, reel.icons[reel.variantIcon], symbol, state.reelVariant, false, layout.symbolSize);
                    else
                        reel.variantIcon = -1;
                }
            }

            //--------------------------------------------------------------
            // 絵柄の位置（窓の真ん中が 0。窓の外へ出たものは隠す）
            //--------------------------------------------------------------
            const float visibleHalf = float(layout.windowSize.y) * 0.5f + pitch * 0.5f;
            for(size_t i = 0; i < reel.anchors.size() && i < reel.strip.size(); ++i) {
                const float y = WrapCentered(static_cast<float>(i) * pitch + reel.scroll, stripLength);
                if(auto* transform = registry.try_get<Tsukino::BuiltIn::ECS::TransformComponent>(reel.anchors[i])) {
                    transform->position = hlslpp::float3(0.0f, y, 0.0f);
                    transform->dirty    = true;
                }
                SetFruitIconVisible(registry, reel.icons[i], shown && std::abs(y) < visibleHalf);
            }

            //--------------------------------------------------------------
            // 縁の光り方: 止まった直後・リーチの3列目の点滅・当たりの点滅
            //--------------------------------------------------------------
            reel.flash        = std::max(0.0f, reel.flash - deltaTime);
            float level01     = (ui.slotFlashSeconds > 0.0f) ? reel.flash / ui.slotFlashSeconds : 0.0f;
            const float blink = 0.5f + 0.5f * std::sin(m_time * 18.0f);
            if(mine && spinning && state.reach && reel.reel == kReelCount - 1 && state.reelsStopped >= kReelCount - 1)
                level01 = std::max(level01, blink);
            const bool won = reel.jackpot ? (state.phase == RoulettePhase::JackpotResult && state.jackpotWin)
                                          : (state.phase == RoulettePhase::Result && (state.resultHit || state.resultCoins > 0));
            if(mine && won)
                level01 = std::max(level01, blink);
            flashLevel[reel.jackpot ? 1 : 0][reel.reel] = level01;
        });

        //--------------------------------------------------------------
        // 板・窓・縁・玉
        //--------------------------------------------------------------
        registry.View<SlotPartComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, SlotPartComponent& part, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                const bool shown = part.jackpot ? jackpotShown : true;
                transform.scale  = shown ? part.shownScale : hlslpp::float3(0.0f, 0.0f, 1.0f);
                transform.dirty  = true;

                auto* sprite = registry.try_get<Tsukino::BuiltIn::ECS::SpriteComponent>(entity);
                if(!sprite)
                    return;
                switch(part.part) {
                    case SlotPart::Flash: {
                        const float level01 = (part.index >= 0 && part.index < kReelCount) ? flashLevel[part.jackpot ? 1 : 0][part.index] : 0.0f;
                        sprite->tintColor   = hlslpp::float4(hlslpp::float3(part.color.xyz), float(part.color.w) * level01);
                        break;
                    }
                    case SlotPart::Lamp:
                        sprite->tintColor = (part.index < state.stock) ? ui.slotLampOn : ui.slotLampOff;
                        break;
                    default:
                        break;
                }
            });

        //--------------------------------------------------------------
        // 当たりの果物がスロットから台の上の出る位置へ、弧を描いて飛ぶ（飛びながら台の上の大きさになる）
        //--------------------------------------------------------------
        const RouletteConfig config = registry.HasContext<RouletteConfig>() ? registry.GetContext<RouletteConfig>() : RouletteConfig{};
        registry.View<SlotFlyerComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, SlotFlyerComponent& flyer, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                hlslpp::float2 to;
                if(state.flyFruit < 0 || !ProjectToUI(registry, state.flyTarget, to)) {
                    SetFruitIconVisible(registry, entity, false);
                    return;
                }

                // 飛び始め: 果物を入れ、真ん中のリールから出す
                if(flyer.spinId != state.spinId) {
                    flyer.spinId = state.spinId;
                    flyer.from   = ui.slot.center + hlslpp::float2(0.0f, ui.slot.windowOffsetY);
                    SetFruitIcon(registry, entity, state.flyFruit, state.flyVariant, false, ui.slotFlySize);
                }

                // 着く所での大きさ（台の上の果物を画面に映した大きさ）
                float endSize = ui.slotFlySize;
                if(state.flyFruit < static_cast<int>(catalog.Fruits().size())) {
                    const hlslpp::float3 half    = PrizeFactory::FruitHalfExtent(catalog.Fruits()[state.flyFruit]);
                    const float          longest = std::max({float(half.x), float(half.y), float(half.z)}) * 2.0f;
                    hlslpp::float2       edge;
                    if(ProjectToUI(registry, state.flyTarget + hlslpp::float3(longest, 0.0f, 0.0f), edge))
                        endSize = std::max(4.0f, float(hlslpp::length(edge - to)));
                }

                const float u     = std::clamp(state.flyTime / config.flySeconds, 0.0f, 1.0f);
                const float eased = u * u * (3.0f - 2.0f * u);
                const hlslpp::float2 position = flyer.from + (to - flyer.from) * eased - hlslpp::float2(0.0f, ui.slotFlyArc * std::sin(kPi * u));
                if(auto* screen = registry.try_get<Tsukino::BuiltIn::ECS::ScreenModelComponent>(entity))
                    screen->screenPosition = position;
                const float scale = 1.0f + (endSize / std::max(ui.slotFlySize, 1.0f) - 1.0f) * eased;
                transform.scale   = hlslpp::float3(scale, scale, scale);
                transform.dirty   = true;
                SetFruitIconVisible(registry, entity, true);
            });
    }
}    // namespace FruitMagic::ECS
