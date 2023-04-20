// Pong Game
//
// Compile: gcc -o ponggame ponggame.c -lncurses -lm
// Run: ./ponggame

#include <stdio.h>
#include <ncurses.h>
#include <math.h>

#define BOARD_HEIGHT 30
#define BOARD_WIDTH 80
#define PADDLE_LENGHT 4
#define GAP_PADDLE_WALL 2
#define INITIAL_BALL_SPEED 0.1
#define INITIAL_BALL_THETA 90
#define PI 3.1415

typedef enum
{
    UP,
    DOWN,
    STANDBY
}Direction;

typedef struct
{
    int x;
    int y;
}Point;

typedef struct
{
    Point segments[PADDLE_LENGHT];
    Direction direction;
}Paddle;

typedef struct
{
    Point location;
    double speed;
    double speed_x;
    double speed_y;
    double theta_degrees;
}Ball;

void init_ncurses(); // Initializes ncurses and sets up terminal for gameplay
void draw_board();
void generate_paddles(Paddle *paddle_left, Paddle *paddle_right);
void generate_ball(Ball *ball);

void draw_paddles(Paddle *paddle_left, Paddle *paddle_right);
void draw_ball(Ball *ball);
void handle_input(Paddle *paddle_left, Paddle *paddle_right, int user_input);
void move_paddles(Paddle *paddle_left, Paddle *paddle_right);
void move_ball(Ball *ball);

int main()
{
    init_ncurses();
    draw_board();

    Paddle paddle_left;
    Paddle paddle_right;
    Ball ball;
    
    generate_paddles(&paddle_left, &paddle_right);
    generate_ball(&ball);
    

    while(1)
    {
        int ch;
        ch = getch();

        draw_paddles(&paddle_left, &paddle_right);
        draw_ball(&ball);

        handle_input(&paddle_left, &paddle_right, ch);
        move_paddles(&paddle_left, &paddle_right);
        move_ball(&ball);

        if (ch == 'q')
        {
            break;
        }
    }
    
    // Clean up and close ncurses
    endwin();
}

// Initializes ncurses and sets up terminal for gameplay
void init_ncurses()
{
    initscr(); // Initialize the library
    cbreak(); // Disable line buffering, but allow interrupt keys
    keypad(stdscr, TRUE); // Enable special keys
    noecho(); // Don't echo user input
    curs_set(0); // Hide the cursor
    timeout(300); // Set getch() non-blocking with 100ms delay
}

void draw_board()
{
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

void generate_paddles(Paddle *paddle_left, Paddle *paddle_right)
{
    paddle_left->direction = STANDBY;
    paddle_right->direction = STANDBY;

    for (int i = 0; i < PADDLE_LENGHT; i++)
    {
        paddle_left->segments[i].x = GAP_PADDLE_WALL;
        paddle_left->segments[i].y = (BOARD_HEIGHT / 2 - PADDLE_LENGHT/2 + 1) + i;

        paddle_right->segments[i].x = BOARD_WIDTH - GAP_PADDLE_WALL;
        paddle_right->segments[i].y = paddle_left->segments[i].y;
    }
}

void draw_paddles(Paddle *paddle_left, Paddle *paddle_right)
{
    for (int i = 0; i < PADDLE_LENGHT; i++)
    {
        mvprintw(paddle_left->segments[i].y, paddle_left->segments[i].x, "||");
        mvprintw(paddle_right->segments[i].y, paddle_right->segments[i].x, "||");
    }

    paddle_left->direction = STANDBY;
    paddle_right->direction = STANDBY;

}

void handle_input(Paddle *paddle_left, Paddle *paddle_right, int user_input)
{
    switch (user_input)
    {
        case 'w':
            paddle_left->direction = UP;
            break;
        case 's':
            paddle_left->direction = DOWN;
            break;
        case KEY_UP:
            paddle_right->direction = UP;
            break;
        case KEY_DOWN:
            paddle_right->direction = DOWN;
            break;
        default:
            paddle_left->direction = STANDBY;
            paddle_right->direction = STANDBY;
            break;
    }
}

void move_paddles(Paddle *paddle_left, Paddle *paddle_right)
{
    // Update left paddle
    if (paddle_left->direction == UP && paddle_left->segments[0].y != 1)
    {
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            mvprintw(paddle_left->segments[PADDLE_LENGHT - 1].y, paddle_left->segments[PADDLE_LENGHT - 1].x, "  ");
            paddle_left->segments[i].y -= 1;
        }
    }
    else if (paddle_left->direction == DOWN && paddle_left->segments[PADDLE_LENGHT - 1].y != BOARD_HEIGHT)
    {
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            mvprintw(paddle_left->segments[0].y, paddle_left->segments[0].x, "  ");
            paddle_left->segments[i].y += 1;
        }
    }
    // Update right paddle
    if (paddle_right->direction == UP && paddle_right->segments[0].y != 1)
    {
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            mvprintw(paddle_right->segments[PADDLE_LENGHT - 1].y, paddle_right->segments[PADDLE_LENGHT - 1].x, "  ");
            paddle_right->segments[i].y -= 1;
        }
    }
    else if (paddle_right->direction == DOWN && paddle_right->segments[PADDLE_LENGHT - 1].y != BOARD_HEIGHT)
    {
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            mvprintw(paddle_right->segments[0].y, paddle_right->segments[0].x, "  ");
            paddle_right->segments[i].y += 1;
        }
    }
}

void generate_ball(Ball *ball)
{
    ball->location.x = BOARD_WIDTH / 2;
    ball->location.y = BOARD_HEIGHT / 2;
    ball->speed = INITIAL_BALL_SPEED;
    ball->theta_degrees = INITIAL_BALL_THETA;

    int ball_theta_radians = ball->theta_degrees * PI / 180.0;
    ball->speed_x = ball->speed * cos(ball_theta_radians);
    ball->speed_y = ball->speed * sin(ball_theta_radians);
}

void draw_ball(Ball *ball)
{
    mvprintw(ball->location.y, ball->location.x, "o");
}

void move_ball(Ball *ball)
{
    // mvprintw(ball->location.y, ball->location.x, " ");

    // Check collision TOP and BOTTOM Wall
    if (ball->location.y == 1 || ball->location.y == BOARD_HEIGHT)
    {
        ball->theta_degrees = 360 - ball->theta_degrees;
    }

    if (ball->location.x == 1 || ball->location.x == BOARD_WIDTH)
    {
        ball->theta_degrees = 180 - ball->theta_degrees;
    }

    ball->location.x += ball->speed_x;
    ball->location.y += ball->speed_y;

}









