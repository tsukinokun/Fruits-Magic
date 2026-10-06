//----------------------------------------------------------------------------
//! @file   PrizeFactory.cpp
//! @brief  台の部品と景品のエンティティ生成の実装
//----------------------------------------------------------------------------
#include <FruitMagic/Game/PrizeFactory.hpp>

#include <FruitMagic/Game/AssetPaths.hpp>
#include <FruitMagic/Game/FruitCatalog.hpp>
#include <FruitMagic/ECS/Component/FruitPartsComponent.hpp>
#include <FruitMagic/ECS/Component/PrizeComponent.hpp>

#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Engine/Asset/Model/ModelAsset.hpp>
#include <Tsukino/Engine/Asset/Material/MaterialAsset.hpp>
#include <Tsukino/GraphicsCommon/Node/NodeData.hpp>
#include <Tsukino/Core/ECS/Registry/Registry.hpp>
#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CollisionComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RimGlowComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/MaterialPropertyBlockComponent.hpp>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <memory>

// 名前空間 : FruitMagic
namespace FruitMagic {
    namespace {
        //--------------------------------------------------------------
        //! メッシュの AABB 上の点を、ノードの回転と移動で描画時の空間へ移します。
        //! @param  [in]     node 変換に使うノード
        //! @param  [in,out] p    変換する点（x, y, z）
        //! @note   拡縮は掛けない。モデルの読み込み時に MeshData::bounds はノードの拡縮を
        //!         掛けた後の値で作られている（例: Block.fbx は単位立方体＋ノード拡縮 50,50,10 で、
        //!         bounds が ±50,±50,±10 になっている）ため、ここで掛けると二重になる
        //--------------------------------------------------------------
        void TransformPoint(const Tsukino::GraphicsCommon::NodeData& node, float p[3]) {
            const float v[3] = {p[0], p[1], p[2]};

            // 回転（クォータニオン q で v を回す: v' = v + w*t + q.xyz × t, t = 2 * (q.xyz × v)）
            const float qx = node.rotation.x, qy = node.rotation.y, qz = node.rotation.z, qw = node.rotation.w;
            const float tx = 2.0f * (qy * v[2] - qz * v[1]);
            const float ty = 2.0f * (qz * v[0] - qx * v[2]);
            const float tz = 2.0f * (qx * v[1] - qy * v[0]);
            const float rx = v[0] + qw * tx + (qy * tz - qz * ty);
            const float ry = v[1] + qw * ty + (qz * tx - qx * tz);
            const float rz = v[2] + qw * tz + (qx * ty - qy * tx);

            // 移動
            p[0] = rx + node.translation.x;
            p[1] = ry + node.translation.y;
            p[2] = rz + node.translation.z;
        }

