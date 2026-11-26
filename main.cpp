#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;

const int   WIN_W = 600, WIN_H = 700, HUD_H = 50;
const int   ROWS = 8, COLS = 10;
const float BRICK_W = 60.f, BRICK_H = 25.f, BRICKS_Y = 100.f;
const int   PADDLE_W = 100, PADDLE_H = 16, PADDLE_Y = 625, PADDLE_SPEED = 8;
const int   BALL_SIZE = 16;
const int   START_LIVES = 3, POINTS_HIT = 10, POINTS_BREAK = 50;

const int NUM_STARS = 70;
float starX[NUM_STARS];
float starY[NUM_STARS];
float starSpeed[NUM_STARS];

const int MENU = 0, PLAYING = 1, GAME_OVER = 2;

const int KEY_NONE = 0, KEY_LAUNCH = 1, KEY_ENTER = 2, KEY_ESCAPE = 3, KEY_UP = 4, KEY_DOWN = 5;

int  gameState = MENU;
int  pressedKey = KEY_NONE;
int  bricks[ROWS][COLS];
int  paddleX = 250;
int  ballX = 0, ballY = 638, dx = 0, dy = 0;
bool ballWaiting = true;
int  score = 0, lives = START_LIVES;
bool playerWon = false;
int  menuChoice = 0;
std::string messageText = "";

sf::Font font;
sf::Texture bgTex;
sf::Texture brickTex[4];

const int MAX_POWERUPS = 5;
const int POWER_BIG = 1;
const int POWER_SPEED = 2;
const int POWER_SIZE = 18;

bool  powerActive[MAX_POWERUPS];
float powerX[MAX_POWERUPS], powerY[MAX_POWERUPS];
int   powerType[MAX_POWERUPS];

int currentPaddleW = PADDLE_W;
int bigPaddleTimer = 0;
int speedTimer = 0;

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
        for (int j = 0; j < cols; j++)
        {
            fin >> bricks[i][j];
        }
    }
    return true;
}

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

	// Reset power-ups:
										  // reset the power-up state so a new game doesn't carry over a previous power-up effect
    currentPaddleW = PADDLE_W;
    bigPaddleTimer = 0;
    speedTimer = 0;                                               
    for (int p = 0; p < MAX_POWERUPS; p++)
    {
        powerActive[p] = false;
    }
    //---------------
    if (!loadLevel())
    {
        return;
    }
    gameState = PLAYING;
}

