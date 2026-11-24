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
const int PADDLE_W = 110, PADDLE_H = 48, PADDLE_Y = 625, PADDLE_SPEED = 8;
const int   BALL_SIZE = 22;
const int   START_LIVES = 3, POINTS_HIT = 10, POINTS_BREAK = 50;
const int NUM_STARS = 70;
float starX[NUM_STARS];
float starY[NUM_STARS];
float starSpeed[NUM_STARS];

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
sf::Texture bgTex;
sf::Texture paddleTex;
sf::Texture ballTex;
sf::Texture brickTex[4]; // index 1 = green, 2 = yellow, 3 = red

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
    ballY = PADDLE_Y - BALL_SIZE + 6;
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
           ballY = PADDLE_Y - BALL_SIZE + 6;
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void bounceOnPaddle()
{
    int visualTop = PADDLE_Y + 14; // Has no relation to logic...just for graphics
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
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            int hp = bricks[r][c];
            if (hp > 0)
            {
                sf::Sprite bSprite(brickTex[hp]);
                bSprite.setScale({
                    58.f / brickTex[hp].getSize().x,
                    23.f / brickTex[hp].getSize().y
                    });
                bSprite.setPosition({ c * 60.f + 1.f, 100.f + r * 25.f + 1.f });
                w.draw(bSprite);
            }
        }
    }
}

void drawPaddleAndBall(sf::RenderWindow& w)
{
    // Paddle (missile4fav.png)
    sf::Sprite pSprite(paddleTex);
    pSprite.setScale({
        (float)PADDLE_W / paddleTex.getSize().x,
        (float)PADDLE_H / paddleTex.getSize().y
        });
    pSprite.setPosition({ (float)paddleX, (float)PADDLE_Y });
    w.draw(pSprite);

    // Ball (ball.png)
    sf::Sprite ballSprite(ballTex);
    ballSprite.setScale({
        (float)BALL_SIZE / ballTex.getSize().x,
        (float)BALL_SIZE / ballTex.getSize().y
        });
    ballSprite.setPosition({ (float)ballX, (float)ballY + 14.f });
    w.draw(ballSprite);
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
    auto drawCenteredText = [&](const std::string& str, unsigned int size, sf::Color fill, sf::Color outline, float y)
        {
            sf::Text txt(font, str, size);
            txt.setFillColor(fill);
            txt.setOutlineColor(outline);
            txt.setOutlineThickness(3.f);

            sf::FloatRect bounds = txt.getLocalBounds();
            txt.setOrigin({
                bounds.position.x + bounds.size.x / 2.f,
                bounds.position.y + bounds.size.y / 2.f
                });

            // Subtle dark drop shadow
            sf::Text shadow = txt;
            shadow.setFillColor(sf::Color(0, 0, 0, 160));
            shadow.setOutlineThickness(0.f);
            shadow.setPosition({ WIN_W / 2.f + 4.f, y + 4.f });
            w.draw(shadow);

            // Main text centered at X = 300
            txt.setPosition({ WIN_W / 2.f, y });
            w.draw(txt);
        };

    // Title
    drawCenteredText("BLOCK BUSTER", 54, sf::Color(255, 220, 40), sf::Color(200, 60, 20), 170.f);

    // Menu options
    std::string items[3] = { "New Game", "Continue", "Exit" };
    for (int i = 0; i < 3; i++)
    {
        float y = 320.f + i * 65.f;
        if (menuChoice == i)
        {
            // Draw a glowing pill/box behind the selected option
            sf::RectangleShape box({ 240.f, 46.f });
            box.setOrigin({ 120.f, 23.f });
            box.setPosition({ WIN_W / 2.f, y });
            box.setFillColor(sf::Color(255, 215, 0, 40));
            box.setOutlineColor(sf::Color(255, 220, 40));
            box.setOutlineThickness(2.f);
            w.draw(box);

            drawCenteredText("> " + items[i] + " <", 30, sf::Color(255, 235, 80), sf::Color(20, 20, 40), y);
        }
        else
        {
            drawCenteredText(items[i], 28, sf::Color(220, 230, 255), sf::Color(15, 15, 30), y);
        }
    }

    // Error / info message (e.g., "No saved game")
    if (!messageText.empty())
    {
        drawCenteredText(messageText, 22, sf::Color(255, 90, 90), sf::Color(20, 0, 0), 540.f);
    }
}


