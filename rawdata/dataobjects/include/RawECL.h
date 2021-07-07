/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWECL_H
#define RAWECL_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw ECL class
   * Class for RawCOPPER class data taken by ECL
   * Currently, this class is almost same as RawCOPPER class.
   */
  class RawECL : public RawCOPPER {
  public:
    //! Default constructor
    RawECL();
    //! Destructor
    virtual ~RawECL();
    /// To derive from TObject
    ClassDef(RawECL, 1);
  };
}

#endif
