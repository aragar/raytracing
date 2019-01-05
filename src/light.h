#ifndef RAYTRACING_LIGHT_H
#define RAYTRACING_LIGHT_H

#include "scene.h"

struct Light : public SceneElement
{
    Vector pos;
    double intensity;

    virtual ElementType GetElementType() const override { return ElementType::LIGHT; }
    virtual void FillProperties(ParsedBlock& pb) override;
};

#endif //RAYTRACING_LIGHT_H