void drawEndScreen(sf::RenderWindow& w)
{
    auto drawCenteredText = [&](const std::string& str, unsigned int size, sf::Color fill, sf::Color outline, float y)
        {
            sf::Text txt(font, str, size);
            txt.setFillColor(fill);
            txt.setOutlineColor(outline);
            txt.setOutlineThickness(3.f);

            sf::FloatRect bounds = txt.getLocalBounds();
            txt.setOrigin({
                bounds.position.x + bounds.size.x / 2.f,
                bounds.position.y + bounds.size.y / 2.f
                });

            // Subtle dark drop shadow
            sf::Text shadow = txt;
            shadow.setFillColor(sf::Color(0, 0, 0, 160));
            shadow.setOutlineThickness(0.f);
            shadow.setPosition({ WIN_W / 2.f + 4.f, y + 4.f });
            w.draw(shadow);

            // Main text centered at X = 300
            txt.setPosition({ WIN_W / 2.f, y });
            w.draw(txt);
        };

    // 1. Main Title ("YOU WIN" in gold/green or "GAME OVER" in crimson/gold)
    if (playerWon)
        drawCenteredText("YOU WIN!", 56, sf::Color(80, 255, 120), sf::Color(15, 90, 35), 220.f);
    else
        drawCenteredText("GAME OVER", 56, sf::Color(255, 75, 75), sf::Color(110, 15, 25), 220.f);

    // 2. Final Score
    drawCenteredText("Final Score: " + std::to_string(score), 32, sf::Color(255, 220, 40), sf::Color(20, 20, 40), 320.f);

    // 3. Glowing pill box + "Press Enter" prompt matching the menu selection style
    float promptY = 420.f;
    sf::RectangleShape box({ 260.f, 48.f });
    box.setOrigin({ 130.f, 24.f });
    box.setPosition({ WIN_W / 2.f, promptY });
    box.setFillColor(sf::Color(255, 215, 0, 40));
    box.setOutlineColor(sf::Color(255, 220, 40));
    box.setOutlineThickness(2.f);
    w.draw(box);

    drawCenteredText("> Press Enter <", 28, sf::Color(255, 235, 80), sf::Color(20, 20, 40), promptY);
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

    sf::Sprite bgSprite(bgTex);
    bgSprite.setScale({ (float)WIN_W / bgTex.getSize().x, (float)WIN_H / bgTex.getSize().y });
    window.draw(bgSprite);

    sf::RectangleShape starShape({ 2.f, 2.f });
    starShape.setFillColor(sf::Color(180, 200, 255, 160));

    for (int i = 0; i < NUM_STARS; i++) {
        starY[i] += starSpeed[i];
        if (starY[i] > WIN_H) {
            starY[i] = 0;
            starX[i] = rand() % WIN_W;
        }
        starShape.setPosition({ starX[i], starY[i] });
        window.draw(starShape);
    }
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
    for (int i = 0; i < NUM_STARS; i++) {
        starX[i] = rand() % WIN_W;
        starY[i] = rand() % WIN_H;
        starSpeed[i] = 0.5f + (rand() % 3); // different speeds for depth
    }
    sf::RenderWindow window(sf::VideoMode({ 600, 700 }), "Block Buster");
    window.setFramerateLimit(60);
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) return -1;
    if (!bgTex.loadFromFile("space_background.png")) return -1;
    if (!paddleTex.loadFromFile("missile4fav.png")) return -1;
    if (!ballTex.loadFromFile("ball.png")) return -1;
    if (!brickTex[1].loadFromFile("space_brick.png")) return -1;
    if (!brickTex[2].loadFromFile("asteroid_brick.png")) return -1;
    if (!brickTex[3].loadFromFile("blackhole_brick.png")) return -1;
    while (window.isOpen()) 
    {
        handleInput(window);
        updateGame(window);
        drawGame(window);
    }
    return 0;
}