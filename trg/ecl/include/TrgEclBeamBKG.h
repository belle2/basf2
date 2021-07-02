//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TrgEclBeamBKG.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------
// Description : A class to represent TRG ECL.
//---------------------------------------------------------
// $Log$
//---------------------------------------------------------

#ifndef TRGECLBeamBKG_FLAG_
#define TRGECLBeamBKG_FLAG_

#include <vector>

#include "trg/ecl/TrgEclMapping.h"
//
//
//
namespace Belle2 {
//
//
//
  //
  /*! A Class of  ECL Trigger clustering  */
  //
  class TrgEclBeamBKG {

  public:
    /** Constructor */
    TrgEclBeamBKG();    /// Constructor

    /** Destructor */
    virtual ~TrgEclBeamBKG();/// Destructor

    /** Copy constructor, deleted. */
    TrgEclBeamBKG(TrgEclBeamBKG&) = delete;

    /** Assignment operator, deleted. */
    TrgEclBeamBKG& operator=(TrgEclBeamBKG&) = delete;

  public:

    int GetBeamBkg(std::vector<std::vector<double>>);

  private:
    //
    /** 4 region along r phi plane  */
    int Quadrant[3][4];
    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;
  };
//
//
//
} // namespace Belle2

#endif /* TRGECLBeamBKG_FLAG_ */
