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

#include <TMatrixD.h>

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
  if (m_reference_frames.empty()) {
    static LabFrame _default;
    return _default;
  } else {
    return *m_reference_frames.top();
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

TMatrixFSym RestFrame::getMomentumErrorMatrix(const Particle* particle) const
{
  TMatrixD lorentzrot(4, 4);

  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      lorentzrot(i, j) = m_lab2restframe(i, j);

  return particle->getMomentumErrorMatrix().Similarity(lorentzrot);
}

TVector3 LabFrame::getVertex(const Particle* particle) const
{
  return particle->getVertex();
}

TLorentzVector LabFrame::getMomentum(const Particle* particle) const
{
  return particle->get4Vector();
}

TMatrixFSym LabFrame::getMomentumErrorMatrix(const Particle* particle) const
{
  return particle->getMomentumErrorMatrix();
}

TVector3 CMSFrame::getVertex(const Particle* particle) const
{
  // Transform Vertex from lab into cms frame:
  // 1. Use Lorentz Transformation to Boost Vertex vector into cms frame
  // 2. Subtract movement of vertex end due to the time difference between
  //    the former simultaneous measured vertex points (see derivation of Lorentz contraction)
  TLorentzVector a = m_transform.rotateLabToCms() * TLorentzVector(particle->getVertex(), 0);
  return a.Vect() - m_transform.getBoostVector().BoostVector() * a.T();
}

TLorentzVector CMSFrame::getMomentum(const Particle* particle) const
{
  // 1. Boost momentum into cms frame
  return m_transform.rotateLabToCms() * particle->get4Vector();
}

TMatrixFSym CMSFrame::getMomentumErrorMatrix(const Particle* particle) const
{
  TMatrixD lorentzrot(4, 4);

  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      lorentzrot(i, j) = m_transform.rotateLabToCms()(i, j);

  return particle->getMomentumErrorMatrix().Similarity(lorentzrot);
}
