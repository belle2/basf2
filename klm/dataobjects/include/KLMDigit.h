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
#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>
#include <klm/eklm/dataobjects/EKLMSimHit.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>

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
      return m_fitStatus == KLM::c_ScintillatorFirmwareSuccessfulFit;
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
     * Get fit status.
     * @return Fit status.
     */
    int getFitStatus()
    {
      return m_fitStatus;
    }

    /**
     * Set fit status.
     * @param[in] s Fit status.
     */
    void setFitStatus(int s)
    {
      m_fitStatus = s;
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

    /** Generated number of photoelectrons (MC only). */
    int m_GeneratedNPE;

    /** Fit status. */
    int m_fitStatus;

    /** MC time at SiPM. */
    float m_sMCTime;

    /** Class version. */
    ClassDefOverride(Belle2::KLMDigit, 1);

  };

}
