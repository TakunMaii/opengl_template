#ifndef COLOR_H
#define COLOR_H

// r, g, b: range(0, 1)
typedef struct {
    float r;
    float g;
    float b;
    float a;
} Color;

Color color(float r, float g, float b, float a);
Color colorFrom256(int r, int g, int b, int a);
Color colorFromHex(unsigned int hex);
float* color_data(Color *color);

#endif /* COLOR_H */

