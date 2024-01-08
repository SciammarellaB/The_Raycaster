#include <GLFW/glfw3.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "mapa.hpp"
#include "mapaTexturas.hpp"

#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DegreeRad 0.0174533

int screenWidth = 640, screenHeight = 480, screenHeightFactor = screenHeight;
double lastFrame, currentFrame, deltaTime = 0;

//BOTOES POSSIVEIS DO JOGO
typedef struct buttonKeys {
    int forward, backward, left, right, action;
};
buttonKeys input;

//JOGADOR
float playerX, playerY, playerAngle, playerDX, playerDY, playerSpeed, playerRotateSpeed, playerDetectionDistance, playerColliderRadius;

void initialize() {
    glClearColor(0, 0, 0, 0);
    playerX = 96;
    playerY = 96;
    playerAngle = 0;
    playerSpeed = 135;
    playerRotateSpeed = 5;
    playerDetectionDistance = 30;
    playerColliderRadius = 20;
}

float dist(float ax, float ay, float bx, float by, float ang) {
    return sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));
}

void drawBackground() {
    //CEU
    glColor3f(0.34, 0.54, 0.70);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(screenWidth, 0);
    glVertex2i(screenWidth, screenHeight / 2);
    glVertex2i(0, screenHeight / 2);
    glEnd();

    //CHAO
    glColor3f(0.70, 0.70, 0.70);
    glBegin(GL_QUADS);
    glVertex2i(0, screenHeight / 2);
    glVertex2i(screenWidth, screenHeight / 2);
    glVertex2i(screenWidth, screenHeight);
    glVertex2i(0, screenHeight);
    glEnd();
}

void drawMap2D() {
    int x, y, xOffset, yOffset;
    for (x = 0; x < mapWidth; x++) {
        for (y = 0; y < mapWidth; y++) {
            //DESENHAR OS VALORES ACIMA DE 0
            if (walls[y * mapWidth + x] > 0)
                glColor3f(1, 1, 1);
            //QUANDO 0 FICAR PRETO
            else
                glColor3f(0, 0, 0);

            //DESENHAR
            glBegin(GL_QUADS);
            xOffset = x * tileSize;
            yOffset = y * tileSize;
            glVertex2i(xOffset, yOffset);
            glVertex2i(xOffset, yOffset + tileSize);
            glVertex2i(xOffset + tileSize, yOffset + tileSize);
            glVertex2i(xOffset + tileSize, yOffset);
            glEnd();
        }
    }
}

