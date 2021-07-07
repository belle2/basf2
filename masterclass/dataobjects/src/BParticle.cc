/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "masterclass/dataobjects/BParticle.h"


BParticle::BParticle(float px, float py, float pz, float e,
                     float charge, SIMPLEPID pid)
{
  m_px = px;
  m_py = py;
  m_pz = pz;
  m_e = e;
  m_charge = charge;
  m_pid = pid;
}


float BParticle::GetMass(SIMPLEPID pid)
{

  switch (pid) {
    case PHOTON:   return 0;
    case ELECTRON: return 0.51;
    case PION:     return 0.139;
    case MUON:     return 0.105;
    case KAON:     return 0.497;
    case PROTON:   return 0.938;
    case JPSI:     return 3.1;
    case D:        return 1.86;
    case DSTAR:    return 2.01;
    case B:        return 5.27;
    case PHI:      return 1.02;
    case LAMBDA0:  return 1.115683;
    case ALL:      return -1;
    default: return 0;
  }

}

float BParticle::GetMass()
{
  float m2 = m_e * m_e - m_px * m_px - m_py * m_py - m_pz * m_pz;
  if (m2 < 0) m2 = 0;
  return sqrt(m2);
}

void BParticle::SetEnergyFromMass(float mass)
{
  if (mass < 0) return;
  m_e = sqrt(mass * mass + m_px * m_px + m_py * m_py + m_pz * m_pz);

}

void BParticle::SetEnergyFromPid()
{
  SetEnergyFromMass(GetMass(m_pid));
}

int SelectParticles(TClonesArray* pin , int charge, SIMPLEPID type, TClonesArray* pout)
{
  pout->Clear();
  int nprt = 0;

  for (TIter next(pin); BParticle* p = dynamic_cast<BParticle*>(next());) {
    if (p->charge() == charge && p->pid() == type) {
      TClonesArray& list = *pout;
      new(list[nprt++]) BParticle(*p);
    }
  }
  return nprt;
}

int CombineParticles(TClonesArray* plist1 , TClonesArray* plist2 , int same, float masslow, float massup, SIMPLEPID pid,
                     TClonesArray* pout)
{
// Loop over all the particles in both lists.
  pout->Clear();
  int nprt = 0;

  for (TIter next1(plist1); BParticle* p1 = dynamic_cast<BParticle*>(next1());) {
    // the second loop
    for (TIter next2 = (plist1 != plist2
                        && same == 0) ?  TIter(plist2) : TIter(next1) ; BParticle* p2 = dynamic_cast<BParticle*>(next2());) {
      BParticle  p = *p1 + *p2; // Combine two particles into a new particle
      if (p.InMassRange(masslow, massup)) {
        p.SetPid(pid);
        TClonesArray& list = *pout;
        new(list[nprt++]) BParticle(p);     // create a new entry in kslist list of particles
      }

    }

  }
  return nprt;
}



