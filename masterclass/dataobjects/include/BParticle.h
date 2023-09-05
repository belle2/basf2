/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TClonesArray.h>

enum SIMPLEPID {PHOTON, ELECTRON, PION, MUON, KAON, PROTON, JPSI, D, DSTAR, B, PHI, LAMBDA0, DEUTERON, ALL };

//! The Class for Masterclass particle information
//! This class provides the data structure of the particle that are used by Belle II Masterclass application.
class BParticle : public TObject {

private:
  float m_px; /*!< x component of the particle momentum */
  float m_py; /*!< y component of the particle momentum */
  float m_pz; /*!< z component of the particle momentum */
  float m_e;  /*!< energy of the particle */
  float m_charge;  /*!< charge of the particle */
  SIMPLEPID m_pid;  /*!< particle identity */
  float m_logL_e;  /*!< log likelihood of the electron hypothesis */
  float m_logL_mu;  /*!< log likelihood of the muon hypothesis */
  float m_logL_pi;  /*!< log likelihood of the pion hypothesis */
  float m_logL_k;  /*!< log likelihood of the kaon hypothesis */
  float m_logL_p;  /*!< log likelihood of the proton hypothesis */
  float m_logL_d;  /*!< log likelihood of the deuteron hypothesis */

public:
//! Default constructor
  BParticle() {};
//! Constructor using the particle
  BParticle(const BParticle&) = default;
//! Operator to add particle
  BParticle& operator+=(const BParticle&);

  /**
   * Constructor of fthe particle
   * @param px - x component of the particle momentum
   * @param py - y component of the particle momentum
   * @param pz - z component of the particle momentum
   * @param energy  - energy of the particle
   * @param charge  - unit charge of the particle
   * @param pid - particle identity - hypothesis with the highest maximum likelihood
   * @param logL_e - log likelihood of the electron hypothesis
    * @param logL_mu - log likelihood of the muon hypothesis
    * @param logL_pi - log likelihood of the pion hypothesis
    * @param logL_k - log likelihood of the kaon hypothesis
    * @param logL_p - log likelihood of the proton hypothesis
    * @param logL_d - log likelihood of the deuteron hypothesis
   */
  BParticle(float px, float py, float pz, float energy, float charge, SIMPLEPID pid,
            float logL_e = 0, float logL_mu = 0, float logL_pi = 0, float logL_k = 0, float logL_p = 0, float logL_d = 0);

//! Default destructor
  ~BParticle() {};


  /**
   * @return x component of the particle momentum GeV/c
   */
  float px() const { return m_px; };
  /**
   * @return y component of the particle momentum GeV/c
   */
  float py() const { return m_py; };
  /**z
   * @return z component of the particle momentum GeV/c
   */
  float pz() const { return m_pz; };
  /**
   * @return particle energy GeV
   */
  float e() const { return m_e; };
  /**
   * Get the momentum of the particle
   * @return particle momentum GeV/c
   */
  float GetMomentum() const { return sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz); };
  /**
   * Get the transverse momentum of the particle
   * @return transverse particle momentum GeV/c
   */
  float GetTransverseMomentum() const { return sqrt(m_px * m_px + m_py * m_py); };
  /**
   * Get the particle charge
   * @return particle unit charge
   */
  float charge() const { return m_charge; };

  /**
   * Get the particle identity
   * @return the particle identity
   */

  SIMPLEPID pid() const { return m_pid; };
  float logL_e() const { return m_logL_e; };
  float logL_mu() const { return m_logL_mu; };
  float logL_pi() const { return m_logL_pi; };
  float logL_k() const { return m_logL_k; };
  float logL_p() const { return m_logL_p; };
  float logL_d() const { return m_logL_d; };
  /**
   * Get the log likelihood for a particle hypothesis
    * @param pid particle identity
   * @return the log likelihood for a particle hypothesis
   */
  float GetLogL(SIMPLEPID pid);
  /**
    * Get the log likelihood ratio for a particle hypothesis with respect to all other hypotheses
    * @param pid particle identity
    * @return the log likelihood ratio for a particle hypothesis
    */
  float GetPID(SIMPLEPID pid);
  /**
    * Get the binary log likelihood ratio for a particle hypothesis with respect to a second hypothesis
    * @param pid1 particle identity
    * @param pid2 particle identity
    * @return the binary log likelihood ratio for two particle hypotheses
    */
  float GetBinPID(SIMPLEPID pid1, SIMPLEPID pid2);

  /**
   * Get the mass of the particle with the identity
   * @param pid particle identity
   * @return mass of the particle GeV/c2
   */

  float GetMass(SIMPLEPID pid);
  /**
   * Get the mass of the particle
   * @return mass of the particle GeV/c2
   */

  float GetMass();

  /**
  * Recalculate energy from particle mass  and momentum
  * @param mass of the particle
  */

  void SetEnergyFromMass(float mass);
  /**
  * Recalculate energy from particle identity  and momentum
  */
  void SetEnergyFromPid();
  /**
  * Set particle identity
  * @param pid - particle identity
  */

  void SetPid(SIMPLEPID pid) { m_pid = pid; };

  /**
  * Test if the mass is in range
  * @param mlower lower limit of the range
  * @param mupper upper limit of the range
  */

  int InMassRange(float mlower, float mupper) { float m = GetMass(); if (m >= mlower && m <= mupper) return 1; else return 0; };

//! Assign a particle
  BParticle& operator=(const BParticle&  p)
  {
    m_px = p.px();
    m_py = p.py();
    m_pz = p.pz();
    m_e  = p.e();
    m_charge  = p.charge();
    m_pid  =  p.pid();
    m_logL_e = p.logL_e();
    m_logL_mu = p.logL_mu();
    m_logL_pi = p.logL_pi();
    m_logL_k = p.logL_k();
    m_logL_p = p.logL_p();
    m_logL_d = p.logL_d();
    return *this;
  };

  ClassDef(BParticle, 2)    // Simple particle class
};

int SelectParticles(TClonesArray* pin, int charge, SIMPLEPID type, TClonesArray* pout);

int CombineParticles(TClonesArray* plist1, TClonesArray* plist2, int same, float masslow, float massup, SIMPLEPID pid,
                     TClonesArray* pout);
