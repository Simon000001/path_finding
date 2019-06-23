#include <SFML\Graphics.hpp>
#include <cmath>
int main()
{
	sf::RenderWindow window(sf::VideoMode(1000, 800), "Path Finder", sf::Style::Close | sf::Style::Titlebar);
	sf::RectangleShape car(sf::Vector2f(35.0f, 70.0f));
	car.setFillColor(sf::Color::Green);
	car.setRotation(90.f);
	car.setPosition(250.f, 250.f);
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
			//case sf::Event::
			}
		}
		float rotation = car.getRotation();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		{
			car.rotate(-1.f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		{
			car.rotate(1.f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		{
			car.move(0.0f,-1.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		{
			car.move(0.0f,1.0f);
		}
	window.clear();
	window.draw(car);
	window.display();
	}

	return 0;
}