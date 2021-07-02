//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLBGTCHit.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#pragma once

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLBGTCHit : public TObject {

  public:

    //! The method to set TC id
    void setTCId(int TCId) { m_TCId = TCId; }
    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }
    //! The method to set hit average time
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }
    //! The method to get TC id
    int getTCId() const { return m_TCId; }
    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }
    //! The method to get hit average time
    double getTimeAve() const {return m_TimeAve;}

    // Empty constructor
    // Recommended for ROOT IO
    TRGECLBGTCHit()
    {
      m_TCId = 0;
      m_edep = 0;
      m_TimeAve = 0;
    }
    //! Useful Constructor
    TRGECLBGTCHit(
      int TCId,
      double edep,
      double TimeAve)
    {
      m_TCId = TCId;
      m_edep = edep;
      m_TimeAve = TimeAve;
    }

  private:

    //! The TC id of this hit.
    int m_TCId;
    //! Deposited energy of this hit.
    Double32_t m_edep;
    //! Hit average time
    Double32_t m_TimeAve;

    /** the class title */
    ClassDef(TRGECLBGTCHit, 1);

  };

} // end namespace Belle2
