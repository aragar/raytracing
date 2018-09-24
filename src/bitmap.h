#ifndef RAYTRACING_BITMAP_H
#define RAYTRACING_BITMAP_H

#include "color.h"

class Bitmap
{
public:
    Bitmap();
    ~Bitmap();

    void freeMem();

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    bool IsOK() const { return (m_Data != nullptr); }

    void GenerateEmptyImage(int width, int height);

    Color GetPixel(int x, int y) const;
    void SetPixel(int x, int y, const Color& color);

    bool LoadBMP(const char* filename);
    bool SaveBMP(const char* filename);

    bool LoadEXR(const char* filename);
    bool SaveEXR(const char* filename);

    virtual bool LoadImage(const char* filename);
    virtual bool SaveImage(const char* filename);

private:
    int m_Width;
    int m_Height;

    Color* m_Data;
};

#endif //RAYTRACING_BITMAP_H
