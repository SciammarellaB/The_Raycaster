#include <GLFW/glfw3.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "mapa.hpp"
#include "mapaTexturas.hpp"

#define PI 3.1415926535

int screenWidth = 960, screenHeight = 640, screenHeightFactor = screenHeight;
double lastFrame, currentFrame, deltaTime = 0, frames, fpsLastFrame;

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
    playerRotateSpeed = 250;
    playerDetectionDistance = 30;
    playerColliderRadius = 20;
}

static float degreeRad(float angle) {
    return angle * PI / 180.0;
}

static float fixAngle(float angle) {
    if (angle > 359) {
        angle -= 360;
    }
    if (angle < 0) {
        angle += 360;
    }
    return angle;
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float dist(float ax, float ay, float bx, float by, float ang) {
    return cos(degreeRad(ang)) * (bx-ax) - sin(degreeRad(ang)) * (by - ay);
}

void drawBackground() {
    //CEU
    int ceu;
    float ceuR = 0.34, ceuG = 0.54, ceuB = 0.70;
    for (ceu = 0; ceu < screenHeight / 2; ceu++) {
        glColor3f(ceuR, ceuG, ceuB);
        glBegin(GL_LINES);
        glVertex2i(0, ceu);
        glVertex2i(screenWidth, ceu);
        glEnd();
        ceuR -= 0.003f;
        ceuG -= 0.003f;
        ceuB -= 0.003f;
    }

    //CHAO
    int chao;
    float chaoR = 0.7, chaoG = 0.7, chaoB = 0.7;
    for (chao = screenHeight / 2; chao > 0; chao--) {
        glColor3f(chaoR, chaoG, chaoB);
        glBegin(GL_LINES);
        glVertex2i(0, screenHeight / 2 + chao);
        glVertex2i(screenWidth, screenHeight / 2 + chao);
        glEnd();
        chaoR -= 0.003f;
        chaoG -= 0.003f;
        chaoB -= 0.003f;
    }
}

void drawRaycast2D() {
    int mapaX, mapaY, mapaPonto;
    float ray, rayQtd = 120, depthOfField, depthOfFieldMax = 100;
    float rayX, rayY, rayAngle, rayXO, rayYO, verticalX, verticalY;
    double rayDist;

    rayAngle = fixAngle(playerAngle + 30);

    for (ray = 0; ray < rayQtd; ray++) {
        double verticalMapaTextura = 0, horizontalMapaTextura = 0;

        float Tan = tan(degreeRad(rayAngle));

        //VERTICAL
        depthOfField = 0;
        float verticalDist = 100000000;
        //OLHANDO PARA ESQUERDA        
        if (cos(degreeRad(rayAngle)) > 0.001) {
            rayX = (((int)playerX / tileSize) * tileSize) + tileSize;
            rayY = (playerX - rayX) * Tan + playerY;
            rayXO = tileSize;
            rayYO = -rayXO * Tan;
        }
        //OLHANDO PARA DIREITA
        else if (cos(degreeRad(rayAngle)) < -0.001) {
            rayX = (((int)playerX / tileSize) * tileSize) - 0.0001;
            rayY = (playerX - rayX) * Tan + playerY;
            rayXO = -tileSize;
            rayYO = -rayXO * Tan;
        }
        //OLHANDO PARA CIMA OU BAIXO
        else {
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
                verticalDist = dist(playerX, playerY, rayX, rayY, rayAngle);
                depthOfField = depthOfFieldMax;
            }
            //PROXIMA LINHA
            else {
                rayX += rayXO;
                rayY += rayYO;
                depthOfField += 1;
            }
            verticalX = rayX;
            verticalY = rayY;
        }

        //HORIZONTAL
        depthOfField = 0;
        float horizontalDist = 100000000, horizontalX = playerX, horizontalY = playerY;
        Tan = 1.0 / Tan;
        //OLHANDO PARA CIMA (> 180)
        if (sin(degreeRad(rayAngle)) > 0.001) {
            rayY = (((int)playerY / tileSize) * tileSize ) - 0.0001;
            rayX = (playerY - rayY) * Tan + playerX;
            rayYO = -tileSize;
            rayXO = -rayYO * Tan;
        }
        //OLHANDO PARA BAIXO (< 180)
        else if (sin(degreeRad(rayAngle)) < -0.001) {
            rayY = (((int)playerY / tileSize) * tileSize) + tileSize;
            rayX = (playerY - rayY) * Tan + playerX;
            rayYO = tileSize;
            rayXO = -rayYO * Tan;
        }
        //OLHANDO PARA OS LADOS
        else {
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

        //MOSTRAR QUEM BATEU PRIMEIRO
        if (verticalDist < horizontalDist) {
            horizontalMapaTextura = verticalMapaTextura;
            rayX = verticalX;
            rayY = verticalY;
            rayDist = verticalDist;

            float teste = (rayDist < 255 ? rayDist / 255 : 1) / 3;
            glColor3f(0.3 - teste, 0.21 - teste, 0.08 - teste);
        }
        if (verticalDist > horizontalDist) {
            verticalMapaTextura = horizontalMapaTextura;
            rayX = horizontalX;
            rayY = horizontalY;
            rayDist = horizontalDist;

            float teste = (rayDist < 255 ? rayDist / 255 : 1) / 3;
            glColor3f(0.3 - teste, 0.21 - teste, 0.08 - teste);
        }

        //glLineWidth(2); glBegin(GL_LINES); glVertex2f(playerX, playerY); glVertex2f(rayX, rayY); glEnd();

        //RAYCAST 3D
        float castAngle = fixAngle(playerAngle - rayAngle);
        
        rayDist = rayDist * cos(degreeRad(castAngle));

        float lineHeight = (tileSize * screenHeight) / rayDist;
        
        if (lineHeight > screenHeightFactor) {            
            lineHeight = screenHeightFactor;
        }

        float lineOffset = screenHeight / 2 - lineHeight / 2;
        
        glLineWidth(8); glBegin(GL_LINES); glVertex2i(ray * 8, lineOffset); glVertex2i(ray * 8, lineOffset + lineHeight); glEnd();

        //ATUALIZAR ANGULO HORIZONTAL
        rayAngle = fixAngle(rayAngle - 0.5);
    }
}

void movePlayer() {
    //OFFSET PARA NAO ATRAVESSAR AS PAREDES
    int xOffset = 0, yOffset = 0;

    playerDX = cos(degreeRad(playerAngle));
    playerDY = -sin(degreeRad(playerAngle));

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
        playerAngle += playerRotateSpeed * deltaTime;
        playerAngle = fixAngle(playerAngle);
    }
    if (input.right) {
        playerAngle -= playerRotateSpeed * deltaTime;
        playerAngle = fixAngle(playerAngle);
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

void fps() {

    frames++;
    if (currentFrame - fpsLastFrame > 1.0) {
        system("cls");
        printf("FPS: %f \n", frames);
        frames = 0;
        fpsLastFrame = currentFrame;
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

        fps();

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