//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TrgEcl.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECL_FLAG_
#define TRGECL_FLAG_

#include <iostream>
#include <string>
#include <vector>
#include "trg/ecl/TrgEclFAM.h"
//
//
//
namespace Belle2 {
//
//
//
  class TrgEcl;
//
//
//
  class TrgEcl {

  public:

    // get pointer of TrgEcl object
    static TrgEcl* getTrgEcl(void);

  private:

    /// Constructor
    TrgEcl(void);

    /// Destructor
    virtual ~TrgEcl() {};

  public:

    // initialize
    void initialize(int);
    // simulates ECL trigger.
    void simulate(int);

  public:

    /// returns name.
    std::string name(void) const;

    // returns version.
    std::string version(void) const;

    // ECL bit information for GDL
    int getECLtoGDL(void) { return bitECLtoGDL; }

    //
    void setPRS(TrgEclFAM*, int HitTC[][20]);
    void getEventTiming(TrgEclFAM*, int);


  private:

    int HitTC[160][576][20];
    double EventTiming[160];

    static TrgEcl* _ecl;

    int bitECLtoGDL;
    double _PhiRingSum[17];

  };
//
//
//
} // namespace Belle2

#endif /* TRGECL_FLAG_ */
