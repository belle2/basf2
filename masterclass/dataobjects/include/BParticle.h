//+
// File : BParticle.h
// Description : class to contain particle info.
//-

#pragma once

#include "TObject.h"
#include "TClonesArray.h"

enum SIMPLEPID {PHOTON, ELECTRON, PION, MUON, KAON, PROTON, JPSI, D, DSTAR, B, PHI, LAMBDA0, ALL };

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

public:
//! Default constructor
  BParticle() {};
//! Constructor using the particle
  BParticle(const BParticle&) = default;

  /**
   * Constructor of fthe particle
   * @param px - x component of the particle momentum
   * @param py - y component of the particle momentum
   * @param pz - z component of the particle momentum
   * @param energy  - energy of the particle
   * @param charge  - unit charge of the particle
   * @param pid - particle identity - hypothesis with the highest maximum likelihood
   */
  BParticle(float px, float py, float pz, float energy, float charge, SIMPLEPID pid);

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

  int  InMassRange(float mlower, float mupper) { float m = GetMass(); if (m >= mlower && m <= mupper) return 1; else return 0; };

//! Adds to particles
  BParticle operator+(const BParticle& b)
  {

    BParticle particle(
      px() + b.px(),
      py() + b.py(),
      pz() + b.pz(),
      e()  + b.e(),
      charge() + b.charge(),
      PHOTON // wrong
    );

    return particle;
  };

//! Assign a particle
  BParticle& operator=(const BParticle&  p)
  {
    m_px = p.px();
    m_py = p.py();
    m_pz = p.pz();
    m_e  = p.e();
    m_charge  = p.charge();
    m_pid  =  p.pid();
    return *this;
  };

  ClassDef(BParticle, 1)    // Simple particle class
};

int SelectParticles(TClonesArray* pin , int charge, SIMPLEPID type, TClonesArray* pout);

int CombineParticles(TClonesArray* plist1 , TClonesArray* plist2 , int same, float masslow, float massup, SIMPLEPID pid,
                     TClonesArray* pout);
