#define _USE_MATH_DEFINES

#include <cstdlib>
#include <cmath>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#define ROWS 8  // Number of rows of asteroids.
#define COLUMNS 6 // Number of columns of asteroids.
#define FILL_PROBABILITY 100 // Percentage probability that a particular row-column slot will be
#define NUMBER_OF_PLANETS 9  //including sun
#define EARTH_RADIUS 100
#define EARTH_SPEED 1

// filled with an asteroid. It should be an integer between 0 and 100.
enum Planets {
    Sun,
    Mercury,
    Venus,
    Earth,
    Mars,
    Jupiter,
    Saturn,
    Uranus,
    Neptune
};

// Globals.
static uintptr_t font = (uintptr_t)GLUT_BITMAP_8_BY_13; // Font selection
static int width, height; // Size of the OpenGL window.
static float angle = 0.0; // Angle of the spacecraft.
static float xVal = 0, zVal = 1000; // Co-ordinates of the spacecraft.
static int isCollision = 0; // Is there collision between the spacecraft and an asteroid?
static unsigned int spacecraft; // Display lists base index.
static int frameCount = 0; // Number of frames

float planetSize[NUMBER_OF_PLANETS]={
                EARTH_RADIUS * 109.0,   // Sun
                EARTH_RADIUS * 0.38,    // Mercury
                EARTH_RADIUS * 0.95,    // Venus
                EARTH_RADIUS * 1.0,     // Earth
                EARTH_RADIUS * 0.53,    // Mars
                EARTH_RADIUS * 11.2,    // Jupiter
                EARTH_RADIUS * 9.45,    // Saturn
                EARTH_RADIUS * 4.0,     // Uranus
                EARTH_RADIUS * 3.88     // Neptune
                 };

float orbitalPeriods[NUMBER_OF_PLANETS] = {
        0,                  //Sun
        365.0f / 88.0f,     // Mercury
        365.0f / 225.0f,    // Venus
        365.0f / 365.0f,    // Earth
        365.0f / 687.0f,    // Mars
        365.0f / 4333.0f,   // Jupiter
        365.0f / 10759.0f,  // Saturn
        365.0f / 30687.0f,  // Uranus
        365.0f / 60190.0f   // Neptune
};

//zPositions
float planetPositions[NUMBER_OF_PLANETS] = {
        0,              //Sun
        -0.39f * 100,    // Mercury
        -0.72f * 100,    // Venus
        -1.00f * 100,    // Earth
        -1.52f * 100,    // Mars
        -5.20f * 100,    // Jupiter
        -9.58f * 100,    // Saturn
        -19.18f * 100,   // Uranus
        -30.07f * 100    // Neptune
};

struct Color {
    float r;
    float g;
    float b;
};
Color planetColors[] = {
        {1.0f, 0.9f, 0.0f},    // Sun (Yellow)
        {0.6f, 0.6f, 0.6f},    // Mercury (Gray)
        {0.9f, 0.7f, 0.0f},    // Venus (Orange)
        {0.0f, 0.0f, 1.0f},    // Earth (Blue)
        {0.7f, 0.2f, 0.0f},    // Mars (Reddish)
        {0.8f, 0.6f, 0.4f},    // Jupiter (Brownish)
        {0.9f, 0.8f, 0.6f},    // Saturn (Pale Yellow)
        {0.7f, 0.9f, 1.0f},    // Uranus (Light Blue)
        {0.1f, 0.3f, 0.7f}     // Neptune (Dark Blue)
};
struct MaterialProp {
    float ambient[3];
    float diffuse[3];
    float shininess;
};
MaterialProp materialProp[] = {
        {{0.2f, 0.2f, 0.2f}, {0.8f, 0.8f, 0.8f}, 32.0f},   // Sun
        {{0.1f, 0.1f, 0.1f}, {0.5f, 0.5f, 0.5f}, 16.0f},   // Mercury
        {{0.15f, 0.15f, 0.15f}, {0.6f, 0.6f, 0.6f}, 32.0f},   // Venus
        {{0.2f, 0.2f, 0.2f}, {0.7f, 0.7f, 0.7f}, 64.0f},   // Earth
        {{0.15f, 0.15f, 0.15f}, {0.6f, 0.6f, 0.6f}, 32.0f},   // Mars
        {{0.2f, 0.2f, 0.2f}, {0.8f, 0.8f, 0.8f}, 128.0f},   // Jupiter
        {{0.2f, 0.2f, 0.2f}, {0.8f, 0.8f, 0.8f}, 64.0f},   // Saturn
        {{0.2f, 0.2f, 0.2f}, {0.8f, 0.8f, 0.8f}, 32.0f},   // Uranus
        {{0.2f, 0.2f, 0.2f}, {0.8f, 0.8f, 0.8f}, 32.0f}    // Neptune
};
struct MoonProp {
    float ambient[3];
    float diffuse[3];
    float shininess;
    Color color;
    float sizeRelativeToEarth;
};
MoonProp moonProp = {
        {0.1f, 0.1f, 0.1f},   // Ambient (Dark gray)
        {0.6f, 0.6f, 0.6f},   // Diffuse (Light gray)
        16.0f,                // Shininess
        {0.7f, 0.7f, 0.7f},   // Color (Gray)
        EARTH_RADIUS*0.27
};
// Routine to draw a bitmap character string.
void writeBitmapString(void *font, char *string)
{
    char *c;

    for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}

