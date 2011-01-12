/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/PGunInputModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace Generators;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PGunInputModule, "PGunInput")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PGunInputModule::PGunInputModule() : Module()
{
  vector<int> defaultPIDcodes;
  defaultPIDcodes.push_back(11);
  defaultPIDcodes.push_back(-11);

  //Set module properties
  setDescription("particle gun for tracks");
  setPropertyFlags(c_TriggersEndOfData | c_ReadsDataSingleProcess | c_RequiresSingleProcess);

  //Parameter definition
  addParam("ntracks", m_ntracks, 1, "The number of tracks to be generated per event");
  addParam("p_par1", m_p_par1, 0.2, "The first parameter for momentum distribution function");
  addParam("p_par2", m_p_par2, 10.0,  "The second parameter for momentum distribution function");
  addParam("ph_par1", m_ph_par1, 0.0, "The first parameter for the phi distribution function");
  addParam("ph_par2", m_ph_par2, 360.0,  "The second parameter for the phi distribution function");
  addParam("th_par1", m_th_par1, 17.0, "The first parameter for theta distribution function");
  addParam("th_par2", m_th_par2, 150.0,  "The second parameter for theta distribution function");
  addParam("x_par1", m_x_par1, 0.0, "The first parameter for vertex x-coordinate distribution function");
  addParam("x_par2", m_x_par2, 0.7,  "The second parameter for x-coordinate distribution distribution function");
  addParam("y_par1", m_y_par1, 0.0, "The first parameter for vertex y-coordinate distribution function");
  addParam("y_par2", m_y_par2, 0.7,  "The second parameter for y-coordinate distribution distribution function");
  addParam("z_par1", m_z_par1, 0.0, "The first parameter for vertex z-coordinate distribution function");
  addParam("z_par2", m_z_par2,  1.0,  "The second parameter for z-coordinate distribution distribution function");
  addParam("momentumGeneration", m_genMom, 0,  "Choice of distribution function for momentum generation");
  addParam("vertexGeneration", m_genVert, 2,  "Choice of distribution function for vertex generation");
  addParam("angleGeneration", m_genAngle, 0,  "Choice of distribution function for angle generation");
  addParam("PIDcodes", m_PIDcodes, defaultPIDcodes, "PID codes for generated particles");
}


void PGunInputModule::initialize()
{
  m_pgun.m_ntracks = m_ntracks;
  m_pgun.m_p_par1 = m_p_par1;
  m_pgun.m_p_par2 = m_p_par2;
  m_pgun.m_th_par1 = m_th_par1;
  m_pgun.m_th_par2 = m_th_par2;
  m_pgun.m_ph_par1 = m_ph_par1;
  m_pgun.m_ph_par2 = m_ph_par2;
  m_pgun.m_z_par1 = m_z_par1;
  m_pgun.m_z_par2 = m_z_par2;
  m_pgun.m_y_par1 = m_y_par1;
  m_pgun.m_y_par2 = m_y_par2;
  m_pgun.m_x_par1 = m_x_par1;
  m_pgun.m_x_par2 = m_x_par2;
  if (m_PIDcodes.size() > 0)
    m_pgun.m_PIDcodes = m_PIDcodes;
  else {
    m_pgun.m_PIDcodes.push_back(11);
    m_pgun.m_PIDcodes.push_back(-11);
  }

  if (m_genMom<2 && m_genMom> -1)
    m_pgun.m_genMom = static_cast<PGUNgenOpt>(m_genMom);
  else m_genMom = 0;
  if (m_genVert<2 && m_genVert> -1)
    m_pgun.m_genVert = static_cast <PGUNgenOpt>(m_genVert);
  else m_genVert = 2;
  if (m_genAngle<2 && m_genAngle> -1)
    m_pgun.m_genAngle = static_cast <PGUNgenOpt>(m_genAngle);
  else m_genMom = 0;

}


void PGunInputModule::event()
{
  try {
    mpg.clear();
    m_pgun.generateEvent(mpg);
    mpg.generateList(DEFAULT_MCPARTICLES, MCParticleGraph::set_decay_info | MCParticleGraph::check_cyclic);
  } catch (runtime_error &e) {
    B2ERROR(e.what());
  }
}

