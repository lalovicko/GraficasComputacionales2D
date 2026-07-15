#include "Core/Window.h"

Window::Window(int width, int height, const std::string& title) {
    m_window = std::make_unique<sf::RenderWindow>(
        sf::VideoMode({ static_cast<unsigned int>(width),
                        static_cast<unsigned int>(height) }),
        title,
        sf::Style::Default
    );

    if (!m_window) {
        ERROR("Window", "Window", "Failed to create window");
    }

    m_window->setFramerateLimit(60);
    handleResize(m_window->getSize());
    MESSAGE("Window", "Window", "Window created successfully");
}

bool Window::isOpen() const {
    if (!m_window) {
        return false;
    }
    return m_window->isOpen();
}

void Window::clear(const sf::Color& color) {
    if (!m_window) {
        ERROR("Window", "clear", "Window is null");
    }
    m_window->clear(color);
}

void Window::draw(const sf::Drawable& drawable, const sf::RenderStates& states) {
    if (!m_window) {
        ERROR("Window", "draw", "Window is null");
    }
    m_window->draw(drawable, states);
}

void Window::display() {
    if (!m_window) {
        ERROR("Window", "display", "Window is null");
    }
    m_window->display();
}

void Window::close() {
    if (m_window) {
        m_window->close();
    }
}

void Window::handleResize(const sf::Vector2u& size) {
    if (!m_window || size.x == 0 || size.y == 0) {
        return;
    }

    m_baseViewSize = {
        static_cast<float>(size.x),
        static_cast<float>(size.y)
    };

    m_view.setSize(m_baseViewSize);
    m_view.setCenter({ 0.f, 0.f });
    m_window->setView(m_view);
}

void Window::applyCameraView(const sf::Vector2f& center,
                             float zoom,
                             float rotationDegrees) {
    if (!m_window) {
        return;
    }

    if (zoom <= 0.f) {
        zoom = 1.f;
    }

    m_view.setSize(m_baseViewSize / zoom);
    m_view.setCenter(center);
    m_view.setRotation(sf::degrees(rotationDegrees));
    m_window->setView(m_view);
}

void Window::update() {
    m_deltaTime = m_clock.restart();
}

void Window::render() {
}

void Window::destroy() {
    m_window.reset();
}
