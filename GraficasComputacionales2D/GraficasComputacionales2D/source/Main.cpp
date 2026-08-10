/**
 * @file main.cpp
 * @brief Punto de entrada de la demo de Steering Behaviors.
 *
 * @details Crea una ventana, registra los sistemas del ECS (Steering, Camera,
 * Render e Inspector) y arma una escena con:
 * - Un "Objetivo" controlado manualmente con las teclas W/A/S/D.
 * - Una entidad con comportamiento Seek, que persigue al objetivo.
 * - Una entidad con comportamiento Flee, que huye del objetivo.
 * - Una entidad con comportamiento Arrive, que se acerca y frena cerca del objetivo.
 * - Una camara que sigue al objetivo.
 *
 * El bucle principal procesa los eventos de la ventana, mueve al objetivo
 * segun el teclado, actualiza todos los sistemas del ECS y presenta el frame.
 */

#include "Prerequisitesr.h"
#include "Core/Window.h"
#include "ECS/Components/Path.h"
#include "ECS/Systems/PathRenderSystem.h"
#include "ECS/Registry.h"

 // Componentes
#include "ECS/Components/Camera.h"
#include "ECS/Components/InspectorName.h"
#include "ECS/Components/Movement.h"
#include "ECS/Components/Render.h"
#include "ECS/Components/SteeringBehavior.h"
#include "ECS/Components/Transform.h"

// Sistemas
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/SimpleInspectorSystem.h"
#include "ECS/Systems/SteeringSystem.h"

/**
 * @brief Arma la escena de la demo y ejecuta el bucle principal del juego.
 *
 * @return Codigo de salida del programa (0 en una terminacion normal).
 */