void drawPlayer() {
    glColor3f(1, 1, 0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(playerX, playerY);
    glEnd();

    glBegin(GL_LINES);
    glVertex2i(playerX, playerY);
    glVertex2i(playerX + playerDX * playerDetectionDistance, playerY + playerDY * playerDetectionDistance);
    glEnd();
}

void drawRaycast2D() {
    int mapaX, mapaY, mapaPonto;
    int ray, rayQtd = 60, depthOfField, depthOfFieldMax = 100;
    float rayX, rayY, rayAngle, rayXO, rayYO, rayDist;

    rayAngle = playerAngle - DegreeRad * 30;    
    if (rayAngle < 0) {
        rayAngle += 2 * PI;
    }
    if (rayAngle > 2 * PI) {
        rayAngle -= 2 * PI;
    }

    for (ray = 0; ray < rayQtd; ray++) {
        double verticalMapaTextura = 0, horizontalMapaTextura = 0;

        //HORIZONTAL
        depthOfField = 0;
        float horizontalDist = 100000000, horizontalX = playerX, horizontalY = playerY;
        float angleTan = -1 / tan(rayAngle);
        //OLHANDO PARA CIMA (> 180)
        if (rayAngle > PI) {
            rayY = (((int)playerY / tileSize) * tileSize ) - 0.0001;
            rayX = (playerY - rayY) * angleTan + playerX;
            rayYO = -tileSize;
            rayXO = -rayYO * angleTan;
        }
        //OLHANDO PARA BAIXO (< 180)
        if (rayAngle < PI) {
            rayY = (((int)playerY / tileSize) * tileSize) + tileSize;
            rayX = (playerY - rayY) * angleTan + playerX;
            rayYO = tileSize;
            rayXO = -rayYO * angleTan;
        }
        //OLHANDO PARA OS LADOS
        if (rayAngle == 0 || rayAngle == PI) {
            rayX = playerX;
            rayY = playerY;
            depthOfField = depthOfFieldMax;
        }
        while (depthOfField < depthOfFieldMax)
        {
            mapaX = (int)(rayX) / tileSize;
            mapaY = (int)(rayY) / tileSize;
            mapaPonto = mapaY * mapWidth + mapaX;
            // ACHOU A PAREDE
            if (mapaPonto > 0 && mapaPonto < mapWidth * mapHeight && walls[mapaPonto] > 0) {
                horizontalMapaTextura = walls[mapaPonto] - 1;
                horizontalX = rayX;
                horizontalY = rayY;
                horizontalDist = dist(playerX, playerY, horizontalX, horizontalY, rayAngle);
                depthOfField = depthOfFieldMax;
            }
            //PROXIMA LINHA
            else {
                rayX += rayXO;
                rayY += rayYO;
                depthOfField += 1;
            }
        }

        //VERTICAL
        depthOfField = 0;
        float verticalDist = 100000000, verticalX = playerX, verticalY = playerY;
        float negTan = -tan(rayAngle);
        //OLHANDO PARA ESQUERDA        
        if (rayAngle > P2 && rayAngle < P3) {
            rayX = (((int)playerX / tileSize) * tileSize) - 0.0001;
            rayY = (playerX - rayX) * negTan + playerY;
            rayXO = -tileSize;
            rayYO = -rayXO * negTan;
        }
        //OLHANDO PARA DIREITA
        if (rayAngle < P2 || rayAngle > P3) {
            rayX = (((int)playerX / tileSize) * tileSize) + tileSize;
            rayY = (playerX - rayX) * negTan + playerY;
            rayXO = tileSize;
            rayYO = -rayXO * negTan;
        }
        //OLHANDO PARA CIMA OU BAIXO
        if (rayAngle == 0 || rayAngle == PI) {
            rayX = playerX;
            rayY = playerY;
            depthOfField = depthOfFieldMax;
        }
        while (depthOfField < depthOfFieldMax)
        {
            mapaX = (int)(rayX) / tileSize;
            mapaY = (int)(rayY) / tileSize;
            mapaPonto = mapaY * mapWidth + mapaX;
            // ACHOU A PAREDE
            if (mapaPonto > 0 && mapaPonto < mapWidth * mapHeight && walls[mapaPonto] > 0) {
                verticalMapaTextura = walls[mapaPonto] - 1;
                verticalX = rayX;
                verticalY = rayY;
                verticalDist = dist(playerX, playerY, verticalX, verticalY, rayAngle);
                depthOfField = depthOfFieldMax;
            }
            //PROXIMA LINHA
            else {
                rayX += rayXO;
                rayY += rayYO;
                depthOfField += 1;
            }
        }

        //SHADE DE SOMBRA
        float shade = 1;
        glColor3f(0, 0.8, 0);

        //MOSTRAR QUEM BATEU PRIMEIRO
        if (verticalDist < horizontalDist) {
            horizontalMapaTextura = verticalMapaTextura;
            shade = 0.5;
            rayX = verticalX;
            rayY = verticalY;
            rayDist = verticalDist;
            glColor3f(0.7, 0, 0);
        }
        if (verticalDist > horizontalDist) {
            verticalMapaTextura = horizontalMapaTextura;
            rayX = horizontalX;
            rayY = horizontalY;
            rayDist = horizontalDist;
            glColor3f(0.9, 0, 0);
        }

        //glLineWidth(2); glBegin(GL_LINES); glVertex2f(playerX, playerY); glVertex2f(rayX, rayY); glEnd();

        //RAYCAST 3D
        float castAngle = playerAngle - rayAngle;
        if (castAngle < 0) {
            castAngle += 2 * PI;
        }
        if (castAngle > 2 * PI) {
            castAngle -= 2 * PI;
        }
        rayDist = rayDist * cos(castAngle);

        float lineHeight = (tileSize * screenHeightFactor) / rayDist;
        float texturaY_step = mapaTexturaOffset / (float)lineHeight;
        float texturaYOffset = 0;
        
        if (lineHeight > screenHeightFactor) {
            texturaYOffset = (lineHeight - screenHeightFactor) / 2;
            lineHeight = screenHeightFactor;
        }
        
        int y;
        float lineOffset = screenHeight / 2 - lineHeight / 2;
        float texturaY = texturaYOffset * texturaY_step + horizontalMapaTextura * mapaTexturaOffset;
        float texturaX;

        if (shade == 1) {
            texturaX = (int)(rayX / 2) % mapaTexturaOffset;
            if (rayAngle > 180) {
                texturaX = 31 - texturaX;
            }
        }
        else {
            texturaX = (int)(rayY / 2) % mapaTexturaOffset;
            if (rayAngle > 90 && rayAngle < 270) {
                texturaX = 31 - texturaX;
            }
        }

        for (y = 0; y < lineHeight; y++) {
            float c = mapaTextura[(int)(texturaY) * mapaTexturaOffset + (int)texturaX] * shade;
            glColor3f(c, c, c);
            //BRICK
            if (horizontalMapaTextura == 0) {
                glColor3f(c * 0.75, c * 0.42, c * 0.27);
            }
            glPointSize(12); glBegin(GL_POINTS); glVertex2i(ray * 12, y + lineOffset); glEnd();
            texturaY += texturaY_step;
        }        

        //ATUALIZAR ANGULO HORIZONTAL
        rayAngle += DegreeRad;
        if (rayAngle < 0) {
            rayAngle += 2 * PI;
        }
        if (rayAngle > 2 * PI) {
            rayAngle -= 2 * PI;
        }
    }
}

void movePlayer() {
    //OFFSET PARA NAO ATRAVESSAR AS PAREDES
    int xOffset = 0, yOffset = 0;

    playerDX = cos(playerAngle);
    playerDY = sin(playerAngle);

    if (playerDX < 0)
        xOffset = -playerColliderRadius;
    else
        xOffset = playerColliderRadius;

    if (playerDY < 0)
        yOffset = -playerColliderRadius;
    else
        yOffset = playerColliderRadius;
    
    int indexPlayerX = playerX / tileSize, indexPlayerX_ADD_XO = (playerX + xOffset) / tileSize, indexPlayerX_SUB_XO = (playerX - xOffset) / tileSize;
    int indexPlayerY = playerY / tileSize, indexPlayerY_ADD_YO = (playerY + yOffset) / tileSize, indexPlayerY_SUB_YO = (playerY - yOffset) / tileSize;
    
    //VERIFICAR SE ESTÁ TOCANDO OU IRA TOCAR EM UM PEDACO DO MAPA QUE NAO SEJA VAZIO

    if (input.forward) {
        if (walls[indexPlayerY * mapWidth + indexPlayerX_ADD_XO] == 0) {
            playerX += playerDX * deltaTime * playerSpeed;
        }
        if (walls[indexPlayerY_ADD_YO * mapWidth + indexPlayerX] == 0) {
            playerY += playerDY * deltaTime * playerSpeed;
        }
    }
    if (input.backward) {
        if (walls[indexPlayerY * mapWidth + indexPlayerX_SUB_XO] == 0) {
            playerX -= playerDX * deltaTime * playerSpeed;
        }
        if (walls[indexPlayerY_SUB_YO * mapWidth + indexPlayerX] == 0) {
            playerY -= playerDY * deltaTime * playerSpeed;
        }
    }
    if (input.left) {
        playerAngle -= playerRotateSpeed * deltaTime;
        if (playerAngle < 0) {
            playerAngle += 2 * PI;
        }
    }
    if (input.right) {
        playerAngle += playerRotateSpeed * deltaTime;
        if (playerAngle > 2 * PI) {
            playerAngle -= 2 * PI;
        }
    }
}

void playerActions() {
    //OFFSET PARA ESTAR TOCANDO EM UM OBJETO
    int xOffset = 0, yOffset = 0;

    if (playerDX < 0)
        xOffset = -playerDetectionDistance;
    else
        xOffset = playerDetectionDistance;

    if (playerDY < 0)
        yOffset = -playerDetectionDistance;
    else
        yOffset = playerDetectionDistance;

    int indexPlayerX = playerX / tileSize, indexPlayerX_ADD_XO = (playerX + xOffset) / tileSize;
    int indexPlayerY = playerY / tileSize, indexPlayerY_ADD_YO = (playerY + yOffset) / tileSize;

    //VERIFICAR SE ESTÁ TOCANDO OU IRA TOCAR EM UM PEDACO DO MAPA
    
    //ACOES DO JOGADOR
    if (input.action) {

        //ABRIR PORTA
        if (walls[indexPlayerY_ADD_YO * mapWidth + indexPlayerX_ADD_XO] == 4) {
            walls[indexPlayerY_ADD_YO * mapWidth + indexPlayerX_ADD_XO] = 0;
        }

    }
}

void display() {
    drawBackground();
    //drawMap2D();
    //drawPlayer();
    drawRaycast2D();
    movePlayer();
    playerActions();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
        if (action == GLFW_PRESS) input.forward = 1;
        if (action == GLFW_RELEASE) input.forward = 0;
    }
    if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
        if (action == GLFW_PRESS) input.backward = 1;
        if (action == GLFW_RELEASE) input.backward = 0;
    }
    if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT) {
        if (action == GLFW_PRESS) input.left = 1;
        if (action == GLFW_RELEASE) input.left = 0;
    }
    if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) {
        if (action == GLFW_PRESS) input.right = 1;
        if (action == GLFW_RELEASE) input.right = 0;
    }
    if (key == GLFW_KEY_E) {
        if (action == GLFW_PRESS) input.action = 1;
        if (action == GLFW_RELEASE) input.action = 0;
    }
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(screenWidth, screenHeight, "The Raycaster", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    initialize();

    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    glOrtho(0.0f, screenWidth, screenHeight, 0.0f, 0.0f, 1.0f);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //JOGO
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