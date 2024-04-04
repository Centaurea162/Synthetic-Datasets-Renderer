#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "glad/glad.c"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "learnopengl/shader_s.h"
#include "learnopengl/camera.h"
#include "learnopengl/model.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include "cwd/cwd.cpp"
#include <direct.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#define PI 3.1415926f


void framebuffer_size_callback(GLFWwindow* window, int SCR_WIDTH, int SCR_HEIGHT);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void CaptureScreen(int num, const string& folderPath);
void viewInHemisphere(float radius, int thetaDensity, int phiDensity, vector<glm::vec3>& cameraPositions);
void viewOfForwardFacing(glm::vec3 bias,float width, float height, int widthDensity, int heightDensity, vector<glm::vec3>& cameraPositions);
string timestr();

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Display camera pyramid
bool visualize = false;
bool inCameraView = false;
glm::mat4 cameraViewMat;
// current display camera index
int indexCamera = 0;

vector<glm::vec3> cameraPositions;
vector<glm::vec3> cameraLookAts;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // 先创建用于存放图片的文件夹
    string folderPath = exewd() + "\\output";
    string command = "mkdir " + folderPath;
    system(command.c_str());


    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sampler", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Setup Dear ImGui context
    const char* glsl_version = "#version 130";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float metalnessFactor = 1.0f;
    float roughnessFactor = 1.0f;

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // enable msaa
    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    // build and compile shaders
    // -------------------------
    Shader pbrShader(std::string(exewd() + "\\Shader\\pbr_vertex.glsl").c_str(),
                     std::string(exewd() + "\\Shader\\pbr_fragment.glsl").c_str());

    Shader pyramidShader(std::string(exewd() + "\\Shader\\pyramid_vertex.glsl").c_str(),
                     std::string(exewd() + "\\Shader\\pyramid_fragment.glsl").c_str());


    // load models
    // -----------
    Model ourModel(std::string(exewd() + "\\model\\model.obj").c_str());

    // pyramid
    float vertices[] = {
            // positions
            0.0f,   0.0f, -1.0f,          // top
            1.0f, 1.0f, 1.0f,         // bottom

            0.0f,   0.0f, -1.0f,          // top
            1.0f, -1.0f, 1.0f,         // bottom

            0.0f,   0.0f, -1.0f,          // top
            -1.0f, -1.0f, 1.0f,         // bottom

            0.0f,   0.0f, -1.0f,         // top
            -1.0f, 1.0f, 1.0f,         // bottom

            1.0f,  1.0f, 1.0f,     // bottom
            1.0f,  -1.0f, 1.0f,     // bottom

            1.0f,  -1.0f, 1.0f,     // bottom
            -1.0f,  -1.0f, 1.0f,     // bottom

            -1.0f,  -1.0f, 1.0f,     // bottom
            -1.0f,  1.0f, 1.0f,     // bottom

            -1.0f,  1.0f, 1.0f,     // bottom
            1.0f,  1.0f, 1.0f,     // bottom
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // lights
    // ------
    glm::vec3 lightDirections[] = {
            glm::vec3(-10.0f,  3.0f, 0.0f),
            glm::vec3( -10.0f,  3.0f, 0.0f),
            glm::vec3( 0.0f,  10.0f, 0.0f),
    };
    glm::vec3 lightColors[] = {
            glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, 1.0f, 1.0f),
    };


    vector<float> rotate = {0.0f, 0.0f, 0.0f};
    vector<float> intensity = {100.0f, 100.0f, 100.0f};
    // Hemisphere sample
    float radius = 10.0f;
    int thetaDensity = 1;
    int phiDensity = 1;
    // Forward Facing sample
    float forwardFaceRotaion = 0.0f;
    float distance = 1.0f;
    float width = 1.0f;
    float height = 1.0f;
    int widthDensity = 1;
    int heightDensity = 1;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float scale = 0.01f;
    float ambient = 0.03f;


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // Setup ImGui
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("Model Parameters");                          // Create a window called "Hello, world!" and append into it.

            for (int i = 0; i < intensity.size(); ++i) {
                string intensityName = "Intensity" + to_string(i+1);
                ImGui::SliderFloat(intensityName.c_str(), &intensity[i], 0.0f, 200.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                string dirName = "LightDirection" + to_string(i+1);
                ImGui::SliderFloat(dirName.c_str(), &rotate[i], 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            }

            ImGui::SliderFloat("Metalness", &metalnessFactor, 0.0f, 5.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::SliderFloat("Roughness", &roughnessFactor, 0.0f, 5.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::SliderFloat("Ambient", &ambient, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color


            ImGui::Text("Translate");
            ImGui::SliderFloat("x", &x, -100.0f, 100.0f);
            ImGui::SliderFloat("y", &y, -100.0f, 100.0f);
            ImGui::SliderFloat("z", &z, -100.0f, 100.0f);
            ImGui::SliderFloat("Scale", &scale, 0.01f, 1.0f);


            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }


        // render
        // ------
        // 通过imgui的颜色控制背景颜色
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // don't forget to enable shader before setting uniforms
        pbrShader.use();

        // light setup
        glm::mat4 rotation = glm::mat4(1.0f);
        for (int i = 0; i < intensity.size(); ++i) {
            rotation = glm::mat4(1.0f);
            if(i == 3){
                rotation = glm::rotate(rotation, rotate[i] * 6.28f, glm::vec3(0.0, 0.0, 1.0));
            }else{
                rotation = glm::rotate(rotation, rotate[i] * 6.28f, glm::vec3(0.0, 1.0, 0.0));
            }
            rotation = glm::rotate(rotation, rotate[i] * 6.28f, glm::vec3(0.0, 1.0, 0.0));
            glm::vec4 litDir = rotation * glm::vec4(lightDirections[i], 1.0);
            pbrShader.setVec3("lightDirections[" + std::to_string(i) + "]" , glm::vec3(litDir.x, litDir.y, litDir.z));
            pbrShader.setFloat("intensity[" + std::to_string(i) + "]", intensity[i]);
        }

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        pbrShader.setMat4("projection", projection);
        if(inCameraView){
            pbrShader.setMat4("view", cameraViewMat);
        }else{
            pbrShader.setMat4("view", view);
        }


        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(scale));
//        model = glm::rotate(model, -1.57f, glm::vec3(1.0, 0.0, 0.0));
        model = glm::translate(model, glm::vec3(x, y, z));

        pbrShader.setMat4("model", model);
        for (int i = 0; i < intensity.size(); ++i) {

        }

        pbrShader.setFloat("metalnessFactor", metalnessFactor);
        pbrShader.setFloat("roughnessFactor", roughnessFactor);
        pbrShader.setVec3("cameraPos", camera.Position);
        pbrShader.setFloat("ambient", ambient);
        ourModel.Draw(pbrShader);

        if(visualize){
            // render pyramid
            pyramidShader.use();
            int size = cameraPositions.size();
            pyramidShader.setInt("size", size);

            // view/projection transformations
            pyramidShader.setMat4("projection", projection);
            if(inCameraView){
                pyramidShader.setMat4("view", cameraViewMat);
            }else{
                pyramidShader.setMat4("view", view);
            }
            for (int i = 0; i < size; ++i) {
                glm::mat4 pyramidModel = glm::mat4(0.5f);
                pyramidModel = glm::transpose(glm::lookAt(glm::vec3(0.0), cameraPositions[i], glm::vec3(0.0, 1.0, 0.0))) * pyramidModel;
                pyramidModel[3][0] = cameraPositions[i][0];
                pyramidModel[3][1] = cameraPositions[i][1];
                pyramidModel[3][2] = cameraPositions[i][2];
                pyramidShader.setMat4("model", pyramidModel);
                glBindVertexArray(VAO);
                glDrawArrays(GL_LINES, 0, 16);
            }
        }

        {
            ImGui::Begin("Camera Sampler");                          // Create a window called "Hello, world!" and append into it.
            ImGui::Text("Hemisphere Sampler");
            ImGui::SliderFloat("Radius", &radius, 0.0f, 50.0f);
            ImGui::SliderInt("ThetaDensity", &thetaDensity, 1, 15);
            ImGui::SliderInt("PhiDensity", &phiDensity, 1, 15);

            // Buttons return true when clicked (most widgets return true when edited/activated)
            if (ImGui::Button("Generate Sample In Hemisphere"))  {
                visualize = true;
                // render multi frame and capture
                viewInHemisphere(radius, thetaDensity, phiDensity, cameraPositions);
            }


            ImGui::Text("Forward Facing Sampler");
            ImGui::Text("Forward Facing is defined to sample in a square");
            ImGui::SliderFloat("Rotation", &forwardFaceRotaion, 0.0f, 1.0f);
            ImGui::SliderFloat("Distance", &distance, 1.0f, 100.0f);
            ImGui::SliderFloat("Width", &width, 1.0f, 100.0f);
            ImGui::SliderFloat("Height", &height, 1.0f, 100.0f);
            ImGui::SliderInt("WidthDensity", &widthDensity, 1, 15);
            ImGui::SliderInt("HeightDensity", &heightDensity, 1, 15);

            // int distance ,int width, int height, int widthDensity, int heightDensity
            // Buttons return true when clicked (most widgets return true when edited/activated)
            if (ImGui::Button("Generate Forward Facing Sample"))  {
                visualize = true;
                // render multi frame and capture
                glm::vec4 originBias = glm::vec4(0.0, 0.0, distance, 0.0);
                originBias = glm::rotate(glm::mat4(1.0), forwardFaceRotaion * PI * 2, glm::vec3(0.0, 1.0, 0.0)) * originBias;
                glm::vec3 rotatedBias = glm::vec3(originBias.x, originBias.y, originBias.z);

                viewOfForwardFacing(rotatedBias, width, height, widthDensity, heightDensity, cameraPositions);
            }
            ImGui::Text("If you having generated camera view, you can press\n[ and ] to enter and switch among sampler view.\nTo quit sampler view , press backspace");
            ImGui::Text("\n");
            ImGui::Text("To render datasets, you need to generate samples first");
            if(ImGui::Button("Render datasets")){
                // mkdir for result
                // 先创建用于存放图片的文件夹
                string folderPath = exewd() + "\\output\\" + timestr();
                string command = "mkdir " + folderPath;
                system(command.c_str());
                pbrShader.use();
                // projection transformations
                glm::mat4 sampleProjection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
                pbrShader.setMat4("projection", sampleProjection);
                // model transformations
                glm::mat4 sampleModel = glm::mat4(1.0f);
                sampleModel = glm::scale(sampleModel, glm::vec3(scale));
//                model = glm::rotate(model, -1.57f, glm::vec3(1.0, 0.0, 0.0));
//                sampleModel = glm::rotate(sampleModel, -1.57f, glm::vec3(1.0, 0.0, 0.0)); // translate it down so it's at the center of the scene
                sampleModel = glm::translate(sampleModel,  glm::vec3(x, y, z));
                pbrShader.setMat4("model", sampleModel);
                for (int i = 0; i < cameraPositions.size(); ++i) {
                    glClearColor(clear_color.x, clear_color.y, clear_color.z, 0.0f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glm::mat4 sampleView = glm::lookAt(cameraPositions[i], glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    pbrShader.setMat4("view", sampleView);
                    ourModel.Draw(pbrShader);
                    CaptureScreen(i+1, folderPath);
                }
            }

            ImGui::End();
        }

        // Rendering gui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();


    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
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
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);


    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessMouseMovement(0.0f, 1.0f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessMouseMovement(0.0f, -1.0f);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessMouseMovement(-1.0f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessMouseMovement(1.0f, 0.0f);

    // camera mode
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS && visualize){
        inCameraView = true;
        cameraViewMat = glm::lookAt(cameraPositions[indexCamera], glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        indexCamera = (indexCamera + 1) % cameraPositions.size();
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS && visualize){
        inCameraView = true;
        cameraViewMat = glm::lookAt(cameraPositions[indexCamera], glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        indexCamera = (indexCamera - 1) % cameraPositions.size();
    }
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS){
        inCameraView = false;
        indexCamera = 0;
    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int SCR_WIDTH, int SCR_HEIGHT)
{
    // make sure the viewport matches the new window dimensions; note that SCR_WIDTH and
    // SCR_HEIGHT will be significantly larger than specified on retina displays.
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}



// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void CaptureScreen(int num, const string& folderPath){
    unsigned char * screenPixel = new unsigned char[SCR_WIDTH * SCR_HEIGHT * 4];
    glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenPixel);
    stbi_flip_vertically_on_write(true);
    stringstream ss;
    streambuf* buffer = cout.rdbuf(); //oldbuffer,STDOUT的缓冲区
    cout.rdbuf(ss.rdbuf());
    cout << folderPath << "\\IMG_" << setfill('0') << setw(4) << num << ".png";
    string path(ss.str());
    stbi_write_png(path.c_str(), SCR_WIDTH, SCR_HEIGHT, 4, screenPixel, 0);
    delete screenPixel;

}

void viewInHemisphere(float radius, int thetaDensity, int phiDensity, vector<glm::vec3>& cameraPositions){
    cameraPositions.clear();
    float thetaStride = PI / thetaDensity;
    float phiStride = PI / phiDensity;

    for (int i = 0; i <= thetaDensity; ++i) {
        for (int j = 0; j <= phiDensity; ++j) {
            // opengl坐标系的xyz轴并不是传统笛卡尔式 故采样是theta[0, pi] phi[-2/pi, 2/pi]
            float x = radius * sin(PI + phiStride * j) * cos(PI + thetaStride * i);
            float y = radius * sin(PI + phiStride * j) * sin(PI + thetaStride * i);
            float z = radius * cos(PI + phiStride * j);
            cameraPositions.push_back(glm::vec3(x, y, z));
        }
    }
}

void viewOfForwardFacing(glm::vec3 bias, float width, float height, int widthDensity, int heightDensity, vector<glm::vec3>& cameraPositions){
    cameraPositions.clear();
    float widthStride = width / widthDensity;
    float heightStride = height / heightDensity;
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 right = glm::normalize(glm::cross(bias, up));

    for (int i = 0; i <= widthDensity; ++i) {
        for (int j = 0; j <= heightDensity; ++j) {
            glm::vec3 currentVec = bias + up * widthStride * float(i - widthDensity/2) + right * heightStride * float(j - heightDensity/2);
            cameraPositions.push_back(currentVec);
        }
    }
}


string timestr(){
    time_t nowtime;
    time(&nowtime); //获取1970年1月1日0点0分0秒到现在经过的秒数
    tm* p = localtime(&nowtime); //将秒数转换为本地时间,年从1900算起,需要+1900,月为0-11,所以要+1
    return to_string(p->tm_year + 1900) + "." + to_string(p->tm_mon + 1) + "-" + to_string(p->tm_mday) + "_" + to_string(p->tm_hour) + "-" + to_string(p->tm_min) + "-" + to_string(p->tm_sec);
}


