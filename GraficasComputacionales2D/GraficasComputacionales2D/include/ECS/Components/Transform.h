#pragma once

#include "Prerequisitesr.h"

namespace ECS {

    /**
     * @struct Transform
     * @brief Componente que almacena la posicion, rotacion y escala de una entidad en el mundo 2D.
     *
     * @details Es el componente espacial base del ECS: cualquier entidad que necesite
     * ubicarse, rotarse o escalarse en la escena lo utiliza. Sistemas como el de
     * renderizado, movimiento o steering behaviors lo leen y/o lo modifican en cada frame.
     */
    struct Transform {
        sf::Vector2f position{ 0.f, 0.f }; ///< Posicion de la entidad en coordenadas del mundo.
        float rotation{ 0.f };             ///< Rotacion de la entidad, en grados.
        sf::Vector2f scale{ 1.f, 1.f };    ///< Escala de la entidad en los ejes X e Y.

        /**
         * @brief Construye un Transform por defecto.
         *
         * @details Posicion en el origen (0, 0), sin rotacion y escala 1:1.
         */
        Transform() = default;

        /**
         * @brief Construye un Transform con valores iniciales especificos.
         *
         * @param initialPosition Posicion inicial de la entidad, en coordenadas del mundo.
         * @param initialRotation Rotacion inicial en grados. Por defecto 0.
         * @param initialScale Escala inicial en X e Y. Por defecto 1:1.
         */
        explicit Transform(sf::Vector2f initialPosition,
            float initialRotation = 0.f,
            sf::Vector2f initialScale = { 1.f, 1.f }) noexcept
            : position(initialPosition),
            rotation(initialRotation),
            scale(initialScale) {}

        /**
         * @brief Desplaza la posicion actual sumando un vector de delta.
         *
         * @param delta Vector de desplazamiento que se suma a la posicion actual.
         */
        void Translate(sf::Vector2f delta) noexcept {
            position += delta;
        }

        /**
         * @brief Incrementa la rotacion actual sumando un angulo en grados.
         *
         * @param degrees Cantidad de grados que se suma a la rotacion actual.
         */
        void Rotate(float degrees) noexcept {
            rotation += degrees;
        }
    };

} // namespace ECS