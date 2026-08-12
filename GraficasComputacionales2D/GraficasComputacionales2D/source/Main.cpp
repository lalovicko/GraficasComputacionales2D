/**
 * @file main.cpp
 * @brief Punto de entrada: pista + carrera con Path Following, leaderboard
 * y camara que sigue al que va primero.
 *
 * @details Crea una ventana, registra los sistemas del ECS (Steering, Camera,
 * PathRender, Render, Leaderboard e Inspector) y arma una carrera con:
 * - Una pista (componente Path) con curvas redondeadas y una chicana en S.
 * - Varios "corredores" (RacerConfig) que NO se persiguen entre si: cada uno
 *   sigue la pista de forma independiente con PathFollow (inspirado en
 *   https://natureofcode.com/autonomous-agents/#path-following), con su
 *   propia velocidad y su propio "carril" (laneOffset, un desplazamiento
 *   lateral fijo respecto al centro de la pista). Asi el auto mas rapido
 *   naturalmente alcanza y rebasa a uno mas lento, sin necesitar logica de
 *   colisiones ni de adelantamiento explicita.
 * - Un RaceLeaderboardSystem que ordena a los corredores por vueltas y
 *   progreso sobre la pista, dibuja la tabla de posiciones (arriba a la
 *   derecha) y mueve la camara para que siga siempre al que va primero.
 *
 * El bucle principal procesa los eventos de la ventana, actualiza todos los
 * sistemas del ECS y presenta el frame. Toda la escena (pista + corredores)
 * se arma UNA sola vez antes del bucle.
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
#include "ECS/Systems/RaceLeaderboardSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/SimpleInspectorSystem.h"
#include "ECS/Systems/SteeringSystem.h"

namespace
{
    /// Configuracion de un corredor: que imagen usa, de que color se tine,
    /// que tan rapido es y en que "carril" corre.
    struct RacerConfig
    {
        std::string name;
        std::string texturePath;
        sf::Color tint;
        float maxSpeed;
        float maxForce;
        float laneOffset;
    };
}

/**
 * @brief Arma la carrera y ejecuta el bucle principal del juego.
 *
 * @return Codigo de salida del programa (0 en una terminacion normal).
 */
