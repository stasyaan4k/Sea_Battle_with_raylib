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

enum GameState { MENU, GAME_PROCESS, EXIT };

class Game {
public:
    Game();
    ~Game();

    void Run();              
    void HandleInput();      
    void Draw();             
    void PlayerShoot(Vector2 cell);
    void BotShoot();         
    void SwitchTurn();    
    void InitMusic();
    void UnloadMusic();
   

private:
    const float screenWidth = 1800;
    const float screenHeight = 1000;

    bool isSoundOn = true;

    GameState state;

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

    bool isGameOver = false ;
    std::string winner = "";

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
    void CheckVictory();
    void DrawVictoryScreen();
    void DrawMenu();
    void Reset();
    void DrawFrame();
};

#endif // GAME_H
