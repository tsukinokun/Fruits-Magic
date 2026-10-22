//----------------------------------------------------------------------------
//! @file   CoinScatter.cpp
//! @brief  起動時に台へ置くコインの散らばらせ方の実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/CoinScatter.hpp>

#include <FruitMagic/Game/PrizeFactory.hpp>
#include <FruitMagic/Game/TableLayout.hpp>

#include <algorithm>
#include <cmath>

// 名前空間 : FruitMagic::CoinScatter
namespace FruitMagic::CoinScatter {

    namespace {
        constexpr float kPi       = 3.14159265358979323846f;
        constexpr float kDegToRad = kPi / 180.0f;
        constexpr int   kTries    = 30;    // 重ねるコインの置き場所を探す回数

        //--------------------------------------------------------------
        //! 床に落とした影（真上から見た四角）です。
        //--------------------------------------------------------------
        struct Footprint {
            float x   = 0.0f;    // 中心の X
            float z   = 0.0f;    // 中心の Z
            float yaw = 0.0f;    // 向き（ラジアン）
            float gap = 0.0f;    // この場所でほかのコインと空けるすき間（手前ほど狭い）
        };

        //--------------------------------------------------------------
        //! 回した2つの正方形が、すき間（2つの広い方）を空けて置けないか（重なるか）を、分離軸の判定で調べます。
        //! 2つの四角の辺の向き（4本の軸）のどれかに投影して離れていれば、重ならない。
        //! @param  [in] coinHalf コインの半分の大きさ
        //--------------------------------------------------------------
        bool Overlaps(const Footprint& a, const Footprint& b, float coinHalf) {
            const float half = coinHalf + std::max(a.gap, b.gap) * 0.5f;
            const float dx = b.x - a.x;
            const float dz = b.z - a.z;
            const float distanceSq = dx * dx + dz * dz;
            const float reach      = half * 2.0f * 1.4143f;    // 角どうしが届く最大の距離（対角線）
            if(distanceSq > reach * reach)
                return false;

            const float axes[4] = {a.yaw, a.yaw + kPi * 0.5f, b.yaw, b.yaw + kPi * 0.5f};
            for(float axis : axes) {
                const float c = std::cos(axis);
                const float s = std::sin(axis);
                // 正方形を軸へ投影したときの半分の長さ（中心から辺の向き2本ぶん）
                auto radius = [&](float yaw) { return half * (std::abs(std::cos(axis - yaw)) + std::abs(std::sin(axis - yaw))); };
                if(std::abs(dx * c + dz * s) > radius(a.yaw) + radius(b.yaw))
                    return false;
            }
            return true;
        }

        //! 既に置いた四角のどれかと重なるかを返します。
        bool OverlapsAny(const std::vector<Footprint>& placed, const Footprint& candidate, float half) {
            return std::any_of(placed.begin(), placed.end(), [&](const Footprint& f) { return Overlaps(f, candidate, half); });
        }

        //! 0〜1 の乱数を返します。
        float Unit(std::mt19937& rng) { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); }

        //! min〜max の乱数を返します。
        float Range(std::mt19937& rng, float min, float max) { return min + (max - min) * Unit(rng); }

