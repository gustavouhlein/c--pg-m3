#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);


int setup();
void pickColor(GLdouble xpos, GLdouble ypos);
void initRandomColors();
bool colorsAreSimilar(const glm::vec3 & color1, const glm::vec3 & color2);


const GLuint WIDTH = 1024, HEIGHT = 768;
const int COLUMNS = 5, LINES = 15;
const int MAX_CLICKS = 30;

int remainingClicks = MAX_CLICKS;
int score = 0;
std::vector<std::vector<glm::vec3>> colorMatrix(COLUMNS, std::vector<glm::vec3>(LINES));
std::vector<std::vector<bool>> visibilityMatrix(COLUMNS, std::vector<bool>(LINES, true));

int main()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Jogo das cores", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader shader("../shaders/retangulo.vs", "../shaders/retangulo.fs");

    GLuint VAO = setup();
    initRandomColors();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        shader.Use();
        GLfloat xc = -0.77f, xl = 0.90f;

        for (int c = 0; c < COLUMNS; c++) {
            for (int l = 0; l < LINES; l++) {
                if (!visibilityMatrix[c][l]) continue;

                glm::vec3 color = colorMatrix[c][l];
                shader.setVec3("cor", color.r, color.g, color.b);

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(xc + c * 0.385f, xl - l * 0.125f, 0.0f));
                model = glm::scale(model, glm::vec3(0.38f, 0.38f, 1.0f));
                float matArray[16];
                memcpy(matArray, glm::value_ptr(model), sizeof(float) * 16);
                shader.setMat4("model", matArray);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (remainingClicks <= 0) return;

        GLdouble xpos, ypos;
        int w, h;
        glfwGetCursorPos(window, &xpos, &ypos);
        glfwGetWindowSize(window, &w, &h);

        xpos = (xpos / w) * 2 - 1;
        ypos = 1 - (ypos / h) * 2;

        for (int c = 0; c < COLUMNS; c++) {
            for (int l = 0; l < LINES; l++) {
                if (!visibilityMatrix[c][l]) continue;

                GLfloat xc = -0.77f + c * 0.385f;
                GLfloat xl = 0.90f - l * 0.125f;

                if (xpos >= xc - 0.19f && xpos <= xc + 0.19f && ypos >= xl - 0.0625f && ypos <= xl + 0.0625f) {
                    glm::vec3 clickedColor = colorMatrix[c][l];
                    bool similarFound = false;

                    for (int c2 = 0; c2 < COLUMNS; c2++) {
                        for (int l2 = 0; l2 < LINES; l2++) {
                            if (visibilityMatrix[c2][l2] && colorsAreSimilar(clickedColor, colorMatrix[c2][l2])) {
                                visibilityMatrix[c2][l2] = false;
                                score++;
                                similarFound = true;
                            }
                        }
                    }

                    if (!similarFound) {
                        visibilityMatrix[c][l] = false;
                        score++;
                    }

                    remainingClicks--;
                    if (remainingClicks) {
                        std::cout << "Cliques Restantes: " << remainingClicks << " Pontos: " << score << std::endl;
                    }
                    else {
                        std::cout << "Total: " << score << std::endl;
                    }
                    return;
                }
            }
        }
    }
}

void initRandomColors()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int c = 0; c < COLUMNS; c++) {
        for (int l = 0; l < LINES; l++) {
            colorMatrix[c][l] = glm::vec3(dis(gen), dis(gen), dis(gen));
        }
    }
}

int setup()
{
    GLfloat vertices[] = {
        -0.5f, -0.15f, 0.0f,
        -0.5f,  0.15f, 0.0f,
         0.5f, -0.15f, 0.0f,
         0.5f,  0.15f, 0.0f
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

bool colorsAreSimilar(const glm::vec3 & color1, const glm::vec3 & color2)
{
    float threshold = 0.15f;

    float diffR = std::abs(color1.r - color2.r);
    float diffG = std::abs(color1.g - color2.g);
    float diffB = std::abs(color1.b - color2.b);

    return (diffR < threshold && diffG < threshold && diffB < threshold);
}
