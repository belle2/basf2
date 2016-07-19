/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TDatabasePDG.h>

/* Belle2 headers. */
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>

using namespace Belle2;

KLMCluster::KLMCluster() : m_time(0), m_layers(0), m_innermostLayer(0),
  m_globalX(0), m_globalY(0), m_globalZ(0), m_p(0),
  m_momentumErrorMatrix(7)
{
}

KLMCluster::KLMCluster(float x, float y, float z, float time, int nLayers,
                       int nInnermostLayer, float px, float py, float pz) :
  m_time(time), m_layers(nLayers), m_innermostLayer(nInnermostLayer),
  m_globalX(x), m_globalY(y), m_globalZ(z),
  m_momentumErrorMatrix(7)
{
  m_p = sqrt(px * px + py * py + pz * pz);
}

KLMCluster::KLMCluster(float x, float y, float z, float time, int nLayers,
                       int nInnermostLayer, float p) :
  m_time(time), m_layers(nLayers), m_innermostLayer(nInnermostLayer),
  m_globalX(x), m_globalY(y), m_globalZ(z), m_p(p),
  m_momentumErrorMatrix(7)
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
  static double mass = TDatabasePDG::Instance()->GetParticle(130)->Mass();
  return sqrt(mass * mass + m_p * m_p);
}

TLorentzVector KLMCluster::getMomentum() const
{
  static double mass = TDatabasePDG::Instance()->GetParticle(130)->Mass();
  TVector3 p3(m_globalX, m_globalY, m_globalZ);
  p3 = p3.Unit() * m_p;
  return TLorentzVector(p3, getEnergy());
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

