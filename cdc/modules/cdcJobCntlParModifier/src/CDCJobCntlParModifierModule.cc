/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcJobCntlParModifier/CDCJobCntlParModifierModule.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

// register module
REG_MODULE(CDCJobCntlParModifier)
CDCJobCntlParModifierModule::CDCJobCntlParModifierModule() : Module(), m_scp(CDCSimControlPar::getInstance()),
  m_gcp(CDCGeoControlPar::getInstance()), m_wireSag(), m_modLeftRightFlag(), m_debug4Sim(), m_thresholdEnergyDeposit(),
  m_minTrackLength(), m_debug4Geo(), m_materialDefinitionMode(), m_senseWireZposMode()
{
  // Set description
  setDescription("Change Job contorol parameters.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //N.B. The following default values must be identical to the ones in xxControlPar objects.
  //For Simulation
  //Switch for debug
  addParam("Debug4Sim", m_debug4Sim, "Switch on/off debug in FullSim", false);
  //Switch for wire sag
  addParam("WireSag", m_wireSag, "Switch on/off sense wire sag in FullSim", true);
  //Switch for modified left/right flag
  addParam("ModLeftRightFlag", m_modLeftRightFlag, "Switch on/off calculation of modified left/right flag in FullSim", false);
  //energy thresh
  addParam("ThresholdEnergyDeposit",  m_thresholdEnergyDeposit,  "Energy thresh. for G4 step (GeV)",  0.0);
  //min. track length
  addParam("MinTrackLength",  m_minTrackLength,  "Minimum track length for G4 step (cm) ",  15.0);

  //For Geometry
  //Switch for debug
  addParam("Debug4Geo", m_debug4Geo, "Switch on/off debug in Geo", false);
  //material definition mode
  addParam("MaterialDefinitionMode",  m_materialDefinitionMode,
           "Material definition mode: =0: define a mixture of gases and wires in the entire tracking volume; =1: dummy; =2: define all sense and field wires explicitly in the volume.",
           0);

  //sense wire z pos mode
  addParam("SenseWireZposMode",  m_senseWireZposMode,
           "Sense wire z position mode: =1: correct for feedthrough effect; =0: not correct.",  1);

}

void CDCJobCntlParModifierModule::initialize()
{
  //  CDCSimControlPar& cp = CDCSimControlPar::getInstance();

  //For Simulation
  if (m_scp.getWireSag() != m_wireSag) {
    B2INFO("CDCJobCntlParModifier: wireSag modified: " << m_scp.getWireSag() << " to " << m_wireSag);
    m_scp.setWireSag(m_wireSag);
  }

  if (m_scp.getModLeftRightFlag()  !=  m_modLeftRightFlag) {
    B2INFO("CDCJobCntlParModifier: modLeftRightFlag modified: " << m_scp.getModLeftRightFlag()  << " to " << m_modLeftRightFlag);
    m_scp.setModLeftRightFlag(m_modLeftRightFlag);
  }

  if (m_scp.getDebug()  !=  m_debug4Sim) {
    B2INFO("CDCJobCntlParModifier: debug4Sim modified: " << m_scp.getDebug()  << " to " << m_debug4Sim);
    m_scp.setDebug(m_debug4Sim);
  }

  if (m_scp.getThresholdEnergyDeposit() != m_thresholdEnergyDeposit) {
    B2INFO("CDCJobCntlParModifier: thresholdEnergyDeposit modified: " << m_scp.getThresholdEnergyDeposit() << " to " <<
           m_thresholdEnergyDeposit);
    m_scp.setThresholdEnergyDeposit(m_thresholdEnergyDeposit);
  }

  if (m_scp.getMinTrackLength() != m_minTrackLength) {
    B2INFO("CDCJobCntlParModifier: minTrackLength modified: "  << m_scp.getMinTrackLength() << " to " << m_minTrackLength);
    m_scp.setMinTrackLength(m_minTrackLength);
  }

  //For Geometry
  if (m_gcp.getMaterialDefinitionMode() != m_materialDefinitionMode) {
    B2INFO("CDCJobCntlParModifier: materialDefinitionMode modified: " << m_gcp.getMaterialDefinitionMode() << " to " <<
           m_materialDefinitionMode);
    m_gcp.setMaterialDefinitionMode(m_materialDefinitionMode);
  }

  if (m_gcp.getSenseWireZposMode() != m_senseWireZposMode) {
    B2INFO("CDCJobCntlParModifier: senseWireZposMode modified: " << m_gcp.getSenseWireZposMode() << " " << m_senseWireZposMode);
    m_gcp.setSenseWireZposMode(m_senseWireZposMode);
  }

  if (m_gcp.getDebug() != m_debug4Geo) {
    B2INFO("CDCJobCntlParModifier: debug4Geo modified: " << m_gcp.getDebug() << " to " << m_debug4Geo);
    m_gcp.setDebug(m_debug4Geo);
  }
}

void CDCJobCntlParModifierModule::event()
{
}
