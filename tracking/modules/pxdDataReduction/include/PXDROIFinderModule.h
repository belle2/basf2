/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <tracking/pxdDataReductionClasses/ROIinfo.h>
#include <framework/database/DBObjPtr.h>
#include <tracking/dbobjects/ROIParameters.h>

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
     * Initializes the module.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called at the end of the event processing.
     */
    void endRun() override;

    StoreArray<RecoTrack> m_RecoTracks;   /**< RecoTracks StoreArray */
    StoreArray<PXDIntercept> m_PXDIntercepts;   /**< PXDIntercepts StoreArray */
    StoreArray<ROIid> m_ROIs;   /**< ROIs StoreArray */
    ROIinfo m_ROIinfo; /**< contains the parameters that can be changed by the user*/

    std::string m_PXDInterceptListName; /**< intercept list name*/
    std::string m_ROIListName; /**< ROI list name*/
    std::string m_recoTracksListName; /**< track list name*/

    ROIPixelTranslator* m_thePixelTranslator = nullptr; /**< the pixel translator object*/
    PXDInterceptor* m_thePXDInterceptor = nullptr; /**< the pxd interceptor object*/

    double m_toleranceZ; /**< tolerance for finding sensor in Z coordinate (cm) */
    double m_tolerancePhi; /**< tolerance for finding sensor in phi coordinate (radians) */

    double m_sigmaSystU; /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI U width */
    double m_sigmaSystV;  /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI V width */
    double m_numSigmaTotU; /**< number of sigma (stat+syst) determining the U width of the ROI*/
    double m_numSigmaTotV; /**< number of sigma (stat+syst) determining the U width of the ROI*/
    double m_maxWidthU;  /**< maximum U width of the ROI */
    double m_maxWidthV;  /**< maximum V width of the ROI */


    DBObjPtr<ROIParameters> m_roiParameters;  /**< Configuration parameters for ROIs */


  };
}
