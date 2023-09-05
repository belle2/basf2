/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "masterclass/dataobjects/BEvent.h"
#include <iostream>

BEvent::BEvent() :
  m_evno{0},
  m_nprt{0}
{
  m_particles = new TClonesArray("BParticle", 500);
}

BEvent::~BEvent()
{
  delete m_particles;
}

void BEvent::EventNo(int evt)
{
  m_evno = evt;
}

int BEvent::EventNo()
{
  return m_evno;
}

void BEvent::AddTrack(float px, float py, float pz, float e,
                      float charge, SIMPLEPID pid)
{
  // const TClonesArray& particles = *m_particles;
  // new (particles[m_nprt++]) BParticle(px, py, pz, e, charge, pid);
  new ((*m_particles)[m_nprt++]) BParticle(px, py, pz, e, charge, pid);
}

void BEvent::AddTrack(float px, float py, float pz, float e,
                      float charge, SIMPLEPID pid,
                      float logL_e, float logL_mu, float logL_pi, float logL_k, float logL_p, float logL_d)
{
  // const TClonesArray& particles = *m_particles;
  // new (particles[m_nprt++]) BParticle(px, py, pz, e, charge, pid);
  new ((*m_particles)[m_nprt++]) BParticle(px, py, pz, e, charge, pid, logL_e, logL_mu, logL_pi, logL_k, logL_p, logL_d);
}

int BEvent::NParticles()
{
  return m_nprt;
}

TClonesArray* BEvent::GetParticleList()
{
  return m_particles;
}

void BEvent::Clear(Option_t*)
{
  m_particles->Clear();
  m_nprt = 0;
}

