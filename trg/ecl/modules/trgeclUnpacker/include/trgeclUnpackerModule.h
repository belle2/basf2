//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : trgeclUnpackerModule.h
// Section  : TRG ECL
// Owner    : SungHyun Kim
// Email    : sungnhyun.kim@belle2.org
//---------------------------------------------------------------
// Description : TRG ECL Unpacker Module
//---------------------------------------------------------------
// 1.00 : 2017/05/06 : First version
//---------------------------------------------------------------

#ifndef TRGECLUNPACKER_H
#define TRGECLUNPACKER_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  /*! A module of TRG ECL Unpacker */
  class TRGECLUnpackerModule : public Module {

  public:

    /** Constructor */
    TRGECLUnpackerModule();

    /** Destructor */
    virtual ~TRGECLUnpackerModule();

    /** Initilizes TRGECLUnpackerModuel.*/
    virtual void initialize();

    /** Called event by event.*/
    virtual void event();

    /** Called when processing ended.*/
    virtual void terminate();

    /** Called when new run started.*/
    virtual void beginRun();

    /** Called when run ended*/
    virtual void endRun();

    /** returns version of TRGECLUnpackerModule.*/
    std::string version() const;

    /** Read data from TRG copper.*/
    virtual void readCOPPEREvent(RawTRG*, int);

    /** Unpacker main function.*/
    virtual void checkBuffer(int*);

    /** Utility of TRG ECL.*/
    std::string u_int2string(int, int);

  protected :
    /** Event number */
    int n_basf2evt;

  private :


  };

} // end namespace Belle2

#endif // TRGECLUnpackerModule_H
