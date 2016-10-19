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
#include "trg/ecl/TrgEclCluster.h"
//
//
//
//
//
namespace Belle2 {
//
//
  /*! ETM class */
  class TrgEcl;
//
//
//
  class TrgEcl {

  public:

    /**  get pointer of TrgEcl object  */

    static TrgEcl* getTrgEcl(void);


    /** Constructor */
    TrgEcl(void);

    /** Destructor */
    virtual ~TrgEcl();

  public:

    /** initialize */
    void initialize(int);
    /**simulates ECL trigger. */
    void simulate(int);

  public:

    /** returns name. */
    std::string name(void) const;

    /** returns version. */
    std::string version(void) const;
    /** ECL bit information for GDL */
    int getECLtoGDL(void) { return bitECLtoGDL; }

    /** Set Phi Ring Sum  */
    void setPRS(int iBin);
    /** Get Event timing */
    void getEventTiming(int option);


  private:
    /** Hitted TC  */
    int   HitTC[160][576][64];
    /** Event timing */
    double EventTiming[160];
    /** TC Timing */
    double Timing[576][64];
    /**  TC Energy */
    double Energy[576][64];
    /**  5 top TCs */
    double Etop5[5];
    /** 5 top TCs  */
    double Ttop5[5];
    /** ecl object */
    static TrgEcl* _ecl;
    /**  bit inforamtion */
    int bitECLtoGDL;
    /**  Phi ring sum */
    double _PhiRingSum[17];
    /**  Cluster object */
    TrgEclCluster* obj_cluster;


  };
//
//
//
} /// namespace Belle2

#endif /* TRGECL_FLAG_ */
