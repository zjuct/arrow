#include "player.h"
#include "shader.h"
#include <control.h>
#include <defs.h>
#include <glm/glm.hpp>

#include <vector>

#include <iostream>
#include <sync.hpp>

static Control *control = Control::getInstance();
static ArrowManager *arrowMgr = ArrowManager::getInstance();
static CandyManager *candyMgr = CandyManager::getInstance();

Player::Player() : position(glm::vec3(0.0f)), speed(2.5f), sensitivity(0.1f), yaw(-90.0f), pitch(0.0f), lastyaw(-90.0f)
{
}

Player::~Player()
{
}

void Player::init(const char *objfile, glm::vec3 position)
{
    static int cnt = 0;
    id = cnt++;
    this->position = position;
    jumpSpeed = 0.0f;
    updatePlayerVectors();

    obj = Scene::LoadObj(objfile);
    std::vector<Mesh> &meshes = obj->getMesh();
    body = Object(OBJECT_MESH, &meshes[0], player_shader);
    head = Object(OBJECT_MESH, &meshes[5], player_shader);
    larm = Object(OBJECT_MESH, &meshes[3], player_shader);
    lleg = Object(OBJECT_MESH, &meshes[4], player_shader);
    rarm = Object(OBJECT_MESH, &meshes[1], player_shader);
    rleg = Object(OBJECT_MESH, &meshes[2], player_shader);
}

void Player::processKeyboard()
{
    // float velocity = speed * deltaTime;
    if (id == PLAYER_ID)
    {
        inputDir = glm::vec3(0.0f);
        if (state == PLAYER_DEAD)
            return;
        if (control->frontPress)
            inputDir += glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        if (control->backPress)
            inputDir -= glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        if (control->leftPress)
            inputDir -= right;
        if (control->rightPress)
            inputDir += right;
        if (inputDir != glm::vec3(0.0f))
        {
            this->state = PLAYER_RUN;
            inputDir = glm::normalize(inputDir);
        }
        else
            this->state = PLAYER_STILL;
    }
    else if (id == ANOTHER_PLAYER_ID)
    {
        inputDir = glm::vec3(0.0f);
        if (state == PLAYER_DEAD)
            return;
        if (control->another_frontPress)
            inputDir += glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        if (control->another_backPress)
            inputDir -= glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        if (control->another_leftPress)
            inputDir -= right;
        if (control->another_rightPress)
            inputDir += right;
        if (inputDir != glm::vec3(0.0f))
        {
            this->state = PLAYER_RUN;
            inputDir = glm::normalize(inputDir);
        }
        else
            this->state = PLAYER_STILL;
    }
}
void Player::jump()
{
    //	std::cout << "jump" << std::endl;
    if (jumpTime <= 0)
        return;
    --jumpTime;
    jumpSpeed = 1.0f * jumpHeight;
    //	std::cout << "time: " << jumpTime << std::endl;
}
void Player::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (pitch > 30.0f)
            pitch = 30.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updatePlayerVectors();
}