int main()
{
    Window window(
        800,
        600,
        "Steering Behaviors - Path Following"
    );

    ECS::Registry registry;

    // --- Registro de sistemas ---
    // El orden de registro importa: Steering calcula las fuerzas de
    // movimiento antes de que Camera y Render usen la posicion resultante.
    // PathRender dibuja la pista antes que los autos, Render dibuja los
    // autos, Leaderboard calcula posiciones y mueve la camara al primer
    // lugar, y el Inspector se registra al final para dibujarse encima
    // de todo lo demas.
    registry.AddSystem<ECS::SteeringSystem>();
    registry.AddSystem<ECS::CameraSystem>(window);
    registry.AddSystem<ECS::PathRenderSystem>(window);
    registry.AddSystem<ECS::RenderSystem>(window);
    registry.AddSystem<ECS::RaceLeaderboardSystem>(window);
    registry.AddSystem<ECS::SimpleInspectorSystem>(window);

    // --- Pista ---
    // Se crea UNA sola vez, antes del bucle (nunca dentro del
    // while(window.isOpen())).
    //
    // Un rectangulo con las 4 esquinas redondeadas (varios puntos por
    // esquina, en vez de un solo vertice filoso) mas una "S" suave de un
    // solo lado. Sigue siendo una simple lista de puntos -no hay curvas
    // de verdad ni codigo nuevo-, pero al usar hartos puntos con giros
    // chiquitos entre si, se ve como una curva prolija en vez de cortada.
    const ECS::EntityID trackEntity =
        registry.CreateEntity();

    ECS::Path& track =
        registry.AddComponent<ECS::Path>(
            trackEntity
        );

    // Pista ancha, para que quepan varios carriles (laneOffset) uno al
    // lado del otro sin que nadie se salga del asfalto.
    track.radius = 85.f;
    track.closed = true;

    track.points =
    {
        {   -290.0f,   -300.0f },
        {    290.0f,   -300.0f },
        {    330.2f,   -293.6f },
        {    366.4f,   -275.2f },
        {    395.2f,   -246.4f },
        {    413.6f,   -210.2f },
        {    420.0f,   -170.0f },
        {    419.4f,   -158.3f },
        {    417.7f,   -146.6f },
        {    415.2f,   -134.8f },
        {    412.0f,   -123.1f },
        {    408.6f,   -111.4f },
        {    405.4f,    -99.7f },
        {    402.8f,    -87.9f },
        {    401.2f,    -76.2f },
        {    400.8f,    -64.5f },
        {    401.7f,    -52.8f },
        {    404.0f,    -41.0f },
        {    407.6f,    -29.3f },
        {    412.1f,    -17.6f },
        {    417.3f,     -5.9f },
        {    422.7f,      5.9f },
        {    427.9f,     17.6f },
        {    432.4f,     29.3f },
        {    436.0f,     41.0f },
        {    438.3f,     52.8f },
        {    439.2f,     64.5f },
        {    438.8f,     76.2f },
        {    437.2f,     87.9f },
        {    434.6f,     99.7f },
        {    431.4f,    111.4f },
        {    428.0f,    123.1f },
        {    424.8f,    134.8f },
        {    422.3f,    146.6f },
        {    420.6f,    158.3f },
        {    420.0f,    170.0f },
        {    413.6f,    210.2f },
        {    395.2f,    246.4f },
        {    366.4f,    275.2f },
        {    330.2f,    293.6f },
        {    290.0f,    300.0f },
        {   -290.0f,    300.0f },
        {   -330.2f,    293.6f },
        {   -366.4f,    275.2f },
        {   -395.2f,    246.4f },
        {   -413.6f,    210.2f },
        {   -420.0f,    170.0f },
        {   -420.0f,   -170.0f },
        {   -413.6f,   -210.2f },
        {   -395.2f,   -246.4f },
        {   -366.4f,   -275.2f },
        {   -330.2f,   -293.6f }
    };

    // --- Corredores ---
    // Ahora si: cada uno en su propio "carril" (laneOffset), asi que el
    // mas rapido rebasa de lado, no encimado. Probe esta combinacion en
    // una simulacion en Python antes de pasarla aca: los 4 se quedan
    // comodos dentro de la pista (radius=85) en la parte mas cerrada de
    // la chicana.
    //
    // OJO -detalle raro pero real de ESTA pista-: la chicana no es
    // simetrica, asi que un carril "hacia un lado" (offset negativo) se
    // sale de la pista mucho mas facil que uno "hacia el otro lado"
    // (offset positivo), aunque el desplazamiento sea igual de chico. Por
    // eso los 4 carriles de aca son todos positivos: es el lado que la
    // pista aguanta bien. Si en algun momento quieren carriles negativos
    // tambien, hay que suavizar mas la chicana o bajarles la velocidad.
    //
    // El tinte de color (sf::Color) MULTIPLICA los colores de la imagen;
    // sobre un sprite oscuro como Coche1/Coche2 apenas se nota. Si mas
    // adelante quieren autos de colores bien distintos, conviene que la
    // imagen base sea blanca o gris clara.
    const std::vector<RacerConfig> racers =
    {
        { "Auto 1", "assets/sprites/Coche1.png", sf::Color::White,          205.f, 605.f,  5.f },
        { "Auto 2", "assets/sprites/Coche2.png", sf::Color(210, 225, 255),  215.f, 635.f, 18.f },
        { "Auto 3", "assets/sprites/Coche1.png", sf::Color(255, 225, 200),  225.f, 665.f, 31.f },
        { "Auto 4", "assets/sprites/Coche2.png", sf::Color(210, 255, 220),  235.f, 695.f, 44.f }
    };

    // Le damos a la camara un objetivo valido desde el primer frame; el
    // RaceLeaderboardSystem lo va a ir actualizando solo al que vaya
    // primero en cada vuelta.
    ECS::EntityID firstRacer = ECS::NULL_ENTITY;

    for (std::size_t i = 0; i < racers.size(); ++i)
    {
        const RacerConfig& config = racers[i];

        const ECS::EntityID racer =
            registry.CreateEntity();

        if (i == 0)
        {
            firstRacer = racer;
        }

        registry.AddComponent<ECS::InspectorName>(
            racer,
            config.name
        );

        // Todos arrancan juntos en la linea de salida (como en una
        // carrera de verdad) y se acomodan en su carril apenas arranca
        // la simulacion.
        registry.AddComponent<ECS::Transform>(
            racer,
            track.points.front()
        );

        auto& render =
            registry.AddComponent<ECS::Render>(
                racer,
                ECS::Render::MakeSprite(
                    config.texturePath,
                    { 60.f, 22.f }
                )
            );

        render.fillColor = config.tint;

        registry.AddComponent<ECS::Movement>(
            racer,
            config.maxSpeed,
            config.maxForce
        );

        auto& steering =
            registry.AddComponent<ECS::SteeringBehavior>(
                racer,
                ECS::SteeringType::PathFollow,
                trackEntity
            );

        // Que tan lejos "mira" el auto sobre la pista. Mas chico = pega
        // mas a su carril; mas grande = curvas mas suaves pero cortando
        // un poco mas las esquinas.
        steering.pathPredictDistance = 55.f;
        steering.laneOffset = config.laneOffset;
    }

    // --- Entidad "Camara": arranca en el primer corredor; despues la
    // controla el RaceLeaderboardSystem (sigue siempre al que va primero) ---
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

    camera.followTarget = firstRacer;
    camera.followSpeed = 4.f;
    camera.zoom = 0.65f; // Alejada para que se note el trazado completo.

    sf::Clock frameClock;

    // --- Bucle principal ---
    while (window.isOpen())
    {
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

        window.clear(sf::Color::Black);

        registry.UpdateSystems(deltaTime);

        window.display();
    }

    registry.RemoveAllSystems();

    return 0;

}