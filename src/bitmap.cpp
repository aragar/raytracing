#include "bitmap.h"

#include "constants.h"
#include "utils.h"
#include "bitmap_image.hpp"

#include <cstring>
#include <cstdio>
#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <Iex.h>
#include <vector>

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
    : m_BMP(bitmap)
    {}

    ~ImageOpenRAII()
    {
        if (!m_ImageIsOK) m_BMP->freeMem();
        if (m_FP) fclose(m_FP); m_FP = nullptr;
    }

    void SetImageIsOK(bool ImageIsOK) { m_ImageIsOK = ImageIsOK; }
    void SetFile(FILE* fp) { m_FP = fp; }

private:
    bool m_ImageIsOK = false;
    Bitmap* m_BMP;
    FILE* m_FP = nullptr;
};

const int BM_MAGIC = 19778;

struct BmpHeader {
    int fs;       // filesize
    int lzero;
    int bfImgOffset;  // basic header size
};
struct BmpInfoHeader {
    int ihdrsize; 	// info header size
    int x,y;      	// image dimensions
    unsigned short channels;// number of planes
    unsigned short bitsperpixel;
    int compression; // 0 = no compression
    int biSizeImage; // used for compression; otherwise 0
    int pixPerMeterX, pixPerMeterY; // dots per meter
    int colors;	 // number of used colors. If all specified by the bitsize are used, then it should be 0
    int colorsImportant; // number of "important" colors (wtf?..)
};

bool Bitmap::LoadBMP(const char* filename)
{
    freeMem();
    ImageOpenRAII helper(this);

    BmpHeader hd;
    BmpInfoHeader hi;
    Color palette[256];
    int toread = 0;
    unsigned char *xx;
    int rowsz;
    unsigned short sign;
    FILE* fp = fopen(filename, "rb");

    if (fp == NULL) {
        printf("loadBMP: Can't open file: `%s'\n", filename);
        return false;
    }
    helper.SetFile(fp);
    if (!fread(&sign, 2, 1, fp)) return false;
    if (sign != BM_MAGIC) {
        printf("loadBMP: `%s' is not a BMP file.\n", filename);
        return false;
    }
    if (!fread(&hd, sizeof(hd), 1, fp)) return false;
    if (!fread(&hi, sizeof(hi), 1, fp)) return false;

    /* header correctness checks */
    if (!(hi.bitsperpixel == 8 || hi.bitsperpixel == 24 ||  hi.bitsperpixel == 32)) {
        printf("loadBMP: Cannot handle file format at %d bpp.\n", hi.bitsperpixel);
        return false;
    }
    if (hi.channels != 1) {
        printf("loadBMP: cannot load multichannel .bmp!\n");
        return false;
    }
    /* ****** header is OK *******/

    // if image is 8 bits per pixel or less (indexed mode), read some pallete data
    if (hi.bitsperpixel <= 8) {
        toread = (1 << hi.bitsperpixel);
        if (hi.colors) toread = hi.colors;
        for (int i = 0; i < toread; i++) {
            unsigned temp;
            if (!fread(&temp, 1, 4, fp)) return false;
            palette[i] = Color(temp);
        }
    }
    toread = hd.bfImgOffset - (54 + toread*4);
    fseek(fp, toread, SEEK_CUR); // skip the rest of the header
    int k = hi.bitsperpixel / 8;
    rowsz = hi.x * k;
    if (rowsz % 4 != 0)
        rowsz = (rowsz / 4 + 1) * 4; // round the row size to the next exact multiple of 4
    xx = new unsigned char[rowsz];
    GenerateEmptyImage(hi.x, hi.y);
    if (!IsOK()) {
        printf("loadBMP: cannot allocate memory for bitmap! Check file integrity!\n");
        delete [] xx;
        return false;
    }
    for (int j = hi.y - 1; j >= 0; j--) {// bitmaps are saved in inverted y
        if (!fread(xx, 1, rowsz, fp)) {
            printf("loadBMP: short read while opening `%s', file is probably incomplete!\n", filename);
            delete [] xx;
            return 0;
        }
        for (int i = 0; i < hi.x; i++){ // actually read the pixels
            if (hi.bitsperpixel > 8)
                SetPixel(i, j, Color(xx[i*k+2]/255.0f, xx[i*k+1]/255.0f, xx[i*k]/255.0f));
            else
                SetPixel(i, j,  palette[xx[i*k]]);
        }
    }
    delete [] xx;

    helper.SetImageIsOK(true);
    return true;
}

