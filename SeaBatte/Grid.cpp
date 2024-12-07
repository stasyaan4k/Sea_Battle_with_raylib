#include "Grid.h"

Grid::Grid(int x, int y, int cellSize, int width, int height)
    : x(x), y(y), cellSize(cellSize), width(width), height(height) {
    cellMap.resize(height, std::vector<CellState>(width)), shots.resize(height, std::vector<bool>(width, false));
    ;
}

//Отрисовка сетки
void Grid::Draw(bool highlightCells) const {
    
    const char* letters = "ABCDEFGHIJ";
    for (int j = 0; j < width; j++) {
        char letter[2] = { letters[j], '\0' };
        Vector2 textPosition = {
            static_cast<float>(x + j * cellSize + cellSize / 2 - MeasureText(letter, 20) / 2),
            static_cast<float>(y - cellSize / 2 - 10) 
        };
        DrawText(letter, textPosition.x, textPosition.y, 20, BLACK);
    }

    for (int i = 0; i < height; i++) {
        char number[3];
        snprintf(number, sizeof(number), "%d", i + 1); 
        Vector2 textPosition = {
            static_cast<float>(x - cellSize / 2 - 20), 
            static_cast<float>(y + i * cellSize + cellSize / 2 - 10)
        };
        DrawText(number, textPosition.x, textPosition.y, 20, BLACK);
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            Rectangle cell = {
                static_cast<float>(x + j * cellSize),
                static_cast<float>(y + i * cellSize),
                static_cast<float>(cellSize),
                static_cast<float>(cellSize)
            };

            if (highlightCells && CheckCollisionPointRec(GetMousePosition(), cell)) {
                DrawRectangleRec(cell, Fade(SKYBLUE, 0.5f));
            }

            DrawRectangleLinesEx(cell, 1.5f, DARKGRAY);

            if (cellMap[i][j].isHit) {
                float padding = cellSize * 0.1f; 
                Vector2 start1 = { cell.x + padding, cell.y + padding };
                Vector2 end1 = { cell.x + cell.width - padding, cell.y + cell.height - padding };
                Vector2 start2 = { cell.x + padding, cell.y + cell.height - padding };
                Vector2 end2 = { cell.x + cell.width - padding, cell.y + padding };
                DrawLineEx(start1, end1, 3.0f, RED);
                DrawLineEx(start2, end2, 3.0f, RED);
            }
            else if (cellMap[i][j].isMiss || cellMap[i][j].isBlocked) {
                DrawCircle(cell.x + cell.width / 2, cell.y + cell.height / 2, cellSize * 0.1f, BLUE);
            }
        }
    }
}

//Геттеры
int Grid::GetWidth() const {
    return width;
}

int Grid::GetHeight() const {
    return height;
}

//Проверка возможности размещения корабля
bool Grid::CanPlaceShip(const Ship& ship) const {
    auto occupiedCells = ship.GetOccupiedCells(cellSize, x, y);

    for (const auto& cell : occupiedCells) {
        int gridX = cell.x;
        int gridY = cell.y;

        if (gridX < 0 || gridX >= width || gridY < 0 || gridY >= height || cellMap[gridY][gridX].isOccupied) {
            return false;
        }

        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int bufferX = gridX + dx;
                int bufferY = gridY + dy;

                if (bufferX >= 0 && bufferX < width && bufferY >= 0 && bufferY < height) {
                    if (cellMap[bufferY][bufferX].isOccupied) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

//Привязка корабля к сетке
bool Grid::SnapShipToGrid(Ship& ship) {
    int gridX = (ship.GetPosition().x - x + cellSize / 2) / cellSize;
    int gridY = (ship.GetPosition().y - y + cellSize / 2) / cellSize;

    if (gridX < 0 || gridX >= width || gridY < 0 || gridY >= height) {
        return false;
    }

    ship.SetPosition(x + gridX * cellSize, y + gridY * cellSize);

    if (!CanPlaceShip(ship)) {
        return false;
    }

    UpdateCellMap(ship, true);
    return true;
}

//ОБновление карты ячеек
void Grid::UpdateCellMap(const Ship& ship, bool isPlacing) {
    auto occupiedCells = ship.GetOccupiedCells(cellSize, x, y);

    for (const auto& cell : occupiedCells) {
        int gridX = cell.x;
        int gridY = cell.y;

        if (gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
            cellMap[gridY][gridX].isOccupied = isPlacing;
        }
    }

    if (isPlacing) {
        for (const auto& cell : occupiedCells) {
            int gridX = cell.x;
            int gridY = cell.y;

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int bufferX = gridX + dx;
                    int bufferY = gridY + dy;

                    if (bufferX >= 0 && bufferX < width && bufferY >= 0 && bufferY < height) {
                        cellMap[bufferY][bufferX].bufferCount++;
                    }
                }
            }
        }
    }
    else {
        for (const auto& cell : occupiedCells) {
            int gridX = cell.x;
            int gridY = cell.y;

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int bufferX = gridX + dx;
                    int bufferY = gridY + dy;

                    if (bufferX >= 0 && bufferX < width && bufferY >= 0 && bufferY < height) {
                        cellMap[bufferY][bufferX].bufferCount--;
                    }
                }
            }
        }
    }
}

//Удаление корабля с карты
void Grid::RemoveShipFromMap(const Ship& ship) {
    auto occupiedCells = ship.GetOccupiedCells(cellSize, x, y);

    for (const auto& cell : occupiedCells) {
        int gridX = cell.x;
        int gridY = cell.y;

        if (gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
            cellMap[gridY][gridX].isOccupied = false;
        }
    }

    for (const auto& cell : occupiedCells) {
        int gridX = cell.x;
        int gridY = cell.y;

        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int bufferX = gridX + dx;
                int bufferY = gridY + dy;

                if (bufferX >= 0 && bufferX < width && bufferY >= 0 && bufferY < height) {
                    cellMap[bufferY][bufferX].bufferCount--;
                }
            }
        }
    }
}

