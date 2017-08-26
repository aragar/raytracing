#include "camera.h"

#include "sdl.h"
#include "utils.h"

void Camera::FrameBegin()
{
    float wantedAngle = ToRadians(m_FOV/2);
    float wantedLength = tan(wantedAngle);
    float actualLength = sqrt(Sqr(m_AspectRatio) + 1.f);
    float scaleFactor = wantedLength / actualLength;

    float x2d = -m_AspectRatio*scaleFactor*1.5f;
    float y2d = scaleFactor*1.5f;

    m_Rotation =
            RotationAroundZ(ToRadians(m_Roll)) *
            RotationAroundX(ToRadians(m_Pitch)) *
            RotationAroundY(ToRadians(m_Yaw));

    m_TopLeft = Vector{x2d, y2d, 1} * m_Rotation + m_Position;
    m_TopRight = Vector{-x2d, y2d, 1} * m_Rotation + m_Position;
    m_BottomLeft = Vector{x2d, -y2d, 1} * m_Rotation + m_Position;
}

Ray Camera::GetScreenRay(float x, float y) const
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
