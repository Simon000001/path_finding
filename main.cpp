#include <SFML\Graphics.hpp>
#include <cmath>
int main()
{
	float x = 350, y = 350;
	float speed=0, angle=0;
	float maxSpeed = 4.0;
	float acc = 0.3f, deacc = 0.3f;
	float turnSpeed = 0.04f;
	sf::RenderWindow window(sf::VideoMode(1000, 800), "Path Finder", sf::Style::Close | sf::Style::Titlebar);
	sf::RectangleShape car(sf::Vector2f(35.0f, 70.0f));
	car.setOrigin(17.5f, 35.0f);
	while (window.isOpen())
	{
		sf::Event evt;
		while (window.pollEvent(evt))
		{
			switch (evt.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			}
		}
		bool up = 0, down = 0, left = 0, right = 0;
		float rotation = car.getRotation();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))left = 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) right = 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) up = 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) down = 1;

		//movement
		if (up && speed < maxSpeed)
		{
			if (speed < 0) speed += deacc;
			else speed += acc;
		}
		if (down && speed >-maxSpeed)
		{
			if (speed > 0) speed -= deacc;
			else speed -= acc;
		}
		if (!up && !down)
		{
			if (speed - deacc > 0) speed -= deacc;
			else if (speed + deacc < 0) speed += deacc;
			else speed = 0;
		}
		if (right && speed != 0) angle += turnSpeed * speed / maxSpeed;
		if (left && speed != 0) angle -= turnSpeed * speed / maxSpeed;

		x += sin(angle) * speed;
		y -= cos(angle) * speed;
		//mouse input
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			sf::Vector2i mousepos = sf:: Mouse::getPosition(window);
			x = (float)mousepos.x;
			y = (float)mousepos.y;
		}
		//draw car
	window.clear();
	car.setPosition(x, y);
	car.setRotation(angle * 180 / 3.141592);
	car.setFillColor(sf::Color::Green);
	window.draw(car);
	window.display();
	}

	return 0;
}