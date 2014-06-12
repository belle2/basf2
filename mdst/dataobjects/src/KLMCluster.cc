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

KLMCluster::KLMCluster() : m_time(0), m_layers(0), m_innermostLayer(0), m_globalX(0), m_globalY(0), m_globalZ(0), m_e(0), m_pX(0), m_pY(0), m_pZ(0), m_momentumErrorMatrix(7), m_trackFound(false), m_eclClusterFound(false)
{
}

KLMCluster::KLMCluster(ROOT::Math::XYZPointF coordinates, float time, int nLayers, int nInnermostLayer, TVector3 momentum, TMatrixFSym errormatrix) : m_time(time), m_layers(nLayers), m_innermostLayer(nInnermostLayer), m_globalX(coordinates.X()), m_globalY(coordinates.Y()), m_globalZ(coordinates.Z()), m_e(0), m_pX(momentum.Px()), m_pY(momentum.Py()), m_pZ(momentum.Pz()), m_momentumErrorMatrix(errormatrix), m_trackFound(false), m_eclClusterFound(false)
{
}


KLMCluster::KLMCluster(float x, float y, float z, float time, int nLayers, int nInnermostLayer, float px, float py, float pz) : m_time(time), m_layers(nLayers), m_innermostLayer(nInnermostLayer), m_globalX(x), m_globalY(y), m_globalZ(z), m_e(0), m_pX(px), m_pY(py), m_pZ(pz), m_momentumErrorMatrix(7), m_trackFound(false), m_eclClusterFound(false)
{
}



KLMCluster::~KLMCluster()
{
}


