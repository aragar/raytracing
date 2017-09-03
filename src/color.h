#ifndef RAYTRACING_COLOR_H
#define RAYTRACING_COLOR_H

struct Color
{
public:
    Color() {}
    Color(float r, float g, float b);
    explicit Color(unsigned rgbColor);

    unsigned toRGB32(int redShift = 16, int greenShift = 8, int blueShift = 0) const;
    unsigned toSRGB32(int redShift = 16, int greenShift = 8, int blueShift = 0) const;

    void SetColor(float r, float g, float b);
    void MakeZero() { SetColor(0.f, 0.f, 0.f); }

    float Intensity() const { return (r + g + b) / 3; }
    float IntensityPerceptual() const { return (r*0.299f + g*0.587f + b*0.114f); }

    void operator+=(const Color& rhs);
    void operator*=(float multiplier);
    void operator/=(float divider);

    const float& operator[](int index) const { return components[index]; }
    float& operator[](int index) { return components[index]; }

    union
    {
        struct { float r, g, b; };
        float components[3];
    };
};

inline Color operator+(const Color& a, const Color& b)         { return {a.r + b.r,        a.g + b.g,        a.b + b.b       }; }
inline Color operator-(const Color& a, const Color& b)         { return {a.r - b.r,        a.g - b.g,        a.b - b.b       }; }
inline Color operator*(const Color& a, const Color& b)         { return {a.r * b.r,        a.g * b.g,        a.b * b.b       }; }
inline Color operator*(const Color& a, float multiplier)       { return {a.r * multiplier, a.g * multiplier, a.b * multiplier}; }
inline Color operator*(const float multiplier, const Color& a) { return {a.r * multiplier, a.g * multiplier, a.b * multiplier}; }
inline Color operator/(const Color& a, float divider)          { return {a.r / divider,    a.g / divider,    a.b / divider   }; }

#endif //RAYTRACING_COLOR_H
