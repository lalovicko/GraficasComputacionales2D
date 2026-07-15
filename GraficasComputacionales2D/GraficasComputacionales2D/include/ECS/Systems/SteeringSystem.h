#pragma once
#include "ECS/Components/Movement.h"
#include "ECS/Components/SteeringBehavior.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/System.h"

namespace ECS {

class SteeringSystem final : public System {
public:
    void OnUpdate(Registry& registry, float deltaTime) override {
        // Evita saltos enormes si la ventana se pausa o se arrastra.
        const float dt = std::clamp(deltaTime, 0.f, 0.05f);
        if (dt <= 0.f) {
            return;
        }

        registry.GetView<Transform, Movement, SteeringBehavior>().Each(
            [&registry, dt](EntityID,
                            Transform& transform,
                            Movement& movement,
                            SteeringBehavior& steering) {
                movement.acceleration = { 0.f, 0.f };

                if (!steering.enabled ||
                    steering.targetEntity == NULL_ENTITY ||
                    !registry.IsAlive(steering.targetEntity)) {
                    return;
                }

                const Transform* targetTransform =
                    registry.TryGetComponent<Transform>(steering.targetEntity);

                if (!targetTransform) {
                    return;
                }

                sf::Vector2f desiredVelocity{ 0.f, 0.f };
                const sf::Vector2f toTarget =
                    targetTransform->position - transform.position;
                const float distance = Length(toTarget);

                switch (steering.type) {
                case SteeringType::Seek:
                    desiredVelocity =
                        NormalizeOrZero(toTarget) * movement.maxSpeed;
                    break;

                case SteeringType::Flee:
                    desiredVelocity =
                        NormalizeOrZero(-toTarget) * movement.maxSpeed;
                    break;

                case SteeringType::Arrive:
                    desiredVelocity = CalculateArriveVelocity(
                        toTarget,
                        distance,
                        movement.maxSpeed,
                        steering.slowingRadius,
                        steering.stopRadius
                    );
                    break;
                }

                // Fuerza de direccion = velocidad deseada - velocidad actual.
                sf::Vector2f steeringForce =
                    desiredVelocity - movement.velocity;
                steeringForce = Limit(steeringForce, movement.maxForce);

                const float safeMass = std::max(movement.mass, 0.001f);
                movement.acceleration = steeringForce / safeMass;
                movement.velocity += movement.acceleration * dt;
                movement.velocity = Limit(
                    movement.velocity,
                    std::max(movement.maxSpeed, 0.f)
                );

                // Arrive queda completamente quieto dentro del radio de parada.
                if (steering.type == SteeringType::Arrive &&
                    distance <= std::max(steering.stopRadius, 0.f) &&
                    Length(movement.velocity) < 3.f) {
                    movement.velocity = { 0.f, 0.f };
                    movement.acceleration = { 0.f, 0.f };
                }

                transform.position += movement.velocity * dt;

                if (movement.orientToVelocity &&
                    LengthSquared(movement.velocity) > 1.f) {
                    constexpr float radiansToDegrees = 57.2957795f;
                    transform.rotation = std::atan2(
                        movement.velocity.y,
                        movement.velocity.x
                    ) * radiansToDegrees;
                }
            }
        );
    }

private:
    [[nodiscard]] static float LengthSquared(
        const sf::Vector2f& vector) noexcept {
        return vector.x * vector.x + vector.y * vector.y;
    }

    [[nodiscard]] static float Length(
        const sf::Vector2f& vector) noexcept {
        return std::sqrt(LengthSquared(vector));
    }

    [[nodiscard]] static sf::Vector2f NormalizeOrZero(
        const sf::Vector2f& vector) noexcept {
        const float length = Length(vector);
        if (length <= 0.0001f) {
            return { 0.f, 0.f };
        }
        return vector / length;
    }

    [[nodiscard]] static sf::Vector2f Limit(
        const sf::Vector2f& vector,
        float maximum) noexcept {
        maximum = std::max(maximum, 0.f);
        const float lengthSquared = LengthSquared(vector);
        const float maximumSquared = maximum * maximum;

        if (lengthSquared <= maximumSquared || lengthSquared <= 0.0001f) {
            return vector;
        }

        return NormalizeOrZero(vector) * maximum;
    }

    [[nodiscard]] static sf::Vector2f CalculateArriveVelocity(
        const sf::Vector2f& toTarget,
        float distance,
        float maxSpeed,
        float slowingRadius,
        float stopRadius) noexcept {
        stopRadius = std::max(stopRadius, 0.f);
        slowingRadius = std::max(slowingRadius, stopRadius + 0.001f);

        if (distance <= stopRadius) {
            return { 0.f, 0.f };
        }

        const float speedFactor = std::clamp(
            distance / slowingRadius,
            0.f,
            1.f
        );

        return NormalizeOrZero(toTarget) *
               (std::max(maxSpeed, 0.f) * speedFactor);
    }
};

} // namespace ECS
