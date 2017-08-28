#ifndef RAYTRACING_CAMERA_H
#define RAYTRACING_CAMERA_H

#include "matrix.h"
#include "ray.h"
#include "vector.h"

class Camera
{
public:
    void SetPosition(const Vector& position) { m_Position = position; }
    void SetYaw(double Yaw) { m_Yaw = Yaw; }
    void SetPitch(double Pitch) { m_Pitch = Pitch; }
    void SetRoll(double Roll) { m_Roll = Roll; }
    void SetAspectRatio(double AspectRatio) { m_AspectRatio = AspectRatio; }
    void SetFOV(double FOV) { m_FOV = FOV; }

    void FrameBegin();
    Ray GetScreenRay(double x, double y) const;

private:
    Vector m_TopLeft;
    Vector m_TopRight;
    Vector m_BottomLeft;

    Vector m_Position;
    Matrix m_Rotation;

    double m_Yaw;
    double m_Pitch;
    double m_Roll;

    double m_AspectRatio;
    double m_FOV;
};

#endif //RAYTRACING_CAMERA_H
