#pragma once
#include "ECS/Components/Movement.h"
#include "ECS/Components/Path.h"
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

                        sf::Vector2f desiredVelocity{ 0.f, 0.f };
                        float distanceToTarget = 0.f;
                        bool hasDesiredVelocity = true;

                        if (steering.type == SteeringType::PathFollow) {
                            // El "objetivo" de PathFollow no es una entidad con
                            // Transform, sino la pista (componente Path).
                            const Path* path =
                                registry.TryGetComponent<Path>(steering.targetEntity);

                            if (!path || path->points.size() < 2) {
                                return;
                            }

                            desiredVelocity = CalculatePathFollowVelocity(
                                transform,
                                movement,
                                *path,
                                steering
                            );
                        }
                        else {
                            // Seek / Flee / Arrive / Follow si necesitan un
                            // Transform valido en la entidad objetivo.
                            const Transform* targetTransform =
                                registry.TryGetComponent<Transform>(steering.targetEntity);

                            if (!targetTransform) {
                                return;
                            }

                            const sf::Vector2f toTarget =
                                targetTransform->position - transform.position;
                            const float distance = Length(toTarget);

                            switch (steering.type) {
                            case SteeringType::Seek:
                                desiredVelocity =
                                    NormalizeOrZero(toTarget) * movement.maxSpeed;
                                distanceToTarget = distance;
                                break;

                            case SteeringType::Flee:
                                desiredVelocity =
                                    NormalizeOrZero(-toTarget) * movement.maxSpeed;
                                distanceToTarget = distance;
                                break;

                            case SteeringType::Arrive:
                                desiredVelocity = CalculateArriveVelocity(
                                    toTarget,
                                    distance,
                                    movement.maxSpeed,
                                    steering.slowingRadius,
                                    steering.stopRadius
                                );
                                distanceToTarget = distance;
                                break;

                            case SteeringType::Follow: {
                                // Punto "de rezago": un poco detras del lider,
                                // medido segun hacia donde el lider se mueve (o
                                // hacia donde mira, si esta quieto). Asi el
                                // convoy no se amontona arriba del lider.
                                const Movement* targetMovement =
                                    registry.TryGetComponent<Movement>(steering.targetEntity);

                                sf::Vector2f leaderHeading =
                                    targetMovement
                                    ? NormalizeOrZero(targetMovement->velocity)
                                    : sf::Vector2f{ 0.f, 0.f };

                                if (LengthSquared(leaderHeading) < 0.0001f) {
                                    constexpr float radiansToDegrees = 0.0174532925f;
                                    const float radians =
                                        targetTransform->rotation * radiansToDegrees;
                                    leaderHeading = { std::cos(radians), std::sin(radians) };
                                }

                                const sf::Vector2f trailPoint =
                                    targetTransform->position -
                                    leaderHeading * std::max(steering.followDistance, 0.f);

                                const sf::Vector2f toTrail =
                                    trailPoint - transform.position;
                                const float trailDistance = Length(toTrail);

                                desiredVelocity = CalculateArriveVelocity(
                                    toTrail,
                                    trailDistance,
                                    movement.maxSpeed,
                                    steering.slowingRadius,
                                    steering.stopRadius
                                );
                                distanceToTarget = trailDistance;
                                break;
                            }

                            default:
                                hasDesiredVelocity = false;
                                break;
                            }
                        }

                        if (!hasDesiredVelocity) {
                            return;
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

                        // Arrive/Follow quedan quietos dentro del radio de parada
                        // (PathFollow nunca "llega": siempre sigue avanzando).
                        if ((steering.type == SteeringType::Arrive ||
                            steering.type == SteeringType::Follow) &&
                            distanceToTarget <= std::max(steering.stopRadius, 0.f) &&
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

        [[nodiscard]] static float Dot(
            const sf::Vector2f& a,
            const sf::Vector2f& b) noexcept {
            return a.x * b.x + a.y * b.y;
        }

        /**
         * @brief Calcula la velocidad deseada para el comportamiento PathFollow.
         *
         * @details Implementa la idea de Reynolds / Nature of Code cap. 5
         * (https://natureofcode.com/autonomous-agents/#path-following):
         * 1) Predecir la posicion futura del vehiculo segun su velocidad actual.
         * 2) Buscar, entre los segmentos de la pista, el punto normal (la
         *    proyeccion escalar clampeada al segmento) mas cercano a esa
         *    posicion futura.
         * 3) Tomar como objetivo un punto un poco mas adelante de ese normal,
         *    siguiendo la direccion del segmento.
         *
         * A diferencia del pseudocodigo original del libro -que solo corrige
         * la trayectoria cuando el vehiculo se sale del radio de la pista y,
         * fuera de eso, no aplica ninguna fuerza- aqui SIEMPRE se persigue
         * (seek) el punto-objetivo. Esto evita que el vehiculo se quede quieto
         * si arranca con velocidad cero y da como resultado un movimiento
         * continuo, como un auto recorriendo un circuito sin parar.
         *
         * Tambien se recuerda el ultimo segmento encontrado (steering.pathSegmentHint)
         * y solo se buscan unos pocos segmentos hacia adelante desde ahi. Esto
         * evita que, en pistas con tramos paralelos muy cercanos entre si (como
         * una pista con chicanas), el vehiculo "salte" por error a un tramo
         * distinto del circuito que pasa cerca en el espacio pero que en
         * realidad esta lejos en el recorrido.
         */
        [[nodiscard]] static sf::Vector2f CalculatePathFollowVelocity(
            const Transform& transform,
            const Movement& movement,
            const Path& path,
            SteeringBehavior& steering) noexcept {
            const std::size_t pointCount = path.points.size();
            const int segmentCount = static_cast<int>(
                path.closed ? pointCount : pointCount - 1
                );

            if (segmentCount <= 0) {
                return { 0.f, 0.f };
            }

            // 1) Prediccion: hacia donde va a estar el vehiculo en un instante.
            sf::Vector2f heading = NormalizeOrZero(movement.velocity);
            if (LengthSquared(heading) < 0.0001f) {
                // Sin velocidad todavia (p.ej. el primer frame): usamos hacia
                // donde esta orientado el Transform como direccion de respaldo.
                constexpr float degreesToRadians = 0.0174532925f;
                const float radians = transform.rotation * degreesToRadians;
                heading = { std::cos(radians), std::sin(radians) };
            }

            const sf::Vector2f futurePosition =
                transform.position + heading * steering.pathPredictDistance;

            // 2) Ventana de busqueda: si ya tenemos un segmento recordado,
            // solo miramos unos pocos tramos hacia adelante; si no (primer
            // frame, o el vehiculo se perdio), miramos toda la pista.
            const bool hasHint =
                steering.pathSegmentHint >= 0 &&
                steering.pathSegmentHint < segmentCount;

            const int searchStart = hasHint ? steering.pathSegmentHint : 0;
            const int searchCount = hasHint
                ? std::min(segmentCount, 4)
                : segmentCount;

            float bestDistanceSquared = std::numeric_limits<float>::max();
            sf::Vector2f bestTarget = futurePosition;
            sf::Vector2f bestPerpendicular{ 0.f, 0.f };
            int bestIndex = hasHint ? steering.pathSegmentHint : 0;
            float bestT = 0.f;

            for (int offset = 0; offset < searchCount; ++offset) {
                const int i = (searchStart + offset) % segmentCount;
                const sf::Vector2f a = path.points[i];
                const sf::Vector2f b = path.points[(i + 1) % pointCount];

                const sf::Vector2f segment = b - a;
                const float segmentLengthSquared = LengthSquared(segment);

                float t = segmentLengthSquared > 0.0001f
                    ? Dot(futurePosition - a, segment) / segmentLengthSquared
                    : 0.f;
                t = std::clamp(t, 0.f, 1.f);

                const sf::Vector2f normalPoint = a + segment * t;
                const float distanceSquared =
                    LengthSquared(futurePosition - normalPoint);

                if (distanceSquared < bestDistanceSquared) {
                    bestDistanceSquared = distanceSquared;
                    bestIndex = i;
                    bestT = t;

                    const sf::Vector2f segmentDirection = NormalizeOrZero(segment);
                    bestTarget = normalPoint +
                        segmentDirection * steering.pathPredictDistance;

                    // Perpendicular al segmento: sirve para desplazar el
                    // objetivo hacia un "carril" propio (laneOffset), en vez
                    // de que todos los vehiculos persigan la misma linea
                    // central y terminen encimados unos con otros.
                    bestPerpendicular = { -segmentDirection.y, segmentDirection.x };
                }
            }

            // Si dimos toda la vuelta (pasamos del ultimo segmento al primero
            // otra vez) sumamos una vuelta. Se compara contra el segmento
            // recordado del frame anterior, antes de pisarlo con el nuevo.
            if (hasHint &&
                steering.pathSegmentHint > segmentCount * 7 / 10 &&
                bestIndex < segmentCount * 3 / 10) {
                steering.pathLapCount++;
            }

            // Recordamos el segmento y el progreso para el proximo frame (y
            // para que el leaderboard sepa quien va mas adelante).
            steering.pathSegmentHint = bestIndex;
            steering.pathProgressT = bestT;

            const sf::Vector2f laneTarget =
                bestTarget + bestPerpendicular * steering.laneOffset;

            return NormalizeOrZero(laneTarget - transform.position) *
                std::max(movement.maxSpeed, 0.f);
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