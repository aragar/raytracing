#include "texture.h"
#include "utils.h"

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

Color MandelbrotTexture::Sample(const IntersectionInfo& info) const
{
    double x0 = info.u / m_Scaling;
    double y0 = info.v / m_Scaling;

    double x = 0;
    double y = 0;
    unsigned  i;
    const unsigned MAX_ITERATIONS = 128;
    for ( i = 0; i < MAX_ITERATIONS && Sqr(x) + Sqr(y) < 4; ++i )
    {
        double temp = Sqr(x) - Sqr(y) + x0;
        y = 2*x*y + y0;
        x = temp;
    }

    Color result;
    if ( i == 0 )
        result = m_Color1;
    else if ( i < 16 )
        result = 0.75*m_Color1 + 0.25*m_Color2;
    else if ( i < 32 )
        result = 0.5*m_Color1 + 0.5*m_Color2;
    else if ( i < 64 )
        result = 0.25*m_Color1 + 0.75*m_Color2;
    else
        result = m_Color2;

    return result;
}
