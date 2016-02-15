/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef __TRACKTIMEEXTRACTIONMODULE_H__
#define __TRACKTIMEEXTRACTIONMODULE_H__

#include <framework/core/HistoModule.h>

#include <memory>

#include "TH1.h"
#include "TH2.h"
#include "TTree.h"

template <typename t>
class TMatrixTSym;
template <typename t>
class TVectorT;

namespace genfit {
  class Track;
  class KalmanFitterInfo;
}

namespace Belle2 {
  class PXDRecoHit;
  class SVDRecoHit;
}

namespace Belle2 {
  class TrackTimeExtractionModule : public HistoModule {
  public:
    /** Constructor, for setting module description and parameters. */
    TrackTimeExtractionModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~TrackTimeExtractionModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize();

    virtual void defineHisto();

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event();

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun();

    /** Clean up anything you created in initialize(). */
    virtual void terminate();
  private:

    /** Collects the dimensions of the various measurements in the track.  */
    void getMeasurementDimensions(const genfit::Track* tr, std::vector<int>& vDimMeas);
    /** Builds the full covariance matrix for the track.  */
    bool buildFullCovarianceMatrix(const genfit::Track* tr,
                                   TMatrixTSym<double>& fullCovariance);
    /** Builds the full covariance matrix for the track.  In the process
     it also builds the inverse of the (block-diagonal) measurement
     covariance matrix.  */
    bool buildFullResidualCovarianceMatrix(const genfit::Track* tr,
                                           const std::vector<int>& vDimMeas,
                                           const TMatrixTSym<double>& fullCovariance,
                                           TMatrixTSym<double>& fullResidualCovariance,
                                           TMatrixTSym<double>& inverseFullMeasurementCovariance);
    /** Puts together the vector of residuals and their time derivatives.  */
    void buildResidualsAndTimeDerivative(const genfit::Track* tr,
                                         const std::vector<int>& vDimMeas,
                                         TVectorT<double>& residuals,
                                         TVectorT<double>& residualTimeDerivative);


    std::string m_InTrackColName; //! Column from which to read tracks.
    std::string m_InTrackCandColName; //! Column for the corresponding track candidates.
    std::string m_OutTrackCandColName; //! New track candidates are output her.

    bool m_DoHistogramming;  //! If set analysis histograms and root tree are written.
  };
}

#endif
