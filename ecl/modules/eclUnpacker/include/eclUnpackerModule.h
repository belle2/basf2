/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vasily Shebalin, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// Framework
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/core/FrameworkExceptions.h>
#include <framework/core/Module.h>

// ECL
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/dataobjects/ECLTrig.h>

namespace Belle2 {

  class EventMetaData;
  class RawECL;
  class ECLDigit;
  class ECLDsp;

  /** the ECL unpacker module */
  class ECLUnpackerModule : public Module {
  public:
    /** constructor */
    ECLUnpackerModule();
    /** destructor */
    virtual ~ECLUnpackerModule();

    /** initialize */
    virtual void initialize() override;
    /** beginRun */
    virtual void beginRun() override;
    /** event */
    virtual void event() override;
    /** endRun */
    virtual void endRun() override;
    /** terminate */
    virtual void terminate() override;

    /** exeption should be thrown when the unexpected      */
    BELLE2_DEFINE_EXCEPTION(Unexpected_end_of_FINESSE_buffer,
                            "Unexpected end of the FINESS buffer is reached while reading ShpaerDSP data");
    /** exeption should be thrown when the Shaepr DSP header is corrupted    */

    BELLE2_DEFINE_EXCEPTION(Bad_ShaperDSP_header, "Corrupted Shaper DSP header");

    /** ECL unpacker run-dependent parameters (per channel) */
    enum ECLUnpack {
      /** Skip ECLDigit unpacking */
      ECL_DISCARD_DSP_DATA   = 0x00000001,
      /** Keep ECLDigits for quality flag 0 even if ECL_DISCARD_DSP_DATA is set */
      ECL_KEEP_GOOD_DSP_DATA = 0x00000002,
      /** Get ECLDigits from offline waveform fit */
      ECL_OFFLINE_ADC_FIT = 0x00000004,
    };

  protected:


  private:
    /** event number from EventMetaData */
    int m_globalEvtNum;
    /** Internally counted event number */
    int m_localEvtNum;

    /** pointer to data from COPPER */
    unsigned int* m_bufPtr;
    /** position in the COPPER data array */
    int m_bufPos;
    /** length of COPPER data  */
    int m_bufLength;
    /** bit position for bit-by-bit data read  */
    int m_bitPos;

    /** flag for whether or not to store collection with trigger times */
    bool m_storeTrigTime;
    /** flag for whether or not to store ECLDsp data for unmapped channels */
    bool m_storeUnmapped;
    /** Use ECLUnpackingParameters payload for run-dependent unpacking */
    bool m_useUnpackingParameters;

    /** report only once per crate about inconsistency between trg tag and evt number */
    long m_evtNumReportedMask = 0;
    /** report only once per crate about problem with different trg tags */
    long m_tagsReportedMask = 0;
    /** report only once per crate about problem with different trg phases */
    long m_phasesReportedMask = 0;
    /** report only once per crate about problem with shaper header */
    long m_badHeaderReportedMask = 0;

    /**
     * Report the problem with inconsistency between trg tag and evt number.
     * Exclude the crate from further reports of this type.
     */
    void doEvtNumReport(unsigned int iCrate, int tag, int evt_number);
    /**
     * Report the problem with trigger tags and exclude the crate
     * from further reports of this type.
     */
    void doTagsReport(unsigned int iCrate, int tag0, int tag1);
    /**
     * Report the problem with trigger phases and exclude the crate
     * from further reports of this type.
     */
    void doPhasesReport(unsigned int iCrate, int phase0, int phase1);
    /**
     * Report the problem with bad shaper header and exclude the crate
     * from further reports of this type.
     */
    void doBadHeaderReport(unsigned int iCrate);

    /**
     * Check if the problem with trg tag <-> evt number inconsistency was
     * already reported for crate iCrate.
     */
    bool evtNumReported(unsigned int iCrate) { return m_evtNumReportedMask & (1L << (iCrate - 1)); }
    /**
     * Check if the problem with different trigger tags was already reported
     * for crate iCrate.
     */
    bool tagsReported(unsigned int iCrate) { return m_tagsReportedMask & (1L << (iCrate - 1)); }
    /**
     * Check if the problem with different trigger phases was already reported
     * for crate iCrate.
     */
    bool phasesReported(unsigned int iCrate) { return m_phasesReportedMask & (1L << (iCrate - 1)); }
    /**
     * Check if the problem with bad shaper header was already reported
     * for crate iCrate.
     */
    bool badHeaderReported(unsigned int iCrate) { return m_badHeaderReportedMask & (1L << (iCrate - 1)); }

    /** name of output collection for ECLDigits  */
    std::string m_eclDigitsName;
    /** name of output collection for ECLTrig  */
    std::string m_eclTrigsName;
    /** name of output collection for ECLDsp  */
    std::string m_eclDspsName;
    /** name of the file with correspondence between cellID and crate/shaper/channel numbers  */
    std::string m_eclMapperInitFileName;

    /** ECL channel mapper **/
    ECL::ECLChannelMapper m_eclMapper;

    /** Run-dependent unpacking parameters for each channel */
    DBObjPtr<ECLChannelMap> m_unpackingParams;

    /*   Input data   */

    /** store array for RawECL**/
    StoreArray<RawECL>   m_rawEcl;
    /** store objptr for EventMetaData **/
    StoreObjPtr<EventMetaData> m_eventMetaData;

    /*   Output data  */

    /** store array for digitized gits**/
    StoreArray<ECLDigit> m_eclDigits;
    /** store array for eclTrigs data (trigger time and tag)**/
    StoreArray<ECLTrig>  m_eclTrigs;
    /** store array for waveforms**/
    StoreArray<ECLDsp>   m_eclDsps;

    /** ECLTrigs objects before they are added to m_eclTrigs array */
    ECLTrig m_eclTrigsBuffer[ECL::ECL_CRATES];

    /** Cached debug level from LogSystem */
    int m_debugLevel;

    /** read nex word from COPPER data, check if the end of data is reached  */
    unsigned int readNextCollectorWord();
    /** rean N bits from COPPER buffer (needed for reading the compressed ADC data) */
    unsigned int readNBits(int bitsToRead);
    /** read raw data from COPPER and fill output m_eclDigits container */
    void readRawECLData(RawECL* rawCOPPERData, int n);
    /** Check if DSP data should be saved to datastore */
    bool isDSPValid(int cellID, int crate, int shaper, int channel, int amp, int time, int quality);

  };
}//namespace Belle2
