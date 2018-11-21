/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Shebalin Vasily, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include "ecl/utility/ECLChannelMapper.h"

namespace Belle2 {

  class RawECL;
  class ECLDigit;
  class ECLTrig;
  class ECLDsp;

  class ECLUnpackerModule : public Module {
  public:
    ECLUnpackerModule();
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

  protected:


  private:
    /** Event number */
    int    m_EvtNum;

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
    /** report only once per crate about problem with different trg tags */
    long m_tagsReportedMask;
    /** report only once per crate about problem with different trg phases */
    long m_phasesReportedMask;
    /** report only once per crate about problem with shaper header */
    long m_badHeaderReportedMask;

    /**
     * Report the problem with trigger tags and exclude the crate
     * from further reports of this type.
     */
    void doTagsReport(int iCrate, int tag0, int tag1);
    /**
     * Report the problem with trigger phases and exclude the crate
     * from further reports of this type.
     */
    void doPhasesReport(int iCrate, int phase0, int phase1);
    /**
     * Report the problem with bad shaper header and exclude the crate
     * from further reports of this type.
     */
    void doBadHeaderReport(int iCrate);

    /**
     * Check if the problem with different trigger tags was already reported
     * for crate iCrate.
     */
    bool tagsReported(int iCrate) { return m_tagsReportedMask & (1 << (iCrate - 1)); }
    /**
     * Check if the problem with different trigger phases was already reported
     * for crate iCrate.
     */
    bool phasesReported(int iCrate) { return m_phasesReportedMask & (1 << (iCrate - 1)); }
    /**
     * Check if the problem with bad shaper header was already reported
     * for crate iCrate.
     */
    bool badHeaderReported(int iCrate) { return m_badHeaderReportedMask & (1 << (iCrate - 1)); }

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

    /** Output data  */
    /** store array for digitized gits**/
    StoreArray<ECLDigit> m_eclDigits;
    /** store array for eclTrigs data (trigger time and tag)**/
    StoreArray<ECLTrig>  m_eclTrigs;
    /** store array for waveforms**/
    StoreArray<ECLDsp>   m_eclDsps;
    /** store array for RawECL**/
    StoreArray<RawECL>   m_rawEcl;

    /** Cached debug level from LogSystem */
    int m_debugLevel;

    /** read nex word from COPPER data, check if the end of data is reached  */
    unsigned int readNextCollectorWord();
    /** rean N bits from COPPER buffer (needed for reading the compressed ADC data) */
    unsigned int readNBits(int bitsToRead);
    /** read raw data from COPPER and fill output m_eclDigits container */
    void readRawECLData(RawECL* rawCOPPERData, int n);

  };
}//namespace Belle2
