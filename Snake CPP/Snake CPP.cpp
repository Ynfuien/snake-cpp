#include <iostream>
#include <raylib.h>
#include <list>
#include <chrono>
#include <sstream>

using namespace std;



// Configuration variables
const int _gridSize = 32;
const int _snakeSize = 5;
const int _scale = 20;
const int _size = _gridSize * _scale + _gridSize - 1;
const int _tickTime = 100;



// Game classes
class Pixel
{
public:
    int x;
    int y;

    Pixel() { }

    Pixel(int x, int y)
    {
        this->x = x > _gridSize ? _gridSize : x;
        this->y = y > _gridSize ? _gridSize : y;
    };

    void render(Color color)
    {
        DrawRectangle(x * _scale + x, y * _scale + y, _scale, _scale, color);
    };

    bool equals(Pixel pixel)
    {
        return pixel.x == x && pixel.y == y;
    };
};


class Border
{
private:
    list<Pixel> borderPixels;

public:
    Border(int size)
    {
        for (int i = 0; i <= size - 1; i++)
        {
            // Border in width
            borderPixels.push_back(Pixel(i, 0));
            borderPixels.push_back(Pixel(i, size - 1));

            // Border in height
            if (i == 0 || i == size - 1) continue;
            borderPixels.push_back(Pixel(0, i));
            borderPixels.push_back(Pixel(size - 1, i));
        }
    }

    void render(Color color)
    {
        for (Pixel p : borderPixels)
        {
            p.render(color);
        }
    }

    bool contains(Pixel pixel)
    {
        for (Pixel p : borderPixels)
        {
            if (p.equals(pixel)) return true;
        }

        return false;
    }
};


enum SnakeDirection
{
    Up, Down, Left, Right
};


class Snake
{
private:
    list<Pixel> bodyPixels;
    Pixel headPixel;

public:
    Snake(int size)
    {
        headPixel = Pixel(_gridSize / 2 + (size / 2), _gridSize / 2 - 1);
        for (int i = size - 1; i > 0; i--)
        {
            bodyPixels.push_back(Pixel(headPixel.x - i, headPixel.y));
        }
    }

    void render(Color headColor, Color bodyColor)
    {
        headPixel.render(headColor);
        for (Pixel p : bodyPixels)
        {
            p.render(bodyColor);
        }
    }

    bool move(SnakeDirection direction, Border border)
    {
        int x = headPixel.x;
        int y = headPixel.y;
        
        if (direction == Up) y--;
        else if (direction == Right) x++;
        else if (direction == Down) y++;
        else if (direction == Left) x--;

        Pixel newHead = Pixel(x, y);
        if (contains(newHead)) return false;
        if (border.contains(newHead)) return false;

        bodyPixels.push_back(headPixel);
        bodyPixels.pop_front();
        headPixel = newHead;
        return true;
    }

    
    void grow()
    {
        Pixel newBody = Pixel(bodyPixels.front().x, bodyPixels.front().y);
        bodyPixels.push_front(newBody);
    }

    int getSize()
    {
        return bodyPixels.size() + 1;
    }

    bool contains(Pixel pixel)
    {
        if (headPixel.equals(pixel)) return true;
        for (Pixel p : bodyPixels)
        {
            if (p.equals(pixel)) return true;
        }

        return false;
    }
};


class Berry
{
public:
    Pixel position;

    Berry(Snake snake)
    {
        srand(time(NULL));
        int min = 1;
        int max = _gridSize - 2;

        do
        {
            int randX = rand() % (max - min + 1) + min;
            int randY = rand() % (max - min + 1) + min;
            position = Pixel(randX, randY);
        } while (snake.contains(position));
    }

    void render(Color color)
    {
        position.render(color);
    }
};



// Game variables
Border border(_gridSize);
Snake snake(_snakeSize);
Berry berry(snake);
SnakeDirection direction = Right;
SnakeDirection newDirection = direction;
bool gameOver = false;


// Colors
const Color _colorBackground{ 36, 36, 36, 255};
const Color _colorSnakeHead{255, 170, 0, 255};
const Color _colorSnakeBody{255, 255, 85, 255};
const Color _colorBerry{255, 85, 85, 255};
const Color _colorBorder{85, 85, 85, 255};
const Color _colorGameover{255, 85, 85, 255};
const Color _colorScore{255, 255, 85, 255};
const Color _colorScoreNumber{255, 170, 0, 255};



void keyPressed(SnakeDirection key) {
    if (gameOver) return;

    switch (key)
    {
    case Up:
        if (direction == Down) break;
        newDirection = Up;
        break;
    case Down:
        if (direction == Up) break;
        newDirection = Down;
        break;
    case Left:
        if (direction == Right) break;
        newDirection = Left;
        break;
    case Right:
        if (direction == Left) break;
        newDirection = Right;
        break;
    }
}


void tick() {
    direction = newDirection;

    // Move snake and check if it actually moved
    if (!snake.move(direction, border))
    {
        // Game over
        gameOver = true;
        return;
    }

    // Check if snake got the berry
    if (snake.contains(berry.position))
    {
        berry = Berry(snake);
        snake.grow();
    }
};


// Gets unix timestamp in milliseconds
uint64_t getUnixTimestamp() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
};


int main()
{
    InitWindow(_size, _size, "Snake C++");
    SetTargetFPS(60);


    uint64_t lastTick = getUnixTimestamp();
    
    // Game loop
    while (!WindowShouldClose())
    {
        // Key press events
        if (IsKeyPressed(KEY_UP)) keyPressed(Up);
        if (IsKeyPressed(KEY_RIGHT)) keyPressed(Right);
        if (IsKeyPressed(KEY_DOWN)) keyPressed(Down);
        if (IsKeyPressed(KEY_LEFT)) keyPressed(Left);


        // Check if it's time for next tick
        uint64_t now = getUnixTimestamp();
        if (now - lastTick >= _tickTime) {
            lastTick = now;
            tick();
        }

        // Render everything
        BeginDrawing();
        ClearBackground(_colorBackground);

        // Game over screen
        if (gameOver) {
            int scale = (int)(_scale * 1.4);
            int score = snake.getSize() - _snakeSize;

            // "Game over!"
            int width = MeasureText("Game over!", scale);
            DrawText("Game over!", (_size - width) / 2, _size / 2 - (scale * 2), scale, _colorGameover);

            // "Score: 0"
            ostringstream s;
            s << "Score: " << score;
            string scoreString(s.str());
            const char* scoreText = scoreString.c_str();

            width = MeasureText(scoreText, scale);

            DrawText(scoreText, (_size - width) / 2, _size / 2 - (scale), scale, _colorScoreNumber);
            DrawText("Score: ", (_size - width) / 2, _size / 2 - (scale), scale, _colorScore);

            // Draw border and end rendering
            border.render(_colorBorder);
            EndDrawing();
            continue;
        }

        
        // Render everything
        border.render(_colorBorder);
        snake.render(_colorSnakeHead, _colorSnakeBody);
        berry.render(_colorBerry);

        EndDrawing();
    }


    CloseWindow();
    return 0;
}