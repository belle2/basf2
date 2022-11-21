/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef TRGTOPUNPACKERWAVEFORM_H
#define TRGTOPUNPACKERWAVEFORM_H

#include <string>

//#pragma once

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/top/dataobjects/TRGTOPWaveFormTimeStampsSlot.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>


#define NUMBER_OF_TOP_SLOTS 16

namespace Belle2 {
  /**
  * TRG TOP Unpacker for Timestamps
  *
  * TOP TRG timestamps unpacker
  *
  *
  */

  class TRGTOPUnpackerWaveformModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TRGTOPUnpackerWaveformModule();

    /**  */
    virtual ~TRGTOPUnpackerWaveformModule() override;

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

    /** returns version of TRGTOPUnpackerWaveformModule.*/
    std::string version() const;

    /** Read data from TRG DAQ.*/
    virtual void readDAQEvent(RawTRG*, int, int);

    /** Unpacker main function.*/
    virtual void unpackWaveforms(int*, int);

  protected:

    int m_eventNumber;    /**Event number (according to L1/global) */
    int m_trigType;       /**Trigger type */
    int m_nodeId;         /**Our read-out ID */
    int m_nWords;         /**N words in raw data */
    bool m_pciedata;         /**PCIe40 data or copper data */

    bool m_reportedAlreadyRun_1;
    bool m_reportedAlreadyRun_2;

    bool m_overrideControlBits;

  private:

    std::string m_outputWaveFormTimeStampsSlotsName;    /**< name of TOPTRGTimeStampsSlot store array */
    std::string m_outputWaveFormTimeStampsName;    /**< name of TOPTRGTimeStamp store array */

    StoreArray<TRGTOPWaveFormTimeStampsSlot>  m_TRGTOPWaveFormTimeStampsSlots;
    StoreArray<TRGTOPWaveFormTimeStamp>  m_TRGTOPWaveFormTimeStamps;

  };
}

#endif
