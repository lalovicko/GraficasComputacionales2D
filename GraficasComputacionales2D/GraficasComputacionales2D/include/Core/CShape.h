#pragma once

#include "Prerequisitesr.h"

class Window;

class CShape {
public:
    CShape() = default;
    explicit CShape(ShapeType shapeType);
    ~CShape() = default;

    void draw(Window& window);
    [[nodiscard]] sf::Shape* getShape() noexcept;
    [[nodiscard]] const sf::Shape* getShape() const noexcept;

private:
    [[nodiscard]] std::unique_ptr<sf::Shape> createShape(ShapeType shapeType);

private:
    std::unique_ptr<sf::Shape> m_shape;
    ShapeType m_shapeType{ EMPTY };
};
