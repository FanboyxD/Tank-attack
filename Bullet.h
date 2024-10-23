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
    sf::CircleShape shape;  // Forma gráfica de la bala
    sf::Vector2f position;  // Posición actual de la bala
    sf::Vector2f direction;  // Dirección de movimiento de la bala
    float speed;  // Velocidad de la bala
    bool active;  // Estado de la bala, indica si está activa
    sf::Color color;  // Color de la bala
    int bounceCount;  // Número de rebotes realizados
    static const int MAX_BOUNCES = 3;  // Número máximo de rebotes permitidos
    Player* owner;  // Puntero al jugador que disparó la bala
    bool hasBounced;  // Indica si la bala ha rebotado al menos una vez

    // Variables para gestionar el trazo de la trayectoria de la bala
    std::deque<sf::Vector2f> trajectory;  // Cola para almacenar la trayectoria de la bala
    sf::VertexArray trajectoryLine;  // Línea gráfica que representa la trayectoria
    static const int MAX_TRAJECTORY_LENGTH = 100000000;  // Longitud máxima de la trayectoria

    // Disparo guiado (A*): booleano que indica si la bala es guiada
    bool isGuidedShot;
    std::vector<sf::Vector2f> guidedPath;  // Ruta guiada calculada por A*
    size_t currentPathIndex;  // Índice actual en la ruta guiada

    // Función heurística utilizada en el algoritmo A* para estimar la distancia
    float heuristic(sf::Vector2f a, sf::Vector2f b) {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));  // Distancia Euclidiana
    }

    // Implementación del algoritmo A* para encontrar una ruta guiada
    std::vector<sf::Vector2f> aStar(sf::Vector2f start, sf::Vector2f goal, const Graph& matrix) {
        int rows = matrix.size();
        int cols = matrix.size();
        std::vector<std::vector<float>> gScore(rows, std::vector<float>(cols, INFINITY));  // Coste acumulado
        std::vector<std::vector<float>> fScore(rows, std::vector<float>(cols, INFINITY));  // Coste estimado total
        std::vector<std::vector<sf::Vector2f>> cameFrom(rows, std::vector<sf::Vector2f>(cols));  // Rutas anteriores

        // Función lambda para comparar las puntuaciones fScore
        auto compare = [&](sf::Vector2f a, sf::Vector2f b) {
            return fScore[a.y][a.x] > fScore[b.y][b.x];
        };

        // Cola de prioridad para nodos abiertos en el A*
        std::priority_queue<sf::Vector2f, std::vector<sf::Vector2f>, decltype(compare)> openSet(compare);

        // Convertir posiciones a coordenadas de la cuadrícula
        sf::Vector2f startGrid(std::floor(start.x / dw), std::floor(start.y / dh));
        sf::Vector2f goalGrid(std::floor(goal.x / dw), std::floor(goal.y / dh));

        gScore[startGrid.y][startGrid.x] = 0;  // El coste del nodo inicial es 0
        fScore[startGrid.y][startGrid.x] = heuristic(startGrid, goalGrid);  // Heurística inicial
        openSet.push(startGrid);

        // Bucle principal del A*
        while (!openSet.empty()) {
            sf::Vector2f current = openSet.top();  // Nodo con menor coste estimado
            openSet.pop();

            // Si se alcanza el objetivo, reconstruir el camino
            if (current == goalGrid) {
                std::vector<sf::Vector2f> path;
                while (current != startGrid) {
                    path.push_back(sf::Vector2f(current.x * dw + dw/2, current.y * dh + dh/2));
                    current = cameFrom[current.y][current.x];
                }
                path.push_back(start);  // Añadir la posición inicial al camino
                std::reverse(path.begin(), path.end());  // Invertir el camino para que esté en orden correcto
                return path;
            }

            // Vecinos del nodo actual
            std::vector<sf::Vector2f> neighbors = {
                    {current.x + 1, current.y}, {current.x - 1, current.y},
                    {current.x, current.y + 1}, {current.x, current.y - 1}
            };

            // Evaluar vecinos
            for (auto& neighbor : neighbors) {
                if (neighbor.x < 0 || neighbor.x >= cols || neighbor.y < 0 || neighbor.y >= rows) continue;
                if (matrix.xy(neighbor.y, neighbor.x) == 0) continue;  // Evitar nodos inaccesibles

                float tentative_gScore = gScore[current.y][current.x] + 1;

                // Actualizar la mejor ruta si se encuentra un camino más corto
                if (tentative_gScore < gScore[neighbor.y][neighbor.x]) {
                    cameFrom[neighbor.y][neighbor.x] = current;
                    gScore[neighbor.y][neighbor.x] = tentative_gScore;
                    fScore[neighbor.y][neighbor.x] = gScore[neighbor.y][neighbor.x] + heuristic(neighbor, goalGrid);
                    openSet.push(neighbor);
                }
            }
        }

        return {};  // No se encontró un camino
    }

