#include "Game.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>

// �����������
Game::Game()
    : playerGrid(200, 200, 40, 10, 10),
    botGrid(screenWidth - 200 - 400, 200, 40, 10, 10),
    draggedShip(nullptr), isDragging(false), isGameStarted(false),
    botHuntingMode(false), playerTurn(true), isBotShooting(false), actionDelay(0.0f) {
    InitWindow(screenWidth, screenHeight, "Sea Battle");
    startButton = { screenWidth / 2 - 150, screenHeight - 100, 300, 80 }; // ������ ������
    SetTargetFPS(60);
    InitShips();
    std::srand(std::time(nullptr)); // ������������� ���������� ��������� �����
}

// ����������
Game::~Game() {
    CloseWindow();
}

// ������������� ��������
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

// ��������� �����
void Game::HandleInput() {
    Vector2 mousePosition = GetMousePosition();

    if (IsKeyPressed(KEY_ESCAPE)) {
        CloseWindow();
        return;
    }

    if (!isGameStarted) {
        // �������������� ������� ������
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
        // �������� ������
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

// ������� ������
void Game::Shoot(Vector2 cell) {
    int x = static_cast<int>(cell.x);
    int y = static_cast<int>(cell.y);

    // ��������, ���� �� ������ ��� ����������
    if (botGrid.HasBeenShot(x, y)) {
        std::cout << "����� ������ ��� ������������ ������: (" << x << ", " << y << ").\n";
        return;
    }

    // ���� ������� �����
    if (botGrid.Shoot(x, y)) {
        std::cout << "����� ����� �� ������ (" << x << ", " << y << ").\n";

        // ���������, ��������� �� �������
        if (botGrid.IsShipDestroyed(x, y)) {
            std::cout << "������� ��������� �� ������ (" << x << ", " << y << ").\n";

            // ��������� � ������������ ����� ������ �������
            botGrid.BlockSurroundingCellsAndDraw(x, y);
        }
    }
    // ���� ������� ����
    else {
        std::cout << "����� �����������! �������� ���� ����.\n";
        SwitchTurn(); // ������� ��� ����
    }
}

// ������� ����
void Game::BotShoot() {
    if (isBotShooting) {
        std::cout << "��� ��� ��������. ��� ����������...\n";
        return;
    }

    isBotShooting = true;
    std::cout << "��� ����� ���.\n";

    SetTimeout([this]() {
        Vector2 target = { -1, -1 }; // ������������� target
        bool validShot = false;
        int x = -1, y = -1;

        // ���� ���� ���������� � �������, ���� ��
        if (!botTargetQueue.empty()) {
            target = botTargetQueue.back();
            botTargetQueue.pop_back();

            x = static_cast<int>(target.x);
            y = static_cast<int>(target.y);

            validShot = playerGrid.IsValidCell(x, y) && !playerGrid.HasBeenShot(x, y);
        }

        // ���� ������� ����� ��� ���������� �� ������� �����������, ���� ��������� ������
        while (!validShot) {
            target = GetRandomCell();
            x = static_cast<int>(target.x);
            y = static_cast<int>(target.y);

            validShot = playerGrid.IsValidCell(x, y) && !playerGrid.HasBeenShot(x, y);
        }

        // ��������� �������
        if (playerGrid.Shoot(x, y)) {
            std::cout << "��� ����� �� ������ (" << x << ", " << y << ").\n";
            playerGrid.MarkHit(x, y); // �������� ���������

            if (playerGrid.IsShipDestroyed(x, y)) {
                std::cout << "������� ���������!\n";
                botTargetQueue.clear(); // ������� �������
                playerGrid.BlockSurroundingCellsAndDraw(x, y);
            }
            else {
                AddSurroundingTargets(x, y); // ��������� �������� ������ ��� ���������� ����
            }

            // ����� ���������, ��� �������� ����� � ���������
            SetTimeout([this]() {
                isBotShooting = false; // ��������� ������� ���
                BotShoot(); // ��� ���������� ���
                }, 2.0f);

        }
        else {
            // ���� �����������
            std::cout << "��� ����������� � ������ (" << x << ", " << y << ").\n";
            isBotShooting = false; // ��������� ���

            // �������� ���� ������ ����� ������� � ���������

            std::cout << "��� �������� ���. �������� ���� ������.\n";
            SwitchTurn(); // ������� ��� ������

        }

        }, 2.0f); // �������� ����� ������ ���������
}

// ��������� ��������
void Game::SetTimeout(std::function<void()> func, float seconds) {
    std::cout << "SetTimeout: ��������� ������ �� " << seconds << " ������.\n";
    std::thread([func, seconds]() {
        std::this_thread::sleep_for(std::chrono::duration<float>(seconds));
        std::cout << "SetTimeout: ������ ��������, ��������� �������.\n";
        func();
        }).detach();
}


// �������� ���������� ������
bool Game::IsCellValid(Vector2 cell) {
    int x = static_cast<int>(cell.x);
    int y = static_cast<int>(cell.y);

    return x >= 0 && x < playerGrid.GetWidth() &&
        y >= 0 && y < playerGrid.GetHeight() &&
        !playerGrid.HasBeenShot(x, y);
}

// ��������� ��������� ������
Vector2 Game::GetRandomCell() {
    int x = GetRandomValue(0, playerGrid.GetWidth() - 1);
    int y = GetRandomValue(0, playerGrid.GetHeight() - 1);
    return { static_cast<float>(x), static_cast<float>(y) };
}

// ������������ ����
void Game::SwitchTurn() {
    playerTurn = !playerTurn; // ������ �������� ������

    if (playerTurn) {
        std::cout << "��� ��������� � ������.\n";
    }
    else {
        std::cout << "��� ��������� � ����.\n";
        BotShoot(); // ��� �������� ���
    }
    DrawTurnIndicator(); // ��������� ��������� ���� ����� ������������
}

// ��������� ������ ������
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

// ��������� ���������� ����
void Game::DrawTurnIndicator() const {
    const int padding = 10;
    const int boxWidth = 200;
    const int boxHeight = 60;

    int boxX = GetScreenWidth() - boxWidth - padding;
    int boxY = GetScreenHeight() - boxHeight - padding;

    Color boxColor = WHITE; // ������ ��� ������, ������� ��� ����
    Color textColor = BLACK;

    const char* turnText = playerTurn ? "Player's Turn" : "Bot's Turn";

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, boxColor);
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, BLACK);
    DrawText(turnText, boxX + 10, boxY + 15, 20, textColor);
}



