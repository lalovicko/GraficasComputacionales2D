#pragma once
#include "Prerequisitesr.h"

class Window;

class CShape {
public:

    CShape() = default;

    explicit CShape(ShapeType shapeType); 

    ~CShape() = default;
    
    void 
    draw(Window& window);
   
    sf::Shape* 
    getShape();

private:

    std::unique_ptr<sf::Shape> 
    createShape(ShapeType shapeType);

private:
    std::unique_ptr<sf::Shape> m_shape = nullptr;
    ShapeType m_ShapeType;
};