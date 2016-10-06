/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Shebalin Vasily                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ECLPACKERMODULE_H
#define ECLPACKERMODULE_H


#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawECL.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <framework/core/Module.h>
#include <ecl/utility/eclChannelMapper.h>


namespace Belle2 {

  enum {
    ECL_ADC_SAMPLES_PER_CHANNEL = 31
  };

  namespace ECL {

    class ECLPackerModule : public Module {
    public:
      ECLPackerModule();
      virtual ~ECLPackerModule();

      virtual void initialize();
      virtual void beginRun();
      virtual void event();
      virtual void endRun();
      virtual void terminate();

      BELLE2_DEFINE_EXCEPTION(Write_adc_samples_error,
                              "Error packing adc samples to buffer");
      BELLE2_DEFINE_EXCEPTION(eclPacker_internal_error,
                              "Something wrong with ECL Packer");


    protected:


    private:
//      /** CPU time     */
//      double m_timeCPU;
//      /** Run number   */
//      int    m_nRun;
      /** Event number */
      int    m_EvtNum;

      /** pointer to data  */
      unsigned int* m_bufPtr;
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
      int m_EclWaveformSamples[ECL_ADC_SAMPLES_PER_CHANNEL]; // 31

      ECLChannelMapper* m_eclMapper;

      /** Output data  */
      StoreArray<RawECL> m_eclRawCOPPERs;

      unsigned int adcBuffer_temp[ECL_CHANNELS_IN_SHAPER * ECL_ADC_SAMPLES_PER_CHANNEL];

      // number of hits, masks etc ...
      int collectorMaskArray[ECL_CRATES];
      int shaperMaskArray[ECL_CRATES][ECL_BARREL_SHAPERS_IN_CRATE];
      int shaperADCMaskArray[ECL_CRATES][ECL_BARREL_SHAPERS_IN_CRATE];
      int shaperNWaveform[ECL_CRATES][ECL_BARREL_SHAPERS_IN_CRATE];
      int shaperNHits[ECL_CRATES][ECL_BARREL_SHAPERS_IN_CRATE];
      int* iEclDigIndices;
      int* iEclWfIndices;



//      unsigned int readNBits(unsigned int* buff, unsigned int bitsToRead);
      void writeNBits(unsigned int* buff, unsigned int value, unsigned int bitsToWrite);
      void resetBuffPosition();
      void setBuffLength(int bufLength);

    };
  }//namespace ECL

}//namespace Belle2

#endif

