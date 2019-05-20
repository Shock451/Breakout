//
// breakout.c
//
// Computer Science 50
// Problem Set 3
//
 
// standard libraries
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
 
// Stanford Portable Library
#include <spl/gevents.h>
#include <spl/gobjects.h>
#include <spl/gwindow.h>
 
// height and width of game's window in pixels
#define HEIGHT 600
#define WIDTH 400
 
// height and width of game's paddle in pixels
#define PWIDTH 60
#define PHEIGHT 10
 
// number of rows of bricks
#define ROWS 5
 
// number of columns of bricks
#define COLS 10
 
// radius of ball in pixels
#define RADIUS 10
 
// lives
#define LIVES 3
 
// prototypes
void initBricks(GWindow window);
GOval initBall(GWindow window);
GRect initPaddle(GWindow window);
GLabel initScoreboard(GWindow window);
void updateScoreboard(GWindow window, GLabel label, int points);
GObject detectCollision(GWindow window, GOval ball);
void removeGWindow(GWindow gw, GObject gobj);
 
int main(void)
{
    // seed pseudorandom number generator
    srand48(time(NULL));
 
    // instantiate window
    GWindow window = newGWindow(WIDTH, HEIGHT);
 
    // instantiate bricks
    initBricks(window);
 
    // instantiate ball, centered in middle of window
    GOval ball = initBall(window);
 
    // instantiate paddle, centered at bottom of window
    GRect paddle = initPaddle(window);
 
    // instantiate scoreboard, centered in middle of window, just above ball
    GLabel label = initScoreboard(window);
 
    // number of bricks initially
    int bricks = COLS * ROWS;
 
    // number of lives initially
    int lives = LIVES;
 
    // number of points initially
    int points = 0;
     
    // ball hor and ver velocities    
    double Yvelocity = 2.5;
    double Xvelocity = 0.5+drand48();
    // keep playing until game over
    updateScoreboard(window, label, points);
    waitForClick();
    while (lives > 0 && bricks > 0)
    {                
        // Mouse movements for paddle
        GEvent event = getNextEvent(MOUSE_EVENT);
        if (event != NULL)
        {
            if (getEventType(event) == MOUSE_MOVED)
            {
                double x = getX(event) - getWidth(paddle) / 2;
                setLocation(paddle, x, HEIGHT - (PHEIGHT * 4));
            }
        }
        // move along x axis
        move(ball, Xvelocity, Yvelocity);
        
        // bounce off right edge of window
        if (getX(ball) + getWidth(ball) >= getWidth(window) || getX(ball) <= 0)
        {
            Xvelocity = -Xvelocity;
            pause(10);
        }
        else if (getY(ball) <= 0)
        {
            Yvelocity = -Yvelocity;
            pause(10);   
        }
         
        // lose a life when paddle mises ball
        else if (getY(ball) + getHeight(ball) >= (getY(paddle) + PHEIGHT) - 3)
        {
            // lose life
            lives--;
            // Reset paddle position
            setLocation(paddle,  (WIDTH / 2) - (PWIDTH / 2), HEIGHT - (PHEIGHT * 4));
            // Reset ball position
            setLocation(ball,(WIDTH / 2) - 10, (HEIGHT / 2) - 10);
            // Wait for user to click to continue
            waitForClick();
        }
        // Slow things a bit
        pause(10);
         
        // Collision detector
        GObject object = detectCollision( window, ball);
            
        // And some error checking
        if (object != NULL)
        {
            // direction change when ball hits pad
            if (object == paddle)
            {
                Yvelocity = -Yvelocity;
            }    
            // ball on paddle
            else if (strcmp(getType(object), "GLabel") == 0)
            {
                Yvelocity*=1;
                Xvelocity*=1;
            }
            else
            {
                // Removal of destroyed brick
                removeGWindow(window, object);
                Yvelocity = -Yvelocity;
                // Updates for points and bricks
                points++;
                updateScoreboard(window, label, points);
                bricks--;       
            }
        }
         
    }    
             
    // You no longer need the ball. You won..or lost :-p
    removeGWindow( window, ball);
     
    // wait for click before exiting
    waitForClick();
 
    // game over
    closeGWindow(window);
    return 0;
 
}
 
/**
 * Initializes window with a grid of bricks.
 */
void initBricks(GWindow window)
{
    // Here comes the bricks
    char* colors[5] = { "BLUE", "YELLOW", "GREEN", "CYAN", "PINK"};
    for(int n = 0; n < ROWS; n++)
    {
        for (int i = 5; i < COLS * 40; i+=40)
        {
            GRect bricks = newGRect(i, 50 + ((n * 60) / 4), 35, 10);
            setFilled( bricks, true);
            setColor( bricks, colors[n]);
            add(window, bricks);     
        }
    }
}
 
/**
 * Instantiates ball in center of window.  Returns ball.
 */
GOval initBall(GWindow window)
{
    // One ball for you...
    GOval ball = newGOval( (WIDTH / 2) - 10, (HEIGHT / 2) - 10, 20, 20); 
    setFilled(ball, true);
    setColor( ball, "BLACK");
    add(window, ball);
    return ball;
}
 
/**
 * Instantiates paddle in bottom-middle of window.
 */
GRect initPaddle(GWindow window)
{
    // Here's a paddle
    GRect paddle = newGRect((WIDTH / 2) - (PWIDTH / 2), HEIGHT - (PHEIGHT * 4), PWIDTH, PHEIGHT);
    setFilled( paddle, true);
    setColor( paddle, "BLACK");
    add( window, paddle );
    return paddle;
}
 
/**
 * Instantiates, configures, and returns label for scoreboard.
 */
GLabel initScoreboard(GWindow window)
{
    GLabel label = newGLabel(""); 
    setFont(label, "SansSerif-30");
    setColor(label, "RED");
    add(window, label);
    return label;
}
 
/**
 * Updates scoreboard's label, keeping it centered in window.
 */
void updateScoreboard(GWindow window, GLabel label, int points)
{
    // update label
    char s[12];
    sprintf(s, "%i", points);
    setLabel(label, s);
 
    // center label in window
    double x = (getWidth(window) - getWidth(label)) / 2;
    double y = (getHeight(window) - getHeight(label)) / 2;
    setLocation(label, x, y);
}
 
/**
 * Detects whether ball has collided with some object in window
 * by checking the four corners of its bounding box (which are
 * outside the ball's GOval, and so the ball can't collide with
 * itself).  Returns object if so, else NULL.
 */
GObject detectCollision(GWindow window, GOval ball)
{
    // ball's location
    double x = getX(ball);
    double y = getY(ball);
 
    // for checking for collisions
    GObject object;
 
    // check for collision at ball's top-left corner
    object = getGObjectAt(window, x, y);
    if (object != NULL)
    {
        return object;
    }
 
    // check for collision at ball's top-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y);
    if (object != NULL)
    {
        return object;
    }
 
    // check for collision at ball's bottom-left corner
    object = getGObjectAt(window, x, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }
 
    // check for collision at ball's bottom-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }
 
    // no collision
    return NULL;
}
