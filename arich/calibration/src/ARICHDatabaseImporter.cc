/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/calibration/ARICHDatabaseImporter.h>
#include <arich/dbobjects/ARICHAerogelInfo.h>
#include <arich/dbobjects/ARICHAsicInfo.h>
#include <arich/dbobjects/ARICHHapdQA.h>
#include <arich/dbobjects/ARICHFebTest.h>
#include <arich/dbobjects/ARICHHapdChipInfo.h>
#include <arich/dbobjects/ARICHHapdInfo.h>
#include <arich/dbobjects/ARICHHapdQE.h>
#include <arich/dbobjects/ARICHMagnetTest.h>
#include <arich/dbobjects/ARICHModuleTest.h>
#include <arich/dbobjects/ARICHSensorModuleInfo.h>
#include <arich/dbobjects/ARICHSensorModuleMap.h>

// database classes used by simulation/reconstruction software
#include <arich/dbobjects/ARICHChannelMask.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHModulesInfo.h>
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <arich/dbobjects/ARICHCopperMapping.h>
#include <arich/dbobjects/ARICHSimulationPar.h>
#include <arich/dbobjects/ARICHGeometryConfig.h>

// channel histogram
#include <arich/utility/ARICHChannelHist.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/database/EventDependency.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBStore.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TKey.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <TClonesArray.h>
#include <TTree.h>
#include <tuple>
#include <boost/filesystem.hpp>

using namespace std;
using namespace Belle2;

ARICHDatabaseImporter::ARICHDatabaseImporter(vector<string> inputFilesHapdQA, vector<string> inputFilesAsicRoot,
                                             vector<string> inputFilesAsicTxt, vector<string> inputFilesHapdQE, vector<string> inputFilesFebTest)
{
  for (unsigned int i = 0; i < inputFilesHapdQA.size(); i++) {  m_inputFilesHapdQA.push_back(inputFilesHapdQA[i]); }
  for (unsigned int i = 0; i < inputFilesAsicRoot.size(); i++) {  m_inputFilesAsicRoot.push_back(inputFilesAsicRoot[i]); }
  for (unsigned int i = 0; i < inputFilesAsicTxt.size(); i++) {  m_inputFilesAsicTxt.push_back(inputFilesAsicTxt[i]); }
  for (unsigned int i = 0; i < inputFilesHapdQE.size(); i++) {  m_inputFilesHapdQE.push_back(inputFilesHapdQE[i]); }
  for (unsigned int i = 0; i < inputFilesFebTest.size(); i++) {  m_inputFilesFebTest.push_back(inputFilesFebTest[i]); }
}



