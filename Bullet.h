#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <queue>
#include "Player.h"

#ifndef BULLET_H
#define BULLET_H



class Bullet {
private:
    sf::CircleShape shape;
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
    bool active;
    sf::Color color;
    int bounceCount;
    static const int MAX_BOUNCES = 3; // Número máximo de rebotes permitidos
    Player* owner; // Nuevo: puntero al jugador que disparó la bala
    bool hasBounced; // Nueva variable para rastrear si la bala ha rebotado

    // Nueva estructura para el trazo de la bala
    std::deque<sf::Vector2f> trajectory;
    sf::VertexArray trajectoryLine;
    static const int MAX_TRAJECTORY_LENGTH = 100000000; // Ajusta según sea necesario
// Nuevo: para el disparo guiado por A*
    bool isGuidedShot;
    std::vector<sf::Vector2f> guidedPath;
    size_t currentPathIndex;

    // Función de heurística para A*
    float heuristic(sf::Vector2f a, sf::Vector2f b) {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
    }

    // Implementación de A*
    std::vector<sf::Vector2f> aStar(sf::Vector2f start, sf::Vector2f goal, const Graph& matrix) {
        int rows = matrix.size();
        int cols = matrix.size();
        std::vector<std::vector<float>> gScore(rows, std::vector<float>(cols, INFINITY));
        std::vector<std::vector<float>> fScore(rows, std::vector<float>(cols, INFINITY));
        std::vector<std::vector<sf::Vector2f>> cameFrom(rows, std::vector<sf::Vector2f>(cols));

        auto compare = [&](sf::Vector2f a, sf::Vector2f b) {
            return fScore[a.y][a.x] > fScore[b.y][b.x];
        };
        std::priority_queue<sf::Vector2f, std::vector<sf::Vector2f>, decltype(compare)> openSet(compare);

        sf::Vector2f startGrid(std::floor(start.x / dw), std::floor(start.y / dh));
        sf::Vector2f goalGrid(std::floor(goal.x / dw), std::floor(goal.y / dh));

        gScore[startGrid.y][startGrid.x] = 0;
        fScore[startGrid.y][startGrid.x] = heuristic(startGrid, goalGrid);
        openSet.push(startGrid);

        while (!openSet.empty()) {
            sf::Vector2f current = openSet.top();
            openSet.pop();

            if (current == goalGrid) {
                std::vector<sf::Vector2f> path;
                while (current != startGrid) {
                    path.push_back(sf::Vector2f(current.x * dw + dw/2, current.y * dh + dh/2));
                    current = cameFrom[current.y][current.x];
                }
                path.push_back(start);
                std::reverse(path.begin(), path.end());
                return path;
            }

            std::vector<sf::Vector2f> neighbors = {
                    {current.x + 1, current.y}, {current.x - 1, current.y},
                    {current.x, current.y + 1}, {current.x, current.y - 1}
            };

            for (auto& neighbor : neighbors) {
                if (neighbor.x < 0 || neighbor.x >= cols || neighbor.y < 0 || neighbor.y >= rows) continue;
                if (matrix.xy(neighbor.y, neighbor.x) == 0) continue;

                float tentative_gScore = gScore[current.y][current.x] + 1;

                if (tentative_gScore < gScore[neighbor.y][neighbor.x]) {
                    cameFrom[neighbor.y][neighbor.x] = current;
                    gScore[neighbor.y][neighbor.x] = tentative_gScore;
                    fScore[neighbor.y][neighbor.x] = gScore[neighbor.y][neighbor.x] + heuristic(neighbor, goalGrid);
                    openSet.push(neighbor);
                }
            }
        }

        return {}; // No path found
    }

public:
    float damage;

    Bullet(sf::Color bulletColor = sf::Color::Black)
            : speed(0.40f), active(false), color(bulletColor), bounceCount(0), owner(nullptr),
              hasBounced(false), isGuidedShot(false), currentPathIndex(0) {
        shape.setRadius(3.0f);
        shape.setFillColor(color);
        trajectoryLine.setPrimitiveType(sf::LineStrip);
    }

    void fire(const sf::Vector2f& start, const sf::Vector2f& target, Player* shooter, bool guided = false, const Graph* matrix = nullptr) {
        position = start;
        active = true;
        bounceCount = 0;
        owner = shooter;
        hasBounced = false;
        trajectory.clear();
        trajectory.push_back(start);
        updateTrajectoryLine();

        isGuidedShot = guided;
        if (isGuidedShot && matrix) {
            guidedPath = aStar(start, target, *matrix);
            currentPathIndex = 0;
            if (!guidedPath.empty()) {
                direction = guidedPath[0] - position;
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                direction /= length;
            }
        } else {
            direction = target - start;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            direction /= length;
        }
    }

    void update(int windowWidth, int windowHeight) {
        if (active) {
            if (isGuidedShot && currentPathIndex < guidedPath.size()) {
                sf::Vector2f target = guidedPath[currentPathIndex];
                direction = target - position;
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length < speed) {
                    position = target;
                    currentPathIndex++;
                } else {
                    direction /= length;
                    position += direction * speed;
                }
            } else {
                position += direction * speed;
            }

            trajectory.push_back(position);
            if (trajectory.size() > MAX_TRAJECTORY_LENGTH) {
                trajectory.pop_front();
            }
            updateTrajectoryLine();

            if (position.x <= 0 || position.x >= windowWidth) {
                direction.x = -direction.x;
                bounceCount++;
                hasBounced = true;
            }
            if (position.y <= 0 || position.y >= windowHeight) {
                direction.y = -direction.y;
                bounceCount++;
                hasBounced = true;
            }
            if (bounceCount >= MAX_BOUNCES) {
                active = false;
            }
            shape.setPosition(position);
        }
    }

    void updateTrajectoryLine() {
        trajectoryLine.clear();
        for (const auto& pos : trajectory) {
            trajectoryLine.append(sf::Vertex(pos, sf::Color(color.r, color.g, color.b, 128)));
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(trajectoryLine);
            window.draw(shape);
        }
    }


    bool isActive() const { return active; }
    void deactivate() { active = false; }
    sf::Vector2f getPosition() const { return position; }
    Player* getOwner() const { return owner; }
    bool hasBouncedOnce() const { return hasBounced; }
};

class TrajectoryLine {
private:
    sf::VertexArray line;
    bool visible;
    sf::Color color;

public:
    TrajectoryLine() : line(sf::Lines, 2), visible(false), color(sf::Color(255, 0, 0, 128)) {
        line[0].color = color;
        line[1].color = color;
    }

    void update(sf::Vector2f start, const sf::Vector2f& end) {
        start.x += 15;
        start.y += 15;
        line[0].position = start;
        line[1].position = end;
        visible = true;
    }

    void draw(sf::RenderWindow& window) {
        if (visible) {
            window.draw(line);
        }
    }

    void hide() {
        visible = false;
    }

    void setColor(const sf::Color& newColor) {
        color = newColor;
        line[0].color = color;
        line[1].color = color;
    }
};

#endif //BULLET_H
