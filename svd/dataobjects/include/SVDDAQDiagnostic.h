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
#include <framework/dataobjects/DigitBase.h>

namespace Belle2 {
  /**
   * Class to store SVD DAQ diagnostic information
   */
  class SVDDAQDiagnostic : public TObject {
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
      FTBErrorBits = 8,
      /** Number of bits available to represent a FTB Flags Field */
      FTBFlagsBits = 5,
      /** Number of bits available to represent an Emulated pipeline adress */
      EMUPipelineAddressBits = 8,
      /** Number of bits available to represent an APV error OR code */
      APVErrorORBits = 4,

      /** Total bit size of the SVDDAQDiagnostic */
      Bits = TriggerTypeBits + TriggerNumberBits + PipelineAddressBits + CMC1Bits + CMC2Bits + APVErrorBits + FTBErrorBits + FTBFlagsBits + EMUPipelineAddressBits + APVErrorORBits,
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
    SVDDAQDiagnostic(uint64_t code = 0)
    {
      m_info.code = code;
    }

    /** Copy constructor */
    SVDDAQDiagnostic(const SVDDAQDiagnostic& d): TObject(), m_info(d.m_info) {}

    /** Assignment operator */
    SVDDAQDiagnostic& operator=(const SVDDAQDiagnostic& d)
    {
      TObject::operator=(d);
      m_info = d.m_info;
      return *this;
    }
    /** Convert to uint64_t */
    operator uint64_t() const { return getCode(); }
    /** Check for equality */
    bool operator==(const SVDDAQDiagnostic& d) const   { return getCode() == d.getCode(); }

    /** Get the diagnostic code */
    uint64_t getCode() const { return m_info.code; }
    /** Get the trigger number */
    uint16_t getTriggerNumber() const { return static_cast<uint16_t>(m_info.parts.triggerNumber); }
    /** Get the trigger type */
    uint16_t getTriggerType() const { return static_cast<uint16_t>(m_info.parts.triggerType); }
    /** Get the pipeline address */
    uint16_t getPipelineAddress() const { return static_cast<uint16_t>(m_info.parts.pipelineAddress); }
    /** Get the Common Mode Correction w/o masking out particle signals */
    uint16_t getCMC1() const { return static_cast<uint16_t>(m_info.parts.cmc1); }
    /** Get the Common Mode Correction performed after masking particle signals */
    uint16_t getCMC2() const { return static_cast<uint16_t>(m_info.parts.cmc2); }
    /** Get the APV error code */
    uint16_t getAPVError() const { return static_cast<uint16_t>(m_info.parts.apvError); }
    /** Get the FTB errors field */
    uint16_t getFTBError() const { return static_cast<uint16_t>(m_info.parts.ftbError); }
    /** Get the FTB Flags field */
    uint16_t getFTBFlags() const { return m_info.parts.ftbFlags; }
    /** Get the emulated pipeline address */
    uint16_t getEmuPipelineAddress() const { return static_cast<uint16_t>(m_info.parts.emuPipelineAddress); }
    /** Get the APV error OR code */
    uint16_t getAPVErrorOR() const { return static_cast<uint16_t>(m_info.parts.apvErrorOR); }

    /** functions for setting values unpacked from FADC trailer
     * - FTB Flags Field
     * - emulated pipeline Address
     * - APV errors OR */
    void setFTBFlags(uint16_t ftbFlags) { m_info.parts.ftbFlags = ftbFlags; }
    void setEmuPipelineAddress(uint8_t emuPipelineAddress) { m_info.parts.emuPipelineAddress = emuPipelineAddress; }
    void setApvErrorOR(uint8_t apvErrorOR) { m_info.parts.apvErrorOR = apvErrorOR; }

    // redefinition of virtual methods
    //unsigned int getUniqueChannelID() const {return 0;}
    //EAppendStatus addBGDigit(const DigitBase* bg) {return DigitBase::c_DontAppend;}

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
        /** FTB Flags Field */
uint16_t ftbFlags : FTBFlagsBits;
        /** emulated pipeline Address */
uint8_t emuPipelineAddress : EMUPipelineAddressBits;
        /** APV error code in FADC trailer*/
uint8_t apvErrorOR : APVErrorORBits;

      } parts; /**< Struct to contain all components */
    } m_info; /**< Union to store the diagnostic code and all the inividual components */

    //ClassDef(SVDDAQDiagnostic, 5)
  }; // class SVDDAQDiagnostic
} // namespace Belle2

#endif //SVDDAQDIAGNOSTICS_H