// classses for simulation/reconstruction software
void ARICHDatabaseImporter::importModulesInfo()
{

  // geometry configuration
  DBObjPtr<ARICHGeometryConfig> geoConfig;

  // QE 2D maps from DB
  DBArray<ARICHHapdQE> QEMaps("ARICHHapdQE");

  ARICHModulesInfo modInfo;

  // channel mapping used in QA tests (QE measurements, etc.)
  ARICHChannelMapping QAChMap;

  // read mapping from xml file
  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/ChannelMapping");
  istringstream chstream;
  int x, y, asic;
  chstream.str(content.getString("QAChannelMapping"));
  while (chstream >> x >> y >> asic) {
    QAChMap.mapXY2Asic(x, y, asic);
  }

  // get list of installed modules from xml
  content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/InstalledModules");
  std::cout << "Installed modules" << std::endl;

  std::vector<std::string> installed;

  for (const GearDir& module : content.getNodes("Module")) {
    std::string hapdID = module.getString("@hapdID");

    unsigned sector = module.getInt("Sector");
    unsigned ring = module.getInt("Ring");
    unsigned azimuth = module.getInt("Azimuth");
    bool isActive = (bool)module.getInt("isActive");
    std::cout << " " << hapdID << ":  S " << sector << "  R " << ring << "  Z " << azimuth <<  ", isActive: " << isActive << std::endl;

    if (std::find(installed.begin(), installed.end(), hapdID) != installed.end()) {
      B2WARNING("ARICHDatabaseImporter::importModulesInfo: hapd " << hapdID << " installed multiple times!");
    } else installed.push_back(hapdID);

    unsigned moduleID = geoConfig->getDetectorPlane().getSlotIDFromSRF(sector, ring, azimuth);

    // get and set QE map
    std::vector<float> qs;
    qs.assign(144, 0);
    bool init = false;
    for (const auto& QEMap : QEMaps) {
      if (hapdID ==  QEMap.getHapdSerialNumber()) {
        TH2F* qe2d = QEMap.getQuantumEfficiency2D();
        for (int k = 1; k < 13; k++) {
          for (int l = 1; l < 13; l++) {
            int asicCh = QAChMap.getAsicFromXY(k - 1, l - 1);
            qs[asicCh] = qe2d->GetBinContent(k, l);
          }
        }
        init = true;
        std::cout << "  Channels QE map found and set." << std::endl;
      }
    }

    if (!init) {
      for (int k = 0; k < 144; k++) {
        qs[k] = 27.0;
      }
      B2WARNING("ARICHDatabaseImporter::importModulesInfo: QE map for hapd " << hapdID << " not found! Setting 27% QE for all channels!");
    }

    modInfo.addModule(moduleID, qs, isActive);

  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHModulesInfo> importObj;
  importObj.construct(modInfo);
  importObj.import(iov);

}

void ARICHDatabaseImporter::setHAPDQE(unsigned modID, double qe, bool import)
{

  DBObjPtr<ARICHModulesInfo> modInfo;
  if (modID < 1 || modID > 420) { B2ERROR("Module ID out of range!"); return;}

  for (int k = 0; k < 144; k++) {
    modInfo->setChannelQE(modID, k, qe);
  }

  if (import) {
    IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
    DBImportObjPtr<ARICHModulesInfo> importObj;
    importObj.construct(*modInfo);
    importObj.import(iov);
  }
}


void ARICHDatabaseImporter::importChannelMask()
{

  DBObjPtr<ARICHGeometryConfig> geoConfig;

  // module test results from DB (we take list of dead channels from here)
  DBArray<ARICHModuleTest> moduleTest("ARICHModuleTest");

  ARICHChannelMask chanMask;

  // read mapping of HAPD channels to asic channels from xml file
  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/ChannelMapping");
  istringstream chstream2;
  int hapdCh, asic;
  chstream2.str(content.getString("HapdAsicChannelMapping"));
  std::vector<int> hapdChMap;
  hapdChMap.assign(144, -1);
  while (chstream2 >> hapdCh >> asic) {
    hapdChMap[hapdCh - 1] = asic;
  }

  for (auto ch : hapdChMap) if (ch == -1)
      B2ERROR("ARICHDatabaseImporter::importLWClasses: HAPD channel to asic channel mapping not set correctly!");

  // loop over installed modules (from xml file)
  content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/InstalledModules");
  std::cout << "Installed modules" << std::endl;
  for (const GearDir& module : content.getNodes("Module")) {
    std::string hapdID = module.getString("@hapdID");
    unsigned sector = module.getInt("Sector");
    unsigned ring = module.getInt("Ring");
    unsigned azimuth = module.getInt("Azimuth");
    bool isActive = (bool)module.getInt("isActive");
    std::cout << " " << hapdID << ":  S " << sector << "  R " << ring << "  Z " << azimuth <<  ", isActive: " << isActive << std::endl;
    unsigned moduleID = geoConfig->getDetectorPlane().getSlotIDFromSRF(sector, ring, azimuth);

    // get and set channel mask (mask dead channels)
    bool init = false;
    for (const auto& test : moduleTest) {
      if (hapdID ==  test.getHapdSN()) {

        // loop over list of dead channels
        for (int i = 0; i < test.getDeadChsSize(); i++) {
          unsigned hapdCh = test.getDeadCh(i);
          chanMask.setActiveCh(moduleID, hapdChMap[hapdCh - 1], false);
        }
        init = true;
        std::cout << "  List of dead channels (from module test) found and set." << std::endl;
      }
    }

    if (!init) {
      B2WARNING("ARICHDatabaseImporter::importLWClasses: List of dead channels for hapd " << hapdID <<
                " not found! All channels set to active.");
      continue;
    }
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHChannelMask> importObj;
  importObj.construct(chanMask);
  importObj.import(iov);

}


void ARICHDatabaseImporter::importSimulationParams()
{

  ARICHSimulationPar simPar;

  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content/SimulationParameters");

  double qeScale = content.getDouble("qeScale");
  double winAbs = content.getDouble("windowAbsorbtion");
  double crossTalk = content.getDouble("chipNegativeCrosstalk");
  double colEff = content.getDouble("colEff");

  GearDir qeParams(content, "QE");
  float lambdaFirst = qeParams.getLength("LambdaFirst") / Unit::nm;
  float lambdaStep = qeParams.getLength("LambdaStep") / Unit::nm;
  double peakQE = qeParams.getDouble("peakQE");

  std::vector<float> qes;
  for (const auto& qeff : qeParams.getNodes("Qeffi")) {
    qes.push_back(qeff.getDouble("qe"));
  }

  simPar.setQECurve(lambdaFirst, lambdaStep, qes);
  simPar.setCollectionEff(colEff);
  simPar.setChipNegativeCrosstalk(crossTalk);
  simPar.setWindowAbsorbtion(winAbs);
  simPar.setQEScaling(qeScale);
  simPar.setPeakQE(peakQE);

  simPar.print();

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHSimulationPar> importObj;
  importObj.construct(simPar);
  importObj.import(iov);

}

void ARICHDatabaseImporter::importChannelMapping()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content");
  ARICHChannelMapping chMap;

  istringstream chstream;
  int x, y, asic;
  chstream.str(content.getString("ChannelMapping/SoftChannelMapping"));
  std::cout << "Importing channel x,y to asic channel map" << std::endl;
  std::cout << "  x   y   asic" << std::endl;
  while (chstream >> x >> y >> asic) {
    chMap.mapXY2Asic(x, y, asic);
    std::cout << " " << setw(2) << x << "  " << setw(2) << y << "   " << setw(3) << asic << std::endl;
  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHChannelMapping> importObjMap;
  importObjMap.construct(chMap);
  importObjMap.import(iov);

}

void ARICHDatabaseImporter::importFEMappings()
{

  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content");

  DBObjPtr<ARICHGeometryConfig> geoConfig;

  ARICHMergerMapping mergerMap;
  ARICHCopperMapping copperMap;
  GearDir mapping(content, "FrontEndMapping");

  for (const GearDir& merger : mapping.getNodes("Merger")) {
    unsigned mergerID = (unsigned) merger.getInt("@id");
    std::cout << std::endl << "Mapping of modules to merger no. " << mergerID << std::endl;
    for (const GearDir& module : merger.getNodes("Modules/Module")) {
      unsigned sector = module.getInt("Sector");
      unsigned ring = module.getInt("Ring");
      unsigned azimuth = module.getInt("Azimuth");
      unsigned moduleID = geoConfig->getDetectorPlane().getSlotIDFromSRF(sector, ring, azimuth);
      unsigned slot = (unsigned) module.getInt("@FEBSlot");
      mergerMap.addMapping(moduleID, mergerID, slot);
      std::cout << std::endl << " FEB slot: " << slot << ", module position: S" << sector << " R" << ring << " Z" << azimuth <<
                ", module ID: " << moduleID << std::endl;
      std::cout << " crosscheck:  mergerMap.getMergerID(" << moduleID << ") = " <<  mergerMap.getMergerID(
                  moduleID) << ", mergerMap.getFEBSlot(" << moduleID << ") = " << mergerMap.getFEBSlot(moduleID) << ", mergerMap.getModuleID(" <<
                mergerID << "," << slot << ") = " <<  mergerMap.getModuleID(mergerID, slot) << std::endl;
    }
    std::cout << std::endl;

    unsigned copperID = (unsigned) merger.getInt("COPPERid");
    string finesseSlot = merger.getString("FinesseSlot");
    int finesse = 0;
    if (finesseSlot == "A") {finesse = 0;}
    else if (finesseSlot == "B") {finesse = 1;}
    else if (finesseSlot == "C") {finesse = 2;}
    else if (finesseSlot == "D") {finesse = 3;}
    else {
      B2ERROR(merger.getPath() << "/FinesseSlot " << finesseSlot <<
              " ***invalid slot (valid are A, B, C, D)");
      continue;
    }
    copperMap.addMapping(mergerID, copperID, finesse);
    std::cout << "Merger " << mergerID << " connected to copper " << copperID << ", finesse " << finesse << std::endl;

  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHMergerMapping> importObjMerger;
  importObjMerger.construct(mergerMap);
  importObjMerger.import(iov);

  DBImportObjPtr<ARICHCopperMapping> importObjCopper;
  importObjCopper.construct(copperMap);
  importObjCopper.import(iov);

}

void ARICHDatabaseImporter::importGeometryConfig()
{

  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content");
  ARICHGeometryConfig arichGeometryConfig(content);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHGeometryConfig> importObj;
  importObj.construct(arichGeometryConfig);
  importObj.import(iov);

}

void ARICHDatabaseImporter::importCosmicTestGeometry()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name='ARICH']/Content");
  GearDir cosmic(content, "CosmicTest");
  DBObjPtr<ARICHGeometryConfig> geoConfig;

  GearDir masterDir(cosmic, "MasterVolume");
  ARICHGeoMasterVolume master = geoConfig->getMasterVolume();
  master.setPlacement(masterDir.getLength("Position/x"), masterDir.getLength("Position/y"), masterDir.getLength("Position/z"),
                      masterDir.getAngle("Rotation/x"), masterDir.getAngle("Rotation/y"), masterDir.getAngle("Rotation/z"));
  master.setVolume(master.getInnerRadius(), master.getOuterRadius(), 100., master.getMaterial());
  geoConfig->setMasterVolume(master);


  GearDir aerogel(cosmic, "Aerogel");
  std::vector<double> par = {aerogel.getLength("xSize"), aerogel.getLength("ySize"), aerogel.getLength("xPosition"), aerogel.getLength("yPosition"), aerogel.getAngle("zRotation")};
  ARICHGeoAerogelPlane plane = geoConfig->getAerogelPlane();
  plane.setSimple(par);
  geoConfig->setAerogelPlane(plane);

  GearDir scints(cosmic, "Scintilators");
  double size[3] = {scints.getLength("xSize"), scints.getLength("ySize"), scints.getLength("zSize")};
  std::string scintMat = scints.getString("Material");

  ARICHGeoSupport support = geoConfig->getSupportStructure();
  support.clearBoxes();
  for (const GearDir& scint : scints.getNodes("Scintilator")) {
    std::string name = scint.getString("@name");
    double position[3] = {scint.getLength("Position/x"), scint.getLength("Position/y"), scint.getLength("Position/z")};
    double rotation[3] = {scint.getAngle("Rotation/x"), scint.getAngle("Rotation/y"), scint.getAngle("Rotation/z")};
    support.addBox(name, scintMat, size, position, rotation);
  }

  geoConfig->setSupportStructure(support);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  DBImportObjPtr<ARICHGeometryConfig> geoImport;
  geoImport.construct(*geoConfig);
  geoImport.import(iov);

}

void ARICHDatabaseImporter::printSimulationPar()
{
  DBObjPtr<ARICHSimulationPar> simPar;
  simPar->print();
}

void ARICHDatabaseImporter::printGeometryConfig()
{
  DBObjPtr<ARICHGeometryConfig> geoConfig;
  geoConfig->print();

}

void ARICHDatabaseImporter::printChannelMapping()
{
  DBObjPtr<ARICHChannelMapping> chMap;
  chMap->print();
}

void ARICHDatabaseImporter::printFEMappings()
{
  DBObjPtr<ARICHMergerMapping> mrgMap;
  DBObjPtr<ARICHCopperMapping> copMap;
  mrgMap->print();
  copMap->print();
}

void ARICHDatabaseImporter::printModulesInfo()
{
  DBObjPtr<ARICHModulesInfo> modinfo;
  modinfo->print();
}

void ARICHDatabaseImporter::printChannelMask()
{
  DBObjPtr<ARICHChannelMask> chMask;
  chMask->print();
}

void ARICHDatabaseImporter::dumpModuleNumbering()
{

  ARICHChannelHist* hist = new ARICHChannelHist("moduleNum", "HAPD module slot numbering", 1);
  for (int hapdID = 1; hapdID < 421; hapdID++) {
    hist->setBinContent(hapdID, hapdID);
  }
  hist->SetOption("TEXT");
  hist->SaveAs("ModuleNumbering.root");

}

void ARICHDatabaseImporter::dumpQEMap(bool simple)
{
  DBObjPtr<ARICHModulesInfo> modInfo;
  DBObjPtr<ARICHGeometryConfig> geoConfig;
  DBObjPtr<ARICHChannelMapping> chMap;

  if (simple) {
    TGraph2D* gr = new TGraph2D();
    int point = 0;
    for (int i = 1; i < 421; i++) {
      if (modInfo->isInstalled(i)) {
        for (int j = 0; j < 144; j++) {
          int xCh, yCh;
          chMap->getXYFromAsic(j, xCh, yCh);
          TVector2 chPos = geoConfig->getChannelPosition(i, xCh, yCh);
          gr->SetPoint(point, chPos.X(), chPos.Y(), modInfo->getChannelQE(i, j));
          point++;
        }
      }
    }
    gr->SaveAs("QEMap.root");
  } else {
    ARICHChannelHist* hist = new ARICHChannelHist("hapdQE", "hapd QE map");
    for (int hapdID = 1; hapdID < 421; hapdID++) {
      if (!modInfo->isInstalled(hapdID)) continue;
      for (int chID = 0; chID < 144; chID++) {
        hist->setBinContent(hapdID, chID, modInfo->getChannelQE(hapdID, chID));
      }
    }
    hist->SaveAs("QEMap.root");
  }
}




// classes for quality assessment and test data
void ARICHDatabaseImporter::importAerogelInfo()
{
  GearDir content = GearDir("/ArichData/AllData/AerogelData/Content");

  // define data array
  TClonesArray agelConstants("Belle2::ARICHAerogelInfo");
  int agel = 0;

  // loop over xml files and extract the data
  for (const auto& aerogel : content.getNodes("aerogel")) {
    float version = (float) aerogel.getDouble("version");
    string serial = aerogel.getString("serial");
    string id = aerogel.getString("id");
    float index = (float) aerogel.getDouble("index");
    float trlen = ((float) aerogel.getDouble("translength")) * Unit::mm;
    float thickness = ((float) aerogel.getDouble("thick")) * Unit::mm;
    vector<int> lambdas;
    vector<float> transmittances;
    for (const auto& transmittance : aerogel.getNodes("transmittance/transpoint")) {
      int lambda = transmittance.getInt("@lambda");
      float val = (float) transmittance.getDouble(".");
      lambdas.push_back(lambda);
      transmittances.push_back(val);
    }

    // save data as an element of the array
    new(agelConstants[agel]) ARICHAerogelInfo(version, serial, id, index, trlen, thickness, lambdas, transmittances);
    agel++;
  }

  // define interval of validity
//  IntervalOfValidity iov(0, 0, 1, 99);
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // store under default name:
  // Database::Instance().storeData(&agelConstants, iov);
  // store under user defined name:
  Database::Instance().storeData("ARICHAerogelInfo", &agelConstants, iov);
}

void ARICHDatabaseImporter::exportAerogelInfo()
{

  // Definition:
  // This function extracts data from the database for chosen event.

  /*
    // Extract data from payload with bounded validity

    // Define event, run and experiment numbers
    EventMetaData event = EventMetaData(1200,4,0); // (event, run, exp)

    // Extract object and IOV from database
    std::pair<TObject*, IntervalOfValidity> podatki = Database::Instance().getData(event, "dbstore", "ARICHAerogelInfo");

    // print interval of validity
  //  IntervalOfValidity iov = std::get<1>(podatki);
  //  B2INFO("iov = " << iov);

    // Convert between different class types and extract TClonesArray
    // for chosen event
    TObject* data;
    data = std::get<0>(podatki);
    TClonesArray* elements = static_cast<TClonesArray*>(data);

    // Get entries from TClonesArray and print aerogel info
    (*elements).GetEntries();
    for (int i = 0; i < elements->GetSize(); i++) {
      ARICHAerogelInfo* myelement = (ARICHAerogelInfo*)elements->At(i);
      B2INFO("Version = " << myelement->getAerogelVersion() << ", SN = " << myelement->getAerogelSerial() << ", n = " << myelement->getAerogelRefractiveIndex() << ", trl = " << myelement->getAerogelTransmissionLength() << ", thickness = " << myelement->getAerogelThickness());
    }
  */



  // Extract data from payload with unbounded validity
  DBArray<ARICHAerogelInfo> elements("ARICHAerogelInfo");
  elements.getEntries();

  // Print aerogel info
  for (const auto& element : elements) {
    B2INFO("Version = " << element.getAerogelVersion() << ", serial = " << element.getAerogelSerial() <<
           ", id = " << element.getAerogelID() << ", n = " << element.getAerogelRefractiveIndex() << ", transmLength = " <<
           element.getAerogelTransmissionLength() << ", thickness = " << element.getAerogelThickness());
  }
}



void ARICHDatabaseImporter::importAerogelInfoEventDep()
{
  GearDir content = GearDir("/ArichData/AllData/AerogelData/Content");

  // define data arrays
  TClonesArray agelConstantsA("Belle2::ARICHAerogelInfo");
  TClonesArray agelConstantsB("Belle2::ARICHAerogelInfo");
  TClonesArray agelConstantsC("Belle2::ARICHAerogelInfo");


  for (int someint = 0; someint < 3; someint++) {
    int agel = 0;
    // loop over xml files and extract the data
    for (const auto& aerogel : content.getNodes("aerogel")) {
      // different version is made up - only used to check performance
      // of intrarun dependat function
      float version = 0;
      if (someint == 0)     version = (float) aerogel.getDouble("version");
      if (someint == 1)     version = 4.0;
      if (someint == 2)     version = 5.0;

      string serial = aerogel.getString("serial");
      string id = aerogel.getString("id");
      float index = (float) aerogel.getDouble("index");
      float trlen = ((float) aerogel.getDouble("translength")) * Unit::mm;
      float thickness = ((float) aerogel.getDouble("thick")) * Unit::mm;
      vector<int> lambdas;
      vector<float> transmittances;
      for (const auto& transmittance : aerogel.getNodes("transmittance/transpoint")) {
        int lambda = transmittance.getInt("@lambda");
        float val = (float) transmittance.getDouble(".");
        lambdas.push_back(lambda);
        transmittances.push_back(val);
      }

      // save data as an element of the array
      if (someint == 0)    new(agelConstantsA[agel]) ARICHAerogelInfo(version, serial, id, index, trlen, thickness, lambdas,
            transmittances);
      if (someint == 1)    new(agelConstantsB[agel]) ARICHAerogelInfo(version, serial, id, index, trlen, thickness, lambdas,
            transmittances);
      if (someint == 2)    new(agelConstantsC[agel]) ARICHAerogelInfo(version, serial, id, index, trlen, thickness, lambdas,
            transmittances);
      agel++;
    }
  }

  // set interval of validity
  IntervalOfValidity iov(0, 0, 0, 99);
//  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // convert pointers to ARICHAerogelInfo into pointers to TObject
  TObject* agelObj[3];
  agelObj[0] = static_cast<TObject*>(&agelConstantsA);
  agelObj[1] = static_cast<TObject*>(&agelConstantsB);
  agelObj[2] = static_cast<TObject*>(&agelConstantsC);

  // add objects with different validity
  EventDependency intraRun(agelObj[0]);
  intraRun.add(500, agelObj[1]);   // valid from event number 500
  intraRun.add(1000, agelObj[2]);  // valid from event number 1000

  // store under user defined name
  Database::Instance().storeData("ARICHAerogelInfoEventDep", &intraRun, iov);

}

void ARICHDatabaseImporter::exportAerogelInfoEventDep()
{
  // Definition:
  // This function extracts intrarun dependant data from the database.
  // It converts between different class types to get the saved
  // TClonesArray for chosen event, run and experiment.


  // Define event, run and experiment numbers
  EventMetaData event = EventMetaData(1200, 4, 0); // (event, run, exp)

  // Extract object and IOV from database
  std::pair<TObject*, IntervalOfValidity> podatki = Database::Instance().getData(event, "dbstore", "ARICHAerogelInfoEventDep");

  // print interval of validity
//  IntervalOfValidity iov = std::get<1>(podatki);
//  B2INFO("iov = " << iov);

  // Convert between different class types and extract TClonesArray
  // for chosen event
  TObject* data = std::get<0>(podatki);
  EventDependency* data2 = static_cast<EventDependency*>(data);
  TObject* myobject = data2->getObject(event);
  TClonesArray* elements = static_cast<TClonesArray*>(myobject);

  // Get entries from TClonesArray and print aerogel info
  (*elements).GetEntries();
  for (int i = 0; i < elements->GetSize(); i++) {
    ARICHAerogelInfo* myelement = (ARICHAerogelInfo*)elements->At(i);
    B2INFO("Version = " << myelement->getAerogelVersion() << ", SN = " << myelement->getAerogelSerial() << ", n = " <<
           myelement->getAerogelRefractiveIndex() << ", trl = " << myelement->getAerogelTransmissionLength() << ", thickness = " <<
           myelement->getAerogelThickness());
  }

  /*

    // Extract data from the last added payload
    DBArray<ARICHAerogelInfo> elements("ARICHAerogelInfo");
    elements.getEntries();

    // Print aerogel info

    for (const auto& element : elements) {
      B2INFO("Version = " << element.getAerogelVersion() << ", serial = " << element.getAerogelSerial() <<
             ", id = " << element.getAerogelID() << ", n = " << element.getAerogelRefractiveIndex() << ", transmLength = " <<
             element.getAerogelTransmissionLength() << ", thickness = " << element.getAerogelThickness())
    }

  */
}


void ARICHDatabaseImporter::importHapdQA()
{
  // define data array
  TClonesArray hapdQAConstants("Belle2::ARICHHapdQA");
  int hapd = 0;

  // loop over root riles
  for (const string& inputFile : m_inputFilesHapdQA) {
    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    int size = inputFile.length();
    string hapdSerial = inputFile.substr(size - 16, 6);

    vector<TGraph*> leakCurrent;
    TH2F* hitData2D = 0;
    vector<TGraph*> noise;
    vector<TH1S*> hitCount;
    TTimeStamp arrivalDate;
    int arrival;
    TTree* tree = 0;

    // extract data
    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {

      string strime = key->GetName();

      if (strime.compare(0, 8, "gcurrent") == 0) {
        TGraph* graphcurrent = (TGraph*)f->Get(strime.c_str());
        leakCurrent.push_back(graphcurrent);
      }

      else if (strime.compare(0, 7, "h2dscan") == 0) {
        hitData2D = (TH2F*)f->Get(strime.c_str());
        hitData2D->SetDirectory(0);
      }

      else if (strime.compare(0, 9, "gnoise_ch") == 0) {
        TGraph* graphnoise = (TGraph*)f->Get(strime.c_str());
        noise.push_back(graphnoise);
      }

      else if (strime.compare(0, 7, "hchscan") == 0) {
        TH1F* hhist3 = (TH1F*)f->Get(strime.c_str());
        hhist3->SetDirectory(0);

        // conversion TH1F -> TH1S

        const char* hhist3_ime = hhist3->GetName();
        const char* hhist3_naslov = hhist3->GetTitle();
        int hhist3_nbins = hhist3->GetSize();
        TH1S* hhist3short = new TH1S(hhist3_ime, hhist3_naslov, hhist3_nbins - 2, 0, 1);
        for (int bin = 0; bin < hhist3_nbins; bin++) {
          hhist3short->SetBinContent(bin, hhist3->GetBinContent(bin));
        }
        hhist3short->SetDirectory(0);
        hitCount.push_back(hhist3short);
      }

      else if (strime.compare(0, 4, "tree") == 0) {
        tree = (TTree*)f->Get(strime.c_str());
        tree->SetBranchAddress("arrival", &arrival);
        tree->GetEntry(0);
        arrivalDate = TTimeStamp(arrival, 0);
      }

      else { B2INFO("Key name does not match any of the following: gcurrent, 2dscan, gnoise, hchscan, tree_ts! - serial number: " << hapdSerial << "; key name = " << strime.c_str()); }
    }

    // save data as an element of the array
    new(hapdQAConstants[hapd]) ARICHHapdQA(hapdSerial, arrivalDate, leakCurrent, hitData2D, noise, hitCount);
    hapd++;
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData("ARICHHapdQA", &hapdQAConstants, iov);
}


void ARICHDatabaseImporter::exportHapdQA()
{
  DBArray<ARICHHapdQA> elements("ARICHHapdQA");
  elements.getEntries();

  // Print serial numbers of HAPDs
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("Serial number = " << element.getHapdSerialNumber() << "; arrival date = " << element.getHapdArrivalDate());
    el++;
  }
}

void ARICHDatabaseImporter::importAsicInfo()
{
  GearDir content = GearDir("/ArichData/AllData/asicList");
  // define data array
  TClonesArray asicConstants("Belle2::ARICHAsicInfo");
  int asic = 0;

  for (const auto& asiclist : content.getNodes("asicinfo")) {
    string asicSerial = asiclist.getString("serial");
    string comment = asiclist.getString("comment");
    string numCH = asiclist.getString("num");

    // get time of measurement
    TTimeStamp timeFinishGain = ARICHDatabaseImporter::getAsicDate(asicSerial, "gain");
    TTimeStamp timeFinishOffset = ARICHDatabaseImporter::getAsicDate(asicSerial, "offset");

    // get lists of bad channels
    vector<int> nosignalCHs = ARICHDatabaseImporter::channelsList(asiclist.getString("nosignal"));
    vector<int> badconnCHs = ARICHDatabaseImporter::channelsList(asiclist.getString("badconn"));
    vector<int> badoffsetCHs = ARICHDatabaseImporter::channelsList(asiclist.getString("badoffset"));
    vector<int> badlinCHs = ARICHDatabaseImporter::channelsList(asiclist.getString("badlin"));

    int num = 0;
    if (numCH.find("many") != string::npos) {num = 5000; }
    else if (numCH.find("all") != string::npos) {num = 10000; }
    else {num = atoi(numCH.c_str()); }


    // save data as an element of the array
    new(asicConstants[asic])  ARICHAsicInfo();
    auto* asicConst = static_cast<ARICHAsicInfo*>(asicConstants[asic]);

    asicConst->setAsicID(asicSerial);
    asicConst->setTimeFinishGain(timeFinishGain);
    asicConst->setTimeFinishOffset(timeFinishOffset);
    asicConst->setDeadChannels(nosignalCHs);
    asicConst->setBadConnChannels(badconnCHs);
    asicConst->setBadOffsetChannels(badoffsetCHs);
    asicConst->setBadLinChannels(badlinCHs);
    asicConst->setNumOfProblematicChannels(num);
    asicConst->setComment(comment);

    asic++;
  }

  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("ARICHAsicInfo", &asicConstants, iov);
}

void ARICHDatabaseImporter::importAsicInfoRoot()
{
  TFile f1("asicInfoHistograms.root", "recreate");
  TH3F* gain0 = 0;
  TH3F* gain1 = 0;
  TH3F* gain2 = 0;
  TH3F* gain3 = 0;
  TH3F* offsetF = 0;
  TH3F* offsetR = 0;
  string asicSerial = "";

  // define tree
  TTree* tree = new TTree("asicInfo", "asic info data");

  tree->Branch("asicSerial", (void*)asicSerial.c_str(), "string/C", 1024);
  tree->Branch("gain0", "TH3F", &gain0);
  tree->Branch("gain1", "TH3F", &gain1);
  tree->Branch("gain2", "TH3F", &gain2);
  tree->Branch("gain3", "TH3F", &gain3);
  tree->Branch("offsetF", "TH3F", &offsetF);
  tree->Branch("offsetR", "TH3F", &offsetR);

  GearDir content = GearDir("/ArichData/AllData/asicList");

  // loop over root files
  for (const string& inputFile : m_inputFilesAsicRoot) {

    string inputFileNew = (string) inputFile;
    string asicName = inputFileNew.substr(inputFileNew.find("/asicData") + 17);
    size_t findRoot = asicName.find(".root");
    if (findRoot != string::npos) {
      // extract the data from files
      TFile* f = TFile::Open(inputFile.c_str(), "READ", "", 0);
      TIter next(f->GetListOfKeys());
      TKey* key;

      // fill vectors with histograms with different gain and offset settings
      while ((key = (TKey*)next())) {
        string strname = key->GetName();
        if (strname.find("_g0") != string::npos) gain0 = (TH3F*)f->Get(strname.c_str());
        else if (strname.find("_g1") != string::npos) gain1 = (TH3F*)f->Get(strname.c_str());
        else if (strname.find("_g2") != string::npos) gain2 = (TH3F*)f->Get(strname.c_str());
        else if (strname.find("_g3") != string::npos) gain3 = (TH3F*)f->Get(strname.c_str());
        else if (strname.find("_f") != string::npos) offsetF = (TH3F*)f->Get(strname.c_str());
        else if (strname.find("_c") != string::npos) offsetR = (TH3F*)f->Get(strname.c_str());
        else  B2INFO("Key name does not match any of the following: gain, offset!");
      }

      tree->Fill();
      f->Close();
    }
  }

  f1.cd();
  tree->Write();
  f1.Close();

//   define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().addPayload("ARICHAsicInfoRoot", "asicInfoHistograms.root", iov);
}

TTimeStamp ARICHDatabaseImporter::getAsicDate(const std::string& asicSerial, const std::string& type)
{
  TTimeStamp timeFinish;
  string line;
  size_t findText = 100;

  // extract measurement date for serial number
  for (const string& inputFile : m_inputFilesAsicTxt) {
    string inputFileNew = (string) inputFile;
    if (type == "gain") { findText = inputFileNew.find("dateGain.txt"); }
    else if (type == "offset") { findText = inputFileNew.find("dateOffset.txt"); }
    else {B2INFO("Check type of measurement!"); }
    if (findText != string::npos) {
      std::ifstream ifs(inputFile);
      if (ifs.is_open()) {
        while (ifs.good()) {
          std::getline(ifs, line);
          string asicSerial2 = asicSerial + ":";
          size_t findSerial = line.find(asicSerial2);
          if (findSerial != string::npos) {timeFinish = ARICHDatabaseImporter::timedate(line);}
        }
      }
      ifs.clear();
      ifs.close();
    }
  }
  return timeFinish;
}

TTimeStamp ARICHDatabaseImporter::timedate(std::string enddate)
{
  // convert string into TTimeStamp
  string dateEnd = enddate.substr(enddate.find(":") + 2);
  string yearStr = dateEnd.substr(0, 4);
  string monthStr = dateEnd.substr(4, 2);
  string dayStr = dateEnd.substr(6, 2);
  string hourStr = dateEnd.substr(8, 2);
  string minStr = dateEnd.substr(10, 2);
  int year = atoi(yearStr.c_str());
  int month = atoi(monthStr.c_str());
  int day = atoi(dayStr.c_str());
  int hour = atoi(hourStr.c_str());
  int min = atoi(minStr.c_str());
  TTimeStamp datum = TTimeStamp(year, month, day, hour, min, 0, 0, kTRUE, -9 * 60 * 60);
  return datum;
}

vector<int> ARICHDatabaseImporter::channelsList(std::string badCH)
{

  vector<int> CHs;

  // parse string to get numbers of all bad channels
  if (!badCH.empty()) {
    if (badCH.find(",") != string::npos) {
      while (badCH.find(",") != string::npos) {
        string CH = badCH.substr(0, badCH.find(","));
        int badchannel = atoi(CH.c_str());
        CHs.push_back(badchannel);
        badCH = badCH.substr(badCH.find(",") + 1);
      }
      int badchannel = atoi(badCH.c_str());
      CHs.push_back(badchannel);
    }
  }
  // store 5000 if there are many bad channels
  else if (badCH.find("many") != string::npos) {
    CHs.push_back(5000);
  }
  // store 10000 if all channels are bad
  else if (badCH.find("all") != string::npos) {
    CHs.push_back(10000);
  } else {
    int badchannel = atoi(badCH.c_str());
    CHs.push_back(badchannel);
  }
  return CHs;
}


void ARICHDatabaseImporter::exportAsicInfo()
{

  DBArray<ARICHAsicInfo> elements("ARICHAsicInfo");
  elements.getEntries();

  // Print serial numbers of ASICs
  for (const auto& element : elements) {
    B2INFO("asic SN: " << element.getAsicID());
  }
}


void ARICHDatabaseImporter::importFebTest()
{

  // define data array
  TClonesArray febConstants("Belle2::ARICHFebTest");

  int feb = 0;

  GearDir content = GearDir("/ArichData/AllData/arich");
  GearDir content1 = GearDir("/ArichData/AllData/dnamap");
  GearDir content2 = GearDir("/ArichData/AllData/FEBData/Content");
  GearDir content2HV = GearDir("/ArichData/AllData/FEBDataHV/Content");

  for (const auto& runserial : content.getNodes("run")) {
    int serial = runserial.getInt("sn");
    // save data as an element of the array
    new(febConstants[feb]) ARICHFebTest();
    auto* febConst = static_cast<ARICHFebTest*>(febConstants[feb]);
    febConst->setFebSerial(serial);

    string runLV = runserial.getString("lv");
    string runHV = runserial.getString("hv");
    string runSCAN = runserial.getString("scan");
    string comment = runserial.getString("comment");

    febConst->setRunLV(runLV);
    febConst->setRunHV(runHV);
    febConst->setRunSlowC(runSCAN);
    febConst->setComment(comment);

    int l = 0;
    string dna;

    for (const auto& febmap : content1.getNodes("febps")) {
      string somedna = febmap.getString("@dna");
      int sn = febmap.getInt("sn");
      if ((l == 0) && (sn == serial)) {
        dna = somedna;
        febConst->setFebDna(dna);
        //febConst->setDeadChannels(ARICHDatabaseImporter::getDeadChFEB(dna));
        l++;
      }
    }

    // slow control data
    if (!runSCAN.empty()) {
      int scanRun;
      if (stoi(runSCAN.c_str()) < 23) {scanRun = stoi(runSCAN.c_str());}
      else {scanRun = stoi(runSCAN.c_str()) - 77;}
      for (const auto& testFEB : content2.getNodes("febtest[" + std::to_string(scanRun) + "]")) {
        string timeSlowC = testFEB.getString("time");
        int i = 0;
        int positionOld = 0;

        for (const auto& testFEBslowctest : testFEB.getNodes("slowc")) {
          int position = testFEBslowctest.getInt("id");
          if (position == positionOld) {i++; }
          else {i = 1; }
          if (i == 10) {
            string dnaNew = testFEBslowctest.getString("dna");
            if (dnaNew == dna) {
              float tmon0 = (float) testFEBslowctest.getDouble("TMON0");
              float tmon1 = (float) testFEBslowctest.getDouble("TMON1");
              float vdd = (float) testFEBslowctest.getDouble("VDD");
              float v2p = (float) testFEBslowctest.getDouble("V2P");
              float v2n = (float) testFEBslowctest.getDouble("V2N");
              float vss = (float) testFEBslowctest.getDouble("VSS");
              float vth1 = (float) testFEBslowctest.getDouble("VTH1");
              float vth2 = (float) testFEBslowctest.getDouble("VTH2");
              float vcc12 = (float) testFEBslowctest.getDouble("VCC12");
              float vcc15 = (float) testFEBslowctest.getDouble("VCC15");
              float vcc25 = (float) testFEBslowctest.getDouble("VCC25");
              float v38p = (float) testFEBslowctest.getDouble("V38P");

              febConst->setTemperature0(tmon0);
              febConst->setTemperature1(tmon1);
              febConst->setVdd(vdd);
              febConst->setV2p(v2p);
              febConst->setV2n(v2n);
              febConst->setVss(vss);
              febConst->setVth1(vth1);
              febConst->setVth2(vth2);
              febConst->setVcc12(vcc12);
              febConst->setVcc15(vcc15);
              febConst->setVcc25(vcc25);
              febConst->setV38p(v38p);
              febConst->setTimeSlowC(ARICHDatabaseImporter::timedate2(timeSlowC));

            }
          }
          positionOld = position;
        }
      }

      // slopes (from offset settings measurements)
      pair<vector<float>, vector<float>> slopes = ARICHDatabaseImporter::getSlopes(serial, runSCAN);

      febConst->setSlopesFine(slopes.first); // std::vector<float>
      febConst->setSlopesRough(slopes.second); // std::vector<float>

      // set FWHM values&sigmas)
      vector<pair<float, float>> fwhm = ARICHDatabaseImporter::getFwhm(serial, runSCAN);
      febConst->setFWHM(fwhm);
    }


    // high voltage test data
    if (!runHV.empty()) {
      int hvRun;
      if (stoi(runHV.c_str()) < 43) {hvRun = stoi(runHV.c_str());}
      else {hvRun = stoi(runHV.c_str()) - 57;}
      tuple<string, float> HVtest = ARICHDatabaseImporter::getFebHVtestData(serial, hvRun);
      febConst->setTimeHV(ARICHDatabaseImporter::timedate2(get<0>(HVtest)));
      febConst->setCurrentV99p(get<1>(HVtest));
    }


    // low voltage test data
    if (!runLV.empty()) {
      int lvRun;
      if (stoi(runLV.c_str()) < 43) {lvRun = stoi(runLV.c_str());}
      else {lvRun = stoi(runLV.c_str()) - 57;}
      tuple<string, float, float, float> LVtest = ARICHDatabaseImporter::getFebLVtestData(serial, lvRun);
      febConst->setTimeLV(ARICHDatabaseImporter::timedate2(get<0>(LVtest)));
      febConst->setCurrentV20p(get<1>(LVtest));
      febConst->setCurrentV21n(get<2>(LVtest));
      febConst->setCurrentV38p(get<3>(LVtest));
    }

    feb++;

  }

//   define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("ARICHFebTest", &febConstants, iov);
}



void ARICHDatabaseImporter::importFebTestRoot()
{
  TFile f("febTestHistograms.root", "recreate");
  TH3F* offsetRough = 0;
  TH3F* offsetFine = 0;
  TH2F* testPulse = 0;
  int serial;

  // define tree
  TTree* tree = new TTree("febTest", "feb test data");

  tree->Branch("serial", &serial, "sn/I");
  tree->Branch("offsetRough", "TH3F", &offsetRough);
  tree->Branch("offsetFine", "TH3F", &offsetFine);
  tree->Branch("testPulse", "TH2F", &testPulse);

  int febposition = -1;

  GearDir content = GearDir("/ArichData/AllData/arich");
  GearDir content1 = GearDir("/ArichData/AllData/dnamap");
  GearDir content2 = GearDir("/ArichData/AllData/FEBData/Content");
  GearDir content2HV = GearDir("/ArichData/AllData/FEBDataHV/Content");

  for (const auto& runserial : content.getNodes("run")) {
    serial = runserial.getInt("sn");
    string runSCAN = runserial.getString("scan");
    int l = 0;
    string dna;

    for (const auto& febmap : content1.getNodes("febps")) {
      string somedna = febmap.getString("@dna");
      int sn = febmap.getInt("sn");
      if ((l == 0) && (sn == serial)) {
        dna = somedna;
        l++;
      }
    }
    // slow control data
    if (!runSCAN.empty()) {
      int scanRun;
      if (stoi(runSCAN.c_str()) < 23) {scanRun = stoi(runSCAN.c_str());}
      else {scanRun = stoi(runSCAN.c_str()) - 77;}
      for (const auto& testFEB : content2.getNodes("febtest[" + std::to_string(scanRun) + "]")) {
        int i = 0;
        int positionOld = 0;

        for (const auto& testFEBslowctest : testFEB.getNodes("slowc")) {
          int position = testFEBslowctest.getInt("id");
          if (position == positionOld) {i++; }
          else {i = 1; }
          if (i == 10) {
            string dnaNew = testFEBslowctest.getString("dna");
            if (dnaNew == dna) febposition = position;
          }
          positionOld = position;
        }
      }
      vector<TH3F*> histograms = getFebTestHistograms(dna, runSCAN, febposition);
      offsetRough = histograms[0];
      offsetFine = histograms[1];

      testPulse = ARICHDatabaseImporter::getFebTestPulse(dna, runSCAN, febposition); // TH2F*

      testPulse->SetDirectory(0);
      offsetRough->SetDirectory(0);
      offsetFine->SetDirectory(0);

      tree->Fill();

      delete testPulse;
      delete offsetRough;
      delete offsetFine;
    }
  }

  f.cd();
  tree->Write();
  f.Close();


//   define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().addPayload("ARICHFebTestRoot", "febTestHistograms.root", iov);

}

std::tuple<std::string, float, float, float> ARICHDatabaseImporter::getFebLVtestData(int serial, int lvRun)
{
  GearDir content2LV = GearDir("/ArichData/AllData/FEBDataLV/Content");
  std::tuple<std::string, float, float, float> LVtest;
  float currentV20p = 0.0, currentV21n = 0.0, currentV38p = 0.0;

  for (const auto& testFEBlv : content2LV.getNodes("febtest[" + std::to_string(lvRun) + "]")) {
    string timeLV = testFEBlv.getString("time");
    for (const auto& testFEBlv_sn : testFEBlv.getNodes("lvtest/feb[sn='" + to_string(serial) + "']/febps/n[@id='14']")) {
      for (const auto& testFEBlv_pw : testFEBlv_sn.getNodes("pw18")) {
        if (testFEBlv_pw.getInt("@id") == 0) {  currentV20p = (float) testFEBlv_pw.getDouble("I");  }
        if (testFEBlv_pw.getInt("@id") == 1) {  currentV21n = (float) testFEBlv_pw.getDouble("I");  }
        if (testFEBlv_pw.getInt("@id") == 2) {  currentV38p = (float) testFEBlv_pw.getDouble("I");  }
        LVtest = make_tuple(timeLV, currentV20p, currentV21n, currentV38p);
      }
    }
  }
  return LVtest;
}


std::tuple<std::string, float> ARICHDatabaseImporter::getFebHVtestData(int serial, int hvRun)
{
  GearDir content2HV = GearDir("/ArichData/AllData/FEBDataHV/Content");
  std::tuple<std::string, float> HVtest;
  for (const auto& testFEBhv : content2HV.getNodes("febtest[" + std::to_string(hvRun) + "]")) {
    string timeHV = testFEBhv.getString("time");
    for (const auto& testFEBhv_sn : testFEBhv.getNodes("hvtest/feb[sn='" + to_string(serial) + "']/febhv/n[@id='9']")) {
      float currentV99p = (float) testFEBhv_sn.getDouble("n1470/I");
      HVtest = std::make_tuple(timeHV, currentV99p);
    }
  }
  return HVtest;
}

std::vector<int> ARICHDatabaseImporter::getDeadChFEB(const std::string& dna)
{
  vector<int> listCHs;
  string line;
  ifstream fileFEB("febTest/FEBdeadChannels.txt");
  if (fileFEB.is_open()) {
    while (getline(fileFEB, line)) {
      string ch2 = line.substr(line.find(",") + 1);
      string dna2 = line.erase(line.find(",") - 1);
      if (dna2 == dna) { listCHs.push_back(atoi(ch2.c_str())); }
    }
  } else { B2INFO("No file FEBdeadChannels.txt"); }
  fileFEB.close();

  return listCHs;
}

TTimeStamp ARICHDatabaseImporter::timedate2(std::string time)
{
  // convert string into TTimeStamp
  string dateMeas = time.substr(4);
  string yearStr = dateMeas.substr(16, 4);
  string monthStr = dateMeas.substr(0, 3);
  string dayStr = dateMeas.substr(4, 2);
  string hourStr = dateMeas.substr(7, 2);
  string minStr = dateMeas.substr(10, 2);
  string secStr = dateMeas.substr(13, 2);

  int year = atoi(yearStr.c_str());
  int day = atoi(dayStr.c_str());
  int hour = atoi(hourStr.c_str());
  int min = atoi(minStr.c_str());
  int sec = atoi(secStr.c_str());
  map<string, int> months  {
    { "Jan", 1 },  { "Feb", 2 },  { "Mar", 3 },  { "Apr", 4 },  { "May", 5 },  { "Jun", 6 },  { "Jul", 7 },  { "Aug", 8 },  { "Sep", 9 },  { "Oct", 10 },  { "Nov", 11 },  { "Dec", 12 }
  };
  int month = months.find(monthStr)->second;

  TTimeStamp datum = TTimeStamp(year, month, day, hour, min, sec, 0, kTRUE, 0);
  return datum;
}

std::pair<std::vector<float>, std::vector<float>> ARICHDatabaseImporter::getSlopes(int serialNum, const std::string& run)
{
  GearDir contentData = GearDir("/ArichData/AllData/SlopesFebTest/Content");
  pair<vector<float>, vector<float>> slopesFebTest;
  vector<float> slopesCoarse, slopesFine;

  for (const auto& contentSlopes : contentData.getNodes("febtest")) {
    if ((contentSlopes.getInt("@id") == serialNum) && (contentSlopes.getString("@run") == run)) {
      for (int i = 0; i < 144; i++) {
        for (const auto& contSlopes : contentSlopes.getNodes("slope/ch[@id='" + to_string(i) + "']")) {
          float slopeCoarse = ((float) contSlopes.getDouble("coarse")) * 2.5 / 1024; // slope in V/step
          float slopeFine = ((float) contSlopes.getDouble("fine")) * 2.5 / 1024;   // slope in V/step
          slopesCoarse.push_back(slopeCoarse);
          slopesFine.push_back(slopeFine);
        }
      }
    }
    slopesFebTest = std::make_pair(slopesFine, slopesCoarse);
  }
  return slopesFebTest;
}

std::vector<std::pair<float, float>> ARICHDatabaseImporter::getFwhm(int serialNum, const std::string& run)
{
  GearDir contentData = GearDir("/ArichData/AllData/SlopesFebTest/Content");
  vector<pair<float, float>> fwhm;

  for (const auto& contentSlopes : contentData.getNodes("febtest")) {
    if ((contentSlopes.getInt("@id") == serialNum) && (contentSlopes.getString("@run") == run)) {
      for (const auto& contSlopes : contentSlopes.getNodes("data/ch")) {
        float fwhmVal = (float) contSlopes.getDouble("fwhm");
        float fwhmSig = (float) contSlopes.getDouble("sigma");
        pair<float, float> fwhmPair = std::make_pair(fwhmVal, fwhmSig);
        fwhm.push_back(fwhmPair);
      }
    }
  }
  return fwhm;
}

std::vector<TH3F*> ARICHDatabaseImporter::getFebTestHistograms(const std::string& dna, const std::string& run, int febposition)
{
  vector<TH3F*> histogrami;

  for (const string& inputFile : m_inputFilesFebTest) {

    if (inputFile.find(run) != string::npos) {
      TFile* f = TFile::Open(inputFile.c_str(), "READ");
      int iMIN = 0, iMAX = 0, delta, stepsNum = 16, i = 0, point2 = 24;
      string option;
      float stepsMax = 15.5;

      // check how many steps were measured
      TIter next(f->GetListOfKeys());
      TKey* key;
      while ((key = (TKey*)next())) {
        string strime = key->GetName();
        if (strime.compare(0, 5, "h2d_0") == 0) i++;
      }

      // fill histogram for coarse/fine settings
      for (int k = 0; k < 2; k++) {
        if (i < 32) {
          stepsNum = 15;
          stepsMax = 14.5;
          point2 = 23;
          if (k == 0) { option = "coarse"; iMIN = 2; iMAX = 16; delta = 2;}
          if (k == 1) { option = "fine"; iMIN = 17; iMAX = 31; delta = 17; }
        } else {
          if (k == 0) { option = "coarse"; iMIN = 2; iMAX = 17; delta = 2; }
          if (k == 1) { option = "fine"; iMIN = 18; iMAX = 33; delta = 18; }
        }

        TH3F* histogram = new TH3F((option + " " + dna).c_str(), (option + " " + dna).c_str(), 144, -0.5, 143.5, 250, 299, 799, stepsNum,
                                   -0.5, stepsMax);

        for (int j = iMIN; j < iMAX + 1; j++)  {
          TH2F* hist2d = (TH2F*)f->Get(("h2d_0;" + std::to_string(j)).c_str());
          hist2d->SetDirectory(0);

          int binZ;
          if ((j - delta) < 8) {binZ = 8 - (j - delta);}
          if ((j - delta) > 7) {binZ = point2 - (j - delta);}
          // conversion TH2F -> TH3F
          for (int binX = 144 * febposition + 1; binX < 144 * (febposition + 1) + 1; binX++) {
            for (int binY = 1; binY < 251; binY++) {
              histogram->SetBinContent(binX - 144 * febposition, binY, binZ, hist2d->GetBinContent(binX, binY));
            }
          }
        }
        histogram->SetDirectory(0);
        histogrami.push_back(histogram);
        for (int kanal = 1; kanal < 145; kanal ++) {
          for (int offset = 1; offset < stepsNum + 1; offset++) {
            TH1D* h1 = histogram->ProjectionY("A", kanal, kanal, offset, offset);
            h1->SetName((dna + " kanal: " + to_string(kanal) + ", offset: " + to_string(offset)).c_str());
          }
        }
      }
      f->Close();
    }
  }

  return histogrami;
}

TH2F* ARICHDatabaseImporter::getFebTestPulse(const std::string& dna, const std::string& run, int febposition)
{
  TH2F* testPulse = new TH2F(("test pulse " + dna).c_str(), ("test pulse " + dna).c_str(), 144, -0.5, 143.5, 250, 299, 799);

  for (const string& inputFile : m_inputFilesFebTest) {
    if (inputFile.find(run) != string::npos) {
      TFile* f = TFile::Open(inputFile.c_str(), "READ");
      TH2F* pulseTest = (TH2F*)f->Get("h2d_0;1");
      pulseTest->SetDirectory(0);
      for (int binX = 144 * febposition + 1; binX < 144 * (febposition + 1) + 1; binX++) {
        for (int binY = 1; binY < 251; binY++) {
          testPulse->SetBinContent(binX - 144 * febposition, binY, pulseTest->GetBinContent(binX, binY));
        }
      }
      f->Close();
    }
  }
  return testPulse;
}



void ARICHDatabaseImporter::exportFebTest()
{

  DBArray<ARICHFebTest> elements("ARICHFebTest");
  elements.getEntries();

  // Print serial numbers of FEBs
  for (const auto& element : elements) {
    B2INFO("Serial = " << element.getFebSerial() << "; dna = " << element.getFebDna() << "; slope R (ch143) = " <<
           element.getSlopeRough(143) << "; slope F (ch143) = " << element.getSlopeFine(143) << "; comment = " << element.getComment());
  }

}

void ARICHDatabaseImporter::importHapdChipInfo()
{
  int chip_i = 0;
  GearDir content = GearDir("/ArichData/AllData/hapdData/Content");

  // define data array
  TClonesArray chipConstants("Belle2::ARICHHapdChipInfo");

  // extract chip info, such as bias voltage, lists of dead and bad channels etc.
  for (const auto& hapdInfo : content.getNodes("hapd")) {
    // extract information about HAPD
    string sn = hapdInfo.getString("serial");

    // define objects for chip info
    string chip[4];
    int channel_label_aval[4], bias[4], gain[4];
    TGraph** bombardmentGain, **avalancheGain;
    bombardmentGain = new TGraph *[4];
    avalancheGain = new TGraph *[4];
    TH2F** bias2DV, **bias2DI;
    bias2DV = new TH2F *[4];
    bias2DI = new TH2F *[4];
    vector<int> badlist[4], cutlist[4];
    vector<TGraph*> bombCurrents[4], avalCurrents[4];
    int chip_ABCD = 0;

    // extract information for each chip
    for (const auto& chipInfo : hapdInfo.getNodes("chipinfo")) {
      chip[chip_ABCD] = chipInfo.getString("chip");
      bias[chip_ABCD] = chipInfo.getInt("bias");
      string badL = chipInfo.getString("deadlist");
      string cutL = chipInfo.getString("cutlist");
      if (badL.find("ch") != string::npos) { string badLsub = badL.substr(3); badlist[chip_ABCD] = ARICHDatabaseImporter::channelsListHapd(badLsub.c_str(), 0); }
      if (cutL.find("ch") != string::npos) {  string cutLsub = cutL.substr(3); cutlist[chip_ABCD] = ARICHDatabaseImporter::channelsListHapd(cutLsub.c_str(), 0); }
      string gain_str = chipInfo.getString("gain");
      gain[chip_ABCD] = atoi(gain_str.c_str());
      chip_ABCD++;
    }

    // prepare TGraphs for bombardment gain and current
    const int n3 = 30;
    int i3 = 0;
    //int channel_label_bomb;
    float hv_bomb[n3], gain_bomb[n3], current1_bomb[n3], current2_bomb[n3], current3_bomb[n3];
    chip_ABCD = 0;
    for (const auto& BG : hapdInfo.getNodes("bombardmentgain/ch")) {
      string value = BG.getString("value");
      string chip_label = value.erase(1);
      for (const auto& BG2 : BG.getNodes("point")) {
        hv_bomb[i3] = (float) BG2.getDouble("hv");
        gain_bomb[i3] = (float) BG2.getDouble("gain");
        current1_bomb[i3] = (float) BG2.getDouble("current1");
        current2_bomb[i3] = (float) BG2.getDouble("current2");
        current3_bomb[i3] = (float) BG2.getDouble("current3");
        i3++;
      }

      bombardmentGain[chip_ABCD] = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "gain", chip_label, i3, hv_bomb, gain_bomb);
      TGraph* bombardmentCurrent1 = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "current1", chip_label, i3, hv_bomb,
                                    current1_bomb);
      TGraph* bombardmentCurrent2 = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "current2", chip_label, i3, hv_bomb,
                                    current2_bomb);
      TGraph* bombardmentCurrent3 = ARICHDatabaseImporter::getGraphGainCurrent("Bombardment", "current3", chip_label, i3, hv_bomb,
                                    current3_bomb);
      bombCurrents[chip_ABCD].push_back(bombardmentCurrent1);
      bombCurrents[chip_ABCD].push_back(bombardmentCurrent2);
      bombCurrents[chip_ABCD].push_back(bombardmentCurrent3);
      chip_ABCD++;
      i3 = 0;
    }

    // prepare TGraphs for avalanche gain and current
    const int n4 = 30;
    int i4 = 0;
    float hv_aval[n4], gain_aval[n4], current1_aval[n4], current2_aval[n4], current3_aval[n4];
    chip_ABCD = 0;
    for (const auto& BG : hapdInfo.getNodes("avalanchegain/ch")) {
      string value = BG.getString("value");
      string value_1 = value;
      string chip_label = value.erase(1);
      string value_2 = value_1.substr(2);
      channel_label_aval[chip_ABCD] = atoi(value_2.c_str());
      for (const auto& BG2 : BG.getNodes("point")) {
        hv_aval[i4] = (float) BG2.getDouble("biasv");
        gain_aval[i4] = (float) BG2.getDouble("gain");
        current1_aval[i4] = (float) BG2.getDouble("current1");
        current2_aval[i4] = (float) BG2.getDouble("current2");
        current3_aval[i4] = (float) BG2.getDouble("current3");
        i4++;
      }

      avalancheGain[chip_ABCD] = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "gain", chip_label, i4, hv_aval, gain_aval);
      TGraph* avalancheCurrent1 = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "current1", chip_label, i4, hv_aval,
                                  current1_aval);
      TGraph* avalancheCurrent2 = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "current2", chip_label, i4, hv_aval,
                                  current2_aval);
      TGraph* avalancheCurrent3 = ARICHDatabaseImporter::getGraphGainCurrent("Avalanche", "current3", chip_label, i4, hv_aval,
                                  current3_aval);
      avalCurrents[chip_ABCD].push_back(avalancheCurrent1);
      avalCurrents[chip_ABCD].push_back(avalancheCurrent2);
      avalCurrents[chip_ABCD].push_back(avalancheCurrent3);

      chip_ABCD++;
      i4 = 0;
    }


    chip_ABCD = 0;
    // prepare 2D histograms for bias voltage and current
    const int n5 = 150;
    int i5 = 0, chipnum[n5];
    float biasv[n5], biasi[n5];
    for (const auto& HI : hapdInfo.getNodes("bias2d/biasvalue")) {
      string chip_2d = HI.getString("@chip");
      chipnum[i5] = HI.getInt("@ch");
      biasv[i5] = (float) HI.getDouble("biasv");
      biasi[i5] = (float) HI.getDouble("biasi");
      if (chipnum[i5] == 36) {
        bias2DV[chip_ABCD] = ARICHDatabaseImporter::getBiasGraph(chip_2d, "voltage", chipnum, biasv);
        bias2DI[chip_ABCD] = ARICHDatabaseImporter::getBiasGraph(chip_2d, "current", chipnum, biasi);
        i5 = -1;
        chip_ABCD++;
      }
      i5++;
    }

    // prepare ARICHHapdChipInfo class for each chip
    for (unsigned int l = 0; l < 4; l++)  {
      new(chipConstants[4 * chip_i + l]) ARICHHapdChipInfo();
      auto* chipConst = static_cast<ARICHHapdChipInfo*>(chipConstants[4 * chip_i + l]);

      chipConst->setHapdSerial(sn);
      chipConst->setChipLabel(chip[l]);
      chipConst->setBiasVoltage(bias[l]);
      chipConst->setGain(gain[l]);
      chipConst->setBadChannel(badlist[l]);
      chipConst->setCutChannel(cutlist[l]);
      chipConst->setBombardmentGain(bombardmentGain[l]);
      chipConst->setBombardmentCurrent(bombCurrents[l]);
      chipConst->setAvalancheGain(avalancheGain[l]);
      chipConst->setAvalancheCurrent(avalCurrents[l]);
      chipConst->setChannelNumber(channel_label_aval[l]);
      chipConst->setBiasVoltage2D(bias2DV[l]);
      chipConst->setBiasCurrent2D(bias2DI[l]);
    }

    chip_i++;

    delete[] bombardmentGain;
    delete[] avalancheGain;
    delete[] bias2DV;
    delete[] bias2DI;
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("ARICHHapdChipInfo", &chipConstants, iov);
}

