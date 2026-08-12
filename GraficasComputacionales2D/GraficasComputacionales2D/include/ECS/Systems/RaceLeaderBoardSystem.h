#pragma once

#include <algorithm>
#include <SFML/Graphics.hpp>
#include "Prerequisitesr.h"
#include "Core/Window.h"
#include "ECS/Registry.h"
#include "ECS/System.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/InspectorName.h"
#include "ECS/Components/SteeringBehavior.h"

namespace ECS
{
    /**
     * @class RaceLeaderboardSystem
     * @brief Dibuja la tabla de posiciones de la carrera (arriba a la derecha)
     * y hace que la camara siga siempre al auto que va primero.
     *
     * @details Considera "corredores" a las entidades con SteeringBehavior de
     * tipo PathFollow. Cada una guarda su vuelta actual (pathLapCount), el
     * segmento de la pista en el que esta (pathSegmentHint) y que tan
     * avanzada esta dentro de ese segmento (pathProgressT) -todo esto lo
     * actualiza SteeringSystem solo-. Ordenando por esos tres valores (en
     * ese orden) se obtiene el orden de la carrera, sin necesitar calcular
     * la distancia real recorrida.
     */
    class RaceLeaderboardSystem : public System
    {
    private:
        Window& window;

        sf::Font font;
        bool fontLoaded = false;

        /// Si esta en true, la camara del juego persigue automaticamente
        /// al auto que va primero. Se puede desactivar si algun dia se
        /// quiere una camara fija o manual.
        bool followLeader = true;

        struct Standing
        {
            EntityID entity;
            std::string name;
            int lapCount;
            int segmentHint;
            float progressT;
        };

    public:
        explicit RaceLeaderboardSystem(Window& windowReference)
            : window(windowReference)
        {
        }

        void OnStart(Registry& registry) override
        {
            (void)registry;

            fontLoaded = font.openFromFile(
                "C:/Windows/Fonts/arial.ttf"
            );
        }

        void OnUpdate(Registry& registry, float deltaTime) override
        {
            (void)deltaTime;

            std::vector<Standing> standings =
                BuildStandings(registry);

            if (standings.empty())
            {
                return;
            }

            if (followLeader)
            {
                UpdateCameraTarget(
                    registry,
                    standings.front().entity
                );
            }

            if (fontLoaded)
            {
                DrawLeaderboard(standings);
            }
        }

    private:
        /**
         * @brief Junta a todos los corredores (PathFollow) y los ordena
         * de primero a ultimo.
         */
        std::vector<Standing> BuildStandings(Registry& registry)
        {
            std::vector<Standing> standings;

            registry.GetView<SteeringBehavior>().Each(
                [&registry, &standings](EntityID entity, SteeringBehavior& steering)
                {
                    if (steering.type != SteeringType::PathFollow)
                    {
                        return;
                    }

                    std::string label = "Auto " + std::to_string(entity);

                    if (const auto* inspectorName =
                        registry.TryGetComponent<InspectorName>(entity))
                    {
                        label = inspectorName->name;
                    }

                    standings.push_back(Standing{
                        entity,
                        label,
                        steering.pathLapCount,
                        steering.pathSegmentHint,
                        steering.pathProgressT
                        });
                }
            );

            std::sort(
                standings.begin(),
                standings.end(),
                [](const Standing& a, const Standing& b)
                {
                    if (a.lapCount != b.lapCount)
                    {
                        return a.lapCount > b.lapCount;
                    }

                    if (a.segmentHint != b.segmentHint)
                    {
                        return a.segmentHint > b.segmentHint;
                    }

                    return a.progressT > b.progressT;
                }
            );

            return standings;
        }

        /// Mueve el followTarget de la camara hacia el auto en primer lugar.
        void UpdateCameraTarget(Registry& registry, EntityID leaderEntity)
        {
            registry.GetView<Camera>().Each(
                [leaderEntity](EntityID, Camera& camera)
                {
                    camera.followTarget = leaderEntity;
                }
            );
        }

        /**
         * @brief Dibuja la tabla de posiciones, chiquita, en la esquina
         * superior derecha.
         */
        void DrawLeaderboard(const std::vector<Standing>& standings)
        {
            sf::RenderWindow& renderWindow = *window.m_window;

            const sf::View previousView = renderWindow.getView();
            renderWindow.setView(renderWindow.getDefaultView());

            const sf::Vector2u windowSize = renderWindow.getSize();

            constexpr float rowHeight = 20.f;
            constexpr float paddingTop = 34.f;
            constexpr float paddingBottom = 10.f;
            constexpr float panelWidth = 190.f;

            const std::size_t rowCount = std::min<std::size_t>(standings.size(), 8);
            const float panelHeight = paddingTop + paddingBottom + rowHeight * rowCount;

            sf::RectangleShape panel({ panelWidth, panelHeight });
            panel.setPosition({
                static_cast<float>(windowSize.x) - panelWidth - 16.f,
                16.f
                });
            panel.setFillColor(sf::Color(15, 16, 22, 165));
            panel.setOutlineColor(sf::Color(255, 255, 255, 60));
            panel.setOutlineThickness(1.f);
            renderWindow.draw(panel);

            const float x = panel.getPosition().x + 12.f;
            float y = panel.getPosition().y + 8.f;

            DrawText(renderWindow, "POSICIONES", x, y, 13, sf::Color(255, 255, 255, 230));
            y += 22.f;

            for (std::size_t i = 0; i < rowCount; ++i)
            {
                const Standing& standing = standings[i];

                const sf::Color rowColor = (i == 0)
                    ? sf::Color(255, 210, 90)
                    : sf::Color(225, 225, 230, 210);

                const std::string place = std::to_string(i + 1) + ". ";
                const std::string lapSuffix =
                    " (V" + std::to_string(standing.lapCount + 1) + ")";

                DrawText(
                    renderWindow,
                    place + standing.name + lapSuffix,
                    x,
                    y,
                    12,
                    rowColor
                );

                y += rowHeight;
            }

            renderWindow.setView(previousView);
        }

        void DrawText(
            sf::RenderWindow& renderWindow,
            const std::string& message,
            float x,
            float y,
            unsigned int size,
            sf::Color color)
        {
            sf::Text text(font, message, size);
            text.setPosition({ x, y });
            text.setFillColor(color);
            renderWindow.draw(text);
        }
    };
}