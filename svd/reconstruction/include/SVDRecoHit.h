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
#include <genfit/PlanarMeasurement.h>
#include <genfit/HMatrixU.h>
#include <genfit/HMatrixV.h>
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
  class SVDRecoHit: public genfit::PlanarMeasurement {
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
      *
      * @param hit    SVDCluster to use as base.
      * FIXME: Parameter sigma is no longer used and will be removed.
     */
    SVDRecoHit(const SVDCluster* hit, const genfit::TrackCandHit* trackCandHit = NULL);

    /** Destructor. */
    virtual ~SVDRecoHit() {}

    /** Creating a deep copy of this hit.
     * Overrides the method inherited from GFRecoHit.
     */
    genfit::AbsMeasurement* clone() const;

    /** Get the compact ID.*/
    VxdID getSensorID() const { return m_sensorID; }

    /** Get pointer to the TrueHit used when creating this RecoHit, can be NULL if created from something else */
    const SVDTrueHit* getTrueHit() const { return m_trueHit; }

    /** Get pointer to the Cluster used when creating this RecoHit, can be NULL if created from something else */
    const SVDCluster* getCluster() const { return m_cluster; }

    /** Is the coordinate u or v? */
    bool isU() const { return m_isU; }

    /** Get coordinate.*/
    float getPosition() const { return rawHitCoords_(0); }

    /** Get coordinate variance */
    float getPositionVariance() const { return rawHitCov_(0, 0); }

    /** Get deposited energy. */
    float getEnergyDep() const { return m_energyDep; }

    /** Get rotation angle. */
    float getRotation() const { return m_rotationPhi; }

    /** Methods that actually interface to Genfit.  */
    //virtual genfit::SharedPlanePtr constructPlane(const genfit::StateOnPlane&) const;
    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const;

    // TODO: use HMatrixPhi for wedge sensors instead of rotating the plane!
    virtual const genfit::AbsHMatrix* constructHMatrix(const genfit::AbsTrackRep*) const { if (m_isU) return new genfit::HMatrixU(); else return new genfit::HMatrixV(); }

  private:

    enum { HIT_DIMENSIONS = 1 /**< sensitive Dimensions of the Hit */ };

    unsigned short m_sensorID; /**< Unique sensor identifier.*/
    const SVDTrueHit* m_trueHit; /**< Pointer to the Truehit used to generate this hit */
    const SVDCluster* m_cluster; /**< Pointer to the Cluster used to generate this hit */
    bool m_isU; /**< True if the hit has u-coordinate, false if v. */
    float m_energyDep; /**< deposited energy.*/
    //float m_energyDepError; /**< error in dep. energy.*/
    float m_rotationPhi; /**< angle of the plane rotation, for u in wedge sensors.*/

    /** Set up Detector plane information */
    void setDetectorPlane();

    ClassDef(SVDRecoHit, 4)
  };

} // namespace Belle2

#endif /* SVDRECOHIT_H_ */
