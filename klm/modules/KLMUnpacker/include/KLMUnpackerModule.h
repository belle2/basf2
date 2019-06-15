/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <string>

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dataobjects/BKLMDigitOutOfRange.h>
#include <bklm/dbobjects/BKLMADCThreshold.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <eklm/dbobjects/EKLMChannels.h>
#include <eklm/dbobjects/EKLMElectronicsMap.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <klm/dataobjects/KLMDigitEventInfo.h>
#include <klm/dataobjects/KLMDigitRaw.h>
#include <klm/dbobjects/KLMTimeConversion.h>
#include <rawdata/dataobjects/RawKLM.h>

namespace Belle2 {

  /**
   * KLM unpacker.
   */
  class KLMUnpackerModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMUnpackerModule();

    /**
     * Destructor.
     */
    virtual ~KLMUnpackerModule();

    /**
     * Initializer.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * This method is called for each event.
     */
    virtual void event() override;

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate() override;

  private:

    /**
     * Unpack one EKLM digit.
     * @param[in] rawData           Data to be unpacked.
     * @param[in] lane              Lane.
     * @param[in] klmDigitEventInfo KLMDigitEventInfo.
     */
    void unpackEKLMDigit(const int* rawData, EKLMDataConcentratorLane* lane,
                         KLMDigitEventInfo* klmDigitEventInfo);

    /**
     * Unpack one BKLM digit.
     * @param[in] rawData           Data to be unpacked.
     * @param[in] copper            Copper identifier.
     * @param[in] hslb              HSLB number.
     * @param[in] klmDigitEventInfo KLMDigitEventInfo.
     */
    void unpackBKLMDigit(const int* rawData, int copper, int hslb,
                         KLMDigitEventInfo* klmDigitEventInfo);

    /**
     * Fill m_electIdToModuleId from database.
     */
    void loadMapFromDB();

    /**
     * To be used to map electronics address to module id.
     *
     * @param copperId
     * Id of the copper board.
     *
     * @param finesseNum
     * The Finesse slot on the copper boards.
     *
     * @param lane
     * The lane number, giving for the rpcs the slot number in the crate.
     *
     * @param axis
     * The axis bit in the datapacket.
     */
    int electCooToInt(int copper, int finesse, int lane, int axis, int channel);

    /* Module parameters. */

    /** Name of BKLMDigit store array. */
    std::string m_outputBKLMDigitsName;

    /** Name of EKLMDigit store array. */
    std::string m_outputEKLMDigitsName;

    /* EKLM parameters. */

    /** Record wrong hits (e.g. for debugging). */
    bool m_WriteWrongHits;

    /**
     * Do not issue B2ERROR on wrong hits, with certain firmware versions
     * wrong strip numbers are expected.
     */
    bool m_IgnoreWrongHits;

    /**
     * Ignore hits with strip = 0. Such hits are normally expected for normal
     * firmware versions.
     */
    bool m_IgnoreStrip0;

    /* BKLM parameters. */

    /** The flag to keep the even packages. */
    bool m_keepEvenPackages = false;

    /** Load threshold from DataBase (true) or not (false). */
    bool m_loadThresholdFromDB = true;

    /** Threshold for the scintillator NPE .*/
    double m_scintThreshold = 140;

    /* Common database objects. */

    /** Time conversion. */
    DBObjPtr<KLMTimeConversion> m_TimeConversion;

    /** Raw data. */
    StoreArray<RawKLM> m_RawKLMs;

    /** Event information. */
    StoreArray<KLMDigitEventInfo> m_DigitEventInfos;

    /* EKLM database objects. */

    /** Electronics map. */
    DBObjPtr<EKLMElectronicsMap> m_ElectronicsMap;

    /** Channels. */
    DBObjPtr<EKLMChannels> m_Channels;

    /** EKLM digits. */
    StoreArray<EKLMDigit> m_eklmDigits;

    /* BKLM database objects. */

    /** Map: hardware coordinates to logical coordinates. */
    std::map<int, int> m_electIdToModuleId;

    /** ADC offset and threshold read from database. */
    DBObjPtr<BKLMADCThreshold> m_ADCParams;

    /** BKLM digits. */
    StoreArray<BKLMDigit> m_bklmDigits;

    /** Raw digits. */
    StoreArray<KLMDigitRaw> m_klmDigitRaws;

    /** Out-of-range digits. */
    StoreArray<BKLMDigitOutOfRange> m_bklmDigitOutOfRanges;

    /* Other common variables. */

    /** Trigger ctime of the previous event. */
    unsigned int m_triggerCTimeOfPreviousEvent;

    /* Other EKLM variables. */

    /** Element numbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbers;

    /* Other BKLM variables. */

    /**
     * Counter for channels that were rejected due to unreasonable
     * channel number.
     */
    long m_rejectedCount = 0;

    /**
     * Warning message: number of channels rejected due to unreasonable
     * channel number.
     */
    std::map<std::string, long> m_rejected;

  };

}
