#include "raylib.h"
#include <fstream>

enum GameState { MENU, PLAYING, GAME_OVER, INFO };
GameState gameState = MENU;

const int screenWidth = 500;
const int screenHeight = 500;

Rectangle player = { screenWidth / 2 - 10, screenHeight / 2 - 10, 20, 10 };
bool movingLeft = true;
int playerSpeed = 5;

const int blockCount = 6;
Rectangle blocks[blockCount];
float blockSpeed[blockCount] = { 3.0f, 3.0f, 3.0f, -3.0f, -3.0f, -3.0f };

float timer = 0.0f;
float highScore = 0.0f;

float LoadHighScore() {
    std::ifstream file("Game Files/highscore.txt");
    float score = 0.0f;
    if (file) file >> score;
    return score;
}

void SaveHighScore(float score) {
    std::ofstream file("Game Files/highscore.txt");
    file << score;
}

void ResetGame() {
    player.x = screenWidth / 2 - 10;
    player.y = screenHeight / 2 - 10;
    timer = 0.0f;

    for (int i = 0; i < blockCount; i++) {
        if (i < 3) {
            blocks[i] = { (float)GetRandomValue(0, screenWidth - 20), 0, 20, 20 };
        } else {
            blocks[i] = { (float)GetRandomValue(0, screenWidth - 20), screenHeight - 20, 20, 20 };
        }
    }
    gameState = PLAYING;
}

const Vector2 infoButtonPos = { 20.0f, 30.0f };
const Rectangle deleteButton = { 150, 400, 200, 40 };

void DeleteGameData() {
    highScore = 0.0f;
    SaveHighScore(highScore);
}

