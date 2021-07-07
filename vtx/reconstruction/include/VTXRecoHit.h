/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <vtx/dataobjects/VTXTrueHit.h>
#include <vtx/dataobjects/VTXCluster.h>

// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <genfit/PlanarMeasurement.h>
#include <genfit/HMatrixUV.h>
#include <genfit/TrackCandHit.h>

namespace Belle2 {
  /**
   * VTXRecoHit - an extended form of VTXCluster containing geometry information.
   *
   * To create a list of VTXRecoHits for all VTXTrueHits belonging to one MCParticle do something like:
   *
   * @code
   * //Get the MCParticle in question
   * MCParticle* mcParticle = ...
   * //Assume some error on the position
   * float sigmaU = 10 * Unit::um;
   * float sigmaV = 15 * Unit::um;
   *
   * //Iterate over the relation and create a list of hits
   * vector<VTXRecoHit*> hits;
   * RelationIndex<MCParticle,VTXTrueHit> relMCTrueHit;
   * RelationIndex<MCParticle,VTXTrueHit>::range_from it = relMCTrueHit.getElementsFrom(mcParticle);
   * for(; it.first!=it.second; ++it.first){
   *   hits.push_back(new VTXRecoHit(it.first->to, sigmaU, sigmaV));
   * }
   * @endcode
   */
  class VTXRecoHit: public genfit::PlanarMeasurement {
  public:

    /** Default constructor for ROOT IO. */
    VTXRecoHit();

    /** Construct VTXRecoHit from a VTXTrueHit for Monte Carlo based tracking.
     * This requires a valid random number generator to be initialized at
     * gRandom. The Hit position will be smeared using a gaussian smearing with
     * sigmaU and sigmaV along u and v respectively
     *
     * If one of the errors is set <0, a default resolution will be
     * assumed for both values by dividing the pixel size by sqrt(12).
     *
     * @param hit    VTXTrueHit to use as base
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     */
    explicit VTXRecoHit(const VTXTrueHit* hit, const genfit::TrackCandHit* trackCandHit = nullptr, float sigmaU = -1,
                        float sigmaV = -1);

    /** Construct VTXRecoHit from a VTX cluster.
     * For users that want to supply their own errors on construction
     *
     * @param hit    VTXCluster to use as base
     * @param sigmaU Error of the Hit along u
     * @param sigmaV Error of the Hit along v
     * @param covUV  Covariance between u and v
     */
    VTXRecoHit(const VTXCluster* hit, float sigmaU, float sigmaV, float covUV);

    /** Construct VTXRecoHit from a VTX cluster
     * This constructor is intended as a temporary solution for people who want
     * to test the impact of realistic clusters right now using the current
     * tracking stack and before the final error handling is implemented. The
     * pitch / sqrt(12) will be added as measurement error estimation.  This is
     * of course not the exact error of the cluster and one cannot expect
     * perfect tracking results when using this constructor
     *
     * @param hit    VTXCluster to use as base
     */
    explicit VTXRecoHit(const VTXCluster* hit, const genfit::TrackCandHit* trackCandHit = nullptr);

    /** Creating a deep copy of this hit. */
    genfit::AbsMeasurement* clone() const override;

    /** Methods that actually interface to Genfit.  */
    /* This method allows to provide hit position dependent on track direction. */
    virtual std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const override;

    /** Get the compact ID.*/
    VxdID getSensorID() const { return m_sensorID; }

    /** Get pointer to the TrueHit used when creating this RecoHit, can be NULL if created from something else */
    const VTXTrueHit* getTrueHit() const { return m_trueHit; }
    /** Get pointer to the Cluster used when creating this RecoHit, can be NULL if created from something else */
    const VTXCluster* getCluster() const { return m_cluster; }

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

    virtual const genfit::AbsHMatrix* constructHMatrix(const genfit::AbsTrackRep*) const override { return new genfit::HMatrixUV(); };

  private:

    enum { HIT_DIMENSIONS = 2 /**< sensitive Dimensions of the Hit */ };

    unsigned short m_sensorID; /**< Unique sensor identifier.*/
    /** Pointer to the TrueHit used when creating this object */
    const VTXTrueHit* m_trueHit; //! transient member (not written out during streaming)
    /** Pointer to the Cluster used when creating this object */
    const VTXCluster* m_cluster; //! transient member (not written out during streaming)
    float m_energyDep; /**< deposited energy.*/
    //float m_energyDepError; /**< error in dep. energy.*/

    /** Set up Detector plane information */
    void setDetectorPlane();

    /** Apply planar deformation of sensors*/
    TVectorD applyPlanarDeformation(TVectorD hitCoords, std::vector<double> planarParameters, const genfit::StateOnPlane& state) const;

    ClassDefOverride(VTXRecoHit, 1)
  };

} // namespace Belle2