void ARICHDatabaseImporter::exportHapdChipInfo()
{
  DBArray<ARICHHapdChipInfo> elements("ARICHHapdChipInfo");
  elements.getEntries();

  for (const auto& element : elements) {
    B2INFO("Serial = " << element.getHapdSerial() << ", chip = " << element.getChipLabel() << ", bias= " << element.getBiasVoltage());
  }
}



void ARICHDatabaseImporter::importHapdInfo()
{
  int hapd_i = 0;
  GearDir content = GearDir("/ArichData/AllData/hapdData/Content");

  // define data array
  TClonesArray hapdConstants("Belle2::ARICHHapdInfo");

  // extract chip info, such as bias voltage, lists of dead and bad channels etc.
  for (const auto& hapdInfo : content.getNodes("hapd")) {
    // define element of TClonesArray
    new(hapdConstants[hapd_i]) ARICHHapdInfo();
    auto* hapdConst = static_cast<ARICHHapdInfo*>(hapdConstants[hapd_i]);

    // extract information about HAPD
    string serial = hapdInfo.getString("serial");
    float qe400 = (float) hapdInfo.getDouble("qe400");
    float hv = 1000 * (float) hapdInfo.getDouble("hv");
    float current = (float) hapdInfo.getDouble("current");
    string gb = hapdInfo.getString("guardbias");
    int guardbias = atoi(gb.c_str());

    // prepare TGraph of quantum efficiency as function of lambda
    const int n1 = 70;
    float lambda[n1], qepoint[n1];
    int i1 = 0;
    for (const auto& QE : hapdInfo.getNodes("qe/qepoint")) {
      lambda[i1] = (float) QE.getInt("@lambda");
      qepoint[i1] = (float) QE.getDouble(".");
      i1++;
    }
    TGraph* qe = new TGraph(i1, lambda, qepoint);
    qe->SetName("qe");
    qe->SetTitle("qe");
    qe->GetXaxis()->SetTitle("lambda");
    qe->GetYaxis()->SetTitle("qe");

    // prepare TGraph of pulse height distribution
    const int n2 = 4100;
    int channel_adc[n2], pulse_adc[n2];
    int i2 = 0;
    for (const auto& ADC : hapdInfo.getNodes("adc/value")) {
      channel_adc[i2] = ADC.getInt("@ch");
      string str = ADC.getString(".");
      pulse_adc[i2] = atoi(str.c_str());
      i2++;
    }
    TGraph* adc = new TGraph(i2, channel_adc, pulse_adc);
    adc->SetName("adc");
    adc->SetTitle("Pulse Height Distribution");
    adc->GetXaxis()->SetTitle("channel");
    adc->GetYaxis()->SetTitle("pulse height");

    // save HAPD data to the element of TClonesArray
    hapdConst->setSerialNumber(serial);
    hapdConst->setQuantumEfficiency400(qe400);
    hapdConst->setHighVoltage(hv);
    hapdConst->setGuardBias(guardbias);
    hapdConst->setCurrent(current);
    hapdConst->setQuantumEfficiency(qe);
    hapdConst->setPulseHeightDistribution(adc);

    // export ARICHHapdChipInfo class for each chip from DB and
    // add it as an element of ARICHHapdInfo class
    DBArray<ARICHHapdChipInfo> elementsChip("ARICHHapdChipInfo");
    elementsChip.getEntries();
    for (const auto& element : elementsChip) {
      if (element.getHapdSerial() == serial) {
        ARICHHapdChipInfo& elementValue = const_cast<ARICHHapdChipInfo&>(element);
        if (element.getChipLabel() == "A") hapdConst->setHapdChipInfo(0, &elementValue);
        if (element.getChipLabel() == "B") hapdConst->setHapdChipInfo(1, &elementValue);
        if (element.getChipLabel() == "C") hapdConst->setHapdChipInfo(2, &elementValue);
        if (element.getChipLabel() == "D") hapdConst->setHapdChipInfo(3, &elementValue);
      }
    }

    hapd_i++;
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1);
  Database::Instance().storeData("ARICHHapdInfo", &hapdConstants, iov);
}


