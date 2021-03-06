#ifndef RAYTRACING_ENVIRONMENT_H
#define RAYTRACING_ENVIRONMENT_H

#include "color.h"
#include "vector.h"
#include "scene.h"

enum class CubeOrder
{
    NEGX,
    NEGY,
    NEGZ,
    POSX,
    POSY,
    POSZ,
};

class Environment : public SceneElement
{
public:
    virtual ~Environment() = default;

    /// gets a color from the environment at the specified direction
    virtual Color GetEnvironment(const Vector& dir) const = 0;

    virtual ElementType GetElementType() const override { return ElementType::ENVIRONMENT; }
};

class Bitmap;
class CubemapEnvironment : public Environment
{
public:
    /// loads a cubemap from 6 separate images, from the specified folder.
    /// The images have to be named "posx.bmp", "negx.bmp", "posy.bmp", ...
    /// (or they may be .exr images, not .bmp).
    /// The folder specification shouldn't include a trailing slash;
    /// e.g. "/images/cubemaps/cathedral" is OK.
    virtual ~CubemapEnvironment() override;

    virtual Color GetEnvironment(const Vector& dir) const override;
    virtual void FillProperties(ParsedBlock& pb) override;

private:
    Bitmap* m_Maps[6] = {nullptr};
    bool m_UseBilinearFiltering = false;

    Color GetSide(const Bitmap& bmp, double x, double y) const;
    bool LoadMaps(const char* folder);
    void UnloadMaps();
};

#endif //RAYTRACING_ENVIRONMENT_H
