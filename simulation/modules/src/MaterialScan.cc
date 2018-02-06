/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/modules/MaterialScan.h>
#include <framework/gearbox/Unit.h>
#include <framework/utilities/Utils.h>
#include <boost/format.hpp>
#include <algorithm>

#include <TFile.h>
#include <TH2D.h>

#include <G4Event.hh>
#include <G4EventManager.hh>
#include <G4UserEventAction.hh>
#include <G4UserStackingAction.hh>
#include <G4UserTrackingAction.hh>
#include <G4UserSteppingAction.hh>
#include <G4RayShooter.hh>
#include <G4Track.hh>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MaterialScan);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

bool MaterialScanBase::checkStep(const G4Step* step)
{
  double stlen = step->GetStepLength();
  G4StepPoint* preStepPoint = step->GetPreStepPoint();
  G4Region* region = preStepPoint->GetPhysicalVolume()->GetLogicalVolume()->GetRegion();
  if (stlen < c_zeroTolerance) {
    ++m_zeroSteps;
  } else {
    m_zeroSteps = 0;
  }
  if (m_zeroSteps > c_maxZeroStepsNudge) {
    if (m_zeroSteps > c_maxZeroStepsKill) {
      B2ERROR("Track is stuck at " << preStepPoint->GetPosition() << " in volume '"
              << preStepPoint->GetPhysicalVolume()->GetLogicalVolume()->GetName()
              << " (" << region->GetName() << "): "
              << m_zeroSteps << " consecutive steps with length less then "
              << c_zeroTolerance << " mm, killing it");
      step->GetTrack()->SetTrackStatus(fStopAndKill);
    } else {
      B2WARNING("Track is stuck at " << preStepPoint->GetPosition() << " in volume '"
                << preStepPoint->GetPhysicalVolume()->GetLogicalVolume()->GetName()
                << " (" << region->GetName() << "): "
                << m_zeroSteps << " consecutive steps with length less then "
                << c_zeroTolerance << " mm, nudging it along");
      G4ThreeVector pos = step->GetTrack()->GetPosition();
      G4ThreeVector dir = step->GetTrack()->GetMomentumDirection();
      step->GetTrack()->SetPosition(pos + c_zeroTolerance * dir);
    }
    return false;
  }
  return true;
}


MaterialScan2D::MaterialScan2D(TFile* rootFile, const std::string& name, const std::string& axisLabel, ScanParams params):
  MaterialScanBase(rootFile, name, axisLabel), m_params(params), m_curDepth(0)
{
  //Sort the parameters accordingly
  if (m_params.minU > m_params.maxU) std::swap(m_params.minU, m_params.maxU);
  if (m_params.minV > m_params.maxV) std::swap(m_params.minV, m_params.maxV);
  //Calculate step size of the parameters
  m_stepU = (m_params.maxU - m_params.minU) / m_params.nU;
  m_stepV = (m_params.maxV - m_params.minV) / m_params.nV;
  //Set Start values
  m_curU  = m_params.minU - m_stepU / 2.;
  m_curV  = m_params.minV + m_stepV / 2.;
  //Convert max depth to G4 units
  m_params.maxDepth /= Unit::mm;
  //Sort the list of ignored materials so that we can use binary search
  std::sort(m_params.ignoredMaterials.begin(), m_params.ignoredMaterials.end());
}


bool MaterialScan2D::createNext(G4ThreeVector& origin, G4ThreeVector& direction)
{
  if (m_regions.empty()) {
    // create summary histogram right now, not on demand, to make sure it is in the file
    if (m_params.splitByMaterials) {
      getHistogram("All_Materials_x0");
      getHistogram("All_Materials_lambda");
    } else {
      getHistogram("All_Regions_x0");
      getHistogram("All_Regions_lambda");
    }
  }
  //Increase the internal coordinates
  m_curU += m_stepU;
  if (m_curU >= m_params.maxU) {
    m_curU  = m_params.minU + m_stepU / 2.;
    m_curV += m_stepV;
  }
  //Reset depth counter
  m_curDepth = 0;

  //Get the origin and direction of the ray
  getRay(origin, direction);

  //Check wether we are finished
  return (m_curV <= m_params.maxV);
}

