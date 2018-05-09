/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter, Moritz Nadler,           *
 *               Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>


// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <genfit/PlanarMeasurement.h>
#include <genfit/HMatrixUV.h>
#include <genfit/TrackCandHit.h>

namespace Belle2 {
  /**
   * PXDRecoHit - an extended form of PXDCluster containing geometry information.
   *
   * To create a list of PXDRecoHits for all PXDTrueHits belonging to one MCParticle do something like:
   *
   * @code
   * //Get the MCParticle in question
   * MCParticle* mcParticle = ...
   * //Assume some error on the position
   * float sigmaU = 10 * Unit::um;
   * float sigmaV = 15 * Unit::um;
   *
   * //Iterate over the relation and create a list of hits
   * vector<PXDRecoHit*> hits;
   * RelationIndex<MCParticle,PXDTrueHit> relMCTrueHit;
   * RelationIndex<MCParticle,PXDTrueHit>::range_from it = relMCTrueHit.getElementsFrom(mcParticle);
   * for(; it.first!=it.second; ++it.first){
   *   hits.push_back(new PXDRecoHit(it.first->to, sigmaU, sigmaV));
   * }
   * @endcode
   */
  class PXDRecoHit: public genfit::PlanarMeasurement {
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
    PXDRecoHit(const PXDTrueHit* hit, const genfit::TrackCandHit* trackCandHit = NULL, float sigmaU = -1, float sigmaV = -1);

    /** Construct PXDRecoHit from a PXD cluster.
     * For users that want to supply their own errors on construction
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
    PXDRecoHit(const PXDCluster* hit, const genfit::TrackCandHit* trackCandHit = NULL);

    /** Creating a deep copy of this hit. */
    genfit::AbsMeasurement* clone() const;

    /** Methods that actually interface to Genfit.  */
    /* This method allows to provide hit position dependent on track direction. */
    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const;

    /* This method allows to provide hit position dependent on track direction. */
    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state, int clusterkind,
        int shape_index, float eta) const;

    /** Get the compact ID.*/
    VxdID getSensorID() const { return m_sensorID; }

    /** Get pointer to the TrueHit used when creating this RecoHit, can be NULL if created from something else */
    const PXDTrueHit* getTrueHit() const { return m_trueHit; }
    /** Get pointer to the Cluster used when creating this RecoHit, can be NULL if created from something else */
    const PXDCluster* getCluster() const { return m_cluster; }

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

    /** Get the likelyhood that cluster shape is likely to be created from track state. */
    float getShapeLikelyhood(const genfit::StateOnPlane& state) const;

    /** Get the likelyhood that cluster shape is likely to be created from track state. */
    float getShapeLikelyhood(const genfit::StateOnPlane& state, int clusterkind, int shape_index) const;

    virtual const genfit::AbsHMatrix* constructHMatrix(const genfit::AbsTrackRep*) const { return new genfit::HMatrixUV(); };

  private:

    enum { HIT_DIMENSIONS = 2 /**< sensitive Dimensions of the Hit */ };

    unsigned short m_sensorID; /**< Unique sensor identifier.*/
    /** Pointer to the TrueHit used when creating this object */
    const PXDTrueHit* m_trueHit; //! transient member (not written out during streaming)
    /** Pointer to the Cluster used when creating this object */
    const PXDCluster* m_cluster; //! transient member (not written out during streaming)
    float m_energyDep; /**< deposited energy.*/
    //float m_energyDepError; /**< error in dep. energy.*/

    /** Set up Detector plane information */
    void setDetectorPlane();

    ClassDef(PXDRecoHit, 8)
  };

} // namespace Belle2
