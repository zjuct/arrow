#include "ui.hpp"
#include "control.h"
#include "ui.hpp"
#include <ft2build.h>
#include <sstream>
#include <string>
#include FT_FREETYPE_H

extern Shader *text_shader;
static UI *ui = UI::getInstance();
static Control *control = Control::getInstance();

unsigned int F_VAO, F_VBO; // 渲染字体专用
std::map<GLchar, Character> Characters;

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
/* ================================= class Background ================================ */
void Background::init()
{
    float v[20] = {
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f};
    unsigned int idx[6] = {0, 1, 3, 0, 2, 3};

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

void Background::draw(Shader *shader)
{
    shader->use();
    shader->setBool("has_texture", true);

    int tex = TextureMgr::getInstance()->gettex(texname, TEX_2D);
    if (tex < 0)
    {
        std::cerr << "[ERROR] No texture." << std::endl;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex); // 将纹理数据绑定到纹理单元

    glm::vec3 color(1.0f);
    glm::mat4 model(1.0f);
    shader->setvec3fv("color", glm::value_ptr(color));
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(VAO);
    glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
    glEnable(GL_DEPTH_TEST);
}
/* ================================= class Button ==================================== */

void Button::init()
{
    float v[20];
    v[0] = position.x, v[1] = position.y, v[2] = 0.0f, v[3] = 0.0f, v[4] = 0.0f;
    v[5] = position.x + width, v[6] = position.y, v[7] = 0.0f, v[8] = 1.0f, v[9] = 0.0f;
    v[10] = position.x, v[11] = position.y - height, v[12] = 0.0f, v[13] = 0.0f, v[14] = 1.0f;
    v[15] = position.x + width, v[16] = position.y - height, v[17] = 0.0f, v[18] = 1.0f, v[19] = 1.0f;

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

void Button::handleMousePress(int button, int action)
{
    if (action == GLFW_PRESS)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_RIGHT:
            break;
        case GLFW_MOUSE_BUTTON_LEFT:
            std::cerr << "[DEBUG] Left button pressed." << std::endl;
            if (state == BUTTON_ON)
                clicked = true;
            break;
        }
    }
    if (action == GLFW_RELEASE)
    {
        clicked = false;
    }
}
void Button::handleMouseMove(double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    xpos = xpos / control->wwidth * 2;
    ypos = ypos / control->wheight * 2;
    xpos = xpos - 1;
    ypos = 1 - ypos;

    if (xpos > position.x && xpos < position.x + width &&
        ypos < position.y && ypos > position.y - height)
    {
        state = BUTTON_ON;
        //   std::cout << "set button on" << std::endl;
    }
    else
        state = BUTTON_OFF;
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

/* ================================ class Text =========================================*/
void Text::init() {}
void Text::draw(Shader *shader)
{
    GLfloat x = posx;
    GLfloat y = posy;

    // Activate corresponding render state
    shader->use();
    shader->setvec3fv("textColor", glm::value_ptr(color));
    glm::mat4 m = glm::mat4(1.0f);
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(F_VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = content.begin(); c != content.end(); c++)
    {

        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            {xpos, ypos + h, 0.0, 0.0},
            {xpos, ypos, 0.0, 1.0},
            {xpos + w, ypos, 1.0, 1.0},

            {xpos, ypos + h, 0.0, 0.0},
            {xpos + w, ypos, 1.0, 1.0},
            {xpos + w, ypos + h, 1.0, 0.0}};
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, F_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Text::setText(std::string _text)
{
    this->content = _text;
}
/* ================================ class Rectangle =====================================*/

void Rectangle::init()
{
    float v[20];
    v[0] = position.x, v[1] = position.y, v[2] = 0.0f, v[3] = 0.0f, v[4] = 0.0f;
    v[5] = position.x + width, v[6] = position.y, v[7] = 0.0f, v[8] = 1.0f, v[9] = 0.0f;
    v[10] = position.x + width, v[11] = position.y - height, v[12] = 0.0f, v[13] = 1.0f, v[14] = 1.0f;
    v[15] = position.x, v[16] = position.y - height, v[17] = 0.0f, v[18] = 0.0f, v[19] = 1.0f;

    if (has_texture)
        TextureMgr::getInstance()->load(texname, TEX_2D);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void Rectangle::draw(Shader *shader)
{
    glm::mat4 m = glm::mat4(1.0f);
    float v[20];
    v[0] = position.x, v[1] = position.y, v[2] = 0.0f, v[3] = 0.0f, v[4] = 0.0f;
    v[5] = position.x + width, v[6] = position.y, v[7] = 0.0f, v[8] = 1.0f, v[9] = 0.0f;
    v[10] = position.x + width, v[11] = position.y - height, v[12] = 0.0f, v[13] = 1.0f, v[14] = 1.0f;
    v[15] = position.x, v[16] = position.y - height, v[17] = 0.0f, v[18] = 0.0f, v[19] = 1.0f;

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
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (wireFrame)
    {
        glPolygonMode(GL_FRONT, GL_LINE);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
    else
    {
        glPolygonMode(GL_FRONT, GL_FILL);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glPolygonMode(GL_FRONT, GL_FILL);
}

/* ================================ class UI =================================== */

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

    bt.handleMousePress(button, action);

    if (action == GLFW_PRESS)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_RIGHT:
            std::cerr << "[DEBUG] Right button pressed." << std::endl;
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
    bt.handleMouseMove(xposIn, yposIn);
}
void UI::handleKeyInput(int key, int action)
{
    if (levelUpBuff.size())
    {
        if (action == GLFW_PRESS)
        {
            Buff b1, b2, b3;
            std::tie(b1, b2, b3) = levelUpBuff.front();
            if (key == GLFW_KEY_1)
            {
                std::cout << "choose 1: do something" << std::endl;
                levelUpBuff.pop_front();
                control->players[PLAYER_ID].getBuff(b1);
            }
            else if (key == GLFW_KEY_2)
            {
                std::cout << "choose 2: do something" << std::endl;
                levelUpBuff.pop_front();
                control->players[PLAYER_ID].getBuff(b2);
            }
            else if (key == GLFW_KEY_3)
            {
                std::cout << "choose 3: do something" << std::endl;
                levelUpBuff.pop_front();
                control->players[PLAYER_ID].getBuff(b3);
            }
        }
    }
}
void UI::handleScroll(double xoffset, double yoffset)
{
}

void UI::init()
{
    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "resource/assets/fonts/Antonio-Bold.ttf", 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x};
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &F_VAO);
    glGenBuffers(1, &F_VBO);
    glBindVertexArray(F_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, F_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // levelUp = false;
    gstate = GLOBAL_INIT;
    glfwSetInputMode(control->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    aim.init();
    option[0] = Text("Option 1: ....", -0.6f, 0.6f, 0.002, glm::vec3(0.95f));
    option[1] = Text("Option 2: ....", -0.6f, 0.4f, 0.002, glm::vec3(0.95f));
    option[2] = Text("Option 3: ....", -0.6f, 0.2f, 0.002, glm::vec3(0.95f));
    blood_1 = Rectangle(glm::vec3(0.4, -0.5, 0), 0.5f, 0.15f, true, glm::vec3(1.0f));
    blood = Rectangle(glm::vec3(0.41, -0.51, 0), 0.48f, 0.13f, false, glm::vec3(1.0f));
    experience_1 = Rectangle(glm::vec3(0.4, -0.66, 0), 0.5f, 0.02f, true, glm::vec3(1.0f));
    experience = Rectangle(glm::vec3(0.4, -0.66, 0), 0.5f, 0.02f, false, glm::vec3(1.0f, 0.8f, 0.0f));
    std::stringstream ss;
    std::string _level;
    ss << control->players[PLAYER_ID].level;
    ss >> _level;
    level = Text("Level: " + _level, 0.4f, -0.49f, 0.002, glm::vec3(1.0f));

    bt = Button(glm::vec3(0.2f, 0.0f, 0.0f), 0.6f, 0.5f, true, "resource/assets/button/btn.png");
    bt.init();
    bg.init();
    blood_1.init();
    blood.init();
    experience_1.init();
    experience.init();
    control->ground.updateModel();
// enum Buff
// {
//     BUFF_SPEED_UP,
//     BUFF_JUMP_HEIGHT_UP,
//     BUFF_JUMP_TIME_UP,
//     BUFF_ARROW_SPEED_UP,
//     BUFF_ARROW_LOAD_TIME_DOWN,
//     BUFF_ARROW_STRENGTH_TIME_DOWN,
//     BUFF_ARROW_DAMAGE_UP,
//     BUFF_ARROW_REFLECT,
//     BUFF_ARROW_LASER,
//     BUFF_ARROW_NOT_REFLECT,
//     BUFF_ARROW_NORMAL,
//     BUFF_LIVE_TIME_UP,
//     BUFF_HP_RECOVER,
//     BUFF_MAXHP_UP,
//     BUFF_NUM,

// };
    BuffName.resize(BUFF_NUM);
    BuffName[BUFF_SPEED_UP] = "Player Speed Up";
    BuffName[BUFF_JUMP_HEIGHT_UP] = "Player Jump Height Up";
    BuffName[BUFF_JUMP_TIME_UP] = "Player Jump Time Up";
    BuffName[BUFF_ARROW_SPEED_UP] = "Arrow Speed Up";
    BuffName[BUFF_ARROW_LOAD_TIME_DOWN] = "Arrow Reloading Time Down";
    BuffName[BUFF_ARROW_STRENGTH_TIME_DOWN] = "Player Charging Time Down";
    BuffName[BUFF_ARROW_DAMAGE_UP] = "Arrow Damage Up";
    BuffName[BUFF_ARROW_REFLECT] = "Arrow Reflect";
    BuffName[BUFF_ARROW_LASER] = "Arrow Laser(No Gravity)";
    BuffName[BUFF_ARROW_NOT_REFLECT] = "Arrow Not Reflect";
    BuffName[BUFF_ARROW_NORMAL] = "Arrow Normal(With Gravity)";
    BuffName[BUFF_LIVE_TIME_UP] = "Arrow Live Time Up (for reflect arrow)";
    BuffName[BUFF_HP_RECOVER] = "HP Recover";
    BuffName[BUFF_MAXHP_UP] = "Max HP Up";

}

void UI::updateBlood()
{
    blood.setWidth(0.48f * control->players[PLAYER_ID].hp / control->players[PLAYER_ID].maxHp);
}

void UI::updateExperience()
{
    // printf("oldWidth: %f\nexp: %d\nlimit: %d\n", experience.getWidth(), control->players[PLAYER_ID].exp, control->players[PLAYER_ID].expToLevelUp[control->players[PLAYER_ID].level]);
    experience.setWidth(0.5f * (float)(control->players[PLAYER_ID].exp) / control->players[PLAYER_ID].expToLevelUp[control->players[PLAYER_ID].level]);
    // printf("newWidth: %f\n\n", experience.getWidth());
}

void UI::updateLevel()
{
    std::string newLevel;
    std::stringstream ss;
    ss << control->players[PLAYER_ID].level;
    ss >> newLevel;
    level.setText("Level: " + newLevel);
    // option[0].setText("123213" + newLevel);
}

void UI::update()
{
    gstate_last = gstate;
    if (gstate == GLOBAL_GAME)
    {
        updateBlood();
        updateExperience();
        updateLevel();
    }
    else if (gstate == GLOBAL_INIT)
    {
        if (bt.clicked)
        {
            gstate = GLOBAL_GAME;
            glfwSetInputMode(control->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void UI::setLevelUp(Buff b1, Buff b2, Buff b3)
{
    levelUpBuff.push_back(std::make_tuple(b1, b2, b3));
}

void UI::updateModel()
{
    switch (gstate)
    {
    case GLOBAL_GAME:
        for (auto &[_, player] : control->players)
            player.updateModel();
        control->arrowMgr->updateModel();
        control->candyMgr->updateModel();
        break;

    case GLOBAL_INIT:
        break;
    }
}

void UI::draw(Shader *shader)
{

    shader->use();
    switch (gstate)
    {
    case GLOBAL_GAME:
#if PRT_ENABLE
        if(shader == shadowmap_shader) {
            control->ground.draw(shader);
        } else {
            control->ground.draw(prt_shader);
        }
#else
        control->ground.draw(shader);
#endif
        for (auto &[_, player] : control->players)
            player.draw(shader);
        control->arrowMgr->draw(shader);
        control->candyMgr->draw(shader);
        aim.draw(flat_shader);
        blood_1.draw(flat_shader);
        blood.draw(flat_shader);
        experience_1.draw(flat_shader);
        experience.draw(flat_shader);
        level.draw(text_shader);

        if (levelUpBuff.size())
        {
            option[0].setText("Buff 1: " + BuffName[std::get<0>(levelUpBuff.front())]);
            option[1].setText("Buff 2: " + BuffName[std::get<1>(levelUpBuff.front())]);
            option[2].setText("Buff 3: " + BuffName[std::get<2>(levelUpBuff.front())]);
            for (int i = 0; i < 3; ++i)
                option[i].draw(text_shader);
        }
        break;

    case GLOBAL_INIT:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        bg.draw(flat_shader);
        bt.draw(flat_shader);
        break;
    }
}

// void RenderText(Shader *shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
// {
//     // Activate corresponding render state
//     shader->use();
//     shader->setvec3fv("textColor", glm::value_ptr(color));
//     glm::mat4 m = glm::mat4(1.0f);
//     shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));

//     glActiveTexture(GL_TEXTURE0);
//     glBindVertexArray(F_VAO);

//     // Iterate through all characters
//     std::string::const_iterator c;
//     for (c = text.begin(); c != text.end(); c++)
//     {
//         Character ch = Characters[*c];

//         GLfloat xpos = x + ch.Bearing.x * scale;
//         GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

//         GLfloat w = ch.Size.x * scale;
//         GLfloat h = ch.Size.y * scale;
//         // Update VBO for each character
//         GLfloat vertices[6][4] = {
//             { xpos,     ypos + h,   0.0, 0.0 },
//             { xpos,     ypos,       0.0, 1.0 },
//             { xpos + w, ypos,       1.0, 1.0 },

//             { xpos,     ypos + h,   0.0, 0.0 },
//             { xpos + w, ypos,       1.0, 1.0 },
//             { xpos + w, ypos + h,   1.0, 0.0 }
//         };
//         // Render glyph texture over quad
//         glBindTexture(GL_TEXTURE_2D, ch.TextureID);
//         // Update content of VBO memory
//         glBindBuffer(GL_ARRAY_BUFFER, F_VBO);
//         glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
//         glBindBuffer(GL_ARRAY_BUFFER, 0);
//         // Render quad
//         glDrawArrays(GL_TRIANGLES, 0, 6);
//         // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
//         x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
//     }
//     glBindVertexArray(0);
//     glBindTexture(GL_TEXTURE_2D, 0);
// }