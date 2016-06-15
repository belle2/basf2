/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter, Moritz Nadler            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TELRECOHIT_H_
#define TELRECOHIT_H_

#include <vxd/dataobjects/VxdID.h>
#include <testbeam/vxd/dataobjects/TelTrueHit.h>
#include <testbeam/vxd/dataobjects/TelCluster.h>


// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <genfit/PlanarMeasurement.h>
#include <genfit/HMatrixUV.h>
#include <genfit/TrackCandHit.h>
#include <genfit/ICalibrationParametersDerivatives.h>

namespace Belle2 {
  /**
   * TelRecoHit - an extended form of Telescope hit containing geometry information.
   *
   * To create a list of TelRecoHits for all TelTrueHits belonging to one MCParticle do something like:
   *
   * @code
   * //Get the MCParticle in question
   * MCParticle* mcParticle = ...
   * //Assume some error on the position
   * float sigmaU = 10 * Unit::um;
   * float sigmaV = 15 * Unit::um;
   *
   * //Iterate over the relation and create a list of hits
   * vector<TelRecoHit*> hits;
   * RelationIndex<MCParticle,TelTrueHit> relMCTrueHit;
   * RelationIndex<MCParticle,TelTrueHit>::range_from it = relMCTrueHit.getElementsFrom(mcParticle);
   * for(; it.first!=it.second; ++it.first){
   *   hits.push_back(new TelRecoHit(it.first->to, sigmaU, sigmaV));
   * }
   * @endcode
   */
  class TelRecoHit: public genfit::PlanarMeasurement, public genfit::ICalibrationParametersDerivatives {
  public:
    /** Default constructor for ROOT IO. */
    TelRecoHit();

    /** Construct TelRecoHit from a TelTrueHit for Monte Carlo based tracking.
     * This requires a valid random number generator to be initialized at
     * gRandom. The Hit position will be smeared using a gaussian smearing with
     * sigmaU and sigmaV along u and v respectively
     *
     * If one of the errors is set <0, a default resolution will be
     * assumed for both values by dividing the pixel size by sqrt(12).
     *
     * @param hit    TelTrueHit to use as base
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     */
    TelRecoHit(const TelTrueHit* hit, const genfit::TrackCandHit* trackCandHit = NULL, float sigmaU = -1, float sigmaV = -1);

    /** Construct TelRecoHit from a Tel cluster.
     * For users that want to supply their own errors on construction
     *
     * @param hit    TelCluster to use as base
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     * @param covUV  Covariance between u and v
     */
    TelRecoHit(const TelCluster* hit, float sigmaU, float sigmaV, float covUV);

    /** Construct TelRecoHit from a Tel cluster
     * This constructor is intended as a temporary solution for people who want
     * to test the impact of realistic clusters right now using the current
     * tracking stack and before the final error handling is implemented. The
     * pitch / sqrt(12) will be added as measurement error estimation.  This is
     * of course not the exact error of the cluster and one cannot expect
     * perfect tracking results when using this constructor
     *
     * @param hit    TelCluster to use as base
     */
    TelRecoHit(const TelCluster* hit, const genfit::TrackCandHit* trackCandHit = NULL);

    /** Destructor. */
    virtual ~TelRecoHit() {}

    /** Creating a deep copy of this hit. */
    genfit::AbsMeasurement* clone() const;

    /** Methods that actually interface to Genfit.  */
    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const;

    /** Get the compact ID.*/
    VxdID getSensorID() const { return m_sensorID; }

    /** Get pointer to the TrueHit used when creating this RecoHit, can be NULL if created from something else */
    const TelTrueHit* getTrueHit() const { return m_trueHit; }
    /** Get pointer to the Cluster used when creating this RecoHit, can be NULL if created from something else */
    const TelCluster* getCluster() const { return m_cluster; }

    /** Get u coordinate.*/
    float getU() const { return rawHitCoords_(0); }
    /** Get v coordinate.*/
    float getV() const { return rawHitCoords_(1); }

    /** Get u coordinate variance */
    float getUVariance() const { return rawHitCov_(0, 0); }
    /** Get v coordinate variance */
    float getVVariance() const { return rawHitCov_(1, 1); }
    /** Get u-v error covariance.*/
    float getUVCov() const { return rawHitCov_(0, 1); }

    /** Get deposited energy. */
    float getEnergyDep() const { return m_energyDep; }

    /** Get deposited energy error. */
    //float getEnergyDepError() const { return m_energyDepError; }

    virtual const genfit::AbsHMatrix* constructHMatrix(const genfit::AbsTrackRep*) const { return new genfit::HMatrixUV(); };

    /**
     * @brief Derivatives for alignment parameters
     *
     * @param sop Predicted track state on plane
     * @return TMatrixD
     */
    TMatrixD derivatives(const genfit::StateOnPlane* sop);

    /**
     * @brief Labels for alignment parameters
     *
     * @return std::vector< int, std::allocator< void > >
     */
    std::vector< int > labels();

  private:

    enum { HIT_DIMENSIONS = 2 /**< sensitive Dimensions of the Hit */ };

    unsigned short m_sensorID; /**< Unique sensor identifier.*/
    const TelTrueHit* m_trueHit; /**< Pointer to the TrueHit used when creating this object */
    const TelCluster* m_cluster; /**< Pointer to the Cluster used when creating this object */
    float m_energyDep; /**< deposited energy.*/
    //float m_energyDepError; /**< error in dep. energy.*/

    /** Set up Detector plane information */
    void setDetectorPlane();

    ClassDef(TelRecoHit, 4)
  };

} // namespace Belle2

#endif /* TELRECOHIT_H_ */