void movePaddle()                                // speedTimer boosts paddle speed and currentPaddleW handles the right wall and ball centering
{
    int spd = (speedTimer > 0) ? 14 : PADDLE_SPEED;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        paddleX = paddleX - spd;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        paddleX = paddleX + spd;

    if (paddleX < 0)
    {
        paddleX = 0;
    }
    if (paddleX + currentPaddleW > WIN_W)
    {
        paddleX = WIN_W - currentPaddleW;
    }
    if (ballWaiting == true)
    {
        ballX = paddleX + (currentPaddleW - BALL_SIZE) / 2;
    }
}

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
				bool wasHard = (rand() % 3 == 0);            // now powerups will spawn randomly
                bricks[r][c]--;
                if (bricks[r][c] == 0)
                {
                    score += POINTS_BREAK;
                    if (wasHard)   // spawns a powerup
                    {
                        for (int p = 0; p < MAX_POWERUPS; p++)
                        {
                            if (!powerActive[p])
                            {
                                powerActive[p] = true;
                                powerX[p] = c * BRICK_W + BRICK_W / 2.f - POWER_SIZE / 2.f;
                                powerY[p] = BRICKS_Y + r * BRICK_H;
                                powerType[p] = (rand() % 2) + 1;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    score += POINTS_HIT;
                }
                hit = true;
                lastR = r;
                lastC = c;
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

void bounceOnPaddle() // swapped PADDLE_W for currentPaddleW and divided by (currentPaddleW / 4) so the 4 bounce angles stretch evenly when the paddle is wide
{
    if (dy > 0)
    {
        if (ballY + BALL_SIZE >= PADDLE_Y && ballY <= PADDLE_Y + PADDLE_H &&
            ballX + BALL_SIZE >= paddleX && ballX <= paddleX + currentPaddleW)
        {
            ballY = PADDLE_Y - BALL_SIZE;
            dy = -5;
            int ballCenter = ballX + BALL_SIZE / 2;
            int part = (ballCenter - paddleX) / (currentPaddleW / 4);
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

void hitBricks()
{
    bool allBroken = true;
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
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

void saveGame()
{
    std::ofstream fout("savegame.txt");
    if (!fout)
        return;
    fout << score << " " << lives << " " << paddleX << " " << ballX << " " << ballY << " " << dx << " " << dy << " " << ballWaiting << endl;
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            fout << bricks[r][c] << " ";
        }
        fout << endl;
    }
    fout.close();
}

bool loadGame()
{
    std::ifstream fin("savegame.txt");
    if (!fin || !(fin >> score))
    {
        return false;
    }
    fin >> lives >> paddleX >> ballX >> ballY >> dx >> dy >> ballWaiting;
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            fin >> bricks[r][c];
        }
    }
    fin.close();
    playerWon = false;
    messageText = "";
    gameState = PLAYING;
    return true;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------

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
                    (BRICK_W - 2.f) / brickTex[hp].getSize().x,
                    (BRICK_H - 2.f) / brickTex[hp].getSize().y
                    });
                bSprite.setPosition({ c * BRICK_W + 1.f, BRICKS_Y + r * BRICK_H + 1.f });
                w.draw(bSprite);
            }
        }
    }
}

void drawPaddleAndBall(sf::RenderWindow& w)
{
    float px = (float)paddleX;
    float py = (float)PADDLE_Y;
    float pw = (float)currentPaddleW;
    float ph = (float)PADDLE_H;
    float r = ph / 2.f;

    sf::Color bodyColor(25, 130, 210);
    sf::Color rimColor(140, 235, 255);

    sf::CircleShape leftCap(r);
    leftCap.setFillColor(bodyColor);
    leftCap.setOutlineThickness(2.f);
    leftCap.setOutlineColor(rimColor);
    leftCap.setPosition({ px, py });
    w.draw(leftCap);

    sf::CircleShape rightCap(r);
    rightCap.setFillColor(bodyColor);
    rightCap.setOutlineThickness(2.f);
    rightCap.setOutlineColor(rimColor);
    rightCap.setPosition({ px + pw - ph, py });
    w.draw(rightCap);

    sf::RectangleShape centerBody({ pw - ph, ph });
    centerBody.setFillColor(bodyColor);
    centerBody.setPosition({ px + r, py });
    w.draw(centerBody);

    sf::RectangleShape topEdge({ pw - ph, 2.f });
    topEdge.setFillColor(rimColor);
    topEdge.setPosition({ px + r, py - 2.f });
    w.draw(topEdge);

    sf::RectangleShape bottomEdge({ pw - ph, 2.f });
    bottomEdge.setFillColor(rimColor);
    bottomEdge.setPosition({ px + r, py + ph });
    w.draw(bottomEdge);

    sf::RectangleShape coreGlow({ pw - ph - 16.f, 4.f });
    coreGlow.setFillColor(sf::Color(200, 250, 255));
    coreGlow.setPosition({ px + r + 8.f, py + ph / 2.f - 2.f });
    w.draw(coreGlow);

    float bx = (float)ballX;
    float by = (float)ballY;
    float br = BALL_SIZE / 2.f;

    sf::CircleShape outerGlow(br + 3.f);
    outerGlow.setFillColor(sf::Color(255, 120, 20, 75));
    outerGlow.setPosition({ bx - 3.f, by - 3.f });
    w.draw(outerGlow);

    sf::CircleShape ballBody(br);
    ballBody.setFillColor(sf::Color(255, 165, 20));
    ballBody.setOutlineThickness(1.5f);
    ballBody.setOutlineColor(sf::Color(255, 240, 150));
    ballBody.setPosition({ bx, by });
    w.draw(ballBody);

    sf::CircleShape innerCore(br * 0.5f);
    innerCore.setFillColor(sf::Color(255, 250, 210));
    innerCore.setPosition({ bx + br * 0.35f, by + br * 0.35f });
    w.draw(innerCore);
}

