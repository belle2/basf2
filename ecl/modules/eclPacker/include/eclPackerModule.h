/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

//Framework
#include <framework/core/FrameworkExceptions.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

//ECL
#include <ecl/utility/ECLChannelMapper.h>

namespace Belle2 {

  enum {
    ECL_ADC_SAMPLES_PER_CHANNEL = 31
  };

  class ECLDigit;
  class ECLDsp;
  class RawECL;

  /** Module that pack's MC info into a dataformat that comes from the detector */
  class ECLPackerModule : public Module {
  public:
    ECLPackerModule();
    virtual ~ECLPackerModule();

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

    /// exception for errors during packing ADC data buffer
    BELLE2_DEFINE_EXCEPTION(Write_adc_samples_error,
                            "Error packing adc samples to buffer");
    /// wrong indexes for ShaperDSP, channel or crate are apperared
    BELLE2_DEFINE_EXCEPTION(eclPacker_internal_error,
                            "Something wrong with ECL Packer");

  private:
    /** Event number */
    int m_EvtNum;

    /** position in the  data array */
    int m_bufPos;

    /** length data  */
    int m_bufLength;

    /** bit position for bit-by-bit data read  */
    int m_bitPos;

    /** DSP amplitude threshold */
    int m_ampThreshold;

    /** eneble/disable compression of waveform data */
    bool m_compressMode;

    /** the rate of writing of the ADC samples*/
    int m_WaveformRareFactor;

    /** name of output collection for RawCOPPER */
    std::string m_eclRawCOPPERsName;

    /** name of the file with correspondence between cellID and crate/shaper/channel numbers  */
    std::string m_eclMapperInitFileName;

    /** array of ADC samples */
    int m_EclWaveformSamples[ECL_ADC_SAMPLES_PER_CHANNEL]; // == 31

    /** channel mapper */
    ECL::ECLChannelMapper m_eclMapper;

    /** Output data  */
    StoreArray<RawECL> m_eclRawCOPPERs;

    /** temporary buffer to store ADC data */
    unsigned int adcBuffer_temp[ECL::ECL_CHANNELS_IN_SHAPER * ECL_ADC_SAMPLES_PER_CHANNEL];

    // number of hits, masks etc ...
    /** array of triggered collectors */
    int collectorMaskArray[ECL::ECL_CRATES];
    /** triggered shapers */
    int shaperMaskArray[ECL::ECL_CRATES][ECL::ECL_BARREL_SHAPERS_IN_CRATE];
    /** shapers with ADC data */
    int shaperADCMaskArray[ECL::ECL_CRATES][ECL::ECL_BARREL_SHAPERS_IN_CRATE];
    /** Number of waveforms per shaper*/
    int shaperNWaveform[ECL::ECL_CRATES][ECL::ECL_BARREL_SHAPERS_IN_CRATE];
    /** Number of hits per shaper*/
    int shaperNHits[ECL::ECL_CRATES][ECL::ECL_BARREL_SHAPERS_IN_CRATE];

    /** indexes of related eclDigits*/
    int iEclDigIndices[ECL::ECL_TOTAL_CHANNELS];

    /** indexes of related waveforms*/
    int iEclWfIndices[ECL::ECL_TOTAL_CHANNELS];

    //DataStore variables
    StoreArray<ECLDigit> m_eclDigits; /**< ECLDigit dataStore object*/
    StoreArray<ECLDsp> m_eclDsps; /**< ECLDSP dataStore object*/

    /** write N bits to the collector buffer */
    void writeNBits(unsigned int* buff, unsigned int value, unsigned int bitsToWrite);
    /** reset current position in the buffer */
    void resetBuffPosition();
    /** set buffer length*/
    void setBuffLength(int bufLength);

  };
}//namespace Belle2
