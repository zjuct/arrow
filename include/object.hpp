#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include "glm/gtc/matrix_transform.hpp"
#include "shape.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <material.h>
#include <iostream>

#include <obb.h>

enum ObjectType {
    OBJECT_NONE = 0,
    OBJECT_MESH,
    OBJECT_BOX,
    OBJECT_BEZIER_CURVE,
    OBJECT_BEZIER_SURFACE,
    OBJECT_CIRCLE,
    OBJECT_SPHERE,
    OBJECT_BSPLINE_CURVE,
    OBJECT_BSPLINE_SURFACE,
    OBJECT_LIGHT,
};

// 质心固定为(0, 0, 0)
class Object {
public:
    Object(ObjectType _type, Shape* _shape, Shader* _shader, glm::mat4 _model = glm::mat4(1.0f), Object* _parent = nullptr, bool _show = true)
        : type(_type), shape(_shape), shader(_shader), lmodel(_model), parent(_parent), show(_show), obb(nullptr) {
        lmodel_noscale = lmodel;
        if(parent)
            parent->addChild(this);
        updateModel();

        if(shape) {
            obb = shape->getObb();
            if(obb) {
                obb->init();
                obb->object = this;
            }
        }
    }

    Object(const Object& o) {
        type = o.type;
        shape = o.shape;
        shader = o.shader;
        lmodel = o.lmodel;
        lmodel_noscale = o.lmodel_noscale;
        parent = o.parent;
        show = o.show;
        if(o.obb) {
            obb = new Obb(*(o.obb));
            obb->object = this;
        } else {
            obb = nullptr;
        }
        for(Object* c : o.children) {
            addChild(new Object(*c));
        }
        updateModel();
    }

    Object& operator= (const Object& o) {
        if(this == &o) return *this;
        type = o.type;
        shape = o.shape;
        shader = o.shader;
        lmodel = o.lmodel;
        lmodel_noscale = o.lmodel_noscale;
        parent = o.parent;
        show = o.show;
        if(o.obb) {
            obb = new Obb(*(o.obb));
            obb->object = this;
        } else {
            obb = nullptr;
        }
        children.clear();
        for(Object* c : o.children) {
            addChild(new Object(*c));
        }
        updateModel();
        return *this;
    }

    ~Object() {
      if(obb)
            delete obb;
        for(Object* c : children) {
            delete c;
        }
    }

    ObjectType getType() {
        return type;
    }

    void setColor(const glm::vec3& c) {
        color = c;
    }

    void addChild(Object* o) {
        if (o) {
            children.push_back(o);
            o->parent = this;
        }
    }

    void setModel(const glm::mat4& model) {
        lmodel = model;
        updateModel();
    }

    void setModel_noscale(const glm::mat4& model) {
        lmodel_noscale = model;
        updateModel();
    }

    void mulleft(const glm::mat4& m) {
        lmodel = m * lmodel;
        updateModel();
    }

    void mulright(const glm::mat4& m) {
        lmodel = lmodel * m;
        updateModel();
    }

    virtual void draw() {
        if(show) {
            shader->use();
            shader->setmat4fv("model", GL_FALSE, glm::value_ptr(gmodel));
            material.configShader(shader);
//            shader->setvec3fv("color", glm::value_ptr(color));
            if(obb) {
                obb->drawLine(segment_shader);
            }
            if(shape)
                shape->draw(shader);
        }
        for(Object* c: children) {
            c->updateModel();
            c->draw();
        }
    }

    const glm::mat4& getGModel() const  {
        return gmodel;
    }

    const glm::mat4& getGModelNoscale() const {
        return gmodel_noscale;
    }

    virtual void update(float dt) {}

    void updateModel() {
        if(parent) {
            /*
                这里有一个问题: parent缩放会带着所有children一起缩放
                两个解决方法: 1. 放着不管 2. 将缩放直接放到Shape里 3. 额外存一个不含缩放的model矩阵
                方法2违背了将Shpae和model分离，以实现Shape复用的初衷
            */
            // 采用方案3
            gmodel = parent->gmodel_noscale * lmodel;
            gmodel_noscale = parent->gmodel_noscale * lmodel_noscale;
        } else {
            gmodel = lmodel;
            gmodel_noscale = lmodel_noscale;
        }
        for(Object* c : children) {
            c->updateModel();
        }
    }

    Object()
        : type(OBJECT_NONE), shape(nullptr), lmodel_noscale(glm::mat4(1.0f)),
        lmodel(glm::mat4(1.0f)), parent(nullptr), shader(nullptr), obb(nullptr) {
        updateModel();
    }

    Obb* getObb() const{
        return obb;
    }

    const std::vector<Object*>& getChildren() const{
        return children;
    }

    bool intersectWith(const Object &other) const 
    {
        if(obb)
        {
            if(other.obb)
            {
                return obb->intersectWith(*(other.obb));
            }
            else
            {
                for(Object* c : other.children)
                {
                    if(intersectWith(*c))
                        return true;
                }
                return false;
            }
        }
        else
        {
            for(Object* c : children)
            {
                if(c->intersectWith(other))
                    return true;
            }
            return false;
        }
    }

protected:
    ObjectType type;
    Shape* shape;
    glm::mat4 gmodel;       // global
    glm::mat4 lmodel;       // local
    glm::mat4 gmodel_noscale;
    glm::mat4 lmodel_noscale;

    Object* parent;
    std::vector<Object*> children;

    Shader* shader;
    glm::vec3 color;        // used for single color Object
    bool show;

    Obb* obb;

public:
    material_t material;
};

#endif