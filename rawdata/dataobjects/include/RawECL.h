//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWECL_H
#define RAWECL_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawHeader.h>
#include <rawdata/dataobjects/RawTrailer.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

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
    // To derive from TObject
    ClassDef(RawECL, 1);
  };
}

#endif