// 根据 pitch 和 yaw 修改 front right up 等
void Player::updatePlayerVectors()
{
    // calculate the new Front vector
    glm::vec3 f;

    // 这里调用的是 cmath 库中的三角函数，输入为弧度制
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = glm::sin(glm::radians(pitch));
    f.z = glm::sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->front = glm::normalize(f);
    // also re-calculate the Right and Up vector
    this->right = glm::normalize(glm::cross(front, control->camera.WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    this->up = glm::normalize(glm::cross(right, front));
}

void Player::draw()
{
    //    head.getObb()->drawFlag = true;
    //    body.getObb()->drawFlag = true;
    //    larm.getObb()->drawFlag = true;
    //    lleg.getObb()->drawFlag = true;
    //    rarm.getObb()->drawFlag = true;
    //    rleg.getObb()->drawFlag = true;
    head.draw();
    body.draw();
    larm.draw();
    rarm.draw();
    lleg.draw();
    rleg.draw();
}

bool Player::checkBlocked(enum intersectType type)
{
    for (auto obj : control->ground.getModel().getChildren())
    {
        if (body.getObb()->intersectWith(*(obj->getObb())) == type ||
            rleg.getObb()->intersectWith(*(obj->getObb())) == type ||
            lleg.getObb()->intersectWith(*(obj->getObb())) == type) // 小人踢墙，镜头抖动
        // if (body.getObb()->intersectWith(*(obj->getObb())) == type)
        {
            if (type == INTERSECT_UNDER)
                obj->getObb()->drawFlag = 100;
            return true;
        }
    }
    return false;
}
bool Player::navigate(float speedfactor, float anglefactor, float dt)
{
    if (glm::dot(inputDir, inputDir) < EPS)
    {
        return false;
    }

    glm::vec3 moveDir;
    for (int i = 0; i < 6; ++i)
    {
        glm::mat4 m = glm::rotate(glm::mat4(1.0), M_PIf * anglefactor * i / 10.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        moveDir = m * glm::vec4(inputDir, 1.0);
        // position += moveDir * speed * glm::dot(moveDir, inputDir) * speedfactor * dt;
        position += moveDir * speed * speedfactor * dt;
        updateModel_obb();
        if (!checkBlocked(INTERSECT_SOMETHING))
        {
            return true;
        }
        // position -= moveDir * speed * glm::dot(moveDir, inputDir) * speedfactor * dt;
        position -= moveDir * speed * speedfactor * dt;
    }
    for (int i = 0; i < 6; ++i)
    {
        glm::mat4 m = glm::rotate(glm::mat4(1.0), M_PIf * anglefactor * i / 10.0f, glm::vec3(0.0f, -1.0f, 0.0f));
        moveDir = m * glm::vec4(inputDir, 1.0);
        // position += moveDir * speed * glm::dot(moveDir, inputDir) * speedfactor * dt;
        position += moveDir * speed * speedfactor * dt;
        updateModel_obb();
        if (!checkBlocked(INTERSECT_SOMETHING))
        {
            return true;
        }
        // position -= moveDir * speed * glm::dot(moveDir, inputDir) * speedfactor * dt;
        position -= moveDir * speed * speedfactor * dt;
    }

    return false;
}

void Player::updatey(float dt)
{
    float radix = 1.0f;
    float oldy = position.y;
    for (int ct = 0; ct < CHECK_TIME; ++ct)
    {
        position.y = oldy + jumpSpeed * dt * radix;
        updateModel_obb();

        if ((jumpSpeed > 0) ? checkBlocked(INTERSECT_ON) : checkBlocked(INTERSECT_UNDER))
        {
            if (ct > 0)
                printf("jump backtrace time %d\n", ct);
            radix *= 0.5;
        }
        else
        {
            break;
        }
    }
}

void Player::update(float dt)
{
    processKeyboard();
    candyMgr->eat(*this);
    switch (this->state)
    {
    case PLAYER_STILL:
        theta = 0.0f;
        break;
    case PLAYER_RUN:
        // std::cout << "update" << std::endl;
        theta += omega * dt * 3.0f;
        if (theta > M_PIf / 6.0f)
        {
            theta = M_PIf / 6.0f;
            omega = -omega;
        }
        else if (theta < -M_PIf / 6.0f)
        {
            theta = -M_PIf / 6.0f;
            omega = -omega;
        }
        break;
    }
    if (fireTime > 0.0f)
        fireTime -= dt / arrowMgr->getArrowSetting(id).loadTime;
    else if (control->leftMousePress && id == PLAYER_ID)
        fireTime -= dt / arrowMgr->getArrowSetting(id).strengthTime;
    if (fireTime < -1.0f)
        fireTime = -1.0f;

    if (checkBlocked(INTERSECT_ON))
    {
        jumpSpeed = (jumpSpeed > 0) ? jumpSpeed : 0;
        jumpTime = (jumpSpeed > 0) ? jumpTime : maxJumpTime;
    }
    else
    {
        jumpSpeed -= GRAVITY * dt;
    }

    if (checkBlocked(INTERSECT_UNDER))
    {
        jumpSpeed = -jumpSpeed;
    }

    // position.y += jumpSpeed * dt;
    updatey(dt);

    // if(id == PLAYER_ID)
    // 	std::cout<<"downBlocked: "<<downBlocked()<<" "<<"aroundBlocked: "<<aroundBlocked()<<std::endl;

    if (checkBlocked(INTERSECT_SOMETHING))
    { // 如果在位置更新前，就已经碰撞，需要允许人物能走出来
      // printf("escape.\n");
        navigate(3.0, 2.0, dt);
    }
    else
    { // 否则，更新位置，如果发生碰撞则撤销
        navigate(1.0, 1.0, dt);
    }

    // std::cout<<"jumpSpeed: "<<jumpSpeed<<std::endl;
    if (position.y <= FLOOR_Y)
    {
        jumpSpeed = 0.0f;
        position.y = FLOOR_Y;
        jumpTime = maxJumpTime;
    }
    // if(lleg.intersectWith(control->ground.getModel()) == INTERSECT_ON || rleg.intersectWith(control->ground.getModel()) == INTERSECT_ON)
    // {
    // 	jumpSpeed = 0.0f;
    // 	position.y = FLOOR_Y;
    // 	jumpTime = 2;
    // }
}

void Player::updateModel()
{

    glm::mat4 basemodel(1.0f);
    basemodel = glm::translate(basemodel, position);
    basemodel = glm::rotate(basemodel, (float)glm::radians(yaw + 90), glm::vec3(0.0f, -1.0f, 0.0f));
    if (state == PLAYER_DEAD)
        basemodel = glm::rotate(basemodel, (float)glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // 这里的 rotate 输入为弧度制
    // if (state == PLAYER_RUN)
    // 	basemodel = glm::rotate(basemodel, (float)glm::radians(yaw+90), glm::vec3(0.0f, -1.0f, 0.0f));
    // else
    // 	basemodel = glm::rotate(basemodel, (float)glm::radians(lastyaw+90), glm::vec3(0.0f, -1.0f, 0.0f));

    // head: 世界坐标平移 * 局部坐标平移 * 整体模型转动 * 头俯仰角转动
    glm::mat4 model = basemodel;
    // model = glm::rotate(model, , right);
    // model = glm::rotate(model, control->camera.Pitch, glm::vec3(0.0f, 0.0f, 1.0f));
    head.setModel(model);
    head.setModel_noscale(model);

    // body: 世界坐标平移 * 局部坐标平移 * 整体模型转动
    model = basemodel;
    // model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    body.setModel(model);
    body.setModel_noscale(model);

    // larm, rarm, lleg, rleg: 世界坐标平移 * 局部坐标平移 * 整体模型转动 * 手臂/腿部转动
    model = basemodel;
    // model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, theta, glm::vec3(1.0f, 0.0f, 0.0f));
    larm.setModel(model);
    larm.setModel_noscale(model);

    model = basemodel;
    // model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, -fireTime, glm::vec3(1.0f, 0.0f, 0.0f));
    rarm.setModel(model);
    rarm.setModel_noscale(model);

    model = basemodel;
    // model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, -theta, glm::vec3(1.0f, 0.0f, 0.0f));
    lleg.setModel(model);
    lleg.setModel_noscale(model);

    model = basemodel;
    // model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, theta, glm::vec3(1.0f, 0.0f, 0.0f));
    rleg.setModel(model);
    rleg.setModel_noscale(model);
}

// 补丁: 碰撞检测时试探性地移动应改变gmodel_obb，而非lmodel，以防止模型抖动
void Player::updateModel_obb()
{
    glm::mat4 basemodel(1.0f);
    basemodel = glm::translate(basemodel, position);
    basemodel = glm::rotate(basemodel, (float)glm::radians(yaw + 90), glm::vec3(0.0f, -1.0f, 0.0f));

    // head: 世界坐标平移 * 局部坐标平移 * 整体模型转动 * 头俯仰角转动
    glm::mat4 model = basemodel;
    head.setLModelObb(model);

    // body: 世界坐标平移 * 局部坐标平移 * 整体模型转动
    model = basemodel;
    body.setLModelObb(model);

    // larm, rarm, lleg, rleg: 世界坐标平移 * 局部坐标平移 * 整体模型转动 * 手臂/腿部转动
    model = basemodel;
    model = glm::rotate(model, theta, glm::vec3(1.0f, 0.0f, 0.0f));
    larm.setLModelObb(model);

    model = basemodel;
    model = glm::rotate(model, -fireTime, glm::vec3(1.0f, 0.0f, 0.0f));
    rarm.setLModelObb(model);

    model = basemodel;
    model = glm::rotate(model, -theta, glm::vec3(1.0f, 0.0f, 0.0f));
    lleg.setLModelObb(model);

    model = basemodel;
    model = glm::rotate(model, theta, glm::vec3(1.0f, 0.0f, 0.0f));
    rleg.setLModelObb(model);
}

void Player::fire()
{
    if (arrowMgr->fire(PLAYER_ID))
        fireTime = 1.0f;
}

void Player::getCandy(CandyType type)
{
    exp += candyExp;
    if (exp >= expToLevelUp[level])
    {
        exp -= expToLevelUp[level];
        level++;
        maxHp += 5;
        hp = maxHp;
        std::cout << "level up: " << level << std::endl;
    }
}

void Player::getHit(const Arrow &arrow)
{
    std::cout << "id: " << id << " hp: " << hp << std::endl;
    hp -= arrow.speed * arrow.damage;
    if (hp <= 0)
    {
        hp = 0;
        state = PLAYER_DEAD;
    }
    std::cout << "id: " << id << " hp: " << hp << std::endl;
}

PlayerSyncPackage::PlayerSyncPackage(Player *player)
{
    // position, front, right, up, yaw, pitch, hp, level, exp, id
    type = Sync_Player;
    timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    size = sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int) * 4;
    data = new char[size];
    memset(data, 0, size);
    memcpy(data, &player->position, sizeof(glm::vec3));
    memcpy(data + sizeof(glm::vec3), &player->front, sizeof(glm::vec3));
    memcpy(data + sizeof(glm::vec3) * 2, &player->right, sizeof(glm::vec3));
    memcpy(data + sizeof(glm::vec3) * 3, &player->up, sizeof(glm::vec3));
    memcpy(data + sizeof(glm::vec3) * 4, &player->yaw, sizeof(float));
    memcpy(data + sizeof(glm::vec3) * 4 + sizeof(float), &player->pitch, sizeof(float));
    memcpy(data + sizeof(glm::vec3) * 4 + sizeof(float) * 2, &player->hp, sizeof(int));
    memcpy(data + sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int), &player->level, sizeof(int));
    memcpy(data + sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int) * 2, &player->exp, sizeof(int));
    memcpy(data + sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int) * 3, &player->id, sizeof(int));
    // std::cout<<"pack Id: "<<*(int *)(data + sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int) * 3)<<std::endl;
}

