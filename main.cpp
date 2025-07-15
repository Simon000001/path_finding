#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>

// --- Enums ---
enum Orientation {
    LEFT = 0,
    RIGHT,
    STRAIGHT
};

enum TileType {
    OBSTACLE = 0,
    START,
    STOP,
    EMPTY
};

// --- Check point inside hexagon ---
bool pointInHexagon(sf::Vector2f point, sf::Vector2f hexCenter, float radius) {
    const int numSides = 6;
    sf::Vector2f hexPoints[numSides];

    for (int i = 0; i < numSides; ++i) {
        float angle_deg = 60 * i - 30;
        float angle_rad = M_PI / 180.0f * angle_deg;
        hexPoints[i] = hexCenter + sf::Vector2f(radius * cos(angle_rad), radius * sin(angle_rad));
    }

    int crossings = 0;
    for (int i = 0; i < numSides; ++i) {
        sf::Vector2f p1 = hexPoints[i];
        sf::Vector2f p2 = hexPoints[(i + 1) % numSides];
        if (((p1.y > point.y) != (p2.y > point.y)) &&
            (point.x < (p2.x - p1.x) * (point.y - p1.y) / (p2.y - p1.y) + p1.x)) {
            crossings++;
        }
    }
    return (crossings % 2 == 1);
}

