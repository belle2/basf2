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

#include <string>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerSumStore.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerEvtStore.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

namespace Belle2 {

  /*! A module of TRG ECL Unpacker */
  class TRGECLUnpackerModule : public Module {

  public:

    /** Constructor */
    TRGECLUnpackerModule();

    /** Destructor */
    virtual ~TRGECLUnpackerModule();

    /** Initilizes TRGECLUnpackerModuel.*/
    void initialize() override;

    /** Called event by event.*/
    void event() override;

    /** Called when processing ended.*/
    void terminate() override;

    /** Called when new run started.*/
    void beginRun() override;

    /** Called when run ended*/
    void endRun() override;

    /** returns version of TRGECLUnpackerModule.*/
    std::string version() const;

    /** Read data from TRG copper.*/
    virtual void readCOPPEREvent(RawTRG*, int, int);

    /** Unpacker main function.*/
    virtual void checkBuffer(int*, int);

    /** Unpacker main function for upto version 136.*/
    virtual void checkBuffer_v136(int*, int);

  protected :
    /** Event number */
    int n_basf2evt;
    //! ETM Version
    int etm_version = 0;
    //! Node Id
    unsigned int nodeid = 0;
    //! N Word
    int nwords = 0;
    //! Finess
    int iFiness = 0;
    //! Trigger Type
    int trgtype = 0;

  private :
    //! ECL Trigger Unpacker TC output
    StoreArray<TRGECLUnpackerStore>     m_TRGECLTCArray;
    //! ECL Trigger Unpacker Summary output
    StoreArray<TRGECLUnpackerSumStore>  m_TRGECLSumArray;
    //! ECL Trigger Unpacker Event output
    StoreArray<TRGECLUnpackerEvtStore>  m_TRGECLEvtArray;
    //! ECL Trigger Cluster output
    StoreArray<TRGECLCluster>  m_TRGECLClusterArray;
  };

} // end namespace Belle2

#endif // TRGECLUnpackerModule_H
