// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>
#include <iostream>
#include <vector>
#include <fstream>

// Include GLFW
#include <GLFW/glfw3.h>

GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace glm;
#include <shader.hpp>
#include <texture.hpp>
#include <objloader.hpp>

glm::vec3 camUp = glm::vec3(0.0, 1.0, 0.0);
float near;

glm::vec3 angleMoon = vec3(0.0, 0.0, 1.0);
glm::vec3 lookAtVec = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 oneOfTheGlobeCenters = vec3(-2.99293e-08, 6.36168e-09, -0.7);

float radiusEarth;
float radiusMoon;

GLfloat vertices[15660];
GLfloat uvs[10440];
GLfloat normals[15660];

glm::vec3 cameraPosition;

int h;
int w;
int partCount = 30;
int fragmentCount = 30;

const float PI = 3.1415926535897;

glm::mat4 rotationMatrix(float angle)
{
    vec3 axis = vec3(0.0, 1.0, 0.0);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
                oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
                oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
                0.0, 0.0, 0.0, 1.0);
}

glm::vec3 rotate(vec3 v, float angle)
{
    mat4 m = rotationMatrix(angle);
    return glm::vec3((m * vec4(v, 1.0)).x, (m * vec4(v, 1.0)).y, (m * vec4(v, 1.0)).z);
}

class Ball
{
public:
    Ball(float r, std::string vs, std::string fs, bool Moon)
    {
        radius = r;
        programID = LoadShaders(vs.c_str(), fs.c_str());
        std::vector<GLuint> res = setBuffers();
        bufferId = res[0];
        if (!Moon)
        {
            Texture = loadBMP_custom("earth3.bmp");
            texId = res[1];
            TextureID = glGetUniformLocation(programID, "myTexture");
            angleId = glGetUniformLocation(programID, "angle");
            axisId = glGetUniformLocation(programID, "axis");
        }
        else
        {
            angleMoonId = glGetUniformLocation(programID, "angleMoon");
        }

        MatrixID = glGetUniformLocation(programID, "MVP");
    }

    std::vector<GLuint> setBuffers()
    {
        std::vector<GLuint> res;

        GLuint particles_position_buffer;
        glGenVertexArrays(1, &VertexArrayID);
        glGenBuffers(1, &particles_position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, 15660 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        glGenVertexArrays(1, &VertexArrayID);
        res.push_back(particles_position_buffer);

        GLuint uvbuffer;
        glGenBuffers(1, &uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, 10440 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        res.push_back(uvbuffer);
        return res;
    }

    void draw(glm::vec3 cP, bool Moon)
    {
        glBindVertexArray(VertexArrayID);
        glUseProgram(programID);
        if (!Moon)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture);
            glUniform1i(TextureID, 0);

            glBindBuffer(GL_ARRAY_BUFFER, texId);
            glBufferData(GL_ARRAY_BUFFER, 10440 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 10440 * sizeof(GLfloat), uvs);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, texId);
            glVertexAttribPointer(
                1,
                2,
                GL_FLOAT,
                GL_FALSE,
                0,
                (void *)0);

            glm::vec3 axis = glm::cross(normalize(oneOfTheGlobeCenters), vec3(0.0, 1.0, 0.0));

            glUniform3f(axisId, axis.x, axis.y, axis.z);
            glUniform1f(angleId, acos(glm::dot(vec3(0.0, 1.0, 0.0), normalize(oneOfTheGlobeCenters))));
        }
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (GLfloat)w / (GLfloat)(h), near, 150.0f);
        glm::mat4 View = glm::lookAt(cP, lookAtVec, camUp);
        glm::mat4 Model = glm::mat4(1.0f);
        glm::mat4 MVP = Projection * View * Model;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, bufferId);
        glBufferData(GL_ARRAY_BUFFER, 15660 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 15660, vertices);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, bufferId);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *)0);

        glUniform1f(23, radius);
        if (Moon)
            glUniform3f(angleMoonId, angleMoon.x, angleMoon.y, angleMoon.z);
        glVertexAttribDivisor(0, 0);
        glVertexAttribDivisor(1, 0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 15660, 1);
    }

private:
    float radius;
    GLuint programID;
    GLuint bufferId;
    GLuint MatrixID;
    GLuint VertexArrayID;
    GLuint texId;
    GLuint TextureID;
    GLuint angleId;
    GLuint axisId;
    GLuint angleMoonId;
    GLuint Texture;
};

void init()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    h = 600;
    w = 800;
    cameraPosition = glm::vec3(-2.0, 0.0, -1.0);

    window = glfwCreateWindow(800, 600, "Earth with a moon", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
    }
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.51f, 0.87f, 0.9f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    radiusEarth = 0.7;
    radiusMoon = 0.2;
    near = 0.7;
    bool res = loadOBJ("sphere.obj", vertices, uvs, normals);
}

void moveCam(float angle)
{
    cameraPosition = vec3(cos(angle) * cameraPosition.x + sin(angle) * cameraPosition.z, cameraPosition.y, -sin(angle) * cameraPosition.x + cos(angle) * cameraPosition.z);
}

int main(int argc, char *argv[])
{
    init();
    Ball earth = Ball(radiusEarth, "EarthVS.vertexshader", "EarthFS.fragmentshader", false);
    Ball moon = Ball(radiusMoon, "MoonVS.vertexshader", "MoonFS.fragmentshader", true);
    int height;
    int width;
    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        angleMoon = rotate(angleMoon, 0.02f);
        glfwGetWindowSize(window, &width, &height);
        w = width;
        h = height;
        glViewport(0, 0, ((float)w), h);
        earth.draw(cameraPosition, false);
        moon.draw(cameraPosition, true);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            moveCam(-0.015f);
            angleMoon = rotate(angleMoon, 0.015f);
        }
        else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            moveCam(0.015f);
            angleMoon = rotate(angleMoon, -0.012f);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);
    glfwTerminate();
    return 0;
}