void drawHUD(sf::RenderWindow& w)
{
    sf::RectangleShape strip({ (float)WIN_W, (float)HUD_H });
    strip.setFillColor(sf::Color(15, 20, 40, 230));
    w.draw(strip);

    sf::RectangleShape bottomBorder({ (float)WIN_W, 2.f });
    bottomBorder.setFillColor(sf::Color(80, 190, 255, 180));
    bottomBorder.setPosition({ 0.f, (float)HUD_H - 2.f });
    w.draw(bottomBorder);

    sf::RectangleShape scoreBox({ 175.f, 34.f });
    scoreBox.setFillColor(sf::Color(25, 35, 65, 220));
    scoreBox.setOutlineThickness(1.5f);
    scoreBox.setOutlineColor(sf::Color(100, 210, 255, 160));
    scoreBox.setPosition({ 12.f, 8.f });
    w.draw(scoreBox);

    drawText(w, "SCORE", 22.f, 15.f, 16, sf::Color(130, 220, 255));
    drawText(w, std::to_string(score), 90.f, 12.f, 22, sf::Color(255, 225, 70));

    sf::RectangleShape livesBox({ 135.f, 34.f });
    livesBox.setFillColor(sf::Color(25, 35, 65, 220));
    livesBox.setOutlineThickness(1.5f);
    livesBox.setOutlineColor(sf::Color(255, 90, 120, 160));
    livesBox.setPosition({ (float)WIN_W - 147.f, 8.f });
    w.draw(livesBox);

    sf::Color heartFill(240, 45, 75);
    sf::Color heartOutline(255, 175, 190);
    for (int i = 0; i < lives; i++)
    {
        float x = (float)WIN_W - 130.f + i * 36.f;
        float y = 16.f;

        sf::CircleShape leftLobe(6.f);
        leftLobe.setFillColor(heartFill);
        leftLobe.setOutlineThickness(1.f);
        leftLobe.setOutlineColor(heartOutline);
        leftLobe.setPosition({ x, y });
        w.draw(leftLobe);

        sf::CircleShape rightLobe(6.f);
        rightLobe.setFillColor(heartFill);
        rightLobe.setOutlineThickness(1.f);
        rightLobe.setOutlineColor(heartOutline);
        rightLobe.setPosition({ x + 10.f, y });
        w.draw(rightLobe);

        sf::RectangleShape tip({ 12.f, 12.f });
        tip.setFillColor(heartFill);
        tip.setOrigin({ 6.f, 6.f });
        tip.setRotation(sf::degrees(45.f));
        tip.setPosition({ x + 11.f, y + 9.5f });
        w.draw(tip);

        sf::CircleShape shine(2.f);
        shine.setFillColor(sf::Color(255, 230, 235, 200));
        shine.setPosition({ x + 2.5f, y + 2.5f });
        w.draw(shine);
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

            sf::Text shadow = txt;
            shadow.setFillColor(sf::Color(0, 0, 0, 160));
            shadow.setOutlineThickness(0.f);
            shadow.setPosition({ WIN_W / 2.f + 4.f, y + 4.f });
            w.draw(shadow);

            txt.setPosition({ WIN_W / 2.f, y });
            w.draw(txt);
        };

    drawCenteredText("BLOCK BUSTER", 54, sf::Color(255, 220, 40), sf::Color(200, 60, 20), 170.f);

    std::string items[3] = { "New Game", "Continue", "Exit" };
    for (int i = 0; i < 3; i++)
    {
        float y = 320.f + i * 65.f;
        if (menuChoice == i)
        {
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

            sf::Text shadow = txt;
            shadow.setFillColor(sf::Color(0, 0, 0, 160));
            shadow.setOutlineThickness(0.f);
            shadow.setPosition({ WIN_W / 2.f + 4.f, y + 4.f });
            w.draw(shadow);

            txt.setPosition({ WIN_W / 2.f, y });
            w.draw(txt);
        };

    if (playerWon)
        drawCenteredText("YOU WIN!", 56, sf::Color(80, 255, 120), sf::Color(15, 90, 35), 220.f);
    else
        drawCenteredText("GAME OVER", 56, sf::Color(255, 75, 75), sf::Color(110, 15, 25), 220.f);

    drawCenteredText("Final Score: " + std::to_string(score), 32, sf::Color(255, 220, 40), sf::Color(20, 20, 40), 320.f);

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

void updatePowerUps() 
{
    for (int p = 0; p < MAX_POWERUPS; p++)
    {
        if (powerActive[p])
        {
            powerY[p] += 3.f;

            if (powerY[p] + POWER_SIZE >= PADDLE_Y && powerY[p] <= PADDLE_Y + PADDLE_H &&
                powerX[p] + POWER_SIZE >= paddleX && powerX[p] <= paddleX + currentPaddleW)
            {
                powerActive[p] = false;
                if (powerType[p] == POWER_BIG)
                {
                    currentPaddleW = 150;
                    bigPaddleTimer = 600;
                }
                else if (powerType[p] == POWER_SPEED)
                {
                    speedTimer = 480;
                }
            }
            else if (powerY[p] > WIN_H)
            {
                powerActive[p] = false;
            }
        }
    }

    if (bigPaddleTimer > 0)
    {
        bigPaddleTimer--;
        if (bigPaddleTimer == 0)
        {
            currentPaddleW = PADDLE_W;
        }
    }

    if (speedTimer > 0)
    {
        speedTimer--;
    }
}

void handleInput(sf::RenderWindow& window)
{
    pressedKey = KEY_NONE;
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
                if (!loadGame())
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
        updatePowerUps();
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

void drawGame(sf::RenderWindow& window)
{
    window.clear(sf::Color(25, 25, 45));

    sf::Sprite bgSprite(bgTex);
    bgSprite.setScale({ (float)WIN_W / bgTex.getSize().x, (float)WIN_H / bgTex.getSize().y });
    window.draw(bgSprite);

    sf::RectangleShape starShape({ 2.f, 2.f });
    starShape.setFillColor(sf::Color(180, 200, 255, 160));

    for (int i = 0; i < NUM_STARS; i++)
    {
        starY[i] += starSpeed[i];
        if (starY[i] > WIN_H)
        {
            starY[i] = 0;
            starX[i] = rand() % WIN_W;
        }
        starShape.setPosition({ starX[i], starY[i] });
        window.draw(starShape);
    }

    switch (gameState)
    {
    case MENU:
        drawMenu(window);
        break;
    case PLAYING:
        drawBricks(window);
        drawPaddleAndBall(window);
        drawHUD(window);
        for (int p = 0; p < MAX_POWERUPS; p++)
        {
            if (powerActive[p])
            {
                sf::CircleShape orb(POWER_SIZE / 2.f);
                orb.setPosition({ powerX[p], powerY[p] });
                orb.setOutlineThickness(2.f);
                orb.setOutlineColor(sf::Color::White);
                if (powerType[p] == POWER_BIG)
                    orb.setFillColor(sf::Color(50, 235, 140));
                else
                    orb.setFillColor(sf::Color(220, 70, 255));
                window.draw(orb);
            }
        }
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
    for (int i = 0; i < NUM_STARS; i++)
    {
        starX[i] = rand() % WIN_W;
        starY[i] = rand() % WIN_H;
        starSpeed[i] = 0.5f + (rand() % 3);
    }

    sf::RenderWindow window(sf::VideoMode({ 600, 700 }), "Block Buster");
    window.setFramerateLimit(60);
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) return -1;
    if (!bgTex.loadFromFile("space_background.png")) return -1;
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