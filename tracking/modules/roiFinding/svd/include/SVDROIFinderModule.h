/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/roiFinding/VXDInterceptor.h>
#include <tracking/roiFinding/ROIToUnitTranslator.h>
#include <tracking/roiFinding/ROIinfo.h>

#include <string>

namespace Belle2 {
  class RecoTrack;
  class ROIid;
  class SVDIntercept;

  /** The SVD ROI Finder Module
   *
   * this module performs the ROI Finding for SVD data. A list of tracks (reconstructed using RecoTracks) is loaded from the event and then each track is extrapolated on the SVD planes; for each interception, a region around the predicted intersection is identified and the Region of Interest for strips belonging to that region is produced
   *
   */

  class SVDROIFinderModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    SVDROIFinderModule();

    /**
     * Destructor of the module.
     */
    ~SVDROIFinderModule();

    /**
     *Initializes the Module. Parameter handling.
     */
    void initialize() override;

    /**
     * Initializations at the beginning of the run.
     */
    void beginRun() override;

    /**
     * Event loop.
     */
    void event() override;

    /**
     * Deletion of objects at the end of the run.
     */
    void endRun() override;

    /**
     * Termination action.
     */
    void terminate() override;

  protected:
    ROIToUnitTranslator<SVDIntercept>* m_theStripTranslator = nullptr; /**< the strip translator object*/

    VXDInterceptor<SVDIntercept>* m_theSVDInterceptor = nullptr; /**< the svd interceptor object*/
    std::string m_SVDInterceptListName; /**< intercept list name*/
    std::string m_ROIListName; /**< ROI list name*/
    std::string m_recoTracksListName; /**< track list name*/

    StoreArray<RecoTrack> m_recotracks; /**<reco tracks store array */
    StoreArray<ROIid> m_rois; /**< rois store array */
    StoreArray<SVDIntercept> m_intercepts; /**< svd intercept store array */

    double m_toleranceZ; /**< determination of interesting planes, tolerance along Z*/
    double m_tolerancePhi;  /**< determination of interesting planes, tolerance in phi*/

    double m_sigmaSystU; /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI U width */
    double m_sigmaSystV;  /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI V width */
    double m_numSigmaTotU; /**< number of sigma (stat+syst) determining the U width of the ROI*/
    double m_numSigmaTotV; /**< number of sigma (stat+syst) determining the U width of the ROI*/
    double m_maxWidthU;  /**< maximum U width of the ROI */
    double m_maxWidthV;  /**< maximum V width of the ROI */

    ROIinfo m_ROIinfo = {0, 0, 0, 0, 0, 0, "", "", "", ""}; /**< contains the parameters that can be changed by the user*/

  private:

  };
}