// get list of bad channels on HAPD
std::vector<int> ARICHDatabaseImporter::channelsListHapd(std::string chlist, int channelDelay)
{
  vector<int> CHs;
  // parse string to get numbers of all bad channels
  if (chlist.find(",") != string::npos) {
    while (chlist.find(",") != string::npos) {
      string CH = chlist.substr(0, chlist.find(","));
      int badChannel = atoi(CH.c_str()) + channelDelay;
      CHs.push_back(badChannel);
      chlist = chlist.substr(chlist.find(",") + 1);
    }
    int badChannel = atoi(chlist.c_str()) + channelDelay;
    CHs.push_back(badChannel);
  } else {
    int badChannel = atoi(chlist.c_str()) + channelDelay;
    CHs.push_back(badChannel);
  }
  return CHs;
}

// prepare TGraph for bombardment/avalanche gain/current
TGraph* ARICHDatabaseImporter::getGraphGainCurrent(const std::string& bomb_aval, const std::string& g_i,
                                                   const std::string& chip_label, int i, float* HV,
                                                   float* gain_current)
{
  TGraph* hapd_graph = new TGraph(i, HV, gain_current);
  string title = bomb_aval + " " + g_i + ", chip " + chip_label;
  hapd_graph->SetTitle(title.c_str());
  hapd_graph->GetXaxis()->SetTitle("high voltage");
  hapd_graph->GetYaxis()->SetTitle(g_i.c_str());
  return hapd_graph;
}

