/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleParentRestKinematicsTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/utility/ReferenceFrame.h>
#include <iostream>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleParentRestKinematicsTool::setupTree()
{
  // get selected particles' names
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  // initiate private members to hold branch addresses
  m_fP = new float[nDecayProducts];
  m_fP4 = new float*[nDecayProducts];

  // declare branches on the ntuple
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_Pparent").c_str(),
                   &m_fP[iProduct],
                   (strNames[iProduct] + "_Pparent/F").c_str());
    m_fP4[iProduct] = new float[4];
    m_tree->Branch((strNames[iProduct] + "_P4parent").c_str(),
                   &m_fP4[iProduct][0],
                   (strNames[iProduct] + "_P4parent[4]/F").c_str());
  }
}

void NtupleParentRestKinematicsTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleParentRestKinematicsTool::eval - no Particle found!");
    return;
  }

  // get selected particles
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  // get mother (head of decay chain) and use her decay reference frame
  // see ReferenceFrame.h contains definition for "RestFrame" class
  const RestFrame mothersFrame(particle);
  UseReferenceFrame<RestFrame> frame(mothersFrame);

  // loop over selected particles and save momentum in this frame
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fP[iProduct]     = Variable::particleP(selparticles[iProduct]);
    m_fP4[iProduct][0] = Variable::particlePx(selparticles[iProduct]);
    m_fP4[iProduct][1] = Variable::particlePy(selparticles[iProduct]);
    m_fP4[iProduct][2] = Variable::particlePz(selparticles[iProduct]);
    m_fP4[iProduct][3] = Variable::particleE(selparticles[iProduct]);
  }
}
