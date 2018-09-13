/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Leonid Burmistrov                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHModulesInfo.h>
#include <geometry/Materials.h>
#include <arich/dbobjects/ARICHGeoHAPD.h>

#include <cmath>

using namespace std;
using namespace Belle2;

ARICHGeometryConfig::ARICHGeometryConfig(const GearDir& content)
{
  read(content);
}

void ARICHGeometryConfig::read(const GearDir& content)
{
  //------------------------------
  // Get ARICH geometry parameters from the gearbox
  //------------------------------
  m_bbstudy = content.getInt("BeamBackgroundStudy");

  GearDir envParams(content, "Envelope");
  m_masterVolume.setVolume(envParams.getLength("innerRadius"), envParams.getLength("outerRadius"), envParams.getLength("length"),
                           std::string("ARICH_Air"));
  m_masterVolume.setPlacement(envParams.getLength("xPosition"), envParams.getLength("yPosition"),
                              envParams.getLength("zPosition") + envParams.getLength("length") / 2., envParams.getAngle("xRotation"),
                              envParams.getAngle("yRotation"), envParams.getAngle("zRotation"));

  auto& materials = geometry::Materials::getInstance();

  GearDir detParams(content, "Detector/Module");

  m_hapd.setWallThickness(detParams.getLength("HAPD/windowThickness"));
  m_hapd.setPadSize(detParams.getLength("HAPD/padSize"));
  m_hapd.setChipGap(detParams.getLength("HAPD/chipGap"));
  m_hapd.setNPads(int(sqrt(detParams.getInt("HAPD/padNum"))), int(sqrt(detParams.getInt("HAPD/padNum"))));
  m_hapd.setWinThickness(detParams.getLength("HAPD/windowThickness"));
  m_hapd.setWinRefIndex(detParams.getDouble("HAPD/windowRefIndex"));
  m_hapd.setModuleSizeZ(detParams.getLength("height"));
  m_hapd.setWallMaterial(detParams.getString("HAPD/wallMaterial"));
  m_hapd.setWinMaterial(detParams.getString("HAPD/windowMaterial"));

  m_hapd.setFEBVolume(detParams.getLength("FEB/size"), detParams.getLength("FEB/size"), detParams.getLength("FEB/thickness"),
                      detParams.getString("FEB/material"));
  m_hapd.setHAPDVolume(detParams.getLength("HAPD/size"), detParams.getLength("HAPD/size"), detParams.getLength("HAPD/height"),
                       detParams.getString("HAPD/fillMaterial"));

  GearDir apdSurfParams(detParams, "HAPD/Surface");
  m_hapd.setAPDVolume(detParams.getLength("HAPD/apdSize"), detParams.getLength("HAPD/apdSize"),
                      detParams.getLength("HAPD/apdThickness"), detParams.getString("HAPD/apdMaterial"),
                      materials.createOpticalSurfaceConfig(apdSurfParams));

  GearDir mergerParams(content, "MergerPCB");
  // read merger PCB parameters
  m_merger.setMergerPCBMaterialName(mergerParams.getString("merger/material"));
  m_merger.setMergerPCBLenght(mergerParams.getDouble("merger/sizeL"));
  m_merger.setMergerPCBWidth(mergerParams.getDouble("merger/sizeW"));
  m_merger.setMergerPCBThickness(mergerParams.getDouble("merger/thickness"));
  m_merger.setMergerSlotID(mergerParams.getArray("merger/mergerSlotID"));
  m_merger.setMergerPosR(mergerParams.getArray("merger/mergerPosR"));
  m_merger.setMergerAngle(mergerParams.getArray("merger/mergerAngle"));
  m_merger.setEnvelopeCenterPosition(mergerParams.getDouble("mergerEnvelope/x0"), mergerParams.getDouble("mergerEnvelope/y0"),
                                     mergerParams.getDouble("mergerEnvelope/z0"));
  m_merger.setEnvelopeOuterRadius(mergerParams.getDouble("mergerEnvelope/outerRadius"));
  m_merger.setEnvelopeInnerRadius(mergerParams.getDouble("mergerEnvelope/innerRadius"));
  m_merger.setEnvelopeThickness(mergerParams.getDouble("mergerEnvelope/thickness"));
  m_merger.checkMergerPositionsDataConsistency();
  //m_merger.print();

  GearDir cablesEnvelopParams(content, "cablesEnvelope");
  // read cables envelop parameters
  m_cablesenvelope.setCablesEffectiveMaterialName(cablesEnvelopParams.getString("material"));
  m_cablesenvelope.setEnvelopeOuterRadius(cablesEnvelopParams.getDouble("outerRadius"));
  m_cablesenvelope.setEnvelopeInnerRadius(cablesEnvelopParams.getDouble("innerRadius"));
  m_cablesenvelope.setEnvelopeThickness(cablesEnvelopParams.getDouble("thickness"));
  m_cablesenvelope.setEnvelopeCenterPosition(cablesEnvelopParams.getDouble("x0"), cablesEnvelopParams.getDouble("y0"),
                                             cablesEnvelopParams.getDouble("z0"));
  m_cablesenvelope.checkCablesEnvelopDataConsistency();
  //m_cablesenvelope.print();

  GearDir coolingParams(content, "coolingPipe");
  // read ARICH cooling system parameters
  m_cooling.setEnvelopeOuterRadius(coolingParams.getDouble("coolingEnvelope/outerRadius"));
  m_cooling.setEnvelopeInnerRadius(coolingParams.getDouble("coolingEnvelope/innerRadius"));
  m_cooling.setEnvelopeThickness(coolingParams.getDouble("coolingEnvelope/thickness"));
  m_cooling.setEnvelopeCenterPosition(coolingParams.getDouble("coolingEnvelope/x0"), coolingParams.getDouble("coolingEnvelope/y0"),
                                      coolingParams.getDouble("coolingEnvelope/z0"));
  m_cooling.setCoolingPipeMaterialName(coolingParams.getString("cooling/material"));
  m_cooling.setRmin(coolingParams.getDouble("cooling/Rmin"));
  m_cooling.setRmax(coolingParams.getDouble("cooling/Rmax"));
  m_cooling.setCoolingGeometryID(coolingParams.getArray("cooling/coolingGeometryID"));
  m_cooling.setCoolingL(coolingParams.getArray("cooling/coolingL"));
  m_cooling.setCoolingPosPhi(coolingParams.getArray("cooling/coolingPosPhi"));
  m_cooling.setCoolingPosR(coolingParams.getArray("cooling/coolingPosR"));
  m_cooling.setCoolinRotationAngle(coolingParams.getArray("cooling/coolinRotationAngle"));
  m_cooling.setCoolingTestPlateMaterialName(coolingParams.getString("coolingTestPlate/material"));
  m_cooling.setColdTubeMaterialName(coolingParams.getString("coolingTestPlate/materialColdTube"));
  m_cooling.setCoolingTestPlateslengths(coolingParams.getDouble("coolingTestPlate/lengthX"),
                                        coolingParams.getDouble("coolingTestPlate/lengthY"), coolingParams.getDouble("coolingTestPlate/lengthZ"));
  m_cooling.setColdTubeR(coolingParams.getDouble("coolingTestPlate/coldTubeR"));
  m_cooling.setColdTubeSubtractedR(coolingParams.getDouble("coolingTestPlate/coldTubeSubtractedR"));
  m_cooling.setColdTubeWallThickness(coolingParams.getDouble("coolingTestPlate/coldTubeWallThickness"));
  m_cooling.setDepthColdTubeInPlate(coolingParams.getDouble("coolingTestPlate/depthColdTubeInPlate"));
  m_cooling.setColdTubeSpacing(coolingParams.getDouble("coolingTestPlate/coldTubeSpacing"));
  m_cooling.setColdTubeNumber(coolingParams.getInt("coolingTestPlate/coldTubeNumber"));
  m_cooling.setCoolingTestPlatePosR(coolingParams.getArray("coolingTestPlate/coolingTestPlatePosR"));
  m_cooling.setCoolingTestPlatePosPhi(coolingParams.getArray("coolingTestPlate/coolingTestPlatePosPhi"));
  m_cooling.setCoolingTestPlatePosZ0(coolingParams.getArray("coolingTestPlate/coolingTestPlatePosZ0"));
  m_cooling.checkCoolingSystemDataConsistency();
  //m_cooling.print();

  // read detector plane parameters
  modulesPosition(content);

  GearDir mirrParams(content, "Mirrors");
  if (mirrParams) {
    m_mirrors.setNMirrors(mirrParams.getInt("nMirrors"));
    m_mirrors.setStartAngle(mirrParams.getAngle("startAngle"));
    GearDir surfParams(mirrParams, "Surface");
    m_mirrors.setMirrorPlate(mirrParams.getLength("thickness"), mirrParams.getLength("width"), mirrParams.getLength("length"),
                             mirrParams.getString("material"), materials.createOpticalSurfaceConfig(surfParams));
    m_mirrors.setZPosition(mirrParams.getLength("zPosition") + mirrParams.getLength("width") / 2. - m_masterVolume.getLength() / 2.);
    m_mirrors.setRadius(mirrParams.getLength("radius"));
    m_mirrors.initializeDefault();
  }

  // read and prepare aerogel plane parameters
  GearDir aerogel(content, "Aerogel");

  std::vector<double> dPhi;
  std::vector<int> nAeroSlotsIndividualRing;
  for (auto ring : aerogel.getNodes("slotInRing/Ring")) {
    dPhi.push_back(2.* M_PI / ring.getInt());
    nAeroSlotsIndividualRing.push_back(ring.getInt());
  }
  std::vector<double> slotR;
  for (auto ring : aerogel.getNodes("slotR/Ring")) {
    slotR.push_back(ring.getLength());
  }

  m_aerogelPlane.setWallRadius(slotR);
  m_aerogelPlane.setWallDPhi(dPhi);
  m_aerogelPlane.setNAeroSlotsIndividualRing(nAeroSlotsIndividualRing);
  m_aerogelPlane.setTileGap(aerogel.getLength("tileGap"));
  //cout<<"aerogel.getLength(\"tileGap\") = "<<aerogel.getLength("tileGap")<<endl;

  m_aerogelPlane.setWallThickness(aerogel.getLength("wallThickness"));
  m_aerogelPlane.setWallHeight(aerogel.getLength("wallHeight"));
  m_aerogelPlane.addSupportPlate(aerogel.getLength("plateInnerR"), aerogel.getLength("plateOuterR"),
                                 aerogel.getLength("plateThickness"), aerogel.getString("material"));
  int ilayer = 1;
  for (auto layer : aerogel.getNodes("Layers/Layer")) {
    double thick = layer.getLength("thickness") / Unit::cm;
    std::string material = layer.getString("material");
    double refIndex = layer.getDouble("refIndex");
    double trLen = layer.getLength("trLength");
    m_aerogelPlane.setAerogelLayer(ilayer, thick, refIndex, trLen, material);
    ilayer++;
    //std::cout<<"    double thick = layer.getLength(thickness) / Unit::cm = "<<thick<<std::endl;
  }
  m_aerogelPlane.setFullAerogelMaterialDescriptionKey(aerogel.getInt("fullAerogelMaterialDescriptionKey"));
  m_aerogelPlane.setImgTubeThickness(aerogel.getDouble("imgTubeThickness"));
  m_aerogelPlane.setCompensationARICHairVolumeThick_min(aerogel.getDouble("compensationARICHairVolumeThick_min"));

  // Aerogel tiles
  GearDir aerotilesDir(content, "AerogelTiles");
  for (auto tileNode : aerotilesDir.getNodes("Tiles/Tile")) {
    int ring = tileNode.getInt("ring");
    int column = tileNode.getInt("column");
    int layerN = tileNode.getInt("layer");
    double n = tileNode.getDouble("n");
    double transmL = tileNode.getDouble("transmL");
    double thick = tileNode.getDouble("thick");
    std::string materialName = tileNode.getString("material");
    m_aerogelPlane.addTileParameters(ring, column, layerN, n, transmL, thick, materialName);
  }

  if (m_aerogelPlane.getFullAerogelMaterialDescriptionKey() == 0) {
    m_aerogelPlane.setPlacement(0.0, 0.0,
                                aerogel.getLength("zPosition") +
                                (aerogel.getLength("wallHeight") + aerogel.getLength("plateThickness") + aerogel.getLength("imgTubeThickness")) / 2.0 -
                                m_masterVolume.getLength() / 2.0,
                                0, 0, 0);
  } else if (m_aerogelPlane.getFullAerogelMaterialDescriptionKey() == 1) {
    double wallHeightNew = m_aerogelPlane.getMaximumTotalTileThickness() + m_aerogelPlane.getCompensationARICHairVolumeThick_min();
    wallHeightNew = wallHeightNew / 10.0; //convertion from mm to cm - this need to be implemented properly
    //cout<<"m_aerogelPlane.getMaximumTotalTileThickness()           = "<<m_aerogelPlane.getMaximumTotalTileThickness()<<endl
    //  <<"m_aerogelPlane.getCompensationARICHairVolumeThick_min() = "<<m_aerogelPlane.getCompensationARICHairVolumeThick_min()<<endl
    //  <<"wallHeightNew                                           = "<<wallHeightNew<<endl;
    m_aerogelPlane.setPlacement(0.0, 0.0,
                                aerogel.getLength("zPosition") +
                                (wallHeightNew + aerogel.getLength("plateThickness") + aerogel.getLength("imgTubeThickness")) / 2.0 - m_masterVolume.getLength() /
                                2.0,
                                0, 0, 0);
  } else {
    B2ERROR("ARICHGeometryConfig::read --> getFullAerogelMaterialDescriptionKey() is wrong");
  }

  //m_aerogelPlane.print();
  //m_aerogelPlane.printTileParameters();
  //m_aerogelPlane.testGetTileParametersFunction();
  m_aerogelPlane.isConsistent();

  // support structures
  GearDir supportDir(content, "SupportStructure");
  for (auto tube : supportDir.getNodes("Tubes/Tube")) {
    std::string material = tube.getString("material");
    double innerR = tube.getLength("innerR");
    double outerR = tube.getLength("outerR");
    double length = tube.getLength("length");
    double zPosition = tube.getLength("zPosition") - m_masterVolume.getLength() / 2.;
    std::string name = tube.getString("name");
    m_supportStructure.addTube(innerR, outerR, length, zPosition, material, name);
  }
  //m_supportStructure.print();

  m_supportStructure.setMaterial(supportDir.getString("material"));

  for (auto wedge : supportDir.getNodes("Wedges/Wedge")) {
    std::vector<double> pars;
    int id = wedge.getInt("@id");
    if (id > 3 || id < 1) continue;
    for (auto par : wedge.getNodes("par")) {
      pars.push_back(par.getLength());
    }
    m_supportStructure.setWedge(id, pars);
  }

  for (auto wedge : supportDir.getNodes("Wedges/Placements/Placement")) {
    int type = wedge.getInt("type");
    double r = wedge.getLength("r");
    double phi = wedge.getAngle("phi");
    double z = wedge.getLength("z") - m_masterVolume.getLength() / 2.;
    m_supportStructure.addWedge(type, r, phi, z, supportDir.getString("material"));
  }

  //print();

}