        //--------------------------------------------------------------
        //! モデルのスケール1での半サイズをメッシュのバウンディングボックスから求めます。
        //! @param  [in] assetManager アセットマネージャー
        //! @param  [in] handle       モデルのハンドル
        //! @param  [in] path         モデルのパス（ログ用）
        //! @param  [out] center      外接の箱の中心（取得できなかった場合は原点）
        //! @return 半サイズ。取得できなかった場合は (1,1,1)
        //--------------------------------------------------------------
        hlslpp::float3 MeasureModelHalfExtent(Tsukino::Asset::AssetManager& assetManager, Tsukino::Asset::AssetHandle handle, const std::string& path,
                                              hlslpp::float3& center) {
            center     = hlslpp::float3(0.0f, 0.0f, 0.0f);
            auto model = std::dynamic_pointer_cast<Tsukino::Asset::ModelAsset>(assetManager.Get(handle));
            if(!model || model->modelData.meshes.empty()) {
                Tsukino::Core::Log::Warn("PrizeFactory: failed to measure model size: " + path + ". Falling back to 1.");
                return hlslpp::float3(1.0f, 1.0f, 1.0f);
            }

            //--------------------------------------------------------------
            // 描画時と同じく、各メッシュの AABB をノードの回転・移動で移してから合成する。
            // メッシュ単体の AABB だけを見ると、Z-up で作られた FBX（ルートノードで Y-up へ回している）は
            // 軸が入れ替わったままの大きさになってしまう
            //--------------------------------------------------------------
            const auto& nodes  = model->modelData.nodes;
            const auto& meshes = model->modelData.meshes;

            float minPos[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
            float maxPos[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
            auto  addPoint  = [&](const float p[3]) {
                for(int a = 0; a < 3; ++a) {
                    minPos[a] = std::min(minPos[a], p[a]);
                    maxPos[a] = std::max(maxPos[a], p[a]);
                }
            };

            for(size_t n = 0; n < nodes.size(); ++n) {
                for(std::uint32_t meshIndex : nodes[n].meshIndices) {
                    if(meshIndex >= meshes.size())
                        continue;
                    const auto& b = meshes[meshIndex].bounds;

                    // AABB の8頂点をノードのローカル変換で移す。
                    // エンジンの ModelSystem はスキン無しのメッシュに「そのノード自身の」変換だけを掛けて
                    // 描画している（親の変換は掛けない）ので、ここも同じにする
                    for(int corner = 0; corner < 8; ++corner) {
                        float p[3] = {(corner & 1) ? b.max.x : b.min.x, (corner & 2) ? b.max.y : b.min.y, (corner & 4) ? b.max.z : b.min.z};
                        TransformPoint(nodes[n], p);
                        addPoint(p);
                    }
                }
            }

            // ノードにメッシュがぶら下がっていない形式だった場合は、メッシュ単体の AABB を使う
            if(minPos[0] > maxPos[0]) {
                for(const auto& mesh : meshes) {
                    const float lo[3] = {mesh.bounds.min.x, mesh.bounds.min.y, mesh.bounds.min.z};
                    const float hi[3] = {mesh.bounds.max.x, mesh.bounds.max.y, mesh.bounds.max.z};
                    addPoint(lo);
                    addPoint(hi);
                }
            }

            // 厚みが0の軸（板状のモデル）で割り算が壊れないよう、下限を設ける
            const float hx = std::max(0.5f * (maxPos[0] - minPos[0]), 1.0e-3f);
            const float hy = std::max(0.5f * (maxPos[1] - minPos[1]), 1.0e-3f);
            const float hz = std::max(0.5f * (maxPos[2] - minPos[2]), 1.0e-3f);
            center         = hlslpp::float3(0.5f * (maxPos[0] + minPos[0]), 0.5f * (maxPos[1] + minPos[1]), 0.5f * (maxPos[2] + minPos[2]));
            Tsukino::Core::Log::Info("PrizeFactory: " + path + " half extent = " + std::to_string(hx) + ", " + std::to_string(hy) + ", " + std::to_string(hz));
            return hlslpp::float3(hx, hy, hz);
        }

        //--------------------------------------------------------------
        //! モデルの（最初の）マテリアルの基本色を返します。
        //! @param  [in] assetManager アセットマネージャー
        //! @param  [in] handle       モデルのハンドル
        //! @return 基本色の RGB（マテリアルが無ければ白）
        //--------------------------------------------------------------
        hlslpp::float3 ModelBaseColor(Tsukino::Asset::AssetManager& assetManager, Tsukino::Asset::AssetHandle handle) {
            auto model = std::dynamic_pointer_cast<Tsukino::Asset::ModelAsset>(assetManager.Get(handle));
            if(!model || model->materialHandles.empty())
                return hlslpp::float3(1.0f, 1.0f, 1.0f);
            auto material = std::dynamic_pointer_cast<Tsukino::Asset::MaterialAsset>(assetManager.Get(model->materialHandles.front()));
            if(!material)
                return hlslpp::float3(1.0f, 1.0f, 1.0f);
            const auto& c = material->data.baseColor;
            return hlslpp::float3(c.x, c.y, c.z);
        }

        //--------------------------------------------------------------
        //! Dynamic 剛体の共通設定を行います。
        //! @param  [in,out] rb       設定する剛体
        //! @param  [in]     friction 摩擦係数
        //--------------------------------------------------------------
        void SetupDynamicBody(Tsukino::BuiltIn::ECS::RigidbodyComponent& rb, float friction) {
            rb.type            = Tsukino::BuiltIn::ECS::RigidbodyType::Dynamic;
            rb.friction        = friction;
            rb.freezeRotationX = false;
            rb.freezeRotationY = false;
            rb.freezeRotationZ = false;
        }
    }    // namespace

    //----------------------------------------------------------------------------
    //! 生成に使うモデルを読み込みます。
    //----------------------------------------------------------------------------
    void PrizeFactory::Initialize(Tsukino::Asset::AssetManager& assetManager, const TableLayout& layout, const StageConfig& stage) {
        m_assetManager = &assetManager;
        m_layout       = layout;
        m_stage        = stage;
        m_models.clear();
        GetModel(AssetPaths::kBlockModel);
    }

    //----------------------------------------------------------------------------
    //! モデルを読み込み、大きさを測ります（同じパスは2回目から使い回す）。
    //----------------------------------------------------------------------------
    const PrizeFactory::ModelInfo& PrizeFactory::GetModel(const std::string& path) {
        auto it = m_models.find(path);
        if(it != m_models.end())
            return it->second;

        ModelInfo info;
        if(m_assetManager) {
            info.handle     = m_assetManager->Load(Tsukino::Core::Path(path));
            info.halfExtent = MeasureModelHalfExtent(*m_assetManager, info.handle, path, info.center);
            info.baseColor  = ModelBaseColor(*m_assetManager, info.handle);
        }
        return m_models.emplace(path, info).first->second;
    }

    //----------------------------------------------------------------------------
    //! 見た目の色を付けます（モデルの基本色 × 色。白なら何もしない）。
    //----------------------------------------------------------------------------
    void PrizeFactory::SetColor(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity, const ModelInfo& model, const hlslpp::float3& color) {
        if(color.x >= 1.0f && color.y >= 1.0f && color.z >= 1.0f)
            return;
        auto& block     = registry.AddComponent<Tsukino::BuiltIn::ECS::MaterialPropertyBlockComponent>(entity);
        block.baseColor = hlslpp::float4(model.baseColor * color, 1.0f);
    }

    //----------------------------------------------------------------------------
    //! 箱型の物体（見た目＋Boxコライダー＋剛体）を生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateBox(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                                 Tsukino::BuiltIn::ECS::RigidbodyType type, const hlslpp::float3& color) {
        Tsukino::ECS::Entity e = CreateVisualBox(registry, position, halfExtent, 1.0f, color);

        // コライダーはスケールの影響を受けないので、大きさを直接指定する
        Tsukino::BuiltIn::ECS::CollisionComponent& collision = registry.AddComponent<Tsukino::BuiltIn::ECS::CollisionComponent>(e);
        collision.type                                       = Tsukino::BuiltIn::ECS::ColliderType::Box;
        collision.extent                                     = halfExtent;
        collision.isSensor                                   = false;

        Tsukino::BuiltIn::ECS::RigidbodyComponent& rb = registry.AddComponent<Tsukino::BuiltIn::ECS::RigidbodyComponent>(e);
        rb.type                                       = type;
        if(type == Tsukino::BuiltIn::ECS::RigidbodyType::Dynamic) {
            SetupDynamicBody(rb, m_layout.coinFriction);    // 動く箱はコイン
        }

        return e;
    }

