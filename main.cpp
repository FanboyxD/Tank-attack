#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <utility>
#include <random>
#include <list>
#include "Player.h"
#include "Bullet.h"


#include "Matrix.h"

#define Width 800
#define Height 800
#define dw 30
#define dh 30


std::mutex matrixMutex;
std::condition_variable cv;
bool matrixReady = false;
int currentTurn = 0; // Indice del turno actual

int instanttx;
int instantty;


// Función para dibujar un cuadrado de color en una posición específica
void Draw(sf::RenderWindow& window, int A, int B, sf::Color color) {
    // Eliminar esta línea, ya que invierte las coordenadas Y
    // B = Height / dh - 1 - B;
    if (A < 0 || B < 0 || A >= Width / dw || B >= Height / dh) return;

    sf::RectangleShape square(sf::Vector2f(dw, dh));
    square.setFillColor(color);
    square.setPosition(A * dw, B * dh);
    window.draw(square);
}



Graph crearmatriz(int rows, int cols, int obstacleCount, const std::vector<std::pair<int, int>>& playerPositions, Graph jola) {
    jola.alltrue();

    // Mark player positions as occupied
    for (const auto& pos : playerPositions) {
        jola.removeEdge(pos.first, pos.second);
    }

    std::vector<std::pair<int, int>> availablePositions;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (jola.xy(i, j) == 1) {  // If the cell is not occupied by a player
                availablePositions.push_back({i, j});
            }
        }
    }

    // Place obstacles
    for (int i = 0; i < obstacleCount && !availablePositions.empty(); i++) {
        int randIndex = rand() % availablePositions.size();
        auto obstaclePos = availablePositions[randIndex];
        jola.removeEdge(obstaclePos.first, obstaclePos.second);
        availablePositions.erase(availablePositions.begin() + randIndex);
    }

    // Mark player positions as free again
    for (const auto& pos : playerPositions) {
        jola.addEdge(pos.first, pos.second);
    }

    {
        std::lock_guard<std::mutex> lock(matrixMutex);
        matrixReady = true;
    }
    cv.notify_all();
    return jola;
}

struct GamePlayer {
    std::vector<Player*> tanks;
    int movesRemaining;
    int id;

    GamePlayer(int playerId) : id(playerId), movesRemaining(2) {}
};

std::vector<GamePlayer> gamePlayers;
int currentPlayerIndex = 0;

