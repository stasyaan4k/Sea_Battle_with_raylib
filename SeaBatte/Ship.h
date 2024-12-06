#ifndef SHIP_H
#define SHIP_H

#include <vector>
#include "raylib.h"

class Ship {
public:
   
    Ship(int length, float x, float y, float cellSize);

    
    void Draw(bool isBotShip) const; 
    void SetPosition(float x, float y);  
    Vector2 GetPosition() const;
    void ResetPosition();
    void Rotate();
    float GetWidth() const;
    float GetHeight() const;
    Rectangle GetBounds() const;
    bool IsHorizontal() const;
    void MarkHit(int index); 
    std::vector<Vector2> GetOccupiedCells(int cellSize, int gridX, int gridY) const; 

    float GetX() const;
    float GetY() const;

private:
    int length;                       // Длина корабля в клетках
    float x, y;                       // Текущее положение (верхний левый угол)
    float startX, startY;             // Начальное положение корабля
    float cellSize;                   // Размер одной клетки
    bool isHorizontal;                // Ориентация корабля

    std::vector<bool> hits;
};

#endif