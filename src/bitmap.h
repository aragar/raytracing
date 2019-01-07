#ifndef RAYTRACING_BITMAP_H
#define RAYTRACING_BITMAP_H

#include "color.h"

class Bitmap
{
public:
    Bitmap();
    virtual ~Bitmap();

    void freeMem();

    unsigned GetWidth() const { return m_Width; }
    unsigned GetHeight() const { return m_Height; }
    bool IsOK() const { return (m_Data != nullptr); }

    void GenerateEmptyImage(unsigned width, unsigned height);

    Color GetPixel(unsigned int x, unsigned int y) const;
    void SetPixel(unsigned x, unsigned y, const Color& color);

    Color GetBilinearFilteredPixel(double x, double y) const;

    bool LoadBMP(const char* filename);
    bool SaveBMP(const char* filename);

    bool LoadEXR(const char* filename);
    bool SaveEXR(const char* filename);

    enum class OutputFormat
    {
        BMP,
        EXR
    };

    virtual bool LoadImage(const char* filename);
    virtual bool SaveImage(const char* filename);

    void Differentiate();

private:
    unsigned m_Width;
    unsigned m_Height;

    Color* m_Data;
};

#endif //RAYTRACING_BITMAP_H
