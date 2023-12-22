#include "ui.hpp"
#include "control.h"

/* ============================= class Aim ================================ */

void Aim::draw(Shader *shader)
{
    glm::mat4 m = glm::mat4(1.0);
    switch (state)
    {
    case AIM_STILL:

        m = glm::scale(m, glm::vec3(0.05, 0.05, 0.05));
        m = glm::scale(m, glm::vec3((float)WHEIGHT / WWIDTH, 1.0, 1.0)); // 按窗口长宽比缩放
        shader->use();
        shader->setBool("has_texture", false);
        shader->setvec3("color", 1.0, 1.0, 1.0);
        shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));
        break;

    case AIM_FIRE:
        m = glm::scale(m, glm::vec3(0.06, 0.06, 0.06));
        m = glm::scale(m, glm::vec3((float)WHEIGHT / WWIDTH, 1.0, 1.0)); // 按窗口长宽比缩放
        shader->use();
        shader->setBool("has_texture", false);
        shader->setvec3("color", 0.8f, 0.0f, 0.0f);
        shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));
    }

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
    glEnable(GL_DEPTH_TEST);
}

void Aim::init()
{
    float vertices[72] = {
        -0.4,
        1.0,
        0.0,
        -0.3,
        1.0,
        0.0,
        0.3,
        1.0,
        0.0,
        0.4,
        1.0,
        0.0,
        -1.0,
        0.4,
        0.0,
        -0.4,
        0.4,
        0.0,
        0.4,
        0.4,
        0.0,
        1.0,
        0.4,
        0.0,
        -1.0,
        0.3,
        0.0,
        -0.3,
        0.3,
        0.0,
        0.3,
        0.3,
        0.0,
        1.0,
        0.3,
        0.0,
        -0.4,
        -1.0,
        0.0,
        -0.3,
        -1.0,
        0.0,
        0.3,
        -1.0,
        0.0,
        0.4,
        -1.0,
        0.0,
        -1.0,
        -0.4,
        0.0,
        -0.4,
        -0.4,
        0.0,
        0.4,
        -0.4,
        0.0,
        1.0,
        -0.4,
        0.0,
        -1.0,
        -0.3,
        0.0,
        -0.3,
        -0.3,
        0.0,
        0.3,
        -0.3,
        0.0,
        1.0,
        -0.3,
        0.0,
    };

    int indices[48] = {
        8, 9, 4, 4, 9, 5, 5, 9, 0, 0, 9, 1,
        2, 10, 3, 3, 10, 6, 6, 10, 7, 7, 10, 11,
        20, 21, 16, 16, 21, 17, 17, 21, 12, 12, 21, 13,
        14, 22, 15, 15, 22, 18, 18, 22, 19, 19, 22, 23};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// void genCircle()
// {
//   for (int i = 0; i < SEGMENTS; ++i) {
//     float x = cos(2 * M_PIf * i / SEGMENTS);
//     float z = 0;
//     float y = sin(2 * M_PIf * i / SEGMENTS);
//     circle.push_back(x);
//     circle.push_back(y);
//     circle.push_back(z);
//   }
//   for (int i = 0; i < circle.size(); i += 3) {
//     for (int j = 0; j < 3; ++j)
//      std::cout << circle[i+j] << " ";
//     std::cout << std::endl;
//   }
// }

void Aim::setState(enum AimState s)
{
    state = s;
}

/* ================================= class Button ==================================== */

void Button::init()
{
    float v[20];
    v[0] = position.x, v[1] = position.y, v[2] = 0.0f, v[3] = 0.0f, v[4] = 1.0f;
    v[5] = position.x + width, v[6] = position.y, v[7] = 0.0f, v[8] = 1.0f, v[9] = 1.0f;
    v[10] = position.x, v[11] = position.y - height, v[12] = 0.0f, v[13] = 0.0f, v[14] = 0.0f;
    v[15] = position.x + width, v[16] = position.y - height, v[17] = 0.0f, v[18] = 1.0f, v[19] = 0.0f;

    int idx[6] = {0, 1, 3, 0, 2, 3};

    TextureMgr::getInstance()->load(texname, TEX_2D);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Button::draw(Shader *shader)
{
    glm::mat4 m = glm::mat4(1.0f);
    if (state == BUTTON_ON)
    {
        m = glm::scale(m, glm::vec3(1.1f));
    }
    else
    {
    }
    shader->use();
    shader->setBool("has_texture", has_texture);
    if (has_texture)
    {
        int tex = TextureMgr::getInstance()->gettex(texname, TEX_2D);
        if (tex < 0)
        {
            std::cerr << "[ERROR] No texture." << std::endl;
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex); // 将纹理数据绑定到纹理单元
    }

    shader->setvec3fv("color", glm::value_ptr(color));
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
}

/* ================================ class UI =================================== */

static UI *ui = UI::getInstance();
static Control *control = Control::getInstance();

UI::UI() : gstate(GLOBAL_INIT)
{
}

UI *UI::getInstance() // 在本文件中声明/找到静态单例，然后返回
{
    static UI ui;
    return &ui;
}

void UI::handleMousePress(int button, int action)
{
    if (action == GLFW_PRESS)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_RIGHT:
            std::cerr << "[DEBUG] Right button pressed." << std::endl;
            gstate = GLOBAL_GAME;
            leftPress = true;
            break;
        case GLFW_MOUSE_BUTTON_LEFT:
            std::cerr << "[DEBUG] Left button pressed." << std::endl;
            if (gstate == GLOBAL_GAME)
                aim.setState(AimState::AIM_FIRE);
            rightPress = true;
            break;
        }
    }
    else if (action == GLFW_RELEASE)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_RIGHT:
            std::cerr << "[DEBUG] Right button released." << std::endl;
            leftPress = false;
            break;
        case GLFW_MOUSE_BUTTON_LEFT:
            std::cerr << "[DEBUG] Left button released." << std::endl;
            rightPress = false;
            aim.setState(AimState::AIM_STILL);
            break;
        }
    }
}
void UI::handleMouseMove(double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    // cout << "x: " << xpos << " y: " << ypos << endl;
}
void UI::handleKeyInput(int key, int action)
{
}
void UI::handleScroll(double xoffset, double yoffset)
{
}

void UI::init()
{
    gstate = GLOBAL_INIT;
    shader = flat_shader;
    aim.init();
    Button btn_test(glm::vec3(-0.5f, 0.5f, 0.0f), 1.0f, 1.0f, true, "resource/assets/button/btn.png");
    btn_test.init();
    btns.push_back(btn_test);
    control->ground.updateModel();
}

void UI::updateModel()
{
    switch (gstate)
    {
    case GLOBAL_GAME:
        for (auto &player : control->players)
            player.updateModel();
        control->arrowMgr->updateModel();
        control->candyMgr->updateModel();
        break;

    case GLOBAL_INIT:
        break;
    }
}
void UI::draw()
{

    shader->use();
    switch (gstate)
    {
    case GLOBAL_GAME:
        control->ground.draw();
        for (auto &player : control->players)
            player.draw();
        control->arrowMgr->draw();
        control->candyMgr->draw();
        aim.draw(shader);
        break;

    case GLOBAL_INIT:
        for (auto btn : btns)
        {
            btn.draw(shader);
        }
        break;
    }
}