// Asteroid class.
class Asteroid
{
public:
    Asteroid();
    Asteroid(float x, float y, float z, float r, unsigned char colorR,
             unsigned char colorG, unsigned char colorB);
    float getCenterX() { return centerX; }
    float getCenterY() { return centerY; }
    float getCenterZ() { return centerZ; }
    float getRadius() { return radius; }
    void draw();

private:
    float centerX, centerY, centerZ, radius;
    unsigned char color[3];
};

// Asteroid default constructor.
Asteroid::Asteroid()
{
    centerX = 0.0;
    centerY = 0.0;
    centerZ = 0.0;
    radius = 0.0; // Indicates no asteroid exists in the position.
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
}

// Asteroid constructor.
Asteroid::Asteroid(float x, float y, float z, float r, unsigned char colorR,
                   unsigned char colorG, unsigned char colorB)
{
    centerX = x;
    centerY = y;
    centerZ = z;
    radius = r;
    color[0] = colorR;
    color[1] = colorG;
    color[2] = colorB;
}

// Function to draw asteroid.
void Asteroid::draw()
{
    if (radius > 0.0) // If asteroid exists.
    {
        glPushMatrix();
        glTranslatef(centerX, centerY, centerZ);
        glColor3ubv(color);
        glutWireSphere(radius, (int)radius * 6, (int)radius * 6);
        glPopMatrix();
    }
}

Asteroid arrayAsteroids[ROWS][COLUMNS]; // Global array of asteroids.

// Routine to count the number of frames drawn every second.
void frameCounter(int value)
{
    if (value != 0) // No output the first time frameCounter() is called (from main()).
        std::cout << "FPS = " << frameCount << std::endl;
    frameCount = 0;
    glutTimerFunc(1000, frameCounter, 1);
}

void initializePlanets(){

}

// Initialization routine.
void setup(void)
{
    int i, j;

    spacecraft = glGenLists(1);
    glNewList(spacecraft, GL_COMPILE);
    glPushMatrix();
    glRotatef(180.0, 0.0, 1.0, 0.0); // To make the spacecraft point down the $z$-axis initially.
    glColor3f(1.0, 1.0, 1.0);
    glutWireCone(5.0, 10.0, 10, 10);
    glPopMatrix();
    glEndList();

    // Initialize global arrayAsteroids.
    for (j = 0; j<COLUMNS; j++)
        for (i = 0; i<ROWS; i++)
            if (rand() % 100 < FILL_PROBABILITY)
                // If rand()%100 >= FILL_PROBABILITY the default constructor asteroid remains in the slot
                // which indicates that there is no asteroid there because the default's radius is 0.
            {
                // Position the asteroids depending on if there is an even or odd number of columns
                // so that the spacecraft faces the middle of the asteroid field.
                if (COLUMNS % 2) // Odd number of columns.
                    arrayAsteroids[i][j] = Asteroid(30.0*(-COLUMNS / 2 + j), 0.0, -40.0 - 30.0*i, 3.0,
                                                    rand() % 256, rand() % 256, rand() % 256);
                else // Even number of columns.
                    arrayAsteroids[i][j] = Asteroid(15 + 30.0*(-COLUMNS / 2 + j), 0.0, -40.0 - 30.0*i, 3.0,
                                                    rand() % 256, rand() % 256, rand() % 256);
            }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glutTimerFunc(0, frameCounter, 0); // Initial call of frameCounter().
}

// Function to check if two spheres centered at (x1,y1,z1) and (x2,y2,z2) with
// radius r1 and r2 intersect.
int checkSpheresIntersection(float x1, float y1, float z1, float r1,
                             float x2, float y2, float z2, float r2)
{
    return ((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) + (z1 - z2)*(z1 - z2) <= (r1 + r2)*(r1 + r2));
}