//Размещение корабля
bool Grid::PlaceShip(Ship& ship) {
    if (CanPlaceShip(ship)) {
        SnapShipToGrid(ship);
        return true;
    }
    return false;
}

//Сброс карты
void Grid::ResetGrid() {
    for (auto& row : cellMap) {
        for (auto& cell : row) {
            cell.isHit = false;
            cell.isMiss = false;
            cell.isBlocked = false;
            cell.isOccupied = false;
            cell.bufferCount = 0;
        }
    }

    ships.clear();
}

//Размещение кораблей бота случайным образом
void Grid::PlaceRandomShipsForBot(std::vector<Ship>& botShips) {
    srand(time(0));

    for (auto& ship : botShips) {
        bool placed = false;

        while (!placed) {
            int orientation = rand() % 2;
            int startX, startY;

            if (orientation == 0) {
                startX = rand() % static_cast<int>(width - ship.GetWidth() + 1);
                startY = rand() % static_cast<int>(height);
            }
            else {
                startX = rand() % static_cast<int>(width);
                startY = rand() % static_cast<int>(height - ship.GetHeight() + 1);
            }
            Ship tempShip = ship;
            tempShip.SetPosition(x + startX * cellSize, y + startY * cellSize);

            if (orientation == 1) {
                tempShip.Rotate();
            }

            if (CanPlaceShip(tempShip)) {
                SnapShipToGrid(tempShip);
                UpdateCellMap(tempShip, true);
                ship = tempShip;
                placed = true; 
            }
        }
    }
}

//Проверка корабля на нахождение внутри сетки
bool Grid::IsShipOnGrid(const Ship& ship) const {
    float shipX = ship.GetX();
    float shipY = ship.GetY();
    float shipWidth = ship.GetWidth();
    float shipHeight = ship.GetHeight();

    return (shipX >= x && shipY >= y &&
        shipX + shipWidth <= x + cellSize * width &&
        shipY + shipHeight <= y + cellSize * height);
}

//Получение ячейки под курсором мыши
Vector2 Grid::GetCellUnderMouse() const {
    Vector2 mousePos = GetMousePosition();

    if (mousePos.x >= x && mousePos.x < x + width * cellSize &&
        mousePos.y >= y && mousePos.y < y + height * cellSize) {
        int gridX = (mousePos.x - x) / cellSize;
        int gridY = (mousePos.y - y) / cellSize;
        return { static_cast<float>(gridX), static_cast<float>(gridY) };
    }

    return { -1, -1 };
}

//Геттер для состояния ячейки
CellState& Grid::GetCellState(int x, int y) {
    return cellMap[y][x];
}

//Выстрел
bool Grid::Shoot(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false; 
    }
    if (shots[y][x]) {
        return false; 
    }

    shots[y][x] = true;

    if (cellMap[y][x].isOccupied) {
        cellMap[y][x].isHit = true;
        return true;
    }
    else {
        cellMap[y][x].isMiss = true; 
        return false;
    }
}

//Проверка для предотвращения повторного выстрела по клетке
bool Grid::HasBeenShot(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false;
    }

    const CellState& cell = GetCellState(x, y);
    return cell.isHit || cell.isMiss;
}

//Метод для обозначения клетки как обстрелянной
void Grid::MarkHit(int x, int y) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        shots[x][y] = true;
    }
}

//Проверка корректности клетки
bool Grid::IsValidCell(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false; 
    }
    return true;
}

//Проверка корабля на полное уничтожение
bool Grid::IsShipDestroyed(int x, int y) {
    if (!cellMap[y][x].isOccupied) {
        return false;
    }

    int startX = x, startY = y, endX = x, endY = y;

    while (startX > 0 && cellMap[y][startX - 1].isOccupied) {
        startX--;
    }

    while (endX < width - 1 && cellMap[y][endX + 1].isOccupied) {
        endX++;
    }

    while (startY > 0 && cellMap[startY - 1][x].isOccupied) {
        startY--;
    }

    while (endY < height - 1 && cellMap[endY + 1][x].isOccupied) {
        endY++;
    }

    for (int i = startY; i <= endY; i++) {
        for (int j = startX; j <= endX; j++) {
            if (cellMap[i][j].isOccupied && !cellMap[i][j].isHit) {
                return false; 
            }
        }
    }
    return true;
}

//Метод блокировки клеток вокруг уничтоженного корабля
void Grid::BlockSurroundingCells(int x, int y) {
    int startX = x, startY = y, endX = x, endY = y;

    // Поиск границ корабля
    while (startX > 0 && cellMap[y][startX - 1].isOccupied) {
        startX--;
    }
    while (endX < width - 1 && cellMap[y][endX + 1].isOccupied) {
        endX++;
    }
    while (startY > 0 && cellMap[startY - 1][x].isOccupied) {
        startY--;
    }
    while (endY < height - 1 && cellMap[endY + 1][x].isOccupied) {
        endY++;
    }

    for (int i = startY - 1; i <= endY + 1; i++) {
        for (int j = startX - 1; j <= endX + 1; j++) {
            if (i >= 0 && i < height && j >= 0 && j < width && !cellMap[i][j].isOccupied) {
                cellMap[i][j].isBlocked = true; 
                cellMap[i][j].isMiss = true;
            }
        }
    }
}

bool Grid::AreAllShipsDestroyed() const {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (cellMap[y][x].isOccupied && !cellMap[y][x].isHit) {
                return false;
            }
        }
    }
    return true;
}

