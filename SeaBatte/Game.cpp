#include "Game.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>

// Конструктор
Game::Game()
    : playerGrid(200, 200, 40, 10, 10),
    botGrid(screenWidth - 200 - 400, 200, 40, 10, 10),
    draggedShip(nullptr), isDragging(false), isGameStarted(false),
    botHuntingMode(false), playerTurn(true), isBotShooting(false), actionDelay(0.0f) {
    InitWindow(screenWidth, screenHeight, "Sea Battle");
    startButton = { screenWidth / 2 - 150, screenHeight - 100, 300, 80 }; // Кнопка старта
    SetTargetFPS(60);
    InitShips();
    std::srand(std::time(nullptr)); // Инициализация генератора случайных чисел
}

// Деструктор
Game::~Game() {
    CloseWindow();
}

// Инициализация кораблей
void Game::InitShips() {
    ships = {
        Ship(1, 700, 300, 40),
        Ship(1, 800, 300, 40),
        Ship(1, 900, 300, 40),
        Ship(1, 1000, 300, 40),
        Ship(2, 700, 400, 40),
        Ship(2, 800, 400, 40),
        Ship(2, 900, 400, 40),
        Ship(3, 700, 500, 40),
        Ship(3, 850, 500, 40),
        Ship(4, 700, 600, 40),
    };

    botShips = ships;
    botGrid.PlaceRandomShipsForBot(botShips);
}

// Обработка ввода
void Game::HandleInput() {
    Vector2 mousePosition = GetMousePosition();

    if (IsKeyPressed(KEY_ESCAPE)) {
        CloseWindow();
        return;
    }

    if (!isGameStarted) {
        // Перетаскивание корабля игрока
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (Ship& ship : ships) {
                if (CheckCollisionPointRec(mousePosition, ship.GetBounds())) {
                    draggedShip = &ship;
                    playerGrid.RemoveShipFromMap(*draggedShip);
                    break;
                }
            }
        }

        if (draggedShip) {
            draggedShip->SetPosition(mousePosition.x - draggedShip->GetWidth() / 2,
                mousePosition.y - draggedShip->GetHeight() / 2);

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                if (!playerGrid.SnapShipToGrid(*draggedShip)) {
                    draggedShip->ResetPosition();
                }
                draggedShip = nullptr;
            }
        }

        if (IsKeyPressed(KEY_R) && draggedShip) {
            draggedShip->Rotate();
        }
    }
    else {
        // Стрельба игрока
        if (!isBotShooting) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 selectedCell = botGrid.GetCellUnderMouse();
                if (selectedCell.x != -1 && selectedCell.y != -1) {
                    Shoot(selectedCell);
                }
            }
        }
    }
}

// Выстрел игрока
void Game::Shoot(Vector2 cell) {
    int x = static_cast<int>(cell.x);
    int y = static_cast<int>(cell.y);

    if (botGrid.HasBeenShot(x, y)) {
        std::cout << "Игрок выбрал уже обстрелянную клетку: (" << x << ", " << y << ").\n";
        return;
    }

    if (botGrid.Shoot(x, y)) {
        std::cout << "Игрок попал по клетке (" << x << ", " << y << ").\n";
    }
    else {
        std::cout << "Игрок промахнулся! Передача хода боту.\n";
        SwitchTurn(); // Передаём ход боту
    }
}


// Выстрел бота
void Game::BotShoot() {
    if (isBotShooting) {
        std::cout << "Бот уже стреляет. Ждём завершения...\n";
        return;
    }

    isBotShooting = true;
    std::cout << "Бот начал ход.\n";

    SetTimeout([this]() {
        Vector2 target = { -1, -1 }; // Инициализация target
        bool validShot = false;
        int x = -1, y = -1;

        // Если есть координаты в очереди, берём их
        if (!botTargetQueue.empty()) {
            target = botTargetQueue.back();
            botTargetQueue.pop_back();

            x = static_cast<int>(target.x);
            y = static_cast<int>(target.y);

            validShot = playerGrid.IsValidCell(x, y) && !playerGrid.HasBeenShot(x, y);
        }

        // Если очередь пуста или координаты из очереди некорректны, ищем случайную клетку
        while (!validShot) {
            target = GetRandomCell();
            x = static_cast<int>(target.x);
            y = static_cast<int>(target.y);

            validShot = playerGrid.IsValidCell(x, y) && !playerGrid.HasBeenShot(x, y);
        }

        // Совершаем выстрел
        if (playerGrid.Shoot(x, y)) {
            std::cout << "Бот попал по клетке (" << x << ", " << y << ").\n";
            playerGrid.MarkHit(x, y); // Отмечаем попадание

            if (playerGrid.IsShipDestroyed(x, y)) {
                std::cout << "Корабль уничтожен!\n";
                botTargetQueue.clear(); // Очищаем очередь
            }
            else {
                AddSurroundingTargets(x, y); // Добавляем соседние клетки для следующего хода
            }

            // После попадания, бот стреляет снова с задержкой
            SetTimeout([this]() {
                isBotShooting = false; // Завершаем текущий ход
                BotShoot(); // Бот продолжает ход
                }, 2.0f);

        }
        else {
            // Если промахнулся
            std::cout << "Бот промахнулся в клетку (" << x << ", " << y << ").\n";
            isBotShooting = false; // Завершаем ход

            // Передача хода игроку после промаха с задержкой

            std::cout << "Бот завершил ход. Передача хода игроку.\n";
            SwitchTurn(); // Передаём ход игроку

        }

        }, 2.0f); // Задержка перед первым выстрелом
}

