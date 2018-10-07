#include "bitmap.h"
#include "constants.h"
#include "utils.h"

#include <cstring>
#include <cstdio>
#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <Iex.h>
#include <vector>
#include "bitmap_image.hpp"


Bitmap::Bitmap()
: m_Width(0),
  m_Height(0),
  m_Data(nullptr)
{}

Bitmap::~Bitmap()
{
    freeMem();
}

void Bitmap::freeMem()
{
    if ( m_Data )
        delete[] m_Data;

    m_Data = nullptr;
    m_Width = m_Height = 0;
}

void Bitmap::GenerateEmptyImage(unsigned width, unsigned height)
{
    freeMem();

    m_Width = width;
    m_Height = height;
    m_Data = new Color[width * height];
    memset(m_Data, 0, sizeof(m_Data[0] * width * height));
}

Color Bitmap::GetPixel(unsigned int x, unsigned int y) const
{
    if ( !m_Data || x >= m_Width || y >= m_Height)
        return {0, 0, 0};

    return m_Data[x + y*m_Width];
}

void Bitmap::SetPixel(unsigned x, unsigned y, const Color& color)
{
    if ( !m_Data || x >= m_Width || y >= m_Height)
        return;

    m_Data[x + y*m_Width] = color;
}

class ImageOpenRAII
{
public:
    ImageOpenRAII(Bitmap* bitmap)
    : m_ImageIsOK(false),
      m_BMP(bitmap)
    {}

    ~ImageOpenRAII()
    {
        if ( !m_ImageIsOK )
            m_BMP->freeMem();
    }

    void SetImageIsOK(bool ImageIsOK) { m_ImageIsOK = ImageIsOK; }

private:
    bool m_ImageIsOK;
    Bitmap* m_BMP;
};

bool Bitmap::LoadBMP(const char* filename)
{
    freeMem();
    ImageOpenRAII helper(this);

    bitmap_image image(filename);
    if (!image)
    {
        printf("loadBMP: Can't open file: `%s'\n", filename);
        return false;
    }

    const unsigned int height = image.height();
    const unsigned int width  = image.width();
    GenerateEmptyImage(width, height);
    if (!IsOK())
    {
        printf("loadBMP: cannot allocate memory for bitmap! Check file integrity!\n");
        return false;
    }

    for (unsigned y = 0; y < height; ++y)
        for (unsigned x = 0; x < width; ++x)
        {
            rgb_t colour;
            image.get_pixel(x, y, colour);
            SetPixel(x, y, {colour.red/255.f, colour.green/255.f, colour.blue/255.f});
        }

    helper.SetImageIsOK(true);
    return true;
}

bool Bitmap::SaveBMP(const char* filename)
{
    bitmap_image image(filename);
    if (!image)
        return false;

    for (unsigned y = 0; y < m_Height; ++y)
        for (unsigned x = 0; x < m_Width; ++x)
        {
            unsigned t = GetPixel(x, y).toRGB32();
            image.set_pixel(x, y, static_cast<const unsigned char>(0xff     & t)
                                , static_cast<const unsigned char>(0x00ff   & t)
                                , static_cast<const unsigned char>(0x0000ff & t));
        }

    return true;
}

bool Bitmap::LoadEXR(const char* filename)
{
    try
    {
        Imf::RgbaInputFile exr(filename);
        Imf::Array2D<Imf::Rgba> pixels;
        Imath::Box2i dw = exr.dataWindow();
        m_Width  = dw.max.x - dw.min.x + 1;
        m_Height = dw.max.y - dw.min.y + 1;
        pixels.resizeErase(m_Height, m_Width);
        exr.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * m_Width, 1, m_Width);
        exr.readPixels(dw.min.y, dw.max.y);
        m_Data = new Color[m_Width * m_Height];
        for (unsigned y = 0; y < m_Height; y++)
            for (unsigned x = 0; x < m_Width; x++)
            {
                Color& pixel = m_Data[y * m_Width + x];
                pixel.r = pixels[y + dw.min.y][x + dw.min.x].r;
                pixel.g = pixels[y + dw.min.y][x + dw.min.x].g;
                pixel.b = pixels[y + dw.min.y][x + dw.min.x].b;
            }

        return true;
    }
    catch (Iex::BaseExc ex)
    {
        m_Width = m_Height = 0;
        m_Data = NULL;
        return false;
    }
}

bool Bitmap::SaveEXR(const char* filename)
{
    try
    {
        Imf::RgbaOutputFile file(filename, m_Width, m_Height, Imf::WRITE_RGBA);
        std::vector<Imf::Rgba> temp(m_Width * m_Height);
        for (unsigned i = 0; i < m_Width * m_Height; i++)
        {
            temp[i].r = m_Data[i].r;
            temp[i].g = m_Data[i].g;
            temp[i].b = m_Data[i].b;
            temp[i].a = 1.0f;
        }

        file.setFrameBuffer(&temp[0], 1, m_Width);
        file.writePixels(m_Height);
    }
    catch (Iex::BaseExc ex)
    {
        return false;
    }

    return true;
}

bool Bitmap::LoadImage(const char* filename)
{
    if ( ExtensionUpper(filename) == "BMP") return LoadBMP(filename);
    if ( ExtensionUpper(filename) == "EXR") return LoadEXR(filename);
    return false;
}

bool Bitmap::SaveImage(const char* filename)
{
    if ( ExtensionUpper(filename) == "BMP") return SaveBMP(filename);
    if ( ExtensionUpper(filename) == "EXR") return SaveEXR(filename);
    return false;
}
