/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWTOP_H
#define RAWTOP_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw TOP class
   * Class for RawCOPPER class data taken by TOP
   * Currently, this class is almost same as RawCOPPER class.
   */

  class RawTOP : public RawCOPPER {
  public:
    //! Default constructor
    RawTOP();
    //! Destructor
    virtual ~RawTOP();

    /// To derive from TObject
    ClassDef(RawTOP, 1);
  };
}

#endif
