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
  // Also see const char* titles definition in EclPainterFactory.cc
  enum EclPainterType {
    PAINTER_PHI,
    //  PAINTER_THETA,
    PAINTER_CHANNEL,
    PAINTER_SHAPER,
    PAINTER_COLLECTOR,
    PAINTER_AMP,
    PAINTER_AMP_SUM,
    PAINTER_TIME,
    PAINTER_CHANNEL_2D,
    PAINTER_SHAPER_2D
  };

  /**
   * @brief Class that implements Factory pattern to create objects inherited
   * from EclPainter.
   */
  class EclPainterFactory {
  private:
    /// Number of painter types.
    static const int types_count = PAINTER_SHAPER_2D + 1;
    /// Titles for painter types. Also see const char* titles definition in EclPainterFactory.cc
    static const char* titles[types_count];

    /**
     * @brief Constructor for EclPainterFactory
     */
    EclPainterFactory();

  public:

    /**
     * @brief Creates EclPainter of the specified type.
     * It will show data from EclData specific to its type.
     */
    static EclPainter* createPainter(EclPainterType type, EclData* data,
                                     ECLChannelMapper* mapper,
                                     EclData::EclSubsystem subsys = EclData::ALL);
    /**
     * @brief Returns array of titles for each EclPainter type.
     */
    static const char** getTypeTitles();
    /**
     * @brief Size of array from getTypeTitles()
     */
    static int getTypeTitlesCount();
  };
}

#endif // ECLPAINTERFACTORY_H
