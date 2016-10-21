//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLHit.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGECLHIT_H
#define TRGECLHIT_H


#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLHit : public TObject {
  public:


    //! The cell id of this hit.
    int m_eventId;

    //! The cell id of this hit.
    int m_cellId;

    //! The # of output of TC.
    int m_noutput;
    //
    int m_beambkgtag;
    //! Deposited energy of this hit.
    double m_edep;

    //! Hit average time
    double m_TimeAve;

    //! The method to set event id
    void setEventId(int eventId) { m_eventId = eventId; }

    //! The method to set cell id
    void setTCId(int cellId) { m_cellId = cellId; }

    //! The method to set # of output per TC
    void setTCOutput(int noutput) { m_noutput = noutput; }

    void setBeamBkgTag(int beambkgtag) { m_beambkgtag = beambkgtag; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set hit average time
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }

    //! The method to get event id
    int getEventId() const { return m_eventId; }

    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get hit average time
    double getTimeAve() const {return m_TimeAve;}
    int getBeamBkgTag() const {return m_beambkgtag; }


    // Empty constructor
    // Recommended for ROOT IO
    TRGECLHit()
    {
      m_eventId = 0;
      m_cellId  = 0;
      m_edep    = 0;
      m_TimeAve = 0;
      m_noutput = 0;
      m_beambkgtag = 0;
    }

    //! Useful Constructor
    TRGECLHit(
      int eventId,
      int cellId,
      int noutput,
      double edep,
      double TimeAve
    )
    {
      m_eventId = eventId;
      m_cellId = cellId;
      m_edep = edep;
      m_TimeAve = TimeAve;
      m_noutput = noutput;
    }
    ClassDef(TRGECLHit, 1); /*< the class title */

  };

} // end namespace Belle2

#endif