int main() {
    InitWindow(screenWidth, screenHeight, "D-2646");

    // Initialize audio device
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        TraceLog(LOG_ERROR, "Failed to initialize audio device!");
        return -1; // Exit the program if audio device fails to initialize
    }

    // Load assets
    Sound clickSound = LoadSound("Assets/Audio/click.wav");
    if (clickSound.frameCount == 0) {
        TraceLog(LOG_ERROR, "Failed to load click.wav!");
    }

    Sound hitSound = LoadSound("Assets/Audio/hit.wav");
    if (hitSound.frameCount == 0) {
        TraceLog(LOG_ERROR, "Failed to load hit.wav!");
    }

    Image icon = LoadImage("Assets/Visual/icon.ico");
    if (icon.data == NULL) {
        TraceLog(LOG_ERROR, "Failed to load icon.ico!");
    }

    SetWindowIcon(icon);
    UnloadImage(icon);

    highScore = LoadHighScore();
    SetTargetFPS(60);

    bool gameStarted = false;

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        switch (gameState) {
            case MENU: {
                Rectangle infoBtn = { infoButtonPos.x, infoButtonPos.y, 30, 30 };
                if (CheckCollisionPointRec(mousePos, infoBtn)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        PlaySound(clickSound);
                        gameState = INFO;
                        break;
                    }
                }
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(clickSound);
                    gameStarted = true;
                    ResetGame();
                }
                break;
            }

            case PLAYING: {
                if (!gameStarted) {
                    DrawText("Click to Start", screenWidth / 2 - 100, screenHeight / 2, 20, BLACK);
                    break;
                }

                timer += GetFrameTime();

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(clickSound);
                    movingLeft = !movingLeft;
                }

                if (movingLeft) player.x -= playerSpeed;
                else player.x += playerSpeed;

                if (player.x <= 0 || player.x + player.width >= screenWidth) {
                    PlaySound(hitSound);
                    if (timer > highScore) {
                        highScore = timer;
                        SaveHighScore(highScore);
                    }
                    gameState = GAME_OVER;
                }

                for (int i = 0; i < blockCount; i++) {
                    blocks[i].y += blockSpeed[i];

                    if (blocks[i].y > screenHeight) {
                        blocks[i].y = 0;
                        blocks[i].x = (float)GetRandomValue(0, screenWidth - 20);
                    }
                    if (blocks[i].y < 0) {
                        blocks[i].y = screenHeight - 20;
                        blocks[i].x = (float)GetRandomValue(0, screenWidth - 20);
                    }

                    if (CheckCollisionRecs(player, blocks[i])) {
                        PlaySound(hitSound);
                        if (timer > highScore) {
                            highScore = timer;
                            SaveHighScore(highScore);
                        }
                        gameState = GAME_OVER;
                    }
                }
                break;
            }

            case GAME_OVER: {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(clickSound);
                    gameState = MENU;
                }
                break;
            }

            case INFO: {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousePos, deleteButton)) {
                        DeleteGameData();
                        PlaySound(hitSound);
                    }
                    PlaySound(clickSound);
                    gameState = MENU;
                }
                break;
            }
        }

        BeginDrawing();
        ClearBackground((Color){ 253, 249, 0, 255 });

        switch (gameState) {
            case MENU: {
                const char* questionMark = "?";
                int qmWidth = MeasureText(questionMark, 30);
                DrawRectangle(infoButtonPos.x, infoButtonPos.y, 30, 30, BLACK);
                DrawText(questionMark, 
                       infoButtonPos.x + (30 - qmWidth)/2,
                       infoButtonPos.y + 1,
                       30, WHITE);

                int titleWidth = MeasureText("D-2646", 30);
                int playWidth = MeasureText("Click to Play", 20);
                int highScoreWidth = MeasureText(TextFormat("High Score: %.2f sec", highScore), 20);

                DrawText("D-2646", (screenWidth - titleWidth)/2, screenHeight/4, 30, BLACK);
                DrawText("Click to Play", (screenWidth - playWidth)/2, screenHeight/2, 20, BLACK);
                DrawText(TextFormat("High Score: %.2f sec", highScore), 
                       (screenWidth - highScoreWidth)/2, screenHeight - 50, 20, BLACK);
                break;
            }

            case PLAYING: {
                DrawRectangleRec(player, BLUE);
                for (int i = 0; i < blockCount; i++) {
                    DrawRectangleRec(blocks[i], RED);
                }

                int timerWidth = MeasureText(TextFormat("Time: %.2f sec", timer), 20);
                DrawText(TextFormat("Time: %.2f sec", timer), (screenWidth - timerWidth)/2, 10, 20, BLACK);
                break;
            }

            case GAME_OVER: {
                int gameOverWidth = MeasureText("Game Over!", 30);
                int timeWidth = MeasureText(TextFormat("You survived: %.2f sec", timer), 20);
                int returnWidth = MeasureText("Click to Return to Menu", 20);

                DrawText("Game Over!", (screenWidth - gameOverWidth)/2, screenHeight/3, 30, BLACK);
                DrawText(TextFormat("You survived: %.2f sec", timer), (screenWidth - timeWidth)/2, screenHeight/2, 20, BLACK);
                DrawText("Click to Return to Menu", (screenWidth - returnWidth)/2, screenHeight - 50, 20, BLACK);
                break;
            }

            case INFO: {
                int titleWidth = MeasureText("D-2646", 40);
                DrawText("D-2646", (screenWidth - titleWidth)/2, 40, 40, BLACK);

                const char* developer = "Developer: Amin Djellab";
                int devWidth = MeasureText(developer, 20);
                DrawText(developer, (screenWidth - devWidth)/2, 100, 20, BLACK);

                const char* instructions[] = {
                    "HOW TO PLAY",
                    "Tap screen to change direction (left/right)",
                    "Dodge red blocks & screen edges",
                    "Survive as long as possible!",
                    "",
                    "Support Developer via",
                    "Binance UID: 91561758",
                    "MEXC UID: 72844962",
                };
                
                int yPos = 150;
                for (const char* line : instructions) {
                    int width = MeasureText(line, 20);
                    DrawText(line, (screenWidth - width)/2, yPos, 20, BLACK);
                    yPos += 30;
                }

                DrawRectangleRec(deleteButton, RED);
                const char* deleteText = "DELETE DATA";
                int btnTextWidth = MeasureText(deleteText, 20);
                DrawText(deleteText, 
                       deleteButton.x + (deleteButton.width - btnTextWidth)/2,
                       deleteButton.y + 10, 
                       20, WHITE);

                const char* returnText = "Click anywhere to return";
                int returnTextWidth = MeasureText(returnText, 20);
                DrawText(returnText, (screenWidth - returnTextWidth)/2, 470, 20, DARKGRAY);
                break;
            }
        }

        EndDrawing();
    }

    UnloadSound(clickSound);
    UnloadSound(hitSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}