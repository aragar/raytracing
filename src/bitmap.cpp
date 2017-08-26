#include "bitmap.h"
#include "constants.h"

#include <cstring>
#include <cstdio>

Bitmap::Bitmap()
: m_Width(-1),
  m_Height(-1),
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
    m_Width = m_Height = -1;
}

void Bitmap::GenerateEmptyImage(int width, int height)
{
    freeMem();

    if ( width <= 0 || height <= 0 )
        return;

    m_Width = width;
    m_Height = height;
    m_Data = new Color[width * height];
    memset(m_Data, 0, sizeof(m_Data[0] * width * height));
}

Color Bitmap::GetPixel(int x, int y) const
{
    if ( !m_Data
         || x < 0 || x >= m_Width
         || y < 0 || y >= m_Height)
        return {0.f, 0.f, 0.f};

    return m_Data[x + y*m_Width];
}

void Bitmap::SetPixel(int x, int y, const Color& color)
{
    if ( !m_Data
         || x < 0 || x >= m_Width
         || y < 0 || y >= m_Height)
        return;

    m_Data[x + y*m_Width] = color;
}

class ImageOpenRAII
{
public:
    ImageOpenRAII(Bitmap* bitmap)
    : m_ImageIsOK(false),
      m_FP(nullptr),
      m_BMP(bitmap)
    {}

    ~ImageOpenRAII()
    {
        if ( !m_ImageIsOK )
            m_BMP->freeMem();

        if ( m_FP )
            fclose(m_FP);

        m_FP = nullptr;
    }

    void SetImageIsOK(bool ImageIsOK) { m_ImageIsOK = ImageIsOK; }
    void SetFP(FILE* fp) { ImageOpenRAII::m_FP = fp; }

private:
    bool m_ImageIsOK;
    FILE* m_FP;

    Bitmap* m_BMP;
};

const int BM_MAGIC = 19778;

struct BmpHeader
{
    int fs;
    int lzero;
    int bfImgOffset;
};

struct BMPInfoHeader
{
    int ihdrsize;
    int x, y;
    unsigned short channels;
    unsigned short bitsperpixel;
    int compression;
    int biSizeImage;
    int pixPerMeterX, pixPerMeterY;
    int colors;
    int colorsImportant;
};

bool Bitmap::LoadBMP(const char* filename)
{
    freeMem();

    FILE* fp = fopen(filename, "rb");
    if ( !fp )
    {
        printf("LoadBMP: Can't open file: `%s`\n", filename);
        return false;
    }

    ImageOpenRAII helper(this);
    helper.SetFP(fp);

    unsigned short sign;
    if ( !fread(&sign, 2, 1, fp) )
        return false;

    if ( sign != BM_MAGIC )
    {
        printf("LoadBMP: `%s` is not a BMP file.\n", filename);
        return false;
    }

    BmpHeader hd;
    if ( !fread(&hd, sizeof(hd), 1, fp) )
        return false;

    BMPInfoHeader hi;
    if ( !fread(&hi, sizeof(hi), 1, fp) )
        return false;

    // header correctness check
    if ( hi.bitsperpixel != 8 && hi.bitsperpixel != 24 && hi.bitsperpixel != 32 )
    {
        printf("LoadBMP: cannot handle file format at %d bpp.\n", hi.bitsperpixel);
        return false;
    }

    if ( hi.channels != 1 )
    {
        printf("LoadBMP: cannot load multichannel .bmp!\n");
        return false;
    }
    // header is ok

    // if image is 8 bits per pixel or less (indexed mode), read some palette data
    int toread = 0;
    Color palette[256];
    if ( hi.bitsperpixel == 8 )
    {
        toread = (1 << hi.bitsperpixel);
        if ( hi.colors )
            toread = hi.colors;

        for ( int i = 0; i < toread; ++i )
        {
            unsigned tmp;
            if ( !fread(&tmp, 1, 4, fp) )
                return false;

            palette[i] = Color(tmp);
        }
    }

    toread = hd.bfImgOffset - (54 + toread*4);
    fseek(fp, toread, SEEK_CUR); // skip the rest of the header

    GenerateEmptyImage(hi.x, hi.y);
    if ( !IsOK() )
    {
        printf("LoadBMP: cannot allocate memory for bitmap! Check file integrity!\n");
        return false;
    }

    int k = hi.pixPerMeterX / 8;
    int rowsz = hi.x * k;
    if ( rowsz % 4 != 0 )
        rowsz = (rowsz/4 + 1)*4;

    unsigned char *xx = new unsigned char[rowsz];
    for ( int j = hi.y - 1; j >= 0; --j ) // bitmaps are saved in inverted y
    {
        if ( !fread(xx, 1, rowsz, fp) )
        {
            printf("LoadBMP: short read while opening `%s`, file is probably incomplete~\n", filename);
            delete[] xx;
            return false;
        }

        for ( int i = 0; i < hi.x; ++i )
        {
            if ( hi.bitsperpixel > 8 )
                SetPixel(i, j, {xx[i*k + 2]/255.f, xx[i*k + 1]/255.f, xx[i*k]/255.f});
            else
                SetPixel(i, j, palette[xx[i*k]]);
        }
    }

    delete[] xx;
    helper.SetImageIsOK(true);
    return true;
}

bool Bitmap::SaveBMP(const char* filename)
{
    FILE* fp = fopen(filename, "wb");
    if ( !fp )
        return false;

    int rowsz = m_Width*3;
    if ( rowsz % 4 != 0 )
        rowsz = (rowsz/4 + 1)*4;

    BmpHeader hd;
    hd.fs = rowsz*m_Height + 54; // std image size
    hd.lzero = 0;
    hd.bfImgOffset = 54;

    BMPInfoHeader hi;
    hi.ihdrsize = 40;
    hi.x = m_Width;
    hi.y = m_Height;
    hi.channels = 1;
    hi.bitsperpixel = 24; // RGB
    hi.compression = hi.biSizeImage = 0;
    hi.pixPerMeterX = hi.pixPerMeterY = 0;
    hi.colors = hi.colorsImportant = 0;

    fwrite(&BM_MAGIC, 2, 1, fp);
    fwrite(&hd, sizeof(hd), 1, fp);
    fwrite(&hi, sizeof(hi), 1, fp);

    char xx[VFB_MAX_SIZE*3];
    for ( int y = m_Height - 1; y >= 0; --y )
    {
        for ( int x = 0; x < m_Width; ++x )
        {
            unsigned t = GetPixel(x, y).toRGB32();
            xx[x*3    ] = (0xff     & t);
            xx[x*3 + 1] = (0xff00   & t) >> 8;
            xx[x*3 + 2] = (0xff0000 & t) >> 16;
        }

        fwrite(xx, rowsz, 1, fp);
    }

    fclose(fp);
    return true;
}
