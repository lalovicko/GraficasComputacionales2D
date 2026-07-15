#pragma once

#include "ECS/Components/Camera.h"
#include "ECS/Components/Render.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/System.h"
#include "ECS/Components/Movement.h"
#include "ECS/Components/SteeringBehavior.h"

// El proyecto del profesor usa Dear ImGui + ImGui-SFML.
// Para no romper tu proyecto actual, este archivo funciona sin esas librerias.
// Al definir GC2D_ENABLE_IMGUI y agregar dichas dependencias, se activa el panel.
#ifdef GC2D_ENABLE_IMGUI
#include <imgui.h>
#include <imgui-SFML.h>
#endif

namespace ECS {

    /**
     * @class UISystem
     * @brief Sistema que dibuja un panel de depuracion (Outliner + Details) con Dear ImGui.
     *
     * @details Este sistema es opcional y depende del macro de compilacion GC2D_ENABLE_IMGUI:
     * - Sin el macro definido: OnUpdate no dibuja nada (solo evita el warning de parametro
     *   sin usar), por lo que el sistema puede vivir en el proyecto sin requerir ImGui.
     * - Con el macro definido: se dibujan dos ventanas de ImGui cada frame:
     *   - Outliner: lista todas las entidades con componente Transform y permite
     *     seleccionarlas con un clic.
     *   - Details: muestra y permite editar en vivo los componentes de la entidad
     *     seleccionada (Transform, Render, Camera, Movement y SteeringBehavior).
     */
    class UISystem final : public System {
    public:
        /**
         * @brief Actualiza el sistema de UI en cada frame.
         *
         * @param registry Registro de entidades y componentes del ECS.
         *
         * @details Sin GC2D_ENABLE_IMGUI definido, este metodo no hace nada visible.
         * Con el macro definido, dibuja el Outliner y luego el panel de Details.
         */
        void OnUpdate(Registry& registry, float) override {
#ifdef GC2D_ENABLE_IMGUI
            DrawOutliner(registry);
            DrawDetails(registry);
#else
            (void)registry;
#endif
        }

#ifdef GC2D_ENABLE_IMGUI
    private:
        /**
         * @brief Dibuja la ventana "Outliner" con la lista de entidades seleccionables.
         *
         * @param registry Registro de entidades y componentes del ECS.
         *
         * @details Solo se listan las entidades que tienen un componente Transform. Al
         * hacer clic sobre una entidad de la lista, esta se convierte en la entidad
         * seleccionada (m_selectedEntity) y sus componentes se muestran en "Details".
         */
        void DrawOutliner(Registry& registry) {
            ImGui::Begin("Outliner");
            registry.GetView<Transform>().Each(
                [this](EntityID entity, Transform&) {
                    const std::string label = "Entity " + std::to_string(entity);
                    const bool selected = entity == m_selectedEntity;
                    if (ImGui::Selectable(label.c_str(), selected)) {
                        m_selectedEntity = entity;
                    }
                }
            );
            ImGui::End();
        }

        /**
         * @brief Dibuja la ventana "Details" con los componentes de la entidad seleccionada.
         *
         * @param registry Registro de entidades y componentes del ECS.
         *
         * @details Si no hay ninguna entidad seleccionada, o si la entidad seleccionada
         * ya no esta viva, se muestra un mensaje pidiendo elegir una entidad en el
         * Outliner. En caso contrario, se dibuja un bloque editable por cada componente
         * presente en la entidad: Transform, Render, Camera, Movement y SteeringBehavior.
         */
        void DrawDetails(Registry& registry) {
            ImGui::Begin("Details");

            if (m_selectedEntity == NULL_ENTITY ||
                !registry.IsAlive(m_selectedEntity)) {
                ImGui::TextDisabled("Selecciona una entidad en el Outliner.");
                ImGui::End();
                return;
            }

            ImGui::Text("Entity %llu",
                static_cast<unsigned long long>(m_selectedEntity));

            if (auto* transform =
                registry.TryGetComponent<Transform>(m_selectedEntity)) {
                ImGui::SeparatorText("Transform");
                ImGui::DragFloat2("Position", &transform->position.x, 1.f);
                ImGui::DragFloat("Rotation", &transform->rotation, 1.f);
                ImGui::DragFloat2("Scale", &transform->scale.x, 0.01f);
            }

            if (auto* render = registry.TryGetComponent<Render>(m_selectedEntity)) {
                ImGui::SeparatorText("Render");
                ImGui::Checkbox("Visible", &render->visible);
            }

            if (auto* camera = registry.TryGetComponent<Camera>(m_selectedEntity)) {
                ImGui::SeparatorText("Camera");
                ImGui::Checkbox("Active", &camera->active);
                ImGui::DragFloat("Zoom", &camera->zoom, 0.01f, 0.05f, 10.f);
                ImGui::DragFloat("Follow speed", &camera->followSpeed, 0.1f, 0.f, 50.f);
            }
            if (auto* movement =
                registry.TryGetComponent<Movement>(m_selectedEntity)) {
                ImGui::SeparatorText("Movement");
                ImGui::DragFloat2("Velocity", &movement->velocity.x, 1.f);
                ImGui::DragFloat2("Acceleration", &movement->acceleration.x, 1.f);
                ImGui::DragFloat("Max Speed", &movement->maxSpeed, 1.f, 0.f, 1000.f);
                ImGui::DragFloat("Max Force", &movement->maxForce, 1.f, 0.f, 2000.f);
                ImGui::DragFloat("Mass", &movement->mass, 0.05f, 0.01f, 100.f);
                ImGui::Checkbox("Orient To Velocity", &movement->orientToVelocity);
            }

            if (auto* steering =
                registry.TryGetComponent<SteeringBehavior>(m_selectedEntity)) {
                ImGui::SeparatorText("Steering Behavior");
                ImGui::Checkbox("Steering Enabled", &steering->enabled);

                const char* behaviorNames[] = { "Seek", "Flee", "Arrive" };
                int selectedBehavior = static_cast<int>(steering->type);
                if (ImGui::Combo(
                    "Behavior",
                    &selectedBehavior,
                    behaviorNames,
                    IM_ARRAYSIZE(behaviorNames))) {
                    steering->type = static_cast<SteeringType>(selectedBehavior);
                }

                unsigned long long target =
                    static_cast<unsigned long long>(steering->targetEntity);
                if (ImGui::InputScalar(
                    "Target Entity",
                    ImGuiDataType_U64,
                    &target)) {
                    steering->targetEntity = static_cast<EntityID>(target);
                }

                ImGui::DragFloat(
                    "Slowing Radius",
                    &steering->slowingRadius,
                    1.f,
                    1.f,
                    2000.f
                );
                ImGui::DragFloat(
                    "Stop Radius",
                    &steering->stopRadius,
                    0.5f,
                    0.f,
                    500.f
                );
            }

            ImGui::End();
        }

    private:
        EntityID m_selectedEntity{ NULL_ENTITY }; ///< Entidad actualmente seleccionada en el Outliner.
#endif
    };

} // namespace ECS