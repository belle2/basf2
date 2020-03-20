/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/EKLMSimHit.h>
#include <klm/dataobjects/eklm/ElementNumbersSingleton.h>
#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>

/* Belle 2 headers. */
#include <framework/dataobjects/DigitBase.h>

namespace Belle2 {

  /**
   * KLM digit (class representing a digitized hit in RPCs or scintillators).
   */
  class KLMDigit : public DigitBase {

  public:

    /**
     * Constructor.
     */
    KLMDigit();

    /**
     * Constructor.
     * @param[in] hit EKLM simulation hit.
     */
    KLMDigit(const EKLMSimHit* hit);

    /**
     * Destructor.
     */
    ~KLMDigit()
    {
    }

    /**
     * Get section number.
     * @return Section number.
     */
    int getSection() const
    {
      return m_Section;
    }

    /**
     * Set section number.
     * @param[in] section Section number.
     */
    void setSection(int section)
    {
      m_Section = section;
    }

    /**
     * Get sector number.
     * @return Sector number.
     */
    int getSector() const
    {
      return m_Sector;
    }

    /**
     * Set sector number.
     * @param[in] sector Sector number.
     */
    void setSector(int sector)
    {
      m_Sector = sector;
    }

    /**
     * Get layer number.
     * @return Layer number.
     */
    int getLayer() const
    {
      return m_Layer;
    }

    /**
     * Set layer number.
     * @param[in] layer Layer number.
     */
    void setLayer(int layer)
    {
      m_Layer = layer;
    }

    /**
     * Get plane number.
     * @return Plane number.
     */
    int getPlane() const
    {
      return m_Plane;
    }

    /**
     * Set plane number.
     * @param[in] plane Plane number.
     */
    void setPlane(int plane)
    {
      m_Plane = plane;
    }

    /**
     * Get strip number.
     * @return Strip number.
     */
    int getStrip() const
    {
      return m_Strip;
    }

    /**
     * Set strip number.
     * @param[in] strip Strip number.
     */
    void setStrip(int strip)
    {
      m_Strip = strip;
    }

    /**
     * Get charge.
     * @return Charge.
     */
    uint16_t getCharge() const
    {
      return m_Charge;
    }

    /**
     * Set charge.
     * @param[in] charge Charge.
     */
    void setCharge(uint16_t charge)
    {
      m_Charge = charge;
    }

    /**
     * Get CTIME.
     * @return CTIME.
     */
    uint16_t getCTime() const
    {
      return m_CTime;
    }

    /**
     * Set CTIME.
     * @param[in] ctime CTime
     */
    void setCTime(uint16_t ctime)
    {
      m_CTime = ctime;
    }

    /**
     * Get TDC.
     * @return TDC.
     */
    uint16_t getTDC() const
    {
      return m_TDC;
    }

    /**
     * Set TDC.
     * @param[in] tdc TDC.
     */
    void setTDC(uint16_t tdc)
    {
      m_TDC = tdc;
    }

    /**
      * Get hit time.
      * @return Hit time.
      */
    float getTime() const
    {
      return m_Time;
    }

    /**
     * Set hit time.
     * @param[in] time hit time.
     */
    void setTime(float time)
    {
      m_Time = time;
    }

    /**
     * Get energy deposit.
     * @return Energy deposit.
     */
    float getEnergyDeposit() const
    {
      return m_EnergyDeposit;
    }

    /**
     * Set EnergyDeposit.
     * @param[in] eDep Energy deposit.
     */
    void setEnergyDeposit(float eDep)
    {
      m_EnergyDeposit = eDep;
    }

    /**
     * Increase energy deposit.
     * @param[in] eDep Energy deposit.
     */
    void increaseEnergyDeposit(float deltaEnergyDeposit)
    {
      m_EnergyDeposit += deltaEnergyDeposit;
    }

    /**
     * Get the lund code of the (leading) particle.
     * @return Particle code.
     */
    int getPDG() const
    {
      return m_PDG;
    }

    /**
     * Set the lund code of the (leading) particle
     * @param[in] pdg Particle code.
     */
    void setPDG(int pdg)
    {
      m_PDG = pdg;
    }

    /**
     * Get number of photoelectrons (fit result).
     * @return Number of photoelectrons.
     */
    float getNPE() const
    {
      /*
       * TODO: the photoelectron / charge conversion constant should be
       * determined from calibration.
       */
      return float(m_Charge) / 32;
    }

    /**
     * Get generated number of photoelectrons.
     * @return Number of photoelectrons.
     */
    int getGeneratedNPE() const
    {
      return m_GeneratedNPE;
    }

    /**
     * Set generated number of photoelectrons.
     * @param[in] npe Number of photoelectrons.
     */
    void setGeneratedNPE(int npe)
    {
      m_GeneratedNPE = npe;
    }

    /**
     * Whether hit could be used late (if it passed discriminator threshold)
     * @return True if could be used.
     */
    bool isGood() const
    {
      return m_FitStatus == KLM::c_ScintillatorFirmwareSuccessfulFit;
    }

    /**
     * Get fit status.
     * @return Fit status.
     */
    int getFitStatus() const
    {
      return m_FitStatus;
    }

    /**
     * Set fit status.
     * @param[in] s Fit status.
     */
    void setFitStatus(int s)
    {
      m_FitStatus = s;
    }

    /**
     * Get MC time.
     * @return MC time.
     */
    float getMCTime() const
    {
      return m_MCTime;
    }

    /**
     * Set MC time.
     * @param[in] time MC time.
     */
    void setMCTime(float time)
    {
      m_MCTime = time;
    }

    /**
     * Get SiPM MC time.
     * @return Time.
     */
    float getSiPMMCTime() const
    {
      return m_sMCTime;
    }

    /**
     * Set SiPM MC time.
     * @param[in] t Time.
     */
    void setSiPMMCTime(float t)
    {
      m_sMCTime = t;
    }

  protected:

    /** Element numbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbers; //! ROOT streamer

    /** Number of section. */
    int m_Section;

    /** Number of sector. */
    int m_Sector;

    /** Number of layer. */
    int m_Layer;

    /** Number of plane. */
    int m_Plane;

    /** Number of strip. */
    int m_Strip;

    /** Charge (integral of ADC signal). */
    uint16_t m_Charge;

    /** CTIME (time provided by B2TT). */
    uint16_t m_CTime;

    /** TDC (time provided by ASIC). */
    uint16_t m_TDC;

    /** Time of the hit. */
    float m_Time;

    /** Energy deposition. */
    float m_EnergyDeposit;

    /** PDG code of the (leading) particle. */
    int m_PDG;

    /** Generated number of photoelectrons (MC only). */
    int m_GeneratedNPE;

    /** Fit status. */
    int m_FitStatus;

    /** MC time. */
    float m_MCTime;

    /** MC time at SiPM. */
    float m_sMCTime;

    /** Class version. */
    ClassDefOverride(Belle2::KLMDigit, 1);

  };

}
