#pragma once

#include "ECS/Types.h"

namespace ECS {

enum class SteeringType : int {
    Seek = 0,
    Flee = 1,
    Arrive = 2
};

// Configuracion del comportamiento autonomo de una entidad.
struct SteeringBehavior {
    SteeringType type{ SteeringType::Seek };
    EntityID targetEntity{ NULL_ENTITY };
    bool enabled{ true };

    // Solo se usan directamente en Arrive.
    float slowingRadius{ 180.f };
    float stopRadius{ 12.f };

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
