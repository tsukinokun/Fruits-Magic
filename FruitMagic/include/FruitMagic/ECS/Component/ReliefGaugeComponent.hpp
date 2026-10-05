//----------------------------------------------------------------------------
//! @file   ReliefGaugeComponent.hpp
//! @brief  おすそわけ待ちのリングのゲージを表すコンポーネント
//----------------------------------------------------------------------------
#pragma once

// 名前空間 : FruitMagic::ECS
namespace FruitMagic::ECS {

    //! おすそわけ待ちのリングの部品の種類です。
    enum class ReliefGaugePart {
        Background,    // 常に一周している暗い下地
        Fill,          // 次の1枚までの進み具合で、真上から時計回りに満ちる中身
    };

    //! おすそわけ待ちのリングです。SpriteComponent と一緒に付け、HudSystem が表示・塗る割合を変えます。
    //! @note 待っていない間は拡大率 0 で隠すため、表示するときの拡大率を持つ
    struct ReliefGaugeComponent {
        ReliefGaugePart part       = ReliefGaugePart::Fill;    // 部品の種類
        float           shownScale = 1.0f;                     // 表示するときの拡大率
    };
}    // namespace FruitMagic::ECS
