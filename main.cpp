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
#define EARTH_RADIUS 5
#define AU_DISTANCE 20
#define EARTH_SPEED 1

// filled with an asteroid. It should be an integer between 0 and 100.
enum PlanetID {
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
PlanetID planetIDs[] = {
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
static float xVal = 0, zVal = 300; // Co-ordinates of the spacecraft.
static int isCollision = 0; // Is there collision between the spacecraft and an asteroid?
static unsigned int spacecraft; // Display lists base index.
static int frameCount = 0; // Number of frames
static float latAngle = 0.0; // Latitudinal angle.
static float planetsRotationAngle = 0.0; // Longitudinal angle.
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate scene.
static int isAnimate = 0; // Animated?
static int animationPeriod = 100; // Time interval between frames.
float planetSize[NUMBER_OF_PLANETS]={
                 5*EARTH_RADIUS,   // Sun
                 0.5f*EARTH_RADIUS,   // Mercury
                 0.9f*EARTH_RADIUS,   // Venus
                 EARTH_RADIUS,   // Earth
                 0.5f*EARTH_RADIUS,   // Mars
                 4*EARTH_RADIUS,   // Jupiter
                 3*EARTH_RADIUS,   // Saturn
                 2*EARTH_RADIUS,   // Uranus
                 2*EARTH_RADIUS,   // Neptune
                 };

float orbitalPeriods[NUMBER_OF_PLANETS] = {
        0.0f,              // Sun
        4.14773f,            // Mercury
        1.62222f,          // Venus
        1.0f,              // Earth
        0.53258f,          // Mars
        0.0844152f,      // Jupiter
        0.0339314f,      // Saturn
        0.0119189f,      // Uranus
        0.00606553f      // Neptune
};

//zPositions assume AU = 40unit in opengl
float planetPositions[NUMBER_OF_PLANETS] = {
        0,              //Sun
        -1.5f  * AU_DISTANCE,    // Mercury
        -2.0f  * AU_DISTANCE, // Venus
        -3.0f  * AU_DISTANCE, // Earth
        -4.0f  * AU_DISTANCE, // Mars
        -6.0f  * AU_DISTANCE, // Jupiter
        -10.0f  * AU_DISTANCE, // Saturn
        -14.0f * AU_DISTANCE,   // Uranus
        -16.0f * AU_DISTANCE,   // Neptune
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

struct SunLight {
    GLfloat lightPosition[4];  // Position of the light source (at the center of the Sun)
    GLfloat lightAmbient[4]; // Ambient light color
    GLfloat lightDiffuse[4];   // Diffuse light color
    GLfloat lightSpecular[4];  // Specular light color
};
SunLight sunLight{
         { 0.0f, 0.0f, 0.0f, 1.0f },  // Position of the light source (at the center of the Sun)
         { 0.2f, 0.2f, 0.0f, 1.0f },   // Ambient light color
         { 1.0f, 1.0f, 0.0f, 1.0f },   // Diffuse light color
         { 1.0f, 1.0f, 0.0f, 1.0f },  // Specular light color

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
struct SaturnRingsProp {
    float ambient[3];
    float diffuse[3];
    float shininess;
    Color color;
};
SaturnRingsProp saturnRingsProp[] = {
        {//Light Gray:
        {0.2f, 0.2f, 0.2f},
        {0.7f, 0.7f, 0.7f},
        16.0f,
        {0.7f, 0.7f, 0.7f},
},{ //Pale Yellow:
    {0.2f, 0.2f, 0.0f},
    {1.0f, 1.0f,  0.8f},
    16.0f,
            {1.0f, 1.0f,  0.8f},
},
{//Soft Orange:
    {0.2f, 0.1f, 0.0f},
    {1.0f, 0.6f,  0.4f},
    16.0f,
            {1.0f, 0.6f,  0.4f},
}
};
// Routine to draw a bitmap character string.
void writeBitmapString(void *font, char *string)
{
    char *c;

    for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}

struct Position{
    float x;
    float y;
    float z;
};
// Asteroid class.
class Asteroid
{
public:
    Asteroid();
    Asteroid(float x, float y, float z, float r, float colorR, float colorG,float colorB,
             PlanetID planetId);

    float getCenterX() { return centerX; }
    float getCenterY() { return centerY; }
    float getCenterZ() { return centerZ; }
    float getAngleY(){ return angleY; }
    float setAngleY(float angleY){ this->angleY=angleY; }
    Position getPosition() {return currentPosition;}
    void setPosition(Position position) {
        this->currentPosition.x=position.x;
        this->currentPosition.y=position.y;
        this->currentPosition.z=position.z;
    }
    float getRadius() { return radius; }
    void drawSpecialAsteroid();
    void draw();

private:
    Position currentPosition;
    PlanetID planetId;
    float centerX, centerY, centerZ, radius;
    float angleY;
    Color color;
};

// Asteroid default constructor.
Asteroid::Asteroid()
{
    centerX = 0.0;
    centerY = 0.0;
    centerZ = 0.0;
    angleY=0.0;
    currentPosition.x=centerX;
    currentPosition.y=centerY;
    currentPosition.z=centerZ;
    radius = 0.0; // Indicates no asteroid exists in the position.
    this->color={ 0,0,0};
}

// Asteroid constructor.
Asteroid::Asteroid(float x, float y, float z, float r, float colorR,
                   float colorG,float colorB,PlanetID planetId)
{
    this->planetId =planetId;
    centerX = x;
    centerY = y;
    centerZ = z;
    angleY=0.0;
    currentPosition.x=x;
    currentPosition.y=y;
    currentPosition.z=z;
    radius = r;
    this->color={ colorR,colorG,colorB};
}
void Asteroid::drawSpecialAsteroid() {
    switch(planetId){
        case Sun:
            glLightfv(GL_LIGHT0, GL_POSITION, sunLight.lightPosition);
            glLightfv(GL_LIGHT0, GL_AMBIENT, sunLight.lightAmbient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, sunLight.lightDiffuse);
            glLightfv(GL_LIGHT0, GL_SPECULAR, sunLight.lightSpecular);
            glutSolidSphere(radius, 75, 75);
            glEnable(GL_LIGHT0);
            break;
        case Earth:
            glColor3f(moonProp.color.r,moonProp.color.g,moonProp.color.b);
            glMaterialfv(GL_FRONT, GL_AMBIENT, moonProp.ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, moonProp.diffuse);
            glMaterialf(GL_FRONT, GL_SHININESS,moonProp.shininess);
            glRotatef(planetsRotationAngle,0,1,0.5);
            glTranslatef(planetSize[Earth]+5,0,0);
            glutSolidSphere(moonProp.sizeRelativeToEarth, (int)(moonProp.sizeRelativeToEarth *6), (int)(moonProp.sizeRelativeToEarth *6));
            break;
        case Saturn:
            glRotatef(100, 1, 0.0, 0.0);
            for(int i=0;i<sizeof(saturnRingsProp)/sizeof (saturnRingsProp[0]);i++) {
                glColor3f(saturnRingsProp[i].color.r, saturnRingsProp[i].color.g, saturnRingsProp[i].color.b);
                glMaterialfv(GL_FRONT, GL_AMBIENT, saturnRingsProp[i].ambient);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, saturnRingsProp[i].diffuse);
                glMaterialf(GL_FRONT, GL_SHININESS, saturnRingsProp[i].shininess);
                glutSolidTorus(1, planetSize[Saturn] + (5*(i+1))+1.0, 50, 50);
            }

            break;
        default:
            break;
    }

}
// Function to draw asteroid.
void Asteroid::draw()
{
    //TODO : handle lights
    if (radius > 0.0) // If asteroid exists.
    {
        glPushMatrix();
        glColor3f(color.r,color.g,color.b);

        glMaterialfv(GL_FRONT, GL_AMBIENT, materialProp[planetId].ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materialProp[planetId].diffuse);
        glMaterialf(GL_FRONT, GL_SHININESS,materialProp[planetId].shininess);

        glRotatef(angleY,0,1.0,0);
        glTranslatef(centerX, centerY, centerZ);
        glRotatef(planetsRotationAngle,0,1.0,0.5);
        glutSolidSphere(radius, (int)radius * 6, (int)radius * 6);
        if(planetId==Sun ||planetId ==Saturn || planetId== Earth)
            drawSpecialAsteroid();

        glPopMatrix();

    }
}

Asteroid arrayAsteroids[NUMBER_OF_PLANETS]; // Global array of asteroids.

// Routine to count the number of frames drawn every second.
void frameCounter(int value)
{
    if (value != 0) // No output the first time frameCounter() is called (from main()).
        std::cout << "FPS = " << frameCount << std::endl;
    frameCount = 0;
    glutTimerFunc(1000, frameCounter, 1);
}


// Initialization routine.
void setup(void)
{
    int i;

    spacecraft = glGenLists(1);
    glNewList(spacecraft, GL_COMPILE);
    glPushMatrix();
    glRotatef(180.0, 0.0, 1.0, 0.0); // To make the spacecraft point down the $z$-axis initially.
    glColor3f(1.0, 1.0, 1.0);
    glutSolidCone(5.0, 10.0, 10, 10);
    glPopMatrix();
    glEndList();

    // Initialize global arrayAsteroids.
    for (i = 0; i<NUMBER_OF_PLANETS; i++)
        arrayAsteroids[i]=Asteroid(0,0,planetPositions[i],planetSize[i],planetColors[i].r,planetColors[i].g,
                                   planetColors[i].b,planetIDs[i]);


    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glutTimerFunc(0, frameCounter, 0); // Initial call of frameCounter().

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST); // Enable depth testing.

    // Turn on OpenGL lighting.
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
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
    int i;

    // Check for collision with each asteroid.
    for (i = 0; i<NUMBER_OF_PLANETS; i++)
        if (arrayAsteroids[i].getRadius() > 0) // If asteroid exists.
            if (checkSpheresIntersection(x - 5 * sin((M_PI / 180.0) * a), 0.0,
                                         z - 5 * cos((M_PI / 180.0) * a), 7.072,
                                         arrayAsteroids[i].getPosition().x, arrayAsteroids[i].getPosition().y,
                                         arrayAsteroids[i].getPosition().z, arrayAsteroids[i].getRadius()))
                return 1;
    return 0;
}

// Drawing routine.
void drawScene(void)
{
    frameCount++; // Increment number of frames every redraw.
    glDisable(GL_LIGHTING);

    int i;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //begin (Solar system) viewport
    glViewport(3*width/4 , 0 , width / 4, height/4);
    glLoadIdentity();

    // Write text in isolated (i.e., before gluLookAt) translate block.
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glRasterPos3f(-28.0, 25.0, -30.0);
    if (isCollision) writeBitmapString((void*)font, "Cannot - will crash!");
    glPopMatrix();

    // Fixed camera.
    gluLookAt(0.0, 400.0, 0.0, 0.0, 0.0, 0.0,  0.0, 0.0,1.0);


    // Draw all the asteroids in arrayAsteroids.
   for (i = 0; i<NUMBER_OF_PLANETS; i++)
        arrayAsteroids[i].draw();

    // Draw spacecraft.
    glPushMatrix();
    glTranslatef(xVal, 0.0, zVal);
    glRotatef(angle, 0.0, 1.0, 0.0);
    glCallList(spacecraft);
    glPopMatrix();
    // End (Solar system) viewport

    // Begin (Space-craft view) viewport

    glViewport(0, 0, width , height);//demo
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    // Write text in isolated (i.e., before gluLookAt) translate block.
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glRasterPos3f(-28.0, 25.0, -30.0);
    if (isCollision) writeBitmapString((void*)font, "Cannot - will crash!");
    glPopMatrix();

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
    // Draw a vertical line on the left of the viewport to separate the two viewports
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(-5.0, -5.0, -5.0);
    glVertex3f(-5.0, 5.0, -5.0);
    glEnd();
    glLineWidth(1.0);


    for (i = 0; i<NUMBER_OF_PLANETS; i++)
        arrayAsteroids[i].draw();
    // End (Space-craft view) viewport.

    glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 600.0);
    glMatrixMode(GL_MODELVIEW);

    // Pass the size of the OpenGL window.
    width = w;
    height = h;
}
// Timer function.
void animate(int value)
{
    if(!isAnimate)
        return;
    //change angle for each asteriod respectively
    for(int i=0;i<NUMBER_OF_PLANETS;i++) {
        float tempAngle = arrayAsteroids[i].getAngleY() + orbitalPeriods[i];
        if (tempAngle > 360.0) tempAngle -= 360.0;
            arrayAsteroids[i].setAngleY(tempAngle);
    }

        latAngle += 5.0;
    if (latAngle > 360.0) latAngle -= 360.0;
        planetsRotationAngle += 1.0;
        if (planetsRotationAngle > 360.0) planetsRotationAngle -= 360.0;

        glutPostRedisplay();
        glutTimerFunc(animationPeriod, animate, 1);

}
// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
    switch (key)
    {
        case ' ':
            if (isAnimate) isAnimate = 0;
            else
            {
                isAnimate = 1;
                animate(1);
            }
        break;
        case 'x':
            Xangle += 5.0;
            if (Xangle > 360.0) Xangle -= 360.0;
            glutPostRedisplay();
            break;
        case 'X':
            Xangle -= 5.0;
            if (Xangle < 0.0) Xangle += 360.0;
            glutPostRedisplay();
            break;
        case 'y':
            Yangle += 5.0;
            if (Yangle > 360.0) Yangle -= 360.0;
            glutPostRedisplay();
            break;
        case 'Y':
            Yangle -= 5.0;
            if (Yangle < 0.0) Yangle += 360.0;
            glutPostRedisplay();
            break;
        case 'z':
            Zangle += 5.0;
            if (Zangle > 360.0) Zangle -= 360.0;
            glutPostRedisplay();
            break;
        case 'Z':
            Zangle -= 5.0;
            if (Zangle < 0.0) Zangle += 360.0;
            glutPostRedisplay();
            break;

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
        tempxVal = xVal - sin(angle * M_PI / 180.0) *5;
        tempzVal = zVal - cos(angle * M_PI / 180.0)*5;
    }
    if (key == GLUT_KEY_DOWN)
    {
        tempxVal = xVal + sin(angle * M_PI / 180.0)*5;
        tempzVal = zVal + cos(angle * M_PI / 180.0)*5;
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