// use correct mapping of channels on HAPD
int ARICHDatabaseImporter::getChannelPosition(const std::string& XY, const std::string& chip_2d, int chipnum)
{
  int x = 100, y = 100;
  if (chip_2d == "A") {
    y = 12;
    x = 0 + chipnum;
    while (x > 6) {
      x = x - 6;
      y = y - 1;
    }
  } else if (chip_2d == "B") {
    x = 12;
    y = 13 - chipnum;
    while (y < 7) {
      y = y + 6;
      x = x - 1;
    }
  } else if (chip_2d == "C") {
    y = 1;
    x = 13 - chipnum;
    while (x < 7) {
      x = x + 6;
      y = y + 1;
    }
  } else if (chip_2d == "D") {
    x = 1;
    y = 0 + chipnum;
    while (y > 6) {
      y = y - 6;
      x = x + 1;
    }
  }

  if (XY == "x") { return x;}
  else if (XY == "y") { return y;}
  else {return 100;}
}

// prepare 2D histogram for bias voltage/current
TH2F* ARICHDatabaseImporter::getBiasGraph(const std::string& chip_2d, const std::string& voltage_current, int* chipnum,
                                          float* bias_v_i)
{
  string name = "bias " + voltage_current + ", chip " + chip_2d;
  TH2F* bias2d = new TH2F("bias2d", name.c_str(), 6, 0, 6, 6, 0, 6);
  for (int XYname = 0; XYname < 6; XYname++) {
    bias2d->GetXaxis()->SetBinLabel(XYname + 1, to_string(XYname).c_str());
    bias2d->GetYaxis()->SetBinLabel(XYname + 1, to_string(6 * XYname).c_str());
  }
  for (int XY = 0; XY < 36; XY++)  {
    int x = 1;
    int y = 0 + chipnum[XY];
    while (y > 6) {
      y = y - 6;
      x = x + 1;
    }
    bias2d->SetBinContent(x, y, bias_v_i[XY]);
  }
  bias2d->SetDirectory(0);
  return bias2d;
}

