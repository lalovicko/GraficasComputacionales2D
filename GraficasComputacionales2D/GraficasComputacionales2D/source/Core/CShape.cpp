#include"core/CShape.h"
#include"core/Window.h"

CShape::CShape(ShapeType shapeType) : m_ShapeType(shapeType) {
	m_shape = createShape(shapeType);
}
void CShape::draw(Window& window) {
	if (m_shape) {
		window.draw(*m_shape);
	}
}
sf::Shape* CShape::getShape() {
	return m_shape.get();
}

std::unique_ptr<sf::Shape>
CShape::createShape(ShapeType shapeType) {
	switch (shapeType) {
	case EMPTY:
		return nullptr;
	case CIRCLE: {
		auto circle = std::make_unique<sf::CircleShape>(50.f);
		circle->setFillColor(sf::Color::White);
		circle->setPosition({ 100.f, 100.f });
		return circle;
	}
	case RECTANGLE: {
		auto rectangle = std::make_unique<sf::RectangleShape>(sf::Vector2f(120.f, 60.f));
		rectangle->setFillColor(sf::Color::Cyan);
		rectangle->setPosition({ 200.f, 200.f });
		return rectangle;
	}
	case TRIANGLE: {
		auto triangle = std::make_unique<sf::ConvexShape>(3);
		triangle->setPoint(0, { 0.f, 0.f });
		triangle->setPoint(1, { 100.f, 0.f });
		triangle->setPoint(2, { 50.f, 100.f });
		triangle->setFillColor(sf::Color::White);
		triangle->setPosition({ 300.f, 300.f });
		return triangle;
	}
	case POLYGON: {
		auto polygon = std::make_unique<sf::ConvexShape>(5);
		polygon->setPoint(0, { 0.f, 0.f });
		polygon->setPoint(1, { 100.f, 0.f });
		polygon->setPoint(2, { 120.f, 50.f });
		polygon->setPoint(3, { 60.f, 100.f });
		polygon->setPoint(4, { -20.f, 50.f });
		polygon->setFillColor(sf::Color::White);
		polygon->setPosition({ 500.f, 500.f });
		return polygon;
	}
	case LINE: {
		auto square = std::make_unique<sf::RectangleShape>(sf::Vector2f(200.f, 4.f));
		square->setFillColor(sf::Color::Red);
		square->setPosition({ 500.f, 100.f });
		return square;
	}
	default:
		return nullptr;
	}
}