/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jacek Stypula                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDDAQDIAGNOSTICS_H
#define SVDDAQDIAGNOSTICS_H

#include <cstdint>

namespace Belle2 {
  /**
   * Class to store SVD DAQ diagnostic information
   */
  class SVDDAQDiagnostic {
  public:
    enum {
      /** Number of bits available to represent a trigger type */
      TriggerTypeBits = 4,
      /** Number of bits available to represent a trigger number */
      TriggerNumberBits = 8,
      /** Number of bits available to represent a pipeline address */
      PipelineAddressBits = 8,
      /** Number of bits available to represent Common Mode Correction w/o masking out particle signals*/
      CMC1Bits = 8,
      /** Number of bits available to represent Common Mode Correction performed after masking particle signals */
      CMC2Bits = 4,
      /** Number of bits available to represent an APV error code */
      APVErrorBits = 4,
      /** Number of bits available to represent a FTB error code */
      FTBErrorBits = 3,

      /** Total bit size of the SVDDAQDiagnostic */
      Bits = TriggerTypeBits + TriggerNumberBits + PipelineAddressBits + CMC1Bits + CMC2Bits + APVErrorBits + FTBErrorBits,
    };


    /** Constructor.
     * @param triggerNumber as in the FADC header
     * @param triggerType as in the FADC header
     * @param pipelineAddress as in the APV header
     * @param cmc1 Common Mode Correction w/o masking out particle signals
     * @param cmc2 Common Mode Correction performed after masking particle signals
     * @param apvError Error code as in the APV header
     * @param ftbError Errors field as in the FTB header
     */
    SVDDAQDiagnostic(uint8_t triggerNumber, uint8_t triggerType, uint8_t pipelineAddress, uint8_t cmc1, uint8_t cmc2, uint8_t apvError,
                     uint8_t ftbError)
    {
      m_info.parts.triggerNumber = triggerNumber;
      m_info.parts.triggerType = triggerType;
      m_info.parts.pipelineAddress = pipelineAddress;
      m_info.parts.cmc1 = cmc1;
      m_info.parts.cmc2 = cmc2;
      m_info.parts.apvError = apvError;
      m_info.parts.ftbError = ftbError;
    }

    /** Constructor using the code value.  */
    // cppcheck-suppress noExplicitConstructor
    SVDDAQDiagnostic(uint64_t code)
    {
      m_info.code = code;
    }

    /** Copy constructor */
    SVDDAQDiagnostic(const SVDDAQDiagnostic& d): m_info(d.m_info) {}

    /** Assignment operator */
    SVDDAQDiagnostic& operator=(const SVDDAQDiagnostic& d) { m_info = d.m_info; return *this; }
    /** Convert to uint64_t */
    operator uint64_t() const { return getCode(); }
    /** Check for equality */
    bool operator==(const SVDDAQDiagnostic& d) const   { return getCode() == d.getCode(); }

    /** Get the diagnostic code */
    uint64_t getCode() const { return m_info.code; }
    /** Get the trigger number */
    uint8_t getTriggerNumber() const { return m_info.parts.triggerNumber; }
    /** Get the trigger type */
    uint8_t getTriggerType() const { return m_info.parts.triggerType; }
    /** Get the pipeline address */
    uint8_t getPipelineAddress() const { return m_info.parts.pipelineAddress; }
    /** Get the Common Mode Correction w/o masking out particle signals */
    uint8_t getCMC1() const { return m_info.parts.cmc1; }
    /** Get the Common Mode Correction performed after masking particle signals */
    uint8_t getCMC2() const { return m_info.parts.cmc2; }
    /** Get the APV error code */
    uint8_t getAPVError() const { return m_info.parts.apvError; }
    /** Get the FTB errors field */
    uint8_t getFTBError() const { return m_info.parts.ftbError; }

  private:
    // NB: the awful indentation is due to fixstyle.
    union {
      /** Raw value */
uint64_t code : Bits;
      struct {
        /** Trigger number */
uint8_t triggerNumber : TriggerNumberBits;
        /** Trigger type */
uint8_t triggerType : TriggerTypeBits;
        /** Pipeline address */
uint8_t pipelineAddress : PipelineAddressBits;
        /** CMC1 */
uint8_t cmc1 : CMC1Bits;
        /** CMC2 */
uint8_t cmc2 : CMC2Bits;
        /** APV error code */
uint8_t apvError : APVErrorBits;
        /** FTB error code */
uint8_t ftbError : FTBErrorBits;
      } parts; /**< Struct to contain all components */
    } m_info; /**< Union to store the diagnostic code and all the inividual components */
  }; // class SVDDAQDiagnostic
} // namespace Belle2

#endif //SVDDAQDIAGNOSTICS_H
