//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWEPID_H
#define RAWEPID_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /**
   * The Raw EPID class
   * Class for RawCOPPER class data taken by EPID
   * Currently, this class is almost same as RawCOPPER class.
   */

  class RawEPID : public RawCOPPER {
  public:
    //! Default constructor
    RawEPID();
    //! Destructor
    virtual ~RawEPID();
    /// To derive from TObject
    ClassDef(RawEPID, 1);
  };
}

#endif
