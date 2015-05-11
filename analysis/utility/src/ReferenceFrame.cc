/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/ReferenceFrame.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

std::stack<const ReferenceFrame*> ReferenceFrame::m_reference_frames;


RestFrame::RestFrame(const Particle* particle)
{
  m_momentum = particle->get4Vector();
  m_displacement = particle->getVertex();
  m_boost = m_momentum.BoostVector();
  m_lab2restframe = TLorentzRotation(-m_boost);
}

const ReferenceFrame& ReferenceFrame::GetCurrent()
{
  if (m_reference_frames.size() > 0) {
    return *m_reference_frames.top();
  } else {
    static LabFrame _default;
    return _default;
  }
}

TVector3 RestFrame::getVertex(const Particle* particle) const
{
  // Transform Vertex from lab into rest frame:
  // 1. Subtract displacement of rest frame origin in the lab frame
  // 2. Use Lorentz Transformation to Boost Vertex vector into rest frame
  // 3. Subtract movement of vertex end due to the time difference between
  //    the former simultaneous measured vertex points (see derivation of Lorentz contraction)
  TLorentzVector a = m_lab2restframe * TLorentzVector(particle->getVertex() - m_displacement, 0);
  return a.Vect() - m_boost * a.T();
}

TLorentzVector RestFrame::getMomentum(const Particle* particle) const
{
  // 1. Boost momentum into rest frame
  return m_lab2restframe * particle->get4Vector();
}

TVector3 LabFrame::getVertex(const Particle* particle) const
{
  return particle->getVertex();
}

TLorentzVector LabFrame::getMomentum(const Particle* particle) const
{
  return particle->get4Vector();
}

TVector3 CMSFrame::getVertex(const Particle* particle) const
{
  // Transform Vertex from lab into cms frame:
  // 1. Use Lorentz Transformation to Boost Vertex vector into cms frame
  // 2. Subtract movement of vertex end due to the time difference between
  //    the former simultaneous measured vertex points (see derivation of Lorentz contraction)
  TLorentzVector a = T.rotateLabToCms() * TLorentzVector(particle->getVertex(), 0);
  return a.Vect() - T.getBoostVector().BoostVector() * a.T();
}

TLorentzVector CMSFrame::getMomentum(const Particle* particle) const
{
  // 1. Boost momentum into cms frame
  return T.rotateLabToCms() * particle->get4Vector();
}
