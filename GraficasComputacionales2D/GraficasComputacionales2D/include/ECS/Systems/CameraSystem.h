#pragma once

#include "Core/Window.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/System.h"

namespace ECS {

class CameraSystem final : public System {
public:
    explicit CameraSystem(Window& window) noexcept
        : m_window(window) {
    }

    void OnUpdate(Registry& registry, float deltaTime) override {
        bool cameraApplied = false;

        registry.GetView<Transform, Camera>().Each(
            [this, &registry, deltaTime, &cameraApplied]
            (EntityID, Transform& cameraTransform, Camera& camera) {
                if (cameraApplied || !camera.active) {
                    return;
                }

                if (camera.followTarget != NULL_ENTITY &&
                    registry.IsAlive(camera.followTarget)) {
                    if (const auto* target =
                        registry.TryGetComponent<Transform>(camera.followTarget)) {
                        const float blend = 1.f -
                            std::exp(-camera.followSpeed * deltaTime);
                        cameraTransform.position +=
                            (target->position - cameraTransform.position) * blend;
                    }
                }

                m_window.applyCameraView(
                    cameraTransform.position,
                    camera.zoom,
                    cameraTransform.rotation
                );
                cameraApplied = true;
            }
        );
    }

private:
    Window& m_window;
};

} // namespace ECS
