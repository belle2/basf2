/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef ARICHPHOTON_H
#define ARICHPHOTON_H

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  /**
   * Struct for ARICH reconstructed photon (hit related to track) information
   */
  class ARICHPhoton : public RelationsObject {

  public:

    /**
     * default constructor
     */
    ARICHPhoton(): m_hitID(0), m_thetaCer(0), m_phiCer(0),  m_mirror(0), m_sigExpPi(0), m_bkgExpPi(0), m_sigExpK(0),
      m_bkgExpK(0)
    {};

    /**
     * Constructor to allow initialization
     * @param hitID       Id of used ARICHHit
     * @param thetaCer    reconstructed cherenkov angle theta
     * @param phiCer      reconstructed cherenkov angle phi
     * @param mirror      assumed mirror plane of photon reflection
     */
    ARICHPhoton(int hitID, float thetaCer, float phiCer, int mirror): m_hitID(hitID), m_thetaCer(thetaCer),
      m_phiCer(phiCer),
      m_mirror(mirror), m_sigExpPi(0), m_bkgExpPi(0), m_sigExpK(0), m_bkgExpK(0) {};


    /**
     * Set expected signal contribution for pion and kaon hypothesis
     * @param pi signal contribution for pion hypothesis
     * @param k signal contribution for kaon hypothesis
     */
    void setSigExp(double pi, double k)
    {
      m_sigExpPi = pi;
      m_sigExpK = k;
    }

    /**
     * Set expected background contribution for pion and kaon hypothesis
     * @param pi background contribution for pion hypothesis
     * @param k background contribution for kaon hypothesis
     */
    void setBkgExp(double pi, double k)
    {
      m_bkgExpPi = pi;
      m_bkgExpK = k;
    }

    /**
     * Get expected signal contribution for pion hypothesis
     */
    float getSigExpPi() const
    {
      return m_sigExpPi;
    }

    /**
     * Get expected signal contribution for kaon hypothesis
     */
    float getSigExpK() const
    {
      return m_sigExpK;
    }

    /**
     * Get expected background contribution for pion hypothesis
     */
    float getBkgExpPi() const
    {
      return m_bkgExpPi;
    }

    /**
     * Get expected background contribution for kaon hypothesis
     */
    float getBkgExpK() const
    {
      return m_bkgExpK;
    }

    /**
     * Get reconstructed theta cherenkov angle
     */
    double getThetaCer()
    {
      return (double)m_thetaCer;
    }

    /**
     * Get reconstructed phi cherenkov angle
     */
    double getPhiCer()
    {
      return (double)m_phiCer;
    }

    /**
     * Get used mirror hypothesis (0 for no reflection)
     */
    int getMirror()
    {
      return m_mirror;
    }

    /**
     * Get ID of corresponding ARICHHit
     */
    int getHitID()
    {
      return m_hitID;
    }

  private:
    int m_hitID;       /**< id of corresponding ARICHHit */
    float m_thetaCer;  /**< reconstructed theta angle */
    float m_phiCer;    /**< reconstructed phi angle */
    int m_mirror;      /**< assumed reflection of mirror plate (0 for no reflection) */
    float m_sigExpPi;  /**< expected signal contribution for pion hypothesis */
    float m_bkgExpPi;  /**< expected background contribution for pion hypothesis */
    float m_sigExpK;   /**< expected signal contribution for kaon hypothesis */
    float m_bkgExpK;   /**< expected background contribution for kaon hypothesis */

    ClassDef(ARICHPhoton, 2); /**< ClassDef */

  };
} //Belle2 namespace

#endif // ARICHPHOTON_H
