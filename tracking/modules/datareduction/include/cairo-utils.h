#ifndef CAIRO_UTILS_H
#define CAIRO_UTILS_H

#include "cairo.h"
#include "cairo-pdf.h"

#include <vector>
#include <string>
#include <cmath>

#define CAIRO_SIZE 500

void cairo_plot(cairo_t *ctx, double min_x, double max_x, double min_y, double max_y, double width = CAIRO_SIZE, double height = CAIRO_SIZE);

void cairo_sine(cairo_t *cr, double amplitude, double period, double offset, double min_x, double max_x, int steps = 0);
void cairo_straight(cairo_t *cr, double m, double t, double min_x, double max_x, double min_y, double max_y);

void cairo_text(cairo_t *ctx, const std::string &txt, bool flip = true);

class cairo_color {
protected:
  double red;
  double green;
  double blue;
public:
  cairo_color(): red(0), green(0), blue(0) {};
  cairo_color(double red, double green, double blue) {
    this->red = red;
    this->green = green;
    this->blue = blue;
  };
  cairo_color(int red, int green, int blue) {
    this->red = red / 255.0;
    this->green = green / 255.0;
    this->blue = blue / 255.0;
  };
  inline cairo_color operator+(const cairo_color &c) const {
    return cairo_color(red + c.red, green + c.green, blue + c.blue);
  };
  inline cairo_color operator*(double s) const {
    return cairo_color(red*s, green*s, blue*s);
  }
  inline bool operator!=(const cairo_color &c) {
    return red != c.red || green != c.green || blue != c.blue;
  }

  static cairo_color sample(std::vector<cairo_color> cmap, double value, double min, double max) {
    int steps = cmap.size() - 1;
    double position = value / fabs(max - min) * steps;
    int left = floor(position);
    position -= left;
    if (max < min) {
      left = steps - left - 1;
      position = 1 - position;
    }
    int right = left + 1;

    if (left < 0) return cmap[0];
    if (right > steps) return cmap[steps];
    return cmap[left]*(1 - position) + cmap[right]*position;

  }

  friend void cairo_set_source_rgb(cairo_t* ctx, cairo_color c);
  friend void cairo_set_source_rgba(cairo_t* ctx, cairo_color c, double alpha);
};

void cairo_set_source_rgb(cairo_t* ctx, cairo_color c);
void cairo_set_source_rgba(cairo_t* ctx, cairo_color c, double alpha = 1);
void cairo_stroke_abs(cairo_t* ctx, bool preserve = false);

#endif
