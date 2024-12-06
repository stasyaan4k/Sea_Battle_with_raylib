#ifndef GRID_H
#define GRID_H

#include <vector>
#include "Ship.h"
#include <iostream>
#include <cstdlib>
#include <ctime> 

struct CellState {
    bool isOccupied = false;
    bool isHit = false;
    bool isMiss = false;
    bool isBlocked = false;
    int bufferCount = 0; 
    float cellX, cellY;
    int size = 40;

    CellState() = default;
};

class Grid {
public:
    Grid(int x, int y, int cellSize, int width, int height);

    void MarkHit(int x, int y);
    void Draw(bool highlightCells) const; 
    bool CanPlaceShip(const Ship& ship) const; 
    bool SnapShipToGrid(Ship& ship);
    void UpdateCellMap(const Ship& ship, bool isPlacing); 
    void RemoveShipFromMap(const Ship& ship); 
    bool PlaceShip(Ship& ship); 
    void ResetGrid();
    void PlaceRandomShipsForBot(std::vector<Ship>& botShips); 
    bool IsShipOnGrid(const Ship& ship) const;
    Vector2 GetCellUnderMouse() const;
    CellState& GetCellState(int x, int y);
    bool Shoot(int x, int y);         
    bool HasBeenShot(int x, int y);
    bool IsValidCell(int x, int y) const;
    bool IsShipDestroyed(int x, int y);
    void BlockSurroundingCells(int x, int y);

    int GetWidth() const;
    int GetHeight() const;

private:

    int x, y;               // Координаты начала сетки
    int cellSize;           // Размер клетки
    int width, height;      // Размеры сетки
    std::vector<std::vector<CellState>> cellMap; // Карта клеток сетки
    std::vector<std::vector<bool>> shots; //Карта выстрелов
    std::vector<Ship> ships; //Вектор кораблей
};

#endif // GRID_H
