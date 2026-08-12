#pragma once

#include <iomanip>
#include <sstream>
#include <SFML/Graphics.hpp>
#include  "Prerequisitesr.h"
#include "Core/Window.h"
#include "ECS/Registry.h"
#include "ECS/System.h"
#include "ECS/Components/InspectorName.h"
#include "ECS/Components/Movement.h"
#include "ECS/Components/SteeringBehavior.h"
#include "ECS/Components/Transform.h"

namespace ECS
{
    /**
     * @class SimpleInspectorSystem
     * @brief Sistema de depuracion que dibuja un panel tipo "inspector" con SFML puro,
     * sin depender de ImGui.
     *
     * @details Muestra en pantalla los componentes de una entidad a la vez (Transform,
     * Movement y SteeringBehavior), filtrando solo entidades que tengan un componente
     * SteeringBehavior. El usuario puede alternar entre entidades con la tecla Q y
     * mostrar/ocultar el panel con la tecla I. Sirve como alternativa ligera al
     * UISystem basado en ImGui cuando esa libreria no esta disponible en el proyecto.
     */
    class SimpleInspectorSystem : public System
    {
    private:
        Window& window; ///< Referencia a la ventana principal, usada para dibujar sobre su RenderWindow.

        sf::Font font;       ///< Fuente utilizada para dibujar el texto del inspector.
        bool fontLoaded = false; ///< Indica si la fuente se cargo correctamente en OnStart.

        std::vector<EntityID> entities; ///< Lista de entidades candidatas a inspeccionar (con SteeringBehavior).

        int selectedIndex = 0; ///< Indice, dentro de `entities`, de la entidad actualmente seleccionada.

        bool inspectorVisible = true; ///< Indica si el panel del inspector se dibuja o no.
        bool QWasPressed = false;     ///< Estado de la tecla Q en el frame anterior (para detectar flanco de subida).
        bool iWasPressed = false;     ///< Estado de la tecla I en el frame anterior (para detectar flanco de subida).

    public:
        /**
         * @brief Construye el sistema de inspeccion.
         *
         * @param windowReference Referencia a la ventana principal del juego, usada
         * mas adelante para dibujar el panel encima de la escena.
         */
        SimpleInspectorSystem(Window& windowReference)
            : window(windowReference)
        {
        }

        /**
         * @brief Inicializa el sistema cargando la fuente de texto.
         *
         * @param registry Registro de entidades y componentes del ECS (no se utiliza
         * en este metodo).
         *
         * @details Intenta cargar la fuente Arial desde la ruta estandar de Windows
         * ("C:/Windows/Fonts/arial.ttf"). El resultado se guarda en `fontLoaded`; si
         * la carga falla, el inspector no dibujara texto en OnUpdate.
         */
        void OnStart(Registry& registry) override
        {
            // Registry no se utiliza al iniciar.
            (void)registry;

            // Fuente incluida normalmente en Windows.
            fontLoaded = font.openFromFile(
                "C:/Windows/Fonts/arial.ttf"
            );
        }

        /**
         * @brief Actualiza el sistema en cada frame: refresca la lista de entidades,
         * procesa el teclado y dibuja el inspector si corresponde.
         *
         * @param registry Registro de entidades y componentes del ECS.
         * @param deltaTime Tiempo transcurrido desde el frame anterior (no se utiliza).
         *
         * @details No dibuja nada si el inspector esta oculto, si la fuente no cargo,
         * o si no hay entidades con SteeringBehavior en la escena.
         */
        void OnUpdate(
            Registry& registry,
            float deltaTime
        ) override
        {
            (void)deltaTime;

            UpdateEntityList(registry);
            ReadKeyboard();

            if (!inspectorVisible)
            {
                return;
            }

            if (!fontLoaded)
            {
                return;
            }

            if (entities.empty())
            {
                return;
            }

            if (selectedIndex >= entities.size())
            {
                selectedIndex = 0;
            }

            EntityID selectedEntity =
                entities[selectedIndex];

            DrawInspector(
                registry,
                selectedEntity
            );
        }

