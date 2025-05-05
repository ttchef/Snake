
#include <stdlib.h> 
#include <stdint.h> 
#include <stdio.h>
#include <time.h>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define UP 0x1
#define DOWN 0x2
#define LEFT 0x3
#define RIGHT 0x4

#define SCREENWIDTH 640
#define SCREENHEIGHT 640 
#define GRIDSIZE 64 
#define NUMCELLS 10
#define SPEED 2.0f
#define MAX_SNAKE_LENGTH 100

#define CONFIG_FILE "config.txt"

// Gamestates
#define ALIVE 0 
#define DEAD 1 
#define WIN 2

// Function Declaration
void move_body(Vector2*, Vector2*, uint32_t);
void write_high_score(const char*, uint8_t);

Vector2 random_pos() {
    uint16_t x = (rand() % NUMCELLS)*GRIDSIZE;
    uint16_t y = (rand() % NUMCELLS)*GRIDSIZE;
    return (Vector2){x,y};
}

// return 1 if pos is on snake
uint8_t is_position_on_snake(Vector2 pos, Vector2* snake, uint32_t length) {
    for (int i = 0; i < length; i++) {
        if (pos.x == snake[i].x && pos.y == snake[i].y) return 1;
    }
    return 0;
}

Vector2 generate_apple(Vector2* snake, uint32_t length) {
    Vector2 pos;
    do {
        pos = random_pos();
    } while(is_position_on_snake(pos, snake, length)==1);
    return pos;
}

void init(Vector2* head_pos, uint8_t* direction, uint32_t* length, Vector2* apple_pos, uint8_t* input_direction) {
    *head_pos = (Vector2){ (float)SCREENWIDTH/2, (float)SCREENHEIGHT/2 };
    *direction = DOWN;
    *input_direction = DOWN;
    *length = 1; 
    *apple_pos = random_pos();
}

void handle_input(uint8_t direction, uint8_t* input_direction, uint8_t* grid_lines) {
    if (IsKeyDown(KEY_W) && direction != DOWN)  *input_direction = UP;
    if (IsKeyDown(KEY_S) && direction != UP)    *input_direction = DOWN;
    if (IsKeyDown(KEY_A) && direction != RIGHT) *input_direction = LEFT;
    if (IsKeyDown(KEY_D) && direction != LEFT)  *input_direction = RIGHT;

    // Customisation
    if (IsKeyDown(KEY_M)) (*grid_lines) = 1;
    if (IsKeyDown(KEY_N)) (*grid_lines) = 0;
}

void change_direction(uint8_t* direction, uint8_t input_direction, Vector2* snake, Vector2* spawn_pos, uint32_t length, Vector2* prev) {
    if ((uint32_t)snake[0].x % GRIDSIZE == 0 && (uint32_t)snake[0].y % GRIDSIZE == 0) {
        *direction = input_direction;
        *spawn_pos = snake[length];
        move_body(snake, prev, length);
    }
}

void init_position_list(Vector2* prevois, Vector2* pos) {
    for (int i = 0; i < MAX_SNAKE_LENGTH; i++) {
        prevois[i] = pos[i];
    }
}

void move_head(Vector2* snake, uint8_t direction) {
    switch (direction) {
        case UP: snake[0].y -= SPEED; break;
        case DOWN: snake[0].y += SPEED; break;
        case LEFT: snake[0].x -= SPEED; break;
        case RIGHT: snake[0].x += SPEED; break;
    }
}

void move_body(Vector2* snake, Vector2* prev, uint32_t length) {
    for (int i = 1; i < length; i++) {
        snake[i] = prev[i-1];
    }
}

// return 1 if collided
uint8_t handle_collision(Vector2* apple_pos, Vector2* snake, uint32_t* length, Vector2* prev, Vector2 spawn_pos, uint16_t* dead_frame, uint8_t* high_score) {
    if (snake[0].x == (*apple_pos).x && snake[0].y == (*apple_pos).y) {
        snake[*length] = spawn_pos;
        (*length)++;
        if (*length > *high_score) {
            *high_score = *length;
            write_high_score(CONFIG_FILE, *high_score);
        }
        *apple_pos = generate_apple(snake, *length);
        (*dead_frame)++;
    }
}

