/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRECOHIT_H_
#define PXDRECOHIT_H_

#include <pxd/dataobjects/PXDHit.h>

// ROOT includes
#include <TMatrixD.h>

// GenFit includes
#include <GFPlanarHitPolicy.h>
#include <GFRecoHitIfc.h>


#define DEFAULT_PXDRECOHITS "PXDRecoHits"
#define DEFAULT_PXDRECOHITSREL "MCParticlesToPXDRecoHits"


namespace Belle2 {

  /**
   * PXDRecoHit - an extended form of PXDHit containing geometry information.
   */
  class PXDRecoHit: public GFRecoHitIfc<GFPlanarHitPolicy> {
  public:
    /**
     * Default constructor.
     */
    PXDRecoHit();

    /**
     * Useful constructor.
     */
    PXDRecoHit(const PXDHit* hit);

    /**
     * Destructor.
     */
    virtual ~PXDRecoHit() {
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
    int getSensorUID() const {
      return m_sensorUID;
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

    int m_sensorUID; /**< Unique sensor identifier.*/
    float m_energyDep; /**< deposited energy.*/
    float m_energyDepError; /**< error in dep. energy.*/

    ClassDef(PXDRecoHit, 1)

  };

} // namespace Belle2

#endif /* PXDRECOHIT_H_ */
