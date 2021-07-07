/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module
#include <ecl/modules/eclDisplay/EclPainterFactory.h>

//ECL
#include <ecl/modules/eclDisplay/EclPainter1D.h>
#include <ecl/modules/eclDisplay/EclPainter2D.h>
#include <ecl/modules/eclDisplay/EclPainterPolar.h>
#include <ecl/modules/eclDisplay/EclPainterCommon.h>

using namespace Belle2;

const char* EclPainterFactory::titles[types_count] = {
  "Distribution of phi segments",
  "Energy per channel distribution",
  "Energy per shaper distribution",
  "Energy per crate distribution",
  "Energy per phi_id distribution",
  "Energy per theta_id distribution",
  "Energy distribution",
  "Total event energy distribution",
  "Time distribution",
  "Event display (channels)",
  "Event display (shapers)"
};

EclPainterFactory::EclPainterFactory()
{
}

EclPainter* EclPainterFactory::createPainter(EclPainterType type, EclData* data,
                                             ECL::ECLChannelMapper* mapper,
                                             EclData::EclSubsystem subsys)
{
  EclPainter* painter = 0;
  switch (type) {
    case PAINTER_PHI:
      painter = new EclPainterPolar(data, EclPainterPolar::PHI);
      break;
    case PAINTER_CHANNEL:
      painter = new EclPainter1D(data, EclPainter1D::CHANNEL);
      break;
    case PAINTER_SHAPER:
      painter = new EclPainter1D(data, EclPainter1D::SHAPER);
      break;
    case PAINTER_COLLECTOR:
      painter = new EclPainter1D(data, EclPainter1D::CRATE);
      break;
    case PAINTER_1D_PHI:
      painter = new EclPainter1D(data, EclPainter1D::PHI);
      break;
    case PAINTER_1D_THETA:
      painter = new EclPainter1D(data, EclPainter1D::THETA);
      break;
    case PAINTER_ENERGY:
      painter = new EclPainterCommon(data, EclPainterCommon::ENERGY);
      break;
    case PAINTER_ENERGY_SUM:
      painter = new EclPainterCommon(data, EclPainterCommon::ENERGY_SUM);
      break;
    case PAINTER_TIME:
      painter = new EclPainterCommon(data, EclPainterCommon::TIME);
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
