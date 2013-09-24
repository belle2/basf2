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

/* External headers. */
#include <CLHEP/Vector/ThreeVector.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitBase.h>
#include <eklm/dataobjects/EKLMHitMCTime.h>
#include <eklm/dataobjects/EKLMSim2Hit.h>
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Main reconstruction hit class. Contains information about the
   * hitted strips.
   */
  class EKLMDigit : public RelationsObject, public EKLMHitBase,
    public EKLMHitCoord, public EKLMHitMCTime {

  public:

    /**
     * Constructor.
     */
    EKLMDigit();

    /**
     * Constructor from the EKLMSim2Hit.
     * @param[in] Hit EKLMSim2Hit.
     */
    EKLMDigit(const EKLMSim2Hit* Hit);

    /**
     * Destructor.
     */
    ~EKLMDigit() {};

    /**
     * Get number of photoelectrons (fit result).
     * @return Number of photoelectrons.
     */
    float getNPE() const;

    /**
     * Set the number of photoelectrons (fit result).
     * @param[in] npe Number of photoelectrons.
     */
    void setNPE(float npe);

    /**
     * Get generated number of photoelectrons.
     * @return Number of photoelectrons.
     */
    int getGeneratedNPE();

    /**
     * Set generated number of photoelectrons.
     * @param[in] npe Number of photoelectrons.
     */
    void setGeneratedNPE(int npe);

    /**
     * Get volume identifier.
     * @return Identifier.
     */
    int getVolumeID() const;

    /**
     * Set volume identifier.
     * @param[in] id Identifier.
     */
    void setVolumeID(int id);

    /**
     * Whether hit could be used late (if it passed discriminator threshold)
     * (getter).
     * @return True if could be used.
     */
    bool isGood() const;

    /**
     * Whether hit could be used late (if it passed discriminator threshold)
     * (setter).
     * @param[in] status True if could be used.
     */
    void isGood(bool status) ;

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

    /** Number of plane. */
    int m_Plane;

    /** Number of strip. */
    int m_Strip;

    /** If hit passes threshold. */
    bool m_good;

    /** Number of photo electrons. */
    float m_NPE;

    /** Generated number of photoelectrons (MC only). */
    int m_generatedNPE;

    /** Fit status. */
    int m_fitStatus;

    /** Volume identifier. */
    int m_volid;

    /** MC time at SiPM. */
    float m_sMCTime;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMDigit, 2);

  };

}

#endif

