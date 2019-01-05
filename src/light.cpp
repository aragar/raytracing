#include "light.h"

void Light::FillProperties(ParsedBlock& pb)
{
    pb.GetVectorProp("pos", &pos);
    pb.GetDoubleProp("intensity", &intensity);
}