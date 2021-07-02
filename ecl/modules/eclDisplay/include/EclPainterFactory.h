/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//ECL
#include <ecl/modules/eclDisplay/EclData.h>

namespace Belle2 {

  class EclPainter;

  namespace ECL {
    class ECLChannelMapper;
  }
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
                                     ECL::ECLChannelMapper* mapper,
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
