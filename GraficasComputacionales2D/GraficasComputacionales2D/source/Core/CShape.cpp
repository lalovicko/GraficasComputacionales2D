#include "Core/CShape.h"
#include "Core/Window.h"

CShape::CShape(ShapeType shapeType)
    : m_shape(createShape(shapeType)), m_shapeType(shapeType) {
}

void CShape::draw(Window& window) {
    if (m_shape) {
        window.draw(*m_shape);
    }
}

sf::Shape* CShape::getShape() noexcept {
    return m_shape.get();
}

const sf::Shape* CShape::getShape() const noexcept {
    return m_shape.get();
}

std::unique_ptr<sf::Shape> CShape::createShape(ShapeType shapeType) {
    switch (shapeType) {
    case EMPTY:
        return nullptr;

    case CIRCLE: {
        auto shape = std::make_unique<sf::CircleShape>(50.f);
        shape->setOrigin({ 50.f, 50.f });
        shape->setFillColor(sf::Color::White);
        return shape;
    }

    case RECTANGLE: {
        const sf::Vector2f size{ 120.f, 60.f };
        auto shape = std::make_unique<sf::RectangleShape>(size);
        shape->setOrigin(size / 2.f);
        shape->setFillColor(sf::Color::Cyan);
        return shape;
    }

    case TRIANGLE: {
        auto shape = std::make_unique<sf::ConvexShape>(3);
        shape->setPoint(0, { 0.f, 0.f });
        shape->setPoint(1, { 100.f, 0.f });
        shape->setPoint(2, { 50.f, 100.f });
        shape->setOrigin({ 50.f, 50.f });
        shape->setFillColor(sf::Color::White);
        return shape;
    }

    case POLYGON: {
        auto shape = std::make_unique<sf::ConvexShape>(5);
        shape->setPoint(0, { 0.f, -60.f });
        shape->setPoint(1, { 57.f, -19.f });
        shape->setPoint(2, { 35.f, 49.f });
        shape->setPoint(3, { -35.f, 49.f });
        shape->setPoint(4, { -57.f, -19.f });
        shape->setFillColor(sf::Color::White);
        return shape;
    }

    case LINE: {
        const sf::Vector2f size{ 200.f, 4.f };
        auto shape = std::make_unique<sf::RectangleShape>(size);
        shape->setOrigin(size / 2.f);
        shape->setFillColor(sf::Color::Red);
        return shape;
    }

    default:
        return nullptr;
    }
}
