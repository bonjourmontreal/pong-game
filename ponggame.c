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
#define INITIAL_BALL_SPEED 1
#define INITIAL_BALL_THETA 35
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
    double speedX;
    double speedY;
    double thetaDeg;
}Ball;

void init_ncurses(); // Initializes ncurses and sets up terminal for gameplay
void draw_board();
void generate_paddles(Paddle *paddleLeft, Paddle *paddleRight);
void generate_ball(Ball *ball);

void draw_paddles(Paddle *paddleLeft, Paddle *paddleRight);
void draw_ball(Ball *ball);
void handle_input(Paddle *paddleLeft, Paddle *paddleRight, int userInput);
void move_paddles(Paddle *paddleLeft, Paddle *paddleRight);
void move_ball(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight);

void check_ball_top_down_wall_collision(Ball *ball);
void check_ball_side_wall_collision(Ball *ball, Paddle **paddleLeft, Paddle **paddleRight, int scoreLeft, int scoreRight);
void check_ball_paddle_collision(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight);
void reset_game(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight);
void game_loop(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight, int *scoreLeft, int *scoreRight);

int main()
{
    int scoreLeft = 0;
    int scoreRight = 0;

    init_ncurses();
    draw_board();

    Paddle paddleLeft;
    Paddle paddleRight;
    Ball ball;
    
    reset_game(&ball, &paddleLeft, &paddleRight);
    game_loop(&ball, &paddleLeft, &paddleRight, &scoreLeft, &scoreRight);
    
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
    timeout(100); // Set getch() non-blocking with 100ms delay
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

            if (j > 0 && j <= BOARD_HEIGHT && i == BOARD_WIDTH / 2)
            {
                mvprintw(j, i, "|");
            }
        }
    }
}

void generate_paddles(Paddle *paddleLeft, Paddle *paddleRight)
{
    paddleLeft->direction = STANDBY;
    paddleRight->direction = STANDBY;

    for (int i = 0; i < PADDLE_LENGHT; i++)
    {
        paddleLeft->segments[i].x = GAP_PADDLE_WALL;
        paddleLeft->segments[i].y = (BOARD_HEIGHT / 2 - PADDLE_LENGHT/2 + 1) + i;

        paddleRight->segments[i].x = BOARD_WIDTH - GAP_PADDLE_WALL;
        paddleRight->segments[i].y = paddleLeft->segments[i].y;
    }
}

void draw_paddles(Paddle *paddleLeft, Paddle *paddleRight)
{
    for (int i = 0; i < PADDLE_LENGHT; i++)
    {
        mvprintw(paddleLeft->segments[i].y, paddleLeft->segments[i].x, "||");
        mvprintw(paddleRight->segments[i].y, paddleRight->segments[i].x, "||");
    }

    paddleLeft->direction = STANDBY;
    paddleRight->direction = STANDBY;

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
        default:
            paddleLeft->direction = STANDBY;
            paddleRight->direction = STANDBY;
            break;
    }
}

void move_paddles(Paddle *paddleLeft, Paddle *paddleRight)
{
    // Update left paddle
    if (paddleLeft->direction == UP && paddleLeft->segments[0].y != 1)
    {
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            mvprintw(paddleLeft->segments[PADDLE_LENGHT - 1].y, paddleLeft->segments[PADDLE_LENGHT - 1].x, "  ");
            paddleLeft->segments[i].y -= 1;
        }
    }
    else if (paddleLeft->direction == DOWN && paddleLeft->segments[PADDLE_LENGHT - 1].y != BOARD_HEIGHT)
    {
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            mvprintw(paddleLeft->segments[0].y, paddleLeft->segments[0].x, "  ");
            paddleLeft->segments[i].y += 1;
        }
    }
    // Update right paddle
    if (paddleRight->direction == UP && paddleRight->segments[0].y != 1)
    {
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            mvprintw(paddleRight->segments[PADDLE_LENGHT - 1].y, paddleRight->segments[PADDLE_LENGHT - 1].x, "  ");
            paddleRight->segments[i].y -= 1;
        }
    }
    else if (paddleRight->direction == DOWN && paddleRight->segments[PADDLE_LENGHT - 1].y != BOARD_HEIGHT)
    {
        for (int i = 0; i < PADDLE_LENGHT; i++)
        {
            mvprintw(paddleRight->segments[0].y, paddleRight->segments[0].x, "  ");
            paddleRight->segments[i].y += 1;
        }
    }
}

void generate_ball(Ball *ball)
{
    ball->location.x = BOARD_WIDTH / 2;
    ball->location.y = BOARD_HEIGHT / 2;
    ball->speed = INITIAL_BALL_SPEED;
    ball->thetaDeg = INITIAL_BALL_THETA;

    double thetaRad = ball->thetaDeg * PI / 180.0;
    ball->speedX = ball->speed * cos(thetaRad);
    ball->speedY = ball->speed * sin(thetaRad);
}

void draw_ball(Ball *ball)
{
    mvprintw(ball->location.y, ball->location.x, "O");
}

void move_ball(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight)
{
    int prevX = ball->location.x;
    int prevY = ball->location.y;

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
    if (ball->location.y == 1 || ball->location.y == BOARD_HEIGHT)
    {
        ball->thetaDeg = 360 - ball->thetaDeg;
    }
}

void check_ball_side_wall_collision(Ball *ball, Paddle **paddleLeft, Paddle **paddleRight, int scoreLeft, int scoreRight)
{
    if (ball->location.x == 1)
    {
        scoreRight ++;
        reset_game(ball, *paddleLeft, *paddleRight);
    }
    else if (ball->location.x == BOARD_WIDTH)
    {
        scoreRight ++;
        reset_game(ball, *paddleLeft, *paddleRight);
    }
}

void check_ball_paddle_collision(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight)
{
    for (int i = 0; i < PADDLE_LENGHT; i++)
    {
        if (ball->location.x == paddleLeft->segments[i].x + 1 && ball->location.y == paddleLeft->segments[i].y ||
            ball->location.x == paddleRight->segments[i].x - 1 && ball->location.y == paddleRight->segments[i].y)
            {
                ball->thetaDeg = 180 - ball->thetaDeg;
            }
    }
}

void reset_game(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight)
{
    generate_paddles(paddleLeft, paddleRight);
    generate_ball(ball);
}

void game_loop(Ball *ball, Paddle *paddleLeft, Paddle *paddleRight, int *scoreLeft, int *scoreRight)
{
    while(*scoreLeft < 3 || *scoreRight < 3)
    {
        int userInput;
        userInput = getch();

        draw_board();

        draw_paddles(paddleLeft, paddleRight);
        draw_ball(ball);

        handle_input(paddleLeft, paddleRight, userInput);
        move_paddles(paddleLeft, paddleRight);
        move_ball(ball, paddleLeft, paddleRight);
        check_ball_top_down_wall_collision(ball);
        check_ball_side_wall_collision(ball, &paddleLeft, &paddleRight, *scoreLeft, *scoreRight);
        check_ball_paddle_collision(ball, paddleLeft, paddleRight);

        refresh();

        if (userInput == 'q')
        {
            break;
        }
    }
}


