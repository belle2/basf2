/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <genfit/GblTrackSegmentController.h>

namespace Belle2 {

  /**
   * @brief TrackSegmentController for use with GblFitter in Belle2
   *
   * Its main purpose is to switch off GBL scatterers inside CDC.
   * This has no measurable effects except for much improved performance
   * of the fitter.
   */
  class GblMultipleScatteringController : public genfit::GblTrackSegmentController {

  public:

    /// @brief Constructor
    GblMultipleScatteringController() : GblTrackSegmentController() {;}
    /// @brief Destructor
    virtual ~GblMultipleScatteringController() {;}

    /**
     * @brief Change fitter options to not use scatterers
     * inside CDC. Currently by checking the volume name
     * in the middle of entry and exit vectors.
     *
     * The first two arguments are unused in our implementation, so they are not explicitely named here,
     * even though they are present in the base class genfit::GblTrackSegmentController.
     *
     * @param scatTheta Total MS variance accumulated in segment
     * @param fitter Pointer to the fitter
     * @return void
     */
    virtual void controlTrackSegment(TVector3, TVector3, double scatTheta, genfit::GblFitter* fitter);
//     virtual void controlTrackSegment(B2Vector3D, B2Vector3D, double scatTheta, genfit::GblFitter* fitter);

  };

} /* End of namespace Belle2 */

