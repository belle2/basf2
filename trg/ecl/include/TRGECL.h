//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECL.h
// Section  : TRG ECL
// Owner    : Yuuji Unno
// Email    : yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECL_FLAG_
#define TRGECL_FLAG_

#include <string>
#include <vector>
#include "trg/ecl/TCHit.h"
//
//
//
namespace Belle2 {
//
//
//
class TRGECL;
//
//
//
class TRGECL {

 public:

  // get pointer of TRGECL object 
  static TRGECL * getTRGECL(void);

 private:
  
  /// Constructor
  TRGECL(void);

  /// Destructor
  virtual ~TRGECL(){};
  
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
  int getECLtoGDL(void){ return bitECLtoGDL; }

  // 
  void setPRS(TCHit *);
  
 private:

  static TRGECL * _ecl;

  int bitECLtoGDL;
  double _PhiRingSum[17];

};
//
//
//
} // namespace Belle2

#endif /* TRGECL_FLAG_ */
