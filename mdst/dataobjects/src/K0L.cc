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
#include <mdst/dataobjects/K0L.h>

using namespace Belle2;

ClassImp(K0L);

K0L::K0L()
{
}

K0L::K0L(ROOT::Math::XYZPointF coordinates, float time, int nLayers, int nInnermostLayer, TVector3 momentum)
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
}


K0L::K0L(float x, float y, float z, float time, int nLayers, int nInnermostLayer, float px, float py, float pz)
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
}



K0L::~K0L()
{
}


float K0L::getTime() const
{
  return m_time;
}


int K0L::getLayers() const
{
  return m_layers;
}


ROOT::Math::XYZPointF K0L::getGlobalPosition() const
{
  return ROOT::Math::XYZPointF(m_globalX, m_globalY, m_globalZ);
}


TVector3 K0L::getPosition() const
{
  return TVector3(m_globalX, m_globalY, m_globalZ);
}


TLorentzVector K0L::getMomentum() const
{
  return TLorentzVector(m_pX, m_pY, m_pZ, m_e);
}

