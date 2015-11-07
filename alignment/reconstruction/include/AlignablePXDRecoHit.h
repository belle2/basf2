/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, Tobias Schlüter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALIGNABLEPXDRECOHIT_H
#define ALIGNABLEPXDRECOHIT_H


#include <pxd/reconstruction/PXDRecoHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>

#include <genfit/AbsMeasurement.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/TrackCandHit.h>
#include <genfit/HMatrixU.h>

#include <memory>

#include <genfit/ICalibrationParametersDerivatives.h>

namespace Belle2 {
  /** This class is used to transfer PXD information to the track fit. */
  class AlignablePXDRecoHit : public PXDRecoHit, public genfit::ICalibrationParametersDerivatives  {
    friend class PXDRecoHit;
  public:
    /** Default constructor for ROOT IO. */
    AlignablePXDRecoHit() : PXDRecoHit() {}

    /** Construct AlignablePXDRecoHit from a PXDTrueHit for Monte Carlo based tracking.
     * This requires a valid random number generator to be initialized at
     * gRandom. The Hit position will be smeared using a gaussian smearing with
     * sigmaU and sigmaV along u and v respectively
     *
     * If one of the errors is set <0, a default resolution will be
     * assumed for both values by dividing the pixel size by sqrt(12).
     *
     * @param hit    PXDTrueHit to use as base
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     */
    AlignablePXDRecoHit(const PXDTrueHit* hit, const genfit::TrackCandHit* trackCandHit = NULL, float sigmaU = -1,
                        float sigmaV = -1) : PXDRecoHit(hit, trackCandHit, sigmaU, sigmaV) {setDetectorPlane();}

    /** Construct AlignablePXDRecoHit from a PXD cluster.
     * For users that want to supply their own errors on construction
     *
     * @param hit    PXDCluster to use as base
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     * @param covUV  Covariance between u and v
     */
    AlignablePXDRecoHit(const PXDCluster* hit, float sigmaU, float sigmaV, float covUV) : PXDRecoHit(hit, sigmaU, sigmaV,
          covUV) {setDetectorPlane();}

    /** Construct AlignablePXDRecoHit from a PXD cluster
     * This constructor is intended as a temporary solution for people who want
     * to test the impact of realistic clusters right now using the current
     * tracking stack and before the final error handling is implemented. The
     * pitch / sqrt(12) will be added as measurement error estimation.  This is
     * of course not the exact error of the cluster and one cannot expect
     * perfect tracking results when using this constructor
     *
     * @param hit    PXDCluster to use as base
     */
    AlignablePXDRecoHit(const PXDCluster* hit, const genfit::TrackCandHit* trackCandHit = NULL) : PXDRecoHit(hit,
          trackCandHit) {setDetectorPlane();}

    /** Destructor. */
    virtual ~AlignablePXDRecoHit() {}

    genfit::AbsMeasurement* clone() const
    {
      return new AlignablePXDRecoHit(*this);
    }

    /**
     * @brief Set the aligned detector plane
     *
     * @return void
     */
    void setDetectorPlane();

    /**
     * @brief Labels for global derivatives
     *
     * @return Vector of int labels, same size as derivatives matrix #columns
     */
    virtual std::vector< int > labels();

    /**
     * @brief Get alignment derivatives
     *
     * @param sop Track state on sensor plane
     * @return TMatrixD
     */
    virtual TMatrixD derivatives(const genfit::StateOnPlane* sop);

  private:

    ClassDef(AlignablePXDRecoHit, 2); /**< PXD RecoHit extended for alignment/calibration */
  };
}
#endif