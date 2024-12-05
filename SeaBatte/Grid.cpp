#include "Grid.h"
#include <iostream>
#include <cstdlib>  // ��� rand()
#include <ctime>    // ��� srand()

Grid::Grid(int x, int y, int cellSize, int width, int height)
    : x(x), y(y), cellSize(cellSize), width(width), height(height) {
    cellMap.resize(height, std::vector<CellState>(width)), shots.resize(height, std::vector<bool>(width, false));
    ;
}

void Grid::Draw(bool highlightCells) const {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            Rectangle cell = {
                static_cast<float>(x + j * cellSize),
                static_cast<float>(y + i * cellSize),
                static_cast<float>(cellSize),
                static_cast<float>(cellSize)
            };

            // ��������� ������ ��� ������
            if (highlightCells && CheckCollisionPointRec(GetMousePosition(), cell)) {
                DrawRectangleRec(cell, Fade(SKYBLUE, 0.5f));
            }

            // ������ ������ ������
            DrawRectangleLinesEx(cell, 1.5f, DARKGRAY);

            // ������ ���������, ������� � �������
            if (cellMap[i][j].isHit) {
                // ������� ��� ���������
                float padding = cellSize * 0.1f; // ������
                Vector2 start1 = { cell.x + padding, cell.y + padding };
                Vector2 end1 = { cell.x + cell.width - padding, cell.y + cell.height - padding };
                Vector2 start2 = { cell.x + padding, cell.y + cell.height - padding };
                Vector2 end2 = { cell.x + cell.width - padding, cell.y + padding };
                DrawLineEx(start1, end1, 3.0f, RED);
                DrawLineEx(start2, end2, 3.0f, RED);
            }
            else if (cellMap[i][j].isMiss) {
                // ����� ��� �������
                DrawCircle(cell.x + cell.width / 2, cell.y + cell.height / 2, cellSize * 0.1f, BLUE);
            }
        }
    }
}

//�������
int Grid::GetWidth() const {
    return width;
}

int Grid::GetHeight() const {
    return height;
}

