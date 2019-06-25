#include <SFML\Graphics.hpp>
#include <cmath>
int main()
{
	enum {
		LEFT = 0, RIGHT, UP_DOWN
	};
	int state = 2;
	float x = 350, y = 350;
	float speed=0, angle=0;
	float maxSpeed = 4.0;
	float acc = 0.3f, deacc = 0.3f;
	float turnSpeed = 0.07f;
	sf::RenderWindow window(sf::VideoMode(1800, 1000), "Path Finder", sf::Style::Close | sf::Style::Titlebar);
	sf::RectangleShape car(sf::Vector2f(86.0f, 137.5f));
	car.setOrigin(17.5f, 35.0f);
	sf::Texture carTexture;
	carTexture.loadFromFile("texture/car_sprite.png");
	car.setTexture(&carTexture);
	sf::Vector2u texturesize = carTexture.getSize();
	texturesize.x /= 1;
	texturesize.y /= 3;

	car.setTextureRect(sf::IntRect(texturesize.x * 0, texturesize.y * 0, texturesize.x, texturesize.y));

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
		if (right && speed != 0)
		{
			angle += turnSpeed * speed / maxSpeed;
			state = RIGHT;
		}
		else if (left && speed != 0)
		{
			angle -= turnSpeed * speed / maxSpeed;
			state = LEFT;
		}
		else
			state = UP_DOWN;
		

		x += sin(angle) * speed;
		y -= cos(angle) * speed;

		//mouse input
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			sf::Vector2i mousepos = sf:: Mouse::getPosition(window);
			x = (float)mousepos.x;
			y = (float)mousepos.y;
			angle = 0.0;
		}

		//draw car
	window.clear(sf::Color(150,150,150));
	car.setTextureRect(sf::IntRect(texturesize.x *0, texturesize.y * state, texturesize.x, texturesize.y));
	car.setPosition(x, y);
	car.setRotation(angle * 180 / (float)3.141592);
	window.draw(car);
	window.display();
	}

	return 0;
}