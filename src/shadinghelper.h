#ifndef RAYTRACING_SHADINGHELPER_H
#define RAYTRACING_SHADINGHELPER_H

class IntersectionInfo;
class Vector;
class Light;

class ShadingHelper
{
public:
    static double GetLightContribution(const IntersectionInfo& info, const Light& light);

private:
    static float GetShadowTransparency(const Vector& start, const Vector& end);
};

#endif //RAYTRACING_SHADINGHELPER_H
