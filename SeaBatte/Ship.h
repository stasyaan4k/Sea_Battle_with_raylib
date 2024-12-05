#ifndef SHIP_H
#define SHIP_H

#include <vector>
#include "raylib.h"

class Ship {
public:
    // Конструктор
    Ship(int length, float x, float y, float cellSize);

    // Отрисовка корабля, включая отображение попаданий
    void Draw(bool isBotShip) const;

    // Методы управления положением корабля
    void SetPosition(float x, float y);
    Vector2 GetPosition() const;
    void ResetPosition();
    void Rotate();

    // Получение размеров и границ корабля
    float GetWidth() const;
    float GetHeight() const;
    Rectangle GetBounds() const;

    // Проверка ориентации
    bool IsHorizontal() const;

    // Управление попаданиями
    void MarkHit(int index); // Отмечает часть корабля как поражённую

    // Получение клеток, занимаемых кораблём
    std::vector<Vector2> GetOccupiedCells(int cellSize, int gridX, int gridY) const;

    // Геттеры
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

#endif // SHIP_H
#pragma once