TH2D* MaterialScan2D::getHistogram(const std::string& name)
{
  TH2D*& hist = m_regions[name];
  if (!hist) {
    //Create new histogram
    m_rootFile->cd(m_name.c_str());
    hist = new TH2D(name.c_str(), (name + ";" + m_axisLabel).c_str(),
                    m_params.nU, m_params.minU, m_params.maxU,
                    m_params.nV, m_params.minV, m_params.maxV);
  }
  return hist;
}

void MaterialScan2D::fillValue(const std::string& name, double value)
{
  TH2D* hist = getHistogram(name);
  hist->Fill(m_curU, m_curV, value);
}

void MaterialScan2D::UserSteppingAction(const G4Step* step)
{
  //Get information about radiation and interaction length
  G4StepPoint* preStepPoint = step->GetPreStepPoint();
  G4Region* region = preStepPoint->GetPhysicalVolume()->GetLogicalVolume()->GetRegion();
  G4Material* material = preStepPoint->GetMaterial();
  double stlen = step->GetStepLength();
  double x0 = stlen / (material->GetRadlen());
  double lambda = stlen / (material->GetNuclearInterLength());
  B2DEBUG(20, "Step in at " << preStepPoint->GetPosition() << " in volume '"
          << preStepPoint->GetPhysicalVolume()->GetLogicalVolume()->GetName()
          << " (" << region->GetName() << ")"
          << " with length=" << stlen << " mm");
  checkStep(step);

  //check if the depth is limited
  if (m_params.maxDepth > 0) {
    m_curDepth += stlen;
    if (m_curDepth > m_params.maxDepth) {
      //Depth reached, kill track and add remaining part of the material budget
      G4Track* track = step->GetTrack();
      track->SetTrackStatus(fStopAndKill);
      double remaining = stlen - m_curDepth + m_params.maxDepth;
      x0 *= remaining / stlen;
      lambda *= remaining / stlen;
    }
  }

  if (std::binary_search(m_params.ignoredMaterials.begin(), m_params.ignoredMaterials.end(), material->GetName())) {
    return;
  }

  //Fill x0 and lambda in a histogram for each region
  string x0_total = "All_Regions_x0";
  string lambda_total = "All_Regions_lambda";
  string x0_name = region->GetName() + "_x0";
  string lambda_name = region->GetName() + "_lambda";
  //or for each Material
  if (m_params.splitByMaterials) {
    x0_total = "All_Materials_x0";
    lambda_total = "All_Materials_lambda";
    x0_name = material->GetName() + "_x0";
    lambda_name = material->GetName() + "_lambda";
  }
  fillValue(x0_total, x0);
  fillValue(lambda_total, lambda);
  fillValue(x0_name, x0);
  fillValue(lambda_name, lambda);
}

void MaterialScanSpherical::getRay(G4ThreeVector& origin, G4ThreeVector& direction)
{
  //We always shoot from the origin
  origin = m_origin;
  double theta = m_curU * Unit::deg;
  double phi   = m_curV * Unit::deg;
  if (m_doCosTheta) {
    theta = acos(m_curU);
  }
  direction.set(sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                cos(theta));
}

void MaterialScanPlanar::getRay(G4ThreeVector& origin, G4ThreeVector& direction)
{
  //We shoot perpendicular to the plane, so direction is always the same but the position varies.
  origin = m_origin + m_curU / Unit::mm * m_dirU + m_curV / Unit::mm * m_dirV;
  direction = m_dirW;
}

