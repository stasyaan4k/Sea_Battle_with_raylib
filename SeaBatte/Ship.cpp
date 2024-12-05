#include "Ship.h"

// �����������
Ship::Ship(int length, float x, float y, float cellSize)
    : length(length), x(x), y(y), startX(x), startY(y), cellSize(cellSize), isHorizontal(true) {}



// ��������� �������
void Ship::Draw(bool isBotShip) const {
    Color color = isBotShip ? Fade(WHITE, 0.0f) : Fade(BLUE, 0.3f);

    // ������ �������� ������������� �������
    if (isHorizontal) {
        DrawRectangle(x, y, cellSize * length, cellSize, color);
    }
    else {
        DrawRectangle(x, y, cellSize, cellSize * length, color);
    }
}

// ��������� ������ ���������
void Ship::SetPosition(float x, float y) {
    this->x = x;
    this->y = y;
}

// ��������� �������� ���������
Vector2 Ship::GetPosition() const {
    return { x, y };
}

// ����� ��������� � ����������
void Ship::ResetPosition() {
    x = startX;
    y = startY;
    isHorizontal = true;
}

// ��������� ������ �������
float Ship::GetWidth() const {
    return isHorizontal ? length * cellSize : cellSize;
}

// ��������� ������ �������
float Ship::GetHeight() const {
    return isHorizontal ? cellSize : length * cellSize;
}

// ��������� ������ ������� (��� �������� �����������)
Rectangle Ship::GetBounds() const {
    return { x, y, GetWidth(), GetHeight() };
}

// �������� ���������� �������
bool Ship::IsHorizontal() const {
    return isHorizontal;
}

// ��������� ���������� (�������)
void Ship::Rotate() {
    isHorizontal = !isHorizontal;
}

// ������� ����� ������� ��� ���������
void Ship::MarkHit(int index) {
    if (index >= 0 && index < length) {
        hits[index] = true;
    }
}

// ��������� ������, ���������� �������
std::vector<Vector2> Ship::GetOccupiedCells(int cellSize, int gridX, int gridY) const {
    std::vector<Vector2> occupiedCells;

    int startX = static_cast<int>((x - gridX) / cellSize);
    int startY = static_cast<int>((y - gridY) / cellSize);

    if (isHorizontal) {
        for (int i = 0; i < length; ++i) {
            occupiedCells.push_back({ static_cast<float>(startX + i), static_cast<float>(startY) });
        }
    }
    else {
        for (int i = 0; i < length; ++i) {
            occupiedCells.push_back({ static_cast<float>(startX), static_cast<float>(startY + i) });
        }
    }

    return occupiedCells;
}

// ��������� ���������� X
float Ship::GetX() const {
    return x;
}

// ��������� ���������� Y
float Ship::GetY() const {
    return y;
}