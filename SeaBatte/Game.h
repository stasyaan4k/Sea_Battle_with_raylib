#ifndef GAME_H
#define GAME_H

#include <vector>
#include <functional>
#include <thread>
#include <chrono>
#include "Grid.h"
#include "Ship.h"
#include "raylib.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>

class Game {
public:
    Game();
    ~Game();

    void Run();              
    void HandleInput();      
    void Draw();             
    void Shoot(Vector2 cell);
    void BotShoot();         
    void SwitchTurn();    
    void InitMusic();
    void UnloadMusic();

private:

    const float screenWidth = 1920;
    const float screenHeight = 1080;
    
    Music backgroundMusic;
    Sound hitSound;
    Sound destroySound;
    Sound missSound;

    Grid playerGrid;
    Grid botGrid;
    std::vector<Ship> ships;    // ������� ������
    std::vector<Ship> botShips; // ������� ����
    Ship* draggedShip;          // ��������������� �������
    Rectangle startButton;      // ������ ������ ����

    bool isDragging;            // ��������� ����
    bool isGameStarted;
    bool playerTurn;
    bool botHuntingMode;

    Vector2 lastHitCell;                  // ��������� �������� ����� ����
    std::vector<Vector2> huntQueue;       // ������� ������ ��� ���� ���� � ������ "�����"
    std::vector<Vector2> botTargetQueue;  // ������� ������ ��� ���� ����

    // �������
    bool isBotShooting;         // ��� ��������� �������
    float actionDelay;          // ����� �������� ����� ����������

    void InitShips();                
    void DrawStartButton();          
    void DrawTurnIndicator() const;  
    void SetTimeout(std::function<void()> func, float seconds);
    Vector2 GetRandomCell();
    bool IsCellValid(Vector2 cell);
    void AddSurroundingTargets(int x, int y);
};

#endif // GAME_H
