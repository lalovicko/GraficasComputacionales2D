#pragma once

#include "ECS/Types.h"

namespace ECS {

struct Camera {
    float zoom{ 1.f };
    bool active{ true };
    EntityID followTarget{ NULL_ENTITY };
    float followSpeed{ 5.f };
};

} // namespace ECS
