#pragma once

#include "Prerequisitesr.h"

namespace ECS {

    /**
     * @struct Render
     * @brief Componente que asocia una entidad con una forma dibujable de SFML y,
     * opcionalmente, una textura.
     *
     * @details Envuelve un sf::Shape (circulo, rectangulo, triangulo, poligono o
     * linea) en un shared_ptr para que pueda compartirse o reasignarse con
     * facilidad, junto con una textura opcional y un color de relleno guardado
     * para referencia. El metodo estatico Make() es la forma recomendada de
     * construir un Render ya con su forma inicializada segun un ShapeType.
     */
    struct Render {
        std::shared_ptr<sf::Shape> shape;        ///< Forma dibujable asociada a la entidad (puede ser nullptr).
        std::shared_ptr<sf::Texture> texture;    ///< Textura aplicada a la forma (puede ser nullptr si no tiene).
        sf::Color fillColor{ sf::Color::White }; ///< Color de relleno guardado para referencia/edicion.
        bool visible{ true };                    ///< Indica si el sistema de render debe dibujar esta entidad.

        /**
         * @brief Construye un Render vacio, sin forma ni textura.
         */
        Render() = default;

        /**
         * @brief Construye un Render a partir de una forma ya creada.
         *
         * @param drawable Forma de SFML que se va a dibujar para esta entidad.
         * @param color Color de relleno inicial. Por defecto blanco.
         */
        explicit Render(std::shared_ptr<sf::Shape> drawable,
            sf::Color color = sf::Color::White) noexcept
            : shape(std::move(drawable)), fillColor(color) {
        }

        /**
         * @brief Carga una textura desde un archivo y la aplica a la forma actual.
         *
         * @param path Ruta del archivo de imagen a cargar.
         * @param resetRectangle Si es true, reinicia el rectangulo de textura de
         * la forma al tamaño completo de la nueva textura.
         * @return true si la forma existe y la textura se cargo y aplico
         * correctamente; false si no hay forma asignada o si la carga fallo.
         */
        bool SetTexture(const std::string& path, bool resetRectangle = true) {
            if (!shape) {
                return false;
            }

            auto newTexture = std::make_shared<sf::Texture>();
            if (!newTexture->loadFromFile(path)) {
                return false;
            }

            texture = std::move(newTexture);
            shape->setTexture(texture.get(), resetRectangle);
            return true;
        }

        /**
         * @brief Aplica una textura ya cargada (o la quita, si se pasa nullptr) a
         * la forma actual.
         *
         * @param newTexture Textura a aplicar; puede ser nullptr para quitar la
         * textura actual.
         * @param resetRectangle Si es true, reinicia el rectangulo de textura de
         * la forma al tamaño completo de la nueva textura.
         */
        void SetTexture(std::shared_ptr<sf::Texture> newTexture,
            bool resetRectangle = true) {
            if (!shape) {
                return;
            }

            texture = std::move(newTexture);
            shape->setTexture(texture ? texture.get() : nullptr, resetRectangle);
        }

        /**
         * @brief Quita la textura actual de la forma, si tiene una asignada.
         */
        void ClearTexture() {
            if (shape) {
                shape->setTexture(nullptr);
            }
            texture.reset();
        }

        /**
         * @brief Crea un Render con una forma nueva segun el ShapeType indicado.
         *
         * @param type Tipo de forma a crear (CIRCLE, RECTANGLE, TRIANGLE, POLYGON,
         * LINE o EMPTY).
         * @param color Color de relleno de la forma creada. Por defecto blanco.
         * @param texturePath Ruta opcional de una textura a cargar y aplicar de
         * inmediato. Si esta vacia, no se carga ninguna textura.
         * @return Un Render con la forma, color y (si se indico) textura ya
         * configurados. Si `type` es EMPTY o no reconocido, el Render resultante
         * no tiene forma asignada.
         */
        [[nodiscard]] static Render Make(ShapeType type,
            sf::Color color = sf::Color::White,
            const std::string& texturePath = {}) {
            std::shared_ptr<sf::Shape> createdShape;

            switch (type) {
            case CIRCLE: {
                auto circle = std::make_shared<sf::CircleShape>(50.f);
                circle->setOrigin({ 50.f, 50.f });
                createdShape = circle;
                break;
            }

            case RECTANGLE: {
                const sf::Vector2f size{ 120.f, 70.f };
                auto rectangle = std::make_shared<sf::RectangleShape>(size);
                rectangle->setOrigin(size / 2.f);
                createdShape = rectangle;
                break;
            }

            case TRIANGLE: {
                auto triangle = std::make_shared<sf::ConvexShape>(3);
                triangle->setPoint(0, { 0.f, -60.f });
                triangle->setPoint(1, { 55.f, 45.f });
                triangle->setPoint(2, { -55.f, 45.f });
                createdShape = triangle;
                break;
            }

            case POLYGON: {
                auto polygon = std::make_shared<sf::ConvexShape>(5);
                polygon->setPoint(0, { 0.f, -60.f });
                polygon->setPoint(1, { 57.f, -19.f });
                polygon->setPoint(2, { 35.f, 49.f });
                polygon->setPoint(3, { -35.f, 49.f });
                polygon->setPoint(4, { -57.f, -19.f });
                createdShape = polygon;
                break;
            }

            case LINE: {
                const sf::Vector2f size{ 200.f, 4.f };
                auto line = std::make_shared<sf::RectangleShape>(size);
                line->setOrigin(size / 2.f);
                createdShape = line;
                break;
            }

            case EMPTY:
            default:
                break;
            }

            if (createdShape) {
                createdShape->setFillColor(color);
            }

            Render render{ createdShape, color };
            if (!texturePath.empty()) {
                render.SetTexture(texturePath);
            }
            return render;
        }

