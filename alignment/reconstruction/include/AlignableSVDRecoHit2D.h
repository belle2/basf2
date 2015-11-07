/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, Tobias Schlüter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALIGNABLESVDRECOHIT2D_H
#define ALIGNABLESVDRECOHIT2D_H


#include <svd/reconstruction/SVDRecoHit2D.h>
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
  class AlignableSVDRecoHit2D : public SVDRecoHit2D, public genfit::ICalibrationParametersDerivatives  {
    friend class SVDRecoHit2D;
  public:
    /** Default constructor for ROOT IO. */
    AlignableSVDRecoHit2D() : SVDRecoHit2D() {}

    /** Construct SVDRecoHit from a SVDTrueHit for Monte Carlo based tracking.
     * This requires a valid random number generator to be initialized at
     * gRandom. The Hit position will be smeared using a gaussian smearing with
     * sigmaU and sigmaV along u and v respectively
     *
     * If one of the errors is set <0, a default resolution will be
     * assumed for both values by dividing the pixel size by sqrt(12).
     *
     * @param hit    SVDTrueHit to use as base
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     */
    AlignableSVDRecoHit2D(const SVDTrueHit* hit, const genfit::TrackCandHit* trackCandHit = NULL, float sigmaU = -1,
                          float sigmaV = -1) : SVDRecoHit2D(hit, trackCandHit, sigmaU, sigmaV) {setDetectorPlane();}


    /** Construct SVDRecoHit directly from vxdid and coordinates without using a hit object from the storearray
     * in the constructor
     *
     * @param vxdid  the vxd id so the reco hit knows on which sensor the measurment took place
     * @param u coordinate of the Hit along u
     * @param v coordinate of the Hit along v
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     */
    AlignableSVDRecoHit2D(VxdID::baseType vxdid, const double u, const double v, double sigmaU = -1,
                          double sigmaV = -1) : SVDRecoHit2D(vxdid, u, v, sigmaU, sigmaV) {setDetectorPlane();}

    /** Construct SVDRecoHit from a u- and v- SVDCluster.
     * @param uHit the u-type SVDCluster
     * @param vHit the v-type SVDCluster
     */
    AlignableSVDRecoHit2D(const SVDCluster& uHit, const SVDCluster& vHit) : SVDRecoHit2D(uHit, vHit) {setDetectorPlane();}

    /** Construct SVDRecoHit from a u- and v- (1D) SVDRecoHit.
     * @param uRecoHit the u-type SVDRecoHit
     * @param vRecoHit the v-type SVDRecoHit
     */
    AlignableSVDRecoHit2D(const SVDRecoHit& uRecoHit, const SVDRecoHit& vRecoHit) : SVDRecoHit2D(uRecoHit, vRecoHit) {setDetectorPlane();}


    /** Destructor. */
    virtual ~AlignableSVDRecoHit2D() {}

    genfit::AbsMeasurement* clone() const
    {
      return new AlignableSVDRecoHit2D(*this);
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

    /** ROOT Macro.*/
    ClassDef(AlignableSVDRecoHit2D, 3);
  };
}
#endif