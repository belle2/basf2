//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWKLM_H
#define RAWKLM_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw KLM class
   * Class for RawCOPPER class data taken by KLM.
   * Currently, this class is almost same as RawCOPPER class.
   */

  class RawKLM : public RawCOPPER {
  public:
    //! Default constructor
    RawKLM();
    //! Destructor
    virtual ~RawKLM();
    /// To derive from TObject
    ClassDef(RawKLM, 1);
  };
}

#endif