void ARICHGeometryConfig::print(const std::string& title) const
{
  ARICHGeoBase::print(title);
  m_detectorPlane.print();
  m_hapd.print();
  m_merger.print();
  m_cablesenvelope.print();
  m_cooling.print();
  m_masterVolume.print();
  m_aerogelPlane.print();
  m_aerogelPlane.printTileParameters();
  m_mirrors.print();
  m_supportStructure.print();
}

void ARICHGeometryConfig::modulesPosition(const GearDir& content)
{

  GearDir detParams(content, "Detector/Plane");

  std::vector<std::pair<double, double>> ringPar;

  for (auto ring : detParams.getNodes("Rings/Ring")) {
    double r = ring.getLength("r");
    int nMod = ring.getInt("nModules");
    ringPar.push_back({r, 2.*M_PI / nMod});
  }

  m_detectorPlane.setRingPar(ringPar);

  GearDir supportParams(content, "Detector/supportPlate");
  double modHeight  = content.getLength("Detector/Module/height");
  m_detectorPlane.addSupportPlate(supportParams.getLength("innerRadius"), supportParams.getLength("outerRadius"),
                                  supportParams.getLength("thickness"), supportParams.getString("material"));
  m_detectorPlane.setModuleHoleSize(supportParams.getLength("moduleHoleSize"));
  m_detectorPlane.setSupportZPosition(supportParams.getLength("zPosition") + (supportParams.getLength("thickness") +
                                      supportParams.getLength("backWallHeight")) / 2. - m_masterVolume.getLength() / 2.);
  m_detectorPlane.setSupportBackWallHeight(supportParams.getLength("backWallHeight"));
  m_detectorPlane.setSupportBackWallThickness(supportParams.getLength("backWallThickness"));

  m_detectorPlane.setPlacement(0.0, 0.0, detParams.getLength("zPosition") + modHeight / 2. - m_masterVolume.getLength() / 2., 0.0,
                               0.0, 0.0);

}