void PlayerSyncPackage::update(Player *player)
{
    // int id = player->id;
    int packageId = *(int *)(data + sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int) * 3);
    // std::cout<<"update Id: "<<packageId<<std::endl;
    // if (id != packageId)
    //     return;
    memcpy(&player->position, data, sizeof(glm::vec3));
    memcpy(&player->front, data + sizeof(glm::vec3), sizeof(glm::vec3));
    memcpy(&player->right, data + sizeof(glm::vec3) * 2, sizeof(glm::vec3));
    memcpy(&player->up, data + sizeof(glm::vec3) * 3, sizeof(glm::vec3));
    memcpy(&player->yaw, data + sizeof(glm::vec3) * 4, sizeof(float));
    memcpy(&player->pitch, data + sizeof(glm::vec3) * 4 + sizeof(float), sizeof(float));
    memcpy(&player->hp, data + sizeof(glm::vec3) * 4 + sizeof(float) * 2, sizeof(int));
    memcpy(&player->level, data + sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int), sizeof(int));
    memcpy(&player->exp, data + sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int) * 2, sizeof(int));
    memcpy(&player->id, data + sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int) * 3, sizeof(int));
}

int PlayerSyncPackage::getId()
{
    return *(int *)(data + sizeof(glm::vec3) * 4 + sizeof(float) * 2 + sizeof(int) * 3);
}