// Function to check if the spacecraft collides with an asteroid when the center of the base
// of the craft is at (x, 0, z) and it is aligned at an angle a to to the -z direction.
// Collision detection is approximate as instead of the spacecraft we use a bounding sphere.
int asteroidCraftCollision(float x, float z, float a)
{
    int i, j;

    // Check for collision with each asteroid.
    for (j = 0; j<COLUMNS; j++)
        for (i = 0; i<ROWS; i++)
            if (arrayAsteroids[i][j].getRadius() > 0) // If asteroid exists.
                if (checkSpheresIntersection(x - 5 * sin((M_PI / 180.0) * a), 0.0,
                                             z - 5 * cos((M_PI / 180.0) * a), 7.072,
                                             arrayAsteroids[i][j].getCenterX(), arrayAsteroids[i][j].getCenterY(),
                                             arrayAsteroids[i][j].getCenterZ(), arrayAsteroids[i][j].getRadius()))
                    return 1;
    return 0;
}

// Drawing routine.
void drawScene(void)
{
    frameCount++; // Increment number of frames every redraw.

    int i, j;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Beg	in left viewport.
    glViewport(0, 0, width , height);//demo
    glLoadIdentity();

    // Write text in isolated (i.e., before gluLookAt) translate block.
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glRasterPos3f(-28.0, 25.0, -30.0);
    if (isCollision) writeBitmapString((void*)font, "Cannot - will crash!");
    glPopMatrix();

    // Fixed camera.
    gluLookAt(0.0, 10.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // Draw all the asteroids in arrayAsteroids.
    for (j = 0; j<COLUMNS; j++)
        for (i = 0; i<ROWS; i++)
            arrayAsteroids[i][j].draw();

    // Draw spacecraft.
    glPushMatrix();
    glTranslatef(xVal, 0.0, zVal);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glCallList(spacecraft);
    glPopMatrix();
    // End left viewport.

    // Begin right viewport.
    glViewport(3*width/4 , height/4 , width / 4, height/4);
    glLoadIdentity();

    // Write text in isolated (i.e., before gluLookAt) translate block.
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glRasterPos3f(-28.0, 25.0, -30.0);
    if (isCollision) writeBitmapString((void*)font, "Cannot - will crash!");
    glPopMatrix();

    // Draw a vertical line on the left of the viewport to separate the two viewports
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(-5.0, -5.0, -5.0);
    glVertex3f(-5.0, 5.0, -5.0);
    glEnd();
    glLineWidth(1.0);

    // Locate the camera at the tip of the cone and pointing in the direction of the cone.
    gluLookAt(xVal - 10 * sin((M_PI / 180.0) * angle),
              0.0,
              zVal - 10 * cos((M_PI / 180.0) * angle),
              xVal - 11 * sin((M_PI / 180.0) * angle),
              0.0,
              zVal - 11 * cos((M_PI / 180.0) * angle),
              0.0,
              1.0,
              0.0);

    // Draw all the asteroids in arrayAsteroids.
    for (j = 0; j<COLUMNS; j++)
        for (i = 0; i<ROWS; i++)
            arrayAsteroids[i][j].draw();
    // End right viewport.

    glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 250.0);
    glMatrixMode(GL_MODELVIEW);

    // Pass the size of the OpenGL window.
    width = w;
    height = h;
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:
            exit(0);
            break;
        default:
            break;
    }
}

// Callback routine for non-ASCII key entry.
void specialKeyInput(int key, int x, int y)
{
    float tempxVal = xVal, tempzVal = zVal, tempAngle = angle;

    // Compute next position.
    if (key == GLUT_KEY_LEFT) tempAngle = angle + 5.0;
    if (key == GLUT_KEY_RIGHT) tempAngle = angle - 5.0;
    if (key == GLUT_KEY_UP)
    {
        tempxVal = xVal - sin(angle * M_PI / 180.0);
        tempzVal = zVal - cos(angle * M_PI / 180.0);
    }
    if (key == GLUT_KEY_DOWN)
    {
        tempxVal = xVal + sin(angle * M_PI / 180.0);
        tempzVal = zVal + cos(angle * M_PI / 180.0);
    }

    // Angle correction.
    if (tempAngle > 360.0) tempAngle -= 360.0;
    if (tempAngle < 0.0) tempAngle += 360.0;

    // Move spacecraft to next position only if there will not be collision with an asteroid.
    if (!asteroidCraftCollision(tempxVal, tempzVal, tempAngle))
    {
        isCollision = 0;
        xVal = tempxVal;
        zVal = tempzVal;
        angle = tempAngle;
    }
    else isCollision = 1;

    glutPostRedisplay();
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
    std::cout << "Interaction:" << std::endl;
    std::cout << "Press the left/right arrow keys to turn the craft." << std::endl
              << "Press the up/down arrow keys to move the craft." << std::endl;
}

// Main routine.
int main(int argc, char **argv)
{
    printInteraction();
    glutInit(&argc, argv);

    glutInitContextVersion(4, 3);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 400);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("spaceTravel.cpp");
    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);
    glutSpecialFunc(specialKeyInput);

    glewExperimental = GL_TRUE;
    glewInit();

    setup();

    glutMainLoop();
}
