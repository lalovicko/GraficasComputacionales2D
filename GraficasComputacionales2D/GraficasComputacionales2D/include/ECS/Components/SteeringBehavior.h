#pragma once

#include "ECS/Types.h"

namespace ECS {

    enum class SteeringType : int {
        Seek = 0,
        Flee = 1,
        Arrive = 2,

        // Recorre una entidad con componente Path (targetEntity = pista).
        // Basado en el algoritmo de "Path Following" de Craig Reynolds /
        // Nature of Code cap. 5: https://natureofcode.com/autonomous-agents/#path-following
        PathFollow = 3,

        // Persigue a otra entidad manteniendose un poco detras de ella
        // (targetEntity = entidad a seguir, p.ej. el lider de la caravana).
        Follow = 4
    };

    // Configuracion del comportamiento autonomo de una entidad.
    struct SteeringBehavior {
        SteeringType type{ SteeringType::Seek };

        // Seek/Flee/Arrive: entidad objetivo.
        // PathFollow: entidad que tiene el componente Path (la pista).
        // Follow: entidad a la que se le sigue el rastro (p.ej. el lider).
        EntityID targetEntity{ NULL_ENTITY };

        bool enabled{ true };

        // Se usan en Arrive y tambien en Follow (para frenar al alcanzar
        // el punto de seguimiento, en vez de chocar con el lider).
        float slowingRadius{ 180.f };
        float stopRadius{ 12.f };

        // --- Solo para PathFollow ---
        // Que tan lejos "mira" el vehiculo (posicion futura predicha y punto
        // objetivo un poco mas adelante sobre la pista). Un valor chico pega
        // mas a la pista; uno grande suaviza las curvas pero corta un poco
        // las esquinas, tal como explica Reynolds en el libro.
        float pathPredictDistance{ 55.f };

        // Desplazamiento lateral (perpendicular a la pista) para simular un
        // "carril" propio. En 0 va justo por el centro; valores positivos o
        // negativos lo corren hacia un lado u otro. Asi varios autos pueden
        // recorrer la pista al mismo tiempo sin ir todos pegados en la misma
        // linea, y uno mas rapido puede "rebasar" a uno mas lento con solo
        // tener velocidades o carriles distintos (sin logica de colisiones).
        float laneOffset{ 0.f };

        // Recuerda el ultimo segmento de la pista donde se encontro al
        // vehiculo, para no "teletransportarse" a otro tramo cercano en el
        // espacio pero lejano en el recorrido (curvas muy juntas). Uso interno.
        int pathSegmentHint{ -1 };

        // Posicion (0 a 1) dentro del segmento actual y vueltas completadas.
        // Junto con pathSegmentHint sirven para armar el leaderboard: se
        // ordena por (vueltas, segmento, posicion en el segmento). Uso interno,
        // los actualiza SteeringSystem solo.
        float pathProgressT{ 0.f };
        int pathLapCount{ 0 };

        // --- Solo para Follow ---
        // Distancia detras del lider (medida a lo largo de hacia donde
        // el lider se esta moviendo) que este agente intenta mantener.
        float followDistance{ 60.f };

        SteeringBehavior() = default;

        SteeringBehavior(SteeringType behaviorType,
            EntityID target,
            float initialSlowingRadius = 180.f,
            float initialStopRadius = 12.f) noexcept
            : type(behaviorType),
            targetEntity(target),
            slowingRadius(initialSlowingRadius),
            stopRadius(initialStopRadius) {
        }
    };

} // namespace ECS