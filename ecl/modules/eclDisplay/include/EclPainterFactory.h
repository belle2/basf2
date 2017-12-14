/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#ifndef ECLPAINTERFACTORY_H
#define ECLPAINTERFACTORY_H

#include <ecl/modules/eclDisplay/EclPainter.h>
#include <ecl/modules/eclDisplay/EclPainter1D.h>
#include <ecl/modules/eclDisplay/EclPainter2D.h>
#include <ecl/modules/eclDisplay/EclPainterPolar.h>
#include <ecl/modules/eclDisplay/EclPainterCommon.h>

namespace Belle2 {
  /**
   * Enum for type of EclPainter to create. Also see const char* titles
   * definition in EclPainterFactory.cc
   */
  enum EclPainterType {
    PAINTER_PHI, /**< Event count/energy polar angle distribution */
    PAINTER_CHANNEL, /**< Event count/energy distribution per channel */
    PAINTER_SHAPER, /**< Event count/energy distribution per shaperDSP */
    PAINTER_COLLECTOR, /**< Event count/energy distribution per crate/ECLCollector */
    PAINTER_1D_PHI, /**< Event count/energy distribution per phi_id */
    PAINTER_1D_THETA, /**< Event count/energy distribution per theta_id */
    PAINTER_ENERGY,  /**< Channel energy distribution */
    PAINTER_ENERGY_SUM, /**< Event energy distribution */
    PAINTER_TIME, /**< Time distribution */
    PAINTER_CHANNEL_2D, /**< (theta_id:phi_id) histogram. */
    PAINTER_SHAPER_2D /**< (shaper:crate) histogram. */
  };

  /**
   * Class that implements Factory pattern to create objects inherited
   * from EclPainter.
   */
  class EclPainterFactory {
  private:
    /**  Number of painter types. */
    static const int types_count = PAINTER_SHAPER_2D + 1;
    /**  Titles for painter types. Also see const char* titles definition in EclPainterFactory.cc */
    static const char* titles[types_count];

    /**
     * Constructor for EclPainterFactory
     */
    EclPainterFactory();

  public:

    /**
     * Creates EclPainter of the specified type.
     * It will show data from EclData specific to its type.
     */
    static EclPainter* createPainter(EclPainterType type, EclData* data,
                                     ECLChannelMapper* mapper,
                                     EclData::EclSubsystem subsys = EclData::ALL);
    /**
     * Returns array of titles for each EclPainter type.
     */
    static const char** getTypeTitles();
    /**
     * Size of array from getTypeTitles()
     */
    static int getTypeTitlesCount();
  };
}

#endif // ECLPAINTERFACTORY_H
