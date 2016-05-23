/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/ext/ExtModule.h>
#include <tracking/trackExtrapolateG4e/TrackExtrapolateG4e.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <genfit/Track.h>
#include <genfit/DetPlane.h>
#include <genfit/FieldManager.h>
#include <genfit/TrackPoint.h>
#include <genfit/AbsFitterInfo.h>
#include <genfit/Exception.h>
#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/ExtManager.h>
#include <simulation/kernel/ExtCylSurfaceTarget.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include <TMatrixD.h>
#include <TVectorD.h>
#include <TVector3.h>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Matrix/Matrix.h>

#include <globals.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4VPhysicalVolume.hh>
#include <G4ParticleTable.hh>
#include <G4ErrorPropagatorData.hh>
#include <G4ErrorTrajErr.hh>
#include <G4ErrorFreeTrajState.hh>
#include <G4StateManager.hh>
#include <G4UImanager.hh>

#define TWOPI (2.0*M_PI)

using namespace std;
using namespace Belle2;

REG_MODULE(Ext)

ExtModule::ExtModule() :
  Module()
{
  m_Extrapolator = TrackExtrapolateG4e::GetInstance();
  m_PDGCodes.clear();
  setDescription("Extrapolates tracks from CDC to outer detectors using geant4e");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("pdgCodes", m_PDGCodes, "Positive-charge PDG codes for extrapolation hypotheses", m_PDGCodes);
  addParam("TracksColName", m_TracksColName, "Name of collection holding the reconstructed tracks", string("Tracks"));
  addParam("ExtHitsColName", m_ExtHitsColName, "Name of collection holding the ExtHits from the extrapolation", string("ExtHits"));
  addParam("MinPt", m_MinPt, "[GeV/c] Minimum transverse momentum of a particle that will be extrapolated.", double(0.0));
  addParam("MinKE", m_MinKE, "[GeV] Minimum kinetic energy of a particle to continue extrapolation.", double(0.002));
  addParam("MaxStep", m_MaxStep, "[cm] Maximum step size during extrapolation (use 0 for infinity).", double(25.0));
  addParam("Cosmic", m_Cosmic, "Particle source (0 = beam, 1 = cosmic ray.", 0);
  // Additional parameters copied from FullSimModule
  addParam("TrackingVerbosity", m_TrackingVerbosity,
           "Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info.",
           0);
  addParam("EnableVisualization", m_EnableVisualization, "If set to True the Geant4 visualization support is enabled.", false);
  addParam("magneticField", m_MagneticFieldStepperName,
           "Chooses the magnetic field stepper used by Geant4. possible values are: default, nystrom, expliciteuler, simplerunge",
           string("default"));
  addParam("magneticCacheDistance", m_MagneticCacheDistance,
           "Minimum distance for BField lookup in cm. If the next requested point is closer than this distance than return the flast BField value. 0 means no caching",
           0.0);
  addParam("deltaChordInMagneticField", m_DeltaChordInMagneticField,
           "[mm] The maximum miss-distance between the trajectory curve and its linear cord(s) approximation", 0.25);
  vector<string> defaultCommands;
  addParam("UICommands", m_UICommands, "A list of Geant4 UI commands that should be applied at the start of the job",
           defaultCommands);
}

ExtModule::~ExtModule()
{
}

void ExtModule::initialize()
{
  m_Extrapolator->initialize(m_PDGCodes, m_TracksColName, m_ExtHitsColName, m_MinPt, m_MinKE,
                             m_MaxStep, m_Cosmic, m_TrackingVerbosity, m_EnableVisualization,
                             m_MagneticFieldStepperName, m_MagneticCacheDistance, m_DeltaChordInMagneticField,
                             m_UICommands);
}

void ExtModule::beginRun()
{
  m_Extrapolator->beginRun();
}

void ExtModule::event()
{
  m_Extrapolator->event(false);
}

void ExtModule::endRun()
{
  m_Extrapolator->endRun();
}

void ExtModule::terminate()
{
  m_Extrapolator->terminate();
}
