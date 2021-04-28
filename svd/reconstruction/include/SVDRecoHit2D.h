/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter, Moritz Nadler            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDRECOHIT2D_H_
#define SVDRECOHIT2D_H_

#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/reconstruction/SVDRecoHit.h>

// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <genfit/PlanarMeasurement.h>
#include <genfit/TrackCandHit.h>

namespace Belle2 {
  /**
   * SVDRecoHit - an extended form of SVDHit containing geometry information.
   *
   * To create a list of SVDRecoHits for all SVDTrueHits belonging to one MCParticle do something like:
   *
   * @code
   * //Get the MCParticle in question
   * MCParticle* mcParticle = ...
   * //Iterate over the relation and create a list of hits
   * vector<SVDRecoHit*> hits;
   * RelationIndex<MCParticle,SVDTrueHit> relMCTrueHit;
   * RelationIndex<MCParticle,SVDTrueHit>::range_from it = relMCTrueHit.getElementsFrom(mcParticle);
   * for(; it.first!=it.second; ++it.first){
   *   hits.push_back(new SVDRecoHit(it.first->to));
   * }
   * @endcode
   */
  class SVDRecoHit2D: public genfit::PlanarMeasurement {
  public:
    /** Default constructor for ROOT IO. */
    SVDRecoHit2D();

    /** Construct SVDRecoHit from a SVDTrueHit for Monte Carlo based tracking.
     * This requires a valid random number generator to be initialized at
     * gRandom. The Hit position will be smeared using a gaussian smearing with
     * sigmaU and sigmaV along u and v respectively
     *
     * If one of the errors is set <0, a default resolution will be
     * assumed for both values by dividing the pixel size by sqrt(12).
     *
     * @param hit    SVDTrueHit to use as base
     * @param trackCandHit
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     */
    explicit SVDRecoHit2D(const SVDTrueHit* hit, const genfit::TrackCandHit* trackCandHit = nullptr, float sigmaU = -1,
                          float sigmaV = -1);


    /** Construct SVDRecoHit directly from vxdid and coordinates without using a hit object from the storearray
     * in the constructor
     *
     * @param vxdid  the vxd id so the reco hit knows on which sensor the measurment took place
     * @param u coordinate of the Hit along u
     * @param v coordinate of the Hit along v
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     */
    SVDRecoHit2D(VxdID::baseType vxdid, const double u, const double v, double sigmaU = -1, double sigmaV = -1);

    /** Construct SVDRecoHit from a u- and v- SVDCluster.
     * @param uHit the u-type SVDCluster
     * @param vHit the v-type SVDCluster
     */
    SVDRecoHit2D(const SVDCluster& uHit, const SVDCluster& vHit);

    /** Construct SVDRecoHit from a u- and v- (1D) SVDRecoHit.
     * @param uRecoHit the u-type SVDRecoHit
     * @param vRecoHit the v-type SVDRecoHit
     */
    SVDRecoHit2D(const SVDRecoHit& uRecoHit, const SVDRecoHit& vRecoHit);

    /** Destructor. */
    virtual ~SVDRecoHit2D() {}

    /** Creating a deep copy of this hit.
     * Overrides the method inherited from GFRecoHit.
     */
    genfit::AbsMeasurement* clone() const override;

    /** Get the compact ID.*/
    VxdID getSensorID() const { return m_sensorID; }

    /** Get pointer to the TrueHit used when creating this RecoHit, can be nullptr if created from something else */
    const SVDTrueHit* getTrueHit() const { return m_trueHit; }
    /** Get pointer to the u cluster used to create this RecoHit. */
    const SVDCluster* getUCluster() const {return m_uCluster; }
    /** Get pointer to the u cluster used to create this RecoHit. */
    const SVDCluster* getVCluster() const {return m_vCluster; }

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

    /** Methods that actually interface to Genfit.  */
    //virtual genfit::SharedPlanePtr constructPlane(const genfit::StateOnPlane&) const;
    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const override;

  private:

    enum { HIT_DIMENSIONS = 2 /**< sensitive Dimensions of the Hit */ };

    unsigned short m_sensorID; /**< Unique sensor identifier.*/
    const SVDTrueHit* m_trueHit; /**< Pointer to the Truehit used to generate this hit */
    const SVDCluster* m_uCluster; /**< Pointer to mother uCluster. */
    const SVDCluster* m_vCluster; /**< Pointer to mother vCluster. */

    float m_energyDep; /**< deposited energy.*/
    //float m_energyDepError; /**< error in dep. energy.*/

    /** Set up Detector plane information */
    void setDetectorPlane();

    /** Apply planar deformation of sensors */
    TVectorD applyPlanarDeformation(TVectorD rawHit, std::vector<double> planarParameters, const genfit::StateOnPlane& state) const;

    ClassDefOverride(SVDRecoHit2D, 7)
  };

} // namespace Belle2

#endif /* SVDRECOHIT_H_ */
