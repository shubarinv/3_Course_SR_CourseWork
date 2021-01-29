
#include "application.hpp"
#include "camera.hpp"
#include "lights_manager.hpp"
#include "mesh.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "plane.h"
#include <random>
#include <glm/gtx/color_space.hpp>
#include "cube_map_texture.hpp"


LightsManager *lightsManager;
float lastX = 0;
float lastY = 0;
bool firstMouse = true;
// timing
double deltaTime = 0.0f;// time between current frame and last frame
double lastFrame = 0.0f;
Camera *camera;
int pressedKey = -1;

template<typename Numeric, typename Generator = std::mt19937>
Numeric random(Numeric from, Numeric to) {
    thread_local static Generator gen(std::random_device{}());

    using dist_type = typename std::conditional<
            std::is_integral<Numeric>::value, std::uniform_int_distribution<Numeric>, std::uniform_real_distribution<Numeric> >::type;

    thread_local static dist_type dist;

    return dist(gen, typename dist_type::param_type{from, to});
}

std::vector<glm::vec3> getCoordsForVertices(double xc, double yc, double size, int n) {
    std::vector<glm::vec3> vertices;
    auto xe = xc + size;
    auto ye = yc;
    vertices.emplace_back(xe, yc, ye);
    double alpha = 0;
    for (int i = 0; i < n - 1; i++) {
        alpha += 2 * M_PI / n;
        auto xr = xc + size * cos(alpha);
        auto yr = yc + size * sin(alpha);
        xe = xr;
        ye = yr;
        vertices.emplace_back(xe, yc, ye);
    }
    return vertices;
}

void programQuit([[maybe_unused]] int key, [[maybe_unused]] int action, Application *app) {
    app->close();
    LOG_S(INFO) << "Quiting...";
}

void wasdKeyPress([[maybe_unused]] int key, [[maybe_unused]] int action, [[maybe_unused]] Application *app) {
    if (action == GLFW_PRESS) { pressedKey = key; }
    if (action == GLFW_RELEASE) { pressedKey = -1; }
}

void moveCamera() {
    if (pressedKey == GLFW_KEY_W) { camera->ProcessKeyboard(FORWARD, (float) deltaTime); }
    if (pressedKey == GLFW_KEY_S) { camera->ProcessKeyboard(BACKWARD, (float) deltaTime); }
    if (pressedKey == GLFW_KEY_A) { camera->ProcessKeyboard(LEFT, (float) deltaTime); }
    if (pressedKey == GLFW_KEY_D) { camera->ProcessKeyboard(RIGHT, (float) deltaTime); }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float) xpos;
        lastY = (float) ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;// reversed since y-coordinates go from bottom to top

    lastX = (float) xpos;
    lastY = (float) ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera->ProcessMouseScroll(yoffset);
}

int main(int argc, char *argv[]) {
    Application app({1280, 720}, argc, argv);
    Application::setOpenGLFlags();
    app.registerKeyCallback(GLFW_KEY_ESCAPE, programQuit);

    app.registerKeyCallback(GLFW_KEY_W, wasdKeyPress);
    app.registerKeyCallback(GLFW_KEY_A, wasdKeyPress);
    app.registerKeyCallback(GLFW_KEY_S, wasdKeyPress);
    app.registerKeyCallback(GLFW_KEY_D, wasdKeyPress);

    lastX = app.getWindow()->getWindowSize().x / 2.0f;
    lastY = app.getWindow()->getWindowSize().y / 2.0f;

    glDepthFunc(GL_LESS);
    glCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    Shader shader_tex("shaders/lighting_shader.glsl", false);
    shader_tex.bind();
    shader_tex.setUniform1i("NUM_POINT_LIGHTS", 0);
    shader_tex.setUniform1i("NUM_SPOT_LIGHTS", 0);
    shader_tex.setUniform1i("NUM_DIR_LIGHTS", 0);

    Shader shader_skybox("shaders/skybox_shader.glsl");
    shader_skybox.bind();
    shader_skybox.setUniform1i("skybox", 0);
    shader_skybox.setUniform1f("intensity", 0.3);
    ObjLoader objLoader;
    std::vector<Mesh *> meshes;

    std::vector<Plane *> planes;
    float skyboxVertices[] = {
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };
// skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

    // load textures
    // -------------

    std::vector<std::string> faces{
            "textures/skybox/right.jpg",
            "textures/skybox/left.jpg",
            "textures/skybox/top.jpg",
            "textures/skybox/bottom.jpg",
            "textures/skybox/front.jpg",
            "textures/skybox/back.jpg"};
    unsigned int cubemapTexture = CubeMapTexture::loadCubemap(faces);


    lightsManager = new LightsManager;
    lightsManager->addLight(LightsManager::DirectionalLight("sun", {0, 0, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}));
    meshes.push_back(new Mesh("resources/models/baza.obj"));
    meshes.back()->addTexture("textures/textura2.png")->compile();
    meshes.push_back(new Mesh("resources/models/vertik.obj"));
    meshes.back()->addTexture("textures/textura2.png")->compile();
    meshes.push_back(new Mesh("resources/models/vintverh.obj"));
    meshes.back()->addTexture("textures/textura2.png")->compile();
    meshes.push_back(new Mesh("resources/models/vintzhopa.obj"));
    meshes.back()->addTexture("textures/textura2.png")->compile();
    meshes.push_back(new Mesh("resources/models/sand.obj"));
    meshes.back()->addTexture("textures/Sand.png")->compile();

    camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f));
    camera->setWindowSize(app.getWindow()->getWindowSize());

    glfwSetCursorPosCallback(app.getWindow()->getGLFWWindow(), mouse_callback);
    glfwSetScrollCallback(app.getWindow()->getGLFWWindow(), scroll_callback);


    while (!app.getShouldClose()) {
        app.getWindow()->updateFpsCounter();
        auto currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        moveCamera();
        Renderer::clear({0, 0, 0, 1});


        camera->passDataToShader(&shader_tex);
        lightsManager->passDataToShader(&shader_tex);
        //plane.draw(&shader_tex);
        for (auto &plane:planes) {
            plane->draw(&shader_tex);
        }
        for (auto &mesh:meshes) {
            mesh->draw(&shader_tex);
        }

        // draw skybox as last
        glDepthFunc(
                GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        shader_skybox.bind();
        shader_skybox.setUniform1f("intensity", 1);
        auto view = glm::mat4(glm::mat3(camera->GetViewMatrix())); // remove translation from the view matrix
        shader_skybox.setUniformMat4f("view", view);
        shader_skybox.setUniformMat4f("projection", camera->getProjection());
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glCall(glfwSwapBuffers(app.getWindow()->getGLFWWindow()));
        glfwPollEvents();

    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
