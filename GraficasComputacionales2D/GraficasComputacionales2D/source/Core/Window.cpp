#include "Core/Window.h"

Window::Window(int width, int height, const std::string& title) {
	m_window = new sf::RenderWindow(sf::VideoMode({ static_cast<unsigned int>(width), 
		                                            static_cast<unsigned int>(height) }), 
		                                            title,
		                                            sf::Style::Default);

	if (m_window) {
		m_window->setFramerateLimit(60);
		MESSAGE("Window", "Window", "Window created succesfully");
	}
	else {
		ERROR("Window", "Window", "Failed to create window"); 
	}
}
Window::~Window() {
	SAFE_PTR_RELEASE(m_window);
}

 bool
 Window::isOpen() const {
	if (m_window) {
		return m_window && m_window->isOpen();
	}
	else {
		ERROR("Window", "isOpen", "Window is not initialized");
		return false;
	}
}

 void
 Window::clear(const sf::Color& color) {
	if (m_window) {
		m_window->clear(color);
	}
	else {
		ERROR("Window", "clear", "Window is null");
	}
 }
 void Window::draw(const sf::Drawable& drawable, const sf::RenderStates& states) {
	 if (m_window) {
		m_window->draw(drawable, states);
	}
	else {
		ERROR("Window", "draw", "Window is null");
	}
 }

 void 
 Window::display() {
	 if (m_window) {
		m_window->display();
	}
	else {
		ERROR("Window", "display", "Window is null");
	}
 }
 void 
 Window::close() {
	 	 if (m_window) {
		m_window->close();
	}
	else {
		ERROR("Window", "close", "Window is null");
		 }
 }

 void 
 Window::update() {
	 deltaTime = clock.restart();
 }

 void 
 Window::render() {

 }

 void Window::destroy() {
	 SAFE_PTR_RELEASE(m_window);
 }