/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef TRGTOPTRD2TTSCONVERTER_H
#define TRGTOPTRD2TTSCONVERTER_H

//#pragma once

#include <framework/core/Module.h>
#include <string>

#include "trg/top/dataobjects/TRGTOPTimeStampsSlot.h"
#include "top/dataobjects/TOPDigit.h"
#include "top/dataobjects/TOPRawDigit.h"

#include <framework/datastore/StoreArray.h>

#define NUMBER_OF_TOP_SLOTS 16
#define MIN_NUMBER_OF_TIMESTAMPS 5

namespace Belle2 {
  /**
  * TRG TOP Raw Digits to TimeStamps Converter
  *
  * TOP raw digits to TOP TRG timestamps converter
  *
  *
  */

  class TRGTOPTRD2TTSConverterModule : public Module {

  public:

    // see TOPDatabaseImporter and TOPRawDigitConverterModule
    static constexpr int numberOfWindows[] = {0, 428, 852, 1280, 1708, 2132, 2560, 2988, 3412};

    // *64*0.375/8 = 8 * 0.375 = 3
    static constexpr int timeOfWindows[] = {0, 428 * 3, 852 * 3, 1280 * 3, 1708 * 3, 2132 * 3, 2560 * 3, 2988 * 3, 3412 * 3};

    // L1 latency in FTSW clock cycles
    static constexpr int latencyL1 = 611;

    // time correction in ns (estimated by comparing trigger readout with main readout
    static constexpr int timeCorrection = 14;

    // max number of FTSW clocks in revo9 cycle
    static constexpr int revo9CounterMax = 1280 * 9;

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TRGTOPTRD2TTSConverterModule();

    /**  */
    virtual ~TRGTOPTRD2TTSConverterModule() override;

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

    /** returns version of TRGTOPTRD2TTSConverterModule.*/
    std::string version() const;

  protected:

    int m_eventNumber;    /**Event number (according to L1/global) */

  private:

    struct interimTimeStamp {
      int slot;
      int value;
      const Belle2::TOPDigit* refDigit;
    };

    std::vector<interimTimeStamp> m_interimTimeStamps[NUMBER_OF_TOP_SLOTS];

    struct timeOrder {
      inline bool operator()(interimTimeStamp const& a, interimTimeStamp const& b)
      {
        double i = a.value;
        double j = b.value;
        return i < j;
      }
    };


    std::string m_inputRawDigitsName;  /**< name of TOPRawDigit store array */
    std::string m_inputConvertedDigitsName;  /**< name of TOPRawDigit store array */
    std::string m_outputTimeStampsSlotName;    /**< name of TOPTRGTimeStampsSlot store array */
    std::string m_outputTimeStampName;    /**< name of TOPTRGTimeStamp store array */

    bool m_addRelations = false;  /**< switch ON/OFF relations to TOPRawDigits */

    bool m_requireMinNumberOfTimeStamps = false;  /**< switch ON/OFF min number of timestamps requirement for individual slots */

    int m_minNumberOfTimeStamps;

    StoreArray<TOPDigit> m_convertedDigits; /**< collection of raw digits */
    StoreArray<TOPRawDigit> m_rawDigits; /**< collection of raw digits */

    StoreArray<TRGTOPTimeStampsSlot>  m_TRGTOPTimeStampsSlots;
    StoreArray<TRGTOPTimeStamp>  m_TRGTOPTimeStamps;

  };
}

#endif