    private:
        /**
         * @brief Reconstruye la lista de entidades candidatas a inspeccionar.
         *
         * @param registry Registro de entidades y componentes del ECS.
         *
         * @details Recorre todas las entidades del registro y conserva solo las que
         * estan vivas y tienen un componente SteeringBehavior. El resultado se guarda
         * en el miembro `entities`, reemplazando el contenido anterior.
         */
        void UpdateEntityList(Registry& registry)
        {
            entities.clear();

            // Solo mostramos entidades con comportamiento Steering.
            for (EntityID entity : registry.GetEntities())
            {
                if (!registry.IsAlive(entity))
                {
                    continue;
                }

                if (
                    registry.HasComponent<
                    SteeringBehavior
                    >(entity)
                    )
                {
                    entities.push_back(entity);
                }
            }
        }

        /**
         * @brief Procesa el estado del teclado para cambiar de entidad seleccionada
         * y mostrar/ocultar el inspector.
         *
         * @details Detecta el flanco de subida (recien presionada) de las teclas Q e I
         * comparando el estado actual contra el del frame anterior:
         * - Q: avanza `selectedIndex` a la siguiente entidad de la lista, con
         *   envolvente circular al llegar al final.
         * - I: alterna la visibilidad del inspector (`inspectorVisible`).
         */
        void ReadKeyboard()
        {
            bool QPressed =
                sf::Keyboard::isKeyPressed(
                    sf::Keyboard::Key::Q
                );

            bool iPressed =
                sf::Keyboard::isKeyPressed(
                    sf::Keyboard::Key::I
                );

            // Cambiar a la siguiente entidad.
            if (QPressed && !QWasPressed)
            {
                selectedIndex++;

                if (selectedIndex >= entities.size())
                {
                    selectedIndex = 0;
                }
            }

            // Mostrar u ocultar el inspector.
            if (iPressed && !iWasPressed)
            {
                inspectorVisible =
                    !inspectorVisible;
            }

            QWasPressed = QPressed;
            iWasPressed = iPressed;
        }

        /**
         * @brief Dibuja el panel del inspector con los datos de la entidad seleccionada.
         *
         * @param registry Registro de entidades y componentes del ECS.
         * @param entity Entidad cuyos componentes se van a mostrar en el panel.
         *
         * @details Cambia temporalmente a la vista por defecto de la ventana (para que
         * el panel se dibuje en coordenadas de pantalla y no de camara), dibuja el
         * fondo del panel, el titulo, los atajos de teclado y, si estan presentes, los
         * bloques de InspectorName, Transform, Movement y SteeringBehavior. Al final
         * restaura la vista de camara original de la ventana.
         */
        void DrawInspector(
            Registry& registry,
            EntityID entity
        )
        {
            sf::RenderWindow& renderWindow =
                *window.m_window;

            // Guardamos la cámara actual.
            sf::View previousView =
                renderWindow.getView();

            // Usamos la vista normal para la interfaz.
            renderWindow.setView(
                renderWindow.getDefaultView()
            );

            const sf::Vector2u windowSize =
                renderWindow.getSize();

            // Panel chico y semi-transparente, pegado abajo a la
            // izquierda para no taparle la pista al usuario.
            constexpr float panelWidth = 235.f;
            constexpr float panelHeight = 150.f;

            sf::RectangleShape panel(
                { panelWidth, panelHeight }
            );

            panel.setPosition(
                { 16.f, static_cast<float>(windowSize.y) - panelHeight - 16.f }
            );

            panel.setFillColor(
                sf::Color(15, 16, 22, 165)
            );

            panel.setOutlineColor(
                sf::Color(255, 255, 255, 60)
            );

            panel.setOutlineThickness(1.f);

            renderWindow.draw(panel);

            float x = panel.getPosition().x + 12.f;
            float y = panel.getPosition().y + 8.f;

            // Nombre de la entidad (hace de "titulo" del panel: menos
            // texto fijo, mas informacion util de una).
            InspectorName* name =
                registry.TryGetComponent<
                InspectorName
                >(entity);

            DrawText(
                renderWindow,
                name != nullptr
                ? name->name
                : "Entity " + std::to_string(entity),
                x,
                y,
                13,
                sf::Color(255, 220, 100)
            );

            y += 18.f;

            DrawText(
                renderWindow,
                "[Q] cambiar   [I] ocultar",
                x,
                y,
                10,
                sf::Color(160, 165, 175)
            );

            y += 20.f;

            // Componente Transform.
            Transform* transform =
                registry.TryGetComponent<
                Transform
                >(entity);

            if (transform != nullptr)
            {
                DrawText(
                    renderWindow,
                    "Pos: (" +
                    ToFixed(transform->position.x) + ", " +
                    ToFixed(transform->position.y) + ")",
                    x,
                    y,
                    12,
                    sf::Color(210, 210, 215)
                );

                y += 18.f;
            }

            // Componente Movement.
            Movement* movement =
                registry.TryGetComponent<
                Movement
                >(entity);

            if (movement != nullptr)
            {
                DrawText(
                    renderWindow,
                    "Vel max: " + ToFixed(movement->maxSpeed, 0) +
                    "   Fuerza: " + ToFixed(movement->maxForce, 0),
                    x,
                    y,
                    12,
                    sf::Color(210, 210, 215)
                );

                y += 18.f;
            }

            // Componente Steering Behavior.
            SteeringBehavior* steering =
                registry.TryGetComponent<
                SteeringBehavior
                >(entity);

            if (steering != nullptr)
            {
                DrawText(
                    renderWindow,
                    "Comportamiento: " +
                    GetBehaviorName(steering->type),
                    x,
                    y,
                    12,
                    sf::Color(80, 200, 255)
                );

                y += 18.f;

                if (steering->type == SteeringType::PathFollow)
                {
                    DrawText(
                        renderWindow,
                        "Vuelta " +
                        std::to_string(steering->pathLapCount + 1) +
                        "   Carril: " + ToFixed(steering->laneOffset, 0),
                        x,
                        y,
                        12,
                        sf::Color(210, 210, 215)
                    );
                }
                else
                {
                    DrawText(
                        renderWindow,
                        "Objetivo: " +
                        std::to_string(steering->targetEntity),
                        x,
                        y,
                        12,
                        sf::Color(210, 210, 215)
                    );
                }
            }

            // Regresamos a la cámara del juego.
            renderWindow.setView(previousView);
        }

