#include <ecl/modules/eclDisplay/EclPainterFactory.h>

using namespace Belle2;

const char* EclPainterFactory::titles[types_count] = {
  "Distribution of phi segments",
//  "Energy per theta",
  "Distribution of channels",
  "Distribution of shapers",
  "Distribution of crates",
  "Amplitude distribution",
  "Event display (channels)",
  "Event display (shapers)",
//  "Energy on ECL cylinder"
};

EclPainterFactory::EclPainterFactory()
{
}

EclPainter* EclPainterFactory::CreatePainter(EclPainterType type, EclData* data)
{
  EclPainter* painter = 0;
  switch (type) {
    case PAINTER_PHI:
      painter = new EclPainterPolar(data, EclPainterPolar::PHI);
      break;
//    case PAINTER_THETA:
//      painter = new EclPainterPolar(data, EclPainterPolar::THETA);
//      break;
    case PAINTER_CHANNEL:
      painter = new EclPainter1D(data, EclPainter1D::CHANNEL);
      break;
    case PAINTER_SHAPER:
      painter = new EclPainter1D(data, EclPainter1D::SHAPER);
      break;
    case PAINTER_COLLECTOR:
      painter = new EclPainter1D(data, EclPainter1D::CRATE);
      break;
    case PAINTER_AMP:
      painter = new EclPainterAmp(data, 50, 3000);
      break;
    case PAINTER_CHANNEL_2D:
      painter = new EclPainter2D(data, EclPainter2D::CHANNEL_2D);
      break;
    case PAINTER_SHAPER_2D:
      painter = new EclPainter2D(data, EclPainter2D::SHAPER_2D);
      break;
//    case PAINTER_3D:
//      painter = new EclPainter3D(data, EclPainter3D::THETA_PHI);
//      break;
  }
  return painter;
}

const char** EclPainterFactory::GetTypeTitles()
{
  return titles;
}

int EclPainterFactory::GetTypeTitlesCount()
{
  return types_count;
}
