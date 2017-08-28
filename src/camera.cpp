#include "camera.h"

#include "sdl.h"
#include "utils.h"

void Camera::FrameBegin()
{
    double wantedAngle = ToRadians(m_FOV/2.);
    double wantedLength = tan(wantedAngle);
    double actualLength = sqrt(Sqr(m_AspectRatio) + 1.);
    double scaleFactor = wantedLength / actualLength;

    double x2d = -m_AspectRatio*scaleFactor*1.5;
    double y2d = scaleFactor*1.5;

    m_Rotation =
            RotationAroundZ(ToRadians(m_Roll)) *
            RotationAroundX(ToRadians(m_Pitch)) *
            RotationAroundY(ToRadians(m_Yaw));

    m_TopLeft = Vector{x2d, y2d, 1.} * m_Rotation + m_Position;
    m_TopRight = Vector{-x2d, y2d, 1.} * m_Rotation + m_Position;
    m_BottomLeft = Vector{x2d, -y2d, 1.} * m_Rotation + m_Position;
}

Ray Camera::GetScreenRay(double x, double y) const
{
    const int frameWidth = GetFrameWidth();
    const int frameHeight = GetFrameHeight();
    const Vector throughPoint =
            m_TopLeft + (m_TopRight - m_TopLeft)*(x / frameWidth) +
                        (m_BottomLeft - m_TopLeft)*(y / frameHeight);

    Ray ray;
    ray.start = m_Position;

    ray.dir = throughPoint - m_Position;
    ray.dir.Normalize();

    return ray;
}
