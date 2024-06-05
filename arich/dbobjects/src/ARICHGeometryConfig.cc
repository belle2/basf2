/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/tessellatedSolidStr.h>

#include <geometry/Materials.h>
#include <arich/dbobjects/ARICHGeoHAPD.h>

#include <cmath>
#include <fstream>

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

  GearDir displParams(content, "GlobalDisplacement");
  m_displaceGlobal = !displParams.getBool("Disable");
  m_globalDispl.setX(displParams.getLength("x"));
  m_globalDispl.setY(displParams.getLength("y"));
  m_globalDispl.setZ(displParams.getLength("z"));
  m_globalDispl.setAlpha(displParams.getAngle("alpha"));
  m_globalDispl.setBeta(displParams.getAngle("beta"));
  m_globalDispl.setGamma(displParams.getAngle("gamma"));

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
  m_merger.setMergerPCBscrewholeR(mergerParams.getDouble("merger/mergerPCBscrewholeR"));
  m_merger.setMergerPCBscrewholePosdY(mergerParams.getDouble("merger/mergerPCBscrewholePosdY"));
  m_merger.setMergerPCBscrewholePosdX1(mergerParams.getDouble("merger/mergerPCBscrewholePosdX1"));
  m_merger.setMergerPCBscrewholePosdX2(mergerParams.getDouble("merger/mergerPCBscrewholePosdX2"));
  m_merger.setSingleMergerEnvelopeSizeL(mergerParams.getDouble("merger/envelopeSizeL"));
  m_merger.setSingleMergerEnvelopeSizeW(mergerParams.getDouble("merger/envelopeSizeW"));
  m_merger.setSingleMergerEnvelopeThickness(mergerParams.getDouble("merger/envelopeThickness"));
  m_merger.setSingleMergerenvelopeDeltaZ(mergerParams.getArray("merger/envelopeDeltaZ"));
  m_merger.setMergerSlotID(mergerParams.getArray("merger/mergerSlotID"));
  m_merger.setMergerPosR(mergerParams.getArray("merger/mergerPosR"));
  m_merger.setMergerAngle(mergerParams.getArray("merger/mergerAngle"));
  m_merger.setMergerOrientation(mergerParams.getArray("merger/mergerOrientation"));
  m_merger.setEnvelopeCenterPosition(mergerParams.getDouble("mergerEnvelope/x0"),
                                     mergerParams.getDouble("mergerEnvelope/y0"),
                                     mergerParams.getDouble("mergerEnvelope/z0"));
  m_merger.setSingleMergeEnvelopePosition(mergerParams.getDouble("merger/envelopePosX0"),
                                          mergerParams.getDouble("merger/envelopePosY0"),
                                          mergerParams.getDouble("merger/envelopePosZ0"));
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

  GearDir coolingFEBParams(content, "febcoolingv2");
  // read ARICH cooling system (v2) parameters
  // FEB cooling bodies
  m_coolingv2.setSmallSquareSize(coolingFEBParams.getDouble("smallSquareSize"));
  m_coolingv2.setSmallSquareThickness(coolingFEBParams.getDouble("smallSquareThickness"));
  m_coolingv2.setBigSquareSize(coolingFEBParams.getDouble("bigSquareSize"));
  m_coolingv2.setBigSquareThickness(coolingFEBParams.getDouble("bigSquareThickness"));
  m_coolingv2.setRectangleL(coolingFEBParams.getDouble("rectangleL"));
  m_coolingv2.setRectangleW(coolingFEBParams.getDouble("rectangleW"));
  m_coolingv2.setRectangleThickness(coolingFEBParams.getDouble("rectangleThickness"));
  m_coolingv2.setRectangleDistanceFromCenter(coolingFEBParams.getDouble("rectangleDistanceFromCenter"));
  m_coolingv2.setFebcoolingv2GeometryID(coolingFEBParams.getArray("febcoolingv2GeometryID"));

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

  GearDir mirrDisplParams(content, "MirrorDisplacement");
  if (mirrDisplParams) {
    m_displaceMirrors = !mirrDisplParams.getBool("Disable");
    for (auto plate : mirrDisplParams.getNodes("Plate")) {
      int id = plate.getInt("@id");
      double r = plate.getLength("r");
      double phi = plate.getAngle("phi");
      double z = plate.getLength("z");
      double alpha = plate.getLength("alpha");
      double beta = plate.getLength("beta");
      double gamma = plate.getLength("gamma");
      double origPhi = m_mirrors.getPoint(id).Phi();
      ARICHPositionElement displEl(r * cos(origPhi + phi), r * sin(origPhi + phi), z, alpha, beta, gamma);
      m_mirrorDispl.setDisplacementElement(id, displEl);
      // displEl.print();
    }
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
  for (int il = 0; il < ilayer - 1; il++) {
    int iring = 0;
    for (auto ns_ring :  nAeroSlotsIndividualRing) {
      iring++;
      for (int islot = 1; islot < ns_ring + 1; islot++) {
        for (auto tileNode : aerotilesDir.getNodes("Tiles/Tile")) {
          int ring = tileNode.getInt("ring");
          int column = tileNode.getInt("column");
          int layerN = tileNode.getInt("layer");
          if (iring == ring && column == islot && il == layerN) {
            double n = tileNode.getDouble("n");
            double transmL = tileNode.getDouble("transmL");
            double thick = tileNode.getDouble("thick");
            std::string materialName = tileNode.getString("material");
            m_aerogelPlane.addTileParameters(ring, column, layerN, n, transmL, thick, materialName);
          }
        }
      }
    }
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

ROOT::Math::XYVector ARICHGeometryConfig::getChannelPosition(unsigned moduleID, unsigned chX, unsigned chY) const
{
  const double radius = m_detectorPlane.getSlotR(moduleID);
  const double phi = m_detectorPlane.getSlotPhi(moduleID);
  const double cosPhi = std::cos(phi);
  const double sinPhi = std::sin(phi);
  ROOT::Math::XYVector origin(radius * cosPhi, radius * sinPhi);
  double x, y;
  m_hapd.getXYChannelPos(chX, chY, x, y);
  // create a vector from x, y that is rotated by phi
  ROOT::Math::XYVector locPos(x * cosPhi - y * sinPhi,
                              x * sinPhi + y * cosPhi);
  return origin + locPos;
}
