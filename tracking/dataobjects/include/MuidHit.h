/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MUIDHIT_H
#define MUIDHIT_H

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Store one muon-identification hit in the KLM as a ROOT object.
  class MuidHit : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    MuidHit();

    //! Constructor with initial values used by Muid module
    MuidHit(int, bool, bool, int, int, const TVector3&, const TVector3&, double, double, double);

    //! Destructor
    virtual ~MuidHit() {}

    //! returns the PDG particleID hypothesis used in this extrapolation
    int getPdgCode() const { return m_PdgCode; }

    //! returns position of the extrapolated point (cm) in global coordinates
    TVector3 getExtPosition() const { return m_ExtPosition; }

    //! returns position of the matching KLM hit (cm) in global coordinates
    TVector3 getHitPosition() const { return m_HitPosition; }

    //! returns time of the extrapolated point (ns)
    double getExtTime() const { return m_ExtTime; }

    //! returns time of the matching KLM hit (ns)
    double getHitTime() const { return m_HitTime; }

    //! returns flag to indicate if hit is in barrel (true) or endcap (false)
    bool inBarrel() const { return m_InBarrel; }

    //! returns flag to indicate if hit is in forward (true) or backward (false) half
    bool isForward() const { return m_IsForward; }

    //! returns sector number (1..8 for barrel, 1..4 for endcap)
    int getSector() const { return m_Sector; }

    //! returns layer number (1..15 for barrel, 1..14 for endcap)
    int getLayer() const { return m_Layer; }

    //! returns chi-squared contribution for extrapolation point and KLM hit
    double getChiSquared() const { return m_ChiSquared; }

  private:

    //! PDG particleID code for the hypothesis used in this extrapolation
    int m_PdgCode;

    //! position of the extrapolated point (cm) in global coordinates
    TVector3 m_ExtPosition;

    //! time of the extrapolated point (ns)
    double m_ExtTime;

    //! position of the matching KLM hit (cm) in global coordinates
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

    //! chi-squared contribution for extrapolation point and KLM hit
    double m_ChiSquared;

    //! Needed to make the ROOT object storable
    ClassDef(MuidHit, 1)

  };

}
#endif
