#ifndef RAYTRACING_CAMERA_H
#define RAYTRACING_CAMERA_H

#include "matrix.h"
#include "ray.h"
#include "vector.h"

class Camera
{
public:
    const Vector& GetPosition() const { return m_Position; }
    void SetPosition(const Vector& position) { m_Position = position; }
    float GetYaw() const { return m_Yaw; }
    void SetYaw(float Yaw) { m_Yaw = Yaw; }
    float GetPitch() const { return m_Pitch; }
    void SetPitch(float Pitch) { m_Pitch = Pitch; }
    float GetRoll() const { return m_Roll; }
    void SetRoll(float Roll) { m_Roll = Roll; }
    float GetAspectRatio() const { return m_AspectRatio; }
    void SetAspectRatio(float AspectRatio) { m_AspectRatio = AspectRatio; }
    float GetFOV() const { return m_FOV; }
    void SetFOV(float FOV) { m_FOV = FOV; }

    void FrameBegin();
    Ray GetScreenRay(float x, float y) const;

private:
    Vector m_TopLeft;
    Vector m_TopRight;
    Vector m_BottomLeft;

    Vector m_Position;
    Matrix m_Rotation;

    float m_Yaw;
    float m_Pitch;
    float m_Roll;

    float m_AspectRatio;
    float m_FOV;
};

#endif //RAYTRACING_CAMERA_H
