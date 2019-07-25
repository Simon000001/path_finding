#include <SFML\Graphics.hpp>
#include <iostream>
#include <cmath>
#define maxrows 20
#define maxcolms 13
int main()
{
	enum {
		LEFT = 0, RIGHT, UP_DOWN
	};
	enum {
		obstacle = 0, start, stop
	};
	int grid[maxrows][maxcolms];
	int hexgrid[maxrows][maxcolms];
	int state = 2;
	int oi, oj;
	float ox = 0, oy = 0;
	static float sx, sy;
	float x = -100, y = -100;
	float speed=0, angle=0.0;
	float maxSpeed = 2.0;
	float acc = 0.05f, deacc = 0.08f;
	float turnSpeed = 0.02f;
	int  tiletexturesizex = 150;
	int tiletexturesizey = 172;

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(1400, 800), "Path Finder", sf::Style::Close | sf::Style::Titlebar);
	sf::RectangleShape car(sf::Vector2f(76.0f, 110.5f));
	sf::RectangleShape tile(sf::Vector2f(110.0f, 110.0f));
	car.setOrigin(43.0f, 69.0f);
	tile.setOrigin(35.0f, 35.0f);
	sf::Texture carTexture;
	sf::Texture tileTexture;
	carTexture.loadFromFile("texture/car_sprite.png");
	car.setTexture(&carTexture);
	tileTexture.loadFromFile("texture/tiles.png");
	tile.setTexture(&tileTexture);
	sf::Vector2u texturesize = carTexture.getSize();
	texturesize.x /= 1;
	texturesize.y /= 3;

	for (int i = 0; i < maxrows; i++)
		for (int j = 0; j < maxcolms; j++)
		{
			hexgrid[i][j] = 3;
		}

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
		//float rotation = car.getRotation();
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
		else if (right != 0)
		{
			state = RIGHT;
		}
		else if (left && speed != 0)
		{
			angle -= turnSpeed * speed / maxSpeed;
			state = LEFT;
		}
		else if (left != 0)
		{
			state = LEFT;
		}
		else
			state = UP_DOWN;
		

		x += sin(angle) * speed;
		y -= cos(angle) * speed;

		//mouse input
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			sf::Vector2i mousepos = sf:: Mouse::getPosition(window);
			x = (float)mousepos.x;
			y = (float)mousepos.y;
			angle = 0.0;
		}
		
		//check collision
		if (car.getGlobalBounds().intersects(tile.getGlobalBounds()))
		{
		
			x = sx;
			y = sy;
		}
		else 
			std::cout << "no collision" << std::endl;
		    std::cout << sx <<"  " <<sy<< std::endl;

		

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / 104;
			oj = (int)oy / 84;
			hexgrid[oi][oj] = 0;
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::X))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / 104;
			oj = (int)oy / 84;
			hexgrid[oi][oj] = 3;
		}
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (int)mousepos.x;
			oy = (int)mousepos.y;
			x = ox;
			y = oy;
			sx = ox;
			sy = oy;
			oi = (int)ox / 104;
			oj = (int)oy / 84;
			hexgrid[oi][oj] = 1;
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / 104;
			oj = (int)oy / 84;
			hexgrid[oi][oj] = 3;
		}
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / 104;
			oj = (int)oy / 84;
			hexgrid[oi][oj] = 2;
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / 104;
			oj = (int)oy / 84;
			hexgrid[oi][oj] = 3;
		}
			

		//render
	window.clear(sf::Color(150,150,150));
	car.setTextureRect(sf::IntRect(texturesize.x *0, texturesize.y * state, texturesize.x, texturesize.y));
	car.setPosition(x, y);
	car.setRotation(angle *180 / (float)3.1415);
		for (int i = 0; i <maxrows; i++)
			for (int j = 0; j < maxcolms; j++)
			{
				if (j % 2 == 0) {
					tile.setTextureRect(sf::IntRect(150 * hexgrid[i][j], 172 * 0, 150, 172));
					tile.setPosition(i * 104, j * 84);
				}
				if (j % 2 != 0) {
					tile.setTextureRect(sf::IntRect(150 * hexgrid[i][j], 172 * 0, 150, 172));
					tile.setPosition((i+0.5) * 104, j * 84);
				}

				window.draw(tile);
			}
	window.draw(car);
	window.display();
	}

	return 0;
}