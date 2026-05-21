#include "color.h"

Color color(float r, float g, float b, float a)
{
    return (Color) {
        .r = r,
        .g = g,
        .b = b,
        .a = a,
    };
}

Color colorFrom256(int r, int g, int b, int a)
{
    return color(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
}

Color colorFromHex(unsigned int hex)
{
    return colorFrom256(
        (hex & 0xFF000000) >> 24,
        (hex & 0x00FF0000) >> 16,
        (hex & 0x0000FF00) >> 8,
        (hex & 0x000000FF) >> 0
    );
}

float* color_data(Color *color)
{
    return &color->r;
}