MaterialScanModule::MaterialScanModule(): m_rootFile(0), m_sphericalOrigin(3, 0), m_doCosTheta(false)
{
  //Set module properties
  setDescription("This Module is intended to scan the material budget of the "
                 "geometry.  Currently, there are two different kinds of scans "
                 "available: Spherical and Planar scan. \n"
                 "Spherical scan will shoot rays from the origin of the "
                 "detector and scan along the polar and azimuth angle.\n"
                 "Planar scan will shoot rays perpendicular to a given "
                 "plane.");

  //Set default ignored Materials
  m_spherical.ignoredMaterials.push_back("Vacuum");
  m_spherical.ignoredMaterials.push_back("Air");
  m_spherical.ignoredMaterials.push_back("G4_AIR");
  m_planar.ignoredMaterials = m_spherical.ignoredMaterials;

  addParam("Filename",            m_filename,
           "The filename where the material scan will be stored",
           string("MaterialScan.root"));
  addParam("spherical",           m_doSpherical,
           "Do a spherical scan, that is shooting rays from the origin with "
           "varying angles", true);
  addParam("spherical.origin",    m_sphericalOrigin,
           "Origin for the spherical scan", m_sphericalOrigin);
  addParam("spherical.nTheta",    m_spherical.nU,
           "Number of rays in theta", 200);
  addParam("spherical.minTheta",  m_spherical.minU,
           "Theta start angle", 17.);
  addParam("spherical.maxTheta",  m_spherical.maxU,
           "Theta stop angle", 150.);
  addParam("spherical.nPhi",      m_spherical.nV,
           "Number of rays in phi", 200);
  addParam("spherical.minPhi",    m_spherical.minV,
           "Phi start angle", 0.);
  addParam("spherical.maxPhi",    m_spherical.maxV,
           "Phi stop angle", 360.);
  addParam("spherical.maxDepth",  m_spherical.maxDepth,
           "Maximum scan depth in cm. The ray will be killed after having "
           "reached the maximum Depth. <=0 means no Limit.", -1.0);
  addParam("spherical.ignored",      m_spherical.ignoredMaterials,
           "Names of Materials which should be ignored when doing the scan", m_spherical.ignoredMaterials);
  addParam("spherical.splitByMaterials", m_spherical.splitByMaterials,
           "If True, split output by material names instead of by regions", false);
  addParam("spherical.cosTheta", m_doCosTheta,
           "If True, perform the spherical scan uniform in cos(theta) instead of theta", false);

  addParam("planar",              m_doPlanar,
           "Do a plane scan, that is shooting parallel rays from a defined "
           "plane", true);
  addParam("planar.plane",        m_planeName,
           "Plane to use for scanning, available are all two letter "
           "combinations of X,Y and Z like XY and XZ and custom", string("ZX"));
  addParam("planar.nU",           m_planar.nU,
           "Number of rays in U", 200);
  addParam("planar.minU",         m_planar.minU,
           "U start", -400.0);
  addParam("planar.maxU",         m_planar.maxU,
           "U stop", 400.0);
  addParam("planar.nV",           m_planar.nV,
           "Number of rays in V", 200);
  addParam("planar.minV",         m_planar.minV,
           "V start", -400.0);
  addParam("planar.maxV",         m_planar.maxV,
           "V stop", 400.0);
  addParam("planar.maxDepth",     m_planar.maxDepth,
           "Maximum scan depth in cm. The ray will be killed after having "
           "reached the maximum Depth. <=0 means no Limit.", -1.0);
  addParam("planar.custom",       m_customPlane,
           "Parameters of the plane when choosing custom. This is supposed to "
           "be a list of 9 values: The first three are the coordinates of the "
           "plane origin. The second three are the direction of U in the "
           "r-phi coordinates. The last three are the direction of V in the "
           "coordinates parallel to the detector axis (beamline). ", m_customPlane);
  addParam("planar.ignored",      m_planar.ignoredMaterials,
           "Names of Materials which should be ignored when doing the scan", m_planar.ignoredMaterials);
  addParam("planar.splitByMaterials", m_planar.splitByMaterials,
           "If True, split output by material names instead of by regions", false);

}

void MaterialScanModule::initialize()
{
  m_rootFile = new TFile(m_filename.c_str(), "RECREATE");

  boost::to_lower(m_planeName);
  boost::trim(m_planeName);
  //Check if plane definition makes sense
  if (m_planeName == "custom") {
    if (m_customPlane.size() < 9) {
      B2ERROR("planar.custom: At least 9 values needed to define custom plane, only " << m_customPlane.size() << " provided");
    }
  } else if (m_planeName.size() != 2 || !boost::all(m_planeName, boost::is_any_of("xyz"))) {
    B2ERROR("planar.plane: Only custom or any two letter combinations of X,Y and Z are recognized");
  } else if (m_planeName[0] == m_planeName[1]) {
    B2ERROR("planar.plane: " << m_planeName << " not valid, cannot use the same axis twice");
  }

  //Check if we have enough values to define the origin
  if (m_sphericalOrigin.size() < 3) {
    B2ERROR("spherical.origin: Three values are needed to define a point, only " << m_sphericalOrigin.size() << " given.");
  }

  //Convert plane definition to mm since Geant4 is of course using other units
  for (double& value : m_customPlane) value /= Unit::mm;
  for (double& value : m_sphericalOrigin) value /= Unit::mm;
}