public:
    float damage;  // Daño que la bala puede infligir

    // Constructor de la clase Bullet
    Bullet(sf::Color bulletColor = sf::Color::Black)
            : speed(0.40f), active(false), color(bulletColor), bounceCount(0), owner(nullptr),
              hasBounced(false), isGuidedShot(false), currentPathIndex(0) {
        shape.setRadius(3.0f);  // Establecer el radio de la bala
        shape.setFillColor(color);  // Establecer el color de la bala
        trajectoryLine.setPrimitiveType(sf::LineStrip);  // Tipo de línea de trayectoria
    }

    // Método para disparar la bala
    void fire(const sf::Vector2f& start, const sf::Vector2f& target, Player* shooter, bool guided = false, const Graph* matrix = nullptr) {
        position = start;
        active = true;  // Activar la bala
        bounceCount = 0;  // Reiniciar el contador de rebotes
        owner = shooter;
        hasBounced = false;
        trajectory.clear();
        trajectory.push_back(start);  // Añadir la posición inicial a la trayectoria
        updateTrajectoryLine();  // Actualizar la representación gráfica de la trayectoria

        isGuidedShot = guided;  // Si es un disparo guiado, calcular la ruta
        if (isGuidedShot && matrix) {
            guidedPath = aStar(start, target, *matrix);
            currentPathIndex = 0;
            if (!guidedPath.empty()) {
                direction = guidedPath[0] - position;  // Calcular la dirección inicial hacia el primer punto de la ruta
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                direction /= length;  // Normalizar la dirección
            }
        } else {
            direction = target - start;  // Calcular la dirección hacia el objetivo
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            direction /= length;  // Normalizar la dirección
        }
    }

    // Actualizar la posición y comportamiento de la bala en cada frame
    void update(int windowWidth, int windowHeight) {
        if (active) {
            // Si es un disparo guiado, seguir la ruta
            if (isGuidedShot && currentPathIndex < guidedPath.size()) {
                sf::Vector2f target = guidedPath[currentPathIndex];
                direction = target - position;
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length < speed) {
                    position = target;  // Moverse al siguiente punto de la ruta
                    currentPathIndex++;
                } else {
                    direction /= length;
                    position += direction * speed;  // Avanzar en la dirección calculada
                }
            } else {
                position += direction * speed;  // Movimiento normal si no es guiada
            }

            trajectory.push_back(position);  // Actualizar la trayectoria
            if (trajectory.size() > MAX_TRAJECTORY_LENGTH) {
                trajectory.pop_front();  // Limitar el tamaño de la trayectoria
            }
            updateTrajectoryLine();  // Actualizar visualmente la línea de trayectoria

            // Gestionar rebotes en los bordes de la ventana
            if (position.x <= 0 || position.x >= windowWidth) {
                direction.x = -direction.x;
                bounceCount++;  // Incrementar el contador de rebotes
                hasBounced = true;
            }
            if (position.y <= 0 || position.y >= windowHeight) {
                direction.y = -direction.y;
                bounceCount++;
                hasBounced = true;
            }
            if (bounceCount >= MAX_BOUNCES) {
                active = false;  // Desactivar la bala si supera el número máximo de rebotes
            }
            shape.setPosition(position);  // Actualizar la posición gráfica de la bala
        }
    }

    // Actualizar la representación gráfica de la trayectoria
    void updateTrajectoryLine() {
        trajectoryLine.clear();
        for (const auto& pos : trajectory) {
            trajectoryLine.append(sf::Vertex(pos, sf::Color(color.r, color.g, color.b, 128)));  // Añadir los puntos de la trayectoria
        }
    }

    // Dibujar la bala y su trayectoria en la ventana
    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(trajectoryLine);  // Dibujar la línea de trayectoria
            window.draw(shape);  // Dibujar la bala
        }
    }

    // Métodos auxiliares para la gestión de la bala
    bool isActive() const { return active; }
    void deactivate() { active = false; }
    sf::Vector2f getPosition() const { return position; }
    Player* getOwner() const { return owner; }
    bool hasBouncedOnce() const { return hasBounced; }
};

// Clase para dibujar una línea de trayectoria preliminar
class TrajectoryLine {
private:
    sf::VertexArray line;  // Línea gráfica
    bool visible;  // Si la línea es visible
    sf::Color color;  // Color de la línea

public:
    TrajectoryLine() : line(sf::Lines, 2), visible(false), color(sf::Color(255, 0, 0, 128)) {
        line[0].color = color;
        line[1].color = color;
    }

    // Actualizar la línea entre dos puntos
    void update(sf::Vector2f start, const sf::Vector2f& end) {
        start.x += 15;
        start.y += 15;
        line[0].position = start;
        line[1].position = end;
        visible = true;
    }

    // Dibujar la línea si es visible
    void draw(sf::RenderWindow& window) {
        if (visible) {
            window.draw(line);
        }
    }

    // Ocultar la línea
    void hide() {
        visible = false;
    }

    // Cambiar el color de la línea
    void setColor(const sf::Color& newColor) {
        color = newColor;
        line[0].color = color;
        line[1].color = color;
    }
};

#endif //BULLET_H
