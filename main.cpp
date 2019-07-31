#include <SFML\Graphics.hpp>
#include <iostream>
#include <cmath>
#include <queue>
#include <list>
#include <algorithm>

#define maxrows 8
#define maxcolms 14
int hexgrid[maxcolms][maxrows];


class point {
public:
	point(int a = 0, int b = 0) { x = a; y = b; }
	bool operator ==(const point& o) { return o.x == x && o.y == y; }
	point operator +(const point& o) { return point(o.x + x, o.y + y); }
	int x, y;
};

class map {
public:
	map() {
		char t[14][8];
		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 14; j++)
			{
				t[i][j] = 0;
			}
		
		w = 14;
		h = 8;
		for (int r = 0; r < h; r++)
			for (int s = 0; s < w; s++)
				m[s][r] = t[r][s];
	}
	void obstacleinput(int i, int j, int c) {
		if (c ==1) {
			m[i][j] = 0;
		}else m[i][j] = 1;

	}
	int operator() (int x, int y) { return m[x][y]; }
	char m[14][8];
	int w, h;
};

class node {
public:
	bool operator == (const node& o) { return pos == o.pos; }
	bool operator == (const point& o) { return pos == o; }
	bool operator < (const node& o) { return dist + cost < o.dist + o.cost; }
	point pos, parent;
	int dist, cost;
};

class aStar {
public:
	aStar() {
		neighbours[0] = point(-1, -1); neighbours[1] = point(1, -1);
		neighbours[2] = point(-1, 1); neighbours[3] = point(1, 1);
		neighbours[4] = point(0, -1); neighbours[5] = point(-1, 0);
		neighbours[6] = point(0, 1); neighbours[7] = point(1, 0);
	}
	


	int calcDist(point& p) {
		//  for a better heuristic
		int x = end.x - p.x, y = end.y - p.y;
		return(x * x + y * y);
	}

	bool isValid(point& p) {
		return (p.x > -1 && p.y > -1 && p.x < m.w && p.y < m.h);
	}

	bool existPoint(point& p, int cost) {
		std::list<node>::iterator i;
		i = std::find(closed.begin(), closed.end(), p);
		if (i != closed.end()) {
			if ((*i).cost + (*i).dist < cost) return true;
			else { closed.erase(i); return false; }
		}
		i = std::find(open.begin(), open.end(), p);
		if (i != open.end()) {
			if ((*i).cost + (*i).dist < cost) return true;
			else { open.erase(i); return false; }
		}
		return false;
	}

	bool fillOpen(node& n) {
		int stepCost, nc, dist;
		point neighbour;

		for (int x = 0; x < 8; x++) {
			// one can make diagonals have different cost
			stepCost = x < 4 ? 1.5 : 1;
			
				neighbour = n.pos + neighbours[x];
			
			if (neighbour == end) return true;

			if (isValid(neighbour) && m(neighbour.x, neighbour.y) != 1) {
				nc = stepCost + n.cost;
				dist = calcDist(neighbour);
				if (!existPoint(neighbour, nc + dist)) {
					node m;
					m.cost = nc; m.dist = dist;
					m.pos = neighbour;
					m.parent = n.pos;
					open.push_back(m);
				}
			}
		}
		return false;
	}

	bool search(point& s, point& e, map& mp) {
		node n; end = e; start = s; m = mp;
		n.cost = 0; n.pos = s; n.parent = 0; n.dist = calcDist(s);
		open.push_back(n);
		while (!open.empty()) {
			open.sort();
			node n = open.front();
			open.pop_front();
			closed.push_back(n);
			if (fillOpen(n)) return true;
		}
		return false;
	}

	int path(std::list<point>& path) {
		path.push_front(end);
		int cost = 1 + closed.back().cost;
		path.push_front(closed.back().pos);
		point parent = closed.back().parent;

		for (std::list<node>::reverse_iterator i = closed.rbegin(); i != closed.rend(); i++) {
			if ((*i).pos == parent && !((*i).pos == start)) {
				path.push_front((*i).pos);
				parent = (*i).parent;
			}
		}
		path.push_front(start);
		return cost;
	}

	map m; point end, start;
	point neighbours[8];
	std::list<node> open;
	std::list<node> closed;
};