int main()
{
    Window window(
        800,
        600,
        "Steering Behaviors"
    );

    ECS::Registry registry;

    // --- Registro de sistemas ---
    // El orden de registro importa: Steering calcula las fuerzas de
    // movimiento antes de que Camera y Render usen la posicion resultante;
    // el Inspector se registra al final para dibujarse encima de todo lo demas.
    registry.AddSystem<ECS::SteeringSystem>();
    registry.AddSystem<ECS::CameraSystem>(window);
    registry.AddSystem<ECS::SteeringSystem>();
    registry.AddSystem<ECS::CameraSystem>(window);

    // La pista se dibuja antes que los agentes.
    registry.AddSystem<ECS::PathRenderSystem>(window);

    registry.AddSystem<ECS::RenderSystem>(window);
    registry.AddSystem<ECS::SimpleInspectorSystem>(window);
    registry.AddSystem<ECS::RenderSystem>(window);
    registry.AddSystem<ECS::SimpleInspectorSystem>(window);

    // NOTA: esta linea registra el mismo sistema (SimpleInspectorSystem) por
    // segunda vez. Si no es intencional, se puede quitar sin afectar el resto.
    registry.AddSystem<ECS::SimpleInspectorSystem>(window);

    // --- Entidad "Objetivo": el punto que el jugador mueve manualmente ---
    const ECS::EntityID player =
        registry.CreateEntity();

    registry.AddComponent<ECS::InspectorName>(
        player,
        "Objetivo"
    );

    registry.AddComponent<ECS::Transform>(
        player,
        sf::Vector2f{ -330.f, -120.f }
    );
    registry.AddComponent<ECS::Render>(
        player,
        ECS::Render::Make(
            CIRCLE,
            sf::Color(100, 250, 50)
        )
    );

    // Hacer el objetivo más pequeño
    registry.GetComponent<ECS::Transform>(
        player
    ).scale = { 0.45f, 0.45f };

    // --- Entidad con comportamiento Seek: persigue al objetivo ---
    const ECS::EntityID triangle =
        registry.CreateEntity();

    registry.AddComponent<ECS::InspectorName>(
        triangle,
        "Entidad Seek"
    );

    registry.AddComponent<ECS::Transform>(
        triangle,
        sf::Vector2f{ 220.f, 100.f }
    );

    registry.AddComponent<ECS::Render>(
        triangle,
        ECS::Render::Make(
            TRIANGLE,
            sf::Color::Cyan
        )
    );

    registry.AddComponent<ECS::Movement>(
        triangle,
        170.f, // Velocidad máxima
        350.f  // Fuerza máxima
    );

    registry.AddComponent<ECS::SteeringBehavior>(
        triangle,
        ECS::SteeringType::Seek,
        player
    );

    // --- Entidad con comportamiento Flee: huye del objetivo ---
    const ECS::EntityID rectangle =
        registry.CreateEntity();

    registry.AddComponent<ECS::InspectorName>(
        rectangle,
        "Entidad Flee"
    );

    registry.AddComponent<ECS::Transform>(
        rectangle,
        sf::Vector2f{ -220.f, 110.f }
    );

    registry.AddComponent<ECS::Render>(
        rectangle,
        ECS::Render::Make(
            RECTANGLE,
            sf::Color(255, 210, 70)
        )
    );

    registry.AddComponent<ECS::Movement>(
        rectangle,
        140.f,
        300.f
    );

    registry.AddComponent<ECS::SteeringBehavior>(
        rectangle,
        ECS::SteeringType::Flee,
        player
    );

    // --- Entidad con comportamiento Arrive: se acerca y frena cerca del objetivo ---
    const ECS::EntityID arrive =
        registry.CreateEntity();

    registry.AddComponent<ECS::InspectorName>(
        arrive,
        "Entidad Arrive"
    );

    registry.AddComponent<ECS::Transform>(
        arrive,
        sf::Vector2f{ 0.f, -230.f }
    );

    registry.AddComponent<ECS::Render>(
        arrive,
        ECS::Render::Make(
            CIRCLE,
            sf::Color(80, 140, 255)
        )
    );

    registry.GetComponent<ECS::Transform>(
        arrive
    ).scale = { 0.55f, 0.55f };

    registry.AddComponent<ECS::Movement>(
        arrive,
        180.f,
        330.f
    );

    registry.AddComponent<ECS::SteeringBehavior>(
        arrive,
        ECS::SteeringType::Arrive,
        player,
        180.f, // Radio para comenzar a frenar
        15.f   // Radio para detenerse
    );

    // --- Entidad "Camara": sigue al objetivo ---
    const ECS::EntityID cameraEntity =
        registry.CreateEntity();

    registry.AddComponent<ECS::InspectorName>(
        cameraEntity,
        "Camara"
    );

    registry.AddComponent<ECS::Transform>(
        cameraEntity,
        sf::Vector2f{ 0.f, 0.f }
    );

    auto& camera =
        registry.AddComponent<ECS::Camera>(
            cameraEntity
        );

    camera.followTarget = player;
    camera.followSpeed = 5.f;
    camera.zoom = 1.f;

    sf::Clock frameClock;

    // --- Bucle principal ---
    while (window.isOpen())
    {
        const ECS::EntityID trackEntity =
            registry.CreateEntity();

        ECS::Path& track =
            registry.AddComponent<ECS::Path>(
                trackEntity
            );

        track.radius = 55.f;
        track.closed = true;

        track.points =
        {
            { -330.f, -120.f },
            { -250.f, -230.f },
            {  20.f,  -260.f },
            {  280.f, -210.f },
            {  370.f,  -40.f },
            {  330.f,  150.f },
            {  140.f,  250.f },
            { -120.f,  250.f },
            { -320.f,  150.f },
            { -390.f,   10.f }
        };
        // Procesamiento de eventos de la ventana (cierre, resize, etc.).
        while (
            const auto event =
            window.m_window->pollEvent()
            )
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (
                const auto* resized =
                event->getIf<sf::Event::Resized>()
                )
            {
                window.handleResize(resized->size);
            }
        }

        const float deltaTime =
            frameClock.restart().asSeconds();

        // Movimiento manual del objetivo
        auto& playerTransform =
            registry.GetComponent<ECS::Transform>(
                player
            );

        sf::Vector2f direction{ 0.f, 0.f };

        if (
            sf::Keyboard::isKeyPressed(
                sf::Keyboard::Key::W
            )
            )
        {
            direction.y -= 1.f;
        }

        if (
            sf::Keyboard::isKeyPressed(
                sf::Keyboard::Key::S
            )
            )
        {
            direction.y += 1.f;
        }

        if (
            sf::Keyboard::isKeyPressed(
                sf::Keyboard::Key::A
            )
            )
        {
            direction.x -= 1.f;
        }

        if (
            sf::Keyboard::isKeyPressed(
                sf::Keyboard::Key::D
            )
            )
        {
            direction.x += 1.f;
        }

        // Normalizamos la direccion para que el movimiento diagonal no sea
        // mas rapido que el movimiento en un solo eje.
        float length = std::sqrt(
            direction.x * direction.x +
            direction.y * direction.y
        );

        if (length > 0.f)
        {
            direction.x /= length;
            direction.y /= length;

            float speed = 230.f;

            playerTransform.Translate(
                {
                    direction.x * speed * deltaTime,
                    direction.y * speed * deltaTime
                }
            );
        }

        window.clear(sf::Color::Black);

        registry.UpdateSystems(deltaTime);

        window.display();
    }

    registry.RemoveAllSystems();

    return 0;

}