// �������� ���� ����
void Game::Run() {
    while (!WindowShouldClose()) {
        HandleInput();
        Draw();
    }
}

// ��������� ����
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
    // ���� ��� ���� ���� �� ���� ���������, ���������� �����������
    bool horizontal = false, vertical = false;

    // ��������� �������� ������ �� �������
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
        {-1, 0}, {1, 0}  // ����� � ������
    };

    const std::vector<Vector2> verticalDirections = {
        {0, -1}, {0, 1} // ������ � �����
    };

    const std::vector<Vector2> allDirections = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0} // ��� �����������
    };

    // ���� ����������� ����������, ���������� ������ ��������������� �����������
    const auto& directions = (horizontal) ? horizontalDirections :
        (vertical) ? verticalDirections :
        allDirections;

    for (const auto& dir : directions) {
        int nx = x + static_cast<int>(dir.x);
        int ny = y + static_cast<int>(dir.y);

        // ���������, ����� ������ ���������� ������ ������ ���� � ���� ����������
        if (!playerGrid.IsValidCell(nx, ny)) {
            continue; // ���������� ������ �� ��������� �������� ����
        }

        if (!playerGrid.HasBeenShot(nx, ny)) {
            // ���������, ����� ������ �� ���� ��� ��������� � �������
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

