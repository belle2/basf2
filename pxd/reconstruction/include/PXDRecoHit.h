/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter, Moritz Nadler            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRECOHIT_H_
#define PXDRECOHIT_H_

#include <vxd/dataobjects/VxdID.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <vxd/dataobjects/VXDSimpleDigiHit.h>


// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <GFPlanarHitPolicy.h>
#include <GFRecoHitIfc.h>

namespace Belle2 {
  /**
   * PXDRecoHit - an extended form of PXDHit containing geometry information.
   *
   * To create a list of PXDRecoHits for all PXDTrueHits belonging to one MCParticle do something like:
   *
   * @code
   * //Set up random number generator if not already done
   * if(!gRandom) gRandom = new TRandom3(0);
   * //Get the MCParticle in question
   * MCParticle* mcParticle = ...
   * //Assume some error on the position
   * float sigmaU = 10 * Unit::um;
   * float sigmaV = 15 * Unit::um;
   *
   * //Iterate over the relation and create a list of hits
   * vector<PXDRecoHit*> hits;
   * RelationIndex<MCParticle,PXDTrueHit> relMCTrueHit;
   * RelationIndex<MCParticle,PXDTrueHit>::range_from it = relMCTrueHit.getFrom(mcParticle);
   * for(; it.first!=it.second; ++it.first){
   *   hits.push_back(new PXDRecoHit(it.first->to, sigmaU, sigmaV));
   * }
   * @endcode
   */
  class PXDRecoHit: public GFRecoHitIfc<GFPlanarHitPolicy> {
  public:
    /** Default constructor for ROOT IO. */
    PXDRecoHit();

    /** Construct PXDRecoHit from a PXDTrueHit for Monte Carlo based tracking.
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
    PXDRecoHit(const PXDTrueHit* hit, float sigmaU = -1, float sigmaV = -1);

    /** Construct PXDRecoHit from a PXD cluster
     * FIXME: Error handling not yet defined. For now the errors have to
     * be supplied on construction
     *
     * @param hit    PXDCluster to use as base
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     * @param covUV  Covariance between u and v
     */
    PXDRecoHit(const PXDCluster* hit, float sigmaU, float sigmaV, float covUV);

    /** Construct PXDRecoHit from a PXD cluster
     * This constructor is intended as a temporary solution for people who want
     * to test the impact of realistic clusters right now using the current
     * tracking stack and before the final error handling is implemented. The
     * pitch / sqrt(12) will be added as measurement error estimation.  This is
     * of course not the exact error of the cluster and one cannot expect
     * perfect tracking results when using this constructor
     *
     * @param hit    PXDCluster to use as base
     */
    PXDRecoHit(const PXDCluster* hit);

    /** Construct PXDRecoHit from a VXDSimpleDigiHit
     * The VXDSimpleDigiHit is used by the VXDSimpleBackground module which
     * in turn is used to created artificial background in the VXD to test the
     * correctness and performance of tracking algorithms
     *
     * @param hit    VXDSimpleDigiHit to use as base
     */
    PXDRecoHit(const VXDSimpleDigiHit* hit);

    /** Destructor. */
    virtual ~PXDRecoHit() {}

    /** Creating a deep copy of this hit.
     * Overrides the method inherited from GFRecoHit.
     */
    GFAbsRecoHit* clone();

    /** Projection for the hit ...
     * Overrides the method inherited from GFRecoHit.
     */
    TMatrixD getHMatrix(const GFAbsTrackRep* stateVector);

    /** Get the compact ID.*/
    VxdID getSensorID() const { return m_sensorID; }

    /** Get pointer to the TrueHit used when creating this RecoHit, can be NULL if created from something else */
    const PXDTrueHit* getTrueHit() const { return m_trueHit; }
    /** Get pointer to the Cluster used when creating this RecoHit, can be NULL if created from something else */
    const PXDCluster* getCluster() const { return m_cluster; }
    /** Get pointer to the VXDSimpleDigiHit used when creating this RecoHit, can be NULL if created from something else */
    const VXDSimpleDigiHit* getSimpleDigiHit() const { return m_vxdSimpleDigiHit; }
    /** Get u coordinate.*/
    float getU() const { return fHitCoord(0, 0); }
    /** Get v coordinate.*/
    float getV() const { return fHitCoord(1, 0); }

    /** Get u coordinate variance */
    float getUVariance() const { return fHitCov(0, 0); }
    /** Get v coordinate variance */
    float getVVariance() const { return fHitCov(1, 1); }
    /** Get u-v error covariance.*/
    float getUVCov() const { return fHitCov(0, 1); }

    /** Get deposited energy. */
    float getEnergyDep() const { return m_energyDep; }

    /** Get deposited energy error. */
    //float getEnergyDepError() const { return m_energyDepError; }

  private:

    enum { HIT_DIMENSIONS = 2 /**< sensitive Dimensions of the Hit */ };
    const static double c_HMatrixContent[10]; /**< holds all elements of H Matrix. A C array is the only possibility to set TMatrixD elements with its constructor*/
    const static TMatrixD c_HMatrix; /**< H matrix needed for Genfit. getHMatrix will return this attribute*/

    unsigned short m_sensorID; /**< Unique sensor identifier.*/
    const PXDTrueHit* m_trueHit; /**< Pointer to the TrueHit used when creating this object */
    const PXDCluster* m_cluster; /**< Pointer to the Cluster used when creating this object */
    const VXDSimpleDigiHit* m_vxdSimpleDigiHit; /**< Pointer to the VXDSimpleDigiHit used when creating this object */
    float m_energyDep; /**< deposited energy.*/
    //float m_energyDepError; /**< error in dep. energy.*/

    /** Set up Detector plane information */
    void setDetectorPlane();

    ClassDef(PXDRecoHit, 2)
  };

} // namespace Belle2

#endif /* PXDRECOHIT_H_ */
