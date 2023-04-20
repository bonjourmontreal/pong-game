// Pong Game
//
// Compile: gcc -o ponggame ponggame.c -lncurses -lm
// Run: ./ponggame

#include <stdio.h>
#include <ncurses.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

// Define constants
#define BOARD_HEIGHT 30
#define BOARD_WIDTH 80
#define PADDLE_LENGHT 4
#define GAP_PADDLE_WALL 2
#define INITIAL_BALL_SPEED 1
#define INITIAL_BALL_THETA 35
#define PI 3.1415
#define GAME_OVER_SCORE 3

// Define enums
typedef enum
{
    UP,
    DOWN,
    STANDBY
}Direction;

// Define structs
typedef struct
{
    int x;
    int y;
}Point;

typedef struct
{
    Point segments[PADDLE_LENGHT]; // Paddle is composed of multiple points
    Direction direction;
}Paddle;

typedef struct
{
    Point location;
    double speed;
    double speedX;
    double speedY;
    double thetaDeg;
}Ball;

// Function prototypes
void init_ncurses(); // Initializes ncurses and sets up terminal for gameplay
void draw_board(bool draw_once);

void generate_paddles(Paddle *paddleLeft, Paddle *paddleRight);
void generate_ball(Ball *ball);

void draw_paddles(Paddle *paddleLeft, Paddle *paddleRight);
void draw_ball(Ball *ball);

void handle_input(Paddle *paddleLeft, Paddle *paddleRight, int userInput);

void move_paddles(Paddle *paddleLeft, Paddle *paddleRight);
void move_ball(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight);

void check_ball_top_down_wall_collision(Ball *ball);
bool check_ball_side_wall_collision(Ball *ball, int *scoreLeft, int *scoreRight);
void check_ball_paddle_collision(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight);

void reset_game(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight);
void display_score(int *scoreLeft, int *scoreRight);
void game_loop(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight, int *scoreLeft, int *scoreRight);


// Main function
int main()
{
    int scoreLeft = 0;
    int scoreRight = 0;

    Paddle paddleLeft;
    Paddle paddleRight;
    Ball ball;

    init_ncurses(); // Initializes ncurses and sets up terminal for gameplay
    generate_paddles(&paddleLeft, &paddleRight);
    generate_ball(&ball);

    game_loop(&ball, &paddleLeft, &paddleRight, &scoreLeft, &scoreRight);
    
    
    endwin(); // Clean up and close ncurses
}

// Initializes ncurses and sets up terminal for gameplay
void init_ncurses()
{
    initscr(); // Initialize the library
    cbreak(); // Disable line buffering, but allow interrupt keys
    keypad(stdscr, TRUE); // Enable special keys
    noecho(); // Don't echo user input
    curs_set(0); // Hide the cursor
    timeout(100); // Set getch() non-blocking with 100ms delay
}

void reset_game(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight)
{
    // Reset paddles and ball
    for (int i = 0; i < PADDLE_LENGHT; i++)
    {
        mvprintw(paddleLeft->segments[i].y, paddleLeft->segments[i].x, "  ");
        mvprintw(paddleRight->segments[i].y, paddleRight->segments[i].x, "  ");
        mvprintw(ball->location.y, ball->location.x, " ");
    }

    generate_paddles(paddleLeft, paddleRight);
    generate_ball(ball);
}

void generate_paddles(Paddle *paddleLeft, Paddle *paddleRight)
{
    // Set the initial direction of the paddles
    paddleLeft->direction = STANDBY;
    paddleRight->direction = STANDBY;

    // Generate the paddle segments for the left and right paddles
    for (int i = 0; i < PADDLE_LENGHT; i++)
    {
        paddleLeft->segments[i].x = GAP_PADDLE_WALL;
        paddleLeft->segments[i].y = (BOARD_HEIGHT / 2 - PADDLE_LENGHT/2 + 1) + i;

        paddleRight->segments[i].x = BOARD_WIDTH - GAP_PADDLE_WALL;
        paddleRight->segments[i].y = paddleLeft->segments[i].y;
    }
}

void generate_ball(Ball *ball)
{
    // Set initiale conditions of ball
    ball->location.x = BOARD_WIDTH / 2;
    ball->location.y = BOARD_HEIGHT / 2;
    ball->speed = INITIAL_BALL_SPEED;
    ball->thetaDeg = INITIAL_BALL_THETA;

    // Convert the angle from degrees to radians to calculate the horizontal and vertical speeds of the ball
    double thetaRad = ball->thetaDeg * PI / 180.0;
    ball->speedX = ball->speed * cos(thetaRad);
    ball->speedY = ball->speed * sin(thetaRad);
}

void game_loop(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight, int *scoreLeft, int *scoreRight)
{
    // Flag to control whether the board needs to be redrawn (only needs to be drawn once)
    bool draw_once = true;

    // Loop until one of the players scores GAME_OVER_SCORE points
    while(*scoreLeft < GAME_OVER_SCORE && *scoreRight < GAME_OVER_SCORE)
    {
        if (check_ball_side_wall_collision(ball, scoreLeft, scoreRight))
        {
            reset_game(ball, paddleLeft, paddleRight);
        }
        
        int userInput;
        userInput = tolower(getch());

        // Draw the board, paddles, ball, and display the score
        draw_board(draw_once);
        draw_once = false;
        draw_paddles(paddleLeft, paddleRight);
        handle_input(paddleLeft, paddleRight, userInput);
        move_paddles(paddleLeft, paddleRight);
        check_ball_top_down_wall_collision(ball);
        check_ball_paddle_collision(ball, paddleLeft, paddleRight);
        draw_ball(ball);
        move_ball(ball, paddleLeft, paddleRight);
        display_score(scoreLeft, scoreRight);

        // Redraw the center line after updating the ball
        for (int j = 1; j <= BOARD_HEIGHT; j++) {
            mvprintw(j, BOARD_WIDTH / 2, "|");
        }

        refresh();

        // Exit the game if the user presses 'q'
        if (userInput == 'q')
        {
            break;
        }

    }
}

