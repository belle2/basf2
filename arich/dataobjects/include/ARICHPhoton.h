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
    ARICHPhoton() {};

    /**
     * Constructor to allow initialization
     * @param hitID       Id of used ARICHHit
     * @param thetaCer    reconstructed cherenkov angle theta
     * @param phiCer      reconstructed cherenkov angle phi
     * @param mirror      assumed mirror plane of photon reflection
     */
    ARICHPhoton(int hitID, float thetaCer, float phiCer, int mirror): m_hitID(hitID), m_thetaCer(thetaCer),
      m_phiCer(phiCer),
      m_mirror(mirror) {};

    /**
     * Set ID of corresponding ARICHHit
     */
    void setHitID(int id)
    {
      m_hitID = id;
    }

    /**
     * Set expected signal contribution
     * @param sigExp array of expected signal photons on pad for all pid hypotheses
     */
    void setSigExp(const double* sigExp)
    {
      m_sigExp_e = (float)sigExp[0];
      m_sigExp_mu = (float)sigExp[1];
      m_sigExp_pi = (float)sigExp[2];
      m_sigExp_K = (float)sigExp[3];
      m_sigExp_p = (float)sigExp[4];
    }

    /**
     * Set expected background contribution
     * @param bkgExp array of expected background photons on pad for all pid hypotheses
     */
    void setBkgExp(const double* bkgExp)
    {
      m_bkgExp_e = (float)bkgExp[0];
      m_bkgExp_mu = (float)bkgExp[1];
      m_bkgExp_pi = (float)bkgExp[2];
      m_bkgExp_K = (float)bkgExp[3];
      m_bkgExp_p = (float)bkgExp[4];
    }

    /**
     * Get expected signal contribution for given pid hypothesis
     * @param part pid hypothesis
     */
    float getSigExp(const Const::ChargedStable& part) const
    {
      if (part == Const::electron) return m_sigExp_e;
      if (part == Const::muon) return m_sigExp_mu;
      if (part == Const::pion) return m_sigExp_pi;
      if (part == Const::kaon) return m_sigExp_K;
      if (part == Const::proton) return m_sigExp_p;
      else return 0;
    }

    /**
     * Get expected background  contribution for given pid hypothesis
     * @param part pid hypothesis
     */
    float getBkgExp(const Const::ChargedStable& part) const
    {
      if (part == Const::electron) return m_bkgExp_e;
      if (part == Const::muon) return m_bkgExp_mu;
      if (part == Const::pion) return m_bkgExp_pi;
      if (part == Const::kaon) return m_bkgExp_K;
      if (part == Const::proton) return m_bkgExp_p;
      else return 0;
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

    int m_hitID = 0;     /**< id of corresponding ARICHHit */
    float m_thetaCer = 0.; /**< reconstructed theta angle */
    float m_phiCer = 0.;  /**< reconstructed phi angle */
    int m_mirror = 0;    /**< assumed reflection of mirror plate (0 for no reflection) */
    float m_sigExp_e = 0.; /**< number of expected signal photons on pad for e hypotheses */
    float m_bkgExp_e = 0.; /**< number of expected background photons on pad for e hypotheses */
    float m_sigExp_mu = 0.; /**< number of expected signal photons on pad for mu hypotheses */
    float m_bkgExp_mu = 0.; /**< number of expected background photons on pad for mu hypotheses */
    float m_sigExp_pi = 0.; /**< number of expected signal photons on pad for pi hypotheses */
    float m_bkgExp_pi = 0.; /**< number of expected background photons on pad for pi hypotheses */
    float m_sigExp_K = 0.; /**< number of expected signal photons on pad for K hypotheses */
    float m_bkgExp_K = 0.; /**< number of expected background photons on pad for K hypotheses */
    float m_sigExp_p = 0.; /**< number of expected signal photons on pad for p hypotheses */
    float m_bkgExp_p = 0.; /**< number of expected background photons on pad for p hypotheses */



    ClassDef(ARICHPhoton, 3); /**< ClassDef */

  };
} //Belle2 namespace

#endif // ARICHPHOTON_H
