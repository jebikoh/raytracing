#include "display.hpp"

#include <SDL.h>
#include <future>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <iostream>

const auto VERTEX_SOURCE = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;

        out vec2 TexCoord;
        uniform mat4 uTransform;

        void main() {
            // Apply transformation to position
            gl_Position = uTransform * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";

const auto FRAGMENT_SOURCE = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;

        uniform sampler2D uTexture;

        void main() {
            FragColor = texture(uTexture, TexCoord);
        }
    )";

static GLuint compileShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compilation error: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void setUiTheme() {
    auto &colors               = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg]  = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
    colors[ImGuiCol_MenuBarBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Border
    colors[ImGuiCol_Border]       = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
    colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.24f};

    // Text
    colors[ImGuiCol_Text]         = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
    colors[ImGuiCol_TextDisabled] = ImVec4{0.5f, 0.5f, 0.5f, 1.0f};

    // Headers
    colors[ImGuiCol_Header]        = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_HeaderActive]  = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button]        = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_ButtonActive]  = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_CheckMark]     = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};

    // Popups
    colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.1f, 0.13f, 0.92f};

    // Slider
    colors[ImGuiCol_SliderGrab]       = ImVec4{0.44f, 0.37f, 0.61f, 0.54f};
    colors[ImGuiCol_SliderGrabActive] = ImVec4{0.74f, 0.58f, 0.98f, 0.54f};

    // Frame BG
    colors[ImGuiCol_FrameBg]        = ImVec4{0.13f, 0.13, 0.17, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_FrameBgActive]  = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Tabs
    colors[ImGuiCol_Tab]                = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TabHovered]         = ImVec4{0.24, 0.24f, 0.32f, 1.0f};
    colors[ImGuiCol_TabActive]          = ImVec4{0.2f, 0.22f, 0.27f, 1.0f};
    colors[ImGuiCol_TabUnfocused]       = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Title
    colors[ImGuiCol_TitleBg]          = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBgActive]    = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]          = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4{0.24f, 0.24f, 0.32f, 1.0f};

    // Seperator
    colors[ImGuiCol_Separator]        = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};
    colors[ImGuiCol_SeparatorHovered] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};
    colors[ImGuiCol_SeparatorActive]  = ImVec4{0.84f, 0.58f, 1.0f, 1.0f};

    // Resize Grip
    colors[ImGuiCol_ResizeGrip]        = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
    colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.74f, 0.58f, 0.98f, 0.29f};
    colors[ImGuiCol_ResizeGripActive]  = ImVec4{0.84f, 0.58f, 1.0f, 0.29f};

    auto &style             = ImGui::GetStyle();
    style.TabRounding       = 4;
    style.ScrollbarRounding = 9;
    style.WindowRounding    = 7;
    style.GrabRounding      = 3;
    style.FrameRounding     = 3;
    style.PopupRounding     = 4;
    style.ChildRounding     = 4;
}

Display::Display(const int width, const int height, Camera *camera)
    : width_(width),
      height_(height),
      renderWidth_(0),
      scaleX_(0),
      scaleY_(0),
      window_(nullptr),
      glContext_(nullptr),
      textureId_(0),
      shaderProgram_(0),
      vao_(0),
      vbo_(0),
      ebo_(0),
      camera_(camera) {}

bool Display::init() {
    if (!initWindow()) {
        std::cerr << "Failed to initialize window" << std::endl;
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD.\n";
        return false;
    }

    if (!initShaders()) {
        std::cerr << "Failed to initialize display shaders" << std::endl;
        return false;
    }

    initUI();

    initQuad();

    updateScale();

    return true;
}

