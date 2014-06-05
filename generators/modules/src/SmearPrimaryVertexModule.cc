/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/SmearPrimaryVertexModule.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

//TODO: use framework random number generator --> done
#include <TRandom3.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SmearPrimaryVertex)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SmearPrimaryVertexModule::SmearPrimaryVertexModule() : Module()
{
  //Set module properties
  setDescription("Smears primary vertex and all subsequent vertices in a MCParticle Collection");

  //Parameter definition
  addParam("MCParticleCollectionName", m_particleList, "Collection to print", string(""));

  addParam("fromDataBase", m_useDB, "Use values from Database", false);

  addParam("pvx", m_new_ip_x, "New nominal position of Primary Vertex in x (cm)", 0.0);
  addParam("pvy", m_new_ip_y, "New nominal position of Primary Vertex in y (cm)", 0.0);
  addParam("pvz", m_new_ip_z, "New nominal position of Primary Vertex in z (cm)", 0.0);

  addParam("sigma_pvx", m_sigma_ip_x, "Apply spread (standard deviation) of Primary Vertex in x (cm)", 0.0110);
  addParam("sigma_pvy", m_sigma_ip_y, "Apply spread (standard deviation) of Primary Vertex in y (cm)", 0.0015);
  addParam("sigma_pvz", m_sigma_ip_z, "Apply spread (standard deviation) of Primary Vertex in z (cm)", 0.6000);

  addParam("angle_pv_yz", m_new_angle_ip_yz, "Angle of rotation of Primary Vertex Profile wrt. x-axis (yz-plane) in (rad)", 0.000);
  addParam("angle_pv_zx", m_new_angle_ip_zx, "Angle of rotation of Primary Vertex Profile wrt. y-axis (zx-plane) in (rad)", 0.000);
  addParam("angle_pv_xy", m_new_angle_ip_xy, "Angle of rotation of Primary Vertex Profile wrt. z-axis (xy-plane) in (rad)", 0.000);
}

void SmearPrimaryVertexModule::initialize()
{
}

void SmearPrimaryVertexModule::terminate()
{
}


void SmearPrimaryVertexModule::beginRun()
{
  if (m_useDB) {
    // TODO: obtain values from Database
  } else {
    // use user specified values

    // set TVector3 of new nominal pv
    m_new_nominal_ip = TVector3(m_new_ip_x,
                                m_new_ip_y,
                                m_new_ip_z);

    // set TVector3 of new spread for pv
    m_sigma_ip = TVector3(m_sigma_ip_x,
                          m_sigma_ip_y,
                          m_sigma_ip_z);

    // set TVector3 of new rotation angle
    m_new_angle_ip = TVector3(m_new_angle_ip_yz,
                              m_new_angle_ip_zx,
                              m_new_angle_ip_xy);
  }
}

void SmearPrimaryVertexModule::event()
{
  StoreArray<MCParticle> MCParticles(m_particleList);

  if (MCParticles.getEntries() < 1) {
    B2WARNING("MCParticle array has no entry. Skiping event...");
    return;
  }

  setNewPrimaryVertex();
  setOldPrimaryVertex();

  //Loop over the primary particles (no mother particle exists)
  for (int i = 0; i < MCParticles.getEntries(); i++) {
    MCParticle* mc = MCParticles[i];

    // shift production vertex
    mc->setProductionVertex(getShiftedVertex(mc->getVertex()));
    // shift decay vertex
    mc->setDecayVertex(getShiftedVertex(mc->getDecayVertex()));
  }
}

TVector3 SmearPrimaryVertexModule::getShiftedVertex(TVector3 oldVertex)
{
  return oldVertex + (m_new_ip - old_ip);
}

void SmearPrimaryVertexModule::setNewPrimaryVertex(void)
{
  // randomly smear around nominal vertex position
  //TH TRandom3 rndm(0);

  // obtain random shift from nominal position
  // TODO: use framework random number generator --> done
  TVector3 randomShift(gRandom->Gaus(0.0, m_sigma_ip.X()),
                       gRandom->Gaus(0.0, m_sigma_ip.Y()),
                       gRandom->Gaus(0.0, m_sigma_ip.Z()));

  // rotate
  randomShift.RotateX(m_new_angle_ip.X());
  randomShift.RotateY(m_new_angle_ip.Y());
  randomShift.RotateZ(m_new_angle_ip.Z());

  m_new_ip = m_new_nominal_ip + randomShift;
}

void SmearPrimaryVertexModule::setOldPrimaryVertex(void)
{

  // set current primary vertex position
  StoreArray<MCParticle> MCParticles(m_particleList);
  MCParticle* mc = MCParticles[0];
  old_ip = mc->getVertex();


}

