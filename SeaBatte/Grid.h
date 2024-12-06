#ifndef GRID_H
#define GRID_H

#include <vector>
#include "Ship.h"

struct CellState {
    bool isOccupied = false; // �������� ����, ������ �������
    bool isHit = false;
    bool isMiss = false;
    int bufferCount = 0;
    bool isBlocked = false;
    float cellX, cellY;
    int size = 40;

    CellState() = default;
};

class Grid {
public:
    Grid(int x, int y, int cellSize, int width, int height);

    void MarkHit(int x, int y);
    void Draw(bool highlightCells) const; // ��������� �����
    bool CanPlaceShip(const Ship& ship) const; // �������� ����������� ���������� �������
    bool SnapShipToGrid(Ship& ship); // �������� ������� � �����
    void UpdateCellMap(const Ship& ship, bool isPlacing); // ���������� ����� �����
    void RemoveShipFromMap(const Ship& ship); // �������� ������� � �����
    bool PlaceShip(Ship& ship); // ���������� �������
    void ResetGrid(); // ����� �����
    void PlaceRandomShipsForBot(std::vector<Ship>& botShips); // ��������� ���������� �������� ��� ����
    bool IsShipOnGrid(const Ship& ship) const;
    Vector2 GetCellUnderMouse() const;
    CellState& GetCellState(int x, int y);
    bool Shoot(int x, int y);          // ����� ��� ��������� �������� �� ������
    bool HasBeenShot(int x, int y);   // ��������, �������� �� ��� � ������
    bool IsValidCell(int x, int y) const;
    bool IsShipDestroyed(int x, int y);
    void BlockSurroundingCellsAndDraw(int x, int y);

    int GetWidth() const;
    int GetHeight() const;

private:

    int x, y;               // ���������� ������ �����
    int cellSize;           // ������ ������
    int width, height;      // ������� �����
    std::vector<std::vector<CellState>> cellMap; // ����� ������ �����
    std::vector<std::vector<bool>> shots;
    std::vector<Ship> ships;
};

#endif // GRID_H
