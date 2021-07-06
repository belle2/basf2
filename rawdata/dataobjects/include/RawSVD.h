/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWSVD_H
#define RAWSVD_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw SVD class
   * Class for RawCOPPER class data taken by SVD
   * Currently, this class is almost same as RawCOPPER class.
   */
  class RawSVD : public RawCOPPER {
  public:
    //! Default constructor
    RawSVD();
    //! Destructor
    virtual ~RawSVD();
    /// To derive from TObject
    ClassDef(RawSVD, 1);
  };
}

#endif
