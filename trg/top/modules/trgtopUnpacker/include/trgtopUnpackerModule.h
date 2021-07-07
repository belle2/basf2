/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef TRGTOPUNPACKER_H
#define TRGTOPUNPACKER_H

#include <string>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/top/dataobjects/TRGTOPUnpackerStore.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

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
