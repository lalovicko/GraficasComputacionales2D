#pragma once

#include "Prerequisitesr.h"

namespace ECS {

using EntityIndex = std::uint32_t;
using EntityVersion = std::uint32_t;
using EntityID = std::uint64_t;
using ComponentTypeID = std::uint32_t;

inline constexpr EntityID NULL_ENTITY = std::numeric_limits<EntityID>::max();

[[nodiscard]] inline EntityIndex GetEntityIndex(EntityID id) noexcept {
    return static_cast<EntityIndex>(id & 0xFFFF'FFFFull);
}

[[nodiscard]] inline EntityVersion GetEntityVersion(EntityID id) noexcept {
    return static_cast<EntityVersion>((id >> 32u) & 0xFFFF'FFFFull);
}

[[nodiscard]] inline EntityID MakeEntityID(EntityIndex index,
                                            EntityVersion version) noexcept {
    return (static_cast<EntityID>(version) << 32u) |
           static_cast<EntityID>(index);
}

[[nodiscard]] inline ComponentTypeID NextComponentTypeID() noexcept {
    static ComponentTypeID next = 0;
    return next++;
}

template<typename T>
[[nodiscard]] ComponentTypeID GetComponentTypeID() noexcept {
    static const ComponentTypeID id = NextComponentTypeID();
    return id;
}

} // namespace ECS
