//----------------------------------------------------------------------------
//! @file   TableLayout.cpp
//! @brief  プッシャー台の寸法・配置と、景品の物理の読み込み
//----------------------------------------------------------------------------
#include <FruitMagic/Game/TableLayout.hpp>

#include <FruitMagic/Game/JsonReader.hpp>

#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Log.hpp>

// 名前空間 : FruitMagic
namespace FruitMagic {

    //----------------------------------------------------------------------------
    //! レジストリに置いた台の寸法を返します。
    //----------------------------------------------------------------------------
    const TableLayout& GetTableLayout(Tsukino::ECS::Registry& registry) {
        static const TableLayout kDefault;
        return registry.HasContext<TableLayout>() ? registry.GetContext<TableLayout>() : kDefault;
    }

    //----------------------------------------------------------------------------
    //! 設定ファイルを読み込みます。
    //----------------------------------------------------------------------------
    bool TableLayout::Load(const std::string& path) {
        Json::Document doc;
        if(!Json::ParseFile(path, doc, "TableLayout"))
            return false;

        TableLayout loaded;

        if(const Json::Value* field = Json::FindObject(doc, "field")) {
            Json::Read(*field, "halfWidth", loaded.fieldHalfWidth);
            Json::Read(*field, "frontZ", loaded.fieldFrontZ);
            Json::Read(*field, "backZ", loaded.fieldBackZ);
            Json::Read(*field, "staticThickness", loaded.staticThickness);
        }
        if(const Json::Value* pusher = Json::FindObject(doc, "pusher")) {
            Json::Read(*pusher, "sideGap", loaded.pusherSideGap);
            Json::Read(*pusher, "halfHeight", loaded.pusherHalfHeight);
            Json::Read(*pusher, "sinkDepth", loaded.pusherSinkDepth);
            Json::Read(*pusher, "halfDepth", loaded.pusherHalfDepth);
            Json::Read(*pusher, "minFrontZ", loaded.pusherMinFrontZ);
            Json::Read(*pusher, "amplitude", loaded.pusherAmplitude);
            Json::Read(*pusher, "maxAmplitude", loaded.pusherMaxAmplitude);
            Json::Read(*pusher, "period", loaded.pusherPeriod);
            Json::Read(*pusher, "amplitudeChangeSpeed", loaded.pusherAmplitudeSpeed);
        }
        if(const Json::Value* panel = Json::FindObject(doc, "backPanel")) {
            Json::Read(*panel, "distance", loaded.backPanelDistance);
            Json::Read(*panel, "halfThickness", loaded.backPanelHalfThickness);
            Json::Read(*panel, "halfHeight", loaded.backPanelHalfHeight);
            Json::Read(*panel, "sink", loaded.backPanelSink);
        }
        if(const Json::Value* payout = Json::FindObject(doc, "payout")) {
            Json::Read(*payout, "halfWidth", loaded.payoutHalfWidth);
            Json::Read(*payout, "dropJudgeY", loaded.dropJudgeY);
            Json::Read(*payout, "trayTopY", loaded.trayTopY);
            Json::Read(*payout, "trayOffsetZ", loaded.trayOffsetZ);
            Json::Read(*payout, "trayHalfDepth", loaded.trayHalfDepth);
            Json::Read(*payout, "gutterExtraWidth", loaded.gutterExtraWidth);
            Json::Read(*payout, "gutterDepth", loaded.gutterDepth);
        }
        if(const Json::Value* wall = Json::FindObject(doc, "sideWall")) {
            Json::Read(*wall, "halfThickness", loaded.sideWallHalfThickness);
            Json::Read(*wall, "height", loaded.sideWallHeight);
            Json::Read(*wall, "openLength", loaded.sideWallOpenLength);
        }
        if(const Json::Value* launch = Json::FindObject(doc, "launch")) {
            Json::Read(*launch, "laneHalfWidth", loaded.launchLaneHalfWidth);
            Json::Read(*launch, "dropHeight", loaded.launchDropHeight);
            Json::Read(*launch, "markerHeight", loaded.launchMarkerHeight);
            Json::Read(*launch, "minClearance", loaded.minLaunchClearance);
        }
        if(const Json::Value* shower = Json::FindObject(doc, "coinShower")) {
            Json::Read(*shower, "backMargin", loaded.showerBackMargin);
            Json::Read(*shower, "frontMargin", loaded.showerFrontMargin);
            Json::Read(*shower, "sideMargin", loaded.showerSideMargin);
            Json::Read(*shower, "dropY", loaded.showerDropY);
            Json::Read(*shower, "maxPerFrame", loaded.showerMaxPerFrame);
        }
        if(const Json::Value* coin = Json::FindObject(doc, "coin")) {
            Json::ReadVec(*coin, "halfExtent", loaded.coinHalfExtent);
            Json::Read(*coin, "friction", loaded.coinFriction);
            Json::Read(*coin, "value", loaded.coinValue);
            Json::Read(*coin, "model", loaded.coinModel);
            Json::ReadVec(*coin, "modelRotation", loaded.coinModelRotation);
        }
        if(const Json::Value* fruit = Json::FindObject(doc, "fruit")) {
            Json::Read(*fruit, "friction", loaded.fruitFriction);
            Json::Read(*fruit, "restitution", loaded.fruitRestitution);
        }
        Json::Read(doc, "maxSimulationStep", loaded.maxSimulationStep);
        if(const Json::Value* physics = Json::FindObject(doc, "physics")) {
            Json::Read(*physics, "penetrationSlop", loaded.penetrationSlop);
            Json::Read(*physics, "speculativeContactDistance", loaded.speculativeContactDistance);
        }
        if(const Json::Value* initial = Json::FindObject(doc, "initialPrizes")) {
            Json::Read(*initial, "coinFrontBack", loaded.initialCoinFrontBack);
            Json::Read(*initial, "coinGap", loaded.initialCoinGap);
            Json::Read(*initial, "coinSideMargin", loaded.initialCoinSideMargin);
            Json::Read(*initial, "lift", loaded.initialLift);
            Json::Read(*initial, "pusherTopCoinHalfCount", loaded.pusherTopCoinHalfCount);
            Json::Read(*initial, "pusherTopCoinSpacing", loaded.pusherTopCoinSpacing);
            Json::Read(*initial, "pusherTopCoinOffsetZ", loaded.pusherTopCoinOffsetZ);
            Json::ReadArray(*initial, "fruitX", loaded.initialFruitX);
            Json::Read(*initial, "fruitStepZ", loaded.initialFruitStepZ);
        }

        //--------------------------------------------------------------
        // 寸法の矛盾はコインの滞留・すり抜けにつながるので、1つでもあれば全部を既定値に戻す
        //--------------------------------------------------------------
        std::string reason;
        if(!loaded.Validate(reason)) {
            Tsukino::Core::Log::Warn("TableLayout: " + path + " is inconsistent (" + reason + "). Using the default table.");
            *this = TableLayout{};
            return false;
        }

        *this = loaded;
        return true;
    }

