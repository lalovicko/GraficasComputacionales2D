#pragma once
#include "Prerequisitesr.h"

namespace ECS {
    using EntityIndex = uint32_t;
    using EntityVersion = uint32_t;
    using EntityID = uint64_t;
    using ComponentTypeID = uint32_t;
    inline constexpr EntityID NULL_ENTITY = std::numeric_limits<EntityID>::max();

    [[nodiscard]] inline EntityIndex GetEntityIndex(EntityID id) noexcept {
        return static_cast<EntityIndex>(id & 0xFFF'FFFFFull);
    }
    [[nodiscard]] inline EntityVersion GetEntityVersion(EntityID id) noexcept {
        return static_cast<EntityVersion>((id >> 32) & 0xFFF'FFFFFull);
    }
    [[nodiscard]] inline EntityID MakeEntityID(EntityIndex index, EntityVersion version) noexcept {
        return (static_cast<EntityID>(version) << 32) | static_cast<EntityID>(index);
    }
    [[nodiscard]] inline ComponentTypeID NextComponentTypeID() noexcept {
        static ComponentTypeID counter = 0;
        return counter++;
    }
    template<typename T>
    [[nodiscard]] ComponentTypeID GetComponentTypeID() noexcept {
        static const ComponentTypeID ID = NextComponentTypeID();
        return ID;
    }

}