int main() {
    try {
        sf::Font font;
        if (!font.loadFromFile("../ProyectoII/Assets/Roboto/Roboto-Thin.ttf")) {
            std::cerr << "Error loading font" << std::endl;
            return 1;
        }

        srand(static_cast<unsigned int>(time(0)));

        // Ajustar el tamaño de la ventana para acomodar el temporizador
        sf::RenderWindow window(sf::VideoMode(Width, Height + 40), "Tank Game");  // 40 píxeles extra para el temporizador

        Player player1blue("../ProyectoII/Assets/TanqueAzul.png", false);
        Player player1red("../ProyectoII/Assets/TanqueRojo.png", true);
        Player player2yell("../ProyectoII/Assets/TanqueAmarillo.png", true);
        Player player2sky("../ProyectoII/Assets/TanqueCeleste.png", false);

        // Set up game players
        GamePlayer player1(1);
        player1.tanks = {&player1blue, &player1red};
        GamePlayer player2(2);
        player2.tanks = {&player2yell, &player2sky};
        gamePlayers = {player1, player2};


        std::deque<Player*> activeTanks;
        for (auto& gamePlayer : gamePlayers) {
            for (auto tank : gamePlayer.tanks) {
                activeTanks.push_back(tank);
            }
        }

        player1blue.setSize(dw - 2, dh - 2);
        player1blue.setPosition(dw * 1, dh * 20);

        player2sky.setSize(dw - 2, dh - 2);
        player2sky.setPosition(dw * 22, dh * 2);

        player1red.setSize(dw - 2, dh - 2);
        player1red.setPosition(dw * 1, dh * 3);

        player2yell.setSize(dw - 2, dh - 2);
        player2yell.setPosition(dw * 22, dh * 20);

        player1blue.resetRespawnCount();
        player1red.resetRespawnCount();
        player2yell.resetRespawnCount();
        player2sky.resetRespawnCount();

        std::vector<std::pair<int, int>> playerPositions = {
            player1blue.getPosition(), player2sky.getPosition(),
            player1red.getPosition(), player2yell.getPosition()
        };

        std::vector<Bullet> bullets;
        bool canShoot = true;
        TrajectoryLine trajectoryLine;

        player1blue.setPathColor(sf::Color::Blue);
        player2sky.setPathColor(sf::Color::Cyan);
        player1red.setPathColor(sf::Color::Red);
        player2yell.setPathColor(sf::Color::Yellow);

        int rows = Height / dh;
        int cols = Width / dw;
        int obstacleCount = 20;

        Graph matriz(Height / dh);

        matriz.alltrue();


        std::deque<Player*> players = {&player1blue, &player2sky, &player1red, &player2yell};



        bool moveInitiated = false;

        matriz = crearmatriz(rows, cols, obstacleCount, playerPositions, matriz);

        matriz.toString();

        //std::thread matrixThread(generateAdjacencyMatrix, rows, cols, obstacleCount, playerPositions);

        sf::Clock clock;
        sf::Clock gameClock; // Reloj para el temporizador del juego
        // Configuración del temporizador
        sf::Text timerText;
        timerText.setFont(font);
        timerText.setCharacterSize(24);
        timerText.setFillColor(sf::Color::Black);  // Cambiado a negro

        // Calcular la posición del temporizador
        float timerX = Width / 2.0f;  // Centrado horizontalmente
        float timerY = Height + 10;   // 10 píxeles debajo de la cuadrícula del juego

        timerText.setPosition(timerX, timerY);
        timerText.setOrigin(timerText.getLocalBounds().width / 2, 0);  // Centrar el texto horizontalmente

        const sf::Time gameTime = sf::seconds(300); // 5 minutos
        bool actionTaken = false;
        Player* selectedTank = nullptr;
        int actionsRemaining = 1;
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                // Verifica si se presionó una tecla


                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        int clickX = event.mouseButton.x / dw;
                        int clickY = event.mouseButton.y / dh;

                        GamePlayer &currentPlayer = gamePlayers[currentPlayerIndex];

                        if (!selectedTank) {
                            // Select a tank
                            for (Player *tank : currentPlayer.tanks) {
                                if (!tank->isDestroyedPermanently() && tank->getPosition() == std::make_pair(clickY, clickX)) {
                                    selectedTank = tank;
                                    selectedTank->startTurn();
                                    actionsRemaining = selectedTank->canPerformAction() ? 2 : 1;
                                    trajectoryLine.setColor(selectedTank->getColor());
                                    std::cout << "Tank selected for Player " << currentPlayer.id << std::endl;
                                    break;
                                }
                            }
                        } else if (actionsRemaining > 0 && !selectedTank->isDestroyedPermanently()) {
                            // Attempt to move the selected tank or perform an action
                            actionTaken = selectedTank->moveTo(clickX, clickY, matriz, activeTanks);
                            if (actionTaken) {
                                actionsRemaining--;
                                std::cout << "Tank moved. Actions remaining: " << actionsRemaining << std::endl;
                            }
                        }
                    } else if (event.mouseButton.button == sf::Mouse::Right && selectedTank && actionsRemaining > 0 && !selectedTank->isDestroyedPermanently()) {
                        // Activate power
                        selectedTank->activateRandomPower();
                        actionTaken = true;
                        actionsRemaining--;
                        std::cout << "Power activated. Actions remaining: " << actionsRemaining << std::endl;
                    } else if (event.mouseButton.button == sf::Mouse::Middle && selectedTank && actionsRemaining > 0 && !selectedTank->isDestroyedPermanently()) {
                            // Shoot

                            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                            sf::Vector2f startPos = sf::Vector2f((selectedTank->getPosition().second * dw) + 13.5,
                                                                 (selectedTank->getPosition().first * dh) + 13.5);
                            sf::Color bulletColor = selectedTank->getColor();

                            Bullet newBullet(bulletColor);
                            newBullet.damage = selectedTank->boostdamage ? 1000 : 100; // 45 if boosted, otherwise 22.5
                            newBullet.fire(startPos, mousePos, selectedTank, selectedTank->guidedShot, &matriz);
                            bullets.push_back(newBullet);
                            actionTaken = true;
                            actionsRemaining--;
                            std::cout << "Bullet fired. Actions remaining: " << actionsRemaining << std::endl;
                            selectedTank->boostdamage = false; // Reset boost after use
                            selectedTank->guidedShot = false; // Reset guided shot after use
                            std::cout << "Bullet fired" << std::endl;
                        }
                    }


                if (!players.empty()) {

                    if (event.type == sf::Event::MouseMoved && canShoot) {
                        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                        sf::Vector2f startPos = sf::Vector2f(players[currentTurn]->getPosition().second * dw,
                                                             players[currentTurn]->getPosition().first * dh);
                        trajectoryLine.update(startPos, mousePos);
                    }
                }
            }
            if (selectedTank) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                sf::Vector2f tankPos((selectedTank->getPosition().second * dw),
                                     (selectedTank->getPosition().first * dh));
                trajectoryLine.update(tankPos, mousePos);
            } else {
                trajectoryLine.hide();
            }
            // Actualizar el temporizador
            sf::Time remainingTime = gameTime - gameClock.getElapsedTime();
            if (remainingTime <= sf::Time::Zero) {
                remainingTime = sf::Time::Zero;
            }
            int minutes = static_cast<int>(remainingTime.asSeconds()) / 60;
            int seconds = static_cast<int>(remainingTime.asSeconds()) % 60;
            timerText.setString(
                    "Time: " + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds));
            // Centrar el texto horizontalmente después de actualizar su contenido
            timerText.setOrigin(timerText.getLocalBounds().width / 2, 0);

            {
                std::unique_lock<std::mutex> lock(matrixMutex);
                cv.wait(lock, [] { return matrixReady; });
            }

            window.clear(sf::Color::White);

            // Draw grid and obstacles
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    if (matriz.xy(i, j) == 0) {
                        Draw(window, j, i, sf::Color::Red); // Obstacle
                    } else {
                        Draw(window, j, i, sf::Color(240, 240, 240)); // Normal cell
                    }

                    // Draw cell border
                    sf::RectangleShape border(sf::Vector2f(dw, dh));
                    border.setFillColor(sf::Color::Transparent);
                    border.setOutlineThickness(1);
                    border.setOutlineColor(sf::Color(200, 200, 200));
                    border.setPosition(j * dw, i * dh);
                    window.draw(border);
                }
            }

            // Update and draw bullets
            for (auto it = bullets.begin(); it != bullets.end();) {
                it->update(Width, Height);
                it->draw(window);

                sf::Vector2f bulletPos = it->getPosition();
                int bulletGridX = static_cast<int>(bulletPos.x / dw);
                int bulletGridY = static_cast<int>(bulletPos.y / dh);

                if (bulletGridX >= 0 && bulletGridX < cols && bulletGridY >= 0 && bulletGridY < rows) {
                    if (matriz.xy(bulletGridY, bulletGridX) == 0) {
                        it->deactivate();
                        std::cout << "Bullet deactivated due to collision with obstacle" << std::endl;
                    }

                    for (Player* player : activeTanks) {
                        if (player && !player->isDestroyedPermanently() && player->getPosition() == std::make_pair(bulletGridY, bulletGridX)) {
                            // Comprobar si la bala puede dañar al jugador
                            if (player != it->getOwner() || it->hasBouncedOnce()) {
                                it->deactivate();
                                player->hit(it->damage);
                            }
                        }
                    }
                }

                if (!it->isActive()) {
                    it = bullets.erase(it);
                } else {
                    ++it;
                }
            }
            // Remove inactive bullets
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                                         [](const Bullet &b) { return !b.isActive(); }),
                          bullets.end());

            // Actualizar player positions y manejar respawn
            for (auto it = activeTanks.begin(); it != activeTanks.end();) {
                Player *tank = *it;
                if (tank && !tank->isDestroyedPermanently()) {
                    if (tank->health > 0) {
                        tank->updatePosition();
                        tank->drawPath(window);
                        tank->draw(window);
                        ++it;
                    } else if (tank->canRespawn()) {
                        std::cout << "Respawning tank: " << (tank->getColor() == sf::Color::Cyan ? "Sky" : "Other") << std::endl;
                        std::cout << "Respawn count before: " << tank->respawnCount << std::endl;
                        tank->respawn();
                        std::cout << "Respawn count after: " << tank->respawnCount << std::endl;
                        ++it;
                    } else {
                        std::cout << "Tank destroyed permanently: " << (tank->getColor() == sf::Color::Cyan ? "Sky" : "Other") << std::endl;
                        tank->destroy();
                        if (currentTurn >= activeTanks.size() - 1) {
                            currentTurn = 0;
                        }
                        it = activeTanks.erase(it);
                    }
                } else {
                    it = activeTanks.erase(it);
                    std::cout << "A tank has been removed from the game." << std::endl;
                }
            }


            // Check if the current player's turn is over
            if (actionTaken && actionsRemaining == 0) {
                actionTaken = false;
                if (selectedTank) {
                    selectedTank->consumeAction(); // Consumir la acción del doble turno si estaba disponible
                }
                selectedTank = nullptr;
                trajectoryLine.hide();

                // Cambiar al siguiente jugador que tenga tanques activos
                do {
                    currentPlayerIndex = (currentPlayerIndex + 1) % gamePlayers.size();
                } while (std::all_of(gamePlayers[currentPlayerIndex].tanks.begin(),
                                     gamePlayers[currentPlayerIndex].tanks.end(),
                                     [](Player* tank) { return tank->isDestroyedPermanently(); }));

                std::cout << "Turn of Player " << gamePlayers[currentPlayerIndex].id << std::endl;
            }

            // Draw players and trajectory line
            for (Player *player: players) {
                player->drawPath(window);
                player->draw(window);
            }

            window.draw(timerText);
            trajectoryLine.draw(window);
            window.display();


            // Verificar si el juego ha terminado

            bool timeUp = (remainingTime <= sf::Time::Zero);

            // Contar tanques vivos por jugador
            int player1Tanks = 0;
            int player2Tanks = 0;

            for (const auto& tank : activeTanks) {
                if (tank->getColor() == sf::Color::Blue || tank->getColor() == sf::Color::Red) {
                    player1Tanks++;
                } else if (tank->getColor() == sf::Color::Yellow || tank->getColor() == sf::Color(0, 255, 255)) { // Celeste
                    player2Tanks++;
                }
            }

            if (activeTanks.size() <= 1 || timeUp || (player1Tanks == 0 || player2Tanks == 0)) {
                window.close(); // Cerrar la ventana principal del juego
                sf::Color winnerColor;
                std::string winnerText;

                if (timeUp) {
                    // Contar tanques vivos por jugador
                    std::vector<int> tankCounts(gamePlayers.size(), 0);
                    for (size_t i = 0; i < gamePlayers.size(); ++i) {
                        for (const auto &tank: gamePlayers[i].tanks) {
                            if (tank->health > 0) {
                                tankCounts[i]++;
                            }
                        }
                    }

                    // Encontrar el jugador con más tanques vivos
                    auto maxTanks = std::max_element(tankCounts.begin(), tankCounts.end());
                    int winnerIndex = std::distance(tankCounts.begin(), maxTanks);

                    if (player1Tanks==player2Tanks) {
                        winnerColor = sf::Color::White;
                        winnerText = "Tiempo agotado El juego ha terminado en empate.";
                    } else {
                        winnerText = "Tiempo agotado El jugador " + std::to_string(gamePlayers[winnerIndex].id) +
                                     " gana con " + std::to_string(*maxTanks) + " tanques vivos.";
                    }
                } else {
                    Player *lastTank = activeTanks[0];
                    std::vector<int> tankCounts(gamePlayers.size(), 0);
                    for (size_t i = 0; i < gamePlayers.size(); ++i) {
                        for (const auto &tank: gamePlayers[i].tanks) {
                            if (tank->health > 0) {
                                tankCounts[i]++;
                            }
                        }
                    }
                    // Encontrar el jugador con más tanques vivos
                    auto maxTanks = std::max_element(tankCounts.begin(), tankCounts.end());
                    winnerColor = lastTank->getColor();
                    for (const auto &gamePlayer: gamePlayers) {
                        if (std::find(gamePlayer.tanks.begin(), gamePlayer.tanks.end(), lastTank) !=
                            gamePlayer.tanks.end()) {
                            winnerText = "El jugador " + std::to_string(gamePlayer.id) + " ha ganado con "
                            + std::to_string(*maxTanks) + " tanques vivos.";
                            break;
                        }
                    }
                }

                // Crear una nueva ventana para mostrar al ganador
                sf::RenderWindow winnerWindow(sf::VideoMode(1300, 200), "Resultado del Juego");
                sf::Font font;
                if (!font.loadFromFile(
                        "../ProyectoII/Assets/Sixtyfour_Convergence/SixtyfourConvergence-Regular-VariableFont_BLED,SCAN,XELA,YELA.ttf")) {
                    std::cerr << "Error al cargar la fuente" << std::endl;
                    return 1;
                }
                sf::Text text(winnerText, font, 24);
                text.setFillColor(sf::Color::Black);
                text.setPosition(20, 80);
                sf::RectangleShape background(sf::Vector2f(800, 200));
                background.setFillColor(winnerColor);
                while (winnerWindow.isOpen()) {
                    sf::Event event;
                    while (winnerWindow.pollEvent(event)) {
                        if (event.type == sf::Event::Closed)
                            winnerWindow.close();
                    }
                    winnerWindow.clear(winnerColor);
                    winnerWindow.draw(background);
                    winnerWindow.draw(text);
                    winnerWindow.display();
                }
                break;  // Salir del bucle principal
            }
        }

        //matrixThread.join();
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}