void MaterialScanModule::terminate()
{
  if (m_rootFile) {
    m_rootFile->Write();
    m_rootFile->Close();
  }
}

void MaterialScanModule::beginRun()
{
  G4EventManager* eventManager = G4EventManager::GetEventManager();

  //First we save all user actions
  G4UserEventAction*    vanillaEventAction    = eventManager->GetUserEventAction();
  G4UserStackingAction* vanillaStackingAction = eventManager->GetUserStackingAction();
  G4UserTrackingAction* vanillaTrackingAction = eventManager->GetUserTrackingAction();
  G4UserSteppingAction* vanillaSteppingAction = eventManager->GetUserSteppingAction();

  //Then we clear the user actions
  eventManager->SetUserAction((G4UserEventAction*)0);
  eventManager->SetUserAction((G4UserStackingAction*)0);
  eventManager->SetUserAction((G4UserTrackingAction*)0);

  //We create our own stepping actions which will
  //- create the vectors for shooting rays
  //- record the material budget for each ray
  vector<MaterialScanBase*> scans;
  if (m_doSpherical) {
    G4ThreeVector origin(m_sphericalOrigin[0], m_sphericalOrigin[1], m_sphericalOrigin[2]);
    scans.push_back(new MaterialScanSpherical(m_rootFile, origin, m_spherical, m_doCosTheta));
  }
  if (m_doPlanar) {
    G4ThreeVector origin(0, 0, 0);
    G4ThreeVector uDir;
    G4ThreeVector vDir;
    if (m_planeName == "custom") {
      origin.set(m_customPlane[0], m_customPlane[1], m_customPlane[2]);
      uDir.set(m_customPlane[3], m_customPlane[4], m_customPlane[5]);
      vDir.set(m_customPlane[6], m_customPlane[7], m_customPlane[8]);
    } else {
      uDir = getAxis(m_planeName[0]);
      vDir = getAxis(m_planeName[1]);
    }
    scans.push_back(new MaterialScanPlanar(m_rootFile, origin, uDir, vDir, m_planar));
  }

  //Do each configured scan
  for (MaterialScanBase* scan : scans) {
    //Set the Scan as steppingaction to see material
    eventManager->SetUserAction(scan);
    //Now we can scan
    G4RayShooter rayShooter;
    G4ThreeVector origin;
    G4ThreeVector direction;
    int maxRays = scan->getNRays();
    int curRay(0);
    int lastPercent(-1);
    double start = Utils::getClock();
    //Create one event per ray and process it
    while (scan->createNext(origin, direction)) {
      G4Event* event = new G4Event(++curRay);
      rayShooter.Shoot(event, origin, direction);
      eventManager->ProcessOneEvent(event);
      delete event;

      //Show progress
      int donePercent = 100 * curRay / maxRays;
      if (donePercent > lastPercent) {
        double perRay = (Utils::getClock() - start) / curRay;
        double eta = perRay * (maxRays - curRay);
        B2INFO(boost::format("%s Scan: %3d%%, %.3f ms per ray, ETA: %.2f seconds")
               % scan->getName() % donePercent
               % (perRay / Unit::ms) % (eta / Unit::s));
        lastPercent = donePercent;
      }
    }
    //Free the scan object
    delete scan;
  }

  //And now we reset the user actions
  eventManager->SetUserAction(vanillaEventAction);
  eventManager->SetUserAction(vanillaStackingAction);
  eventManager->SetUserAction(vanillaTrackingAction);
  eventManager->SetUserAction(vanillaSteppingAction);
}

G4ThreeVector MaterialScanModule::getAxis(char name)
{
  switch (name) {
    case 'x': return G4ThreeVector(1, 0, 0);
    case 'y': return G4ThreeVector(0, 1, 0);
    case 'z': return G4ThreeVector(0, 0, 1);
    default:
      B2FATAL("Unknown axis: " << name);
  }
  return G4ThreeVector(0, 0, 0);
}
