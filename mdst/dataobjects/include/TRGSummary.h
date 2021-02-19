/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hideyuki Nakazawa, Thomas Hauth                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

#include <string>

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
  class TRGSummary final : public RelationsObject {

  public:

    /** size of a l1 trigger word */
    static const unsigned int c_trgWordSize = 32;

    /** number of l1 trigger words */
    static const unsigned int c_ntrgWords = 10;

    /** types of trigger timing source defined in b2tt firmware */
    enum ETimingType {
      /** events triggered by ECL timing */
      TTYP_ECL  = 0,
      /** reserved (not defined yet) */
      TTYP_PID1 = 4,
      /** reserved (not defined yet) */
      TTYP_PID2 = 8,
      /** reserved (not defined yet) */
      TTYP_PID3 = 12,
      /** events triggered by self trigger */
      TTYP_SELF = 2,
      /** reserved (not defined yet) */
      TTYP_RSV1 = 6,
      /** reserved (not defined yet) */
      TTYP_RSV2 = 10,
      /** reserved (not defined yet) */
      TTYP_RSV3 = 14,
      /** events triggered by TOP timing */
      TTYP_TOP = 1,
      /** events triggered by CDC timing */
      TTYP_CDC = 3,
      /** delayed physics events for background */
      TTYP_DPHY = 5,
      /** random trigger events */
      TTYP_RAND = 7,
      /** test pulse input */
      TTYP_TEST = 9,
      /** reserved (not defined yet) */
      TTYP_RSV4 = 11,
      /** poisson random trigger */
      TTYP_POIS = 13,
      /** reserved (not defined yet) */
      TTYP_NONE = 15
    };

    /** trigger timing type quality */
    enum ETimingQuality {
      /* Non. Must not happen for TOP/ECL/CDC timing events */
      TTYQ_NONE = 0,
      /* Coarse */
      TTYQ_CORS = 1,
      /* Fine */
      TTYQ_FINE = 2,
      /* Super Fine */
      TTYQ_SFIN = 3,
    };

    /*! default constructor: xxx */
    TRGSummary() = default;

    /*! constructor: xxx */
    TRGSummary(unsigned int inputBits[10],
               unsigned int ftdlBits[10],
               unsigned int psnmBits[10],
               ETimingType timType);

    /** check whether any psnm bit is set
     * @return True if triggered by L1
     */
    bool test() const;

    /** check whether an input bit is set
     * @param bit index of input bit.
     * @return True if the bit is set
     */
    bool testInput(unsigned int bit) const;

    /** check whether an input bit is set
     * @param name name of input bit.
     * @return True if the bit is set
     */
    bool testInput(const std::string& name) const {return testInput(getInputBitNumber(name));}

    /** check whether a ftdl bit is set
     * @param bit index of ftdl bit.
     * @return True if the bit is set
     */
    bool testFtdl(unsigned int bit) const;

    /** check whether a ftdl bit is set
     * @param name name of ftdl bit.
     * @return True if the bit is set
     */
    bool testFtdl(const std::string& name) const {return testFtdl(getOutputBitNumber(name));}

    /** check whether a psnm bit is set
     * @param bit index of psnm bit.
     * @return True if the bit is set
     */
    bool testPsnm(unsigned int bit) const;

    /** check whether a psnm bit is set
     * @param name name of psnm bit.
     * @return True if the bit is set
     */
    bool testPsnm(const std::string& name) const {return testPsnm(getOutputBitNumber(name));}

    /*! check whether poisson random trigger is within injection veto
     * @return True if poisson random trigger is within injection veto
     */
    bool isPoissonInInjectionVeto() const
    {
      return m_isPoissonInInjectionVeto;
    }

    /*! set true if poisson random trigger is within injection veto
     */
    void setPoissonInInjectionVeto(void)
    {
      m_isPoissonInInjectionVeto = true;
    }

    /**set the Final Trigger Decision Logic bit*/
    void setTRGSummary(int i, int word) { m_ftdlBits[i] = word;}

    /**set the prescale factor of each bit*/
    void setPreScale(int i, int bit, int pre) {m_prescaleBits[i][bit] = pre;}

    /**set the input bits*/
    void setInputBits(int i, int word) {m_inputBits[i] = word;}

    /**set the ftdl bits, the same as setTRGSummary(int i, int word)*/
    void setFtdlBits(int i, int word) {m_ftdlBits[i] = word;}

    /**set the Prescaled ftdl bits*/
    void setPsnmBits(int i, int word) {m_psnmBits[i] = word;}

    /** get the trigger result, each word has 32 bits*/
    unsigned int getTRGSummary(int i) const {return m_ftdlBits[i];}

    /** get the prescale factor which the bit is corresponding*/
    unsigned int getPreScale(int i, int bit) const {return m_prescaleBits[i][bit];}

    /**set the timType */
    void setTimType(ETimingType timType) {m_timType = timType;}

    /**set the timQuality */
    void setTimQuality(ETimingQuality timQuality) {m_timQuality = timQuality;}

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
    ETimingType getTimType() const
    {
      return m_timType;
    }

    /*! get timing source quality
     * @return     timing type quality
     */
    ETimingQuality getTimQuality() const
    {
      return m_timQuality;
    }

    /** Return a short summary of this object's contents in HTML format. */
    std::string getInfoHTML() const override;

    /** get number of an input trigger bit
     * @param name input trigger bit name
     * @return     input trigger bit number
    */
    unsigned int getInputBitNumber(const std::string& name) const;

    /** get number of an output trigger bit
     * @param name output trigger bit name
     * @return     output trigger bit number
    */
    unsigned int getOutputBitNumber(const std::string& name) const;

  private:

    /** return the td part of an HTML table with green of the bit is > 0 */
    std::string outputBitWithColor(bool bit) const;

    /**
     * version of this code
     */
    static const int c_Version = 1;

    /** input bits from subdetectors */
    unsigned int m_inputBits[c_ntrgWords] = {0};

    /** ftdl (Final Trigger Decision Logic) bits. Outputs of trigger logic  */
    unsigned int m_ftdlBits[c_ntrgWords] = {0};

    /*! psnm (PreScale aNd Mask) bits. Prescaled ftdl bits
     * For instance, if the prescale factor is 20 of a ftdl bit, only 1/20 of its psnm bit would be fired.
     */
    unsigned int m_psnmBits[c_ntrgWords] = {0};

    /** types of trigger timing source defined in b2tt firmware */
    ETimingType m_timType = TTYP_NONE;

    /** trigger timing type quality */
    ETimingQuality m_timQuality = TTYQ_NONE;

    /** the prescale factor of each bit*/
    unsigned int m_prescaleBits[c_ntrgWords][c_trgWordSize] = {0};

    /** Poisson random trigger in injection veto or not */
    bool m_isPoissonInInjectionVeto = false;

    /**  Trigger Summary Information including bit (input, ftdl, psnm), timing and trigger source. */
    ClassDefOverride(TRGSummary, 7);
  };


} // end namespace Belle2
