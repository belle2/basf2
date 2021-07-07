/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * @param thetaCer    reconstructed cherenkov angle theta for 1st aerogel
     * @param phiCer      reconstructed cherenkov angle phi for 1st aerogel
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
     * Set hit phi angle in track coordinates
     */
    void setPhiCerTrk(float phi)
    {
      m_phiCerTrk = phi;
    }

    /**
     * Set X-Y position of hit
     */
    void setXY(float x, float y)
    {
      m_x = x;
      m_y = y;
    }

    /**
     * Set id of hit module
     */
    void setModuleID(int modID)
    {
      m_module = modID;
    }

    /**
     * set channel (asic) of hit
     */
    void setChannel(int chn)
    {
      m_channel = chn;
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
     * Set n cos(theta_ch)
     * @param n cos(theta_ch) for all pid hypotheses
     */
    void setNCosThetaCh(const double* n_cos_theta_ch)
    {
      m_nCosThetaCh_e = (float)n_cos_theta_ch[0];
      m_nCosThetaCh_mu = (float)n_cos_theta_ch[1];
      m_nCosThetaCh_pi = (float)n_cos_theta_ch[2];
      m_nCosThetaCh_K = (float)n_cos_theta_ch[3];
      m_nCosThetaCh_p = (float)n_cos_theta_ch[4];
    }

    /**
     * Set phi_ch
     * @param phi_ch for all pid hypotheses
     */
    void setPhiCh(const double* phi_ch)
    {
      m_phiCh_e = (float)phi_ch[0];
      m_phiCh_mu = (float)phi_ch[1];
      m_phiCh_pi = (float)phi_ch[2];
      m_phiCh_K = (float)phi_ch[3];
      m_phiCh_p = (float)phi_ch[4];
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
     * Get n cos(theta_ch)
     * @param n cos(theta_ch) for all pid hypotheses
     */
    float getNCosThetaCh(const Const::ChargedStable& part) const
    {
      if (part == Const::electron) return m_nCosThetaCh_e;
      if (part == Const::muon) return m_nCosThetaCh_mu;
      if (part == Const::pion) return m_nCosThetaCh_pi;
      if (part == Const::kaon) return m_nCosThetaCh_K;
      if (part == Const::proton) return m_nCosThetaCh_p;
      else return 0;
    }

    /**
     * Get phi_ch
     * @param phi_ch for all pid hypotheses
     */
    float getPhiCh(const Const::ChargedStable& part) const
    {
      if (part == Const::electron) return m_phiCh_e;
      if (part == Const::muon) return m_phiCh_mu;
      if (part == Const::pion) return m_phiCh_pi;
      if (part == Const::kaon) return m_phiCh_K;
      if (part == Const::proton) return m_phiCh_p;
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
     * Get reconstructed phi cherenkov angle in track coordinate system
     */
    double getPhiCerTrk()
    {
      return (double)m_phiCerTrk;
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

    /**
     * Get hit X position
     */
    double getX()
    {
      return (double)m_x;
    }

    /**
     * Get hit Y position
     */
    double getY()
    {
      return (double)m_y;
    }

    /**
     * Get hit module ID
     */
    int getModuleID()
    {
      return m_module;
    }

    /**
     * Get hit channel (asic)
     */
    int getChannel()
    {
      return m_channel;
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
    float m_nCosThetaCh_e = 0.; /**< n cos(theta_ch) for e hypotheses */
    float m_nCosThetaCh_mu = 0.; /**< n cos(theta_ch) for mu hypotheses */
    float m_nCosThetaCh_pi = 0.; /**< n cos(theta_ch) for pi hypotheses */
    float m_nCosThetaCh_K = 0.; /**< n cos(theta_ch) for K hypotheses */
    float m_nCosThetaCh_p = 0.; /**< n cos(theta_ch) for p hypotheses */
    float m_phiCh_e = 0.; /**< n phi_ch for e hypotheses */
    float m_phiCh_mu = 0.; /**< n phi_ch for mu hypotheses */
    float m_phiCh_pi = 0.; /**< n phi_ch for pi hypotheses */
    float m_phiCh_K = 0.; /**< n phi_ch for K hypotheses */
    float m_phiCh_p = 0.; /**< n phi_ch for p hypotheses */
    float m_x = 0; /**< x of the hit position */
    float m_y = 0; /**< y of the hit position */
    int m_module = 0; /**< hit module */
    int m_channel = 0; /**< hit channel */
    float m_phiCerTrk = 0.;  /**< reconstructed phi angle in track coordinate system */

    ClassDef(ARICHPhoton, 5); /**< ClassDef */

  };
} //Belle2 namespace

#endif // ARICHPHOTON_H
