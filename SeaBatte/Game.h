#ifndef GAME_H
#define GAME_H

#include <vector>
#include <functional>
#include <thread>
#include <chrono>
#include "Grid.h"
#include "Ship.h"
#include "raylib.h"

class Game {
public:
    Game();
    ~Game();

    void Run();                // Основной игровой цикл
    void HandleInput();        // Обработка ввода
    void Draw();               // Отрисовка экрана
    void Shoot(Vector2 cell);  // Выстрел игрока
    void BotShoot();           // Выстрел бота
    void SwitchTurn();         // Переключение хода

private:
    // Константы экрана
    const float screenWidth = 1920;
    const float screenHeight = 1080;

    // Игровые поля и параметры
    Grid playerGrid;
    Grid botGrid;
    std::vector<Ship> ships;    // Корабли игрока
    std::vector<Ship> botShips; // Корабли бота
    Ship* draggedShip;          // Перетаскиваемый корабль
    Rectangle startButton;      // Кнопка начала игры

    // Состояния игры
    bool isDragging;
    bool isGameStarted;
    bool playerTurn;
    bool botHuntingMode;

    Vector2 lastHitCell;        // Последняя успешная атака бота
    std::vector<Vector2> huntQueue; // Очередь клеток для атак бота
    std::vector<Vector2> botTargetQueue;

    // Таймеры
    bool isBotShooting;         // Бот выполняет выстрел
    float actionDelay;          // Общая задержка между действиями

    // Приватные методы
    void InitShips();                // Инициализация кораблей
    void DrawStartButton();          // Отрисовка кнопки старта
    void DrawTurnIndicator() const;  // Отрисовка индикатора текущего хода
    void SetTimeout(std::function<void()> func, float seconds); // Таймер
    Vector2 GetRandomCell();         // Получение случайной клетки
    bool IsCellValid(Vector2 cell);  // Проверка валидности клетки
    void AddSurroundingTargets(int x, int y);
};

#endif // GAME_H
