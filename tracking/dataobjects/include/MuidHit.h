/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Store one muon-identification hit in the KLM as a ROOT object.
  class MuidHit : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    MuidHit();

    //! Constructor with initial values
    //! @param pdgCode PDG particleID hypothesis used in this extrapolation
    //! @param inBarrel flag to indicate if this hit is in the KLM barrel (true) or endcap (false)
    //! @param isForward flag to indicate if this hit is in the forward (true) or backward (false) end
    //! @param sector Sector number of this hit (1..8 for barrel, 1..4 for endcap)
    //! @param layer Layer number of this hit (1..15 for barrel, 1..14 for endcap)
    //! @param extPos Extrapolation position of this hit (cm)
    //! @param hitPos Matching reconstructed position of this hit (cm)
    //! @param extTime extrapolation time of this hit (ns)
    //! @param hitTime Measured time of this hit (ns)
    //! @param chiSq Chi-squared contribution of this hit
    MuidHit(int pdgCode, bool inBarrel, bool isForward, int sector, int layer, const TVector3& extPos, const TVector3& hitPos, double extTime, double hitTime, double chiSq);

    //! Destructor
    virtual ~MuidHit() {}

    //! @return PDG particleID hypothesis used in this extrapolation
    int getPdgCode() const { return m_PdgCode; }

    //! @return global-coordinate position of the extrapolated point (cm)
    TVector3 getExtPosition() const { return m_ExtPosition; }

    //! @return global-coordinate position of the matching KLM hit (cm)
    TVector3 getHitPosition() const { return m_HitPosition; }

    //! @return time since start of event of the extrapolated point (ns)
    double getExtTime() const { return m_ExtTime; }

    //! @return time of the matching KLM hit (ns)
    double getHitTime() const { return m_HitTime; }

    //! @return flag to indicate if hit is in barrel (true) or endcap (false)
    bool inBarrel() const { return m_InBarrel; }

    //! @return flag to indicate if hit is in forward (true) or backward (false) half
    bool isForward() const { return m_IsForward; }

    //! @return sector number (1..8 for barrel, 1..4 for endcap)
    int getSector() const { return m_Sector; }

    //! @return layer number (1..15 for barrel, 1..14 for endcap)
    int getLayer() const { return m_Layer; }

    //! @return chi-squared contribution of this hit for extrapolation point and KLM hit
    double getChiSquared() const { return m_ChiSquared; }

  private:

    //! PDG particleID code for the hypothesis used in this extrapolation
    int m_PdgCode;

    //! global-coordinate position of the extrapolated point (cm)
    TVector3 m_ExtPosition;

    //! time of the extrapolated point (ns)
    double m_ExtTime;

    //! global-coordinate position of the matching KLM hit (cm)
    TVector3 m_HitPosition;

    //! time of the matching KLM hit (ns)
    double m_HitTime;

    //! flag to indicate if hit is in barrel (true) or endcap (false)
    bool m_InBarrel;

    //! flag to indicate if hit is in forward (true) or backward (false) half
    bool m_IsForward;

    //! sector number (1..8 for barrel, 1..4 for endcap)
    int m_Sector;

    //! layer number (1..15 for barrel, 1..14 for endcap)
    int m_Layer;

    //! chi-squared contribution of this hit for extrapolation point and KLM hit
    double m_ChiSquared;

    //! Needed to make the ROOT object storable
    ClassDef(MuidHit, 1)

  };
}
