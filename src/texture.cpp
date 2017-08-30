#include "texture.h"

CheckerTexture::CheckerTexture()
: m_Scaling(1.)
{
    m_Color1.MakeZero();
    m_Color2.MakeZero();
}

Color CheckerTexture::Sample(const IntersectionInfo& info) const
{
    int x = (int)floor(info.u*m_Scaling / 5.);
    int y = (int)floor(info.v*m_Scaling / 5.);

    Color checkerColor = ( (x + y) % 2 == 0 ) ? m_Color1 : m_Color2;
    return checkerColor;
}

