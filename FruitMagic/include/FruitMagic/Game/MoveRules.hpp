//----------------------------------------------------------------------------
//! @file   MoveRules.hpp
//! @brief  屋台のおひっこし（転生）の決まり（できるか・もらえる星のたね・リセット）
//----------------------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/Registry/Registry.hpp>

#include <random>
#include <string>
#include <vector>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //! おひっこしでもらったおみやげ1つです。
    struct SouvenirGift {
        int  souvenirIndex = -1;       // おみやげの添字（SouvenirCatalog::Souvenirs()）
        int  level         = 0;        // もらった後のレベル
        bool isNew         = false;    // 初めてもらったか
    };

    //! おひっこしの結果です。次の屋台（PusherScene）が「おひっこししました」の窓に出します。
    struct MoveResult {
        long long                 seeds = 0;    // もらった星のたね
        std::vector<SouvenirGift> gifts;        // もらったおみやげ
    };
}    // namespace FruitMagic

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

    //! 今おひっこししたらもらえるおみやげを引き、レベルを上げます（おひっこしの中で呼ぶ。Debug の F3 でも使う）。
    //! 数は今回の屋台の収穫の価値、レア度の重みは手持ちのコインで決まります（SouvenirCatalog のもらい方）。
    //! レベルが上限のおみやげは引きません（すべて上限なら何ももらえない）。
    //! @param  [in] registry レジストリ
    //! @param  [in] rng      乱数
    //! @return もらったおみやげ
    std::vector<SouvenirGift> DrawSouvenirs(Tsukino::ECS::Registry& registry, std::mt19937& rng);

    //! おひっこしします。星のたねとおみやげを受け取り、手持ち（コイン・FP・マナ）・コインと FP の強化・今回の収穫の価値を戻します。
    //! 図鑑・記録・星のたね・星のたねの強化・おみやげは残し、最初のコインと妖精は星のたねの強化（とおみやげ）に合わせます。
    //! 台そのものは作り直さないので、呼んだ後にセーブしてシーンを始め直してください。
    //! @param  [in] registry レジストリ
    //! @return 結果（できなかったときは seeds が 0。何も変えない）
    MoveResult Move(Tsukino::ECS::Registry& registry);
}    // namespace FruitMagic::MoveRules
