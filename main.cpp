#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <set>

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
    EMPTY,
    PATH
};

// --- Hexagon Grid Helpers ---
struct HexNode {
    int r, c;
    bool operator==(const HexNode& other) const {
        return r == other.r && c == other.c;
    }
    bool operator!=(const HexNode& other) const {
        return !(*this == other);
    }
    bool operator<(const HexNode& other) const {
        return r < other.r || (r == other.r && c < other.c);
    }
};

// Hash function for HexNode to be used in unordered_map
struct HexNodeHash {
    std::size_t operator()(const HexNode& node) const {
        return std::hash<int>()(node.r) ^ (std::hash<int>()(node.c) << 1);
    }
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

// --- A* Pathfinding ---
std::vector<HexNode> getNeighbors(HexNode node, int max_rows, int max_cols) {
    std::vector<HexNode> neighbors;
    int r = node.r;
    int c = node.c;

    // Offsets for "even-r" layout (even rows shifted right) based on drawing logic
    // row 0 (even): xOffset = TILE_WIDTH / 2.0f
    // row 1 (odd):  xOffset = 0.0f
    
    int even_row_offsets[6][2] = { {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, 0}, {1, 1} };
    int odd_row_offsets[6][2] =  { {-1, -1}, {-1, 0}, {0, -1}, {0, 1}, {1, -1}, {1, 0} };

    auto& current_offsets = (r % 2 == 0) ? even_row_offsets : odd_row_offsets;

    for (int i = 0; i < 6; ++i) {
        int nr = r + current_offsets[i][0];
        int nc = c + current_offsets[i][1];

        if (nr >= 0 && nr < max_rows && nc >= 0 && nc < max_cols) {
            neighbors.push_back({nr, nc});
        }
    }
    return neighbors;
}

float heuristic(HexNode a, HexNode b) {
    // Convert to Axial coordinates (q, r) for "Even-R" layout
    // q = col - (row + (row&1)) / 2
    // r = row
    
    auto get_q = [](int r, int c) {
        return c - (r + (r & 1)) / 2;
    };

    int aq = get_q(a.r, a.c);
    int ar = a.r;
    int bq = get_q(b.r, b.c);
    int br = b.r;

    // Hex distance = (|dq| + |dr| + |dq + dr|) / 2
    // equivalent to (|q1-q2| + |r1-r2| + |s1-s2|) / 2 where s = -q-r
    return (std::abs(aq - bq) + std::abs(ar - br) + std::abs((aq + ar) - (bq + br))) / 2.0f;
}

void aStar(std::vector<std::vector<int>>& grid, HexNode start, HexNode stop, int max_rows, int max_cols) {
    std::priority_queue<std::pair<float, HexNode>, std::vector<std::pair<float, HexNode>>, std::greater<std::pair<float, HexNode>>> openSet;
    std::unordered_map<HexNode, HexNode, HexNodeHash> cameFrom;
    std::unordered_map<HexNode, float, HexNodeHash> gScore;
    
    openSet.push({0.0f, start});
    gScore[start] = 0.0f;

    while (!openSet.empty()) {
        HexNode current = openSet.top().second;
        openSet.pop();

        if (current == stop) {
            // Reconstruct path
            HexNode temp = stop;
            while (temp != start) {
                if (grid[temp.r][temp.c] != TileType::START && grid[temp.r][temp.c] != TileType::STOP) {
                    grid[temp.r][temp.c] = TileType::PATH;
                }
                if (cameFrom.find(temp) == cameFrom.end()) break; // Should not happen if path exists
                temp = cameFrom[temp];
            }
            return;
        }

        for (HexNode neighbor : getNeighbors(current, max_rows, max_cols)) {
            if (grid[neighbor.r][neighbor.c] == TileType::OBSTACLE) continue;

            float tentative_gScore = gScore[current] + 1.0f; // Cost 1 for neighbor
            if (gScore.find(neighbor) == gScore.end() || tentative_gScore < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                float fScore = tentative_gScore + heuristic(neighbor, stop);
                openSet.push({fScore, neighbor});
            }
        }
    }
    std::cout << "No path found!\n";
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
    HexNode startNode = {0, 0};
    HexNode stopNode = {0, 0};

    if (max_rows > 0 && max_cols > 0) {
        hexgrid[0][0] = TileType::START;
        startNode = {0, 0};
        if (max_rows > 1 && max_cols > 1) {
            hexgrid[max_rows - 1][max_cols - 1] = TileType::STOP;
            stopNode = {max_rows - 1, max_cols - 1};
        } else {
            // If grid is too small, place STOP at a different position
            hexgrid[std::max(0, max_rows - 1)][std::max(0, max_cols - 1)] = TileType::STOP;
            stopNode = {std::max(0, max_rows - 1), std::max(0, max_cols - 1)};
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
            
            // Trigger Pathfinding
            if (evt.type == sf::Event::KeyPressed && evt.key.code == sf::Keyboard::Space) {
                // Clear previous path
                for(int r=0; r<max_rows; ++r) {
                    for(int c=0; c<max_cols; ++c) {
                        if(hexgrid[r][c] == TileType::PATH) hexgrid[r][c] = TileType::EMPTY;
                        if(hexgrid[r][c] == TileType::START) startNode = {r, c};
                        if(hexgrid[r][c] == TileType::STOP) stopNode = {r, c};
                    }
                }
                aStar(hexgrid, startNode, stopNode, max_rows, max_cols);
            }
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
                
                // Handle PATH rendering
                if (tileType == TileType::PATH) {
                    tile.setTextureRect(sf::IntRect(150 * TileType::EMPTY, 0, 150, 172));
                    tile.setFillColor(sf::Color(100, 255, 100)); // Light green for path
                } else {
                    tile.setTextureRect(sf::IntRect(150 * tileType, 0, 150, 172));
                    tile.setFillColor(sf::Color::White);
                }

                float xOffset = (row % 2 == 0) ? TILE_WIDTH / 2.0f : 0.0f;
                tile.setPosition(
                    col * TILE_WIDTH + xOffset + TILE_WIDTH / 2.0f, 
                    row * TILE_HEIGHT * 0.75f + TILE_HEIGHT / 2.0f
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