/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWCDC_H
#define RAWCDC_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw CDC class
   * Class for RawCOPPER class data taken by CDC
   * Currently, this class is almost same as RawCOPPER class.
   */

  class RawCDC : public RawCOPPER {
  public:
    //! Default constructor
    RawCDC();
    //! Destructor
    virtual ~RawCDC();
    /// To derive from TObject
    ClassDef(RawCDC, 1);
  };
}

#endif
