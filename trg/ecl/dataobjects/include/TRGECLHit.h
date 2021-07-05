/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

namespace Belle2 {

  //! Example Detector
  class TRGECLHit : public TObject {
  public:


    //! The TC id of this hit.
    int m_eventId;

    //! The TC id of this hit.
    int m_TCId;
    //! beambkg tag
    int m_beambkgtag;
    //! Deposited energy of this hit.
    double m_edep;

    //! Hit average time
    double m_TimeAve;

    //! The method to set event id
    void setEventId(int eventId) { m_eventId = eventId; }

    //! The method to set TC id
    void setTCId(int TCId) { m_TCId = TCId; }

    //! The method to set Beam Background tag
    void setBeamBkgTag(int beambkgtag) { m_beambkgtag = beambkgtag; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set hit average time
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }

    //! The method to get event id
    int getEventId() const { return m_eventId; }

    //! The method to get TC id
    int getTCId() const { return m_TCId; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get hit average time
    double getTimeAve() const {return m_TimeAve;}
    //! The method to get Beam Background tag
    int getBeamBkgTag() const {return m_beambkgtag; }


    // Empty constructor
    // Recommended for ROOT IO
    TRGECLHit()
    {
      m_eventId = 0;
      m_TCId  = 0;
      m_edep    = 0;
      m_TimeAve = 0;
      m_beambkgtag = 0;
    }

    //! Useful Constructor
    TRGECLHit(
      int eventId,
      int TCId,
      int beambkgtag,
      double edep,
      double TimeAve
    )
    {
      m_eventId = eventId;
      m_TCId = TCId;
      m_edep = edep;
      m_TimeAve = TimeAve;
      m_beambkgtag = beambkgtag;

    }
    /** the class title */
    ClassDef(TRGECLHit, 1);

  };

} // end namespace Belle2

#endif
