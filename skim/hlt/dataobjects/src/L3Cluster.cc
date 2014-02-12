/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/hlt/dataobjects/L3Cluster.h>

using namespace Belle2;

ClassImp(L3Cluster)

L3Cluster::L3Cluster()
  : m_energy(0.),
    m_theta(0.),
    m_phi(0.)
{
}

L3Cluster::L3Cluster(const double energy, const TVector3& position)
{
  m_energy = energy;
  m_theta = position.Theta();
  m_phi = position.Phi();
}
