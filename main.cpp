#include <SFML/Graphics.hpp>
#include<iostream>
#include <optional>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;

// ===== CONSTANTS (Section 4 values) =====
const int   WIN_W = 600, WIN_H = 700, HUD_H = 50;
const int   ROWS = 8, COLS = 10;
const float BRICK_W = 60.f, BRICK_H = 25.f, BRICKS_Y = 100.f;
const int   PADDLE_W = 100, PADDLE_H = 14, PADDLE_Y = 650, PADDLE_SPEED = 8;
const int   BALL_SIZE = 12;
const int   START_LIVES = 3, POINTS_HIT = 10, POINTS_BREAK = 50;

// ===== GAME STATES =====
const int MENU = 0, PLAYING = 1, GAME_OVER = 2;

// ===== KEYS PRESSED ONCE (set by handleInput) =====
const int KEY_NONE = 0, KEY_LAUNCH = 1, KEY_ENTER = 2, KEY_ESCAPE = 3, KEY_UP = 4, KEY_DOWN = 5;

// ===== GAME DATA (shared by all functions) =====
int  gameState = MENU;
int  pressedKey = KEY_NONE;
int  bricks[ROWS][COLS];
int  paddleX = 250;
int  ballX = 0, ballY = 638, dx = 0, dy = 0;
bool ballWaiting = true;          // true while the ball sits on the paddle
int  score = 0, lives = START_LIVES;
bool playerWon = false;
int  menuChoice = 0;              // 0 = New Game, 1 = Continue, 2 = Exit
std::string messageText = "";     // e.g. "No saved game" or the level.txt error

sf::Font font;