bool Bitmap::SaveBMP(const char* filename)
{
    FILE* fp = fopen(filename, "wb");
    if (!fp) return false;
    BmpHeader hd;
    BmpInfoHeader hi;
    char xx[VFB_MAX_SIZE * 3];


    // fill in the header:
    int rowsz = m_Width * 3;
    if (rowsz % 4)
        rowsz += 4 - (rowsz % 4); // each row in of the image should be filled with zeroes to the next multiple-of-four boundary
    hd.fs = rowsz * m_Height + 54; //std image size
    hd.lzero = 0;
    hd.bfImgOffset = 54;
    hi.ihdrsize = 40;
    hi.x = m_Width; hi.y = m_Height;
    hi.channels = 1;
    hi.bitsperpixel = 24; //RGB format
    // set the rest of the header to default values:
    hi.compression = hi.biSizeImage = 0;
    hi.pixPerMeterX = hi.pixPerMeterY = 0;
    hi.colors = hi.colorsImportant = 0;

    fwrite(&BM_MAGIC, 2, 1, fp); // write 'BM'
    fwrite(&hd, sizeof(hd), 1, fp); // write file header
    fwrite(&hi, sizeof(hi), 1, fp); // write image header
    for ( unsigned y = m_Height; y > 0; y--) {
        for (unsigned x = 0; x < m_Width; x++) {
            unsigned t = GetPixel(x, y - 1).toRGB32();
            xx[x * 3    ] = (0xff     & t);
            xx[x * 3 + 1] = (0xff00   & t) >> 8;
            xx[x * 3 + 2] = (0xff0000 & t) >> 16;
        }
        fwrite(xx, rowsz, 1, fp);
    }
    fclose(fp);
    return true;
}

bool Bitmap::LoadEXR(const char* filename)
{
    try
    {
        Imf::RgbaInputFile exr(filename);
        Imf::Array2D<Imf::Rgba> pixels;
        Imath::Box2i dw = exr.dataWindow();
        m_Width  = static_cast<unsigned>(dw.max.x - dw.min.x + 1);
        m_Height = static_cast<unsigned>(dw.max.y - dw.min.y + 1);
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

Color Bitmap::GetBilinearFilteredPixel(double x, double y) const
{
    const unsigned xx = static_cast<unsigned>(floor(x));
    const unsigned yy = static_cast<unsigned>(floor(y));
    const double p = x - xx;
    const double q = y - yy;

    const unsigned width = GetWidth();
    const unsigned height = GetHeight();

    const Color center    = GetPixel(xx, yy);
    const Color right     = GetPixel((xx + 1) % width, yy);
    const Color down      = GetPixel(xx, (yy + 1) % height);
    const Color downRight = GetPixel((xx + 1) % width, (yy + 1) % height);

    const Color result = center*(1 - p)*(1 - q) + right*p*(1 - q) + down*(1 - p)*q + downRight*p*q;
    return result;
}

void Bitmap::Differentiate()
{
    Bitmap bumpTexture;
    bumpTexture.GenerateEmptyImage(m_Width, m_Height);

    for (unsigned y = 0; y < m_Height; ++y)
        for (unsigned x = 0; x < m_Width; ++x)
        {
            const float dx = GetPixel(x, y).Intensity() - GetPixel((x + 1) % m_Width, y).Intensity();
            const float dy = GetPixel(x, y).Intensity() - GetPixel(x, (y + 1) % m_Height).Intensity();
            bumpTexture.SetPixel(x, y, Color(dx, dy, 0));
        }

    std::swap(m_Data, bumpTexture.m_Data);
}
