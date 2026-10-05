//----------------------------------------------------------------------------
//! @file   EffectsSystem.cpp
//! @brief  演出のシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/EffectsSystem.hpp>

#include <FruitMagic/ECS/Component/EffectComponents.hpp>
#include <FruitMagic/ECS/Component/PrizeComponent.hpp>
#include <FruitMagic/ECS/Event/MagicCastEvent.hpp>
#include <FruitMagic/ECS/Event/PrizeDroppedEvent.hpp>
#include <FruitMagic/ECS/Event/ZukanRegisteredEvent.hpp>
#include <FruitMagic/Game/AssetPaths.hpp>
#include <FruitMagic/Game/CollectionConfig.hpp>
#include <FruitMagic/Game/EffectsConfig.hpp>
#include <FruitMagic/Game/MagicCatalog.hpp>
#include <FruitMagic/Game/MagicEffects.hpp>
#include <FruitMagic/Game/StageConfig.hpp>
#include <FruitMagic/Game/TableLayout.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RimGlowComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        constexpr float kPi = 3.14159265358979323846f;

        //! @brief 光の粒の画像の一辺のピクセル数（ワールドスプライトの大きさは「ピクセル数 × スケール」cm）
        constexpr float kSparkleTextureSize = 64.0f;
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    EffectsSystem::EffectsSystem(Tsukino::ECS::EventBus& eventBus)
        : m_rng(std::random_device{}()) {
        // ハンドラでは積むだけにして、粒を出すのは Update で行う
        m_effectConnection = eventBus.Subscribe<EffectEvent>([this](const EffectEvent& e) { m_pending.push_back(e); });

        // 魔法: "cast_<魔法の id>" のプリセットを台の中央に（無ければ何も出さない）。id は Update で引く
        m_castConnection = eventBus.Subscribe<MagicCastEvent>([this](const MagicCastEvent& e) {
            m_pending.push_back(EffectEvent{"#cast:" + std::to_string(e.magicIndex), m_castPosition});
        });

        // 払い出し口に落ちた果物: 色違い・金色なら "variant_<バリエーションの id>" を落ちた位置に
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.kind != PrizeKind::Fruit || e.zone != DropZone::Payout)
                return;
            m_lastFruitDrop = hlslpp::float3(e.x, m_dropEffectY, m_dropEffectZ);
            if(e.variantIndex > 0)
                m_pending.push_back(EffectEvent{"#variant:" + std::to_string(e.variantIndex), m_lastFruitDrop});
        });

        // 図鑑に初登録: 直前に収穫した果物の位置に虹色の粒
        m_zukanConnection = eventBus.Subscribe<ZukanRegisteredEvent>([this](const ZukanRegisteredEvent&) { m_pending.push_back(EffectEvent{"zukan", m_lastFruitDrop}); });
    }

    //----------------------------------------------------------------------------
    //! 演出を出して動かします。
    //----------------------------------------------------------------------------
    void EffectsSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        m_time += deltaTime;
        if(!registry.HasContext<EffectsConfig>()) {
            m_pending.clear();
            return;
        }
        const EffectsConfig& config = registry.GetContext<EffectsConfig>();

        // イベントのハンドラ（Update の外）で使う位置を、設定と台の寸法から覚えておく
        m_castPosition = config.motion.castPosition;
        m_dropEffectY  = config.motion.dropEffectY;
        m_dropEffectZ  = GetTableLayout(registry).fieldFrontZ + config.motion.dropEffectAhead;

        // 光の粒の画像は最初に使うときに読み込む
        if(!m_texturesLoaded) {
            Tsukino::EngineIntegration::EngineContext* ctx = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
            if(ctx && ctx->assetManager) {
                m_sparkleTexture = ctx->assetManager->Load(Tsukino::Core::Path(AssetPaths::kSparkleTexture));
                m_glowTexture    = ctx->assetManager->Load(Tsukino::Core::Path(AssetPaths::kGlowTexture));
            }
            m_texturesLoaded = true;
        }

        //--------------------------------------------------------------
        // 頼まれた粒を出す。"#cast:" と "#variant:" は添字からプリセットの名前を引く
        //--------------------------------------------------------------
        std::vector<EffectEvent> pending;
        pending.swap(m_pending);
        for(const EffectEvent& e : pending) {
            std::string name = e.preset;
            if(name.rfind("#cast:", 0) == 0) {
                const int index = std::stoi(name.substr(6));
                if(!registry.HasContext<MagicCatalog>() || index < 0 || index >= static_cast<int>(registry.GetContext<MagicCatalog>().Magics().size()))
                    continue;
                name = "cast_" + registry.GetContext<MagicCatalog>().Magics()[index].id;
            } else if(name.rfind("#variant:", 0) == 0) {
                const int index = std::stoi(name.substr(9));
                if(!registry.HasContext<CollectionConfig>() || index < 0 || index >= static_cast<int>(registry.GetContext<CollectionConfig>().Variants().size()))
                    continue;
                name = "variant_" + registry.GetContext<CollectionConfig>().Variants()[index].id;
            }
            if(const EffectPreset* preset = config.Find(name))
                Burst(registry, *preset, e.position);
        }

        //--------------------------------------------------------------
        // 台の上の色違い・金色の果物から、時々小さな粒をこぼす
        //--------------------------------------------------------------
        m_idleTimer -= deltaTime;
        if(m_idleTimer <= 0.0f) {
            m_idleTimer = config.motion.idleInterval;
            if(const EffectPreset* idle = config.Find("shinyIdle")) {
                std::vector<hlslpp::float3> positions;
                registry.View<PrizeComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
                    [&](Tsukino::ECS::Entity, PrizeComponent& prize, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                        if(prize.kind == PrizeKind::Fruit && prize.variantIndex > 0 && float(transform.position.y) > config.motion.idleMinY)
                            positions.push_back(transform.position + hlslpp::float3(0.0f, config.motion.idleRise, 0.0f));
                    });
                for(const hlslpp::float3& p : positions)
                    Burst(registry, *idle, p);
            }
        }

        UpdateParticles(registry, deltaTime);

        //--------------------------------------------------------------
        // 画面の光を弱めていく（スケール 0 で描画しない）
        //--------------------------------------------------------------
        registry.View<ScreenFlashComponent, Tsukino::BuiltIn::ECS::SpriteComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity, ScreenFlashComponent& flash, Tsukino::BuiltIn::ECS::SpriteComponent& sprite,
                Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                flash.color.w    = std::max(0.0f, float(flash.color.w) - config.motion.flashFadeSpeed * deltaTime);
                const float a    = flash.color.w;
                sprite.tintColor = hlslpp::float4(flash.color.x * a, flash.color.y * a, flash.color.z * a, a);
                transform.scale  = (a > 0.0f) ? flash.fullScale : hlslpp::float3(0.0f, 0.0f, 1.0f);
                transform.dirty  = true;
            });

        //--------------------------------------------------------------
        // ぽよん（見た目のスケールだけ、膨らんでから元に戻る）
        //--------------------------------------------------------------
        std::vector<Tsukino::ECS::Entity> finished;
        registry.View<PopScaleComponent, Tsukino::BuiltIn::ECS::TransformComponent>().each(
            [&](Tsukino::ECS::Entity entity, PopScaleComponent& pop, Tsukino::BuiltIn::ECS::TransformComponent& transform) {
                pop.elapsed += deltaTime;
                const float k   = std::min(1.0f, pop.elapsed / pop.duration);
                transform.scale = pop.baseScale * (1.0f + pop.amount * std::sin(kPi * k) * (1.0f - k));
                transform.dirty = true;
                if(k >= 1.0f)
                    finished.push_back(entity);
            });
        for(Tsukino::ECS::Entity entity : finished)
            registry.RemoveComponent<PopScaleComponent>(entity);

        //--------------------------------------------------------------
        // 魔法「ふくらむ」の間、プッシャーの輪郭をピンクに脈打たせる
        //--------------------------------------------------------------
        const bool swelling = registry.HasContext<MagicEffects>() && registry.GetContext<MagicEffects>().pusherAmplitudeBonus > 0.0f;
        const StageConfig& stage = GetStageConfig(registry);
        std::vector<Tsukino::ECS::Entity> pushers;
        registry.View<PusherComponent>().each([&](Tsukino::ECS::Entity entity, PusherComponent& pusher) {
            if(swelling || pusher.glowing)
                pushers.push_back(entity);
            pusher.glowing = swelling;
        });
        for(Tsukino::ECS::Entity pusher : pushers) {
            auto* rim = registry.try_get<Tsukino::BuiltIn::ECS::RimGlowComponent>(pusher);
            if(!rim) {
                if(!swelling)
                    continue;
                rim = &registry.AddComponent<Tsukino::BuiltIn::ECS::RimGlowComponent>(pusher);
            }
            rim->active       = swelling;
            const float pulse = std::sin(m_time * stage.swellPulseSpeed);
            rim->rimColor     = stage.swellColor;
            rim->rimIntensity = stage.swellIntensity + stage.swellIntensityPulse * pulse;
            rim->glow         = stage.swellGlow + stage.swellGlowPulse * pulse;
        }
    }

    //----------------------------------------------------------------------------
    //! プリセットの粒をまとめて出し、画面を光らせます。
    //----------------------------------------------------------------------------
    void EffectsSystem::Burst(Tsukino::ECS::Registry& registry, const EffectPreset& preset, const hlslpp::float3& position) {
        //--------------------------------------------------------------
        // 画面を光らせる（すでに光っていれば強い方）
        //--------------------------------------------------------------
        if(preset.flash.w > 0.0f) {
            registry.View<ScreenFlashComponent>().each([&](Tsukino::ECS::Entity, ScreenFlashComponent& flash) {
                if(preset.flash.w >= flash.color.w)
                    flash.color = preset.flash;
            });
        }

        if(!registry.HasContext<EffectsConfig>())
            return;
        const EffectsConfig&              config       = registry.GetContext<EffectsConfig>();
        const EffectsMotion&              motion       = config.motion;
        const int                         maxParticles = config.MaxParticles();
        const Tsukino::Asset::AssetHandle texture = (preset.texture == SparkleTexture::Glow) ? m_glowTexture : m_sparkleTexture;

        std::uniform_real_distribution<float> unit(0.0f, 1.0f);
        for(int i = 0; i < preset.count && m_particleCount < maxParticles; ++i) {
            //--------------------------------------------------------------
            // 位置は円の中でばらつかせ、速度は横へランダムに飛び散りつつ上へ
            //--------------------------------------------------------------
            const float angle  = unit(m_rng) * 2.0f * kPi;
            const float radius = std::sqrt(unit(m_rng)) * preset.spread;
            const float speed  = unit(m_rng) * preset.speed;
            const float dir    = unit(m_rng) * 2.0f * kPi;
            const hlslpp::float3 start = position + hlslpp::float3(std::cos(angle) * radius, unit(m_rng) * motion.startHeight, std::sin(angle) * radius);

            SparkleParticleComponent particle;
            particle.velocity = hlslpp::float3(std::cos(dir) * speed, preset.up * (motion.upMin + (1.0f - motion.upMin) * unit(m_rng)), std::sin(dir) * speed);
            particle.color    = preset.colors[i % preset.colors.size()];
            particle.gravity  = preset.gravity;
            particle.maxLife  = preset.life * (motion.lifeMin + (1.0f - motion.lifeMin) * unit(m_rng));
            particle.life     = particle.maxLife;
            particle.size     = preset.size * (motion.sizeMin + motion.sizeRange * unit(m_rng));

            Tsukino::ECS::Entity                       e = registry.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& t = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            t.position                                   = start;
            t.scale                                      = hlslpp::float3(particle.size / kSparkleTextureSize, particle.size / kSparkleTextureSize, 1.0f);
            t.dirty                                      = true;

            Tsukino::BuiltIn::ECS::SpriteComponent& sprite = registry.AddComponent<Tsukino::BuiltIn::ECS::SpriteComponent>(e);
            sprite.textureHandle                           = texture;
            sprite.space                                   = Tsukino::BuiltIn::ECS::SpriteSpace::World;
            sprite.blendMode                               = Tsukino::BuiltIn::ECS::SpriteBlendMode::Additive;
            sprite.tintColor                               = hlslpp::float4(particle.color, 1.0f);

            registry.AddComponent<SparkleParticleComponent>(e, particle);
            ++m_particleCount;
        }
    }

    //----------------------------------------------------------------------------
    //! 光の粒を動かし、寿命が来たものを消します。
    //----------------------------------------------------------------------------
    void EffectsSystem::UpdateParticles(Tsukino::ECS::Registry& registry, float deltaTime) {
        const EffectsMotion motion = registry.HasContext<EffectsConfig>() ? registry.GetContext<EffectsConfig>().motion : EffectsMotion{};

        std::vector<Tsukino::ECS::Entity> dead;
        registry.View<SparkleParticleComponent, Tsukino::BuiltIn::ECS::TransformComponent, Tsukino::BuiltIn::ECS::SpriteComponent>().each(
            [&](Tsukino::ECS::Entity entity, SparkleParticleComponent& particle, Tsukino::BuiltIn::ECS::TransformComponent& transform,
                Tsukino::BuiltIn::ECS::SpriteComponent& sprite) {
                particle.life -= deltaTime;
                if(particle.life <= 0.0f) {
                    dead.push_back(entity);
                    return;
                }

                // 横はだんだん止まり、縦は重力で落ちる
                const float drag    = std::max(0.0f, 1.0f - motion.drag * deltaTime);
                particle.velocity   = hlslpp::float3(particle.velocity.x * drag, particle.velocity.y - particle.gravity * deltaTime, particle.velocity.z * drag);
                transform.position += particle.velocity * deltaTime;

                // 寿命に合わせて小さく・暗く（加算合成なので色ごと暗くする）
                const float k   = particle.life / particle.maxLife;
                const float s   = particle.size * (motion.shrinkMin + (1.0f - motion.shrinkMin) * k) / kSparkleTextureSize;
                transform.scale = hlslpp::float3(s, s, 1.0f);
                transform.dirty = true;
                sprite.tintColor = hlslpp::float4(particle.color * k, k);
            });

        for(Tsukino::ECS::Entity entity : dead)
            registry.QueueDestroy(entity);
        m_particleCount = std::max(0, m_particleCount - static_cast<int>(dead.size()));
    }
}    // namespace FruitMagic::ECS