void ARICHDatabaseImporter::exportHapdInfo()
{

  DBArray<ARICHHapdInfo> elements("ARICHHapdInfo");
  elements.getEntries();

  for (const auto& element : elements) {
    B2INFO("Serial = " << element.getSerialNumber() << "; HV = " << element.getHighVoltage() << "; qe400 = " <<
           element.getQuantumEfficiency400());
    for (int n = 0; n < 4; n++)  {
      ARICHHapdChipInfo* newelement = element.getHapdChipInfo(n);
      B2INFO("biasV(chip" << n << ") = " << newelement->getBiasVoltage());
    }
    /*    TGraph* adc = element.getPulseHeightDistribution();
        TFile file("histogrami.root", "update");
        adc->Write();
        file.Close();
    */
  }
}


void ARICHDatabaseImporter::importHapdQE()
{
  // define data array
  TClonesArray hapdQEConstants("Belle2::ARICHHapdQE");
  int hapd = 0;

  // loop over root riles
  for (const string& inputFile : m_inputFilesHapdQE) {

    TFile* f = TFile::Open(inputFile.c_str(), "READ");

    int size = inputFile.length();
    string hapdSerial = inputFile.substr(size - 11, 6);
    TH2F* qe2D = 0;

    // extract data
    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {

      string strime = key->GetName();

      if (strime.compare(0, 11, "hqe2d_pixel") == 0) {
        qe2D = (TH2F*)f->Get(strime.c_str());
        qe2D->SetTitle("quantum efficiency");
        qe2D->SetName("QE");
        qe2D->SetDirectory(0);
      }

      else { B2INFO("Key name does not match 'hqe2d_pixel'!"); }
    }

    // save data as an element of the array
    new(hapdQEConstants[hapd]) ARICHHapdQE(hapdSerial, qe2D);
    hapd++;
    f->Close();
  }

  // define IOV and store data to the DB
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData("ARICHHapdQE", &hapdQEConstants, iov);
}


void ARICHDatabaseImporter::exportHapdQE()
{
  DBArray<ARICHHapdQE> elements("ARICHHapdQE");
  elements.getEntries();
  gROOT->SetBatch(kTRUE);

  // Example that prints serial numbers of HAPDs and saves QE 2D histograms to root file
  unsigned int el = 0;
  for (const auto& element : elements) {
    B2INFO("SN = " << element.getHapdSerialNumber());
    TH2F* qe2d = element.getQuantumEfficiency2D();
    TFile file("QEhists.root", "update");
    qe2d->Write();
    file.Close();
    el++;
  }

}

