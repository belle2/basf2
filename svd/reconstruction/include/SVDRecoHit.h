/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter, Moritz Nadler            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDRECOHIT_H_
#define SVDRECOHIT_H_

#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>

// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <GFPlanarHitPolicy.h>
#include <GFRecoHitIfc.h>

namespace Belle2 {
  /**
   * SVDRecoHit - an extended form of SVDHit containing geometry information.
   *
   * To create a list of SVDRecoHits for all SVDTrueHits belonging to one MCParticle do something like:
   *
   * @code
   * //Set up random number generator if not already done
   * if(!gRandom) gRandom = new TRandom3(0);
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
  class SVDRecoHit: public GFRecoHitIfc<GFPlanarHitPolicy> {
  public:
    /** Default constructor for ROOT IO. */
    SVDRecoHit();

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
    SVDRecoHit(const SVDTrueHit* hit, bool uDirection, float sigma = -1);

    /** Construct SVDRecoHit from a SVD cluster
      * FIXME: Error handling not yet defined. For now the errors have to
      * be supplied on construction
      *
      * @param hit    SVDCluster to use as base
      * @param sigma  Error of the hit position
      */
    SVDRecoHit(const SVDCluster* hit, float sigma = -1);

    /** Destructor. */
    virtual ~SVDRecoHit() {}

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
    const SVDTrueHit* getTrueHit() const { return m_trueHit; }

    /** Get pointer to the Cluster used when creating this RecoHit, can be NULL if created from something else */
    const SVDCluster* getCluster() const { return m_cluster; }

    /** Is the coordinate u or v? */
    bool isU() const { return m_isU; }

    /** Get coordinate.*/
    float getPosition() const { return fHitCoord(0, 0); }

    /** Get coordinate variance */
    float getPositionVariance() const { return fHitCov(0, 0); }

    /** Get deposited energy. */
    float getEnergyDep() const { return m_energyDep; }

    /** Get rotation angle. */
    float getRotation() const { return m_rotationPhi; }

  private:

    enum { HIT_DIMENSIONS = 1 /**< sensitive Dimensions of the Hit */ };
    const static double c_HMatrixUContent[5]; /**< holds all elements of H Matrix for U coordinate cluster. A C array is the only possibility to set TMatrixD elements with its constructor*/
    const static TMatrixD c_HMatrixU; /**< H matrix needed for Genfit. getHMatrix will return this attribute if cluster is U coordinate*/

    const static double c_HMatrixVContent[5]; /**< holds all elements of H Matrix for V coordinate cluster. A C array is the only possibility to set TMatrixD elements with its constructor*/
    const static TMatrixD c_HMatrixV; /**< H matrix needed for Genfit. getHMatrix will return this attribute if cluster is V coordinate*/

    unsigned short m_sensorID; /**< Unique sensor identifier.*/
    const SVDTrueHit* m_trueHit; /**< Pointer to the Truehit used to generate this hit */
    const SVDCluster* m_cluster; /**< Pointer to the Cluster used to generate this hit */
    bool m_isU; /**< True if the hit has u-coordinate, false if v. */
    float m_energyDep; /**< deposited energy.*/
    //float m_energyDepError; /**< error in dep. energy.*/
    float m_rotationPhi; /**< angle of the plane rotation, for u in wedge sensors.*/

    /** Set up Detector plane information */
    void setDetectorPlane();

    ClassDef(SVDRecoHit, 1)
  };

} // namespace Belle2

#endif /* SVDRECOHIT_H_ */