// =====================================================
//  YOUR TEAM'S WORK: write these functions yourselves
// =====================================================
bool loadLevel() 
{
    std::ifstream fin("level.txt");
    if (!fin)
    {
        messageText = "level.txt not found";
        return false;
    }
    int rows, cols;
    fin >> rows >> cols;
    
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0;j < cols;j++) 
        {
            fin >> bricks[i][j];
        }
    }
    return true; 
}
//--------------------------------------------------------------------------------------------------------------------------------------------
void newGame() 
{
    score = 0;
    lives = START_LIVES;
    paddleX = 250;
    ballX = paddleX + (PADDLE_W - BALL_SIZE) / 2;
    ballY = PADDLE_Y - BALL_SIZE;
    dx = 0;
    dy = 0;
    ballWaiting = true;
    playerWon = false;
    messageText = "";
    if (!loadLevel())
    {
        return;
    }
    gameState = PLAYING;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
void movePaddle() 
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        paddleX = paddleX - PADDLE_SPEED;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        paddleX = paddleX + PADDLE_SPEED;

    if (paddleX < 0)
    {
        paddleX = 0;
    }
    if (paddleX + PADDLE_W > WIN_W)
    {
        paddleX = WIN_W - PADDLE_W;
    }
    if (ballWaiting == true)
    {
        ballX = paddleX + (PADDLE_W - BALL_SIZE) / 2;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
void launchBall() 
{
    if (ballWaiting == true && pressedKey == KEY_LAUNCH)
    {
        ballWaiting = false;
        dy = -5;
        if (rand() % 2 == 0)
           {
                dx = 3;
           }
        else
           {
                dx = -3;
           }
    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------
void checkBricks(bool isXStep)
{
    int cx[4] = { ballX, ballX + BALL_SIZE - 1, ballX, ballX + BALL_SIZE - 1 };
    int cy[4] = { ballY, ballY, ballY + BALL_SIZE - 1, ballY + BALL_SIZE - 1 };
    bool hit = false;
    int lastR = -1;
    int lastC = -1;
    for (int i = 0; i < 4; i++)
    {
        if (cx[i] >= 0 &&
            cx[i] < WIN_W &&
            cy[i] >= BRICKS_Y &&
            cy[i] < BRICKS_Y + ROWS * BRICK_H)
        {
            int c = cx[i] / (int)BRICK_W;
            int r = (cy[i] - (int)BRICKS_Y) / (int)BRICK_H;
            if (bricks[r][c] > 0 && !(r == lastR && c == lastC))
            {
                bricks[r][c]--;
                if (bricks[r][c] == 0)
                {
                    score += POINTS_BREAK;
                    hit = true;
                    lastR = r;
                    lastC = c;
                }
                else
                {
                    score += POINTS_HIT;
                    hit = true;
                    lastR = r;
                    lastC = c;
                }
            }
        }
    }
    if (hit == true)
    {
        if (isXStep == true)
        {
            ballX -= dx;
            dx = -dx;
        }
        else
        {
            ballY -= dy;
            dy = -dy;
        }
    }

}
//-------------------------------------------------------------------------------------------------------------------------------------------
void moveBall() 
{ 
    if (ballWaiting == true)
    {
        return;
    }
    ballX += dx;
    if (ballX < 0 || ballX + BALL_SIZE > WIN_W)
    {
        ballX -= dx;
        dx = -dx;
    }
    checkBricks(true);

    ballY += dy;
    if (ballY < HUD_H)
    {
        ballY -= dy;
        dy = -dy;
    }
    checkBricks(false);
    if (ballY > WIN_H)
    {
        lives--;
        if (lives <= 0)
        {
            playerWon = false;
            gameState = GAME_OVER;
        }
        else
        {
            ballWaiting = true;
            dx = 0;
            dy = 0;
            ballX = paddleX + (PADDLE_W - BALL_SIZE) / 2;
           ballY = PADDLE_Y - BALL_SIZE;
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void bounceOnPaddle()
{
    if (dy > 0)
    {
        if (ballY + BALL_SIZE >= PADDLE_Y && ballY <= PADDLE_Y + PADDLE_H &&
            ballX + BALL_SIZE >= paddleX && ballX <= paddleX + PADDLE_W)
        {
            ballY = PADDLE_Y - BALL_SIZE;
            dy = -5;
            int ballCenter = ballX + BALL_SIZE / 2;
            int part = (ballCenter - paddleX) / 25;
            if (part < 0)
            {
                part = 0;
            }
            if (part > 3)
            {
                part = 3;
            }
            if (part == 0)
            {
                dx = -6;
            }
            if (part == 1)
            {
                dx = -3;
            }
            if (part == 2)
            {
                dx = 3;
            }
            if (part == 3)
            {
                dx = 6;
            }
        }
    }
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
void hitBricks() 
{
    bool allBroken = true;
    for (int i = 0;i < ROWS;i++)
    {
        for (int j = 0;j < COLS;j++)
        {
            if (bricks[i][j] > 0)
            {
                allBroken = false;
            }
        }
    }
    if (allBroken == true)
    {
        playerWon = true;
        gameState = GAME_OVER;
    }
}
//----------------------------------------------------------------------------------------------------------------
void saveGame() 
{
    std::ofstream fout("savegame.txt");
    if (!fout)
        return;
    fout << score << " " << lives << " " << paddleX << " " << ballX << " " << ballY << " " << dx << " " << dy << " " << ballWaiting << endl;
    for (int r = 0; r < ROWS ; r++)
    {
        for (int c = 0; c < COLS ;c++)
        {
            fout << bricks[r][c] << " ";
        }
        fout << endl;
    }
    fout.close(); 

}
bool loadGame() 
{
    bool loadGame;
    std::ifstream fin("savegame.txt");
    if (!fin || !(fin >> score))
    {
        return false;
    }
    fin >> lives >> paddleX >> ballX >> ballY >> dx >> dy >> ballWaiting;
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS;c++)
        {
            fin >> bricks[r][c];
        }
    }
    fin.close();
    playerWon = false;
    messageText = "";
    gameState = PLAYING;
    return true;

    return false; 
}
//================================================================================================================================

void drawText(sf::RenderWindow& w, const std::string& str, float x, float y,
    unsigned size, sf::Color color = sf::Color::White) 
{
    sf::Text t(font, str, size);
    t.setFillColor(color);
    t.setPosition({ x, y });
    w.draw(t);
}

void drawBricks(sf::RenderWindow& w) 
{
    sf::Color colors[4] = { sf::Color(25, 25, 45), sf::Color::Green,
                            sf::Color::Yellow, sf::Color::Red };
    sf::RectangleShape brick({ BRICK_W - 2, BRICK_H - 2 });   // smaller, so a gap shows
    for (int r = 0; r < ROWS; r++) 
    {
        for (int c = 0; c < COLS; c++) 
        {
            if (bricks[r][c] == 0) continue;                  // 0 = no brick
            brick.setFillColor(colors[bricks[r][c]]);
            brick.setPosition({ c * BRICK_W + 1, BRICKS_Y + r * BRICK_H + 1 });
            w.draw(brick);
        }
    }
}

void drawPaddleAndBall(sf::RenderWindow& w) 
{
    sf::RectangleShape paddle({ (float)PADDLE_W, (float)PADDLE_H });
    sf::CircleShape ball(BALL_SIZE / 2.f);                    // radius 6
    paddle.setPosition({ (float)paddleX, (float)PADDLE_Y });
    ball.setPosition({ (float)ballX, (float)ballY });
    w.draw(paddle);
    w.draw(ball);
}

void drawHUD(sf::RenderWindow& w) 
{
    sf::RectangleShape strip({ (float)WIN_W, (float)HUD_H });
    strip.setFillColor(sf::Color(30, 30, 50));
    w.draw(strip);
    drawText(w, "Score: " + std::to_string(score), 15.f, 10.f, 24);

    sf::CircleShape life(8.f);                                // one circle per life
    life.setFillColor(sf::Color::Red);
    for (int i = 0; i < lives; i++) 
    {
        life.setPosition({ 480.f + i * 30.f, 17.f });
        w.draw(life);
    }
}

void drawMenu(sf::RenderWindow& w) 
{
    drawText(w, "BLOCK BUSTER", 130.f, 150.f, 56, sf::Color::Yellow);
    std::string options[3] = { "New Game", "Continue", "Exit" };
    for (int i = 0; i < 3; i++) {
        sf::Color col = (i == menuChoice) ? sf::Color::Yellow : sf::Color::White;
        drawText(w, (i == menuChoice ? "> " : "  ") + options[i], 220.f, 300.f + i * 60.f, 32, col);
    }
    drawText(w, messageText, 200.f, 520.f, 24, sf::Color::Red);   // "No saved game", file error
}

void drawEndScreen(sf::RenderWindow& w) 
{
    drawText(w, playerWon ? "YOU WIN" : "GAME OVER", 170.f, 250.f, 56,
        playerWon ? sf::Color::Green : sf::Color::Red);
    drawText(w, "Final score: " + std::to_string(score), 190.f, 350.f, 32);
    drawText(w, "Press Enter", 220.f, 420.f, 28);
}

// =====================================================
//  THE THREE LOOP FUNCTIONS (Feature 1)
// =====================================================
void handleInput(sf::RenderWindow& window) 
{
    pressedKey = KEY_NONE;                                    // forget last frame's key
    while (const std::optional event = window.pollEvent()) 
    {
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) 
        {
            if (key->code == sf::Keyboard::Key::Space)       pressedKey = KEY_LAUNCH;
            else if (key->code == sf::Keyboard::Key::Enter)  pressedKey = KEY_ENTER;
            else if (key->code == sf::Keyboard::Key::Escape) pressedKey = KEY_ESCAPE;
            else if (key->code == sf::Keyboard::Key::Up)     pressedKey = KEY_UP;
            else if (key->code == sf::Keyboard::Key::Down)   pressedKey = KEY_DOWN;
        }
    }
    // Keys held down (paddle) are checked inside movePaddle(), not here.
}

void updateGame(sf::RenderWindow& window) 
{
    switch (gameState) 
    {
    case MENU:
        if (pressedKey == KEY_UP)
        {
            menuChoice = (menuChoice + 2) % 3;
        }
        if (pressedKey == KEY_DOWN)
        {
            menuChoice = (menuChoice + 1) % 3;
        }
        if (pressedKey == KEY_ENTER)
        {
            if (menuChoice == 0)
            {
                newGame();
            }
            if (menuChoice == 1)
            {
                if(!loadGame())
                    messageText = "No saved game";
            }
            if (menuChoice == 2)
            {
                window.close();
            }
        }
        break;
    case PLAYING:
        if (pressedKey == KEY_ESCAPE)
        {
            saveGame();
            gameState = MENU;
        }
        movePaddle();
        launchBall();
        moveBall();
        bounceOnPaddle();
        hitBricks();
        break;
    case GAME_OVER:
        if (pressedKey == KEY_ENTER)
        {
            std::ofstream fout("savegame.txt");
            messageText = "";
            gameState = MENU;
        }
        break;
    }
}

void drawGame(sf::RenderWindow& window) {
    window.clear(sf::Color(25, 25, 45));
    switch (gameState) {
    case MENU:
        drawMenu(window);
        break;
    case PLAYING:
        drawBricks(window);
        drawPaddleAndBall(window);
        drawHUD(window);
        break;
    case GAME_OVER:
        drawEndScreen(window);
        break;
    }
    window.display();
}

int main() 
{
    srand(static_cast<unsigned>(time(0)));
    sf::RenderWindow window(sf::VideoMode({ 600, 700 }), "Block Buster");
    window.setFramerateLimit(60);
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) return -1;
    while (window.isOpen()) 
    {
        handleInput(window);
        updateGame(window);
        drawGame(window);
    }
    return 0;
}