/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGIT_H
#define EKLMDIGIT_H

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitBase.h>
#include <eklm/dataobjects/EKLMHitMCTime.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <eklm/dataobjects/EKLMFPGAFit.h>
#include <framework/dataobjects/DigitBase.h>

namespace Belle2 {

  /**
   * Main reconstruction hit class. Contains information about the
   * hitted strips.
   */
  class EKLMDigit : public DigitBase, public EKLMHitBase,
    public EKLMHitGlobalCoord, public EKLMHitMCTime {

  public:

    /**
     * Constructor.
     */
    EKLMDigit();

    /**
     * Constructor from the EKLMSim2Hit.
     * @param[in] Hit EKLMSim2Hit.
     */
    EKLMDigit(const EKLMSimHit* Hit);

    /**
     * Destructor.
     */
    ~EKLMDigit() {};

    /**
     * Get unique channel identifier.
     */
    unsigned int getUniqueChannelID() const;

    /**
     * The pile-up method.
     */
    EAppendStatus addBGDigit(const DigitBase* bg);

    /**
     * Get charge.
     * @return Charge.
     */
    uint16_t getCharge() const;

    /**
     * Set charge.
     * @param[in] charge Charge.
     */
    void setCharge(uint16_t charge);

    /**
     * Get CTIME.
     * @return CTIME.
     */
    uint16_t getCTime() const;

    /**
     * Set CTIME.
     * @param[in] ctime CTime
     */
    void setCTime(uint16_t ctime);

    /**
     * Get TDC.
     * @return TDC.
     */
    uint16_t getTDC() const;

    /**
     * Set TDC.
     * @param[in] tdc TDC.
     */
    void setTDC(uint16_t tdc);

    /**
     * Get trigger CTIME.
     * @return Trigger CTIME.
     */
    uint16_t getTriggerCTime() const;

    /**
     * Set trigger CTIME.
     * @param[in] ctime Trigger CTime
     */
    void setTriggerCTime(uint16_t ctime);

    /**
     * Get relative CTIME (event - trigger).
     * @return Relative CTIME.
     */
    int getRelativeCTime() const;

    /**
     * Get number of photoelectrons (fit result).
     * @return Number of photoelectrons.
     */
    float getNPE() const;

    /**
     * Get generated number of photoelectrons.
     * @return Number of photoelectrons.
     */
    int getGeneratedNPE() const;

    /**
     * Set generated number of photoelectrons.
     * @param[in] npe Number of photoelectrons.
     */
    void setGeneratedNPE(int npe);

    /**
     * Whether hit could be used late (if it passed discriminator threshold)
     * @return True if could be used.
     */
    bool isGood() const;

    /**
     * Get plane number.
     * @return Plane number.
     */
    int getPlane() const;

    /**
     * Set plane number.
     * @param[in] Plane Plane number.
     */
    void setPlane(int Plane);

    /**
     * Get strip number.
     * @return Strip number.
     */
    int getStrip() const;

    /**
     * Set strip number.
     * @param[in] Strip Strip number.
     */
    void setStrip(int Strip);

    /**
     * Get fit status.
     * @return Fit status.
     */
    int getFitStatus();

    /**
     * Set fit status.
     * @param[in] s Fit status.
     */
    void setFitStatus(int s);

    /**
     * Get SiPM MC time.
     * @return Time.
     */
    float getSiPMMCTime() const;

    /**
     * Set SiPM MC time.
     * @param[in] t Time.
     */
    void setSiPMMCTime(float t);

  private:

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

    /** Trigger CTIME. */
    uint16_t m_TriggerCTime;

    /** Generated number of photoelectrons (MC only). */
    int m_generatedNPE;

    /** Fit status. */
    int m_fitStatus;

    /** MC time at SiPM. */
    float m_sMCTime;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMDigit, 10);

  };

}

#endif

