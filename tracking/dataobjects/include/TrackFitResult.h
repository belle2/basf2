/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <TVector3.h>

class GFAbsTrackRep;

#include <bitset>

namespace Belle2 {

  /** Values of the result of a track fit with a given particle hypothesis.
   */
  class TrackFitResult : public TObject {
  public:
    /** Constructor for I/O purpuses.
     */
    TrackFitResult();

    /** Constructor taking an object used during the fitting process.
     */
    TrackFitResult(const GFAbsTrackRep& gfAbsTrackRep);

    // This class should be able to give back Helix information either in Perigee Parametrisation
    // or as starting position + momentum.
    TVector3 getPosition();
    TVector3 getMomentum();
    TMatrixF getCovariance6();

    /* Is this necessary at this stage ?! or would above parametrization be just enough?
     * Internally it is definitivaly needed to reduce the space requirement, but it would be possible to
     * not hand this parametrization out.
     */
    float getD0()  const;
    float getPhi() const;
    float getOmega() const;
    float getZ0() const;
    float getCotTheta() const;
    TMatrixF getCovariance5();



    //---------------------------------------------------------------------------------------------------------------------------
    //--- Hit Pattern Arithmetics
    //---------------------------------------------------------------------------------------------------------------------------
    /** Number of PXD hits used in the TrackFitResult. */
    unsigned short getNPXDHits() const {
      return (m_hitPattern[0] + m_hitPattern[1]);
    }

    /** Number of SVD hits used in the TrackFitResult. */
    unsigned short getNSVDHits() const {
      return (m_hitPattern[2] + m_hitPattern[3] + m_hitPattern[4] + m_hitPattern[5]);
    }

    /** Number of CDC hits used in the TrackFitResult. */
    unsigned short getNCDCHits() const {
      return (m_hitPattern.count() - getNPXDHits() - getNSVDHits());
    }

    /** Was there a hit in the specified layer?
     *
     *  @param  iVXDLayer  layer for which the information is requested.
     */
    bool hitInVXDLayer(unsigned short iVXDLayer) const {
      return m_hitPattern[iVXDLayer];
    }

    /** Was there a hit in the specified layer?
     *
     *  @param  iCDCLayer  layer for which the information is requested.
     */
    bool hitInCDCLayer(unsigned short iCDCLayer) const {
      return m_hitPattern[iCDCLayer + 6];
    }

    /** Similar as above, but asking for any hit in the corresponding SuperLayer.*/
    bool hitInSuperLayer(unsigned int iSuperLayer) const;

    /** Were any Stereo hits in the CDC. */
    bool hitInStereoLayer() const {
      return (hitInSuperLayer(2) || hitInSuperLayer(4) || hitInSuperLayer(6) || hitInSuperLayer(8));
    }

    /** Returns the count of the innermost Layer. */
    unsigned short getIInnermostLayer() const;

    ///--------------------------------------------------------------------------------------------------------------------------
    /** Get back a ParticleCode of the hypothesis of the track fit. */
    short getPDGCode() const {
      return m_pdg;
    }

    /** Return track charge (1 or -1). */
    short getCharge() const {
      if (getOmega() >= 0)
        return 1;
      else
        return -1;
    }

    /** Getter for Chi2 Probability of the track fit. */
    float getPValue() const {
      return m_pValue;
    }

  private:
    /** PDG Code for hypothesis with which the corresponding fir was performed. */
    short m_pdg;

    /** Chi2 Probability of the fit. */
    float m_pValue;

    /** Hit Pattern of the corresponding Hit.
     *
     *  Bits 0-1:   PXD <br>
     *  Bits 2-5:   SVD <br>
     *  Bits 6-61:  CDC <br>
     *  Bits 62-63: unused.
     */
    std::bitset<64> m_hitPattern;

    //---------------------------------------------------------------------------------------------------------------------------
    //--- Internal Variables to store track parameters with uncertainties in perigee description of helix.
    //---------------------------------------------------------------------------------------------------------------------------
    float m_perigeeParams [5] ;
    float m_perigeeUncertainties [15];

    ClassDef(TrackFitResult, 0);
  };
}
