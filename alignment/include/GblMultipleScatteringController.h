/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GblMultipleScatteringController_h
#define GblMultipleScatteringController_h

#include <genfit/GblTrackSegmentController.h>
#include <genfit/GblFitter.h>

namespace Belle2 {

  /**
   * @brief TrackSegmentController for use with GblFitter in Belle2
   *
   * Its main purpose is to switch off GBL scatterers inside CDC.
   * This has no measurable effects except for much improved performance
   * of the fitter.
   *
   */
  class GblMultipleScatteringController : public genfit::GblTrackSegmentController {

  public:

    /// @brief Cosntructor
    GblMultipleScatteringController() : GblTrackSegmentController() {;}
    /// @brief Destructor
    virtual ~GblMultipleScatteringController() {;}

    /**
     * @brief Change fitter options to not use scatterers
     * inside CDC. Currently by checking the volume name
     * in the middle of entry and exit vectors
     *
     * @param entry First point in track segment position in global coords
     * @param exit Second point in track segment position in global coords
     * @param fitter Pointer to the fitter
     * @return void
     */
    virtual void controlTrackSegment(TVector3 entry, TVector3 exit, genfit::GblFitter* fitter);

  protected:

  public:

    //ClassDef(GblMultipleScatteringController, 1) /** GBL multiple scattering controller for Belle 2 */

  };

} /* End of namespace Belle2 */
/** @} */

#endif // GblMultipleScatteringController_h
