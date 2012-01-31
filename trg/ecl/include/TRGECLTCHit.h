//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLTCHit.h
// Section  : TRG ECL
// Owner    : Yuuji Unno
// Email    : yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLTCHIT_H
#define TRGECLTCHIT_H


#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLTCHit : public TObject {
  public:


    //! The cell id of this hit.
    int m_eventId;

    //! The TC id of this hit.
    int m_tcId;
    //! The TC phi id of this hit.
    int m_phiId;
    //! The TC theta id of this hit.
    int m_thetaId;

    //! Deposited energy of this hit.
    double m_edep;

    //! Hit average time
    double m_TimeAve;

    //! The method to set event id
    void setEventId(int eventId) { m_eventId = eventId; }

    //! The method to set TC id
    void setTCId(int tcId) { m_tcId = tcId; }

    //! The method to set TC phi id
    void setPhiId(int phiId) { m_phiId = phiId; }

    //! The method to set TC theta id
    void setThetaId(int thetaId) { m_thetaId = thetaId; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set hit average time
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }

    //! The method to get event id
    int getEventId() const { return m_eventId; }

    //! The method to get cell id
    int getTCId() const { return m_tcId; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get hit average time
    double getTimeAve() const {return m_TimeAve;}

    // Empty constructor
    // Recommended for ROOT IO
    TRGECLTCHit() {;}

    //! Useful Constructor
    TRGECLTCHit(
      int eventId,
      int tcId,
      int phiId,
      int thetaId,
      double edep,
      double TimeAve
    ) {
      m_eventId = eventId;
      m_tcId    = tcId;
      m_phiId   = phiId;
      m_thetaId = thetaId;
      m_edep    = edep;
      m_TimeAve = TimeAve;
    }
    ClassDef(TRGECLTCHit, 1); /*< the class title */

  };

} // end namespace Belle2

#endif