void ARICHDatabaseImporter::getBiasVoltagesForHapdChip(const std::string& serialNumber)
{
  // example that shows how to extract and use data
  // it calculates bias voltage at gain = 40 for each chip

  DBArray<ARICHHapdChipInfo> elements("ARICHHapdChipInfo");
  elements.getEntries();

  for (const auto& element : elements) {
    if (element.getHapdSerial() == serialNumber) {
      TGraph* avalgain = element.getAvalancheGain();

      // use linear interpolation to get bias voltage at gain = 40
      /*
            // 1) you can do it by hand

            double A, B, C, D;
            for(int j = 0; j < 100; j++) {
              avalgain->GetPoint(j, A, B);
              if(B>40) {
                avalgain->GetPoint(j-1, A, B);
                avalgain->GetPoint(j, C, D);
                float k = (B-D)/(A-C);
                float n = B - k*A;
                float xgain = (40 - n)/k;
                B2INFO("serial#-chip = " << element.getHapdSerial() << "-" << element.getChipLabel() << "; " << "V(gain=40) = " << (int)(xgain+0.5));
                j = 100;
              }
            }
      */

      // 2) use "Eval" function
      // - avalgain graph is gain(voltage)
      // - function "Eval" can be used to interpolate around chosen x
      // - convert graph gain(voltage) to voltage(gain) to interpolate around gain = 40
      TGraph* gainnew = new TGraph(avalgain->GetN());
      double xpoint, ypoint;
      for (int j = 0; j < avalgain->GetN(); j++) {
        avalgain->GetPoint(j, xpoint, ypoint);
        gainnew->SetPoint(j, ypoint, xpoint);
      }
      B2INFO("serial#-chip = " << element.getHapdSerial() << "-" << element.getChipLabel() << "; " << "V(gain=40) = " << (int)(
               gainnew->Eval(40) + 0.5));
    }
  }
}

void ARICHDatabaseImporter::getMyParams(const std::string& aeroSerialNumber)
{
  map<string, float> aerogelParams = ARICHDatabaseImporter::getAerogelParams(aeroSerialNumber);

  B2INFO("SN = " << aeroSerialNumber << "; n = " << aerogelParams.find("refractiveIndex")->second << "; transLen = " <<
         aerogelParams.find("transmissionLength")->second << "; thickness = " << aerogelParams.find("thickness")->second);
}


std::map<std::string, float> ARICHDatabaseImporter::getAerogelParams(const std::string& aeroSerialNumber)
{
  // Description:
  // This function loops over aerogel tiles and returns refractive index,
  // thickness and transmission length of aerogel for serial number

  std::map<std::string, float> aerogelParams;
  DBArray<ARICHAerogelInfo> elements("ARICHAerogelInfo");
  elements.getEntries();
  for (const auto& element : elements) {
    if ((element.getAerogelSerial()) == aeroSerialNumber) {
      aerogelParams = {
        { "refractiveIndex", element.getAerogelRefractiveIndex() },
        { "transmissionLength", element.getAerogelTransmissionLength() },
        { "thickness", element.getAerogelThickness() }
      };
    }
  }
  return aerogelParams;
}

void ARICHDatabaseImporter::importFEBoardInfo()
{

  GearDir content = GearDir("/ArichData/AllData/febasicmapping");

  // define data array
  TClonesArray febConstants("Belle2::ARICHFEBoardInfo");
  int feb = 0;

  // loop over xml files and extract the data
  for (const auto& febinfo : content.getNodes("febasic")) {
    int febSN = (float) febinfo.getInt("sn");
    string asic1 = febinfo.getString("asic1");
    string asic2 = febinfo.getString("asic2");
    string asic3 = febinfo.getString("asic3");
    string asic4 = febinfo.getString("asic4");
    string delivery = febinfo.getString("delivered");
    string sentKEK = febinfo.getString("sentKEK");

    if (delivery.size() != 10) cout << "feb sn " << febSN << " check delivery time!" << endl;

    string year = delivery.substr(6, 4);
    delivery = febinfo.getString("delivered");
    string month = delivery.substr(3, 2);
    delivery = febinfo.getString("delivered");
    string day = delivery.substr(0, 2);

    int deliveryINT = atoi((year + month + day + "u").c_str());
    TTimeStamp deliverytime(deliveryINT, 0u, 0u);

    string location = "";
    if (!sentKEK.empty()) location = "KEK";

    // save data as an element of the array
    new(febConstants[feb]) ARICHFEBoardInfo();
    auto* febConst = static_cast<ARICHFEBoardInfo*>(febConstants[feb]);
    febConst->setFEBoardSerial(febSN);
    febConst->setAsicPosition(0, asic1);
    febConst->setAsicPosition(1, asic2);
    febConst->setAsicPosition(2, asic3);
    febConst->setAsicPosition(3, asic4);
    febConst->setTimeStamp(deliverytime);
    febConst->setFEBoardLocation(location);

    DBArray<ARICHAsicInfo> elementsAsic("ARICHAsicInfo");
    elementsAsic.getEntries();
    for (const auto& element : elementsAsic) {
      if (element.getAsicID() == asic1) {
        ARICHAsicInfo& elementValue = const_cast<ARICHAsicInfo&>(element);
        febConst->setAsicInfo(0, &elementValue);
      }
      if (element.getAsicID() == asic2) {
        ARICHAsicInfo& elementValue = const_cast<ARICHAsicInfo&>(element);
        febConst->setAsicInfo(1, &elementValue);
      }
      if (element.getAsicID() == asic3) {
        ARICHAsicInfo& elementValue = const_cast<ARICHAsicInfo&>(element);
        febConst->setAsicInfo(2, &elementValue);
      }
      if (element.getAsicID() == asic4) {
        ARICHAsicInfo& elementValue = const_cast<ARICHAsicInfo&>(element);
        febConst->setAsicInfo(3, &elementValue);
      }
    }

    feb++;
  }

  // define interval of validity
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // store under user defined name:
  Database::Instance().storeData("ARICHFEBoardInfo", &febConstants, iov);
}

void ARICHDatabaseImporter::exportFEBoardInfo()
{
  DBArray<ARICHFEBoardInfo> elements("ARICHFEBoardInfo");
  elements.getEntries();

  for (const auto& element : elements) {
    B2INFO("Feb sn = " << element.getFEBoardSerial());
    for (int i = 0; i < 4; i++) {
      B2INFO("ASIC " << i << " = " << element.getAsicPosition(i));
    }
  }
}


void ARICHDatabaseImporter::importModuleTest(const std::string& mypath, const std::string& HVtest)
{

  GearDir content;
  if (HVtest == "no")  content = GearDir("/ArichData/AllData/moduletest");
  else if (HVtest == "yes")  content = GearDir("/ArichData/AllData/moduletestHV");
  else B2INFO("Check HVB test parameter!");

  // define data array
  TClonesArray moduleConstants("Belle2::ARICHModuleTest");
  int module = 0;

  // loop over xml files and extract the data
  for (const auto& moduletest : content.getNodes("module")) {
    int febSN = (float) moduletest.getInt("febserial");
    string hapdSN = moduletest.getString("hapdserial");
    int hvbSN = -1;
    if (HVtest == "yes")  hvbSN = (float) moduletest.getInt("hvbserial");
    int run = moduletest.getInt("run");
    int runposition = moduletest.getInt("runposition");
    int isok = moduletest.getInt("isok");
    bool isOK = false;
    if (isok == 1) isOK = true;
    string comment = moduletest.getString("comment");

    vector<int> deadChannels;

    if (HVtest == "no") {
      vector<string> deadChs;
      string channels = moduletest.getString("dead");
      if (!channels.empty()) {
        if (channels.find(",") != string::npos) {
          while (channels.find(",") != string::npos) {
            string badchannel = channels.substr(0, channels.find(","));
            deadChs.push_back(badchannel);
            channels = channels.substr(channels.find(",") + 1);
          }
          deadChs.push_back(channels);
        }
      }

      for (unsigned int i = 0; i < deadChs.size(); i++)  {
        string CH = deadChs.at(i);
        int CHint = -1;
        if (CH.find("B") != string::npos) CHint = atoi((CH.substr(1)).c_str()) + 36;
        else if (CH.find("C") != string::npos) CHint = atoi((CH.substr(1)).c_str()) + 2 * 36;
        else if (CH.find("D") != string::npos) CHint = atoi((CH.substr(1)).c_str()) + 3 * 36;
        else CHint = atoi((CH.substr(1)).c_str());

        deadChannels.push_back(CHint);
      }
    }

    if (HVtest == "yes") {
      vector<int> deadChs;
      string channels = moduletest.getString("dead");
      if (!channels.empty()) {
        if (channels.find(",") != string::npos) {
          while (channels.find(",") != string::npos) {
            string badchannel = channels.substr(0, channels.find(","));
            deadChs.push_back(atoi(badchannel.c_str()));
            channels = channels.substr(channels.find(",") + 1);
          }
          deadChs.push_back(atoi(channels.c_str()));
        }
      }
      deadChannels = deadChs;
    }

    // define histograms
    TGraph* guardBias_th = 0;
    TGraph* chipVdiff_th[4] = {0};
    TGraph* chipLeak_th[4] = {0};
    TGraph* HV_th = 0;

    TGraph* guardBias_2Dx = 0;
    TGraph* chipVdiff_2Dx[4] = {0};
    TGraph* chipLeak_2Dx[4] = {0};
    TGraph* HV_2Dx = 0;

    TGraph* guardBias_2Dy = 0;
    TGraph* chipVdiff_2Dy[4] = {0};
    TGraph* chipLeak_2Dy[4] = {0};
    TGraph* HV_2Dy = 0;

    TH1F* gain = 0;
    TH2D* charge = 0;
    TH2D* th = 0;
    TH2D* scanX = 0;
    TH2D* scanY = 0;

    string runStr = "";
    if (run < 10) runStr = "000" + to_string(run);
    if ((run > 9) && (run < 100)) runStr = "00" + to_string(run);
    if (run > 99) runStr = "0" + to_string(run);
    TFile* f = TFile::Open((mypath + runStr + "/" + runStr + "_" + hapdSN + "_out.root").c_str(), "READ");

    // extract data
    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {

      string strime = key->GetName();

      if (strime.find("Guard") != string::npos) {
        if (strime.find("_T_0") != string::npos) {
          guardBias_th = (TGraph*)f->Get(strime.c_str());
        }
        if (strime.find("_T_1") != string::npos) {
          guardBias_2Dx = (TGraph*)f->Get(strime.c_str());
        }
        if (strime.find("_T_2") != string::npos) {
          guardBias_2Dy = (TGraph*)f->Get(strime.c_str());
        }
      }

      if (strime.find("BiasDifference") != string::npos) {
        for (int i = 0; i < 4; i++) {
          if (strime.find(("CHIP_" + to_string(i) + "_T_0").c_str()) != string::npos) {
            chipVdiff_th[i] = (TGraph*)f->Get(strime.c_str());
          }
          if (strime.find(("CHIP_" + to_string(i) + "_T_1").c_str()) != string::npos) {
            chipVdiff_2Dx[i] = (TGraph*)f->Get(strime.c_str());
          }
          if (strime.find(("CHIP_" + to_string(i) + "_T_2").c_str()) != string::npos) {
            chipVdiff_2Dy[i] = (TGraph*)f->Get(strime.c_str());
          }
        }
      }

      if (strime.find("LeakageCurrent") != string::npos) {
        for (int i = 0; i < 4; i++) {
          if (strime.find(("CHIP_" + to_string(i) + "_T_0").c_str()) != string::npos) {
            chipLeak_th[i] = (TGraph*)f->Get(strime.c_str());
          }
          if (strime.find(("CHIP_" + to_string(i) + "_T_1").c_str()) != string::npos) {
            chipLeak_2Dx[i] = (TGraph*)f->Get(strime.c_str());
          }
          if (strime.find(("CHIP_" + to_string(i) + "_T_2").c_str()) != string::npos) {
            chipLeak_2Dy[i] = (TGraph*)f->Get(strime.c_str());
          }
        }
      }

      if (strime.find("HV") != string::npos) {
        if (strime.find("_T_0") != string::npos) {
          HV_th = (TGraph*)f->Get(strime.c_str());
        }
        if (strime.find("_T_1") != string::npos) {
          HV_2Dx = (TGraph*)f->Get(strime.c_str());
        }
        if (strime.find("_T_2") != string::npos) {
          HV_2Dy = (TGraph*)f->Get(strime.c_str());
        }
      }

      if (strime.find("Gain_1D") != string::npos) {
        gain = (TH1F*)f->Get(strime.c_str());
        gain->SetDirectory(0);
      }

      if (strime.find("Charge_2D") != string::npos) {
        charge = (TH2D*)f->Get(strime.c_str());
        charge->SetDirectory(0);
      }

      if (strime.find("Threshold_2D") != string::npos) {
        th = (TH2D*)f->Get(strime.c_str());
        th->SetDirectory(0);
      }

      if (strime.find("Scan_2D_X") != string::npos) {
        scanX = (TH2D*)f->Get(strime.c_str());
        scanX->SetDirectory(0);
      }

      if (strime.find("Scan_2D_Y") != string::npos) {
        scanY = (TH2D*)f->Get(strime.c_str());
        scanY->SetDirectory(0);
      }
    }

    // save data as an element of the array
    new(moduleConstants[module]) ARICHModuleTest();
    auto* moduleConst = static_cast<ARICHModuleTest*>(moduleConstants[module]);
    moduleConst->setFebSN(febSN);
    moduleConst->setHapdSN(hapdSN);
    moduleConst->setRun(run);
    moduleConst->setRunPosition(runposition);
    moduleConst->setOK(isOK);
    moduleConst->setDeadChs(deadChannels);
    moduleConst->setComment(comment);
    moduleConst->setGuardBiasTH(guardBias_th);
    moduleConst->setHighVoltageTH(HV_th);
    moduleConst->setGuardBias2Dx(guardBias_2Dx);
    moduleConst->setHighVoltage2Dx(HV_2Dx);
    moduleConst->setGuardBias2Dy(guardBias_2Dy);
    moduleConst->setHighVoltage2Dy(HV_2Dy);
    moduleConst->setGain(gain);
    moduleConst->setChargeScan(charge);
    moduleConst->setTresholdScan(th);
    moduleConst->setLaserScanX(scanX);
    moduleConst->setLaserScanY(scanY);

    for (int i = 0; i < 4; i++) {
      moduleConst->setChipVdiffTH(i, chipVdiff_th[i]);
      moduleConst->setChipLeakTH(i, chipLeak_th[i]);
      moduleConst->setChipVdiff2Dx(i, chipVdiff_2Dx[i]);
      moduleConst->setChipLeak2Dx(i, chipLeak_2Dx[i]);
      moduleConst->setChipVdiff2Dy(i, chipVdiff_2Dy[i]);
      moduleConst->setChipLeak2Dy(i, chipLeak_2Dy[i]);
    }
    if (HVtest == "yes")  moduleConst->setHvbSN(hvbSN);

    module++;
    B2INFO("module no " << module - 1 << " saved to DB. HAPD SN = " << hapdSN << ", FEB SN = " << febSN);
    f->Close();
  }

  // define interval of validity
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // store under user defined name:
  if (HVtest == "no")  Database::Instance().storeData("ARICHModuleTest", &moduleConstants, iov);
  if (HVtest == "yes")  Database::Instance().storeData("ARICHModuleTestHV", &moduleConstants, iov);
}

