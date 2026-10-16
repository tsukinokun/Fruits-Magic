//----------------------------------------------------------------------------
//! @file   MoveRules.hpp
//! @brief  屋台のおひっこし（転生）の決まり（できるか・もらえる星のたね・リセット）
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

// 名前空間 : FruitMagic::MoveRules
//! おひっこしの決まりです。強化画面・確認ウィンドウ・シーン・計測から同じ計算を使います。
//! 設定は MoveConfig、星のたねの倍率と最初のコイン・妖精は星のたねの強化（UpgradeSystem::ApplyUpgrades が TableStats に入れる）。
namespace FruitMagic::MoveRules {

    //! おひっこしが解放されているか（MoveConfig::unlockTreeStage 以上の果物を図鑑に1つでも登録したか）を返します。
    //! @param  [in] registry レジストリ
    //! @return 解放されていれば true
    bool IsUnlocked(Tsukino::ECS::Registry& registry);

    //! 今おひっこししたらもらえる星のたねの数を返します（解放前は 0）。
    //! @param  [in] registry レジストリ
    //! @return 星のたねの数
    long long SeedsIfMovedNow(Tsukino::ECS::Registry& registry);

    //! 今おひっこしできるか（解放済みで、もらえる星のたねが MoveConfig::minSeeds 以上か）を返します。
    //! @param  [in] registry レジストリ
    //! @return できれば true
    bool CanMove(Tsukino::ECS::Registry& registry);

    //! おひっこしします。星のたねを受け取り、手持ち（コイン・FP・マナ）・コインと FP の強化・今回の収穫の価値を戻します。
    //! 図鑑・記録・星のたね・星のたねの強化は残し、最初のコインと妖精は星のたねの強化に合わせます。
    //! 台そのものは作り直さないので、呼んだ後にセーブしてシーンを始め直してください。
    //! @param  [in] registry レジストリ
    //! @return もらった星のたねの数（できなかったときは 0。何も変えない）
    long long Move(Tsukino::ECS::Registry& registry);
}    // namespace FruitMagic::MoveRules