void Display::destroy() const {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Display::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, camera_->width, camera_->height, 0, GL_RGB, GL_UNSIGNED_BYTE, camera_->img.data());

    // renderwidth
    glViewport(0, 0, renderWidth_, height_);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram_);
    glBindVertexArray(vao_);

    // scaleX, scaleY
    GLint transformLoc = glGetUniformLocation(shaderProgram_, "uTransform");
    if (transformLoc != -1) {
        // Construct a scale matrix
        const float transform[16] = {
                scaleX_, 0.0f, 0.0f, 0.0f,
                0.0f, scaleY_, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f};
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glUniform1i(glGetUniformLocation(shaderProgram_, "uTexture"), 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glViewport(0, 0, width_, height_);

    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width_ - SIDEBAR_WIDTH), 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(SIDEBAR_WIDTH), static_cast<float>(height_)), ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Sidebar", nullptr, window_flags);

    bool inputDisabled = false;
    if (isRendering_) {
        inputDisabled = true;
        ImGui::BeginDisabled();
    }

    if (ImGui::Button("Render")) {
        renderWorld();
    }

    if (ImGui::TreeNode("Configuration")) {
        ImGui::SeparatorText("Image");
        int dimensions[2] = {camera_->width, camera_->height};
        ImGui::Text("Dimensions");

        if (ImGui::InputInt2("##Dimensions", dimensions)) {
            camera_->width  = dimensions[0];
            camera_->height = dimensions[1];
        }

        ImGui::SeparatorText("Ray Tracing");

        ImGui::Text("SPP");
        ImGui::InputInt("##SPP", &camera_->samplesPerPx);

        ImGui::Text("Max Depth");
        ImGui::InputInt("##MaxDepth", &camera_->maxDepth);

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Camera Settings")) {
        ImGui::SeparatorText("Orientation");

        ImGui::Text("Position");
        // cast the current position to floats (stored as doubles)
        auto pos = static_cast<jtx::Vec3f>(camera_->center);
        if (ImGui::InputFloat3("##Position", &pos.x)) {
            camera_->center = static_cast<Vec3>(pos);
        }

        ImGui::Text("Target");
        auto target = static_cast<jtx::Vec3f>(camera_->target);
        if (ImGui::InputFloat3("##Target", &target.x)) {
            camera_->target = static_cast<Vec3>(target);
        }

        ImGui::Text("Up");
        auto up = static_cast<jtx::Vec3f>(camera_->up);
        if (ImGui::InputFloat3("##Up", &up.x)) {
            camera_->up = static_cast<Vec3>(up);
        }

        ImGui::SeparatorText("Lens");

        ImGui::Text("Y-FOV");
#ifdef RT_DOUBLE_PRECISION
        ImGui::InputDouble("##Y-FOV", &camera_->yfov);
#else
        ImGui::InputFloat("##Y-FOV", &camera_->yfov);
#endif

        ImGui::Text("Focus Angle");
#ifdef RT_DOUBLE_PRECISION
        ImGui::InputDouble("##FocusAngle", &camera_->defocusAngle);
#else
        ImGui::InputFloat("##FocusAngle", &camera_->defocusAngle);
#endif

        ImGui::Text("Focus Distance");
#ifdef RT_DOUBLE_PRECISION
        ImGui::InputDouble("##FocusDistance", &camera_->focusDistance);
#else
        ImGui::InputFloat("##FocusDistance", &camera_->focusDistance);
#endif

        ImGui::TreePop();
    }

    if (inputDisabled) {
        ImGui::EndDisabled();
    }
    ImGui::End();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window_);
}

bool Display::initWindow() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window_ = SDL_CreateWindow("Ray Tracer",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               width_,
                               height_,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window_) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << "\n";
        return false;
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
        std::cerr << "Failed to create GL context: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_GL_MakeCurrent(window_, glContext_);
    SDL_GL_SetSwapInterval(1);
    return true;
}

bool Display::initShaders() {
    const auto vertex = compileShader(GL_VERTEX_SHADER, VERTEX_SOURCE);
    if (!vertex) { return false; }

    const auto fragment = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SOURCE);
    if (!fragment) {
        glDeleteShader(vertex);
        return false;
    }

    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertex);
    glAttachShader(shaderProgram_, fragment);
    glLinkProgram(shaderProgram_);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return true;
}

void Display::initQuad() {
    constexpr float vertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 1.0f};

    const unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0};

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Texture
    glGenTextures(1, &textureId_);
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Display::initUI() const {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    const ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../assets/proggyvector.ttf", 14);
    (void) io;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window_, glContext_);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Display::renderWorld() {
    if (isRendering_) return;

    isRendering_ = true;
    std::thread([this]() {
        camera_->render(*world);
        isRendering_ = false;
    }).detach();
}

void Display::updateScale() {
    renderWidth_ = width_ - SIDEBAR_WIDTH;

    const auto imageWidth  = static_cast<float>(camera_->width);
    const auto imageHeight = static_cast<float>(camera_->height);

    const float scale = std::min(static_cast<float>(renderWidth_) / imageWidth, static_cast<float>(height_) / imageHeight);

    const float scaledWidth  = imageWidth * scale;
    const float scaledHeight = imageHeight * scale;

    scaleX_ = scaledWidth / static_cast<float>(renderWidth_);
    scaleY_ = scaledHeight / static_cast<float>(height_);
}

void Display::processEvents(bool &isRunning) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);

        if (e.type == SDL_QUIT) {
            isRunning = false;
        }
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
            width_  = e.window.data1;
            height_ = e.window.data2;
            glViewport(0, 0, width_, height_);
            updateScale();
        }
    }
}