/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/svdROIFinder/SVDInterceptor.h>
#include <tracking/svdROIFinder/ROIStripTranslator.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/SVDIntercept.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <time.h>
#include <tracking/pxdDataReductionClasses/ROIinfo.h>

namespace Belle2 {

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

    void beginRun() override;

    void event() override;

    void endRun() override;

    /**
     * Termination action.
     */
    void terminate() override;

  protected:
    ROIStripTranslator* m_theStripTranslator; /**< the strip translator object*/

    SVDInterceptor* m_theSVDInterceptor; /**< the svd interceptor object*/
    std::string m_SVDInterceptListName; /**< intercept list name*/
    std::string m_ROIListName; /**< ROI list name*/
    std::string m_recoTracksListName; /**< track list name*/

    StoreArray<RecoTrack> m_recotracks; /**<reco trcks store array */
    StoreArray<ROIid> m_rois; /**< rois store array */
    StoreArray<SVDIntercept> m_intercepts; /**< svd intercept store array */

    int m_numIterKalmanFilter; /**< number of iterations of the Kalman Filter*/

    double m_toleranceZ; /**< determination of interesting planes, tolerance along Z*/
    double m_tolerancePhi;  /**< determination of interesting planes, tolerance in phi*/

    double m_sigmaSystU; /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI U width */
    double m_sigmaSystV;  /**< fixed width to add in quadrature to the extrapolation error and obtain the ROI V width */
    double m_numSigmaTotU; /**< number of sigma (stat+syst) determining the U width of the ROI*/
    double m_numSigmaTotV; /**< number of sigma (stat+syst) determining the U width of the ROI*/
    double m_maxWidthU;  /**< maximum U width of the ROI */
    double m_maxWidthV;  /**< maximum V width of the ROI */

    ROIinfo m_ROIinfo; /**< contains the parameters that can be changed by the user*/

  private:

  };
}