int main() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(1400, 800), "Path Finder", sf::Style::Close | sf::Style::Titlebar, settings);
    window.setFramerateLimit(60);

    constexpr int TILE_WIDTH = 104;
    constexpr int TILE_HEIGHT = 84;
    constexpr float PI = 3.14159265f;

    // --- Calculate rows and cols based on screen size ---
    int max_cols = window.getSize().x / TILE_WIDTH;
    int max_rows = window.getSize().y / (TILE_HEIGHT * 0.75f);

    // --- Grid ---
    std::vector<std::vector<int>> hexgrid(max_rows, std::vector<int>(max_cols, TileType::EMPTY));

    // --- Set START and STOP tiles within visible range ---
    if (max_rows > 0 && max_cols > 0) {
        hexgrid[0][0] = TileType::START;
        if (max_rows > 1 && max_cols > 1) {
            hexgrid[max_rows - 1][max_cols - 1] = TileType::STOP;
        } else {
            // If grid is too small, place STOP at a different position
            hexgrid[std::max(0, max_rows - 1)][std::max(0, max_cols - 1)] = TileType::STOP;
        }
    }

    // --- Load Textures ---
    sf::Texture carTexture, tileTexture;
    if (!carTexture.loadFromFile("texture/car_sprite.png")) {
        std::cerr << "Failed to load car_sprite.png\n";
        return 1;
    }
    if (!tileTexture.loadFromFile("texture/tiles.png")) {
        std::cerr << "Failed to load tiles.png\n";
        return 1;
    }

    // --- Car ---
    sf::RectangleShape car(sf::Vector2f(76.0f, 110.5f));
    car.setOrigin(38.0f, 55.25f);
    car.setTexture(&carTexture);
    sf::Vector2u carTextureSize = carTexture.getSize();
    carTextureSize.y /= 4;

    // --- Tile sprite ---
    sf::RectangleShape tile(sf::Vector2f(110.0f, 110.0f));
    tile.setOrigin(55.0f, 55.0f); // Center the origin properly
    tile.setTexture(&tileTexture);

    // --- Car State ---
    float x = TILE_WIDTH / 2.0f;
    float y = TILE_HEIGHT / 2.0f;
    float sx = x, sy = y;
    float speed = 0, angle = 0.0f;
    const float maxSpeed = 2.0f, acc = 0.05f, deacc = 0.08f, turnSpeed = 0.02f;
    Orientation state = STRAIGHT;

    // --- Main loop ---
    while (window.isOpen()) {
        sf::Event evt;
        while (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed || (evt.type == sf::Event::KeyPressed && evt.key.code == sf::Keyboard::Q))
                window.close();
        }

        // --- Input ---
        bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
        bool down = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
        bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
        bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);

        if (up && speed < maxSpeed) speed += (speed < 0) ? deacc : acc;
        if (down && speed > -maxSpeed) speed -= (speed > 0) ? deacc : acc;
        if (!up && !down) {
            speed += (speed > 0) ? -deacc : (speed < 0) ? deacc : 0;
            if (std::abs(speed) < 0.01f) speed = 0;
        }

        if (left && speed != 0) {
            angle -= turnSpeed * (speed / maxSpeed);
            state = LEFT;
        } else if (right && speed != 0) {
            angle += turnSpeed * (speed / maxSpeed);
            state = RIGHT;
        } else {
            state = STRAIGHT;
        }

        // --- Move car ---
        sx = x;
        sy = y;
        x += sin(angle) * speed;
        y -= cos(angle) * speed;

        // Clamp to screen
        float halfCarW = car.getSize().x / 2.0f;
        float halfCarH = car.getSize().y / 2.0f;

        x = std::clamp(x, halfCarW, static_cast<float>(window.getSize().x) - halfCarW);
        y = std::clamp(y, halfCarH, static_cast<float>(window.getSize().y) - halfCarH);

        // --- Collision detection ---
        bool collided = false;
        for (int row = 0; row < max_rows && !collided; ++row) {
            for (int col = 0; col < max_cols && !collided; ++col) {
                if (hexgrid[row][col] != OBSTACLE) continue;

                float xOffset = (row % 2 == 0) ? TILE_WIDTH / 2.0f : 0.0f;
                sf::Vector2f hexCenter(
                    col * TILE_WIDTH + xOffset + TILE_WIDTH / 2.0f,
                    row * TILE_HEIGHT * 0.75f + TILE_HEIGHT / 2.0f
                );

                if (pointInHexagon(sf::Vector2f(x, y), hexCenter, TILE_WIDTH / 2.0f)) {
                    collided = true;
                    break;
                }
            }
        }

        if (collided) {
            x = sx;
            y = sy;
            speed = 0;
        }

        // --- Mouse interaction ---
        static bool lastMousePressed = false;
        bool currentMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

        if (currentMousePressed && !lastMousePressed) { // Only trigger on mouse press, not hold
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

            for (int row = 0; row < max_rows; ++row) {
                for (int col = 0; col < max_cols; ++col) {
                    float xOffset = (row % 2 == 0) ? TILE_WIDTH / 2.0f : 0.0f;
                    sf::Vector2f hexCenter(
                        col * TILE_WIDTH + xOffset + TILE_WIDTH / 2.0f,
                        row * TILE_HEIGHT * 0.75f + TILE_HEIGHT / 2.0f
                    );

                    if (pointInHexagon(worldPos, hexCenter, TILE_WIDTH / 2.0f)) {
                        // --- Check if car is inside this hex ---
                        if (pointInHexagon(sf::Vector2f(x, y), hexCenter, TILE_WIDTH / 2.0f)) {
                            std::cout << "Cannot modify hexagon under the car in (" << x << ", " << y << ")!\n";
                            continue;
                        }
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
                            hexgrid[row][col] = TileType::EMPTY;
                        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                            hexgrid[row][col] = TileType::START;
                        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
                            hexgrid[row][col] = TileType::STOP;
                        else
                            hexgrid[row][col] = TileType::OBSTACLE;
                        break;
                    }
                }
            }
        }
        lastMousePressed = currentMousePressed;

        // --- Rendering ---
        window.clear(sf::Color(150, 150, 150));
        for (int row = 0; row < max_rows; row++) {
            for (int col = 0; col < max_cols; col++) {
                int tileType = hexgrid[row][col];
				// Only render non-empty tiles or render all tiles with different colors
                tile.setTextureRect(sf::IntRect(150 * tileType, 0, 150, 172));

                float xOffset = (row % 2 == 0) ? TILE_WIDTH / 2.0f : 0.0f;
                tile.setPosition(
                    col * TILE_WIDTH + xOffset + TILE_WIDTH / 2.0f, 
                    row * TILE_HEIGHT * 0.75f + TILE_HEIGHT / 2.0f // Fixed: consistent with collision detection
                );
                window.draw(tile);
            }
        }

        // Draw car on top
        car.setTextureRect(sf::IntRect(0, carTextureSize.y * state, carTextureSize.x, carTextureSize.y));
        car.setPosition(x, y);
        car.setRotation(angle * 180.0f / PI);
        window.draw(car);
        window.display();
    }

    return 0;
}