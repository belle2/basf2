/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDRECOHIT_H_
#define SVDRECOHIT_H_

#include <svd/dataobjects/SVDHit.h>

// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <GFPlanarHitPolicy.h>
#include <GFRecoHitIfc.h>


#define DEFAULT_SVDRECOHITS "SVDRecoHits"
#define DEFAULT_SVDRECOHITSREL "MCParticlesToSVDRecoHits"


namespace Belle2 {

  /**
   * SVDRecoHit - an extended form of SVDHit containing geometry information.
   */
  class SVDRecoHit: public GFRecoHitIfc<GFPlanarHitPolicy> {
  public:
    /**
     * Default constructor.
     */
    SVDRecoHit();

    /**
     * Useful constructor.
     * @param hit Pointer to a prototype SVDHit.
     */
    SVDRecoHit(const SVDHit* hit);

    /**
     * Destructor.
     */
    virtual ~SVDRecoHit() {
      ;
    }

    /** Creating a deep copy of this hit.
     *
     * Overrides the method inherited from GFRecoHit.
     */
    GFAbsRecoHit* clone();

    /** Projection for the hit ...
     *
     * Overrides the method inherited from GFRecoHit.
     */
    TMatrixD getHMatrix(const GFAbsTrackRep* stateVector);

    /// A dozen getters...

    /** Get the compact ID.*/
    int getSensorUniID() const {
      return m_sensorUniID;
    }

    /** Get u coordinate.*/
    float getU() const {
      return fHitCoord(0, 0);
    }

    /** Get u coordinate error.*/
    float getUError() const {
      return fHitCov(0, 0);
    }

    /** Get v coordinate.*/
    float getV() const {
      return fHitCoord(1, 0);
    }

    /** Get v coordinate error.*/
    float getVError() const {
      return fHitCov(1, 1);
    }

    /** Get u-v error covariance.*/
    float getUVCov() const {
      return fHitCov(0, 1);
    }

    /** Get deposited energy. */
    float getEnergyDep() const {
      return m_energyDep;
    }

    /** Get deposited energy error. */
    float getEnergyDepError() const {
      return m_energyDepError;
    }

  private:

    static const int m_nParHitRep = 2; /**< Dimension parameter for GenFit.*/

    int m_sensorUniID; /**< Unique sensor identifier.*/
    float m_energyDep; /**< deposited energy.*/
    float m_energyDepError; /**< error in dep. energy.*/

    ClassDef(SVDRecoHit, 1)

  };

} // namespace Belle2

#endif /* SVDRECOHIT_H_ */
