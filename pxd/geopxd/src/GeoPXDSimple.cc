#include <test_beam/geotb/GeoPXDSimple.h>
#include <framework/logging/Logger.h>
#include <pxd/simpxd/PXDSensitiveDetector.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>

#include <pxd/dataobjects/PXDVolumeUserInfo.h>


using namespace std;
using namespace boost;
using namespace Belle2;


GeoPXDSimple regGeoPXDSimple;


GeoPXDSimple::GeoPXDSimple() : CreatorBase("PXDSimple")
{
  setDescription("Creates the TGeo objects for the PXD geometry of the Test Beam.");
  //The PXD subdetector uses the "SD_" prefix to flag its sensitive volumes
  addSensitiveDetector("SD_", new PXDSensitiveDetector("PXDSensitiveDetector"));
}


GeoPXDSimple::~GeoPXDSimple()
{

}


void GeoPXDSimple::create(GearDir& content)
{

  // Creation code goes here…
  B2INFO("Test Beam Creator called!");

  // Add subdetector group
  TGeoVolumeAssembly* volTBPXD = addSubdetectorGroup("PXD", new TGeoTranslation(0, 0, 0));

  //Get number of Containers
  int nContainer = 0;
  if (content.isPathValid("Containers/Container")) nContainer = content.getNumberNodes("Containers/Container");

  if (nContainer == 0) {
    B2FATAL("Could not find any containers in Containers node.");
    return;
  }

  // Container Params
  std::string cName = "";
  double cDX = 0.0;
  double cDY = 0.0;
  double cDZ = 0.0;
  double cPhi = 0.0;
  double cTheta = 0.0;
  double cPsi = 0.0;
  std::string cShape = "";
  double cHalfX = 0.0;
  double cHalfZ = 0.0;
  double cHalfY = 0.0;
  std::string cMaterial = "";



  for (int iContainer = 1; iContainer <= nContainer; ++iContainer) {
    int nActiveVolume = 0;
    int nPassiveVolume = 0;

    // initialize new PXD Container object
    GearDir cContent(content);
    cContent.append((format("Containers/Container[%1%]/") % (iContainer)).str());

    // get params
    cName = cContent.getParamString("Name");
    cDX = cContent.getParamLength("DX");
    cDY = cContent.getParamLength("DY");
    cDZ = cContent.getParamLength("DZ");
    cPhi = cContent.getParamAngle("Phi");
    cTheta = cContent.getParamAngle("Theta");
    cPsi = cContent.getParamAngle("Psi");
    cShape = cContent.getParamString("Shape");
    cHalfX = cContent.getParamLength("HalfX");
    cHalfY = cContent.getParamLength("HalfY");
    cHalfZ = cContent.getParamLength("HalfZ");
    cMaterial = cContent.getParamString("Material");

    // count active and passive volumes inside container
    if (cContent.isPathValid("ActiveVolume")) nActiveVolume = cContent.getNumberNodes("ActiveVolume");
    if (cContent.isPathValid("PassiveVolume")) nPassiveVolume = cContent.getNumberNodes("PassiveVolume");
    // either ActiveVolume or PassiveVolume must be present inside a Container
    if (!nActiveVolume && !nPassiveVolume) B2ERROR("At least one ActiveVolume or PassiveVolume element required in a Container.");
    B2INFO("There are " << nActiveVolume << " active and " << nPassiveVolume << " passive volumes in container with name " << cName);

    // Initialize Container volume
    const char * cMaterial_char = cMaterial.c_str();
    TGeoMedium* cMedium = gGeoManager->GetMedium(cMaterial_char);

    // Create the Container volume
    TGeoVolume * cVolume = gGeoManager->MakeBox(cName.c_str(), cMedium, cHalfX, cHalfY, cHalfZ);

    // Loop over passive volumes

    for (int iPVolume = 1; iPVolume <= nPassiveVolume; iPVolume++) {
      GearDir pvContent(cContent);
      pvContent.append((boost::format("PassiveVolume[%1%]/") % (iPVolume)).str());

      std::string pvName = pvContent.getParamString("Name");
      double pvDX = pvContent.getParamLength("DX");
      double pvDY = pvContent.getParamLength("DY");
      double pvDZ = pvContent.getParamLength("DZ");
      double pvPhi = pvContent.getParamAngle("Phi");
      double pvTheta = pvContent.getParamAngle("Theta");
      double pvPsi = pvContent.getParamAngle("Psi");
      std::string pvShape = pvContent.getParamString("Shape");
      double pvHalfX = pvContent.getParamLength("HalfX");
      double pvHalfY = pvContent.getParamLength("HalfY");
      double pvHalfZ = pvContent.getParamLength("HalfZ");
      std::string pvMaterial = pvContent.getParamString("Material");
      int pvColor = pvContent.getParamNumValue("Color");

      // Initialize Container volume
      const char * pvMaterial_char = pvMaterial.c_str();
      TGeoMedium* pvMedium = gGeoManager->GetMedium(pvMaterial_char);

      // Create the Container volume
      TGeoVolume * pvVolume = gGeoManager->MakeBox(pvName.c_str(), pvMedium, pvHalfX, pvHalfY, pvHalfZ);
      pvVolume->SetLineColor(pvColor);
      cVolume->AddNode(pvVolume, 1, new TGeoTranslation(pvDX, pvDY, pvDZ));

    }

    // Loop over active volumes
    for (int iAVolume = 1; iAVolume <= nActiveVolume; iAVolume++) {
      GearDir avContent(cContent);
      avContent.append((boost::format("ActiveVolume[%1%]/") % (iAVolume)).str());

      std::string avName = avContent.getParamString("Name");
      int avSensorId = avContent.getParamNumValue("SensorId");

      double avDX = avContent.getParamLength("DX");
      double avDY = avContent.getParamLength("DY");
      double avDZ = avContent.getParamLength("DZ");
      double avPhi = avContent.getParamAngle("Phi");
      double avTheta = avContent.getParamAngle("Theta");
      double avPsi = avContent.getParamAngle("Psi");
      std::string avShape = avContent.getParamString("Shape");
      double avHalfX = avContent.getParamLength("HalfX");
      double avHalfY = avContent.getParamLength("HalfY");
      double avHalfZ = avContent.getParamLength("HalfZ");
      double avUPitch = avContent.getParamLength("UPitch");
      int avUCells = avContent.getParamNumValue("UCells");
      double avVPitch = avContent.getParamLength("VPitch");

      int avVCells = avContent.getParamNumValue("VCells");

      std::string avMaterial = avContent.getParamString("Material");
      int avColor = avContent.getParamNumValue("Color");

      B2INFO("Basic params for active volume with name " << avName << " OK.");

      // Initialize Container volume
      const char * avMaterial_char = avMaterial.c_str();
      TGeoMedium* avMedium = gGeoManager->GetMedium(avMaterial_char);

      // Create the Container volume
      TGeoVolume * avVolume = gGeoManager->MakeBox(avName.c_str(), avMedium, avHalfX, avHalfY, avHalfZ);
      avVolume->SetLineColor(avColor);
      avVolume->SetField(new PXDVolumeUserInfo(1, iContainer - 1, avSensorId, avUPitch, avUCells, avVPitch, avVCells));
      cVolume->AddNode(avVolume, 1, new TGeoTranslation(avDX, avDY, avDZ));
      B2INFO("PXD with sensor_id " << avSensorId << " created.");
    }

    // Add Container - Set color to invisible!
    cVolume->SetLineColor(0);
    volTBPXD->AddNode(cVolume, 1, new TGeoTranslation(cDX, cDY, cDZ));
  }



}



