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
  m_minTrackLength(), m_debug4Geo(), m_materialDefinitionMode(), m_senseWireZposMode(), m_displacement(), m_alignment(),
  m_misalignment(), m_displacementFile(), m_alignmentFile(), m_misalignmentFile(), m_xtFile(), m_sigmaFile(), m_propSpeedFile(),
  m_t0File(), m_twFile(), m_bwFile(), m_chMapFile()

{
  // Set description
  setDescription("Change Job contorol parameters.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //N.B. The following default values must be identical to the ones in xxControlPar objects.
  //For Simulation
  //Switch for debug
  addParam("Debug4Sim", m_debug4Sim, "Switch on/off debug in FullSim", false);
  //Switch for wire sag
  addParam("WireSag", m_wireSag, "Switch on/off sense wire (gravitational) sag in FullSim", true);
  //Switch for modified left/right flag
  addParam("ModLeftRightFlag", m_modLeftRightFlag, "Switch on/off calculation of modified left/right flag in FullSim", false);
  //energy thresh
  addParam("ThresholdEnergyDeposit",  m_thresholdEnergyDeposit,
           "Energy deposite (edep) thresh. for G4 step (GeV). All hits with smaller edep will be dropped at FullSim level. Set this to a negative value if you want to keep simhits with edep=0.",
           0.0);
  //min. track length
  addParam("MinTrackLength",  m_minTrackLength,
           "Minimum track length (cm) required for saving in MCParticle.; to be applied to all particles other than primaries and decay-daughters.",
           15.0);

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
  //displacement switch
  addParam("Displacement", m_displacement, "Switch for wire displacement: on/off.",  true);
  //alignmentt switch
  addParam("Alignment", m_alignment, "Switch for wire alignment: on/off.",  true);
  //misalignment switch
  addParam("Misalignment", m_misalignment, "Switch for wire misalignment: on/off.",  true);
  //displacement file
  addParam("DisplacementFile", m_displacementFile, "Input file name for wire displacement.",  string("displacement_v1.1.dat"));
  //alignment file
  addParam("AlignmentFile", m_alignmentFile, "Input file name for wire alignment.",  string("alignment_v2.dat"));
  //misalignment file
  addParam("MisalignmentFile", m_misalignmentFile, "Input file name for wire misalignment.", string("misalignment_v2.dat"));
  //xt-relation
  addParam("XtFile", m_xtFile, "Input file name for xt-relations.",  string("xt_v3.dat"));
  //sigma
  addParam("SigmaFile", m_sigmaFile, "Input file name for sigmas.",  string("sigma_v1.dat"));
  //prop-speed
  addParam("PropSpeedFile", m_propSpeedFile, "Input file name for prop-speeds.",  string("propspeed_v0.dat"));
  //t0
  addParam("T0File", m_t0File, "Input file name for t0s.",  string("t0.dat"));
  //time walk
  addParam("TwFile", m_twFile, "Input file name for time walks.",  string("tw_off.dat"));
  //bad wire
  addParam("BwFile", m_bwFile, "Input file name for bad wires.",  string("badwire_v1.dat"));
  //channel map
  addParam("ChMapFile", m_chMapFile, "Input file name for channel map.",  string("ch_map.dat"));

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

  if (m_gcp.getDisplacement() != m_displacement) {
    B2INFO("CDCJobCntlParModifier: displacement switch modified: " << m_gcp.getDisplacement() << " to " << m_displacement);
    m_gcp.setDisplacement(m_displacement);
  }

  if (m_gcp.getAlignment() != m_alignment) {
    B2INFO("CDCJobCntlParModifier: alignment switch modified: " << m_gcp.getAlignment() << " to " << m_alignment);
    m_gcp.setAlignment(m_alignment);
  }

  if (m_gcp.getMisalignment() != m_misalignment) {
    B2INFO("CDCJobCntlParModifier: misalignment switch modified: " << m_gcp.getMisalignment() << " to " << m_misalignment);
    m_gcp.setMisalignment(m_misalignment);
  }

  if (m_gcp.getDisplacementFile() != m_displacementFile) {
    B2INFO("CDCJobCntlParModifier: displacementFile modified: " << m_gcp.getDisplacementFile() << " to " << m_displacementFile);
    m_gcp.setDisplacementFile(m_displacementFile);
  }

  if (m_gcp.getAlignmentFile() != m_alignmentFile) {
    B2INFO("CDCJobCntlParModifier: alignmentFile modified: " << m_gcp.getAlignmentFile() << " to " << m_alignmentFile);
    m_gcp.setAlignmentFile(m_alignmentFile);
  }

  if (m_gcp.getMisalignmentFile() != m_misalignmentFile) {
    B2INFO("CDCJobCntlParModifier: misalignmentFile modified: " << m_gcp.getMisalignmentFile() << " to " << m_misalignmentFile);
    m_gcp.setMisalignmentFile(m_misalignmentFile);
  }

  if (m_gcp.getXtFile() != m_xtFile) {
    B2INFO("CDCJobCntlParModifier: xtFile modified: " << m_gcp.getXtFile() << " to " << m_xtFile);
    m_gcp.setXtFile(m_xtFile);
  }

  if (m_gcp.getSigmaFile() != m_sigmaFile) {
    B2INFO("CDCJobCntlParModifier: sigmaFile modified: " << m_gcp.getSigmaFile() << " to " << m_sigmaFile);
    m_gcp.setSigmaFile(m_sigmaFile);
  }

  if (m_gcp.getPropSpeedFile() != m_propSpeedFile) {
    B2INFO("CDCJobCntlParModifier: propSpeedFile modified: " << m_gcp.getPropSpeedFile() << " to " << m_propSpeedFile);
    m_gcp.setPropSpeedFile(m_propSpeedFile);
  }

  if (m_gcp.getT0File() != m_t0File) {
    B2INFO("CDCJobCntlParModifier: t0File modified: " << m_gcp.getT0File() << " to " << m_t0File);
    m_gcp.setT0File(m_t0File);
  }

  if (m_gcp.getTwFile() != m_twFile) {
    B2INFO("CDCJobCntlParModifier: twFile modified: " << m_gcp.getTwFile() << " to " << m_twFile);
    m_gcp.setTwFile(m_twFile);
  }

  if (m_gcp.getBwFile() != m_bwFile) {
    B2INFO("CDCJobCntlParModifier: bwFile modified: " << m_gcp.getBwFile() << " to " << m_bwFile);
    m_gcp.setBwFile(m_bwFile);
  }

  if (m_gcp.getChMapFile() != m_chMapFile) {
    B2INFO("CDCJobCntlParModifier: chMapFile modified: " << m_gcp.getChMapFile() << " to " << m_chMapFile);
    m_gcp.setChMapFile(m_chMapFile);
  }

}

void CDCJobCntlParModifierModule::event()
{
}
