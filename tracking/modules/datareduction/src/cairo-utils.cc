#include <tracking/modules/datareduction/cairo-utils.h>
#ifdef CAIRO_OUTPUT
#include <boost/format.hpp>

using namespace std;

double nextRoundNumber(double x)
{
  double goodFactors[] = {1.0, 1.25, 2.0, 2.5, 5.0};
  if (x == 0 || x == 1) return x;
  if (x < 0) return nextRoundNumber(-x);

  double lg = log10(x);
  double base(0);
  if (lg == 0) {
    if (x < 1) base = 0.1;
    else    base = 1.0;
  } else if (lg < 0) {
    base = pow(10.0, lg - 1);
  } else {
    base = pow(10.0, lg);
  }
  for (unsigned int i = 0; i < sizeof(goodFactors) / sizeof(double); ++i) {
    double xround = base * goodFactors[i];
    if (xround >= x) return xround;
  }
  return base*10.0;
}


void cairo_plot(cairo_t *ctx, double min_x, double max_x, double min_y, double max_y, double width, double height)
{
  boost::format label("%.4f");
  {
    double tmp = max(min_x, max_x);
    min_x = min(min_x, max_x);
    max_x = tmp;
  }
  {
    double tmp = max(min_y, max_y);
    min_y = min(min_y, max_y);
    max_y = tmp;
  }

  cairo_text_extents_t minx_extents;
  cairo_text_extents_t maxx_extents;
  cairo_text_extents_t miny_extents;
  cairo_text_extents_t maxy_extents;
  string lminx = (label % min_x).str();
  string lmaxx = (label % max_x).str();
  string lminy = (label % min_y).str();
  string lmaxy = (label % max_y).str();
  cairo_font_extents_t font;
  cairo_font_extents(ctx, &font);
  cairo_text_extents(ctx, lminx.c_str(), &minx_extents);
  cairo_text_extents(ctx, lmaxx.c_str(), &maxx_extents);
  cairo_text_extents(ctx, lminy.c_str(), &miny_extents);
  cairo_text_extents(ctx, lmaxy.c_str(), &maxy_extents);
  width -= font.height;
  height -= font.height;
  cairo_translate(ctx, font.height, 0);
  cairo_save(ctx);
  cairo_rotate(ctx, -M_PI / 2);
  cairo_move_to(ctx, -height + 1, -font.descent);
  cairo_show_text(ctx, lminy.c_str());
  cairo_move_to(ctx, -maxy_extents.x_bearing - maxy_extents.width - 1, -font.descent);
  cairo_show_text(ctx, lmaxy.c_str());
  cairo_restore(ctx);
  cairo_move_to(ctx, 1, height + font.ascent);
  cairo_show_text(ctx, lminx.c_str());
  cairo_move_to(ctx, width - maxx_extents.x_bearing - maxx_extents.width - 1, height + font.ascent);
  cairo_show_text(ctx, lmaxx.c_str());

  cairo_set_line_width(ctx, 0.5);
  cairo_rectangle(ctx, 1, 1, width - 2, height - 2);
  cairo_stroke_abs(ctx, true);
  cairo_clip(ctx);

  double xscale = width / (max_x - min_x);
  double yscale = height / (max_y - min_y);
  cairo_translate(ctx, 0, height);
  cairo_scale(ctx, xscale, -yscale);
  cairo_translate(ctx, -min_x, -min_y);
  cairo_set_line_width(ctx, 0.1 / xscale);
}

void cairo_sine(cairo_t *cr, double amplitude, double period, double offset, double min_x, double max_x, int steps)
{
  double range = max_x - min_x;
  double sw(0);
  if (steps == 0) {
    sw = min(fabs(M_PI / (4 * period)), range / 3);
    steps = ceil(range / sw);
  } else {
    sw = range / steps;
  }
  double sw3 = sw / 3;
  double x0 = min_x;
  double y0 = amplitude * sin(offset + period * x0);
  double d0 = amplitude * period * cos(offset + period * x0);
  cairo_move_to(cr, x0, y0);
  for (int i = 0; i < steps; i++) {
    double x1 = x0 + sw;
    double y1 = amplitude * sin(offset + period * x1);
    double d1 = amplitude * period * cos(offset + period * x1);
    cairo_curve_to(cr, x0 + sw3, y0 + d0*sw3, x1 - sw3, y1 - d1*sw3, x1, y1);
    x0 = x1;
    y0 = y1;
    d0 = d1;
  }
}

#include <iostream>

void cairo_straight(cairo_t *cr, double m, double t, double min_x, double max_x, double min_y, double max_y)
{
  if (m == 0) {
    if (t < min_y || t > max_y) return;
    cairo_move_to(cr, min_x, t);
    cairo_line_to(cr, max_x, t);
    return;
  }
  double x = min_x;
  double y = m * x + t;
  //cout << "x " <<  x << " y " << y << endl;
  if (y < min_y) {
    y = min_y;
    x = (y - t) / m;
  } else if (y > max_y) {
    y = max_y;
    x = (y - t) / m;
  }
  //cout << "x " <<  x << " y " << y << endl;
  cairo_move_to(cr, x, y);
  x = max_x;
  y = m * x + t;
  if (y < min_y) {
    y = min_y;
    x = (y - t) / m;
  } else if (y > max_y) {
    y = max_y;
    x = (y - t) / m;
  }
  cairo_line_to(cr, x, y);
}

void cairo_text(cairo_t *ctx, const string &txt, bool flip)
{
  cairo_save(ctx);
  if (flip) cairo_scale(ctx, 1, -1);
  cairo_text_path(ctx, txt.c_str());
  cairo_fill(ctx);
  cairo_restore(ctx);
}

void cairo_set_source_rgb(cairo_t* ctx, cairo_color c)
{
  cairo_set_source_rgb(ctx, c.red, c.green, c.blue);
}

void cairo_set_source_rgba(cairo_t* ctx, cairo_color c, double alpha)
{
  cairo_set_source_rgba(ctx, c.red, c.green, c.blue, alpha);
}

void cairo_stroke_abs(cairo_t* ctx, bool preserve)
{
  cairo_matrix_t matrix;
  cairo_get_matrix(ctx, &matrix);
  cairo_identity_matrix(ctx);
  if (preserve) cairo_stroke_preserve(ctx);
  else cairo_stroke(ctx);
  cairo_set_matrix(ctx, &matrix);
}
#endif

