/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Shebalin Vasily                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ECLUNPACKERMODULE_H
#define ECLUNPACKERMODULE_H


#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawECL.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <framework/core/Module.h>
#include "ecl/utility/eclChannelMapper.h"

namespace Belle2 {
  namespace ECL {

    class ECLUnpackerModule : public Module {
    public:
      ECLUnpackerModule();
      virtual ~ECLUnpackerModule();

      virtual void initialize();
      virtual void beginRun();
      virtual void event();
      virtual void endRun();
      virtual void terminate();

      /** exeption should be thrown when the unexpected      */
      BELLE2_DEFINE_EXCEPTION(Unexpected_end_of_FINESSE_buffer,
                              "Unexpected end of the FINESS buffer is reached while reading ShpaerDSP data");
      /** exeption should be thrown when the Shaepr DSP header is corrupted    */

      BELLE2_DEFINE_EXCEPTION(Bad_ShaperDSP_header, "Corrupted Shaper DSP header");

    protected:


    private:
//      /** CPU time     */
//      double m_timeCPU;
//      /** Run number   */
//      int    m_nRun;
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

      /** name of output collection for ECLLDigits  */
      std::string m_eclDigitsName;
      /** name of the file with correspondence between cellID and crate/shaper/channel numbers  */
      std::string m_eclMapperInitFileName;

      ECLChannelMapper m_eclMapper;

      /** Output data  */
      StoreArray<ECLDigit> m_eclDigits;

      /** read nex word from COPPER data, check if the end of data is reached  */
      unsigned int readNextCollectorWord();
      /** rean N bits from COPPER buffer (needed for reading the compressed ADC data) */
      unsigned int readNBits(int bitsToRead);
      /// read raw data from COPPER and fill output m_eclDigits container
      void readRawECLData(RawECL* rawCOPPERData, int n);

    };
  }//namespace ECL
}//namespace Belle2

#endif

