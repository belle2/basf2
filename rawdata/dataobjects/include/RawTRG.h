/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWTRG_H
#define RAWTRG_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw TOP class
   * Class for RawCOPPER class data taken by TOP
   * Currently, this class is almost same as RawCOPPER class.
   */

  class RawTRG : public RawCOPPER {
  public:
    //! Default constructor
    RawTRG();
    //! Destructor
    virtual ~RawTRG();

    /// To derive from TObject
    ClassDef(RawTRG, 1);
  };
}

#endif
