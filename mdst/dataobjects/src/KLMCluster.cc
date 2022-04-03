/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <mdst/dataobjects/KLMCluster.h>

/* Belle2 headers. */
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>

using namespace Belle2;

KLMCluster::KLMCluster() : m_time(0), m_layers(0), m_innermostLayer(0),
  m_globalX(0), m_globalY(0), m_globalZ(0), m_p(0),
  m_errorX(0), m_errorY(0), m_errorZ(0), m_errorP(0)
{
}

KLMCluster::KLMCluster(float x, float y, float z, float time, int nLayers,
                       int nInnermostLayer, float p) :
  m_time(time), m_layers(nLayers), m_innermostLayer(nInnermostLayer),
  m_globalX(x), m_globalY(y), m_globalZ(z), m_p(p),
  m_errorX(0), m_errorY(0), m_errorZ(0), m_errorP(0)
{
}

KLMCluster::~KLMCluster()
{
}

float KLMCluster::getMomentumMag() const
{
  return m_p;
}

float KLMCluster::getEnergy() const
{
  static double mass = Const::Klong.getMass();
  return sqrt(mass * mass + m_p * m_p);
}

ROOT::Math::PxPyPzEVector KLMCluster::getMomentum() const
{
  B2Vector3D p3(m_globalX, m_globalY, m_globalZ);
  p3 = p3.Unit() * m_p;
  return ROOT::Math::PxPyPzEVector(p3.x(), p3.y(), p3.z(), getEnergy());
}

bool KLMCluster::getAssociatedEclClusterFlag() const
{
  RelationVector<ECLCluster> eclClusters = this->getRelationsTo<ECLCluster>();
  return eclClusters.size() > 0;
}

bool KLMCluster::getAssociatedTrackFlag() const
{
  RelationVector<Track> tracks = this->getRelationsFrom<Track>();
  return tracks.size() > 0;
}

TMatrixDSym KLMCluster::getError4x4() const
{
  /* Elements: px, py, pz, E. */
  double vertexDist3;
  TMatrixDSym errorMatrix(4);
  TMatrixD jacobian(4, 4);
  errorMatrix[0][0] = m_errorX;
  errorMatrix[1][1] = m_errorY;
  errorMatrix[2][2] = m_errorZ;
  errorMatrix[3][3] = m_errorP;
  vertexDist3 = pow(m_globalX * m_globalX + m_globalY * m_globalY +
                    m_globalZ * m_globalZ, 1.5);
  jacobian(0, 0) = m_p * (m_globalY * m_globalY + m_globalZ * m_globalZ) /
                   vertexDist3;
  jacobian(0, 1) = -m_p * m_globalX * m_globalY / vertexDist3;
  jacobian(0, 2) = -m_p * m_globalX * m_globalZ / vertexDist3;
  jacobian(0, 3) = m_globalX / vertexDist3;
  jacobian(1, 0) = jacobian(0, 1);
  jacobian(1, 1) = m_p * (m_globalX * m_globalX + m_globalZ * m_globalZ) /
                   vertexDist3;
  jacobian(1, 2) = -m_p * m_globalY * m_globalZ / vertexDist3;
  jacobian(1, 3) = m_globalY / vertexDist3;
  jacobian(2, 0) = jacobian(0, 2);
  jacobian(2, 1) = jacobian(1, 2);
  jacobian(2, 2) = m_p * (m_globalX * m_globalX + m_globalY * m_globalY) /
                   vertexDist3;
  jacobian(2, 3) = m_globalZ / vertexDist3;
  jacobian(3, 3) = m_p / getEnergy();
  return errorMatrix.Similarity(jacobian);
}

TMatrixDSym KLMCluster::getError7x7() const
{
  /* Elements: px, py, pz, E, x, y, z. */
  int i, j;
  TMatrixDSym errorMatrix(7);
  TMatrixDSym errorMatrix4 = getError4x4();
  for (i = 0; i < 4; i++) {
    for (j = i; j < 4; j++)
      errorMatrix[i][j] = errorMatrix4[i][j];
  }
  for (i = 4; i < 7; i++)
    errorMatrix[i][i] = 1.0;
  return errorMatrix;
}

