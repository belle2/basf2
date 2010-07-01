/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/simkernel/B4PrimaryGeneratorMessenger.h>

#include "G4ios.hh"

using namespace Belle2;

B4PrimaryGeneratorMessenger::B4PrimaryGeneratorMessenger(B4PrimaryGeneratorAction * mpga)
    : m_myAction(mpga)
{
  //------------------------------------------
  // Define a empty pointer of G4UIparameter.
  //------------------------------------------
  G4UIparameter* parameter = NULL;

  //------------------------------------------
  // Get an instance of class G4ParticleTable.
  //------------------------------------------
  m_particleTable = G4ParticleTable::GetParticleTable();

  //-----------------------------------
  // Create a directory for generator.
  //-----------------------------------
  m_generatorDirectory = new G4UIdirectory("/generator/");
  m_generatorDirectory->SetGuidance("Belle2 particle gun gnerator");

  //--------------------------------------
  // Create a command for generator name.
  //--------------------------------------
  m_gennamCmd = new G4UIcmdWithAString("/generator/name", this);
  m_gennamCmd->SetGuidance("Select your event gerator: HEPEvt or tester");
  m_gennamCmd->SetGuidance("The default generator is tester");
  m_gennamCmd->SetParameterName("name", true);
  m_gennamCmd->SetDefaultValue("tester");
  m_gennamCmd->SetCandidates("tester HEPEvt");

  //----------------------------------------
  // Create a command for HEPEvt generator.
  //----------------------------------------
  m_HEPEvtCmd = new G4UIcmdWithAString("/generator/HEPEvt", this);
  m_HEPEvtCmd->SetGuidance("The HEPEvt file name");
  m_HEPEvtCmd->SetGuidance("directory/filename");
  m_HEPEvtCmd->SetParameterName("name", false);

  //----------------------------------------
  // Create a command for tester generator.
  //----------------------------------------
  m_testerCmd = new G4UIcommand("/generator/tester", this);
  m_testerCmd->SetGuidance("The Belle2 TESTER generator");
  m_testerCmd->SetGuidance("NP PID Cosmin Cosmax phimin phimax p delp");

  //----------------------------------------------------------------------------
  // Define and set parameters, please see guidance for each parameter meaning.
  //----------------------------------------------------------------------------
  G4bool omittable;
  parameter = new G4UIparameter("np", 'i', omittable = true);
  parameter->SetGuidance("Number of particles in an event");
  parameter->SetParameterRange("np > 0");
  parameter->SetDefaultValue(1);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("pname", 's', omittable = true);
  parameter->SetGuidance("Particle name in Geant4");
  parameter->SetDefaultValue("pi-");
  G4String candidatelist;
  G4int nPtable = m_particleTable->entries();
  for (G4int i = 0; i < nPtable; i++) {
    candidatelist += m_particleTable->GetParticleName(i);
    candidatelist += " ";
  }
  parameter->SetParameterCandidates(candidatelist);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("cosmin", 'd', omittable = true);
  parameter->SetGuidance("Minimum cos(theta)");
  parameter->SetParameterRange("cosmin >= -1. && cosmin <= 1.");
  parameter->SetDefaultValue(-0.8);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("cosmax", 'd', omittable = true);
  parameter->SetGuidance("Maximum cos(theta)");
  parameter->SetParameterRange("cosmax >= -1. && cosmax <= 1.");
  parameter->SetDefaultValue(0.8);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("phimin", 'd', omittable = true);
  parameter->SetGuidance("Minimum azimuthal angle in degree");
  parameter->SetParameterRange("phimin >= 0. && phimin <= 360.");
  parameter->SetDefaultValue(0.);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("phimax", 'd', omittable = true);
  parameter->SetGuidance("Maximum azimuthal angle in degree");
  parameter->SetParameterRange("phimax >= 0. && phimax <= 360.");
  parameter->SetDefaultValue(360.);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("pp", 'd', omittable = true);
  parameter->SetGuidance("Particle Momentum in GeV");
  parameter->SetParameterRange("pp>=0.");
  parameter->SetDefaultValue(1.0);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("dp", 'd', omittable = true);
  parameter->SetGuidance("Particle Momentum range in GeV");
  parameter->SetParameterRange("dp>=0.");
  parameter->SetDefaultValue(0.0);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("x", 'd', omittable = true);
  parameter->SetGuidance("Particle x position");
  parameter->SetParameterRange("x >= -10000 && x <= 10000");
  parameter->SetDefaultValue(0.);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("y", 'd', omittable = true);
  parameter->SetGuidance("Particle y position");
  parameter->SetParameterRange("y >= -10000 && y <= 10000");
  parameter->SetDefaultValue(0.);
  m_testerCmd->SetParameter(parameter);

  parameter = new G4UIparameter("z", 'd', omittable = true);
  parameter->SetGuidance("Particle z position");
  parameter->SetParameterRange("z >= -10000 && z <= 10000");
  parameter->SetDefaultValue(0.);
  m_testerCmd->SetParameter(parameter);
}

B4PrimaryGeneratorMessenger::~B4PrimaryGeneratorMessenger()
{
  if (m_generatorDirectory) delete m_generatorDirectory;
  if (m_gennamCmd)          delete m_gennamCmd;
  if (m_HEPEvtCmd)          delete m_HEPEvtCmd;
  if (m_testerCmd)          delete m_testerCmd;
}

void B4PrimaryGeneratorMessenger::SetNewValue(G4UIcommand * command, G4String newValue)
{
  //------------------------------------------------------------------
  // Set parameters for member variables in class B4PrimaryGenerator.
  //------------------------------------------------------------------

  if (command == m_gennamCmd) {

    // generator name
    m_myAction->setGeneratorName(newValue);

  } else if (command == m_HEPEvtCmd) {

    // generator HEPEvt
    m_myAction->setHEPEvtFileName(newValue);

  } else if (command == m_testerCmd) {
    G4int np;
    G4String pname;
    G4double cosmin, cosmax, phimin, phimax, pp, dp, x, y, z;
    std::istringstream is((char*)newValue.data());
    is >> np >> pname >> cosmin >> cosmax
    >> phimin >> phimax >> pp >> dp >> x >> y >> z;

    m_myAction->setNParticle(np);
    m_myAction->setParticleName(pname);
    m_myAction->setMinCos(cosmin);
    m_myAction->setMaxCos(cosmax);
    m_myAction->setPhiStart(phimin);
    m_myAction->setPhiEnd(phimax);
    m_myAction->setMomentum(pp);
    m_myAction->setDeltaP(dp);
    m_myAction->setPosX(x);
    m_myAction->setPosY(y);
    m_myAction->setPosZ(z);
  }
}

G4String B4PrimaryGeneratorMessenger::GetCurrentValue(G4UIcommand *)
{
  return "";
}
