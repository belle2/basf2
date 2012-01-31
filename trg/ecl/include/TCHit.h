//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TCHit.h
// Section  : TRG ECL
// Owner    : Yuuji Unno
// Email    : yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------
#ifndef TCHIT_H
#define TCHIT_H

#include <TObject.h>
#include <TVector3.h>
#include "trg/ecl/TRGECLTCMapping.h"

namespace Belle2 {

  //! Example Detector
  class TCHit : public TObject {

  public:

    // Constructor
    TCHit();
  
    // Destructor
    virtual ~TCHit();

    //
    void setup(void);
    //! Hit average time
    double m_TimeAve;
    //! The method to set hit average time
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }

    // The method to get TC id
    std::vector<int> TCId(void) const { return _tcid; }
    std::vector<int> TCThetaId(void) const { return _tcthetaid; }
    std::vector<int> TCPhiId(void) const { return _tcphiid; }
    std::vector<double> TCEnergy(void) const { return _tcenergy; }

    int getTCThetaId(int tcid) const { return _tcthetaid[tcid-1]; }
    int getTCPhiId(int tcid) const { return _tcphiid[tcid-1]; }
    double getTCEnergy(int tcid) const { return _tcenergy[tcid-1]; }
    //! The method to get hit average time
    double getTimeAve() const {return m_TimeAve;}

  private:

    std::vector<int>    _tcid;
    std::vector<int>    _tcphiid;
    std::vector<int>    _tcthetaid;
    std::vector<double> _tcenergy;

    TRGECLTCMapping * _TCMap;

  };

} // end namespace Belle2

#endif
