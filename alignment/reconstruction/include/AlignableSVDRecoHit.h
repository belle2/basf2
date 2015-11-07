/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, Tobias Schlüter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALIGNABLESVDRECOHIT_H
#define ALIGNABLESVDRECOHIT_H


#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <genfit/AbsMeasurement.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/TrackCandHit.h>
#include <genfit/HMatrixU.h>

#include <genfit/ICalibrationParametersDerivatives.h>

#include <memory>


namespace Belle2 {
  /** This class is used to transfer SVD information to the track fit. */
  class AlignableSVDRecoHit : public SVDRecoHit, public genfit::ICalibrationParametersDerivatives  {
    friend class SVDRecoHit;
  public:
    /** Default constructor for ROOT IO. */
    AlignableSVDRecoHit() : SVDRecoHit() {}

    /** Construct SVDRecoHit from a SVDTrueHit for Monte Carlo based tracking.
     * This requires a valid random number generator to be initialized at
     * gRandom. The Hit position will be smeared using a gaussian smearing with
     * sigma along u or v, depending on uDirection.
     *
     * If the error is set to <0, the resolution will be set to pitch/sqrt(12).
     *
     * @param hit    SVDTrueHit to use as base
     * @param uDirection If true, the hit has u coordinate, otherwise v
     * @param sigma Error of the hit coordinate
     */
    AlignableSVDRecoHit(const SVDTrueHit* hit, bool uDirection, float sigma = -1) : SVDRecoHit(hit, uDirection, sigma) {setDetectorPlane();}

    /** Construct AlignableSVDRecoHit from a SVD cluster
     * This constructor is intended as a temporary solution for people who want
     * to test the impact of realistic clusters right now using the current
     * tracking stack and before the final error handling is implemented. The
     * pitch / sqrt(12) will be added as measurement error estimation.  This is
     * of course not the exact error of the cluster and one cannot expect
     * perfect tracking results when using this constructor
     *
     * @param hit    SVDCluster to use as base
     */
    AlignableSVDRecoHit(const SVDCluster* hit, const genfit::TrackCandHit* trackCandHit = NULL) : SVDRecoHit(hit,
          trackCandHit) {setDetectorPlane();}

    /** Destructor. */
    virtual ~AlignableSVDRecoHit() {}

    genfit::AbsMeasurement* clone() const
    {
      return new AlignableSVDRecoHit(*this);
    }
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

    /**
     * @brief Set the aligned detector plane
     *
     * @return void
     */
    void setDetectorPlane();

  private:
    static std::string m_misalignmentDBObjPtrName;

    /** ROOT Macro.*/
    ClassDef(AlignableSVDRecoHit, 1);
  };
}
#endif