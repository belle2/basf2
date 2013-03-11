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
#include <vxd/dataobjects/VXDSimpleDigiHit.h>

// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <RecoHits/GFAbsPlanarHit.h>

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
  class SVDRecoHit2D: public GFAbsPlanarHit {
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
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     */
    SVDRecoHit2D(const SVDTrueHit* hit, float sigmaU = -1, float sigmaV = -1);


    /** Construct SVDRecoHit directly from vxdid and coordinates without using a hit object from the storearray
     * in the constructor
     *
     * @param vxdid  the vxd id so the reco hit knows on which sensor the measurment took place
     * @param u coordinate of the Hit along u
     * @param v coordinate of the Hit along v
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     */
    SVDRecoHit2D(const VxdID vxdid, const double u, const double v, double sigmaU = -1, double sigmaV = -1);

    /** Construct SVDRecoHit2D from a VXDSimpleDigiHit
     * The VXDSimpleDigiHit is used by the VXDSimpleBackground module which
     * in turn is used to created artificial background in the VXD to test the
     * correctness and performance of tracking algorithms
     *
     * @param hit    VXDSimpleDigiHit to use as base
     */
    SVDRecoHit2D(const VXDSimpleDigiHit* hit);

    /** Destructor. */
    virtual ~SVDRecoHit2D() {}

    /** Creating a deep copy of this hit.
     * Overrides the method inherited from GFRecoHit.
     */
    GFAbsRecoHit* clone();

    /** Projection for the hit ...
     * Overrides the method inherited from GFRecoHit.
     */
    const TMatrixD& getHMatrix(const GFAbsTrackRep* stateVector);

    /** Get the compact ID.*/
    VxdID getSensorID() const { return m_sensorID; }

    /** Get pointer to the TrueHit used when creating this RecoHit, can be NULL if created from something else */
    const SVDTrueHit* getTrueHit() const { return m_trueHit; }
    /** Get pointer to the VXDSimpleDigiHit used when creating this RecoHit, can be NULL if created from something else */
    const VXDSimpleDigiHit* getSimpleDigiHit() const { return m_vxdSimpleDigiHit; }
    /** Get u coordinate.*/
    float getU() const { return fHitCoord(0); }
    /** Get v coordinate.*/
    float getV() const { return fHitCoord(1); }

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
    const SVDTrueHit* m_trueHit; /**< Pointer to the Truehit used to generate this hit */
    const VXDSimpleDigiHit* m_vxdSimpleDigiHit; /**< Pointer to the VXDSimpleDigiHit used when creating this object */

    float m_energyDep; /**< deposited energy.*/
    //float m_energyDepError; /**< error in dep. energy.*/

    /** Set up Detector plane information */
    void setDetectorPlane();

    ClassDef(SVDRecoHit2D, 3)
  };

} // namespace Belle2

#endif /* SVDRECOHIT_H_ */
