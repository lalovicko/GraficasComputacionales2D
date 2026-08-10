#pragma once

#include "Prerequisitesr.h"
#include "Core/Window.h"
#include "ECS/Registry.h"
#include "ECS/System.h"
#include "ECS/Components/Path.h"

namespace ECS
{
    /**
     * @brief Dibuja las entidades que tengan el componente Path.
     */
    class PathRenderSystem : public System
    {
    private:
        Window& window;

    public:
        /**
         * @brief Constructor del sistema.
         * @param windowReference Ventana principal del proyecto.
         */
        PathRenderSystem(Window& windowReference)
            : window(windowReference)
        {}

        /**
         * @brief Dibuja todas las pistas registradas.
         * @param registry Registro principal del ECS.
         * @param deltaTime Tiempo transcurrido entre frames.
         */
        void OnUpdate(
            Registry& registry,
            float deltaTime
        ) override
        {
            (void)deltaTime;

            for (EntityID entity : registry.GetEntities())
            {
                if (!registry.IsAlive(entity))
                {
                    continue;
                }

                Path* path =
                    registry.TryGetComponent<Path>(entity);

                if (path == nullptr ||
                    path->points.size() < 2)
                {
                    continue;
                }

                DrawTrack(*path);
            }
        }

    private:
        /**
         * @brief Dibuja los segmentos que forman el circuito.
         * @param path Componente que contiene la pista.
         */
        void DrawTrack(const Path& path)
        {
            int segmentCount =
                static_cast<int>(path.points.size()) - 1;

            if (path.closed)
            {
                segmentCount++;
            }

            for (int i = 0; i < segmentCount; i++)
            {
                int next = i + 1;

                if (next >= path.points.size())
                {
                    next = 0;
                }

                sf::Vector2f start = path.points[i];
                sf::Vector2f end = path.points[next];

                // Césped o borde exterior.
                DrawLine(
                    start,
                    end,
                    path.radius * 2.f + 28.f,
                    sf::Color(190, 190, 190)
                );

                // Asfalto.
                DrawLine(
                    start,
                    end,
                    path.radius * 2.f,
                    sf::Color(55, 55, 60)
                );

                // Línea central.
                DrawLine(
                    start,
                    end,
                    3.f,
                    sf::Color(245, 210, 70)
                );
            }
        }

        /**
         * @brief Dibuja una línea gruesa entre dos posiciones.
         */
        void DrawLine(
            sf::Vector2f start,
            sf::Vector2f end,
            float thickness,
            sf::Color color
        )
        {
            sf::Vector2f difference = end - start;

            float length = std::sqrt(
                difference.x * difference.x +
                difference.y * difference.y
            );

            float angle = std::atan2(
                difference.y,
                difference.x
            );

            sf::RectangleShape line(
                { length, thickness }
            );

            line.setOrigin(
                { 0.f, thickness / 2.f }
            );

            line.setPosition(start);

            line.setRotation(
                sf::radians(angle)
            );

            line.setFillColor(color);

            window.m_window->draw(line);
        }
    };
}