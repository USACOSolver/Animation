#ifndef _FIELD_H_
#define _FIELD_H_

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Field
{
public:
    Field() {}
    virtual ~Field() {}
};
class ScalarField : public Field
{
public:
    ScalarField() {}
    virtual ~ScalarField() {}
    virtual double sample(const glm::vec3 x) const = 0;
};
class VectorField : public Field
{
public:
    VectorField() {}
    virtual ~VectorField() {}
    virtual glm::vec3 sample(const glm::vec3 &x) const = 0;
};

class ConstantVectorField : public VectorField
{
public:
    ConstantVectorField(const glm::vec3 &v) : value(v)
    {
    }
    glm::vec3 sample(const glm::vec3 &x) const override
    {
        return value;
    }

private:
    glm::vec3 value;
};
#endif