    //----------------------------------------------------------------------------
    //! 寸法が矛盾していないかを調べます。
    //----------------------------------------------------------------------------
    bool TableLayout::Validate(std::string& reason) const {
        auto fail = [&](const char* text) {
            reason = text;
            return false;
        };

        if(fieldHalfWidth <= 0.0f || staticThickness <= 0.0f || fieldFrontZ <= fieldBackZ)
            return fail("the field must have a positive size");
        if(pusherHalfHeight <= pusherSinkDepth || pusherHalfDepth <= 0.0f || PusherHalfWidth() <= 0.0f)
            return fail("the pusher must have a positive size and stand above the floor");
        if(pusherAmplitude <= 0.0f || pusherMaxAmplitude < pusherAmplitude || pusherPeriod <= 0.0f)
            return fail("the pusher stroke must be positive and within maxAmplitude");
        if(payoutHalfWidth <= 0.0f || payoutHalfWidth > fieldHalfWidth || dropJudgeY <= trayTopY)
            return fail("the payout must be inside the field and the drop line above the tray");
        if(float(coinHalfExtent.x) <= 0.0f || float(coinHalfExtent.y) <= 0.0f || float(coinHalfExtent.z) <= 0.0f || maxSimulationStep <= 0.0f)
            return fail("the coin size and the simulation step must be positive");

        // 投入位置の前後にコインが収まる余裕が要る（元は static_assert）
        if(pusherMinFrontZ - BackPanelFrontZ() <= minLaunchClearance)
            return fail("there is no room for a coin between the back panel and the pusher");

        // 押し幅が最大でも、プッシャーの後端が背面パネルより奥に残る必要がある（前に出たときに背面パネルの下に隙間ができないように）
        if(pusherMinFrontZ + pusherMaxAmplitude * 2.0f - pusherHalfDepth * 2.0f >= BackPanelZ() - backPanelHalfThickness - 1.0f)
            return fail("the pusher is too short: its back end leaves the back panel at the maximum stroke");

        if(penetrationSlop <= 0.0f || speculativeContactDistance <= 0.0f)
            return fail("the physics contact tolerances must be positive");
        if(penetrationSlop >= float(coinHalfExtent.y) * 2.0f)
            return fail("penetrationSlop must be smaller than the coin thickness, or prizes sink into the coins");
        // プッシャーの横は側壁で囲う（押し幅が最大でも、プッシャーの前面より手前まで壁が要る）
        if(sideWallOpenLength < 0.0f || SideWallFrontZ() < PusherMaxFrontZ())
            return fail("the side walls must reach past the front of the pusher at the maximum stroke");
        if(launchLaneHalfWidth + float(coinHalfExtent.x) > fieldHalfWidth)
            return fail("the launch lane is wider than the field");
        if(pusherMinFrontZ + pusherMaxAmplitude * 2.0f + showerBackMargin >= fieldFrontZ - showerFrontMargin || showerMaxPerFrame < 1)
            return fail("there is no room to drop the coin shower in front of the pusher");
        return true;
    }
}    // namespace FruitMagic
