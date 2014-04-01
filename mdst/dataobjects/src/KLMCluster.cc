/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <mdst/dataobjects/KLMCluster.h>

using namespace Belle2;

ClassImp(KLMCluster);

KLMCluster::KLMCluster()
{
}

KLMCluster::KLMCluster(ROOT::Math::XYZPointF coordinates, float time, int nLayers, int nInnermostLayer, TVector3 momentum)
{
  m_globalX = coordinates.X();
  m_globalY = coordinates.Y();
  m_globalZ = coordinates.Z();
  m_time = time;
  m_layers = nLayers;
  m_innermostLayer = nInnermostLayer;
  m_pX = momentum.Px();
  m_pY = momentum.Py();
  m_pZ = momentum.Pz();
  m_eclClusterFound = false;
  m_trackFound = false;
}


KLMCluster::KLMCluster(float x, float y, float z, float time, int nLayers, int nInnermostLayer, float px, float py, float pz)
{
  m_globalX = x;
  m_globalY = y;
  m_globalZ = z;
  m_time = time;
  m_layers = nLayers;
  m_innermostLayer = nInnermostLayer;
  m_pX = px;
  m_pY = py;
  m_pZ = pz;
  m_eclClusterFound = false;
  m_trackFound = false;
}



KLMCluster::~KLMCluster()
{
}


