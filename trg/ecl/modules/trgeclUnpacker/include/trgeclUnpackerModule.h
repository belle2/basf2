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
// 1.01 : 2017/07/17 : Add FTSW clock from FAM, fine timing
// 2.00 : 2018/02/17 : 8 window data (ETM Ver. 100)
// 3.00 : 2018/07/31 : ETM version dependence included
//---------------------------------------------------------------

#ifndef TRGECLUNPACKER_H
#define TRGECLUNPACKER_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerSumStore.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerEvtStore.h"

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
    virtual void readCOPPEREvent(RawTRG*, int, int);

    /** Unpacker main function.*/
    virtual void checkBuffer(int*, int);

    /** Unpacker main function. ETM verion 114 */
    virtual void checkBuffer_114(int*, int);

    /** Unpacker main function. ETM verion 115 */
    virtual void checkBuffer_115(int*, int);

  protected :
    /** Event number */
    int n_basf2evt;
    int etm_version;

    unsigned int nodeid;
    int nwords;
    int iFiness;

  private :

    StoreArray<TRGECLUnpackerStore>     m_TRGECLTCArray;
    StoreArray<TRGECLUnpackerSumStore>  m_TRGECLSumArray;
    StoreArray<TRGECLUnpackerEvtStore>  m_TRGECLEvtArray;

  };

} // end namespace Belle2

#endif // TRGECLUnpackerModule_H
