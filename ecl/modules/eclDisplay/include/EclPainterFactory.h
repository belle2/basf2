#ifndef ECLPAINTERFACTORY_H
#define ECLPAINTERFACTORY_H

#include <ecl/modules/eclDisplay/EclPainter.h>
#include <ecl/modules/eclDisplay/EclPainter1D.h>
#include <ecl/modules/eclDisplay/EclPainterAmp.h>
#include <ecl/modules/eclDisplay/EclPainter2D.h>
//#include <ecl/modules/eclDisplay/EclPainter3D.h>
#include <ecl/modules/eclDisplay/EclPainterPolar.h>

namespace Belle2 {
  // Also see const char* titles definition in EclPainterFactory.cc
  enum EclPainterType {
    PAINTER_PHI,
    //  PAINTER_THETA,
    PAINTER_CHANNEL,
    PAINTER_SHAPER,
    PAINTER_COLLECTOR,
    PAINTER_AMP,
    PAINTER_CHANNEL_2D,
    PAINTER_SHAPER_2D,
//    PAINTER_3D
  };

  class EclPainterFactory {
  private:
//    static const int types_count = PAINTER_3D+1;
    static const int types_count = PAINTER_SHAPER_2D + 1;
    static const char* titles[types_count];

    EclPainterFactory();

  public:

    static EclPainter* CreatePainter(EclPainterType type, EclData* data);
    static const char** GetTypeTitles();
    static int GetTypeTitlesCount();
  };
}

#endif // ECLPAINTERFACTORY_H