// return 1 when out of bounds
uint8_t check_out_of_bounds(Vector2* snake, uint32_t length) {
    for (int i = 0; i < length; i++) {
        if (snake[i].x < 0 || snake[i].x+GRIDSIZE > SCREENWIDTH) return 1;
        if (snake[i].y < 0 || snake[i].y+GRIDSIZE > SCREENHEIGHT) return 1;
    }
    return 0;
}

// return 1 if collide check if collide with body 
uint8_t check_collide_body(Vector2* snake, uint32_t length) {
    for (int i = 1; i < length; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) return 1;
    }
    return 0;
}

uint8_t check_game_over(Vector2* snake, uint32_t length, uint8_t* game_over) {
    *game_over = 0;
    if (check_out_of_bounds(snake, length) == 1) *game_over = 1;
    if (check_collide_body(snake, length) == 1) *game_over = 1;
    return *game_over;
}

void check_win(uint32_t length, uint8_t* state) {
    if (length == MAX_SNAKE_LENGTH) *state = 2; // Win
}

void draw(Vector2* snake, Vector2 apple_pos, uint32_t length, uint8_t grid_lines, uint8_t high_score) {
    BeginDrawing();
        ClearBackground(BLACK);

        // Draw Map
        uint16_t k = 0;
        for (int i = 0; i < NUMCELLS; i++) {
            for (int j = 0; j < NUMCELLS; j++) {
                k++;
                Color color = k % 2 ? DARKGREEN : LIME;
                DrawRectangle(i*GRIDSIZE, j*GRIDSIZE, GRIDSIZE-grid_lines, GRIDSIZE-grid_lines, color);
            }
            k++;
        }

        // Draw Apple 
        DrawCircleV((Vector2){apple_pos.x+GRIDSIZE/2, apple_pos.y+GRIDSIZE/2}, (GRIDSIZE/2)-5, RED);
            
        // Draw Snake Body
        for (int i = 1; i < length; i++) {
            if (i > 100) i = 100;
            Color color = {102, 191, 150+i*2.5, 255};
            Rectangle rect = {snake[i].x, snake[i].y, GRIDSIZE, GRIDSIZE};
            DrawRectangleRounded(rect, 0.0f, 1, color);
        }

        // Draw Head
        Rectangle rect = {snake[0].x+2, snake[0].y+2, GRIDSIZE-4, GRIDSIZE-4};
        DrawRectangleRounded(rect, 0.5f, 1, SKYBLUE);

        // Text 
        DrawText(TextFormat("Score: %d", length), 10, 10, 23, RAYWHITE);
        DrawText(TextFormat("High Score: %d", high_score), 10, 50, 23, RAYWHITE);
        
    
    EndDrawing();
}

uint8_t handle_menu_input() {
    if (IsKeyDown(KEY_ENTER)) return 1; // Reset
    return -1;
}

void draw_game_over(uint32_t length, uint8_t high_score) {
    BeginDrawing();
        ClearBackground(LIME);

        // Game Over Text 
        const char* game_message = "Game Over!";
        int game_fontSize = 50;
        int textWidth = MeasureText(game_message, game_fontSize);
        DrawText(game_message, (SCREENWIDTH-textWidth)/2, (SCREENHEIGHT-game_fontSize-50)/2, game_fontSize, RAYWHITE);

        // Score Text
        const char* score_message = TextFormat("Score: %d", length);
        int score_fontSize = 25;
        textWidth = MeasureText(score_message, score_fontSize);
        DrawText(score_message, (SCREENWIDTH-textWidth)/2, (SCREENHEIGHT-score_fontSize+30)/2, score_fontSize, RAYWHITE);

        // High Score Text 
        const char* high_score_message = TextFormat("High Score: %d", high_score);
        int high_score_fontSize = 25;
        textWidth = MeasureText(high_score_message, high_score_fontSize);
        DrawText(high_score_message, (SCREENWIDTH-textWidth)/2, (SCREENHEIGHT-high_score_fontSize+80)/2, high_score_fontSize, RAYWHITE);

    EndDrawing();
}