        /**
         * @brief Redondea un float a `decimals` decimales y lo devuelve
         * como string, en vez de los ~6 decimales de std::to_string.
         */
        std::string ToFixed(float value, int decimals = 1)
        {
            std::ostringstream stream;
            stream.precision(decimals);
            stream << std::fixed << value;
            return stream.str();
        }

        /**
         * @brief Convierte un valor de SteeringType en su nombre legible.
         *
         * @param type Tipo de comportamiento de steering a convertir.
         * @return Nombre del comportamiento ("Seek", "Flee" o "Arrive"), o "Unknown"
         * si el valor no coincide con ninguno de los anteriores.
         */
        std::string GetBehaviorName(
            SteeringType type
        )
        {
            if (type == SteeringType::Seek)
            {
                return "Seek";
            }

            if (type == SteeringType::Flee)
            {
                return "Flee";
            }

            if (type == SteeringType::Arrive)
            {
                return "Arrive";
            }

            if (type == SteeringType::PathFollow)
            {
                return "PathFollow";
            }

            if (type == SteeringType::Follow)
            {
                return "Follow";
            }

            return "Unknown";
        }

        /**
         * @brief Dibuja una linea de texto sobre la ventana en una posicion dada.
         *
         * @param renderWindow Ventana de SFML sobre la que se dibuja el texto.
         * @param message Contenido del texto a dibujar.
         * @param x Posicion horizontal, en pixeles, donde se dibuja el texto.
         * @param y Posicion vertical, en pixeles, donde se dibuja el texto.
         * @param size Tamaño de la fuente, en pixeles.
         * @param color Color del texto.
         *
         * @details Usa la fuente cargada en `font` (ver OnStart). No verifica
         * `fontLoaded`; se asume que el llamador ya lo hizo antes de invocar este metodo.
         */
        void DrawText(
            sf::RenderWindow& renderWindow,
            const std::string& message,
            float x,
            float y,
            unsigned int size,
            sf::Color color
        )
        {
            sf::Text text(
                font,
                message,
                size
            );

            text.setPosition(
                { x, y }
            );

            text.setFillColor(color);

            renderWindow.draw(text);
        }
    };
}