#ifndef GRID_H
#define GRID_H

#include <vector>
#include "Ship.h"

struct CellState {
    bool isOccupied = false; // Основная зона, занята кораблём
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
    void Draw(bool highlightCells) const; // Отрисовка сетки
    bool CanPlaceShip(const Ship& ship) const; // Проверка возможности размещения корабля
    bool SnapShipToGrid(Ship& ship); // Привязка корабля к сетке
    void UpdateCellMap(const Ship& ship, bool isPlacing); // Обновление карты ячеек
    void RemoveShipFromMap(const Ship& ship); // Удаление корабля с карты
    bool PlaceShip(Ship& ship); // Размещение корабля
    void ResetGrid(); // Сброс сетки
    void PlaceRandomShipsForBot(std::vector<Ship>& botShips); // Случайное размещение кораблей для бота
    bool IsShipOnGrid(const Ship& ship) const;
    Vector2 GetCellUnderMouse() const;
    CellState& GetCellState(int x, int y);
    bool Shoot(int x, int y);          // Метод для обработки выстрела по клетке
    bool HasBeenShot(int x, int y);   // Проверка, стреляли ли уже в клетку
    bool IsValidCell(int x, int y) const;
    bool IsShipDestroyed(int x, int y);
    void BlockSurroundingCellsAndDraw(int x, int y);

    int GetWidth() const;
    int GetHeight() const;

private:

    int x, y;               // Координаты начала сетки
    int cellSize;           // Размер клетки
    int width, height;      // Размеры сетки
    std::vector<std::vector<CellState>> cellMap; // Карта клеток сетки
    std::vector<std::vector<bool>> shots;
    std::vector<Ship> ships;
};

#endif // GRID_H
