#ifndef RAYTRACING_CAMERA_H
#define RAYTRACING_CAMERA_H

#include "matrix.h"
#include "ray.h"
#include "vector.h"
#include "scene.h"

class Camera : public SceneElement
{
public:
    void SetPosition(const Vector& position) { m_Position = position; }
    void SetYaw(double Yaw) { m_Yaw = Yaw; }
    void SetPitch(double Pitch) { m_Pitch = Pitch; }
    void SetRoll(double Roll) { m_Roll = Roll; }
    void SetAspectRatio(double AspectRatio) { m_AspectRatio = AspectRatio; }
    void SetFOV(double FOV) { m_FOV = FOV; }

    virtual ElementType GetElementType() const override { return ElementType::CAMERA; }
    virtual void BeginFrame() override;

    virtual void FillProperties(ParsedBlock& pb) override;

    Ray GetScreenRay(double x, double y) const;

private:
    Vector m_TopLeft;
    Vector m_TopRight;
    Vector m_BottomLeft;

    Vector m_Position;
    Matrix m_Rotation;

    double m_Yaw = 0.;
    double m_Pitch = 0.;
    double m_Roll = 0.;

    double m_AspectRatio = 4./3.;
    double m_FOV = 90.;
};

#endif //RAYTRACING_CAMERA_H
