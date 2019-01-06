#include "texture.h"

#include "bitmap.h"

#include <cstring>

Color CheckerTexture::Sample(const IntersectionInfo& info) const
{
    int x = (int)floor(info.u*m_Scaling / 5.);
    int y = (int)floor(info.v*m_Scaling / 5.);

    Color checkerColor = ( (x + y) % 2 == 0 ) ? m_Color1 : m_Color2;
    return checkerColor;
}

void CheckerTexture::FillProperties(ParsedBlock& pb)
{
    pb.GetColorProp("color1", &m_Color1);
    pb.GetColorProp("color2", &m_Color2);
    pb.GetDoubleProp("scaling", &m_Scaling);
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

void MandelbrotTexture::FillProperties(ParsedBlock& pb)
{
    pb.GetColorProp("color1", &m_Color1);
    pb.GetColorProp("color2", &m_Color2);
    pb.GetDoubleProp("scaling", &m_Scaling);
}

MandelbrotTexture::MandelbrotTexture(double scaling, const Color& color1, const Color& color2)
: m_Scaling(scaling),
  m_Color1(color1),
  m_Color2(color2)
{
}

Color ProceduralTexture::Sample(const IntersectionInfo& info) const
{
    Color result = {0, 0, 0};
    for (const ProceduralLine& line : m_Lines)
        result += (line.colorU*sin(info.u*line.freqU) + line.colorV*sin(info.v*line.freqV));

    return result;
}

void ProceduralTexture::FillProperties(ParsedBlock& pb)
{
    char name[128];
    char value[256];
    int srcLine;
    for (int i = 0; i < pb.GetBlockLines(); i++)
    {
        // fetch and parse all lines like "element <colorU> <freqU> <colorV> <freqV>"
        pb.GetBlockLine(i, srcLine, name, value);
        if (!strcmp(name, "element"))
        {
            StripBracesAndCommas(value);
            float colorU1, colorU2, colorU3;
            double freqU;
            float colorV1, colorV2, colorV3;
            double freqV;
            if (8 != sscanf(value, "%f%f%f %lf %f%f%f %lf"
                    , &colorU1, &colorU2, &colorU3
                    , &freqU
                    , &colorV1, &colorV2, &colorV3
                    , &freqV))
            {
                throw SyntaxError(srcLine, "Expected a procedural in the format <colorU> <freqU> <colorV> <freqV>");
            }

            ProceduralLine line{Color(colorU1, colorU2, colorU3), freqU, Color(colorV1, colorV2, colorV3), freqV};
            m_Lines.push_back(line);
        }
    }
}

ConstantColorTexture::ConstantColorTexture(const Color& color)
: m_Color(color)
{
}

void ConstantColorTexture::FillProperties(ParsedBlock& pb)
{
    pb.GetColorProp("color", &m_Color);
}

BitmapHelper::BitmapHelper(const char* filename, double scaling, bool useBilinearFiltering)
: m_Scaling(1./scaling)
, m_UseBilinearFiltering(useBilinearFiltering)
{
    m_Bitmap = new Bitmap;
    if (filename)
        m_Bitmap->LoadImage(filename);
}

BitmapHelper::~BitmapHelper()
{
    delete m_Bitmap;
}

Color BitmapHelper::GetColor(double u, double v) const
{
    Color result;
    if (m_UseBilinearFiltering)
    {
        double x, y;
        GetCoords(u, v, x, y);
        result = m_Bitmap->GetBilinearFilteredPixel(x, y);
    }
    else
    {
        int x, y;
        GetCoords(u, v, x, y);
        result = m_Bitmap->GetPixel(static_cast<unsigned>(x), static_cast<unsigned>(y));
    }

    return result;
}

void BitmapHelper::GetCoords(double u, double v, int& x, int& y) const
{
    const unsigned int width = m_Bitmap->GetWidth();
    const unsigned int height = m_Bitmap->GetHeight();
    x = static_cast<int>(floor(u * m_Scaling * width));
    y = static_cast<int>(floor(v * m_Scaling * height));

    // 0 <= x < bitmap.width
    // 0 <= y < bitmap.height
    x %= width;
    if ( x < 0 )
        x += width;

    y %= height;
    if ( y < 0 )
        y += height;
}

void BitmapHelper::GetCoords(double u, double v, double& x, double& y) const
{
    const unsigned int width = m_Bitmap->GetWidth();
    const unsigned int height = m_Bitmap->GetHeight();
    x = u * m_Scaling * m_Bitmap->GetWidth();
    y = v * m_Scaling * m_Bitmap->GetHeight();
    int xx = static_cast<int>(floor(x));
    int yy = static_cast<int>(floor(y));
    const double dx = x - xx;
    const double dy = y - yy;

    // 0 <= x < bitmap.width
    // 0 <= y < bitmap.height
    xx %= width;
    if (xx < 0)
        xx += width;

    yy %= height;
    if (yy < 0)
        yy += height;

    x = static_cast<double>(xx) + dx;
    y = static_cast<double>(yy) + dy;
}

void BitmapHelper::FillProperties(ParsedBlock& pb)
{
    pb.GetDoubleProp("scaling", &m_Scaling);
    m_Scaling = 1./m_Scaling;

    pb.GetBoolProp("useBilinearFiltering", &m_UseBilinearFiltering);

    pb.RequiredProp("file");
    pb.GetBitmapFileProp("file", *m_Bitmap);
}

BitmapTexture::BitmapTexture(const char* filename, double scaling, bool useBilinearFiltering)
: m_BitmapHelper(filename, scaling, useBilinearFiltering)
{
}

Color BitmapTexture::Sample(const IntersectionInfo& info) const
{
    Color result = m_BitmapHelper.GetColor(info.u, info.v);
    return result;
}

void BitmapTexture::FillProperties(ParsedBlock& pb)
{
    m_BitmapHelper.FillProperties(pb);
}

Fresnel::Fresnel(double inOutRatio)
: m_InOutRatio(inOutRatio)
{
}

Color Fresnel::Sample(const IntersectionInfo& info) const
{
    double fresnel;
    if (info.normal * info.rayDir > 0)
        fresnel = FresnelRatio(info.rayDir, -info.normal, 1. / m_InOutRatio);
    else
        fresnel = FresnelRatio(info.rayDir, info.normal, m_InOutRatio);

    float fresnelFloat = static_cast<float>(fresnel);
    Color result{fresnelFloat, fresnelFloat, fresnelFloat};
    return result;
}

void Fresnel::FillProperties(ParsedBlock& pb)
{
    pb.GetDoubleProp("ior", &m_InOutRatio, 1e-6, 10.);
}