bool Grid::CanPlaceShip(const Ship& ship) const {
    auto occupiedCells = ship.GetOccupiedCells(cellSize, x, y);

    for (const auto& cell : occupiedCells) {
        int gridX = cell.x;
        int gridY = cell.y;

        if (gridX < 0 || gridX >= width || gridY < 0 || gridY >= height || cellMap[gridY][gridX].isOccupied) {
            return false;
        }

        // ��������� �������� ����
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

bool Grid::PlaceShip(Ship& ship) {
    if (CanPlaceShip(ship)) {
        SnapShipToGrid(ship);
        return true;
    }
    return false;
}

void Grid::ResetGrid() {
    for (auto& row : cellMap) {
        for (auto& cell : row) {
            cell.isOccupied = false;
            cell.bufferCount = 0;
        }
    }
}

void Grid::PlaceRandomShipsForBot(std::vector<Ship>& botShips) {
    srand(time(0)); // ������������� ���������� ��������� �����

    for (auto& ship : botShips) {
        bool placed = false;

        while (!placed) {
            // �������� �������� ��������� ���������� � ����������
            int orientation = rand() % 2;  // 0 - ��������������, 1 - ������������
            int startX, startY;

            if (orientation == 0) {
                // �������������� ����������: �������� ��������� ������� ���,
                // ����� ������� �� ������� �� ������ �������
                startX = rand() % static_cast<int>(width - ship.GetWidth() + 1);
                startY = rand() % static_cast<int>(height);

            }
            else {
                // ������������ ����������: �������� ��������� ������� ���,
                // ����� ������� �� ������� �� ������ �������
                startX = rand() % static_cast<int>(width);
                startY = rand() % static_cast<int>(height - ship.GetHeight() + 1);

            }

            // ������������� ��������� ������� �������
            Ship tempShip = ship;
            tempShip.SetPosition(x + startX * cellSize, y + startY * cellSize);

            // ���� ���������� ������������, ������������ �������
            if (orientation == 1) {
                tempShip.Rotate(); // ������: RotateVertically() ������ ����������
            }

            // ���������, ����� �� ���������� �������
            if (CanPlaceShip(tempShip)) {
                // ���� ����� ��������, ����������� ������� � �����
                SnapShipToGrid(tempShip);

                // ��������� ����� ������� ������
                UpdateCellMap(tempShip, true);

                // ��������� ������������ � ������ ��������
                ship = tempShip;

                placed = true; // ��������� ������� ��� ����� �������
            }
        }
    }
}

bool Grid::IsShipOnGrid(const Ship& ship) const {
    // �������� ���������� � ������ �������
    float shipX = ship.GetX();
    float shipY = ship.GetY();
    float shipWidth = ship.GetWidth();
    float shipHeight = ship.GetHeight();

    // ���������, ��������� �� ������� ��������� � �������� �����
    return (shipX >= x && shipY >= y &&
        shipX + shipWidth <= x + cellSize * width &&
        shipY + shipHeight <= y + cellSize * height);
}


Vector2 Grid::GetCellUnderMouse() const {
    Vector2 mousePos = GetMousePosition();

    // ���������, ��������� �� ���� ������ ������ �����
    if (mousePos.x >= x && mousePos.x < x + width * cellSize &&
        mousePos.y >= y && mousePos.y < y + height * cellSize) {
        int gridX = (mousePos.x - x) / cellSize; // ������������ ������ ������ �� X
        int gridY = (mousePos.y - y) / cellSize; // ������������ ������ ������ �� Y
        return { static_cast<float>(gridX), static_cast<float>(gridY) };
    }

    return { -1, -1 }; // ���� ���� ��� ����, ���������� -1, -1
}


CellState& Grid::GetCellState(int x, int y) {
    return cellMap[y][x];
}

bool Grid::Shoot(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false; // �������� �������
    }
    if (shots[y][x]) {
        return false; // ��� �������� ����
    }

    shots[y][x] = true;

    if (cellMap[y][x].isOccupied) {
        cellMap[y][x].isHit = true; // ���������
        return true;
    }
    else {
        cellMap[y][x].isMiss = true; // ������
        return false;
    }
}

bool Grid::HasBeenShot(int x, int y) {
    // ���������, �������� �� ��� � ��������� ������
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false;
    }

    const CellState& cell = GetCellState(x, y);
    return cell.isHit || cell.isMiss;
}


void Grid::MarkHit(int x, int y) {
    // ��������� ������� ��������� � ������ ���������
    if (x >= 0 && x < width && y >= 0 && y < height) {
        shots[x][y] = true; // ���������� ������ ��� ������������
    }
}

bool Grid::IsValidCell(int x, int y) const {
    // ���������, ��� ���������� ��������� � �������� ����
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false; // ������ ��� ������ �����
    }

    return true; // ������ � �������� ����
}

bool Grid::IsShipDestroyed(int x, int y) {
    // ���� ������ �� ������ ��������, ���������� false
    if (!cellMap[y][x].isOccupied) {
        return false;
    }

    // �������� � ������� ������ � ���� ������� �������
    int startX = x, startY = y, endX = x, endY = y;

    // ���� ������� ������� �� �����������
    while (startX > 0 && cellMap[y][startX - 1].isOccupied) {
        startX--;
    }
    while (endX < width - 1 && cellMap[y][endX + 1].isOccupied) {
        endX++;
    }

    // ���� ������� ������� �� ���������
    while (startY > 0 && cellMap[startY - 1][x].isOccupied) {
        startY--;
    }
    while (endY < height - 1 && cellMap[endY + 1][x].isOccupied) {
        endY++;
    }

    // ��������� ��� ������ ������ ������ �������
    for (int i = startY; i <= endY; i++) {
        for (int j = startX; j <= endX; j++) {
            if (cellMap[i][j].isOccupied && !cellMap[i][j].isHit) {
                return false; // ���� ���� ���� ������� ������ �� ����������, ������� �� ���������
            }
        }
    }

    // ���� ��� ������� ������ ����������, ������� ���������
    return true;
}