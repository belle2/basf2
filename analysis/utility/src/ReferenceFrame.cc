/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/ReferenceFrame.h>

#include <TMatrixD.h>

using namespace Belle2;

std::stack<const ReferenceFrame*> ReferenceFrame::m_reference_frames;


RestFrame::RestFrame(const Particle* particle) :
  m_momentum(particle->get4Vector()),
  m_displacement(particle->getVertex()),
  m_boost(m_momentum.BoostToCM()),
  m_lab2restframe(m_boost)
{
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

ROOT::Math::XYZVector RestFrame::getVertex(const ROOT::Math::XYZVector& vector) const
{
  // Transform Vertex from lab into rest frame:
  // 1. Subtract displacement of rest frame origin in the lab frame
  // 2. Use Lorentz Transformation to Boost Vertex vector into rest frame
  // 3. Subtract movement of vertex end due to the time difference between
  //    the former simultaneous measured vertex points (see derivation of Lorentz contraction)
  ROOT::Math::XYZVector v(vector - m_displacement);
  ROOT::Math::PxPyPzEVector a = m_lab2restframe * ROOT::Math::PxPyPzEVector(v.x(), v.y(), v.z(), 0);
  return a.Vect() - m_boost * a.T();
}

ROOT::Math::PxPyPzEVector RestFrame::getMomentum(const ROOT::Math::PxPyPzEVector& vector) const
{
  // 1. Boost momentum into rest frame
  return m_lab2restframe * vector;
}

TMatrixFSym RestFrame::getMomentumErrorMatrix(const TMatrixFSym& matrix) const
{
  double lorentzrotationvalues[16];
  m_lab2restframe.GetLorentzRotation(lorentzrotationvalues);
  TMatrixD lorentzrot(4, 4, lorentzrotationvalues);

  TMatrixFSym tmp_matrix(matrix);

  return tmp_matrix.Similarity(lorentzrot);
}

TMatrixFSym RestFrame::getVertexErrorMatrix(const TMatrixFSym& matrix) const
{
  double lorentzrotationvalues[16];
  m_lab2restframe.GetLorentzRotation(lorentzrotationvalues);
  TMatrixD lorentzrot(4, 3);

  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 3; ++j)
      lorentzrot(i, j) = lorentzrotationvalues[4 * i + j];

  TMatrixFSym tmp_matrix(matrix);
  auto rotated_error_matrix = tmp_matrix.Similarity(lorentzrot);

  TMatrixD timeshift(3, 4);
  timeshift.Zero();
  timeshift(0, 0) = 1;
  timeshift(1, 1) = 1;
  timeshift(2, 2) = 1;
  timeshift(0, 3) = m_boost.x();
  timeshift(1, 3) = m_boost.y();
  timeshift(2, 3) = m_boost.z();

  return rotated_error_matrix.Similarity(timeshift);
}

ROOT::Math::XYZVector LabFrame::getVertex(const ROOT::Math::XYZVector& vector) const
{
  return vector;
}

ROOT::Math::PxPyPzEVector LabFrame::getMomentum(const ROOT::Math::PxPyPzEVector& vector) const
{
  return vector;
}

TMatrixFSym LabFrame::getMomentumErrorMatrix(const TMatrixFSym& matrix) const
{
  return matrix;
}

TMatrixFSym LabFrame::getVertexErrorMatrix(const TMatrixFSym& matrix) const
{
  return matrix;
}

ROOT::Math::XYZVector CMSFrame::getVertex(const ROOT::Math::XYZVector& vector) const
{
  // Transform Vertex from lab into cms frame:
  // TODO 0: Subtract fitted IP similar to RestFrame
  // 1. Use Lorentz Transformation to Boost Vertex vector into cms frame
  // 2. Subtract movement of vertex end due to the time difference between
  //    the former simultaneous measured vertex points (see derivation of Lorentz contraction)
  ROOT::Math::PxPyPzEVector a = m_transform.rotateLabToCms() * ROOT::Math::PxPyPzEVector(vector.x(), vector.y(), vector.z(), 0);
  return a.Vect() - ROOT::Math::XYZVector(m_transform.getBoostVector().x(), m_transform.getBoostVector().y(),
                                          m_transform.getBoostVector().z()) * a.T();
}

