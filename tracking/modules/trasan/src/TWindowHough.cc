//-----------------------------------------------------------------------------
// $Id: TWindowHough.cc 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TWindowHough.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display tracking object in Hough finder.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.4  2005/11/03 23:20:13  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.3  2005/04/18 23:41:47  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.2  2005/03/11 03:57:53  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.1  2004/02/18 04:07:27  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------


#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/Strings.h"
#include "tracking/modules/trasan/TWindowHough.h"
#include "tracking/modules/trasan/THoughPlane.h"


#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif


  TWindowHough::TWindowHough(const std::string& name, int x, int y)
    : TWindow(name, x, y)
  {
  }

  TWindowHough::~TWindowHough()
  {
  }

  void
  TWindowHough::draw(const THoughPlane& hp)
  {

    //...Cal. scale for y coordinate...
    const float xMin = hp.xMin();
    const float xMax = hp.xMax();
    const float yMin = hp.yMin();
    const float scale = (xMax - xMin) / (hp.yMax() * 1.15 - hp.yMin());

    //...Initialize window...
    _window.init(hp.xMin(), hp.xMax(), hp.yMin());
    _window.display();

    //...Search maximum...
    unsigned nMax = 0;
    for (unsigned i = 0; i < hp.nX(); i++) {
      for (unsigned j = 0; j < hp.nY(); j++) {
        const unsigned n = hp.entry(i, j);
        if (n > nMax) nMax = n;
      }
    }

    //...Draw...
    leda_color level0 = leda_grey1;
    leda_color level1 = leda_grey2;
    leda_color level2 = leda_grey3;
    leda_color level3 = leda_pink;
    leda_color top = leda_red;
    for (unsigned i = 0; i < hp.nX(); i++) {
      for (unsigned j = 0; j < hp.nY(); j++) {
        const unsigned n = hp.entry(i, j);
        if (n) {
          const float x0 = hp.xMin() + float(i) * hp.xSize();
          const float y0 = hp.yMin() + float(j) * hp.ySize() * scale;
          const float x1 = x0 + hp.xSize();
          const float y1 = y0 + hp.ySize() * scale;
          const float level = float(n) / float(nMax);
          if (level < 0.25)
            _window.draw_filled_rectangle(x0, y0, x1, y1, level0);
          else if (level < 0.5)
            _window.draw_filled_rectangle(x0, y0, x1, y1, level1);
          else if (level < 0.75)
            _window.draw_filled_rectangle(x0, y0, x1, y1, level2);
          else if (level == 1.)
            _window.draw_filled_rectangle(x0, y0, x1, y1, top);
          else
            _window.draw_filled_rectangle(x0, y0, x1, y1, level3);
        }
      }
    }

    //...Draw region...
    const AList<CList<unsigned> > & regions = hp.regions();
    for (unsigned i = 0; i < (unsigned) regions.length(); i++) {

//  std::cout << "TWH ... region " << i << std::endl;

      for (unsigned j = 0; j < (unsigned) regions[i]->length(); j++) {
        const unsigned id = * (* regions[i])[j];
        unsigned x = 0;
        unsigned y = 0;
        hp.id(id, x, y);
        const float x0 = hp.xMin() + float(x) * hp.xSize();
        const float y0 = hp.yMin() + float(y) * hp.ySize() * scale;
        const float x1 = x0 + hp.xSize();
        const float y1 = y0 + hp.ySize() * scale;

//      std::cout << "TWH ... id=" << id << std::endl;

        for (unsigned k = 0; k < 8; k++) {
          if (k % 2) continue;
          unsigned idx = hp.neighbor(id, k);

          if (idx == id) continue;
          bool found = false;
          for (unsigned l = 0;
               l < (unsigned) regions[i]->length();
               l++) {
            if (idx == * (* regions[i])[l]) {
//      std::cout << "        " << idx << " is neighbor " << k << std::endl;
              found = true;
              break;
            }
//        std::cout << "        " << idx << " is not neighbor " << k << std::endl;
          }
          if (found) continue;
          if (k == 0)
            _window.draw_segment(x0, y1, x1, y1, leda_black);
          else if (k == 2)
            _window.draw_segment(x1, y0, x1, y1, leda_black);
          else if (k == 4)
            _window.draw_segment(x0, y0, x1, y0, leda_black);
          else if (k == 6)
            _window.draw_segment(x0, y0, x0, y1, leda_black);
        }
      }
    }

    //...Draw text...
    _xPositionText = hp.xMin() + hp.xSize() * hp.nX() * 0.1;
    _yPositionText = hp.yMin() + hp.ySize() * hp.nY() * 0.9 * scale;
    _window.draw_text(_xPositionText, _yPositionText, _text.c_str());
    std::string text = " y_scale=" + dtostring(1 / scale);
    text += " max.peak=" + itostring(hp.maxEntry());
    text += " #regions=" + itostring(hp.regions().length());
    _yPositionText = hp.yMin() + hp.ySize() * hp.nY() * 0.85 * scale;
    _window.draw_text(_xPositionText, _yPositionText, text.c_str());

    //...Draw reference line(CDC outer wall)...
//     const float outerWall = log10(88. / 2.);
//     const float yOuterWall = outerWall * scale;
//     _window.draw_segment(xMin, yOuterWall, xMax, yOuterWall, leda_grey2);
  }

  void
  TWindowHough::draw(const THoughPlane& hp,
                     const AList<TPoint2D> & list,
                     float radius,
                     leda_color c)
  {

    draw(hp);

    //...Cal. scale for y coordinate...
    const float xMin = hp.xMin();
    const float xMax = hp.xMax();
    const float yMin = hp.yMin();
    const float scale = (xMax - xMin) / (hp.yMax() * 1.15 - hp.yMin());

    //...y=0 line...
    if (yMin < 0) {
      const float z = (0 - yMin) * scale + yMin;
      _window.draw_segment(xMin, z, xMax, z, leda_grey2);
    }

    //...
    if (list.length()) {
      for (unsigned i = 0; i < (unsigned) list.length(); i++) {
        _window.draw_circle(list[i]->x(),
                            (list[i]->y() - yMin) * scale + yMin,
                            radius,
                            c);

//      std::cout << "TWH ... region " << i << ":x=" << list[i]->x()
//          << ",y=" << list[i]->y() << std::endl;
      }
    }
  }

  void
  TWindowHough::wait(void)
  {
    bool loop = true;
    while (loop) {

      //...Read input...
      double x0, y0;
      int b = _window.read_mouse(x0, y0);

      if (b == _closeButton) loop = false;
      else if (b == _coordinateButton) {
        _coordinate = ! _coordinate;
        _window.set_show_coordinates(_coordinate);
      }
    }
  }

  void
  TWindowHough::drawOver(const THoughPlane& hp,
                         const AList<TPoint2D> & list,
                         float radius,
                         leda_color c)
  {

    //...Cal. scale for y coordinate...
    const float xMin = hp.xMin();
    const float xMax = hp.xMax();
    const float yMin = hp.yMin();
    const float scale = (xMax - xMin) / (hp.yMax() * 1.15 - hp.yMin());

    //...
    if (list.length()) {
      for (unsigned i = 0; i < (unsigned) list.length(); i++)
        _window.draw_circle(list[i]->x(),
                            (list[i]->y() - yMin) * scale + yMin,
                            radius,
                            c);
    }
  }


#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif
