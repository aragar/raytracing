#include "bitmap.h"
#include "texture.h"

Color CheckerTexture::Sample(const IntersectionInfo& info) const
{
    int x = (int)floor(info.u*m_Scaling / 5.);
    int y = (int)floor(info.v*m_Scaling / 5.);

    Color checkerColor = ( (x + y) % 2 == 0 ) ? m_Color1 : m_Color2;
    return checkerColor;
}

CheckerTexture::CheckerTexture(const Color& color1, const Color& color2, double scaling)
: m_Scaling(scaling),
  m_Color1(color1),
  m_Color2(color2)
{
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

MandelbrotTexture::MandelbrotTexture(double scaling, const Color& color1, const Color& color2)
: m_Scaling(scaling),
  m_Color1(color1),
  m_Color2(color2)
{
}

ProceduralTexture::ProceduralTexture()
{
    for ( unsigned i = 0; i < NUM; ++i )
    {
        m_ColorU[i] = {(float)Random(), (float)Random(), (float)Random()};
        m_FreqU[i] = Random(0.01, 0.25);

        m_ColorV[i] = {(float)Random(), (float)Random(), (float)Random()};
        m_FreqV[i] = Random(0.01, 0.25);
    }
}

Color ProceduralTexture::Sample(const IntersectionInfo& info) const
{
    Color result = {0, 0, 0};
    for ( unsigned i = 0; i < NUM; ++i )
        result += (m_ColorU[i]*sin(info.u*m_FreqU[i]) + m_ColorV[i]*sin(info.v*m_FreqV[i]));

    return result;
}

ConstantColorTexture::ConstantColorTexture(const Color& color)
: m_Color(color)
{
}

BitmapTexture::BitmapTexture(const char* filename, double scaling)
: m_Scaling(1./scaling)
{
    m_Bitmap = new Bitmap;
    m_Bitmap->LoadImage(filename);
}

BitmapTexture::~BitmapTexture()
{
    delete m_Bitmap;
}

Color BitmapTexture::Sample(const IntersectionInfo& info) const
{
    int x = (int) floor(info.u * m_Scaling * m_Bitmap->GetWidth());
    int y = (int) floor(info.v * m_Scaling * m_Bitmap->GetHeight());

    // 0 <= x < bitmap.width
    // 0 <= y < bitmap.height
    x %= m_Bitmap->GetWidth();
    if (x < 0)
        x += m_Bitmap->GetWidth();

    y %= m_Bitmap->GetHeight();
    if (y < 0)
        y += m_Bitmap->GetHeight();

    const Color& color = m_Bitmap->GetPixel(x, y);
    return color;
}

Fresnel::Fresnel(double inOutRatio)
: m_InOutRatio(inOutRatio)
{
}

Color Fresnel::Sample(const IntersectionInfo& info) const
{
    double fresnel = 0.;
    if (info.normal * info.rayDir > 0)
        fresnel = FresnelRatio(info.rayDir, -info.normal, 1. / m_InOutRatio);
    else
        fresnel = FresnelRatio(info.rayDir, info.normal, m_InOutRatio);

    float fresnelFloat = static_cast<float>(fresnel);
    Color result{fresnelFloat, fresnelFloat, fresnelFloat};
    return result;
}