int main(int argc, char* argv[])
{
	map m;
	
	enum {
		LEFT = 0, RIGHT, UP_DOWN
	};
	enum {
		obstacle = 0, start, stop,normal,path
	};
	int count = 0;
	int temp = 0;
	int state = 2;
	int oi, oj;
	int cx, cy;
	int bx, by;
	float ox = 0, oy = 0;
	static int sx, sy;//cordinate
	static int ex, ey;
	int startnode;
	int endnode;
	static int startx=0, starty=0;//rows,colms
	static int endx=14, endy=8;
	float x = -100, y = -100;
	float speed=0, angle=0.0;
	float maxSpeed = 2.0;
	float acc = 1.2f, deacc = 1.2f;
	float turnSpeed = 0.03f;
	int  tiletexturesizex = 150;
	int tiletexturesizey = 172;
	bool jump = 1;
	bool startex = 0;
	bool endex = 0;


	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(1400, 800), "Path Finder", sf::Style::Close | sf::Style::Titlebar);
	sf::RectangleShape car(sf::Vector2f(76.0f, 110.5f));
	sf::RectangleShape tile(sf::Vector2f(100.0f, 100.0f));
	car.setOrigin(43.0f, 69.0f);
	tile.setOrigin(0.0f, 0.0f);
	sf::Texture carTexture;
	sf::Texture tileTexture;
	carTexture.loadFromFile("texture/car_sprite.png");
	car.setTexture(&carTexture);
	tileTexture.loadFromFile("texture/tiles-2.png");
	tile.setTexture(&tileTexture);
	sf::Vector2u texturesize = carTexture.getSize();
	sf::Vector2u tiletexturesize = tileTexture.getSize();
	texturesize.x /= 1;
	tiletexturesize.x /= 5;
	texturesize.y /= 3;
	tiletexturesize.y /= 1;

	for (int i = 0; i < maxcolms; i++)
		for (int j = 0; j < maxrows; j++)
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

		

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::X))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / tiletexturesize.x;
			oj = (int)oy / tiletexturesize.y;
			hexgrid[oi][oj] = obstacle;
			m.obstacleinput(oi,oj,0);
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && sf::Keyboard::isKeyPressed(sf::Keyboard::X))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / tiletexturesize.x;
			oj = (int)oy / tiletexturesize.y;
			hexgrid[oi][oj] = normal;
			m.obstacleinput(oi,oj,1);
			
		}
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::S)&& startex==0)
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (int)mousepos.x;
			oy = (int)mousepos.y;
			x = ox;
			y = oy;
			sx = ox; 
			sy = oy;
			oi = (int)ox / tiletexturesize.x;
			oj = (int)oy / tiletexturesize.y;
			startx = oi;
			starty = oj;
			hexgrid[oi][oj] = start;
			startex = 1;
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / tiletexturesize.x;
			oj = (int)oy / tiletexturesize.y;
			hexgrid[oi][oj] = normal;
			startex = 0;
		}
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && sf::Keyboard::isKeyPressed(sf::Keyboard::E) && endex==0)
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / tiletexturesize.x;
			oj = (int)oy / tiletexturesize.y;
			endx = oi;
			endy = oj;
			hexgrid[oi][oj] = stop;
			endex = 1;
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			sf::Vector2i mousepos = sf::Mouse::getPosition(window);
			ox = (float)mousepos.x;
			oy = (float)mousepos.y;
			oi = (int)ox / tiletexturesize.x;
			oj = (int)oy / tiletexturesize.y;
			hexgrid[oi][oj] = normal;
			endex = 0;
		}
		//boundry
		bx = (unsigned int)x / tiletexturesize.x;
		by = (unsigned int)y / tiletexturesize.y;
		if (bx>maxcolms ||bx<0||by>maxrows||by<0)
		{
			x = sx;
			y = sy;
		}
		//check collision
		cx = (int)x / tiletexturesize.x;
		cy = (int)y / tiletexturesize.y;
		if (hexgrid[cx][cy] == obstacle)
		{
			x = sx;
			y = sy;
		}
		
			
		//a*algorithm
		
	
		
		
		point s(startx, starty), e(endx, endy);
		aStar as;
		
		//if (startex==1 && endex==1)
		if (as.search(s, e, m)) {
			std::list<point> path;
			int c = as.path(path);
			for (int y = 0; y < 8; y++) {
				for (int x = 0; x < 14; x++) {
					
					if (std::find(path.begin(), path.end(), point(x, y)) != path.end()) {
						
						if (x == startx && y == starty) hexgrid[x][y] = 1;
						else if (x == endx && y == endy) hexgrid[x][y] = 2;
						else hexgrid[x][y] = 4;

					
					}
				}
				
			}

		}
	  

		//render
		
	window.clear(sf::Color(150,150,150));
	car.setTextureRect(sf::IntRect(texturesize.x *0, texturesize.y * state, texturesize.x, texturesize.y));
	car.setPosition(x, y);
	car.setRotation(angle *180 / (float)3.1415);
		for (int i = 0; i <maxcolms; i++){
			for (int j = 0; j < maxrows; j++)
			{
					tile.setTextureRect(sf::IntRect(tiletexturesize.x* hexgrid[i][j],  0, tiletexturesize.x, tiletexturesize.y));
					tile.setPosition(i * tiletexturesize.x, j * tiletexturesize.y);
					window.draw(tile);
			}
		}
		for (int i = 0; i < maxcolms; i++) {
			for (int j = 0; j < maxrows; j++)
			{
				if (hexgrid[i][j] != 0)
					hexgrid[i][j] = 3;
			}
		}
	window.draw(car);
	window.display();
	}

	return 0;
}