        //! 置き場所にします（向きは Y 軸まわりの yaw、傾きは tilt 度。傾けると yaw の向きの (sin, cos) 側が下がる）。
        CoinPlacement Place(float x, float y, float z, float yaw, float tiltDegrees = 0.0f) {
            CoinPlacement placement;
            placement.position = hlslpp::float3(x, y, z);
            placement.rotation = PrizeFactory::EulerDegrees(hlslpp::float3(tiltDegrees, yaw / kDegToRad, 0.0f));
            return placement;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! プッシャーの前の床に置くコインの置き場所を作ります。
    //----------------------------------------------------------------------------
    std::vector<CoinPlacement> ScatterFloor(const TableLayout& layout, const hlslpp::float3& coinHalf, std::mt19937& rng) {
        std::vector<CoinPlacement> result;

        const float size   = float(coinHalf.x) * 2.0f;
        const float half   = float(coinHalf.x);    // 重なりの判定に使う半分の大きさ（すき間は Footprint ごと）
        const float floorY = float(coinHalf.y) + layout.initialLift;

        // 置ける範囲（プッシャーが届く最前の位置の少し奥から手前の端まで、左右は側壁の内側）
        const float maxX = layout.fieldHalfWidth - layout.initialCoinSideMargin - float(coinHalf.x);
        const float minZ = layout.pusherMinFrontZ + layout.pusherAmplitude * 2.0f - layout.initialCoinFrontBack;
        const float maxZ = layout.fieldFrontZ - float(coinHalf.x);
        auto frontness   = [&](float z) { return std::clamp((z - minZ) / std::max(maxZ - minZ, 1.0f), 0.0f, 1.0f); };

        //--------------------------------------------------------------
        // 1. 床の層。プッシャーの押す力を手前の端まで伝える役なので、隙間なく詰める
        //    （ばらばらに置くと詰まり方が足りず、押しても隙間に吸われて、始めてしばらく払い出しが出ない）。
        //    その上で、1行ごとに半分ずらし（れんが積み）、位置と向きを少しだけ揺らし、奥ほど抜けを作る。
        //    間隔は「揺らした四角どうしが重ならない最小」にする（向きを j 度まで揺らすと四角の幅は cos j + sin j 倍になる）
        //--------------------------------------------------------------
        const float yawJitter = layout.initialYawJitter * kDegToRad;
        const float pitch     = size * (std::cos(yawJitter) + std::sin(yawJitter)) + layout.initialCoinGap + layout.initialPositionJitter * 2.0f;
        const int   halfCols  = static_cast<int>(maxX / pitch);

        // 手前の列から奥へ並べる（手前の列の位置をそろえ、その中の edgeCoins 枚は縁からはみ出させる）
        std::vector<Footprint> floor;
        std::vector<float>     edgeOverhang(static_cast<size_t>(halfCols * 2 + 1), 0.0f);
        std::vector<size_t>    columns(edgeOverhang.size());
        for(size_t i = 0; i < columns.size(); ++i)
            columns[i] = i;
        std::shuffle(columns.begin(), columns.end(), rng);
        for(int i = 0; i < std::min(layout.initialEdgeCoins, static_cast<int>(columns.size())); ++i)
            edgeOverhang[columns[i]] = Range(rng, layout.initialEdgeOverhangMin, layout.initialEdgeOverhangMax);

        for(int row = 0;; ++row) {
            const float rowZ = maxZ - static_cast<float>(row) * pitch;
            if(rowZ < minZ)
                break;
            const float shift = (row % 2 == 0) ? 0.0f : pitch * 0.5f;    // れんが積み
            for(int col = -halfCols; col <= halfCols; ++col) {
                const float x = static_cast<float>(col) * pitch + shift;
                if(std::abs(x) > maxX)
                    continue;

                // 奥ほど抜けを作る（holeChanceBack を densityCurve 乗で手前へ減らす。手前の列は抜かない）
                const float hole = layout.initialHoleChanceBack * std::pow(1.0f - frontness(rowZ), layout.initialDensityCurve);
                if(row > 0 && Unit(rng) < hole)
                    continue;

                // 手前の列のうち選んだコインは、縁からはみ出させる（重心は台の上）
                float z = rowZ + Range(rng, -layout.initialPositionJitter, layout.initialPositionJitter);
                if(row == 0) {
                    const float overhang = edgeOverhang[static_cast<size_t>(col + halfCols)];
                    z                    = (overhang > 0.0f) ? layout.fieldFrontZ - size * (0.5f - overhang) : rowZ;
                }
                floor.push_back(Footprint{x + Range(rng, -layout.initialPositionJitter, layout.initialPositionJitter), z,
                                          Range(rng, -yawJitter, yawJitter), layout.initialCoinGap});
            }
        }
        for(const Footprint& f : floor)
            result.push_back(Place(f.x, floorY, f.z, f.yaw));

        //--------------------------------------------------------------
        // 2. 重なり（見た目の主役）。床のコインを手前ほど選ばれやすく選び、その中心から少しずらして、ずらした向きへ下がるよう傾けて乗せる
        //    （1〜2枚にまたがって寄りかかる）。まれにもう1段（小さな山）
        //--------------------------------------------------------------
        std::vector<double> weights;
        for(const Footprint& f : floor)
            weights.push_back(0.1 + std::pow(frontness(f.z), layout.initialStackFrontBias));
        if(weights.empty())
            return result;
        std::discrete_distribution<size_t> pickBase(weights.begin(), weights.end());

        const int              stackCount = static_cast<int>(std::round(static_cast<float>(floor.size()) * layout.initialStackRatio));
        const float            layerStep  = float(coinHalf.y) * 2.0f + layout.initialLift;    // 1段上がる高さ
        std::vector<Footprint> upper;
        std::vector<Footprint> top;
        for(int n = 0; n < stackCount; ++n) {
            for(int attempt = 0; attempt < kTries; ++attempt) {
                const Footprint& base = floor[pickBase(rng)];
                const float      yaw  = Range(rng, 0.0f, kPi * 2.0f);
                const float      dist = Range(rng, layout.initialStackOffsetMin, layout.initialStackOffsetMax) * size;
                Footprint        f{base.x + std::sin(yaw) * dist, base.z + std::cos(yaw) * dist, yaw, layout.initialCoinGap};
                if(std::abs(f.x) > maxX || f.z > layout.fieldFrontZ - size * 0.3f || f.z < minZ || OverlapsAny(upper, f, half))
                    continue;

                // 傾けた分だけ高く置く（傾いた板の端が下のコインにめり込まないように）
                const float tilt = Range(rng, layout.initialStackTiltMin, layout.initialStackTiltMax);
                const float y    = floorY + layerStep + float(coinHalf.x) * std::sin(tilt * kDegToRad);
                upper.push_back(f);
                result.push_back(Place(f.x, y, f.z, yaw, tilt));

                // まれにもう1段（ほぼ真上に、少しずらして）
                if(Unit(rng) < layout.initialPileChance) {
                    Footprint pile{f.x + Range(rng, -0.15f, 0.15f) * size, f.z + Range(rng, -0.15f, 0.15f) * size, Range(rng, 0.0f, kPi * 2.0f), layout.initialCoinGap};
                    if(!OverlapsAny(top, pile, half)) {
                        top.push_back(pile);
                        result.push_back(Place(pile.x, y + layerStep + float(coinHalf.x) * 0.2f, pile.z, pile.yaw, Range(rng, 0.0f, 6.0f)));
                    }
                }
                break;
            }
        }
        return result;
    }

    //----------------------------------------------------------------------------
    //! プッシャーの上面に置くコインの置き場所を作ります。
    //----------------------------------------------------------------------------
    std::vector<CoinPlacement> ScatterPusherTop(const TableLayout& layout, const hlslpp::float3& coinHalf, std::mt19937& rng) {
        std::vector<CoinPlacement> result;
        const float half = float(coinHalf.x);
        const float maxX = layout.PusherHalfWidth() - layout.initialCoinSideMargin - float(coinHalf.x);
        // 背面パネルの手前から、始まりの位置のプッシャーの前面の少し奥まで
        const float minZ = layout.BackPanelFrontZ() + float(coinHalf.x) + 1.0f;
        const float maxZ = layout.PusherCenterZ(layout.pusherAmplitude) + layout.pusherHalfDepth - float(coinHalf.x) - 1.0f;
        const float y    = layout.PusherTopY() + float(coinHalf.y) + layout.initialLift;
        if(maxZ <= minZ || maxX <= 0.0f)
            return result;

        std::vector<Footprint> placed;
        for(int n = 0; n < layout.initialPusherTopCoins; ++n) {
            for(int attempt = 0; attempt < kTries; ++attempt) {
                Footprint f{Range(rng, -maxX, maxX), Range(rng, minZ, maxZ), Range(rng, 0.0f, kPi * 0.5f), layout.initialCoinGap};
                if(OverlapsAny(placed, f, half))
                    continue;
                placed.push_back(f);
                result.push_back(Place(f.x, y, f.z, f.yaw));
                break;
            }
        }
        return result;
    }
}    // namespace FruitMagic::CoinScatter
