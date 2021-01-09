#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <rg/Error.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
//flashlight on/off
bool flashlightOn = false;

struct DirLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    unsigned int loadTexture(const char *path, bool gamma);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //floor
    float planeVertices[] = {
            // positions                normals       texture coords
             5.0f, -0.2f,  5.0f,   0.0f, 1.0f, 0.0f,   20.0f, 0.0f,
            -5.0f, -0.2f,  5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
            -5.0f, -0.2f, -5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 20.0f,

             5.0f, -0.2f,  5.0f,   0.0f, 1.0f, 0.0f,   20.0f, 0.0f,
            -5.0f, -0.2f, -5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 20.0f,
             5.0f, -0.2f, -5.0f,   0.0f, 1.0f, 0.0f,   20.0f, 20.0f
    };
    //sky
    float skyVertices[] = {
            // positions                normals       texture coords
             5.0f, -0.2f,  5.0f,   0.0f, 1.0f, 0.0f,   5.0f, 0.0f,
            -5.0f, -0.2f,  5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
            -5.0f, -0.2f, -5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 5.0f,

             5.0f, -0.2f,  5.0f,   0.0f, 1.0f, 0.0f,   5.0f, 0.0f,
            -5.0f, -0.2f, -5.0f,   0.0f, 1.0f, 0.0f,   0.0f, 5.0f,
             5.0f, -0.2f, -5.0f,   0.0f, 1.0f, 0.0f,   5.0f, 5.0f
    };
    //wall
    float wallVertices[] = {
            // positions                normals       texture coords
             1.0f,  0.25f, 0.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
            -1.0f,  0.25f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,
            -1.0f, -0.25f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,

             1.0f,  0.25f, 0.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
            -1.0f, -0.25f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
             1.0f, -0.25f, 0.0f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f
    };
    //notes
    float transparentVertices[] = {
            // positions               normals        texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,   0.0f, 1.0f, -1.0f,   0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,   0.0f, 1.0f, -1.0f,   0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,   0.0f, 1.0f, -1.0f,   1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,   0.0f, 1.0f, -1.0f,   0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,   0.0f, 1.0f, -1.0f,   1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,   0.0f, 1.0f, -1.0f,   1.0f,  0.0f
    };

    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2,GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // sky VAO
    unsigned int skyVAO, skyVBO;
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyVertices), &skyVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2,GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));


    // wall VAO
    unsigned int wallVAO, wallVBO;
    glGenVertexArrays(1, &wallVAO);
    glGenBuffers(1, &wallVBO);
    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), &wallVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2,GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
    
    unsigned int noteTexture1 = loadTexture("resources/textures/its3.png",true);
    unsigned int noteTexture2 = loadTexture("resources/textures/not3.png",true);
    unsigned int noteTexture3 = loadTexture("resources/textures/real3.png",true);

    unsigned int floorTexture = loadTexture("resources/textures/floor.jpeg",true);
    unsigned int skyTexture = loadTexture("resources/textures/cloud.jpeg",true);
    unsigned int wallTexture = loadTexture("resources/textures/mountain.jpeg",true);


    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader modelShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    // load models
    // -----------
    Model ourModel(FileSystem::getPath("resources/objects/Tree/Tree.obj"));
    ourModel.SetShaderTextureNamePrefix("material.");

    // directional light
    DirLight dirLight;
    dirLight.ambient = glm::vec3(0.01f);
    dirLight.diffuse = glm::vec3(0.5f);
    dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    dirLight.specular = glm::vec3(0.5f, 0.5f, 0.5f);

    SpotLight spotLight;
    spotLight.ambient = glm::vec3(0.0f);
    spotLight.diffuse = glm::vec3(1.0f);
    spotLight.specular = glm::vec3(1.0f);
    spotLight.constant = 1.0f;
    spotLight.linear = 0.09f;
    spotLight.quadratic = 0.032f;
    spotLight.cutOff = glm::cos(glm::radians(12.5f));
    spotLight.outerCutOff = glm::cos(glm::radians(15.0f));

    glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    // render loop
    // -----------

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();


        // don't forget to enable shader before setting uniforms
        modelShader.use();

        // calculating day-night cycle
        float time = glfwGetTime();
        float sin_time = sin(time/10);
        float cos_time = cos(time/10);
        if(sin_time > 0.0f) {
            dirLight.diffuse = glm::vec3(0.5f * sin_time);
            dirLight.specular = glm::vec3( 0.5f * sin_time);
            dirLight.direction = glm::vec3(-cos_time, -sin_time, -1+cos_time);
        }
        else {
            dirLight.direction = glm::vec3(0, 0, 0);
        }
        modelShader.setVec3("dirLight.direction", dirLight.direction);
        modelShader.setVec3("dirLight.ambient", dirLight.ambient);
        modelShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        modelShader.setVec3("dirLight.specular", dirLight.specular);

        spotLight.direction = camera.Front;
        spotLight.position = camera.Position;
        modelShader.setBool("spotLightOn", flashlightOn);
        modelShader.setVec3("spotLight.position", spotLight.position);
        modelShader.setVec3("spotLight.direction", spotLight.direction);
        modelShader.setVec3("spotLight.ambient", spotLight.ambient);
        modelShader.setVec3("spotLight.diffuse", spotLight.diffuse);
        modelShader.setVec3("spotLight.specular", spotLight.specular);
        modelShader.setFloat("spotLight.constant", spotLight.constant);
        modelShader.setFloat("spotLight.linear", spotLight.linear);
        modelShader.setFloat("spotLight.quadratic", spotLight.quadratic);
        modelShader.setFloat("spotLight.cutOff", spotLight.cutOff);
        modelShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);

        modelShader.setVec3("viewPosition", camera.Position);
        modelShader.setFloat("material.shininess", 8.0f);
        // view/projection transformations

        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        // rendering the floor
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(planeVAO);
        glUniform1i(glGetUniformLocation(modelShader.ID, "material.texture_diffuse1"), 0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(15.0f));
        modelShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //rendering the sky
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(skyVAO);
        glUniform1i(glGetUniformLocation(modelShader.ID, "material.texture_diffuse1"), 0);
        glBindTexture(GL_TEXTURE_2D, skyTexture);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 35.0f, 0.0f));
        model = glm::scale(model, glm::vec3(15.0f));
        modelShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //rendering the walls
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(wallVAO);
        glUniform1i(glGetUniformLocation(modelShader.ID, "material.texture_diffuse1"), 0);
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        //front wall
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 15.0f, -75.0f));
        model = glm::scale(model, glm::vec3(75.0f));
        modelShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //back wall
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 15.0f, 75.0f));
        model = glm::rotate(model, glm::radians(180.0f),glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(180.0f),glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(75.0f));
        modelShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //right wall
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(75.0f, 15.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f),glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(75.0f));
        modelShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //left wall
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-75.0f, 15.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f),glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f),glm::vec3(0.0f, .0f, 1.0f));
        model = glm::rotate(model, glm::radians(180.0f),glm::vec3(1.0f, .0f, 0.0f));
        model = glm::scale(model, glm::vec3(75.0f));
        modelShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // rendering notes
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(transparentVAO);
        glUniform1i(glGetUniformLocation(modelShader.ID, "material.texture_diffuse1"), 0);
        glBindTexture(GL_TEXTURE_2D, noteTexture1);
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        transform = glm::translate(transform, glm::vec3(-0.3f, 1.5f, 0.65f));
        modelShader.setMat4("model", transform);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTexture(GL_TEXTURE_2D, noteTexture2);
        transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        transform = glm::translate(transform, glm::vec3(-0.3f, 1.5f, -2.3f));
        modelShader.setMat4("model", transform);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTexture(GL_TEXTURE_2D, noteTexture3);
        transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        transform = glm::translate(transform, glm::vec3 (0.5f, 1.5f, -0.6f));
        modelShader.setMat4("model", transform);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // rendering the tree
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -3.2f,0.0f));
        model = glm::scale(model, glm::vec3(4.5f));

        modelShader.setMat4("model", model);
        ourModel.Draw(modelShader);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Cleanup

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // changing to running/walking
    if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.speedUp();
    if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        camera.slowDown();
    // flashlight control
    if(key == GLFW_KEY_F && action == GLFW_PRESS){
        flashlightOn = !flashlightOn;
    }
}

unsigned int loadTexture(char const * path, bool gamma)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum dataFormat;
        GLenum internalFormat;
        if (nrComponents == 1)
            internalFormat = dataFormat = GL_RED;
        else if (nrComponents == 3) {
            internalFormat = gamma ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4){
            internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}