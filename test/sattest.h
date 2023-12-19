#ifndef _SATTEST_H
#define _SATTEST_H

#include <obb.h>

class SatTest {
public:
    SatTest();
    void init() {
        box1.init();
        box2.init();
    }

    void updateKeyBoard(int key);

    Obb box1, box2;

    void draw(Shader* shader);
    void checkCollision();

    bool control_box1 = true;
};

#endif