ROOT::Math::PxPyPzEVector CMSFrame::getMomentum(const ROOT::Math::PxPyPzEVector& vector) const
{
  // 1. Boost momentum into cms frame
  return m_transform.rotateLabToCms() * vector;
}

TMatrixFSym CMSFrame::getMomentumErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD lorentzrot(4, 4);

  m_transform.rotateLabToCms().GetRotationMatrix(lorentzrot);

  TMatrixFSym tmp_matrix(matrix);
  return tmp_matrix.Similarity(lorentzrot);
}

TMatrixFSym CMSFrame::getVertexErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD lorentzrot(4, 3);

  TMatrixD labToCmsFrame(4, 4);
  m_transform.rotateLabToCms().GetRotationMatrix(labToCmsFrame);
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 3; ++j)
      lorentzrot(i, j) = labToCmsFrame(i, j);

  TMatrixFSym tmp_matrix(matrix);
  auto rotated_error_matrix = tmp_matrix.Similarity(lorentzrot);

  TMatrixD timeshift(3, 4);
  timeshift.Zero();
  auto boost_vector = m_transform.getBoostVector();
  timeshift(0, 0) = 1;
  timeshift(1, 1) = 1;
  timeshift(2, 2) = 1;
  timeshift(0, 3) = boost_vector(0);
  timeshift(1, 3) = boost_vector(1);
  timeshift(2, 3) = boost_vector(2);

  return rotated_error_matrix.Similarity(timeshift);
}

RotationFrame::RotationFrame(const ROOT::Math::XYZVector& newX, const ROOT::Math::XYZVector& newY,
                             const ROOT::Math::XYZVector& newZ) :
  m_rotation(newX.Unit(), newY.Unit(), newZ.Unit())
{
  m_rotation.Invert();
}

ROOT::Math::XYZVector RotationFrame::getVertex(const ROOT::Math::XYZVector& vector) const
{
  return m_rotation * vector;
}

ROOT::Math::PxPyPzEVector RotationFrame::getMomentum(const ROOT::Math::PxPyPzEVector& vector) const
{
  return m_rotation * vector;
}

TMatrixFSym RotationFrame::getMomentumErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD extendedrot(4, 4);
  extendedrot.Zero();
  m_rotation.GetRotationMatrix(extendedrot);
  extendedrot(3, 3) = 1;

  TMatrixFSym tmp_matrix(matrix);
  return tmp_matrix.Similarity(extendedrot);
}

TMatrixFSym RotationFrame::getVertexErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD rotmatrix(3, 3);
  m_rotation.GetRotationMatrix(rotmatrix);

  TMatrixFSym tmp_matrix(matrix);
  return tmp_matrix.Similarity(rotmatrix);
}

CMSRotationFrame::CMSRotationFrame(const ROOT::Math::XYZVector& newX, const ROOT::Math::XYZVector& newY,
                                   const ROOT::Math::XYZVector& newZ) :
  rotationframe(newX, newY, newZ)
{

}

ROOT::Math::XYZVector CMSRotationFrame::getVertex(const ROOT::Math::XYZVector& vector) const
{
  return rotationframe.getVertex(cmsframe.getVertex(vector));
}

ROOT::Math::PxPyPzEVector CMSRotationFrame::getMomentum(const ROOT::Math::PxPyPzEVector& vector) const
{
  return rotationframe.getMomentum(cmsframe.getMomentum(vector));
}

TMatrixFSym CMSRotationFrame::getMomentumErrorMatrix(const TMatrixFSym& matrix) const
{
  return rotationframe.getMomentumErrorMatrix(cmsframe.getMomentumErrorMatrix(matrix));
}

TMatrixFSym CMSRotationFrame::getVertexErrorMatrix(const TMatrixFSym& matrix) const
{
  return rotationframe.getVertexErrorMatrix(cmsframe.getVertexErrorMatrix(matrix));
}
