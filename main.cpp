#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533

int windowWidth = 1024;
int windowHeight = 512;

float pX, pY, pdX, pdY, pa, speed, rotSpeed;
float currentFrame, lastFrame, deltaTime;

int mapX = 8, mapY = 8, mapS = 64;
int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 1, 0, 0, 1,
    1, 0, 1, 0, 1, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 1, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};

typedef struct {
    int w, a, s, d;
} ButtonKeys;

ButtonKeys Keys;

void drawMap2D() {
    int x, y, xo, yo;
    for (y = 0; y < mapY; y++) {
        for (x = 0; x < mapX; x++) {
            //DESENHAR O TIPO 1 DO MAPA
            if (map[y * mapX + x] == 1)
                glColor3f(1, 1, 1);
            //ESPACO PRETO
            else
                glColor3f(0, 0, 0);

            glBegin(GL_QUADS);
            xo = x * mapS;
            yo = y * mapS;
            glVertex2i(xo + 1       , yo + 1);
            glVertex2i(xo + 1       , yo + mapS - 1);
            glVertex2i(xo + mapS - 1, yo + mapS - 1);
            glVertex2i(xo + mapS - 1, yo + 1);
            glEnd();
        }
    }
}

void drawPlayer() {
    glColor3f(1, 1, 0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(pX, pY);
    glEnd();

    glBegin(GL_LINES);
    glVertex2i(pX, pY);
    glVertex2i(pX + pdX * 5, pY + pdY * 5);
    glEnd();
}

void movePlayer() {
    int xOffset = 0;
    int yOffset = 0;
    
    if (pdX < 0)
        xOffset = -20;    
    else
        xOffset = 20;
    if (pdY < 0)
        yOffset = -20;
    else
        yOffset = 20;

    int ipx = pX / 64, ipx_add_xo = (pX + xOffset) / 64, ipx_sub_xo = (pX - xOffset) / 64;
    int ipy = pY / 64, ipy_add_yo = (pY + yOffset) / 64, ipy_sub_yo = (pY - yOffset) / 64;

    if (Keys.w) {
        if (map[ipy * mapX + ipx_add_xo] == 0) {
            pX += pdX * deltaTime * speed;
        }
        if (map[ipy_add_yo * mapX + ipx] == 0) {
            pY += pdY * deltaTime * speed;
        }
    }
    if (Keys.s) {
        if (map[ipy * mapX + ipx_sub_xo] == 0) {
            pX -= pdX * deltaTime * speed;
        }
        if (map[ipy_sub_yo * mapX + ipx] == 0) {
            pY -= pdY * deltaTime * speed;
        }
    }
    if (Keys.a) {
        pa -= 0.1f * deltaTime * speed;
        if (pa < 0) {
            pa += 2 * PI;
        }
        pdX = cos(pa) * 5;
        pdY = sin(pa) * 5;
    }
    if (Keys.d) {
        pa += 0.1f * deltaTime * speed;
        if (pa > 2 * PI) {
            pa -= 2 * PI;
        }
        pdX = cos(pa) * 5;
        pdY = sin(pa) * 5;
    }
}

float dist(float ax, float ay, float bx, float by, float ang) {
    return sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));
}

