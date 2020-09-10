//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

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
