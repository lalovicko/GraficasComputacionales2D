#pragma once
#include "Prerequisitesr.h"

namespace ECS
{
    /**
     * @brief Almacena los puntos y el ancho de una pista.
     */
    struct Path
    {
        /// Puntos centrales que forman el circuito.
        std::vector<sf::Vector2f> points;

        /// Mitad del ancho permitido de la pista.
        float radius = 55.f;

        /// Indica si el último punto se conecta con el primero.
        bool closed = true;
    };
}