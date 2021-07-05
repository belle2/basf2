/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
