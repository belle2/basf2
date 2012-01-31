//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLTCMapping.h
// Section  : TRG ECL
// Owner    : Yuuji Unno
// Email    : yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLTCMAPPING_FLAG_
#define TRGECLTCMAPPING_FLAG_

#include <string>
#include <vector>
//
//
//
namespace Belle2 {
//
//
//
class TRGECLTCMapping;
//
// A class to represent ECL.
//
class TRGECLTCMapping {

 public:

  /// Constructor
  TRGECLTCMapping();
  
  /// Destructor
  virtual ~TRGECLTCMapping(){};
  
 public:

  // get [TC ID] from [Xtal ID] 
  int getTCIdFromXtalId(int);
  // get [TC sub ID] from [Xtal ID] 
  int getTCSubIdFromXtalId(int);
  // get [TC Theta ID] from [TC ID] 
  int getTCThetaIdFromTCId(int);
  // get [TC Phi ID] from [TC ID] 
  int getTCPhiIdFromTCId(int);

 private:
  //
  int _tcid;       // ID = 1-576
  int _tcsubid;    // ID = 0-1
  int _tcthetaid;  // ID = 0-16
  int _tcphiid;    // ID = 0-35(0-31 for both endcap)

};
//
//
//
} // namespace Belle2

#endif /* TRGECLTCMAPPING_FLAG_ */