/*void ARICHGeometryConfig::readMirrorAlignment(const GearDir& content)
  {
  GearDir modParams(content, "Mirrors/Alignment");

  BOOST_FOREACH(const GearDir & plate, modParams.getNodes("Plate")) {
  int id = atoi(plate.getString("@id").c_str());
  double dr = plate.getLength("dr");
  double dphi = plate.getAngle("dphi");
  double dtheta = plate.getAngle("dtheta");
  m_mirrorpoint[id - 1].SetMag(m_mirrorpoint[id - 1].Mag() + dr);
  m_mirrornorm[id - 1].SetTheta(m_mirrornorm[id - 1].Theta() + dtheta);
  m_mirrornorm[id - 1].SetPhi(m_mirrornorm[id - 1].Phi() + dphi);
  }
  }*/

TVector2 ARICHGeometryConfig::getChannelPosition(unsigned moduleID, unsigned chX, unsigned chY) const
{
  TVector2 origin;
  origin.SetMagPhi(m_detectorPlane.getSlotR(moduleID), m_detectorPlane.getSlotPhi(moduleID));
  double x, y;
  m_hapd.getXYChannelPos(chX, chY, x, y);
  TVector2 locPos(x, y);
  return origin + locPos.Rotate(m_detectorPlane.getSlotPhi(moduleID));
}