void ARICHDatabaseImporter::exportModuleTest(const std::string& HVtest)
{

  if (HVtest == "no") {
    DBArray<ARICHModuleTest> elements("ARICHModuleTest");
    elements.getEntries();
    for (const auto& element : elements) {
      B2INFO("Feb sn = " << element.getFebSN() << ", hapd sn = " << element.getHapdSN() << ", run = " << element.getRun() <<
             ", run position = " << element.getRunPosition() << ", module is ok = " << element.getOK() << ", comment = " <<
             element.getComment());
      for (int i = 0; i < element.getDeadChsSize(); i++) {
        B2INFO("dead channel = " << element.getDeadCh(i) << " (hapd mapping)");
      }
    }
  }

  if (HVtest == "yes") {
    DBArray<ARICHModuleTest> elements("ARICHModuleTestHV");
    elements.getEntries();
    for (const auto& element : elements) {
      B2INFO("Feb sn = " << element.getFebSN() << ", hapd sn = " << element.getHapdSN() << ", hvb sn = " << element.getHvbSN() <<
             ", run = " << element.getRun() << ", run position = " << element.getRunPosition() << ", module is ok = " << element.getOK() <<
             ", comment = " << element.getComment());
      for (int i = 0; i < element.getDeadChsSize(); i++) {
        B2INFO("dead channel = " << element.getDeadCh(i) << " (asic mapping)");
      }
    }
  }
}

void ARICHDatabaseImporter::importSensorModuleInfo()
{
  GearDir content = GearDir("/ArichData/AllData/moduleposition");

  // define data array
  TClonesArray moduleInfoConstants("Belle2::ARICHSensorModuleInfo");

  int module = 0;

  // loop over xml files and extract the data
  for (const auto& sensor : content.getNodes("position")) {
    int febSerial = sensor.getInt("febserial");
    string hapdSerial = sensor.getString("hapdserial");
    int id = sensor.getInt("moduleID");
    int hvSerial = sensor.getInt("HVserial");

    // save data as an element of the array
    new(moduleInfoConstants[module]) ARICHSensorModuleInfo();
    auto* moduleInfoConst = static_cast<ARICHSensorModuleInfo*>(moduleInfoConstants[module]);
    moduleInfoConst->setSensorModuleID(id);
    moduleInfoConst->setFEBserial(febSerial);
    moduleInfoConst->setHAPDserial(hapdSerial);
    moduleInfoConst->setHVboardID(hvSerial);

    DBArray<ARICHHapdInfo> elementsHapd("ARICHHapdInfo");
    elementsHapd.getEntries();
    for (const auto& element : elementsHapd) {
      if (element.getSerialNumber() == hapdSerial) {
        ARICHHapdInfo& elementValue = const_cast<ARICHHapdInfo&>(element);
        moduleInfoConst->setHapdID(&elementValue);
      }
    }

    DBArray<ARICHFEBoardInfo> elementsFeb("ARICHFEBoardInfo");
    elementsFeb.getEntries();
    for (const auto& element : elementsFeb) {
      if (element.getFEBoardSerial() == febSerial) {
        ARICHFEBoardInfo& elementValue = const_cast<ARICHFEBoardInfo&>(element);
        moduleInfoConst->setFEBoardID(&elementValue);
      }
    }

    DBArray<ARICHModuleTest> elementsModule("ARICHModuleTestHV");
    elementsModule.getEntries();
    for (const auto& element : elementsModule) {
      if (element.getFebSN() == febSerial) {
        ARICHModuleTest& elementValue = const_cast<ARICHModuleTest&>(element);
        moduleInfoConst->setModuleTest(&elementValue);
      }
    }

    module++;
  }

  // define interval of validity
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // store under default name:
  Database::Instance().storeData("ARICHSensorModuleInfo", &moduleInfoConstants, iov);
}


void ARICHDatabaseImporter::importSensorModuleMap()
{

  GearDir content = GearDir("/ArichData/AllData/moduleposition");

  // define data array
  TClonesArray moduleMapConstants("Belle2::ARICHSensorModuleMap");

  int module = 0;

  // loop over xml files and extract the data
  for (const auto& sensor : content.getNodes("position")) {
    int sextant = sensor.getInt("sector");
    int ring = sensor.getInt("ring");
    int column = sensor.getInt("column");
    int id = sensor.getInt("moduleID");

    // save data as an element of the array
    new(moduleMapConstants[module]) ARICHSensorModuleMap();
    auto* moduleMapConst = static_cast<ARICHSensorModuleMap*>(moduleMapConstants[module]);
    moduleMapConst->setSensorModuleSextantID(sextant);
    moduleMapConst->setSensorModuleRingID(ring);
    moduleMapConst->setSensorModuleColumnID(column);
    moduleMapConst->setSensorGlobalID(id);

    DBArray<ARICHSensorModuleInfo> elementsModule("ARICHSensorModuleInfo");
    elementsModule.getEntries();
    for (const auto& element : elementsModule) {
      if (element.getSensorModuleID() == id) {
        ARICHSensorModuleInfo& elementValue = const_cast<ARICHSensorModuleInfo&>(element);
        moduleMapConst->setSensorModuleId(&elementValue);
      }
    }

    module++;
  }

  // define interval of validity
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // store under default name:
  Database::Instance().storeData("ARICHSensorModuleMap", &moduleMapConstants, iov);
}

void ARICHDatabaseImporter::exportSensorModuleMap()
{
  DBArray<ARICHSensorModuleMap> elements("ARICHSensorModuleMap");
  elements.getEntries();

  for (const auto& element : elements) {
    B2INFO("Sextant = " << element.getSensorModuleSextantID() << ", ring = " << element.getSensorModuleRingID() << ", column = " <<
           element.getSensorModuleColumnID());
    ARICHSensorModuleInfo* newelement = element.getSensorModuleId();
    B2INFO("module ID = " << newelement->getSensorModuleID() << ", feb = " << newelement->getFEBserial() << ", hapd = " <<
           newelement->getHAPDserial());
    ARICHHapdInfo* newerelement = newelement->getHapdID();
    B2INFO("Hapd Serial = " << newerelement->getSerialNumber() << "; HV = " << newerelement->getHighVoltage() << "; qe400 = " <<
           newerelement->getQuantumEfficiency400());
    for (int i = 0; i < 4; i++) {
      ARICHHapdChipInfo* newestelement = newerelement->getHapdChipInfo(i);
      B2INFO("Hapd Serial = " << newestelement->getHapdSerial() << "; chip = " << newestelement->getChipLabel() << "; gain = " <<
             newestelement->getGain());
    }
  }
}


void ARICHDatabaseImporter::importMagnetTest()
{
  GearDir content = GearDir("/ArichData/AllData/magnetTest");

  // define data array
  TClonesArray magnetConstants("Belle2::ARICHMagnetTest");
  int num = 0;
  string sn = "";

  // loop over xml files and extract the data
  for (const auto& module : content.getNodes("module")) {
    // save data as an element of the array
    new(magnetConstants[num]) ARICHMagnetTest();
    auto* magnetConst = static_cast<ARICHMagnetTest*>(magnetConstants[num]);

    int snint = module.getInt("hapdID");
    if (snint < 5000) sn = "KA";
    else sn = "ZJ";
    char hapdID[6];
    sprintf(hapdID, "%s%04d", sn.c_str(), snint);
    magnetConst->setSerialNumber(hapdID);

    vector<float> deadtimes;
    for (const auto& time : module.getNodes("deadtime/measurement")) {
      if (time.getString(".") != "-") {
        float deadtime = (float) time.getDouble(".");
        deadtimes.push_back(deadtime);
      }
    }
    magnetConst->setDeadTime(deadtimes);

    if (module.getString("lowerA") != "-") magnetConst->setDeadTimeLowerA((float) module.getDouble("lowerA"));
    if (module.getString("lowerB") != "-") magnetConst->setDeadTimeLowerB((float) module.getDouble("lowerB"));
    if (module.getString("lowerC") != "-") magnetConst->setDeadTimeLowerC((float) module.getDouble("lowerC"));
    if (module.getString("lowerD") != "-") magnetConst->setDeadTimeLowerD((float) module.getDouble("lowerD"));

    bool getter_reactivation = module.getBool("getter");
    string comment = module.getString("comment");
    magnetConst->setGetter(getter_reactivation);
    magnetConst->setComment(comment);

    num++;
  }

  // define interval of validity
  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

  // store under default name:
  Database::Instance().storeData("ARICHMagnetTest", &magnetConstants, iov);
}

void ARICHDatabaseImporter::exportMagnetTest()
{

  DBArray<ARICHMagnetTest> elements("ARICHMagnetTest");
  elements.getEntries();

  for (const auto& element : elements) {
    string getter = "no";
    if (element.getGetter() == 1) getter = "yes";
    B2INFO("SN = " << element.getSerialNumber() << "; after getter reactivation? " << getter);
    for (int i = 0; i < element.getDeadTimeSize();
         i++)  B2INFO("dead time = " << element.getDeadTime(i) << " (" << (i + 1) << ". measurement)");
    if (element.getDeadTimeLowerA() > 0.) B2INFO("lower voltage on chip A = " << element.getDeadTimeLowerA());
    if (element.getDeadTimeLowerB() > 0.) B2INFO("lower voltage on chip B = " << element.getDeadTimeLowerB());
    if (element.getDeadTimeLowerC() > 0.) B2INFO("lower voltage on chip C = " << element.getDeadTimeLowerC());
    if (element.getDeadTimeLowerD() > 0.) B2INFO("lower voltage on chip D = " << element.getDeadTimeLowerD());
    B2INFO("comment = " << element.getComment());
  }
}

void ARICHDatabaseImporter::exportAll()
{
  ARICHDatabaseImporter::exportAerogelInfo();
  ARICHDatabaseImporter::exportHapdQE();
  ARICHDatabaseImporter::exportModuleTest("no");
  ARICHDatabaseImporter::exportModuleTest("yes");
  ARICHDatabaseImporter::exportFEBoardInfo();
  ARICHDatabaseImporter::exportFebTest();
  ARICHDatabaseImporter::exportSensorModuleMap();
}
