#ifndef RAYTRACING_SHADINGHELPER_H
#define RAYTRACING_SHADINGHELPER_H

class IntersectionInfo;
class Vector;

class ShadingHelper
{
public:
    static double GetLightContribution(const IntersectionInfo& info);

private:
    static bool CheckVisibility(const Vector& start, const Vector& end);
};

#endif //RAYTRACING_SHADINGHELPER_H
