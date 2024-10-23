#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <cstdlib>
#include <utility>
#include <queue>
#include <random>
#include <SFML/Graphics/Vertex.hpp>
#include "Matrix.h"


#ifndef PLAYER_H
#define PLAYER_H

#define Width 800
#define Height 800
#define dw 30
#define dh 30

std::vector<std::pair<int, int>> path; // Para almacenar el camino encontrado

struct Node{
    int x, y;
    int dist;

    bool operator>(const Node& other) const {
        return dist > other.dist;
    }

};

// Clase para manejar un "player" con una imagen PNG
class Player {
private:
    sf::Sprite sprite;
    sf::Texture texture;
    std::pair<int, int> position;
    std::random_device rd;
    std::mt19937 gen;
    sf::Clock moveClock;    // Nuevo temporizador
    float updateInterval;   // Intervalo para la actualización del movimiento
    bool Dijkst;
    sf::Color pathColor;
    sf::Font font;
    sf::Text healthText;
    std::pair<int, int> initialPosition; // Tupla para almacenar posicion inicial

    const int MAX_RESPAWNS = 2; // Maximo numero de spawns
    std::mt19937 rng;
    std::uniform_int_distribution<int> powerDist;
    bool isDestroyed;

public:
    int health = 100; // Salud inicial de los player
    bool dobleturno;
    bool boostdamage;
    bool precision;
    bool _isMoving; // Flag para indicar movimiento
    auto getcol(){return pathColor;}
    bool dobleturnoActivado;
    bool dobleturnoDisponible;
    bool guidedShot;  // Nuevo: para el disparo guiado
    int respawnCount;
    // Se crea el Player con los atributos respectivos
    Player(const std::string& textureFile, bool DK)
            : _isMoving(false), gen(rd()), updateInterval(0.2f), Dijkst(DK),
              rng(std::random_device{}()), powerDist(1, 4),
              dobleturno(false), boostdamage(false), precision(false),
              dobleturnoActivado(false), dobleturnoDisponible(false), guidedShot(false),
              isDestroyed(false), respawnCount(0) {
        if (!texture.loadFromFile(textureFile)) {
            std::cerr << "Error loading texture: " << textureFile << std::endl;
        }
        // Cargar Sprite y centrarlo en una posicion de la cuadricula
        sprite.setTexture(texture);
        setPosition(0, 0);

        // Cargar la fuente para el texto de salud
        if (!font.loadFromFile("../ProyectoII/Assets/Roboto/Roboto-Thin.ttf")) {
            std::cerr << "Error loading font" << std::endl;
        }

        // Configurar el texto de salud
        healthText.setFont(font);
        healthText.setCharacterSize(12);
        healthText.setFillColor(sf::Color::White);
        healthText.setOutlineColor(sf::Color::Black);
        healthText.setOutlineThickness(1);
        updateHealthText();
        initialPosition = {0,0}; // Guardar la posición inicial
    }

    // Actualizar texto de salud
    void updateHealthText() {
        healthText.setString(std::to_string(health));
        // Centrar el texto sobre el sprite
        sf::FloatRect textBounds = healthText.getLocalBounds();
        healthText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
        sf::Vector2f spritePosition = sprite.getPosition();
        healthText.setPosition(spritePosition.x + sprite.getGlobalBounds().width / 2.0f,
                               spritePosition.y - 10); // 10 pixeles arriba del tanque
    }

    // Configurar el intervalo de actualización de movimiento
    void setUpdateInterval(float interval) {
        updateInterval = interval;
    }

