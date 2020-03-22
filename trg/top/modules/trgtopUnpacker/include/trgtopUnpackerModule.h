/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2019 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Tong Pang, Vladimir Savinov                              *
* Email: vladimirsavinov@gmail.com
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#ifndef TRGTOPUNPACKER_H
#define TRGTOPUNPACKER_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

#include <framework/core/Module.h>
#include "rawdata/dataobjects/RawTRG.h"
#include "trg/top/dataobjects/TRGTOPUnpackerStore.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#define NUMBER_OF_SLOTS 16

namespace Belle2 {
  /**
  * TRG TOP Unpacker
  *
  * TOP trigger unpacker
  *
  *
  */

  class TRGTOPUnpackerModule : public Module {

  public:

    static constexpr double clkTo1ns = 0.5 / 0.508877;

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TRGTOPUnpackerModule();

    /**  */
    virtual ~TRGTOPUnpackerModule() override;

    /**  */
    virtual void initialize() override;

    /**  */
    virtual void beginRun() override;

    /**  */
    virtual void event() override;

    /**  */
    virtual void endRun() override;

    /**  */
    virtual void terminate() override;

    /** returns version of TRGGDLUnpackerModule.*/
    std::string version() const;

    /** Read data from TRG copper.*/
    virtual void readCOPPEREvent(RawTRG*, int);

    /** Unpacker main function.*/
    virtual void fillTreeTRGTOP(int*);

  protected:

    int m_eventNumber;    /**Event number (according to L1/global) */
    int m_trigType;       /**Trigger type */
    int m_nodeId;         /**Our read-out ID */
    int m_nWords;         /**N words in raw data */

  private:

    StoreArray<TRGTOPUnpackerStore>  m_TRGTOPCombinedTimingArray;

  };
}

#endif