// Установка задержки
void Game::SetTimeout(std::function<void()> func, float seconds) {
    std::cout << "SetTimeout: запускаем таймер на " << seconds << " секунд.\n";
    std::thread([func, seconds]() {
        std::this_thread::sleep_for(std::chrono::duration<float>(seconds));
        std::cout << "SetTimeout: таймер завершён, выполняем функцию.\n";
        func();
        }).detach();
}


// Проверка валидности клетки
bool Game::IsCellValid(Vector2 cell) {
    int x = static_cast<int>(cell.x);
    int y = static_cast<int>(cell.y);

    return x >= 0 && x < playerGrid.GetWidth() &&
        y >= 0 && y < playerGrid.GetHeight() &&
        !playerGrid.HasBeenShot(x, y);
}

// Генерация случайной клетки
Vector2 Game::GetRandomCell() {
    int x = GetRandomValue(0, playerGrid.GetWidth() - 1);
    int y = GetRandomValue(0, playerGrid.GetHeight() - 1);
    return { static_cast<float>(x), static_cast<float>(y) };
}

// Переключение хода
void Game::SwitchTurn() {
    playerTurn = !playerTurn; // Меняем текущего игрока

    if (playerTurn) {
        std::cout << "Ход переходит к игроку.\n";
    }
    else {
        std::cout << "Ход переходит к боту.\n";
        BotShoot(); // Бот начинает ход
    }
    DrawTurnIndicator(); // Обновляем индикатор хода после переключения
}

// Отрисовка кнопки старта
void Game::DrawStartButton() {
    Vector2 mousePoint = GetMousePosition();
    Color buttonColor = CheckCollisionPointRec(mousePoint, startButton) ? GRAY : LIGHTGRAY;

    DrawRectangleRec(startButton, buttonColor);
    DrawRectangleLinesEx(startButton, 3, BLACK);
    DrawText("START", startButton.x + 100, startButton.y + 25, 30, BLACK);

    if (CheckCollisionPointRec(mousePoint, startButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        bool allShipsPlaced = true;

        for (const Ship& ship : ships) {
            if (!playerGrid.IsShipOnGrid(ship)) {
                allShipsPlaced = false;
                break;
            }
        }

        if (allShipsPlaced) {
            isGameStarted = true;
        }
        else {
            DrawText("Place all ships to start!", screenWidth / 2 - 200, screenHeight / 2, 20, RED);
        }
    }
}

// Отрисовка индикатора хода
void Game::DrawTurnIndicator() const {
    const int padding = 10;
    const int boxWidth = 200;
    const int boxHeight = 60;

    int boxX = GetScreenWidth() - boxWidth - padding;
    int boxY = GetScreenHeight() - boxHeight - padding;

    Color boxColor = playerTurn ? GREEN : RED; // Зелёный для игрока, красный для бота
    Color textColor = WHITE;

    const char* turnText = playerTurn ? "Player's Turn" : "Bot's Turn";

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, boxColor);
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, BLACK);
    DrawText(turnText, boxX + 10, boxY + 15, 20, textColor);
}



// Основной цикл игры
void Game::Run() {
    while (!WindowShouldClose()) {
        HandleInput();
        Draw();
    }
}

// Отрисовка игры
void Game::Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    playerGrid.Draw(false);
    botGrid.Draw(true);

    for (const Ship& ship : ships) {
        ship.Draw(false);
    }

    if (!isGameStarted) {
        DrawStartButton();
    }
    else {
        DrawTurnIndicator();
    }

    EndDrawing();
}

void Game::AddSurroundingTargets(int x, int y) {
    // Если уже есть хотя бы одно попадание, определяем направление
    bool horizontal = false, vertical = false;

    // Проверяем соседние клетки из очереди
    for (const auto& target : botTargetQueue) {
        int tx = static_cast<int>(target.x);
        int ty = static_cast<int>(target.y);

        if (tx == x && abs(ty - y) == 1) {
            vertical = true;
        }
        else if (ty == y && abs(tx - x) == 1) {
            horizontal = true;
        }
    }

    const std::vector<Vector2> horizontalDirections = {
        {-1, 0}, {1, 0}  // Слева и справа
    };

    const std::vector<Vector2> verticalDirections = {
        {0, -1}, {0, 1} // Сверху и снизу
    };

    const std::vector<Vector2> allDirections = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0} // Все направления
    };

    // Если направление определено, используем только соответствующие направления
    const auto& directions = (horizontal) ? horizontalDirections :
        (vertical) ? verticalDirections :
        allDirections;

    for (const auto& dir : directions) {
        int nx = x + static_cast<int>(dir.x);
        int ny = y + static_cast<int>(dir.y);

        // Проверяем, чтобы клетка находилась внутри границ поля и была корректной
        if (!playerGrid.IsValidCell(nx, ny)) {
            continue; // Пропускаем клетки за пределами игрового поля
        }

        if (!playerGrid.HasBeenShot(nx, ny)) {
            // Проверяем, чтобы клетка не была уже добавлена в очередь
            bool alreadyQueued = false;
            for (const auto& queued : botTargetQueue) {
                if (static_cast<int>(queued.x) == nx && static_cast<int>(queued.y) == ny) {
                    alreadyQueued = true;
                    break;
                }
            }

            if (!alreadyQueued) {
                botTargetQueue.push_back({ static_cast<float>(nx), static_cast<float>(ny) });
            }
        }
    }
}

