/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TRGSUMMARY_H
#define TRGSUMMARY_H

#include <TObject.h>

namespace Belle2 {

  /**
   * Trigger Summary Information
   *   input bits
   *     input bits from subdetectors
   *   ftdl (Final Trigger Decision Logic) bits
   *     output bits of trigger logic
   *   psnm (Prescale and Mask) bits
   *     prescaled ftdl bits
   *   timType
   *     types of trigger timing source defined in b2tt firmware
   */
  class TRGSummary : public TObject {

  private:

    /**
     * version of this code
     */
    static const int c_Version = 1;

  public: //enum
    /** types of trigger timing source defined in b2tt firmware */
    enum timingType {
      /** events triggered by top timing */
      TTYP_PID0 = 0,
      /** events triggered by top timing */
      TTYP_PID1 = 4,
      /** events triggered by top timing */
      TTYP_PID2 = 8,
      /** events triggered by top timing */
      TTYP_PID3 = 12,
      /** reserved (not defined yet) */
      TTYP_RSV0 = 2,
      /** reserved (not defined yet) */
      TTYP_RSV1 = 6,
      /** reserved (not defined yet) */
      TTYP_RSV2 = 10,
      /** reserved (not defined yet) */
      TTYP_RSV3 = 14,
      /** events triggered by ecl timing */
      TTYP_ECL = 1,
      /** events triggered by cdc timing */
      TTYP_CDC = 3,
      /** delayed physics events for background */
      TTYP_DPHY = 5,
      /** random trigger events */
      TTYP_RAND = 7,
      /** test pulse input */
      TTYP_TEST = 9,
      /** reserved (not defined yet) */
      TTYP_RSV4 = 11,
      /** f also used for begin-run */
      TTYP_RSV5 = 13,
      /** reserved (not defined yet) */
      TTYP_NONE = 15
    };

  public:

    /*! default constructor: xxx */
    TRGSummary() {;}

    /*! constructor: xxx */
    TRGSummary(unsigned int inputBits[10],
               unsigned int ftdlBits[10],
               unsigned int psnmBits[10],
               timingType timType)
    {
      for (int i = 0; i < 10; i++) {
        m_inputBits[i] = inputBits[i];
        m_ftdlBits[i] = ftdlBits[i];
        m_psnmBits[i] = psnmBits[i];
      }
      m_timType = timType;
    }

    /** Destructor.
     */
    ~TRGSummary() {}

    /*! setter
     * @param xxx explanation
     */
    void setTRGSummary(int i, int word) { m_ftdlBits[i] = word;}

    /**set the prescale factor of each bit*/
    void setPreScale(int i, int bit, int pre) {m_prescaleBits[i][bit] = pre;}

    /**set the ftdl bits, the same as setTRGSummary(int i, int word)*/
    void setFtdlBits(int i, int word) {m_ftdlBits[i] = word;}

    /**set the Prescaled ftdl bits*/
    void setPsnmBits(int i, int word) {m_psnmBits[i] = word;}

    /** get the trigger result, each word has 32 bits*/
    unsigned int getTRGSummary(int i) {return m_ftdlBits[i];}

    /** get the prescale factor which the bit is corresponding*/
    unsigned int getPreScale(int i, int bit) {return m_prescaleBits[i][bit];}

    /*! get input bits
     * @param i index: 0, 1, 2 for bit 0-31, 32-63, 64-95, respectively.
     * @return     input bits
     */
    unsigned int getInputBits(const unsigned i) const
    {
      return m_inputBits[i];
    }

    /*! get ftdl bits (directly determined by the trigger conditions)
     * @param i index: 0, 1, 2 for bit 0-31, 32-63, 64-95, respectively.
     * @return     ftdl bits
     */
    unsigned int getFtdlBits(const unsigned i) const
    {
      return m_ftdlBits[i];
    }

    /*! get psnm bits (prescaled ftdl bits)
     * @param i index: 0, 1, 2 for bit 0-31, 32-63, 64-95, respectively.
     * @return     psnm bits
     */
    unsigned int getPsnmBits(const unsigned i) const
    {
      return m_psnmBits[i];
    }

    /*! get timing source information
     * @return     timing source type
     */
    timingType getTimType() const
    {
      return m_timType;
    }

  private:

    /** input bits from subdetectors */
    unsigned int m_inputBits[10] = {0};

    /** ftdl (Final Trigger Decision Logic) bits. Outputs of trigger logic  */
    unsigned int m_ftdlBits[10] = {0};

    /*! psnm (PreScale aNd Mask) bits. Prescaled ftdl bits
     * For instance, if the prescale factor is 20 of a ftdl bit, only 1/20 of its psnm bit would be fired.
     */
    unsigned int m_psnmBits[10] = {0};

    /** types of trigger timing source defined in b2tt firmware */
    timingType m_timType;

    /** the prescale factor of each bit*/
    unsigned int m_prescaleBits[10][32] = {0};

    /**  Trigger Summary Information including bit (input, ftdl, psnm), timing and trigger source. */
    ClassDef(TRGSummary, 2);

  };


} // end namespace Belle2

#endif
