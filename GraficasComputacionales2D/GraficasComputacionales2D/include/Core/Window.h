#pragma once

#include "Prerequisitesr.h"

class Window {
public:
    Window() = default;
    Window(int width, int height, const std::string& title);
    ~Window() = default;

    [[nodiscard]] bool isOpen() const;
    void clear(const sf::Color& color = sf::Color(0, 0, 0, 255));
    void draw(const sf::Drawable& drawable,
              const sf::RenderStates& states = sf::RenderStates::Default);
    void display();
    void close();

    void handleResize(const sf::Vector2u& size);
    void applyCameraView(const sf::Vector2f& center, float zoom, float rotationDegrees);

    void update();
    void render();
    void destroy();

    [[nodiscard]] sf::Time getDeltaTime() const noexcept { return m_deltaTime; }

public:
    // Se deja publico para conservar compatibilidad con el codigo del curso.
    std::unique_ptr<sf::RenderWindow> m_window;

private:
    sf::View m_view;
    sf::Vector2f m_baseViewSize{};
    sf::Time m_deltaTime{};
    sf::Clock m_clock;
};
