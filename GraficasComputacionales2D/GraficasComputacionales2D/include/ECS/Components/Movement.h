#pragma once

#include "Prerequisitesr.h"

namespace ECS {

// Datos de movimiento que pueden reutilizar distintos comportamientos.
struct Movement {
    sf::Vector2f velocity{ 0.f, 0.f };
    sf::Vector2f acceleration{ 0.f, 0.f };

    float maxSpeed{ 180.f };
    float maxForce{ 350.f };
    float mass{ 1.f };

    // Hace que figuras como el triangulo miren hacia donde avanzan.
    bool orientToVelocity{ true };

    Movement() = default;

    Movement(float initialMaxSpeed,
             float initialMaxForce,
             float initialMass = 1.f) noexcept
        : maxSpeed(initialMaxSpeed),
          maxForce(initialMaxForce),
          mass(initialMass) {
    }
};

} // namespace ECS