void drawRays2D() {
    int r, rMax, mX, mY, mP, dof;
    float rX, rY, ra, xo, yo, distF;
    rMax = 60;
    ra = pa - DR * 30;
    if (ra < 0) {
        ra += 2 * PI;
    }
    if (ra > 2 * PI) {
        ra -= 2 * PI;
    }
    for (r = 0; r < rMax; r++) {
        //HORIZONTAL
        dof = 0;
        float disH = 100000, hx = pX, hy = pY;
        float aTan = -1 / tan(ra);
        // OLHANDO PARA CIMA
        if (ra > PI) {
            rY = (((int)pY >> 6) << 6) - 0.0001;
            rX = (pY - rY) * aTan + pX;
            yo = -64;
            xo = -yo * aTan;
        }
        // OLHANDO PARA BAIXO
        if (ra < PI) {
            rY = (((int)pY >> 6) << 6) + 64;
            rX = (pY - rY) * aTan + pX;
            yo = 64;
            xo = -yo * aTan;
        }
        //OLHANDO PARA OS LADOS
        if (ra == 0 || ra == PI) {
            rX = pX;
            rY = pY;
            dof = 8;
        }
        while (dof < 8)
        {
            mX = (int)(rX) >> 6;
            mY = (int) (rY) >> 6;
            mP = mY * mapX + mX;
            // ACHOU A PAREDE
            if (mP > 0 && mP < mapX * mapY && map[mP] == 1) {
                hx = rX;
                hy = rY;
                disH = dist(pX, pY, hx, hy, ra);
                dof = 8;
            }
            //PROXIMA LINHA
            else {
                rX += xo;
                rY += yo;
                dof += 1;
            }
        }
        
        //VERTICAL
        dof = 0;
        float disV = 100000, vx = pX, vy = pY;
        float nTan = -tan(ra);
        // OLHANDO PARA ESQUERDA
        if (ra > P2 && ra < P3) {
            rX = (((int)pX >> 6) << 6) - 0.0001;
            rY = (pX - rX) * nTan + pY;
            xo = -64;
            yo = -xo * nTan;
        }
        // OLHANDO PARA DIREITA
        if (ra < P2 || ra > P3) {
            rX = (((int)pX >> 6) << 6) + 64;
            rY = (pX - rX) * nTan + pY;
            xo = 64;
            yo = -xo * nTan;
        }
        //OLHANDO PARA CIMA OU BAIXO
        if (ra == 0 || ra == PI) {
            rX = pX;
            rY = pY;
            dof = 8;
        }
        while (dof < 8)
        {
            mX = (int)(rX) >> 6;
            mY = (int)(rY) >> 6;
            mP = mY * mapX + mX;
            // ACHOU A PAREDE
            if (mP > 0 && mP < mapX * mapY && map[mP] == 1) {
                vx = rX;
                vy = rY;
                disV = dist(pX, pY, vx, vy, ra);
                dof = 8;
            }
            //PROXIMA LINHA
            else {
                rX += xo;
                rY += yo;
                dof += 1;
            }
        }

        //MOSTRAR QUEM BATEU PRIMEIRO
        if (disV < disH) {
            rX = vx;
            rY = vy;
            distF = disV;
            glColor3f(0.9, 0, 0);
        }
        if (disV > disH) {
            rX = hx;
            rY = hy;
            distF = disH;
            glColor3f(0.7, 0, 0);
        }

        glLineWidth(3); glBegin(GL_LINES); glVertex2f(pX, pY); glVertex2f(rX, rY); glEnd();

        //FAZER O CAST 3D
        float ca = pa - ra;
        if (ca < 0) {
            ca += 2 * PI;
        }
        if (ca > 2 * PI) {
            ca -= 2 * PI;
        }
        distF = distF * cos(ca);

        float lineH = (mapS * 320) / distF;
        if (lineH > 320) {
            lineH = 320;
        }
        float lineOffset = 160 - lineH / 2;
        glLineWidth(8); glBegin(GL_LINES); glVertex2i(r * 8 + 530, lineOffset); glVertex2i(r * 8 + 530, lineH + lineOffset); glEnd();

        ra += DR;
        if (ra < 0) {
            ra += 2 * PI;
        }
        if (ra > 2 * PI) {
            ra -= 2 * PI;
        }
    }
}

void display() {
    drawMap2D();
    drawRays2D();
    drawPlayer();
    movePlayer();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) Keys.w = 1;
        if (action == GLFW_RELEASE) Keys.w = 0;
    }
    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS) Keys.s = 1;
        if (action == GLFW_RELEASE) Keys.s = 0;
    }
    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS) Keys.a = 1;
        if (action == GLFW_RELEASE) Keys.a = 0;
    }
    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS) Keys.d = 1;
        if (action == GLFW_RELEASE) Keys.d = 0;
    }
}

void init() {
    glClearColor(0.3, 0.3, 0.3, 0);
    glOrtho(0.0f, windowWidth, windowHeight, 0.0f, 0.0f, 1.0f);
    pX = 320;
    pY = 240;
    pdX = cos(pa) * 5;
    pdY = sin(pa) * 5;
    speed = 10;
    rotSpeed = 10;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(windowWidth, windowHeight, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    init();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        display();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        //User Inputs
        glfwSetKeyCallback(window, key_callback);
    }

    glfwTerminate();
    return 0;
}