/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#include <ecl/modules/eclDisplay/EclPainterFactory.h>

using namespace Belle2;

const char* EclPainterFactory::titles[types_count] = {
  "Distribution of phi segments",
//  "Energy per theta",
  "Distribution of channels",
  "Distribution of shapers",
  "Distribution of crates",
  "Amplitude per channel distribution",
  "Amplitude sum per event distribution",
  "Time distribution",
  "Event display (channels)",
  "Event display (shapers)"
};

EclPainterFactory::EclPainterFactory()
{
}

EclPainter* EclPainterFactory::createPainter(EclPainterType type, EclData* data,
                                             ECLChannelMapper* mapper,
                                             EclData::EclSubsystem subsys)
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
      painter = new EclPainterCommon(data, EclPainterCommon::AMP);
      // painter = new EclPainterAmp(data, 50, 3000);
      break;
    case PAINTER_AMP_SUM:
      painter = new EclPainterCommon(data, EclPainterCommon::AMP_SUM);
      break;
    case PAINTER_TIME:
      painter = new EclPainterCommon(data, EclPainterCommon::TIME);
      // painter = new EclPainterTime(data, -2048, 2048);
      break;
    case PAINTER_CHANNEL_2D:
      painter = new EclPainter2D(data, EclPainter2D::CHANNEL_2D);
      break;
    case PAINTER_SHAPER_2D:
      painter = new EclPainter2D(data, EclPainter2D::SHAPER_2D);
      break;
  }
  painter->setMapper(mapper);
  painter->setDisplayedSubsystem(subsys);
  return painter;
}

const char** EclPainterFactory::getTypeTitles()
{
  return titles;
}

int EclPainterFactory::getTypeTitlesCount()
{
  return types_count;
}