    // Funcion para la posicion del tanque
    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
        position = {static_cast<int>(y / dh), static_cast<int>(x / dw)};
        if (initialPosition.first == 0 && initialPosition.second == 0) {
            initialPosition = position; // Guardamos la posición inicial la primera vez que se establece
        }
    }

    // Metodo para darle color al camino
    void setPathColor(sf::Color color) {
        pathColor = color;
    }

    // Getter del color
    sf::Color getColor() const {
        return pathColor;
    }

    // Tamaño del sprite
    void setSize(float width, float height) {
        sf::Vector2u textureSize = texture.getSize();
        float scaleX = width / textureSize.x;
        float scaleY = height / textureSize.y;
        sprite.setScale(scaleX, scaleY);
    }

    // Booleano para tanque que debe ser removido
    bool isDestroyedPermanently() const {
        return isDestroyed;
    }

    // Tanque destruido
    void destroy() {
        isDestroyed = true;
        health = 0;
        clearPath();
        _isMoving = false;
    }

    // Gestiona el respawn del tanque
    void respawn() {
        if (respawnCount < MAX_RESPAWNS) {
            health = 100; // Vida inicial
            setPosition(initialPosition.second * dw, initialPosition.first * dh); // Posicion inicial
            updateHealthText();
            respawnCount++; // Aumenta contador de respawn
            isDestroyed = false;
            std::cout << "Tank respawned. New health: " << health << ", New position: ("
                      << initialPosition.first << ", " << initialPosition.second << ")" << std::endl;
        } else {
            std::cout << "Cannot respawn. Max respawns reached." << std::endl;
        }
    }

    bool canRespawn() const {
        return respawnCount < MAX_RESPAWNS && health <= 0;
    }

    void resetRespawnCount() {
        respawnCount = 0;
    }
    // Activa los poderes
    void activateRandomPower() {
        // Elige de forma aleatoria
        int power = powerDist(rng);
        // Lista de poderes que los pasa a true
        switch (power) {
            case 1:
                dobleturnoActivado = true;
                std::cout << "Doble turno activado para el próximo turno del jugador " << getcol().toInteger() << std::endl;
                break;
            case 2:
                boostdamage = true;
                std::cout << "Boost de daño activado para el jugador " << getcol().toInteger() << std::endl;
                break;
            case 3:
                precision = true;
                std::cout << "Precisión de movimiento activada para el jugador " << getcol().toInteger() << std::endl;
                break;
            case 4:
                guidedShot = true;
                std::cout << "Disparo guiado activado para el jugador " << getcol().toInteger() << std::endl;
                break;
        }
    }

    // Metodo para dibujar el camino a recorrer
    void drawPath(sf::RenderWindow& window) const {
        if (!isDestroyed) {
            for (const auto& step : path) {
                sf::RectangleShape pathRect(sf::Vector2f(dw, dh));
                pathRect.setFillColor(sf::Color(pathColor.r, pathColor.g, pathColor.b, 128)); // Semi-transparent
                pathRect.setPosition(step.second * dw, step.first * dh);
                window.draw(pathRect);
            }
        }
    }

    // Dibuja el tanque y su vida
    void draw(sf::RenderWindow& window) {
        // Si no esta destruido
        if (!isDestroyed) {
            window.draw(sprite);
            updateHealthText();
            window.draw(healthText);
        }
    }

    void startMoving() {
        if (!path.empty()) {
            _isMoving = true; // Inicia el movimiento
        }
    }

    // Metodo para manejar el inicio del turno
    void startTurn() {
        if (dobleturnoActivado) {
            dobleturnoDisponible = true;
            dobleturnoActivado = false;
        }
    }

    // Nuevo metodo para verificar si el jugador puede realizar otra acción
    bool canPerformAction() {
        return dobleturnoDisponible;
    }

    // Nuevo metodo para consumir una acción del doble turno
    void consumeAction() {
        if (dobleturnoDisponible) {
            dobleturnoDisponible = false;
        }
    }

    // Getter de la posicion
    std::pair<int, int> getPosition() const {
        return position;
    }

    // Función que implementa el algoritmo BFS para encontrar un camino hasta las coordenadas objetivo
    void bfsMove(int targetX, int targetY, Graph matrix, std::deque<Player*> players) {
        // Validación inicial: comprueba si las coordenadas objetivo están dentro de los límites
        if (targetX < 0 || targetX >= matrix.size() || targetY < 0 || targetY >= matrix.size()) {
            return;
        }

        // Cola para el BFS que almacena pares de coordenadas (y,x)
        std::queue<std::pair<int, int>> q;

        // Matriz de visitados para evitar ciclos
        std::vector<std::vector<bool>> visited(matrix.size(),
            std::vector<bool>(matrix.size(), false));

        // Matriz que guarda para cada celda, desde qué celda se llegó a ella
        std::vector<std::vector<std::pair<int, int>>> parent(matrix.size(),
            std::vector<std::pair<int, int>>(matrix.size(), {-1, -1}));

        // Iniciar BFS desde la posición actual
        q.push(position);
        visited[position.first][position.second] = true;

        while (!q.empty()) {
            auto current = q.front();
            q.pop();

            // Si llegamos al objetivo, reconstruimos el camino
            if (current.second == targetX && current.first == targetY) {
                path.clear();
                std::pair<int, int> step = current;
                // Reconstruir el camino siguiendo los padres desde el objetivo hasta el inicio
                while (step != position) {
                    path.push_back(step);
                    step = parent[step.first][step.second];
                }
                // Invertir el camino para que vaya desde el inicio hasta el objetivo
                std::reverse(path.begin(), path.end());
                return;
            }

            // Explorar las 4 direcciones posibles: derecha, abajo, izquierda, arriba
            for (const auto& direction : std::vector<std::pair<int, int>>{{0, 1}, {1, 0}, {0, -1}, {-1, 0}}) {
                int neighborX = current.second + direction.second;
                int neighborY = current.first + direction.first;

                // Verificar si la nueva posición es válida:
                // - Dentro de los límites
                // - No es un obstáculo (valor != 0 en la matriz)
                // - No ha sido visitada
                if (neighborX >= 0 && neighborY >= 0 &&
                    neighborX < matrix.size() && neighborY < matrix.size() &&
                    matrix.xy(neighborY, neighborX) != 0 &&
                    !visited[neighborY][neighborX]) {

                    // Verificar si hay otro jugador en esa posición
                    bool isOccupied = false;
                    for (const Player* player : players) {
                        if (player != this &&
                            player->getPosition() == std::make_pair(neighborY, neighborX)) {
                            isOccupied = true;
                            break;
                        }
                    }

                    // Si la casilla está libre, se añade a la cola
                    if (!isOccupied) {
                        visited[neighborY][neighborX] = true;
                        parent[neighborY][neighborX] = current;
                        q.push({neighborY, neighborX});
                    }
                }
            }
        }
        // Si no se encontró camino, limpiar el vector de path
        path.clear();
    }

    // Función que genera un camino aleatorio hacia unas coordenadas objetivo
    void randomMove(int targetX, int targetY, Graph matrix, std::deque<Player*> players) {
        // Limpiar cualquier camino previo
        path.clear();

        // Comenzar desde la posición actual
        std::pair<int, int> current = position;

        // Continuar hasta alcanzar las coordenadas objetivo
        while (current.first != targetY || current.second != targetX) {
            // Vector para almacenar los movimientos válidos posibles
            std::vector<std::pair<int, int>> validMoves;

            // Definir las cuatro direcciones posibles: derecha, izquierda, abajo, arriba
            std::vector<std::pair<int, int>> directions = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

            // Evaluar cada dirección posible
            for (const auto& dir : directions) {
                // Calcular las nuevas coordenadas
                int newX = current.second + dir.second;
                int newY = current.first + dir.first;

                // Verificar si el movimiento es válido:
                // - Dentro de los límites del mapa
                // - No hay obstáculo (valor != 0 en la matriz)
                if (newX >= 0 && newX < matrix.size() &&
                    newY >= 0 && newY < matrix.size() &&
                    matrix.xy(newY, newX) != 0) {

                    // Verificar si hay otro jugador en esa posición
                    bool isOccupied = false;
                    for (const Player* player : players) {
                        if (player != this &&
                            player->getPosition() == std::make_pair(newY, newX)) {
                            isOccupied = true;
                            break;
                        }
                    }

                    // Si la posición está libre, añadirla como movimiento válido
                    if (!isOccupied) {
                        validMoves.push_back({newY, newX});
                    }
                }
            }

            // Si no hay movimientos válidos, terminar
            if (validMoves.empty()) {
                break;
            }

            // Seleccionar aleatoriamente uno de los movimientos válidos
            std::uniform_int_distribution<> dis(0, validMoves.size() - 1);
            auto nextMove = validMoves[dis(gen)];

            // Añadir el movimiento al camino y actualizar la posición actual
            path.push_back(nextMove);
            current = nextMove;
        }
    }

    // Implementación del algoritmo de Dijkstra para encontrar el camino más corto
    void Dijkstra(int targetY, int targetX, Graph matrix, std::deque<Player*> players) {
        // Vector para almacenar el camino final
        std::vector<std::pair<int, int>> pathreal;

        // Dimensiones de la matriz
        int rows = matrix.size();
        int cols = matrix.size();

        // Matriz para almacenar el nodo previo en el camino óptimo
        std::vector<std::vector<std::pair<int, int>>> prev(rows,
            std::vector<std::pair<int, int>>(cols, {-1, -1}));

        // Matriz de distancias, inicialmente infinitas
        std::vector<std::vector<int>> dist(rows,
            std::vector<int>(cols, std::numeric_limits<int>::max()));

        // Cola de prioridad para procesar nodos por distancia mínima
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> q;

        // Posición inicial
        int startX = position.first;
        int startY = position.second;

        // Inicializar distancia en punto de inicio
        dist[startX][startY] = 0;
        q.push({startX, startY, 0});

        // Definir las cuatro direcciones posibles de movimiento
        std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

        // Bucle principal de Dijkstra
        while (!q.empty()) {
            Node current = q.top();
            q.pop();

            // Si llegamos al objetivo, reconstruimos y devolvemos el camino
            if (current.x == targetX && current.y == targetY) {
                // Reconstruir la ruta desde el objetivo hasta el inicio
                int cx = targetX, cy = targetY;
                while (cx != -1 && cy != -1) {
                    path.push_back({cx, cy});
                    std::tie(cx, cy) = prev[cx][cy];
                }
                // Invertir el camino para que vaya desde el inicio hasta el objetivo
                std::reverse(path.begin(), path.end());
                return;
            }

            // Explorar todas las direcciones posibles
            for(const auto& direction : directions) {
                int newX = current.x + direction.first;
                int newY = current.y + direction.second;

                // Verificar si la nueva posición es válida:
                // - Dentro de los límites
                // - No es un obstáculo (valor != 0)
                if (newX >= 0 && newX < rows && newY >= 0 && newY < cols &&
                    matrix.xy(newX, newY) != 0) {

                    // Calcular nueva distancia
                    int newDist = current.dist + 1;

                    // Verificar si hay otro jugador en la posición
                    bool isOccupied = false;
                    for (const Player* player : players) {
                        if (player != this &&
                            player->getPosition() == std::make_pair(newX, newY)) {
                            isOccupied = true;
                            break;
                        }
                    }

                    // Si encontramos un camino más corto y la casilla está libre
                    if (newDist < dist[newX][newY] && !isOccupied) {
                        dist[newX][newY] = newDist;
                        prev[newX][newY] = {current.x, current.y}; // Actualizar el camino
                        q.push({newX, newY, newDist});
                    }
                }
            }
        }
    }

    // Elegir el algoritmo y configurar la velocidad según el tipo de movimiento
    bool moveTo(int targetX, int targetY, Graph matrix, std::deque<Player*> players) {
        // Si el jugador está destruido, no puede moverse
        if (isDestroyed) return false;

        // Verificar si las coordenadas objetivo están dentro de los límites del mapa
        if (targetX < 0 || targetX >= matrix.size() || targetY < 0 || targetY >= matrix.size()) {
            std::cout << "Posición fuera de los límites." << std::endl;
            return false;
        }

        // Verificar si la posición objetivo está ocupada por otro jugador
        for (Player* player : players) {
            if (player != this && player->getPosition() == std::make_pair(targetY, targetX)) {
                std::cout << "La posición está ocupada por otro jugador." << std::endl;
                return false;
            }
        }

        // Lógica de selección del algoritmo de movimiento
        if(!Dijkst) {  // Si no se usa Dijkstra
            if(!precision) {  // Modo de precisión normal
                // 50% de probabilidad para cada algoritmo
                std::uniform_int_distribution<> dis(0, 1);
                if (dis(gen) == 0) {
                    bfsMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.1f);  // Movimiento más lento para BFS
                } else {
                    randomMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.0001f);  // Movimiento más rápido para Random
                }
            }
            else {  // Modo de alta precisión
                // 90% BFS, 10% Random
                std::uniform_int_distribution<> dis(0, 9);
                if (dis(gen) <= 8) {
                    bfsMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.1f);
                } else {
                    randomMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.0001f);
                }
            }
        }
        else {  // Si se usa Dijkstra
            if(!precision) {  // Modo de precisión normal
                // 80% Dijkstra, 20% Random
                std::uniform_int_distribution<> dis(0, 9);
                int num = dis(gen);
                if(num < 8) {
                    Dijkstra(targetX, targetY, matrix, players);
                    setUpdateInterval(0.1f);
                }
                else {
                    randomMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.0001f);
                }
            }
            else {  // Modo de alta precisión
                // 90% Dijkstra, 10% Random
                std::uniform_int_distribution<> dis(0, 9);
                int num = dis(gen);
                if(num <= 8) {
                    Dijkstra(targetX, targetY, matrix, players);
                    setUpdateInterval(0.1f);
                }
                else {
                    randomMove(targetX, targetY, matrix, players);
                    setUpdateInterval(0.0001f);
                }
            }
        }

        // Resetear el modo de precisión y actualizar estado de movimiento
        precision = false;
        _isMoving = !path.empty();
        return _isMoving;
    }

    bool isMoving() const {
        return _isMoving;
    }

    // Metodo para limpiar el camino
    void clearPath() {
        path.clear();
        _isMoving = false;
    }

    // Metodo para la getion de movimiento
    void updatePosition() {
        if (!isDestroyed && _isMoving && !path.empty()) {
            // Si ha pasado suficiente tiempo, actualizar la posición
            if (moveClock.getElapsedTime().asSeconds() >= updateInterval) {
                auto nextStep = path.front();
                path.erase(path.begin());

                // Actualiza la posición del jugador
                setPosition(nextStep.second * dw, nextStep.first * dh);

                // Reiniciar el temporizador después de mover
                moveClock.restart();

                // Verifica si hemos llegado a la última posición
                if (path.empty()) {
                    _isMoving = false;
                    position = nextStep;
                }
            }
        }
    }

    // Metodo para gestionar el daño segun el color del tanque
    void hit(float damage) {
        // Si es celeste o azul recibe 25% de daño
        if(pathColor == sf::Color::Blue || pathColor == sf::Color::Cyan) {
            damage *= 0.25f;
        }
        // Si es rojo o amarillo recibe 50% de daño
        else {
            damage *= 0.5f;
        }
        health -= damage; // Resta a la vida el daño
        if (health <= 0) {
            health = 0;
            if (!canRespawn()) { // Si no tiene mas respawns se destruye
                destroy();
            }
        }
        updateHealthText();
        std::cout << "Bullet hit a player " << damage << " de daño" << std::endl;
    }

};

#endif //PLAYER_H
