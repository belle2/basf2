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
  m_boost(m_momentum.BoostVector()),
  m_lab2restframe(TLorentzRotation(-m_boost))
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

TVector3 RestFrame::getVertex(const TVector3& vector) const
{
  // Transform Vertex from lab into rest frame:
  // 1. Subtract displacement of rest frame origin in the lab frame
  // 2. Use Lorentz Transformation to Boost Vertex vector into rest frame
  // 3. Subtract movement of vertex end due to the time difference between
  //    the former simultaneous measured vertex points (see derivation of Lorentz contraction)
  TLorentzVector a = m_lab2restframe * TLorentzVector(vector - m_displacement, 0);
  return a.Vect() - m_boost * a.T();
}

TLorentzVector RestFrame::getMomentum(const TLorentzVector& vector) const
{
  // 1. Boost momentum into rest frame
  return m_lab2restframe * vector;
}

TMatrixFSym RestFrame::getMomentumErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD lorentzrot(4, 4);

  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      lorentzrot(i, j) = m_lab2restframe(i, j);

  TMatrixFSym tmp_matrix(matrix);

  return tmp_matrix.Similarity(lorentzrot);
}

TMatrixFSym RestFrame::getVertexErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD lorentzrot(4, 3);

  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 3; ++j)
      lorentzrot(i, j) = m_lab2restframe(i, j);

  TMatrixFSym tmp_matrix(matrix);
  auto rotated_error_matrix = tmp_matrix.Similarity(lorentzrot);

  TMatrixD timeshift(3, 4);
  timeshift.Zero();
  timeshift(0, 0) = 1;
  timeshift(1, 1) = 1;
  timeshift(2, 2) = 1;
  timeshift(0, 3) = m_boost(0);
  timeshift(1, 3) = m_boost(1);
  timeshift(2, 3) = m_boost(2);

  return rotated_error_matrix.Similarity(timeshift);
}

TVector3 LabFrame::getVertex(const TVector3& vector) const
{
  return vector;
}

TLorentzVector LabFrame::getMomentum(const TLorentzVector& vector) const
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

TVector3 CMSFrame::getVertex(const TVector3& vector) const
{
  // Transform Vertex from lab into cms frame:
  // TODO 0: Substract fitted IP similar to RestFrame
  // 1. Use Lorentz Transformation to Boost Vertex vector into cms frame
  // 2. Subtract movement of vertex end due to the time difference between
  //    the former simultaneous measured vertex points (see derivation of Lorentz contraction)
  TLorentzVector a = m_transform.rotateLabToCms() * TLorentzVector(vector, 0);
  return a.Vect() - m_transform.getBoostVector() * a.T();
}

TLorentzVector CMSFrame::getMomentum(const TLorentzVector& vector) const
{
  // 1. Boost momentum into cms frame
  return m_transform.rotateLabToCms() * vector;
}

TMatrixFSym CMSFrame::getMomentumErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD lorentzrot(4, 4);

  auto labToCmsFrame = m_transform.rotateLabToCms();
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      lorentzrot(i, j) = labToCmsFrame(i, j);

  TMatrixFSym tmp_matrix(matrix);
  return tmp_matrix.Similarity(lorentzrot);
}

TMatrixFSym CMSFrame::getVertexErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD lorentzrot(4, 3);

  auto labToCmsFrame = m_transform.rotateLabToCms();
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

RotationFrame::RotationFrame(const TVector3& newX, const TVector3& newY, const TVector3& newZ)
{
  m_rotation.RotateAxes(newX.Unit(), newY.Unit(), newZ.Unit());
  m_rotation.Invert();
}

TVector3 RotationFrame::getVertex(const TVector3& vector) const
{
  return m_rotation * vector;
}

TLorentzVector RotationFrame::getMomentum(const TLorentzVector& vector) const
{
  TVector3 rotated_vector = m_rotation * vector.Vect();

  return TLorentzVector(rotated_vector, vector[3]);
}

TMatrixFSym RotationFrame::getMomentumErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD extendedrot(4, 4);
  extendedrot.Zero();
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      extendedrot(i, j) = m_rotation(i, j);
  extendedrot(3, 3) = 1;

  TMatrixFSym tmp_matrix(matrix);
  return tmp_matrix.Similarity(extendedrot);
}

TMatrixFSym RotationFrame::getVertexErrorMatrix(const TMatrixFSym& matrix) const
{
  TMatrixD rotmatrix(3, 3);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      rotmatrix(i, j) = m_rotation(i, j);

  TMatrixFSym tmp_matrix(matrix);
  return tmp_matrix.Similarity(rotmatrix);
}

CMSRotationFrame::CMSRotationFrame(const TVector3& newX, const TVector3& newY, const TVector3& newZ) :
  rotationframe(newX, newY, newZ)
{

}

TVector3 CMSRotationFrame::getVertex(const TVector3& vector) const
{
  return rotationframe.getVertex(cmsframe.getVertex(vector));
}

TLorentzVector CMSRotationFrame::getMomentum(const TLorentzVector& vector) const
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
