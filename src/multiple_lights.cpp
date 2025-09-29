#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <fstream>
#include <string>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

static std::string resolve(const std::string& fn) {
    for (std::string p : {fn, "shaders/" + fn, "resources/shaders/" + fn, "src/2.lighting/6.multiple_lights/" + fn}) {
        std::ifstream f(p); if (f.good()) return p;
    }
    return fn; // fall back (let Shader class print error)
}

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 6.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// runtime toggles
int currentMode = 1; // 1=ORBIT,2=PENDULUM,3=SPIRAL,4=SWARM
bool useBlinn = true;
bool useToon = false;
bool useGamma = true;

int main()
{
    // glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Kinetic Sculpture Assignment", NULL, NULL);
    if (!window) { std::cout << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "Failed to initialize GLAD\n"; return -1; }
    glEnable(GL_DEPTH_TEST);

    // Use ORIGINAL filenames
    Shader lightingShader(resolve("6.multiple_lights.vs").c_str(), resolve("6.multiple_lights.fs").c_str());
    Shader lightCubeShader(resolve("6.light_cube.vs").c_str(), resolve("6.light_cube.fs").c_str());

    // geometry
    float cubeVerts[] = {
        -0.5f,-0.5f,-0.5f,  0,0,-1,  0,0,
         0.5f,-0.5f,-0.5f,  0,0,-1,  1,0,
         0.5f, 0.5f,-0.5f,  0,0,-1,  1,1,
         0.5f, 0.5f,-0.5f,  0,0,-1,  1,1,
        -0.5f, 0.5f,-0.5f,  0,0,-1,  0,1,
        -0.5f,-0.5f,-0.5f,  0,0,-1,  0,0,

        -0.5f,-0.5f, 0.5f,  0,0,1,   0,0,
         0.5f,-0.5f, 0.5f,  0,0,1,   1,0,
         0.5f, 0.5f, 0.5f,  0,0,1,   1,1,
         0.5f, 0.5f, 0.5f,  0,0,1,   1,1,
        -0.5f, 0.5f, 0.5f,  0,0,1,   0,1,
        -0.5f,-0.5f, 0.5f,  0,0,1,   0,0,

        -0.5f, 0.5f, 0.5f, -1,0,0,   1,0,
        -0.5f, 0.5f,-0.5f, -1,0,0,   1,1,
        -0.5f,-0.5f,-0.5f, -1,0,0,   0,1,
        -0.5f,-0.5f,-0.5f, -1,0,0,   0,1,
        -0.5f,-0.5f, 0.5f, -1,0,0,   0,0,
        -0.5f, 0.5f, 0.5f, -1,0,0,   1,0,

         0.5f, 0.5f, 0.5f,  1,0,0,   1,0,
         0.5f, 0.5f,-0.5f,  1,0,0,   1,1,
         0.5f,-0.5f,-0.5f,  1,0,0,   0,1,
         0.5f,-0.5f,-0.5f,  1,0,0,   0,1,
         0.5f,-0.5f, 0.5f,  1,0,0,   0,0,
         0.5f, 0.5f, 0.5f,  1,0,0,   1,0,

        -0.5f,-0.5f,-0.5f,  0,-1,0,  0,1,
         0.5f,-0.5f,-0.5f,  0,-1,0,  1,1,
         0.5f,-0.5f, 0.5f,  0,-1,0,  1,0,
         0.5f,-0.5f, 0.5f,  0,-1,0,  1,0,
        -0.5f,-0.5f, 0.5f,  0,-1,0,  0,0,
        -0.5f,-0.5f,-0.5f,  0,-1,0,  0,1,

        -0.5f, 0.5f,-0.5f,  0,1,0,   0,1,
         0.5f, 0.5f,-0.5f,  0,1,0,   1,1,
         0.5f, 0.5f, 0.5f,  0,1,0,   1,0,
         0.5f, 0.5f, 0.5f,  0,1,0,   1,0,
        -0.5f, 0.5f, 0.5f,  0,1,0,   0,0,
        -0.5f, 0.5f,-0.5f,  0,1,0,   0,1
    };

    float planeVerts[] = {
        -20.0f, 0.0f,-20.0f,  0,1,0,  0,  0,
         20.0f, 0.0f,-20.0f,  0,1,0, 20,  0,
         20.0f, 0.0f, 20.0f,  0,1,0, 20, 20,
         20.0f, 0.0f, 20.0f,  0,1,0, 20, 20,
        -20.0f, 0.0f, 20.0f,  0,1,0,  0, 20,
        -20.0f, 0.0f,-20.0f,  0,1,0,  0,  0
    };

    auto makeVAO = [](float* data, size_t bytes) {
        unsigned int VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, bytes, data, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        return VAO;
        };

    unsigned int cubeVAO = makeVAO(cubeVerts, sizeof(cubeVerts));
    unsigned int planeVAO = makeVAO(planeVerts, sizeof(planeVerts));

    // textures
    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/container2.png").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/container2_specular.png").c_str());

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    // anchors & colors for up to 8 lights (use 6 for demo)
    glm::vec3 anchors[8] = {
        { 0.0f, 1.2f,  2.5f},
        { 2.5f, 1.0f, -2.0f},
        {-2.5f, 1.4f, -4.0f},
        { 0.0f, 2.0f, -3.0f},
        { 1.8f, 2.2f, -1.0f},
        {-3.5f, 1.6f, -2.0f},
        { 3.0f, 1.8f, -3.5f},
        {-1.8f, 2.2f,  1.0f}
    };
    glm::vec3 colors[8] = {
        {1.0,0.6,0.6}, {0.6,1.0,0.6}, {0.6,0.6,1.0}, {1.0,1.0,0.6},
        {1.0,0.6,1.0}, {0.6,1.0,1.0}, {1.0,0.8,0.5}, {0.7,0.9,0.7}
    };

    // lamp gizmo (reuse cube VAO)
    unsigned int lightCubeVAO = cubeVAO;

    while (!glfwWindowShouldClose(window))
    {
        float time = (float)glfwGetTime();
        deltaTime = time - lastFrame;
        lastFrame = time;
        processInput(window);

        glClearColor(0.05f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.use();
        lightingShader.setBool("useBlinn", useBlinn);
        lightingShader.setBool("useToon", useToon);
        lightingShader.setBool("useGamma", useGamma);
        lightingShader.setFloat("gammaValue", 2.2f);
        lightingShader.setVec3("viewPos", camera.Position);

        // Dir light
        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.02f, 0.02f, 0.03f);
        lightingShader.setVec3("dirLight.diffuse", 0.3f, 0.3f, 0.35f);
        lightingShader.setVec3("dirLight.specular", 0.4f, 0.4f, 0.45f);

        // Spotlight from camera with Q/E sweep
        static float sweep = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) sweep -= 0.8f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) sweep += 0.8f * deltaTime;
        glm::vec3 spotDir = glm::normalize(glm::rotate(glm::mat4(1.0f), sweep, glm::vec3(0, 1, 0)) * glm::vec4(camera.Front, 0.0));
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", spotDir);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.0f)));
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 0.95f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);

        // Animated colored point lights (choose 6)
        int numLights = 6;
        lightingShader.setInt("numPointLights", numLights);
        glm::vec3 positions[8];
        for (int i = 0; i < numLights; i++) {
            glm::vec3 pos = anchors[i];
            switch (currentMode) {
            case 1: { // ORBIT
                float r = 0.6f + 0.2f * i;
                pos.x += r * sin(time * 0.9f + i);
                pos.z += r * cos(time * 1.1f + 0.5f * i);
                pos.y += 0.2f * sin(time * 1.3f + i * 0.7f);
            } break;
            case 2: { // PENDULUM
                float a = sin(time * 1.2f + i) * 0.6f;
                pos += glm::vec3(sin(a) * (1.0f + 0.2f * i), -abs(sin(a)) * 0.2f + 0.6f, 0.0f);
            } break;
            case 3: { // SPIRAL
                float h = fmod(time * 0.4f + 0.2f * i, 3.0f) - 1.5f;
                float r = 0.8f + 0.25f * i;
                pos = glm::vec3(r * cos(time * 1.0f + i), 1.0f + h, r * sin(time * 1.0f + i)) + glm::vec3(0, 0, -2.0f);
            } break;
            case 4: { // SWARM
                pos += glm::vec3(
                    0.4f * sin(time * 1.7f + i * 1.1f),
                    0.3f * sin(time * 2.3f + i * 0.9f),
                    0.4f * cos(time * 1.5f + i * 1.3f)
                );
            } break;
            }
            positions[i] = pos;
            std::string base = "pointLights[" + std::to_string(i) + "]";
            lightingShader.setVec3((base + ".position").c_str(), pos);
            lightingShader.setVec3((base + ".color").c_str(), colors[i]);
            lightingShader.setFloat((base + ".constant").c_str(), 1.0f);
            lightingShader.setFloat((base + ".linear").c_str(), 0.09f);
            lightingShader.setFloat((base + ".quadratic").c_str(), 0.032f);
            lightingShader.setVec3((base + ".ambient").c_str(), 0.03f, 0.03f, 0.03f);
            lightingShader.setVec3((base + ".diffuse").c_str(), 0.9f, 0.9f, 0.9f);
            lightingShader.setVec3((base + ".specular").c_str(), 1.0f, 1.0f, 1.0f);
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, specularMap);

        // ground
        {
            glm::mat4 model(1.0f);
            lightingShader.setMat4("model", model);
            glBindVertexArray(planeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // kinetic ring of cubes (instead of the old 10 static boxes)
        glBindVertexArray(cubeVAO);
        for (int i = 0; i < 12; i++) {
            glm::mat4 m(1.0f);
            float ang = i * (glm::two_pi<float>() / 12.0f) + time * 0.4f;
            float radius = 3.0f;
            m = glm::translate(m, glm::vec3(radius * cos(ang), 0.8f + 0.2f * sin(time * 0.8f + i), radius * sin(ang) - 2.0f));
            m = glm::rotate(m, ang * 2.0f, glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setMat4("model", m);
            lightingShader.setFloat("material.shininess", 32.0f + 16.0f * sin(time + i));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // draw lamp gizmos as small white cubes
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        for (int i = 0; i < 6; i++) {
            glm::mat4 m(1.0f);
            m = glm::translate(m, positions[i]);
            m = glm::scale(m, glm::vec3(0.15f));
            lightCubeShader.setMat4("model", m);
            glBindVertexArray(lightCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) currentMode = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) currentMode = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) currentMode = 3;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) currentMode = 4;

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) useBlinn = true;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) useBlinn = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) useToon = true;
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) useToon = false;
    static bool gPressed = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gPressed) { useGamma = !useGamma; gPressed = true; }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) gPressed = false;
}

// callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = (float)xposIn; float ypos = (float)yposIn;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX; float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}

// texture loader
unsigned int loadTexture(char const* path)
{
    unsigned int textureID; glGenTextures(1, &textureID);
    int w, h, n; unsigned char* data = stbi_load(path, &w, &h, &n, 0);
    if (data) {
        GLenum format = GL_RGBA; if (n == 1) format = GL_RED; else if (n == 3) format = GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else { std::cout << "Failed to load texture: " << path << std::endl; stbi_image_free(data); }
    return textureID;
}