        /**
         * @brief Crea un "sprite": un rectangulo con una imagen aplicada,
         * del tamaño que quieras verlo en pantalla.
         *
         * @details Por dentro sigue siendo un sf::RectangleShape (este
         * proyecto no usa sf::Sprite), pero al mapear la textura sobre un
         * rectangulo del tamaño exacto que pediste, se ve identico a un
         * sprite comun. A diferencia de llamar a Make(RECTANGLE, ...,
         * texturePath) a mano, esto evita dos errores tipicos:
         * - El color queda en blanco (sin tenir), porque RenderSystem
         *   vuelve a aplicar `fillColor` cada frame y en SFML el color de
         *   relleno de una forma con textura la tiñe/multiplica.
         * - El rectangulo se crea ya del tamaño que pediste (o del tamaño
         *   real de la imagen si no indicas uno), en vez del {120, 70}
         *   fijo de Make(), asi la imagen no sale estirada.
         *
         * El origen queda centrado, como en las demas formas, para que
         * Transform::rotation (y el auto-orientado de SteeringSystem)
         * roten el sprite desde su centro. El "frente" de tu imagen debe
         * apuntar hacia la derecha (+X): asi coincide con el angulo 0
         * que usa el steering.
         *
         * @param texturePath Ruta de la imagen (png, jpg, bmp, etc).
         * @param displaySize Tamaño en pixeles con el que se dibuja el
         * sprite. Si se deja en {0, 0}, se usa el tamaño real de la
         * imagen (1 pixel de imagen = 1 pixel en pantalla).
         * @return Un Render con el sprite listo, o uno vacio (sin forma,
         * por lo que RenderSystem simplemente no dibuja nada) si la
         * imagen no se pudo cargar.
         */
        [[nodiscard]] static Render MakeSprite(
            const std::string& texturePath,
            sf::Vector2f displaySize = { 0.f, 0.f }) {
            auto loadedTexture = std::make_shared<sf::Texture>();
            if (!loadedTexture->loadFromFile(texturePath)) {
                // No se pudo cargar la imagen: en vez de no dibujar nada
                // (invisible, imposible de diagnosticar a simple vista),
                // devolvemos un rectangulo magenta bien feo -la
                // convencion clasica de "textura faltante"- del tamaño
                // pedido. Si ves esto en pantalla, la ruta esta mal o el
                // archivo no esta ahi; no es un crash ni un bug de logica.
                const sf::Vector2f fallbackSize =
                    (displaySize.x > 0.f && displaySize.y > 0.f)
                    ? displaySize
                    : sf::Vector2f{ 40.f, 40.f };

                auto fallbackShape =
                    std::make_shared<sf::RectangleShape>(fallbackSize);
                fallbackShape->setOrigin(fallbackSize / 2.f);

                return Render{ fallbackShape, sf::Color(255, 0, 255) };
            }

            sf::Vector2f size = displaySize;
            if (size.x <= 0.f || size.y <= 0.f) {
                const sf::Vector2u textureSize = loadedTexture->getSize();
                size = {
                    static_cast<float>(textureSize.x),
                    static_cast<float>(textureSize.y)
                };
            }

            auto rectangle = std::make_shared<sf::RectangleShape>(size);
            rectangle->setOrigin(size / 2.f);
            rectangle->setTexture(loadedTexture.get(), true);

            Render render{ rectangle, sf::Color::White };
            render.texture = std::move(loadedTexture);
            return render;
        }
    };

} // namespace ECS