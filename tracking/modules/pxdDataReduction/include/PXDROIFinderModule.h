/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/

#pragma once

#include <framework/core/Module.h>
#include <tracking/pxdDataReductionClasses/PXDInterceptor.h>
#include <tracking/pxdDataReductionClasses/ROIPixelTranslator.h>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <time.h>
#include <tracking/pxdDataReductionClasses/ROIinfo.h>

namespace Belle2 {

  /** The PXD ROI Finder Module
   *
   * this module performs the ROI Finding for PXD data. A list of tracks (reconstructed using RecoTracks) is loaded from the event and then each track is extrapolated on the PXD planes; for each interception, a region around the predicted intersection is identified and the Region of Interest for pixels belonging to that region is produced
   *
   */

  class PXDROIFinderModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    PXDROIFinderModule();

  private:

    /**
     *Initializes the Module.
     */
    void initialize() override;

    void beginRun() override;

    void event() override;

    void endRun() override;

    ROIPixelTranslator* m_thePixelTranslator; /**< the pixel translator object*/
    PXDInterceptor* m_thePXDInterceptor; /**< the pxd interceptor object*/
    std::string m_PXDInterceptListName; /**< intercept list name*/
    std::string m_ROIListName; /**< ROI list name*/
    std::string m_recoTracksListName; /**< track list name*/

    int m_numIterKalmanFilter; /**< number of iterations of the Kalman Filter*/

    double m_toleranceZ;
    double m_tolerancePhi;

    double m_sigmaSystU; /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI U width */
    double m_sigmaSystV;  /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI V width */
    double m_numSigmaTotU; /**< number of sigma (stat+syst) determining the U width of the ROI*/
    double m_numSigmaTotV; /**< number of sigma (stat+syst) determining the U width of the ROI*/
    double m_maxWidthU;  /**< maximum U width of the ROI */
    double m_maxWidthV;  /**< maximum V width of the ROI */

    ROIinfo m_ROIinfo; /**< contains the parameters that can be changed by the user*/

  };
}