void draw_win_screen(uint32_t length, uint8_t high_score) {
    BeginDrawing();
        ClearBackground(LIME);

        // Game Over Text 
        const char* game_message = "You Won!";
        int game_fontSize = 50;
        int textWidth = MeasureText(game_message, game_fontSize);
        DrawText(game_message, (SCREENWIDTH-textWidth)/2, (SCREENHEIGHT-game_fontSize-50)/2, game_fontSize, RAYWHITE);

        // Score Text
        const char* score_message = TextFormat("Score: %d", length);
        int score_fontSize = 25;
        textWidth = MeasureText(score_message, score_fontSize);
        DrawText(score_message, (SCREENWIDTH-textWidth)/2, (SCREENHEIGHT-score_fontSize+30)/2, score_fontSize, RAYWHITE);

        // High Score Text 
        const char* high_score_message = TextFormat("High Score: %d", high_score);
        int high_score_fontSize = 25;
        textWidth = MeasureText(high_score_message, high_score_fontSize);
        DrawText(high_score_message, (SCREENWIDTH-textWidth)/2, (SCREENHEIGHT-high_score_fontSize+80)/2, high_score_fontSize, RAYWHITE);


    EndDrawing();

}

uint8_t get_high_score(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) { // Doesnt exist
        file = fopen(filename, "w");
        fprintf(file, "%d", 0);
        fclose(file);
        return 0;
    }
    char string[100];
    fgets(string, 100, file);
    int score = atoi(string);
    fclose(file);
    return score;
}

void write_high_score(const char* filename, uint8_t high_score) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) { 
        printf("[ERROR] File Couldnt Open!\n");
        return;
    }
    fprintf(file, "%d", high_score);
    fclose(file);
}

int main() 
{
    // init 
    srand(time(NULL));
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Mef-Snake");
    SetTargetFPS(240);
    uint8_t high_score = get_high_score(CONFIG_FILE);

respawn:
    Vector2 snake[MAX_SNAKE_LENGTH], apple_pos; uint8_t input_direction; uint8_t direction; uint32_t length;
    init(&snake[0], &direction, &length, &apple_pos, &input_direction);
    uint16_t dead_frame = 0; 
    uint8_t grid_lines = 1, state = 0;

    while (!WindowShouldClose()) {

        if (state == ALIVE) {

            Vector2 spawn_pos;
            
            Vector2 previous_pos[MAX_SNAKE_LENGTH];
            init_position_list(previous_pos, snake);

            handle_input(direction, &input_direction, &grid_lines);
            change_direction(&direction, input_direction, snake, &spawn_pos, length, previous_pos);
            move_head(snake, direction);

            // Apple
            handle_collision(&apple_pos, snake, &length, previous_pos, spawn_pos, &dead_frame, &high_score);
            if (dead_frame != 0) dead_frame++;
            if (dead_frame == 50) dead_frame = 0;

            // check lose
            if (dead_frame == 0) check_game_over(snake, length, &state);
                draw(snake, apple_pos, length, grid_lines, high_score);
            // Check Win
            check_win(length, &state);
        } 
        else if (state == DEAD) {
            if (length > high_score) high_score = length; write_high_score(CONFIG_FILE, high_score);
            uint8_t play_again = handle_menu_input();
            if (play_again == 1) goto respawn;
            draw_game_over(length, high_score);
        } 
        else if (state == WIN) {
            if (length > high_score) high_score = length; write_high_score(CONFIG_FILE, high_score);
            uint8_t play_again = handle_menu_input();
            if (play_again == 1) goto respawn;  
            draw_win_screen(length, high_score);
        }
    }
    
    CloseWindow();

    return 0;
}