    //----------------------------------------------------------------------------
    //! 見た目だけの箱（コライダー無し）を生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateVisualBox(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                                       float opacity, const hlslpp::float3& color) {
        const ModelInfo&     block = GetModel(AssetPaths::kBlockModel);
        Tsukino::ECS::Entity e     = registry.CreateEntity();

        // 見た目は箱モデルを伸縮させて合わせる
        Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
        transform.position                                   = position;
        transform.scale                                      = halfExtent / block.halfExtent;
        transform.dirty                                      = true;

        Tsukino::BuiltIn::ECS::ModelComponent& model = registry.AddComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e);
        model.modelHandle                            = block.handle;
        model.visible                                = true;
        model.opacity                                = opacity;
        SetColor(registry, e, block, color);

        return e;
    }

    //----------------------------------------------------------------------------
    //! 景品のコインを生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateCoin(Tsukino::ECS::Registry& registry, const hlslpp::float3& position) {
        Tsukino::ECS::Entity e = entt::null;
        if(m_layout.coinModel.empty()) {
            //--------------------------------------------------------------
            // モデルの指定が無ければ、金色の箱（以前の見た目）
            //--------------------------------------------------------------
            e = CreateBox(registry, position, CoinHalfExtent(), Tsukino::BuiltIn::ECS::RigidbodyType::Dynamic);
            SetColor(registry, e, GetModel(AssetPaths::kBlockModel), m_stage.coinColor);
            Tsukino::BuiltIn::ECS::RimGlowComponent& rim = registry.AddComponent<Tsukino::BuiltIn::ECS::RimGlowComponent>(e);
            rim.active                                   = true;
            rim.rimColor                                 = m_stage.coinColor;
            rim.rimIntensity                             = m_stage.coinGlow;
            rim.rimPower                                 = m_stage.prizeRimPower;
        } else {
            //--------------------------------------------------------------
            // 丸いコイン: 見た目はモデル（子）、当たり判定は箱のまま。
            // 円柱の当たり判定も試したが、プッシャーの前でコイン同士が乗り上げて重なり、押す力が手前へ伝わらない
            // （払い出し/投入が 100% → 8%）。箱は平らな面で押し合うので、敷き詰めたコインを列ごと押し出せる
            //--------------------------------------------------------------
            e = registry.CreateEntity();
            Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            transform.position                                   = position;
            transform.dirty                                      = true;

            Tsukino::BuiltIn::ECS::CollisionComponent& collision = registry.AddComponent<Tsukino::BuiltIn::ECS::CollisionComponent>(e);
            collision.type                                       = Tsukino::BuiltIn::ECS::ColliderType::Box;
            collision.extent                                     = CoinHalfExtent();
            collision.isSensor                                   = false;

            Tsukino::BuiltIn::ECS::RigidbodyComponent& rb = registry.AddComponent<Tsukino::BuiltIn::ECS::RigidbodyComponent>(e);
            SetupDynamicBody(rb, m_layout.coinFriction);

            Tsukino::ECS::Entity visual = AttachModelVisual(registry, e, m_layout.coinModel, hlslpp::float3(1.0f, 1.0f, 1.0f), CoinHalfExtent(),
                                                            m_layout.coinModelRotation, false, 1.0f);

            // 金属のマテリアル（Table.json の coin.material）に差し替える。モデルのマテリアルはそのまま残る
            if(!m_layout.coinMaterial.empty() && m_assetManager) {
                Tsukino::Asset::AssetRef material(m_assetManager->Load(Tsukino::Core::Path(m_layout.coinMaterial)));
                material.path = m_layout.coinMaterial;
                registry.GetComponent<Tsukino::BuiltIn::ECS::ModelComponent>(visual).materials.assign(1, material);
            }
            Tsukino::BuiltIn::ECS::RimGlowComponent& rim = registry.AddComponent<Tsukino::BuiltIn::ECS::RimGlowComponent>(visual);
            rim.active                                   = true;
            rim.rimColor                                 = m_stage.coinColor;
            rim.rimIntensity                             = m_stage.coinGlow;
            rim.rimPower                                 = m_stage.prizeRimPower;
        }

        ECS::PrizeComponent& prize = registry.AddComponent<ECS::PrizeComponent>(e);
        prize.kind                 = ECS::PrizeKind::Coin;
        prize.value                = m_layout.coinValue;

        return e;
    }

    //----------------------------------------------------------------------------
    //! 果物を定義データから生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateFruit(Tsukino::ECS::Registry& registry, const FruitDef& def, int fruitIndex, int variantIndex,
                                                   const hlslpp::float3& color, float glow, const hlslpp::float3& position) {
        // 見た目の色は果物（バリエーション）の色（MaterialPropertyBlockComponent でマテリアルの基本色を置き換える）。
        // 作り込んだモデルで通常の色のときは、モデル自身の色のまま（tintBase が false のとき）
        const bool           keepOwnColor = def.customModel && !def.tintBase && color.x == def.color.x && color.y == def.color.y && color.z == def.color.z;
        const hlslpp::float3 tint         = keepOwnColor ? hlslpp::float3(1.0f, 1.0f, 1.0f) : color;
        const ModelInfo&     modelInfo    = GetModel(def.modelPath);
        Tsukino::ECS::Entity e            = registry.CreateEntity();

        //--------------------------------------------------------------
        // 当たり判定の外形と、それに見た目を合わせるための大きさ
        //--------------------------------------------------------------
        Tsukino::BuiltIn::ECS::CollisionComponent& collision = registry.AddComponent<Tsukino::BuiltIn::ECS::CollisionComponent>(e);
        collision.isSensor                                   = false;

        hlslpp::float3 visualHalfExtent;
        switch(def.shape) {
            case FruitShape::Box:
                collision.type   = Tsukino::BuiltIn::ECS::ColliderType::Box;
                collision.extent = def.halfExtent;
                visualHalfExtent = def.halfExtent;
                break;
            case FruitShape::Capsule:
                collision.type   = Tsukino::BuiltIn::ECS::ColliderType::Capsule;
                collision.extent = hlslpp::float3(def.radius, def.halfHeight, 0.0f);    // Capsule は x が半径、y が円柱部分の半分の高さ
                visualHalfExtent = hlslpp::float3(def.radius, def.halfHeight + def.radius, def.radius);
                break;
            case FruitShape::Sphere:
            default:
                collision.type   = Tsukino::BuiltIn::ECS::ColliderType::Sphere;
                collision.extent = hlslpp::float3(def.radius, 0.0f, 0.0f);    // Sphere は x が半径
                visualHalfExtent = hlslpp::float3(def.radius, def.radius, def.radius);
                break;
        }

        Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
        transform.position                                   = position;
        transform.dirty                                      = true;

        //--------------------------------------------------------------
        // 見た目。作り込んだモデルは子に置いて当たり判定に合わせる（原点が底にあるなど、中心がずれているため）。
        // 球・箱を組み合わせた見た目は、今までどおりこのエンティティを拡縮して合わせる
        //--------------------------------------------------------------
        Tsukino::ECS::Entity visual = e;
        if(def.customModel) {
            visual = AttachModelVisual(registry, e, def.modelPath, tint, visualHalfExtent, def.modelRotation, true, def.modelScale);
        } else {
            transform.scale = visualHalfExtent / modelInfo.halfExtent;
            Tsukino::BuiltIn::ECS::ModelComponent& model = registry.AddComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e);
            model.modelHandle                            = modelInfo.handle;
            model.visible                                = true;
            SetColor(registry, e, modelInfo, tint);
        }

        // 輪郭を果物の色で少し光らせて、台の上で目立たせる（ポップな見た目の仮演出）
        Tsukino::BuiltIn::ECS::RimGlowComponent& rim = registry.AddComponent<Tsukino::BuiltIn::ECS::RimGlowComponent>(visual);
        rim.active                                   = true;
        rim.rimColor                                  = color;
        rim.rimIntensity                              = glow;
        rim.rimPower                                  = m_stage.prizeRimPower;

        Tsukino::BuiltIn::ECS::RigidbodyComponent& rb = registry.AddComponent<Tsukino::BuiltIn::ECS::RigidbodyComponent>(e);
        SetupDynamicBody(rb, m_layout.fruitFriction);
        rb.mass        = def.mass;
        rb.restitution = m_layout.fruitRestitution;

        ECS::PrizeComponent& prize = registry.AddComponent<ECS::PrizeComponent>(e);
        prize.kind                 = ECS::PrizeKind::Fruit;
        prize.value                = def.value;
        prize.fruitIndex           = fruitIndex;
        prize.variantIndex         = variantIndex;

        if(!def.customModel)
            AttachParts(registry, e, def, modelInfo.halfExtent);
        return e;
    }

    //----------------------------------------------------------------------------
    //! 作り込んだモデルを、当たり判定に合わせて子のエンティティとして付けます。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::AttachModelVisual(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity parent, const std::string& path,
                                                         const hlslpp::float3& color, const hlslpp::float3& targetHalf, const hlslpp::float3& rotation,
                                                         bool uniform, float extraScale) {
        const ModelInfo& info = GetModel(path);

        //--------------------------------------------------------------
        // 向きの補正を掛けた後の各軸が、モデルのどの軸から来たかを求める（90 度刻みの補正を想定。
        // 回転行列の各成分の絶対値で、合わせる大きさをモデルの軸へ戻す）
        //--------------------------------------------------------------
        const hlslpp::quaternion q = EulerDegrees(rotation);
        auto rotate = [&](const hlslpp::float3& v) {
            const hlslpp::float3 u(q.x, q.y, q.z);
            const hlslpp::float3 t = 2.0f * hlslpp::cross(u, v);
            return v + q.w * t + hlslpp::cross(u, t);
        };
        const hlslpp::float3 axisX = hlslpp::abs(rotate(hlslpp::float3(1.0f, 0.0f, 0.0f)));
        const hlslpp::float3 axisY = hlslpp::abs(rotate(hlslpp::float3(0.0f, 1.0f, 0.0f)));
        const hlslpp::float3 axisZ = hlslpp::abs(rotate(hlslpp::float3(0.0f, 0.0f, 1.0f)));
        // モデルの各軸が向く先での、合わせる大きさ
        const hlslpp::float3 localTarget(hlslpp::dot(axisX, targetHalf), hlslpp::dot(axisY, targetHalf), hlslpp::dot(axisZ, targetHalf));

        hlslpp::float3 scale = localTarget / info.halfExtent;
        if(uniform) {
            // 縦横比を保ち、当たり判定に収まる最大の大きさにする
            const float s = std::min(std::min(float(scale.x), float(scale.y)), float(scale.z));
            scale         = hlslpp::float3(s, s, s);
        }
        scale *= extraScale;

        //--------------------------------------------------------------
        // 子のエンティティ。モデルの外接の箱の中心が親の中心（当たり判定の中心）に来るようにずらす
        //--------------------------------------------------------------
        Tsukino::ECS::Entity                       e         = registry.CreateEntity();
        Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
        transform.parent                                     = parent;
        transform.rotation                                   = q;
        transform.scale                                      = scale;
        transform.position                                   = -rotate(info.center * scale);
        transform.dirty                                      = true;

        Tsukino::BuiltIn::ECS::ModelComponent& model = registry.AddComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e);
        model.modelHandle                            = info.handle;
        model.visible                                = true;
        SetColor(registry, e, info, color);

        // 親を消すときに一緒に消す（DestroyPrize）
        ECS::FruitPartsComponent* owned = registry.try_get<ECS::FruitPartsComponent>(parent);
        if(!owned)
            owned = &registry.AddComponent<ECS::FruitPartsComponent>(parent);
        owned->parts.push_back(e);
        return e;
    }

    //----------------------------------------------------------------------------
    //! 見た目だけの球（コライダー無し）を生成します。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateVisualBall(Tsukino::ECS::Registry& registry, const hlslpp::float3& position, const hlslpp::float3& halfExtent,
                                                        const hlslpp::float3& color) {
        const ModelInfo&     ball = GetModel(AssetPaths::kBallModel);
        Tsukino::ECS::Entity e    = registry.CreateEntity();

        Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
        transform.position                                   = position;
        transform.scale                                      = halfExtent / ball.halfExtent;
        transform.dirty                                      = true;

        Tsukino::BuiltIn::ECS::ModelComponent& model = registry.AddComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e);
        model.modelHandle                            = ball.handle;
        model.visible                                = true;
        SetColor(registry, e, ball, color);
        return e;
    }

    //----------------------------------------------------------------------------
    //! 見た目だけのモデル（コライダー無し）を置きます。
    //----------------------------------------------------------------------------
    Tsukino::ECS::Entity PrizeFactory::CreateVisualModel(Tsukino::ECS::Registry& registry, const std::string& path, const hlslpp::float3& position,
                                                         const hlslpp::float3& rotation, const hlslpp::float3& scale) {
        const ModelInfo&     info = GetModel(path);
        Tsukino::ECS::Entity e    = registry.CreateEntity();

        Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
        transform.position                                   = position;
        transform.rotation                                   = EulerDegrees(rotation);
        transform.scale                                      = scale;
        transform.dirty                                      = true;

        Tsukino::BuiltIn::ECS::ModelComponent& model = registry.AddComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e);
        model.modelHandle                            = info.handle;
        model.visible                                = true;
        return e;
    }

    //----------------------------------------------------------------------------
    //! 果物に飾りのパーツを付けます。
    //----------------------------------------------------------------------------
    void PrizeFactory::AttachParts(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity fruit, const FruitDef& def, const hlslpp::float3& fruitModelHalf) {
        if(def.parts.empty())
            return;

        ECS::FruitPartsComponent& owned = registry.AddComponent<ECS::FruitPartsComponent>(fruit);
        for(const FruitPart& part : def.parts) {
            const ModelInfo& model = GetModel(part.shape == FruitPartShape::Sphere ? AssetPaths::kBallModel : AssetPaths::kBlockModel);
            Tsukino::ECS::Entity e = registry.CreateEntity();

            //--------------------------------------------------------------
            // 果物を親にする。親のスケールは「果物の見た目の半サイズ ÷ 果物のモデルの半サイズ」なので、
            // ローカルの位置・スケールを果物のモデルの半サイズで書けば、果物の大きさに対する比率どおりになる
            //--------------------------------------------------------------
            Tsukino::BuiltIn::ECS::TransformComponent& transform = registry.AddComponent<Tsukino::BuiltIn::ECS::TransformComponent>(e);
            transform.parent                                     = fruit;
            transform.position                                   = part.offset * fruitModelHalf;
            transform.rotation                                   = EulerDegrees(part.rotation);
            transform.scale                                      = part.size * fruitModelHalf / model.halfExtent;
            transform.dirty                                      = true;

            Tsukino::BuiltIn::ECS::ModelComponent& component = registry.AddComponent<Tsukino::BuiltIn::ECS::ModelComponent>(e);
            component.modelHandle                            = model.handle;
            component.visible                                = true;
            SetColor(registry, e, model, part.color);

            if(part.glow > 0.0f) {
                Tsukino::BuiltIn::ECS::RimGlowComponent& rim = registry.AddComponent<Tsukino::BuiltIn::ECS::RimGlowComponent>(e);
                rim.active                                   = true;
                rim.rimColor                                 = part.color;
                rim.rimIntensity                             = part.glow;
                rim.rimPower                                 = m_stage.prizeRimPower;
            }

            owned.parts.push_back(e);
        }
    }

    //----------------------------------------------------------------------------
    //! 景品を破棄予約します。果物の飾りのパーツも一緒に破棄します。
    //----------------------------------------------------------------------------
    void PrizeFactory::DestroyPrize(Tsukino::ECS::Registry& registry, Tsukino::ECS::Entity entity) {
        if(auto* parts = registry.try_get<ECS::FruitPartsComponent>(entity)) {
            for(Tsukino::ECS::Entity part : parts->parts)
                registry.QueueDestroy(part);
        }
        registry.QueueDestroy(entity);
    }

    //----------------------------------------------------------------------------
    //! 度で表した回転（X → Y → Z の順に回す）をクォータニオンにします。
    //----------------------------------------------------------------------------
    hlslpp::quaternion PrizeFactory::EulerDegrees(const hlslpp::float3& degrees) {
        constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;

        // 各軸の回転を作り、X → Y → Z の順に掛ける（q = qz * qy * qx）
        auto axis = [](float x, float y, float z, float radians) {
            const float s = std::sin(radians * 0.5f);
            return hlslpp::float4(x * s, y * s, z * s, std::cos(radians * 0.5f));
        };
        auto multiply = [](const hlslpp::float4& a, const hlslpp::float4& b) {
            // (a * b) を (x, y, z, w) の成分で計算する
            return hlslpp::float4(float(a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y), float(a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x),
                                  float(a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w), float(a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z));
        };

        const hlslpp::float4 qx = axis(1.0f, 0.0f, 0.0f, float(degrees.x) * kDegToRad);
        const hlslpp::float4 qy = axis(0.0f, 1.0f, 0.0f, float(degrees.y) * kDegToRad);
        const hlslpp::float4 qz = axis(0.0f, 0.0f, 1.0f, float(degrees.z) * kDegToRad);
        const hlslpp::float4 q  = multiply(qz, multiply(qy, qx));
        return hlslpp::quaternion(float(q.x), float(q.y), float(q.z), float(q.w));
    }
}    // namespace FruitMagic
