#pragma once

#include "Prerequisitesr.h"

namespace ECS
{
    struct InspectorName
    {
        std::string name = "Entity";
        bool visibleInInspector = true;

        InspectorName() = default;

        InspectorName(const std::string& entityName,
            bool visible = true)
            : name(entityName),
            visibleInInspector(visible)
        {}
    };
}