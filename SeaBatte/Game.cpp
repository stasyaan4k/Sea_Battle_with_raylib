#include "Game.h"


Game::Game()
    : playerGrid(200, 200, 40, 10, 10),
    botGrid(screenWidth - 200 - 400, 200, 40, 10, 10),
    draggedShip(nullptr), isDragging(false), isGameStarted(false),
    botHuntingMode(false), playerTurn(true), isBotShooting(false), actionDelay(0.0f) {
    InitWindow(screenWidth, screenHeight, "Sea Battle");
    startButton = { screenWidth / 2 - 150, screenHeight - 100, 300, 80 }; 
    SetTargetFPS(60);
    InitShips();
    std::srand(std::time(nullptr)); 
}

Game::~Game() {
    UnloadMusic();
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

//Управление
void Game::HandleInput() {
    Vector2 mousePosition = GetMousePosition();

    if (IsKeyPressed(KEY_ESCAPE)) {
        CloseWindow();
        return;
    }

    if (!isGameStarted) {
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
        return;
    }

    if (botGrid.Shoot(x, y)) {
        if (botGrid.IsShipDestroyed(x, y)) {
            PlaySound(destroySound);
            botGrid.BlockSurroundingCells(x, y);
        }
        else {
            PlaySound(hitSound);
        }
    }
    else {
        PlaySound(missSound);
        SwitchTurn(); 
    }
}

// Выстрел бота
void Game::BotShoot() {
    if (isBotShooting) {
        return;
    }

    isBotShooting = true;

    SetTimeout([this]() {
        Vector2 target = { -1, -1 }; 
        bool validShot = false;
        int x = -1, y = -1;

        if (!botTargetQueue.empty()) {
            target = botTargetQueue.back();
            botTargetQueue.pop_back();

            x = static_cast<int>(target.x);
            y = static_cast<int>(target.y);

            validShot = playerGrid.IsValidCell(x, y) && !playerGrid.HasBeenShot(x, y);
        }

        while (!validShot) {
            target = GetRandomCell();
            x = static_cast<int>(target.x);
            y = static_cast<int>(target.y);

            validShot = playerGrid.IsValidCell(x, y) && !playerGrid.HasBeenShot(x, y);
        }

        if (playerGrid.Shoot(x, y)) {
            playerGrid.MarkHit(x, y); 

            if (playerGrid.IsShipDestroyed(x, y)) {
                PlaySound(destroySound);
                botTargetQueue.clear(); 
                playerGrid.BlockSurroundingCells(x, y);
            }
            else {
                PlaySound(hitSound);
                AddSurroundingTargets(x, y);
            }

            SetTimeout([this]() {
                isBotShooting = false; 
                BotShoot(); 
            }, 2.0f);

        }
        else {
            PlaySound(missSound);
            isBotShooting = false;
            SwitchTurn(); 

        }

        }, 2.0f);
}

// Установка задержки
void Game::SetTimeout(std::function<void()> func, float seconds) {
    std::thread([func, seconds]() {
        std::this_thread::sleep_for(std::chrono::duration<float>(seconds));
        func();
        }).detach();
}

// Проверка корректности клетки
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
    playerTurn = !playerTurn;

    if (playerTurn) {
    }
    else {
        BotShoot(); 
    }
    DrawTurnIndicator(); 
}

//Звуки
void Game::InitMusic() {
    InitAudioDevice();

   backgroundMusic = LoadMusicStream("assets/sounds/background.ogg");
   hitSound = LoadSound("assets/sounds/hit.wav");
   destroySound = LoadSound("assets/sounds/destroy.wav");
   missSound = LoadSound("assets/sounds/miss.wav");
}

void Game::UnloadMusic() {
    UnloadMusicStream(backgroundMusic);
    UnloadSound(hitSound);
    UnloadSound(destroySound);
    UnloadSound(missSound);
    CloseAudioDevice(); // Завершение работы с аудио
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
    }
}

// Отрисовка индикатора хода
void Game::DrawTurnIndicator() const {
    const int padding = 10;
    const int boxWidth = 200;
    const int boxHeight = 60;

    int boxX = GetScreenWidth() - boxWidth - padding;
    int boxY = GetScreenHeight() - boxHeight - padding;

    Color boxColor = RAYWHITE;
    Color textColor = BLACK;

    const char* turnText = playerTurn ? "Player's Turn" : "Bot's Turn";

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, boxColor);
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, BLACK);
    DrawText(turnText, boxX + 10, boxY + 15, 20, textColor);
}

// Основной цикл игры
void Game::Run() {
    while (!WindowShouldClose()) {
        PlayMusicStream(backgroundMusic);
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

//Режим "охоты"
void Game::AddSurroundingTargets(int x, int y) {
 
    bool horizontal = false, vertical = false;

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
        {-1, 0}, {1, 0}  
    };

    const std::vector<Vector2> verticalDirections = {
        {0, -1}, {0, 1}
    };

    const std::vector<Vector2> allDirections = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0}
    };

    const auto& directions = (horizontal) ? horizontalDirections :
        (vertical) ? verticalDirections :
        allDirections;

    for (const auto& dir : directions) {
        int nx = x + static_cast<int>(dir.x);
        int ny = y + static_cast<int>(dir.y);

        if (!playerGrid.IsValidCell(nx, ny)) {
            continue;
        }

        if (!playerGrid.HasBeenShot(nx, ny)) {
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

