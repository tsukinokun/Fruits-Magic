//----------------------------------------------------------------------------
//! @file   HudSystem.cpp
//! @brief  HUD（手持ち枚数・払い出し表示）を更新するシステムの実装
//----------------------------------------------------------------------------
#include <FruitMagic/ECS/System/HudSystem.hpp>

#include <FruitMagic/ECS/Component/HudTextComponent.hpp>
#include <FruitMagic/Game/GameState.hpp>

#include <Tsukino/BuiltIn/ECS/Component/FontComponent.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/ECS/Event/EventBus.hpp>

#include <string>

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {
    namespace {
        //! @brief 払い出し表示を出しておく時間（秒）。続けて落ちたら延長して合算する
        constexpr float kPopupDuration = 1.2f;
    }    // namespace

    //----------------------------------------------------------------------------
    //! コンストラクタです。
    //----------------------------------------------------------------------------
    HudSystem::HudSystem(Tsukino::ECS::EventBus& eventBus) {
        m_dropConnection = eventBus.Subscribe<PrizeDroppedEvent>([this](const PrizeDroppedEvent& e) {
            if(e.zone == DropZone::Payout) {
                m_recentPayout += e.value;
            } else {
                m_recentGutter += 1;
            }
            m_popupTimer = kPopupDuration;
        });
    }

    //----------------------------------------------------------------------------
    //! HUD のテキストを更新します。
    //----------------------------------------------------------------------------
    void HudSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        //--------------------------------------------------------------
        // 払い出し表示の残り時間。切れたら合計をリセット
        //--------------------------------------------------------------
        if(m_popupTimer > 0.0f) {
            m_popupTimer -= deltaTime;
            if(m_popupTimer <= 0.0f) {
                m_recentPayout = 0;
                m_recentGutter = 0;
            }
        }

        const int coins = registry.HasContext<GameState>() ? registry.GetContext<GameState>().coins : 0;

        //--------------------------------------------------------------
        // 各テキストの更新
        //--------------------------------------------------------------
        auto view = registry.View<HudTextComponent, Tsukino::BuiltIn::ECS::FontComponent>();
        view.each([&](Tsukino::ECS::Entity, HudTextComponent& hud, Tsukino::BuiltIn::ECS::FontComponent& font) {
            switch(hud.kind) {
                case HudTextKind::Coins:
                    font.text = L"コイン: " + std::to_wstring(coins);
                    break;

                case HudTextKind::DropPopup: {
                    std::wstring text;
                    if(m_recentPayout > 0)
                        text += L"+" + std::to_wstring(m_recentPayout) + L"  ";
                    if(m_recentGutter > 0)
                        text += L"溝 " + std::to_wstring(m_recentGutter);
                    font.text = text;
                    break;
                }

                case HudTextKind::ControlsHint:
#ifdef _DEBUG
                    font.text = L"←→ / マウス: 位置   Space / クリック: 投入   F5: コリジョン表示";
#else
                    font.text = L"←→ / マウス: 位置   Space / クリック: 投入";
#endif
                    break;
            }
        });
    }
}    // namespace FruitMagic::ECS
