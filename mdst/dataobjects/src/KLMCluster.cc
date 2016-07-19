/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>

using namespace Belle2;

KLMCluster::KLMCluster() : m_time(0), m_layers(0), m_innermostLayer(0),
  m_globalX(0), m_globalY(0), m_globalZ(0), m_e(0), m_pX(0), m_pY(0), m_pZ(0),
  m_momentumErrorMatrix(7)
{
}

KLMCluster::KLMCluster(float x, float y, float z, float time, int nLayers,
                       int nInnermostLayer, float px, float py, float pz) :
  m_time(time), m_layers(nLayers), m_innermostLayer(nInnermostLayer),
  m_globalX(x), m_globalY(y), m_globalZ(z), m_e(0), m_pX(px), m_pY(py),
  m_pZ(pz), m_momentumErrorMatrix(7)
{
}

KLMCluster::~KLMCluster()
{
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

