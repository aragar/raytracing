#include "color.h"
#include "utils.h"

Color::Color(float r, float g, float b)
: r(r),
  g(g),
  b(b)
{}

Color::Color(unsigned rgbColor)
: r(( rgbColor        & 0xff) / 255.f),
  g(((rgbColor >>  8) & 0xff) / 255.f),
  b(((rgbColor >> 16) & 0xff) / 255.f)
{}

unsigned Color::toRGB32(int redShift/* = 16*/, int greenShift/* = 8*/, int blueShift/* = 0*/) const
{
    unsigned  red = ConvertTo8Bit(r);
    unsigned  green = ConvertTo8Bit(g);
    unsigned  blue = ConvertTo8Bit(b);
    unsigned rgbColor = (blue << blueShift) | (green << greenShift) | (red << redShift);
    return rgbColor;
}

void Color::SetColor(float r, float g, float b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

void Color::operator+=(const Color &rhs)
{
    r += rhs.r;
    g += rhs.g;
    b += rhs.b;
}

void Color::operator*=(const float multiplier)
{
    r *= multiplier;
    g *= multiplier;
    b *= multiplier;
}

void Color::operator/=(const float divider)
{

}