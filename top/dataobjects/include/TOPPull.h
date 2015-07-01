/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {


  /**
   * Class to store photon pull in respect to PDF used in reconstruction
   */
  class TOPPull : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPPull(): m_channelID(0), m_t(0), m_t0(0), m_sigma(0), m_phiCer(0), m_wt(0)
    {}

    /**
     * Full constructor
     */
    TOPPull(int channelID, float t, float t0, float sigma, float phiCer, float wt):
      m_channelID(channelID), m_t(t), m_t0(t0), m_sigma(sigma), m_phiCer(phiCer), m_wt(wt)
    {}

    /**
     * Return photon software channel ID
     * @return channel ID
     */
    int getChannelID() const { return m_channelID; }

    /**
     * Return photon time
     * @return photon time
     */
    double getTime() const { return m_t; }

    /**
     * Return PDF mean
     * @return PDF mean time
     */
    double getMeanTime() const { return m_t0; }

    /**
     * Return PDF width
     * @return sigma
     */
    double getSigma() const { return m_sigma; }

    /**
     * Return time difference btw. photon and PDF
     * @return time difference
     */
    double getTimeDiff() const { return m_t - m_t0; }

    /**
     * Return pull
     * @return pull
     */
    double getPull() const { return (m_t - m_t0) / m_sigma; }

    /**
     * Return Cerenkov azimuthal angle
     * @return azimuthal angle
     */
    double getPhiCer() const { return m_phiCer; }

    /**
     * Return probability of photon belonging to this PDF
     * @return weight
     */
    double getWeight() const { return m_wt; }

    /**
     * Checks whether it corresponds to signal PDF
     * @return true (signal) or false (background)
     */
    bool isSignal() const { return m_sigma > 0;}

  private:
    int m_channelID; /**< software channel ID (1-based) */
    float m_t;       /**< photon time */
    float m_t0;      /**< PDF mean time (signal) or minimal PDF mean (background) */
    float m_sigma;   /**< PDF sigma (signal) or 0 (background) */
    float m_phiCer;  /**< azimuthal Cerenkov angle */
    float m_wt;      /**< weight */

    ClassDef(TOPPull, 1); /**< ClassDef */

  };

} // end namespace Belle2



