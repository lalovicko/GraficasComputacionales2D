#pragma once

#include "Core/Window.h"
#include "ECS/Components/Render.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/System.h"

namespace ECS {

class RenderSystem final : public System {
public:
    explicit RenderSystem(Window& window) noexcept
        : m_window(window) {
    }

    void OnUpdate(Registry& registry, float) override {
        registry.GetView<Transform, Render>().Each(
            [this](EntityID, Transform& transform, Render& render) {
                if (!render.visible || !render.shape) {
                    return;
                }

                render.shape->setPosition(transform.position);
                render.shape->setRotation(sf::degrees(transform.rotation));
                render.shape->setScale(transform.scale);
                render.shape->setFillColor(render.fillColor);
                m_window.draw(*render.shape);
            }
        );
    }

private:
    Window& m_window;
};

} // namespace ECS