void draw_board(bool draw_once)
{
    if (!draw_once) return;

    for (int j = 0; j < BOARD_HEIGHT + 2; j++)
    {
        for (int i = 0; i < BOARD_WIDTH + 2; i++)
        {
            if (j == 0 || j == BOARD_HEIGHT + 1 ||
                i == 0 || i == BOARD_WIDTH + 1)
            {
                mvprintw(j, i, "#");
            }
        }
    }
}

void draw_paddles(Paddle *paddleLeft, Paddle *paddleRight)
{
    for (int i = 0; i < PADDLE_LENGHT; i++)
    {
        mvprintw(paddleLeft->segments[i].y, paddleLeft->segments[i].x, "||");
        mvprintw(paddleRight->segments[i].y, paddleRight->segments[i].x, "||");
    }
}

void handle_input(Paddle *paddleLeft, Paddle *paddleRight, int userInput)
{
    switch (userInput)
    {
        case 'w':
            paddleLeft->direction = UP;
            break;
        case 's':
            paddleLeft->direction = DOWN;
            break;
        case KEY_UP:
            paddleRight->direction = UP;
            break;
        case KEY_DOWN:
            paddleRight->direction = DOWN;
            break;
    }
}

void move_paddles(Paddle *paddleLeft, Paddle *paddleRight)
{
    // Update left paddle
    if (paddleLeft->direction == UP && paddleLeft->segments[0].y != 1)
    {
        mvprintw(paddleLeft->segments[PADDLE_LENGHT - 1].y, paddleLeft->segments[PADDLE_LENGHT - 1].x, "  ");
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            paddleLeft->segments[i].y -= 1;
        }
    }
    else if (paddleLeft->direction == DOWN && paddleLeft->segments[PADDLE_LENGHT - 1].y != BOARD_HEIGHT)
    {
        mvprintw(paddleLeft->segments[0].y, paddleLeft->segments[0].x, "  ");
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            paddleLeft->segments[i].y += 1;
        }
    }
    // Update right paddle
    if (paddleRight->direction == UP && paddleRight->segments[0].y != 1)
    {
        mvprintw(paddleRight->segments[PADDLE_LENGHT - 1].y, paddleRight->segments[PADDLE_LENGHT - 1].x, "  ");
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            paddleRight->segments[i].y -= 1;
        }
    }
    else if (paddleRight->direction == DOWN && paddleRight->segments[PADDLE_LENGHT - 1].y != BOARD_HEIGHT)
    {
        mvprintw(paddleRight->segments[0].y, paddleRight->segments[0].x, "  ");
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            paddleRight->segments[i].y += 1;
        }
    }

    // Reset paddle direction
    paddleLeft->direction = STANDBY;
    paddleRight->direction = STANDBY;
}

void draw_ball(Ball *ball)
{
    mvprintw(ball->location.y, ball->location.x, "O");
}

void move_ball(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight)
{
    // Save previous location
    int prevX = ball->location.x;
    int prevY = ball->location.y;

    // Calculate and update new location
    double thetaRad = ball->thetaDeg * PI / 180;
    ball->speedX = ball->speed * cos(thetaRad);
    ball->speedY = ball->speed * sin(thetaRad);

    ball->location.x = round(ball->speedX + ball->location.x);
    ball->location.y = round(ball->speedY + ball->location.y);

    // Draw the ball in its new position
    draw_ball(ball);

    //Clear the previous ball position
    mvprintw(prevY, prevX, " ");
}

void check_ball_top_down_wall_collision(Ball *ball)
{
    // Calculate new angle of reflection
    if (ball->location.y == 1 || ball->location.y == BOARD_HEIGHT)
    {
        ball->thetaDeg = 360 - ball->thetaDeg;
    }
}

bool check_ball_side_wall_collision(Ball *ball, int *scoreLeft, int *scoreRight)
{
    // Left wall collision
    if (ball->location.x == 1)
    {
        (*scoreRight)++;
        return true;
    }
    // Right wall collision
    else if (ball->location.x == BOARD_WIDTH)
    {
        (*scoreLeft)++;
        return true;
    }
    else
    {
        return false;
    }
}

void check_ball_paddle_collision(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight)
{
    // Calculate new angle of reflection
    for (int i = 0; i < PADDLE_LENGHT; i++)
    {
        if (ball->location.x == paddleLeft->segments[i].x + 1 && ball->location.y == paddleLeft->segments[i].y ||
            ball->location.x == paddleRight->segments[i].x - 1 && ball->location.y == paddleRight->segments[i].y)
            {
                ball->thetaDeg = 180 - ball->thetaDeg;
            }
    }
}

void display_score(int *scoreLeft, int *scoreRight)
{
    mvprintw (2, BOARD_WIDTH / 4 - 1, "%d", *scoreLeft);
    mvprintw (2, 3 * BOARD_WIDTH / 4 - 1, "%d", *scoreRight);
}