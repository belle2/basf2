/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/CDCGeoControlPar.h>
#include <cdc/simulation/CDCSimControlPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace boost;
using namespace Belle2;
using namespace CDC;

CDCGeometryPar* CDCGeometryPar::m_B4CDCGeometryParDB = 0;

CDCGeometryPar& CDCGeometryPar::Instance(const CDCGeometry* geom)
{
  if (!m_B4CDCGeometryParDB) m_B4CDCGeometryParDB = new CDCGeometryPar(geom);
  return *m_B4CDCGeometryParDB;
}

CDCGeometryPar::CDCGeometryPar(const CDCGeometry* geom)
{
#if defined(CDC_T0_FROM_DB)
  if (m_t0FromDB.isValid()) {
    m_t0FromDB.addCallback(this, &CDCGeometryPar::setT0);
  }
#endif
#if defined(CDC_BADWIRE_FROM_DB)
  if (m_badWireFromDB.isValid()) {
    m_badWireFromDB.addCallback(this, &CDCGeometryPar::setBadWire);
  }
#endif
#if defined(CDC_PROPSPEED_FROM_DB)
  if (m_propSpeedFromDB.isValid()) {
    m_propSpeedFromDB.addCallback(this, &CDCGeometryPar::setPropSpeed);
  }
#endif
#if defined(CDC_TIMEWALK_FROM_DB)
  if (m_timeWalkFromDB.isValid()) {
    m_timeWalkFromDB.addCallback(this, &CDCGeometryPar::setTW);
  }
#endif
#if defined(CDC_XTREL_FROM_DB)
  if (m_xtRelFromDB.isValid()) {
    m_xtRelFromDB.addCallback(this, &CDCGeometryPar::setXtRel);
  }
#endif
#if defined(CDC_SRESOL_FROM_DB)
  if (m_sResolFromDB.isValid()) {
    m_sResolFromDB.addCallback(this, &CDCGeometryPar::setSResol);
  }
#endif
#if defined(CDC_CHMAP_FROM_DB)
  if (m_chMapFromDB.isValid()) {
    m_chMapFromDB.addCallback(this, &CDCGeometryPar::setChMap);
  }
#endif
#if defined(CDC_DISPLACEMENT_FROM_DB)
  if (m_displacementFromDB.isValid()) {
    m_displacementFromDB.addCallback(this, &CDCGeometryPar::setDisplacement);
  }
#endif
#if defined(CDC_ALIGN_FROM_DB)
  if (m_alignFromDB.isValid()) {
    m_alignFromDB.addCallback(this, &CDCGeometryPar::setWirPosAlignParams);
  }
#endif
#if defined(CDC_MISALIGN_FROM_DB)
  if (m_misalignFromDB.isValid()) {
    m_misalignFromDB.addCallback(this, &CDCGeometryPar::setWirPosMisalignParams);
  }
#endif

  clear();
  if (geom) {
    B2INFO("CDCGeometryPar: readFromDB");
    readFromDB(*geom);
  } else {
    //    std::cout <<"readcalled" << std::endl;
    read();
  }
}

CDCGeometryPar::~CDCGeometryPar()
{
}

void CDCGeometryPar::clear()
{
  m_version = "unknown";
  m_nSLayer = 0;
  m_nFLayer = 0;
  m_senseWireDiameter = 0.0;
  m_senseWireTension  = 0.0;
  m_senseWireDensity  = 0.0;
  m_fieldWireDiameter = 0.0;

  m_tdcOffset         = 0;
  m_clockFreq4TDC     = 0.0;
  m_tdcBinWidth       = 0.0;
  m_nominalDriftV     = 0.0;
  m_nominalPropSpeed  = 0.0;
  m_nominalSpaceResol = 0.0;

  for (unsigned i = 0; i < 4; ++i) {
    m_rWall[i] = 0;
    for (unsigned j = 0; j < 2; ++j)
      m_zWall[i][j] = 0;
  }
  for (unsigned i = 0; i < MAX_N_SLAYERS; ++i) {
    m_rSLayer[i] = 0;
    m_zSForwardLayer[i] = 0;
    m_zSBackwardLayer[i] = 0;
    m_cellSize[i] = 0;
    m_nWires[i] = 0;
    m_offSet[i] = 0;
    m_nShifts[i] = 0;
  }
  for (unsigned i = 0; i < MAX_N_FLAYERS; ++i) {
    m_rFLayer[i] = 0;
    m_zFForwardLayer[i] = 0;
    m_zFBackwardLayer[i] = 0;
  }

  for (unsigned L = 0; L < MAX_N_SLAYERS; ++L) {
    for (unsigned C = 0; C < MAX_N_SCELLS; ++C) {
      for (unsigned i = 0; i < 3; ++i) {
        m_FWirPos        [L][C][i] = 0.;
        m_BWirPos        [L][C][i] = 0.;
        m_FWirPosMisalign[L][C][i] = 0.;
        m_BWirPosMisalign[L][C][i] = 0.;
        m_FWirPosAlign   [L][C][i] = 0.;
        m_BWirPosAlign   [L][C][i] = 0.;
      }
      m_WireSagCoef        [L][C] = 0.;
      m_WireSagCoefMisalign[L][C] = 0.;
      m_WireSagCoefAlign   [L][C] = 0.;
    }
  }

}

void CDCGeometryPar::readFromDB(const CDCGeometry& geom)
{
  m_globalPhiRotation = geom.getGlobalPhiRotation();

  // Get inner wall parameters
  m_rWall[0]    = geom.getInnerWall(2).getRmin();
  m_zWall[0][0] = geom.getInnerWall(0).getZbwd();
  m_zWall[0][1] = geom.getInnerWall(0).getZfwd();

  m_rWall[1] = geom.getInnerWall(0).getRmax();
  m_zWall[1][0] = geom.getInnerWall(0).getZbwd();
  m_zWall[1][1] = geom.getInnerWall(0).getZbwd();

  // Get outer wall parameters
  m_rWall[2] = geom.getOuterWall(0).getRmin();
  m_zWall[2][0] = geom.getOuterWall(0).getZbwd();
  m_zWall[2][1] = geom.getOuterWall(0).getZfwd();

  m_rWall[3] = geom.getOuterWall(1).getRmax();
  m_zWall[3][0] = geom.getOuterWall(0).getZbwd();
  m_zWall[3][1] = geom.getOuterWall(0).getZfwd();

  // Get sense layers parameters
  //  m_debug = geom.getDebugMode();
  m_debug = CDCGeoControlPar::getInstance().getDebug();
  m_nSLayer = geom.getNSenseLayers();

  //  m_materialDefinitionMode = geom.getMaterialDefinitionMode();
  m_materialDefinitionMode = CDCGeoControlPar::getInstance().getMaterialDefinitionMode();
  //  std::cout << m_materialDefinitionMode << std::endl;
  if (m_materialDefinitionMode == 0) {
    B2INFO("CDCGeometryPar: Define a mixture of gases and wires in the tracking volume.");
  } else if (m_materialDefinitionMode == 2) {
    //    B2INFO("CDCGeometryPar: Define all sense and field wires explicitly in the tracking volume.");
    B2FATAL("CDCGeometryPar: Materialdefinition=2 is disabled for now.");
  } else {
    B2FATAL("CDCGeometryPar: Materialdefinition mode you specify is invalid.");
  }

  // Get mode for wire z-position
  //  m_senseWireZposMode = geom.getSenseWireZposMode();
  m_senseWireZposMode = CDCGeoControlPar::getInstance().getSenseWireZposMode();
  //Set z corrections (from input data)
  B2INFO("CDCGeometryPar: sense wire z mode:" << m_senseWireZposMode);

  //
  // The DB version should be implemented ASAP.
  //
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CDC\"]/Content/");
  GearDir gbxParams(content);
  //  if (m_senseWireZposMode == 1) readDeltaz(gbxParams);


  //
  // Sense wires.
  //
  for (const auto& sense : geom.getSenseLayers()) {
    int layerId = sense.getId();
    m_rSLayer[layerId] = sense.getR();
    m_zSBackwardLayer[layerId] = sense.getZbwd();
    m_zSForwardLayer[layerId] = sense.getZfwd();
    m_nWires[layerId] = sense.getNWires();
    m_nShifts[layerId] = sense.getNShifts();
    m_offSet[layerId] = sense.getOffset();
    m_cellSize[layerId] = 2 * M_PI * m_rSLayer[layerId] / (double) m_nWires[layerId];
    m_dzSBackwardLayer[layerId] = sense.getDZbwd();
    m_dzSForwardLayer[layerId] = sense.getDZfwd();

    //correction to z-position
    if (m_senseWireZposMode == 0) {
    } else if (m_senseWireZposMode == 1) {
      //      m_zSBackwardLayer[layerId] += m_bwdDz[layerId];
      //      m_zSForwardLayer [layerId] += m_fwdDz[layerId];
      m_zSBackwardLayer[layerId] += m_dzSBackwardLayer[layerId];
      m_zSForwardLayer [layerId] -= m_dzSForwardLayer [layerId];
    } else {
      B2FATAL("CDCGeometryPar: invalid wire z definition mode specified");
    }

    //Set design sense-wire related params.
    const int nWires = m_nWires[layerId];
    for (int iCell = 0; iCell < nWires; ++iCell) {
      setDesignWirParam(layerId, iCell);
    }

  }

  // Get field layers parameters
  for (const auto& field : geom.getFieldLayers()) {
    int layerId = field.getId();
    m_rFLayer[layerId] = field.getR();
    m_zFBackwardLayer[layerId] = field.getZbwd();
    m_zFForwardLayer[layerId] = field.getZfwd();
  }

  // Get sense wire diameter
  m_senseWireDiameter = geom.getSenseDiameter();

  // Get sense wire tension
  m_senseWireTension = geom.getSenseTension();

  //  // Get sense wire density
  m_senseWireDensity = 19.3; // g/cm3  <- tentatively hard-coded here

  // Get field wire diameter
  m_fieldWireDiameter = geom.getFieldDiameter();

  //Set various quantities (should be moved to CDC.xml later...)
  m_tdcOffset = 8192;  //for common-stop mode; to be adjused later

  m_clockFreq4TDC = 1.017774;  //in GHz
  double tmp = geom.getClockFrequency();

  if (tmp != m_clockFreq4TDC) {
    B2WARNING("CDCGeometryPar: The default clock freq. for TDC (" << m_clockFreq4TDC << " GHz) is replaced with " << tmp << " (GHz).");
    m_clockFreq4TDC = tmp;
  }
  B2INFO("CDCGeometryPar: Clock freq. for TDC= " << m_clockFreq4TDC << " (GHz).");
  m_tdcBinWidth = 1. / m_clockFreq4TDC;  //in ns
  B2INFO("CDCGeometryPar: TDC bin width= " << m_tdcBinWidth << " (ns).");

  m_nominalDriftV    = 4.e-3;  //in cm/ns
  m_nominalDriftVInv = 1. / m_nominalDriftV; //in ns/cm
  m_nominalPropSpeed = 27.25;  //in cm/nsec (Belle's result, provided by iwasaki san)

  m_nominalSpaceResol = geom.getNominalSpaceResolution();

  m_maxSpaceResol = 2.5 * m_nominalSpaceResol;

  //Set displacement params. (from input data)
  //  m_Displacement = geom.getDisplacement();
  m_Displacement = CDCGeoControlPar::getInstance().getDisplacement();

  B2INFO("CDCGeometryPar: Load displacement params. (=1); not load (=0):" <<
         m_Displacement);
  if (m_Displacement) {

#if defined(CDC_DISPLACEMENT_FROM_DB)
    setDisplacement();
#else
    readWirePositionParams(c_Base, &geom, gbxParams);
#endif
  }

  //Set alignment params. (from input data)
  //  m_Alignment = geom.getAlignment();
  m_Alignment = CDCGeoControlPar::getInstance().getAlignment();
  B2INFO("CDCGeometryPar: Load alignment params. (=1); not load (=0):" <<
         m_Alignment);
  if (m_Alignment) {
#if defined(CDC_ALIGN_FROM_DB)
    setWirPosAlignParams();
#else
    readWirePositionParams(c_Aligned, &geom, gbxParams);
#endif
  }

  //Set misalignment params. (from input data)
  //  m_Misalignment = geom.getMisalignment();
  m_Misalignment = CDCGeoControlPar::getInstance().getMisalignment();
  B2INFO("CDCGeometryPar: Load misalignment params. (=1); not load (=0):" <<
         m_Misalignment);
  if (m_Misalignment) {
#if defined(CDC_MISALIGN_FROM_DB)
    setWirPosMisalignParams();
#else
    readWirePositionParams(c_Misaligned, &geom, gbxParams);
#endif
  }

  // Get control params. for CDC FullSim
  //  m_thresholdEnergyDeposit = geom.getEnergyDepositThreshold();
  m_thresholdEnergyDeposit = CDCSimControlPar::getInstance().getThresholdEnergyDeposit();
  //  m_minTrackLength = geom.getMinimumTrackLength();
  m_minTrackLength = CDCSimControlPar::getInstance().getMinTrackLength();
  //  m_wireSag = geom.getWireSagMode();
  m_wireSag = CDCSimControlPar::getInstance().getWireSag();
  //  m_modLeftRightFlag = geom.getModifiedLeftRightFlag();
  m_modLeftRightFlag = CDCSimControlPar::getInstance().getModLeftRightFlag();
  if (m_modLeftRightFlag) {
    B2FATAL("ModifiedLeftRightFlag = true is disabled for now; need to update a G4-related code in framework...");
  }
  //N.B. The following two lines are hard-coded temporarily to avoid job crash
  m_xtFileFormat = 1;
  m_sigmaFileFormat = 1;

  m_XTetc = true;
  if (m_XTetc) {
#if defined(CDC_XTREL_FROM_DB)
    B2INFO("CDCGeometryPar: Read XT from DB");
    setXtRel();  //Set xt param. (from DB)
#else
    readXT(gbxParams);  //Read xt params. (from file)
#endif

#if defined(CDC_SRESOL_FROM_DB)
    B2INFO("CDCGeometryPar: Read Sigma from DB");
    setSResol();  //Set sigma param. (from DB)
#else
    readSigma(gbxParams);  //Read sigma params. (from file)
#endif

#if defined(CDC_PROPSPEED_FROM_DB)
    setPropSpeed();  //Set prop-speed (from DB)
#else
    readPropSpeed(gbxParams);  //Read propagation speed
#endif

#if defined(CDC_T0_FROM_DB)
    setT0();  //Set t0 (from DB)
#else
    readT0(gbxParams);  //Read t0 (from file)
#endif

#if defined(CDC_BADWIRE_FROM_DB)
    setBadWire();  //Set bad-wire (from DB)
#else
    readBadWire(gbxParams);  //Read bad-wire (from file)
#endif

#if defined(CDC_CHMAP_FROM_DB)
    setChMap();  //Set ch-map (from DB)
#else
    readChMap(gbxParams);  //Read ch-map
#endif

#if defined(CDC_TIMEWALK_FROM_DB)
    setTW();  //Set time-walk coeffs. (from DB)
#else
    readTW(gbxParams);  //Read time-walk coeffs. (from file)
#endif

  }

  m_XTetc4Recon = 0;
  //  B2INFO("CDCGeometryPar: Load x-t etc. params. for reconstruction (=1); not load and use the same ones for digitization (=0):" <<
  B2INFO("CDCGeometryPar: Use the same x-t etc. for reconstruction as those used for digitization");
  if (m_XTetc4Recon) {
    readXT(gbxParams, 1);
    readSigma(gbxParams, 1);
    readPropSpeed(gbxParams, 1);
    readT0(gbxParams, 1);
    readTW(gbxParams, 1);
  }

  //calculate and save shifts in super-layers
  setShiftInSuperLayer();

}

void CDCGeometryPar::read()
{
  // Get the version of cdc geometry parameters
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CDC\"]/Content/");
  //------------------------------
  // Get CDC geometry parameters
  //------------------------------
  m_globalPhiRotation = content.getAngle("GlobalPhiRotation");
  //  std:: cout << content.getAngle("GlobalPhiRotation") << std::endl;


  int nBound = content.getNumberNodes("MomVol/ZBound");
  // Loop over to get the parameters of each boundary
  for (int iBound = 0; iBound < nBound; iBound++) {
    m_momZ[iBound] = content.getLength((format("MomVol/ZBound[%1%]/Z") % (iBound + 1)).str()) / Unit::mm;
    m_momRmin[iBound] = content.getLength((format("MomVol/ZBound[%1%]/Rmin") % (iBound + 1)).str()) / Unit::mm;
  }

  // Get inner wall parameters
  GearDir innerWallParams(content, "InnerWalls/");
  m_rWall[0]    = innerWallParams.getLength("InnerWall[3]/InnerR");
  m_zWall[0][0] = innerWallParams.getLength("InnerWall[1]/BackwardZ");
  m_zWall[0][1] = innerWallParams.getLength("InnerWall[1]/ForwardZ");

  m_rWall[1] = innerWallParams.getLength("InnerWall[1]/OuterR");
  m_zWall[1][0] = innerWallParams.getLength("InnerWall[1]/BackwardZ");
  m_zWall[1][1] = innerWallParams.getLength("InnerWall[1]/ForwardZ");

  // Get outer wall parameters
  GearDir outerWallParams(content, "OuterWalls/");
  m_rWall[2] = outerWallParams.getLength("OuterWall[1]/InnerR");
  m_zWall[2][0] = outerWallParams.getLength("OuterWall[1]/BackwardZ");
  m_zWall[2][1] = outerWallParams.getLength("OuterWall[1]/ForwardZ");

  m_rWall[3] = outerWallParams.getLength("OuterWall[2]/OuterR");
  m_zWall[3][0] = outerWallParams.getLength("OuterWall[1]/BackwardZ");
  m_zWall[3][1] = outerWallParams.getLength("OuterWall[1]/ForwardZ");

  // Get sense layers parameters
  GearDir gbxParams(content);
  m_debug = gbxParams.getBool("Debug");
  int nSLayer = gbxParams.getNumberNodes("SLayers/SLayer");
  m_nSLayer = nSLayer;

  // Get control switch for gas and wire material definition
  m_materialDefinitionMode = gbxParams.getInt("MaterialDefinitionMode");
  if (m_materialDefinitionMode == 0) {
    B2INFO("CDCGeometryPar: Define a mixture of gases and wires in the tracking volume.");
  } else if (m_materialDefinitionMode == 2) {
    //    B2INFO("CDCGeometryPar: Define all sense and field wires explicitly in the tracking volume.");
    B2FATAL("CDCGeometryPar: Materialdefinition=2 is disabled for now.");
  } else {
    B2FATAL("CDCGeometryPar: Materialdefinition mode you specify is invalid.");
  }

  // Get control params. for CDC FullSim
  GearDir gd(content);
  gd.append("/SensitiveDetector");
  m_thresholdEnergyDeposit = gd.getWithUnit("EnergyDepositionThreshold");
  m_minTrackLength = gd.getWithUnit("MinTrackLength");
  m_wireSag = gd.getBool("WireSag");
  m_modLeftRightFlag = gd.getBool("ModifiedLeftRightFlag");
  if (m_modLeftRightFlag) {
    B2FATAL("ModifiedLeftRightFlag = true is disabled for now; need to update a G4-related code in framework...");
  }

  // Get control switch for xt file format
  m_xtFileFormat = gbxParams.getInt("XtFileFormat");
  if (m_xtFileFormat == 0) {
    //    B2INFO("CDCGeometryPar: xt-file in old format specified");
    B2FATAL("CDCGeometryPar: xt-file format=0 is disabled now.");
  } else if (m_xtFileFormat == 1) {
    B2INFO("CDCGeometryPar: xt-file in new format specified");
  } else {
    B2FATAL("CDCGeometryPar: xt-file format you specify is invalid.");
  }

  // Get control switch for xt file format
  m_sigmaFileFormat = gbxParams.getInt("SigmaFileFormat");
  if (m_sigmaFileFormat == 0) {
    //    B2INFO("CDCGeometryPar: sigma-file in old format specified");
    B2FATAL("CDCGeometryPar: sigma-file format=0 is disabled now.");
  } else if (m_sigmaFileFormat == 1) {
    B2INFO("CDCGeometryPar: sigma-file in new format specified");
  } else {
    B2FATAL("CDCGeometryPar: sigma-file format you specify is invalid.");
  }

  // Get mode for wire z-position
  m_senseWireZposMode = gbxParams.getInt("SenseWireZposMode");
  //Set z corrections (from input data)
  B2INFO("CDCGeometryPar: sense wire z mode:" << m_senseWireZposMode);
  //  if (m_senseWireZposMode == 1) readDeltaz(gbxParams);

  // Loop over all sense layers
  for (int iSLayer = 0; iSLayer < nSLayer; ++iSLayer) {

    int layerId = atoi((gbxParams.getString((format("SLayers/SLayer[%1%]/@id") % (iSLayer + 1)).str())).c_str());

    m_rSLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/Radius") % (iSLayer + 1)).str());
    m_zSBackwardLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/BackwardZ") % (iSLayer + 1)).str());
    m_zSForwardLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/ForwardZ") % (iSLayer + 1)).str());
    m_nWires[layerId] = atoi((gbxParams.getString((format("SLayers/SLayer[%1%]/NHoles") % (iSLayer + 1)).str())).c_str()) / 2;
    //    std::cout << "layerid,m_nWires = " << layerId <<"  "<< m_nWires[layerId] << std::endl;
    m_nShifts[layerId] = atoi((gbxParams.getString((format("SLayers/SLayer[%1%]/NShift") % (iSLayer + 1)).str())).c_str());
    m_offSet[layerId] = atof((gbxParams.getString((format("SLayers/SLayer[%1%]/Offset") % (iSLayer + 1)).str())).c_str());
    m_cellSize[layerId] = 2 * M_PI * m_rSLayer[layerId] / (double) m_nWires[layerId];
    m_dzSBackwardLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/BwdDeltaZ") % (iSLayer + 1)).str());
    m_dzSForwardLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/FwdDeltaZ") % (iSLayer + 1)).str());

    //correction to z-position
    if (m_senseWireZposMode == 0) {
    } else if (m_senseWireZposMode == 1) {
      //      B2INFO("bwddz,fwddz=" << m_bwdDz[layerId] <<" "<< m_fwdDz[layerId]);
      //      B2INFO("bwd z,dz=" << m_zSBackwardLayer[layerId] <<" "<< m_dzSBackwardLayer[layerId]);
      //      B2INFO("fwd z,dz=" << m_zSForwardLayer[layerId] <<" "<< m_dzSForwardLayer[layerId]);
      //      m_zSBackwardLayer[layerId] += m_bwdDz[layerId];
      //      m_zSForwardLayer [layerId] += m_fwdDz[layerId];
      m_zSBackwardLayer[layerId] += m_dzSBackwardLayer[layerId];
      m_zSForwardLayer [layerId] -= m_dzSForwardLayer [layerId];
    } else {
      B2FATAL("CDCGeometryPar: invalid wire z definition mode specified");
    }
  }

  // Get field layers parameters
  int nFLayer = gbxParams.getNumberNodes("FLayers/FLayer");
  m_nFLayer = nFLayer;

  // Loop over all field layers
  for (int iFLayer = 0; iFLayer < nFLayer; iFLayer++) {
    int layerId = atoi((gbxParams.getString((format("FLayers/FLayer[%1%]/@id") % (iFLayer + 1)).str())).c_str());
    m_rFLayer[layerId] = gbxParams.getLength((format("FLayers/FLayer[%1%]/Radius") % (iFLayer + 1)).str());
    m_zFBackwardLayer[layerId] = gbxParams.getLength((format("FLayers/FLayer[%1%]/BackwardZ") % (iFLayer + 1)).str());
    m_zFForwardLayer[layerId] = gbxParams.getLength((format("FLayers/FLayer[%1%]/ForwardZ") % (iFLayer + 1)).str());
  }

  // Get sense wire diameter
  m_senseWireDiameter = gbxParams.getLength("SenseWire/Diameter");

  // Get sense wire tension
  m_senseWireTension = gbxParams.getLength("SenseWire/Tension");

  //  // Get sense wire density
  //  m_senseWireDensity = gbxParams.getDensity("Tungsten");
  m_senseWireDensity = 19.3; // g/cm3  <- tentatively hard-coded here

  //  cout << "diameter= " << m_senseWireDiameter << endl;
  //  cout << "tension = " << m_senseWireTension  << endl;
  //  cout << "density = " << m_senseWireDensity  << endl;

  // Get field wire diameter
  m_fieldWireDiameter = gbxParams.getLength("FieldWire/Diameter");

  //Set design sense-wire related params.
  for (int iSLayer = 0; iSLayer < nSLayer; ++iSLayer) {
    const int nWires = m_nWires[iSLayer];
    for (int iCell = 0; iCell < nWires; ++iCell) {
      setDesignWirParam(iSLayer, iCell);
      //      outputDesignWirParam(iSLayer, iCell);
    }
  }


  //Set various quantities (should be moved to CDC.xml later...)
  m_tdcOffset = 8192;  //for common-stop mode; to be adjused later

  m_clockFreq4TDC = 1.017774;  //in GHz
  double tmp = gbxParams.getDouble("ClockFrequencyForTDC");
  if (tmp != m_clockFreq4TDC) {
    B2WARNING("CDCGeometryPar: The default clock freq. for TDC (" << m_clockFreq4TDC << " GHz) is replaced with " << tmp << " (GHz).");
    m_clockFreq4TDC = tmp;
  }
  B2INFO("CDCGeometryPar: Clock freq. for TDC= " << m_clockFreq4TDC << " (GHz).");
  m_tdcBinWidth = 1. / m_clockFreq4TDC;  //in ns
  B2INFO("CDCGeometryPar: TDC bin width= " << m_tdcBinWidth << " (ns).");

  m_nominalDriftV    = 4.e-3;  //in cm/ns
  m_nominalDriftVInv = 1. / m_nominalDriftV; //in ns/cm
  m_nominalPropSpeed = 27.25;  //in cm/nsec (Belle's result, provided by iwasaki san)

  m_nominalSpaceResol = gbxParams.getLength("SenseWire/SpaceResol");
  m_maxSpaceResol = 2.5 * m_nominalSpaceResol;

  //Set displacement params. (from input data)
  m_Displacement = gbxParams.getBool("Displacement");
  B2INFO("CDCGeometryPar: Load displacement params. (=1); not load (=0):" <<
         m_Displacement);
  if (m_Displacement) {
    readWirePositionParams(c_Base, nullptr, gbxParams);
  }

  //Set misalignment params. (from input data)
  m_Misalignment = gbxParams.getBool("Misalignment");
  B2INFO("CDCGeometryPar: Load misalignment params. (=1); not load (=0):" <<
         m_Misalignment);
  if (m_Misalignment) {
    readWirePositionParams(c_Misaligned, nullptr, gbxParams);
  }

  //Set alignment params. (from input data)
  m_Alignment = gbxParams.getBool("Alignment");
  B2INFO("CDCGeometryPar: Load alignment params. (=1); not load (=0):" <<
         m_Alignment);
  if (m_Alignment) {
#if defined(CDC_ALIGN_FROM_DB)
    setWirPosAlignParams();
#else
    readWirePositionParams(c_Aligned, nullptr, gbxParams);
#endif
  }

  //Set xt etc. params. for digitization
  m_XTetc = gbxParams.getBool("XTetc");
  B2INFO("CDCGeometryPar: Load x-t etc. params. for digitization (=1); not load (=0):" << m_XTetc);
  if (m_XTetc) {
#if defined(CDC_XTREL_FROM_DB)
    setXtRel();  //Set xt param. (from DB)
#else
    readXT(gbxParams);  //Read xt params. (from file)
#endif

#if defined(CDC_SRESOL_FROM_DB)
    setSResol();  //Set sigma param. (from DB)
#else
    readSigma(gbxParams);  //Read sigma params. (from file)
#endif

#if defined(CDC_PROPSPEED_FROM_DB)
    setPropSpeed();  //Set prop-speed (from DB)
#else
    readPropSpeed(gbxParams);  //Read propagation speed
#endif

#if defined(CDC_T0_FROM_DB)
    setT0();  //Set t0 (from DB)
#else
    readT0(gbxParams);  //Read t0 (from file)
#endif

#if defined(CDC_BADWIRE_FROM_DB)
    setBadWire();  //Set bad-wire (from DB)
#else
    readBadWire(gbxParams);  //Read bad-wire (from file)
#endif

#if defined(CDC_CHMAP_FROM_DB)
    setChMap();  //Set ch-map (from DB)
#else
    readChMap(gbxParams);  //Read ch-map
#endif

#if defined(CDC_TIMEWALK_FROM_DB)
    setTW();  //Set time-walk coeffs. (from DB)
#else
    readTW(gbxParams);  //Read time-walk coeffs. (from file)
#endif
  }

  //Replace xt etc. with those for reconstriction
  m_XTetc4Recon = gbxParams.getBool("XTetc4Recon");
  B2INFO("CDCGeometryPar: Load x-t etc. params. for reconstruction (=1); not load and use the same ones for digitization (=0):" <<
         m_XTetc4Recon);
  if (m_XTetc4Recon) {
    readXT(gbxParams, 1);
    readSigma(gbxParams, 1);
    readPropSpeed(gbxParams, 1);
    readT0(gbxParams, 1);
    readTW(gbxParams, 1);
  }

  //calculate and save shifts in super-layers
  setShiftInSuperLayer();

  //Print();

}


// Read displacement or (mis)alignment params.
void CDCGeometryPar::readWirePositionParams(EWirePosition set,  const CDCGeometry* geom,  const GearDir gbxParams)
{

  std::string fileName0;
  CDCGeoControlPar& gcp = CDCGeoControlPar::getInstance();
  if (geom) {
    if (set == c_Base) {
      //      fileName0 = geom->getDisplacementFile();
      fileName0 = gcp.getDisplacementFile();
    } else if (set == c_Misaligned) {
      //      fileName0 = geom->getMisalignmentFile();
      fileName0 = gcp.getMisalignmentFile();
    } else if (set == c_Aligned) {
      //      fileName0 = geom->getAlignmentFile();
      fileName0 = gcp.getAlignmentFile();
    }
  } else {
    if (set == c_Base) {
      //      fileName0 = gbxParams.getString("displacementFileName");
      fileName0 = gcp.getDisplacementFile();
    } else if (set == c_Misaligned) {
      //      fileName0 = gbxParams.getString("misalignmentFileName");
      fileName0 = gcp.getMisalignmentFile();
    } else if (set == c_Aligned) {
      //      fileName0 = gbxParams.getString("alignmentFileName");
      fileName0 = gcp.getAlignmentFile();
    }
  }

  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  int iL(0), iC(0);
  const int np = 3;
  double back[np], fwrd[np], tension;
  unsigned nRead = 0;

  while (true) {
    ifs >> iL >> iC;
    for (int i = 0; i < np; ++i) {
      ifs >> back[i];
    }
    for (int i = 0; i < np; ++i) {
      ifs >> fwrd[i];
    }
    //    if (set != c_Base)  ifs >> tension;
    ifs >> tension;

    if (ifs.eof()) break;

    ++nRead;

    for (int i = 0; i < np; ++i) {
      if (set == c_Base) {
        m_BWirPos[iL][iC][i] += back[i];
        m_FWirPos[iL][iC][i] += fwrd[i];
      } else if (set == c_Misaligned) {
        m_BWirPosMisalign[iL][iC][i] = m_BWirPos[iL][iC][i] + back[i];
        m_FWirPosMisalign[iL][iC][i] = m_FWirPos[iL][iC][i] + fwrd[i];
      } else if (set == c_Aligned) {
        m_BWirPosAlign[iL][iC][i] = m_BWirPos[iL][iC][i] + back[i];
        m_FWirPosAlign[iL][iC][i] = m_FWirPos[iL][iC][i] + fwrd[i];
      }
    }

    //    double baseTension = 0.;

    if (set == c_Base) {
      m_WireSagCoef[iL][iC] = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.*(m_senseWireTension + tension));
      //      std::cout <<"base iL, iC, m_senseWireTension, tension= " << iL <<" " << iC <<" "<< m_senseWireTension <<" "<< tension << std::endl;
    } else if (set == c_Misaligned) {
      double baseTension = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.* m_WireSagCoef[iL][iC]);
      m_WireSagCoefMisalign[iL][iC] = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.*
                                      (baseTension + tension));
      //      std::cout <<"misa iL, iC,basetension, tension= " << iL <<" " << iC <<" "<< baseTension <<" "<< tension << std::endl;
    } else if (set == c_Aligned) {
      double baseTension = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.* m_WireSagCoef[iL][iC]);
      m_WireSagCoefAlign[iL][iC] = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.*(baseTension + tension));
      //      std::cout <<"algn iL, iC,basetension, tension= " << iL <<" " << iC <<" "<< baseTension <<" "<< tension << std::endl;
    }
    //    std::cout << "baseTension,tension= " << baseTension <<" "<< tension << std::endl;

    if (m_debug) {
      std::cout << iL << " " << iC;
      for (int i = 0; i < np; ++i) cout << " " << back[i];
      for (int i = 0; i < np; ++i) cout << " " << fwrd[i];
      std::cout << " " << tension << std::endl;
    }

  }

  if (nRead != nSenseWires) B2FATAL("CDCGeometryPar::readWirePositionParams: #lines read-in (=" << nRead <<
                                      ") is inconsistent with total #sense wires (=" << nSenseWires << ") !");

  ifs.close();
}


#if defined(CDC_ALIGN_FROM_DB)
// Set alignment wire positions
void CDCGeometryPar::setWirPosAlignParams()
{
  const int np = 3;
  double back[np], fwrd[np];

  for (unsigned iL = 0; iL < MAX_N_SLAYERS; ++iL) {
    for (unsigned iC = 0; iC < m_nWires[iL]; ++iC) {
      //      std::cout << "iLiC= " << iL <<" "<< iC << std::endl;
      WireID wire(iL, iC);
      back[0] = m_alignFromDB->get(wire, CDCAlignment::wireBwdX);
      back[1] = m_alignFromDB->get(wire, CDCAlignment::wireBwdY);
      back[2] = m_alignFromDB->get(wire, CDCAlignment::wireBwdZ);

      fwrd[0] = m_alignFromDB->get(wire, CDCAlignment::wireFwdX);
      fwrd[1] = m_alignFromDB->get(wire, CDCAlignment::wireFwdY);
      fwrd[2] = m_alignFromDB->get(wire, CDCAlignment::wireFwdZ);

      for (int i = 0; i < np; ++i) {
        m_BWirPosAlign[iL][iC][i] = m_BWirPos[iL][iC][i] + back[i];
        m_FWirPosAlign[iL][iC][i] = m_FWirPos[iL][iC][i] + fwrd[i];
      }

      //      double baseTension = 0.;
      double baseTension = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.* m_WireSagCoef[iL][iC]);
      double tension = m_alignFromDB->get(wire, CDCAlignment::wireTension);
      //      std::cout << back[0] <<" "<< back[1] <<" "<< back[2] <<" "<< fwrd[0] <<" "<< fwrd[1] <<" "<< fwrd[2] <<" "<< tension << std::endl;
      m_WireSagCoefAlign[iL][iC] = M_PI * m_senseWireDensity *
                                   m_senseWireDiameter * m_senseWireDiameter / (8.*(baseTension + tension));
      //    std::cout << "baseTension,tension= " << baseTension <<" "<< tension << std::endl;
    } //end of  layer loop
  } //end of cell loop
}
#endif


#if defined(CDC_MISALIGN_FROM_DB)
// Set misalignment wire positions
//TODO: merge this and setWirPosAlignParam() somehow
void CDCGeometryPar::setWirPosMisalignParams()
{
  const int np = 3;
  double back[np], fwrd[np];

  for (unsigned iL = 0; iL < MAX_N_SLAYERS; ++iL) {
    for (unsigned iC = 0; iC < m_nWires[iL]; ++iC) {
      //      std::cout << "iLiC= " << iL <<" "<< iC << std::endl;
      WireID wire(iL, iC);
      back[0] = m_misalignFromDB->get(wire, CDCMisalignment::wireBwdX);
      back[1] = m_misalignFromDB->get(wire, CDCMisalignment::wireBwdY);
      back[2] = m_misalignFromDB->get(wire, CDCMisalignment::wireBwdZ);

      fwrd[0] = m_misalignFromDB->get(wire, CDCMisalignment::wireFwdX);
      fwrd[1] = m_misalignFromDB->get(wire, CDCMisalignment::wireFwdY);
      fwrd[2] = m_misalignFromDB->get(wire, CDCMisalignment::wireFwdZ);

      for (int i = 0; i < np; ++i) {
        m_BWirPosMisalign[iL][iC][i] = m_BWirPos[iL][iC][i] + back[i];
        m_FWirPosMisalign[iL][iC][i] = m_FWirPos[iL][iC][i] + fwrd[i];
      }

      //      double baseTension = 0.;
      double baseTension = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.* m_WireSagCoef[iL][iC]);
      double tension = m_misalignFromDB->get(wire, CDCMisalignment::wireTension);
      //      std::cout << back[0] <<" "<< back[1] <<" "<< back[2] <<" "<< fwrd[0] <<" "<< fwrd[1] <<" "<< fwrd[2] <<" "<< tension << std::endl;
      m_WireSagCoefMisalign[iL][iC] = M_PI * m_senseWireDensity *
                                      m_senseWireDiameter * m_senseWireDiameter / (8.*(baseTension + tension));
      //    std::cout << "baseTension,tension= " << baseTension <<" "<< tension << std::endl;
    } //end of  layer loop
  } //end of cell loop
}
#endif


// Read x-t params.
void CDCGeometryPar::readXT(const GearDir gbxParams, const int mode)
{
  if (m_xtFileFormat == 0) {
    oldReadXT(gbxParams, mode);
  } else {
    newReadXT(gbxParams, mode);
  }
}


// Read x-t params. (new)
void CDCGeometryPar::newReadXT(const GearDir gbxParams, const int mode)
{
  m_linearInterpolationOfXT = true;  //must be true now

  //  std::string fileName0 = gbxParams.getString("xtFileName");
  std::string fileName0 = CDCGeoControlPar::getInstance().getXtFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("xt4ReconFileName");
  }

  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  //read alpha bin info.
  unsigned short nAlphaBins = 0;
  if (ifs >> nAlphaBins) {
    if (nAlphaBins == 0 || nAlphaBins > maxNAlphaPoints) B2FATAL("Fail to read alpha bins !");
  } else {
    B2FATAL("Fail to read alpha bins !");
  }
  m_nAlphaPoints = nAlphaBins;
  double alpha0, alpha1, alpha2;
  for (unsigned short i = 0; i < nAlphaBins; ++i) {
    ifs >> alpha0 >> alpha1 >> alpha2;
    m_alphaPoints[i] = alpha2;
  }

  //read theta bin info.
  unsigned short nThetaBins = 0;
  if (ifs >> nThetaBins) {
    if (nThetaBins == 0 || nThetaBins > maxNThetaPoints) B2FATAL("CDCGeometryPar: fail to read theta bins !");
  } else {
    B2FATAL("CDCGeometryPar: fail to read theta bins !");
  }
  m_nThetaPoints = nThetaBins;
  double theta0, theta1, theta2;

  for (unsigned short i = 0; i < nThetaBins; ++i) {
    ifs >> theta0 >> theta1 >> theta2;
    m_thetaPoints[i] = theta2;
  }

  short np = 0;
  unsigned short iCL, iLR;
  const unsigned short npx = nXTParams - 1;
  double xtc[npx];
  double theta, alpha, dummy1;
  unsigned nRead = 0;

  ifs >> m_xtParamMode >> np;
  if (m_xtParamMode < 0 || m_xtParamMode > 2) B2FATAL("CDCGeometryPar: invalid xt-parameterization mode read !");
  if (m_xtParamMode == 1) B2FATAL("CDCGeometryPar: xt-parameterization mode=1 not ready yet");

  if (np <= 0 || np > npx) B2FATAL("CDCGeometryPar: no. of xt-params. outside limits !");

  const double epsi = 0.1;

  while (ifs >> iCL) {
    ifs >> theta >> alpha >> dummy1 >> iLR;
    for (int i = 0; i < np; ++i) {
      ifs >> xtc[i];
    }
    ++nRead;

    int itheta = -99;
    for (unsigned short i = 0; i < nThetaBins; ++i) {
      if (fabs(theta - m_thetaPoints[i]) < epsi) {
        itheta = i;
        break;
      }
    }
    if (itheta < 0) B2FATAL("CDCGeometryPar: thetas in xt.dat are inconsistent !");

    int ialpha = -99;
    for (unsigned short i = 0; i < nAlphaBins; ++i) {
      if (fabs(alpha - m_alphaPoints[i]) < epsi) {
        ialpha = i;
        break;
      }
    }
    if (ialpha < 0) B2FATAL("CDCGeometryPar: alphas in xt.dat are inconsistent !");

    for (int i = 0; i < np; ++i) {
      m_XT[iCL][iLR][ialpha][itheta][i] = xtc[i];
    }

    double bound = xtc[6];
    m_XT[iCL][iLR][ialpha][itheta][np] =
      xtc[0] + bound
      * (xtc[1] + bound
         * (xtc[2] + bound
            * (xtc[3] + bound
               * (xtc[4] + bound
                  * (xtc[5])))));
  }  //end of while loop

  //convert unit
  const double degrad = M_PI / 180.;
  for (unsigned i = 0; i < nAlphaBins; ++i) {
    m_alphaPoints[i] *= degrad;
  }
  for (unsigned i = 0; i < nThetaBins; ++i) {
    m_thetaPoints[i] *= degrad;
  }

}

// Read x-t params. (old)
void CDCGeometryPar::oldReadXT(const GearDir gbxParams, const int mode)
{
  m_linearInterpolationOfXT = true;  //must be true now

  std::string fileName0 = gbxParams.getString("xtFileName");
  if (mode == 1) {
    fileName0 = gbxParams.getString("xt4ReconFileName");
  }

  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  int iL, lr;
  const int np = 9; //to be moved to appropriate place...
  double alpha, theta, dummy1, xt[np];
  double   oldTheta(-999), oldAlpha(-999);
  unsigned noOfThetaPoints(0);
  //  unsigned noOfAlphaPoints(1); //should start with one for alpha
  unsigned noOfAlphaPoints(0);

  //First read to check no.s of theta and alpha points
  double alphaPoints[maxNAlphaPoints] = {0.};

  int count = 0;
  while (ifs >> iL) {
    ++count;
    ifs >> theta >> alpha >> dummy1 >> lr;
    for (int i = 0; i < np - 1; ++i) {
      ifs >> xt[i];
    }

    if (theta != oldTheta) {
      unsigned short iarg = std::min(noOfThetaPoints, maxNThetaPoints);
      m_thetaPoints[iarg] = theta;
      ++noOfThetaPoints;
      oldTheta = theta;
    }

    if (noOfThetaPoints == 1 && alpha != oldAlpha) {
      unsigned short iarg = std::min(noOfAlphaPoints, maxNAlphaPoints);
      alphaPoints[iarg] = alpha;
      ++noOfAlphaPoints;
      oldAlpha = alpha;
    }
  }

  if (noOfThetaPoints > maxNThetaPoints) B2FATAL("CDCGeometryPar: Inconsistent no. of theta points ! real= " << noOfThetaPoints <<
                                                   " preset= " << maxNThetaPoints);
  m_nThetaPoints = noOfThetaPoints;
  if (noOfAlphaPoints > maxNAlphaPoints) B2FATAL("CDCGeometryPar: Inconsistent no. of alpha points ! real in file= " <<
                                                   noOfAlphaPoints << " preset= " << maxNAlphaPoints);
  m_nAlphaPoints = noOfAlphaPoints;

  //sort in order of magnitude
  for (unsigned i = 0; i < m_nAlphaPoints; ++i) {
    m_alphaPoints[m_nAlphaPoints - 1 - i] = alphaPoints[i];
  }

  //Second read to set all the others
  //  std::cout <<"before rewind" <<" "<< ifs.eof() << std::endl;
  ifs.clear(); //necessary to make the next line work
  ifs.seekg(0, ios_base::beg);
  //  std::cout <<"after  rewind" <<" "<< ifs.eof() << std::endl;
  unsigned nRead = 0;

  while (ifs >> iL) {
    //
    // Read a line of xt-parameter from Garfield calculations.
    //
    ifs >> theta >> alpha >> dummy1 >> lr;
    for (int i = 0; i < np - 1; ++i) {
      ifs >> xt[i];
    }
    ++nRead;

    int itheta = 0;
    for (unsigned i = 0; i < m_nThetaPoints; ++i) {
      if (theta == m_thetaPoints[i]) itheta = i;
      //      std::cout << m_thetaPoints[i] << std::endl;
    }

    int ialpha = 0;
    for (unsigned i = 1; i < m_nAlphaPoints; ++i) {
      if (alpha == m_alphaPoints[i]) ialpha = i;
      //      std::cout << m_alphaPoints[i] << std::endl;
    }

    for (int i = 0; i < np - 1; ++i) {
      m_XT[iL][lr][ialpha][itheta][i] = xt[i];
    }

    if (m_XT[iL][lr][ialpha][itheta][1] * m_XT[iL][lr][ialpha][itheta][7] < 0.) {
      //      B2WARNING("CDCGeometryPar: xt[7] sign is inconsistent with xt[1] sign -> set xt[7]=0");
      m_XT[iL][lr][ialpha][itheta][7] = 0.;
    }
    double bound = m_XT[iL][lr][ialpha][itheta][6];
    int i = np - 1;
    xt[i] = m_XT[iL][lr][ialpha][itheta][0] + bound
            * (m_XT[iL][lr][ialpha][itheta][1] + bound
               * (m_XT[iL][lr][ialpha][itheta][2] + bound
                  * (m_XT[iL][lr][ialpha][itheta][3] + bound
                     * (m_XT[iL][lr][ialpha][itheta][4] + bound
                        * (m_XT[iL][lr][ialpha][itheta][5])))));

    m_XT[iL][lr][ialpha][itheta][i] = xt[i];

    if (m_debug) {
      cout << iL << " " << alpha << " " << theta << " " << dummy1 << " " << lr;
      for (int i = 0; i < np; ++i) {
        cout << " " << xt[i];
      }
      cout << endl;
    }

    //    //convert unit, microsec -> nsec  <- tentative
    //    i = 1;
    //    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-3;
    //    i = 2;
    //    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-6;
    //    i = 3;
    //    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-9;
    //    i = 4;
    //    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-12;
    //    i = 5;
    //    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-15;
    //    i = 6;
    //    m_XT[iL][lr][ialpha][itheta][i] *= 1.e3;
    //    i = 7;
    //    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-3;

  }

  if (nRead != 2 * m_nAlphaPoints * m_nThetaPoints * MAX_N_SLAYERS) B2FATAL("CDCGeometryPar::readXT: #lines read-in (=" << nRead <<
        ") is inconsistent with 2*(#alpha-bins)*(#theta-bins)*(#layers) (=" << 2 * m_nAlphaPoints * m_nThetaPoints * MAX_N_SLAYERS <<
        ") !");

  ifs.close();

  //comment out the following lines since getClosestAlphaPoints is modified.
  /*
  //set xt(L/R,alpha=-90deg) = xt(R/L,alpha=90deg)
  for (unsigned iL = 0; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      //      int lrp = lr;
      int lrp = 0;
      if (lr == 0) lrp = 1;
      for (unsigned itheta = 0; itheta < m_nThetaPoints; ++itheta) {
        for (int i = 0; i < np; ++i) {
          double sgn = -1.;
          if (i == 6) sgn = 1;
          m_XT[iL][lr][0][itheta][i] = sgn * m_XT[iL][lrp][18][itheta][i];
        }
      }
    }
  }
  */

  //set xt(theta= 18) = xt(theta= 40) for the layers >= 20, since xt(theta=18) for these layers are unavailable
  for (unsigned iL = 20; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      for (unsigned ialpha = 0; ialpha < m_nAlphaPoints; ++ialpha) {
        for (int i = 0; i < np; ++i) {
          m_XT[iL][lr][ialpha][0][i] = m_XT[iL][lr][ialpha][1][i];
        }
      }
    }
  }

  //set xt(theta=130) = xt(theta=120) for the layers >= 37, since xt(theta=130) for these layers are unavailable
  for (unsigned iL = 37; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      for (unsigned ialpha = 0; ialpha < m_nAlphaPoints; ++ialpha) {
        for (int i = 0; i < np; ++i) {
          m_XT[iL][lr][ialpha][5][i] = m_XT[iL][lr][ialpha][4][i];
        }
      }
    }
  }

  //set xt(theta=149) = xt(theta=130) for the layers >= 13, since xt(theta=149) for these layers are unavailable
  for (unsigned iL = 13; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      for (unsigned ialpha = 0; ialpha < m_nAlphaPoints; ++ialpha) {
        for (int i = 0; i < np; ++i) {
          m_XT[iL][lr][ialpha][6][i] = m_XT[iL][lr][ialpha][5][i];
        }
      }
    }
  }

  //convert unit
  for (unsigned i = 0; i < m_nAlphaPoints; ++i) {
    m_alphaPoints[i] *= M_PI / 180.;
  }
  for (unsigned i = 0; i < m_nThetaPoints; ++i) {
    m_thetaPoints[i] *= M_PI / 180.;
  }

  /*
  iL = 55;
  int lr = 0;
  int ialpha = 8;
  int itheta = 3;
  for(int i=0; i<9; ++i) {
    std::cout << "xt,iL,lr,ialpha,itheta= " << iL <<" "<< lr <<" "<< ialpha <<" "<< itheta <<" "<< m_XT[iL][lr][ialpha][itheta][i] << std::endl;
  }
  lr = 1;
  for(int i=0; i<9; ++i) {
    std::cout << "xt,iL,lr,ialpha,itheta= " << iL <<" "<< lr <<" "<< ialpha <<" "<< itheta <<" "<< m_XT[iL][lr][ialpha][itheta][i] << std::endl;
  }
  */
}


// Read space resol. params.
void CDCGeometryPar::readSigma(const GearDir gbxParams, const int mode)
{
  if (m_sigmaFileFormat == 0) {
    oldReadSigma(gbxParams, mode);
  } else {
    newReadSigma(gbxParams, mode);
  }
}

void CDCGeometryPar::newReadSigma(const GearDir gbxParams, const int mode)
{
  m_linearInterpolationOfSgm = true; //must be true now

  //  std::string fileName0 = gbxParams.getString("sigmaFileName");
  std::string fileName0 = CDCGeoControlPar::getInstance().getSigmaFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("sigma4ReconFileName");
  }

  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  //read alpha bin info.
  unsigned short nAlphaBins = 0;
  if (ifs >> nAlphaBins) {
    if (nAlphaBins == 0 || nAlphaBins > maxNAlphaPoints) B2FATAL("Fail to read alpha bins !");
  } else {
    B2FATAL("Fail to read alpha bins !");
  }
  m_nAlphaPoints4Sgm = nAlphaBins;
  //  std:: cout << nAlphaBins << std::endl;
  double alpha0, alpha1, alpha2;
  for (unsigned short i = 0; i < nAlphaBins; ++i) {
    ifs >> alpha0 >> alpha1 >> alpha2;
    m_alphaPoints4Sgm[i] = alpha2;
    //    std:: cout << alpha2 << std::endl;
  }

  //read theta bin info.
  unsigned short nThetaBins = 0;
  if (ifs >> nThetaBins) {
    if (nThetaBins == 0 || nThetaBins > maxNThetaPoints) B2FATAL("CDCGeometryPar: fail to read theta bins !");
  } else {
    B2FATAL("CDCGeometryPar: fail to read theta bins !");
  }
  m_nThetaPoints4Sgm = nThetaBins;
  //  std:: cout << nThetaBins << std::endl;
  double theta0, theta1, theta2;

  for (unsigned short i = 0; i < nThetaBins; ++i) {
    ifs >> theta0 >> theta1 >> theta2;
    m_thetaPoints4Sgm[i] = theta2;
    //    std:: cout << theta2 << std::endl;
  }

  unsigned short np = 0;
  unsigned short iCL, iLR;
  double sigma[nSigmaParams];
  double theta, alpha;
  unsigned nRead = 0;

  ifs >> m_sigmaParamMode >> np;
  //  std:: cout << m_sigmaParamMode <<" "<< np << std::endl;
  if (m_sigmaParamMode < 0 || m_sigmaParamMode > 1) B2FATAL("CDCGeometryPar: invalid sigma-parameterization mode read !");
  if (m_sigmaParamMode == 1) B2FATAL("CDCGeometryPar: sigma-parameterization mode=1 not ready yet");

  if (np > nSigmaParams) B2FATAL("CDCGeometryPar: no. of sigma-params. outside limits !");

  const double epsi = 0.1;

  while (ifs >> iCL) {
    ifs >> theta >> alpha >> iLR;
    //    std::cout << iCL <<" "<< theta <<" "<< alpha <<" "<< iLR << std::endl;
    for (int i = 0; i < np; ++i) {
      ifs >> sigma[i];
    }
    ++nRead;

    int itheta = -99;
    for (unsigned short i = 0; i < nThetaBins; ++i) {
      if (fabs(theta - m_thetaPoints4Sgm[i]) < epsi) {
        itheta = i;
        break;
      }
    }
    if (itheta < 0) B2FATAL("CDCGeometryPar: thetas in sigma.dat are inconsistent !");

    int ialpha = -99;
    for (unsigned short i = 0; i < nAlphaBins; ++i) {
      if (fabs(alpha - m_alphaPoints4Sgm[i]) < epsi) {
        ialpha = i;
        break;
      }
    }
    if (ialpha < 0) B2FATAL("CDCGeometryPar: alphas in sigma.dat are inconsistent !");

    for (int i = 0; i < np; ++i) {
      m_Sigma[iCL][iLR][ialpha][itheta][i] = sigma[i];
    }
  }  //end of while loop

  //convert unit
  const double degrad = M_PI / 180.;
  for (unsigned i = 0; i < nAlphaBins; ++i) {
    m_alphaPoints4Sgm[i] *= degrad;
  }
  for (unsigned i = 0; i < nThetaBins; ++i) {
    m_thetaPoints4Sgm[i] *= degrad;
  }

  //  std::cout << "end of newreadsigma " << std::endl;
}

// Read space resol. params.
void CDCGeometryPar::oldReadSigma(const GearDir gbxParams, const int mode)
{
  std::string fileName0 = gbxParams.getString("sigmaFileName");
  if (mode == 1) {
    fileName0 = gbxParams.getString("sigma4ReconFileName");
  }
  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  int iL;
  const int np = 7;
  double sigma[np];
  unsigned nRead = 0;

  while (true) {
    ifs >> iL;
    for (int i = 0; i < np; ++i) {
      ifs >> sigma[i];
    }
    if (ifs.eof()) break;

    ++nRead;

    for (unsigned short iT = 0; iT < maxNThetaPoints; ++iT) {
      for (unsigned short iA = 0; iA < maxNAlphaPoints; ++iA) {
        for (unsigned short lr = 0; lr < 2; ++lr) {
          for (unsigned short i = 0; i < np; ++i) {
            m_Sigma[iL][lr][iA][iT][i] = sigma[i];
          }
        }
      }
    }

    //    m_Sigma[iL][np] = 0.5 * m_cellSize[iL] - 0.75;
    //    std::cout <<"L,p6= " << iL <<" "<< m_Sigma[iL][np] << std::endl;

    if (m_debug) {
      cout << iL;
      //      for (int i = 0; i < np + 1; ++i) {
      for (int i = 0; i < np; ++i) {
        cout << " " << m_Sigma[iL][0][0][0][i];
      }
      cout << endl;
    }
  }

  if (nRead != MAX_N_SLAYERS) B2FATAL("CDCGeometryPar::readSigma: #lines read-in (=" << nRead <<
                                        ") is inconsistent with total #layers (=" << MAX_N_SLAYERS << ") !");

  ifs.close();
}

// Read propagation speed param.
void CDCGeometryPar::readPropSpeed(const GearDir gbxParams, const int mode)
{
  //  std::string fileName0 = gbxParams.getString("propSpeedFileName");
  std::string fileName0 = CDCGeoControlPar::getInstance().getPropSpeedFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("propSpeed4ReconFileName");
  }
  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  int iL;
  double speed;
  unsigned nRead = 0;

  while (true) {
    ifs >> iL >> speed;
    if (ifs.eof()) break;

    ++nRead;

    m_propSpeedInv[iL] = 1. / speed;

    if (m_debug) cout << iL << " " << speed << endl;
  }

  if (nRead != MAX_N_SLAYERS) B2FATAL("CDCGeometryPar::readPropSpeed: #lines read-in (=" << nRead <<
                                        ") is inconsistent with total #layers (=" << MAX_N_SLAYERS << ") !");

  ifs.close();
}

/*
// Read deltaz params.
void CDCGeometryPar::readDeltaz(const GearDir gbxParams)
{
  std::string fileName0 = gbxParams.getString("deltazFileName");
  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  int iL;
  unsigned nRead = 0;

  while (ifs >> iL) {
    ifs >> m_bwdDz[iL] >> m_fwdDz[iL];
    ++nRead;
    if (m_debug) cout << iL << " " << m_bwdDz[iL] << " " << m_fwdDz[iL] << endl;
  }

  if (nRead != MAX_N_SLAYERS) B2FATAL("CDCGeometryPar::readDeltaz: #lines read-in (=" << nRead <<
                                        ") is inconsistent with total #layers (=" << MAX_N_SLAYERS << ") !");

  ifs.close();
}
*/


// Read t0 params.
void CDCGeometryPar::readT0(const GearDir gbxParams, int mode)
{
  //  std::string fileName0 = gbxParams.getString("t0FileName");
  std::string fileName0 = CDCGeoControlPar::getInstance().getT0File();
  if (mode == 1) {
    fileName0 = gbxParams.getString("t04ReconFileName");
  }
  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  int iL(0), iC(0);
  float t0(0);
  unsigned nRead = 0;

  while (true) {
    ifs >> iL >> iC >> t0;

    if (ifs.eof()) break;

    ++nRead;

    m_t0[iL][iC] = t0;

    if (m_debug) {
      std::cout << iL << " " << iC << " " << t0 << std::endl;
    }
  }

  if (nRead != nSenseWires) B2FATAL("CDCGeometryPar::readT0: #lines read-in (=" << nRead <<
                                      ") is inconsistent with total #sense wires (=" << nSenseWires << ") !");

  ifs.close();
}


// Read bad-wires.
void CDCGeometryPar::readBadWire(const GearDir gbxParams, int mode)
{
  //  std::string fileName0 = gbxParams.getString("bwFileName");
  std::string fileName0 = CDCGeoControlPar::getInstance().getBwFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("bw4ReconFileName");
  }
  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  int iCL(0), iW(0);
  unsigned nRead = 0;

  while (true) {
    ifs >> iCL >> iW;

    if (ifs.eof()) break;

    ++nRead;

    m_badWire.push_back(WireID(iCL, iW));

    if (m_debug) {
      std::cout << iCL << " " << iW << std::endl;
    }
  }

  if (nRead > nSenseWires) B2FATAL("CDCGeometryPar::readBadWire: #lines read-in (=" << nRead <<
                                     ") is larger than the total #sense wires (=" << nSenseWires << ") !");

  ifs.close();
}


// Read time-walk parameters
void CDCGeometryPar::readTW(const GearDir gbxParams, const int mode)
{
  //  std::string fileName0 = gbxParams.getString("twFileName");
  std::string fileName0 = CDCGeoControlPar::getInstance().getTwFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("tw4ReconFileName");
  }

  fileName0 = "/cdc/data/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  unsigned iBoard = 0;
  float coef = 0.;
  unsigned nRead = 0;

  while (true) {
    // Read board id and coefficient
    ifs >> iBoard >> coef;
    if (ifs.eof()) break;
    m_timeWalkCoef[iBoard] = coef;
    ++nRead;
  }

  if (nRead != nBoards) B2FATAL("CDCGeometryPar::readTW: #lines read-in (=" << nRead << ") is inconsistent with #boards (=" << nBoards
                                  << ") !");

  ifs.close();
}


// Read ch-map
void CDCGeometryPar::readChMap(const GearDir gbxParams)
{
  //  std::string fileName0 = gbxParams.getString("chmapFileName");
  std::string fileName0 = CDCGeoControlPar::getInstance().getChMapFile();
  fileName0 = "/cdc/data/" + fileName0;

  std::string fileName = FileSystem::findFile(fileName0);

  ifstream ifs;

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName0 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: " << fileName0 << " exists.");
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName0 << " !");
  }

  unsigned short iSL, iL, iW, iB, iC;
  unsigned nRead = 0;

  while (true) {
    // Read a relation
    ifs >> iSL >> iL >> iW >> iB >> iC;
    if (ifs.eof()) break;

    if (iSL >= nSuperLayers) continue;
    ++nRead;
    WireID wID(iSL, iL, iW);
    m_wireToBoard.insert(pair<WireID, unsigned short>(wID, iB));
  }

  if (nRead != nSenseWires) B2FATAL("CDCGeometryPar::readChMap: #lines read-in (=" << nRead <<
                                      ") is inconsistent with #sense-wires (="
                                      << nSenseWires << ") !");

  ifs.close();
}


#if defined(CDC_T0_FROM_DB)
// Set t0 (from DB)
void CDCGeometryPar::setT0()
{
  for (auto const& ent : m_t0FromDB->getT0s()) {
    const WireID wid = WireID(ent.first);
    const unsigned short iCL = wid.getICLayer();
    const unsigned short iW  = wid.getIWire();
    m_t0[iCL][iW]            = ent.second;
  }
}
#endif

#if defined(CDC_BADWIRE_FROM_DB)
// Set bad-wire (from DB)
void CDCGeometryPar::setBadWire()
{
  m_badWire = m_badWireFromDB->getWires();
}
#endif

#if defined(CDC_PROPSPEED_FROM_DB)
// Set prop.-speed (from DB)
void CDCGeometryPar::setPropSpeed()
{
  for (unsigned short iCL = 0; iCL < m_propSpeedFromDB->getEntries(); ++iCL) {
    m_propSpeedInv[iCL] = 1. / m_propSpeedFromDB->getSpeed(iCL);
  }
}
#endif


#if defined(CDC_TIMEWALK_FROM_DB)
// Set time-walk coefficient (from DB)
void CDCGeometryPar::setTW()
{
  for (unsigned short iBd = 0; iBd < m_timeWalkFromDB->getEntries(); ++iBd) {
    m_timeWalkCoef[iBd] = m_timeWalkFromDB->getTimeWalkParam(iBd);
  }
}
#endif


#if defined(CDC_XTREL_FROM_DB)
// Set xt params. (from DB)
void CDCGeometryPar::setXtRel()
{
  //  std::cout <<"setXtRelation called" << std::endl;
  m_nAlphaPoints = m_xtRelFromDB->getNoOfAlphaBins();
  for (unsigned short i = 0; i < m_nAlphaPoints; ++i) {
    m_alphaPoints[i] = m_xtRelFromDB->getAlphaPoint(i);
    //    std::cout << m_alphaPoints[i]*180./M_PI << std::endl;
  }

  m_nThetaPoints = m_xtRelFromDB->getNoOfThetaBins();
  for (unsigned short i = 0; i < m_nThetaPoints; ++i) {
    m_thetaPoints[i] = m_xtRelFromDB->getThetaPoint(i);
    //    std::cout << m_thetaPoints[i]*180./M_PI << std::endl;
  }

  for (unsigned short iCL = 0; iCL < MAX_N_SLAYERS; ++iCL) {
    for (unsigned short iLR = 0; iLR < 2; ++iLR) {
      for (unsigned short iA = 0; iA < m_nAlphaPoints; ++iA) {
        for (unsigned short iT = 0; iT < m_nThetaPoints; ++iT) {
          const std::vector<float> params = m_xtRelFromDB->getXtParams(iCL, iLR, iA, iT);
          unsigned short np = params.size();
          //    std::cout <<"np4xt= " << np << std::endl;
          for (unsigned short i = 0; i < np; ++i) {
            m_XT[iCL][iLR][iA][iT][i] = params[i];
          }

          double bound = m_XT[iCL][iLR][iA][iT][6];
          m_XT[iCL][iLR][iA][iT][np] =
            m_XT[iCL][iLR][iA][iT][0] + bound
            * (m_XT[iCL][iLR][iA][iT][1] + bound
               * (m_XT[iCL][iLR][iA][iT][2] + bound
                  * (m_XT[iCL][iLR][iA][iT][3] + bound
                     * (m_XT[iCL][iLR][iA][iT][4] + bound
                        * (m_XT[iCL][iLR][iA][iT][5])))));
        }
      }
    }
  }

}
#endif


#if defined(CDC_SRESOL_FROM_DB)
// Set sigma params. (from DB)
void CDCGeometryPar::setSResol()
{
  //  std::cout <<"setSResol called" << std::endl;
  m_nAlphaPoints4Sgm = m_sResolFromDB->getNoOfAlphaBins();
  for (unsigned short i = 0; i < m_nAlphaPoints4Sgm; ++i) {
    m_alphaPoints4Sgm[i] = m_sResolFromDB->getAlphaPoint(i);
    //    std::cout << m_alphaPoints4Sgm[i]*180./M_PI << std::endl;
  }

  m_nThetaPoints4Sgm = m_sResolFromDB->getNoOfThetaBins();
  for (unsigned short i = 0; i < m_nThetaPoints4Sgm; ++i) {
    m_thetaPoints4Sgm[i] = m_sResolFromDB->getThetaPoint(i);
    //    std::cout << m_thetaPoints4Sgm[i]*180./M_PI << std::endl;
  }

  //  std::cout << "m_nAlphaPoints4Sgm= " << m_nAlphaPoints4Sgm << std::endl;
  //  std::cout << "m_nThetaPoints4Sgm= " << m_nThetaPoints4Sgm << std::endl;

  for (unsigned short iCL = 0; iCL < MAX_N_SLAYERS; ++iCL) {
    for (unsigned short iLR = 0; iLR < 2; ++iLR) {
      for (unsigned short iA = 0; iA < m_nAlphaPoints4Sgm; ++iA) {
        for (unsigned short iT = 0; iT < m_nThetaPoints4Sgm; ++iT) {
          const std::vector<float> params = m_sResolFromDB->getSigmaParams(iCL, iLR, iA, iT);
          unsigned short np = params.size();
          //    std::cout <<"np4sigma= " << np << std::endl;
          for (unsigned short i = 0; i < np; ++i) {
            m_Sigma[iCL][iLR][iA][iT][i] = params[i];
          }
        }
      }
    }
  }

}
#endif


#if defined(CDC_CHMAP_FROM_DB)
// Set ch-map (from DB)
void CDCGeometryPar::setChMap()
{
  for (const auto& cm : m_chMapFromDB) {
    const unsigned short isl = cm.getISuperLayer();
    if (isl >= nSuperLayers) continue;
    const int il  = cm.getILayer();
    const int iw  = cm.getIWire();
    const int iBd = cm.getBoardID();
    const WireID wID(isl, il, iw);
    m_wireToBoard.insert(pair<WireID, unsigned short>(wID, iBd));
  }
}
#endif


void CDCGeometryPar::Print() const
{}

const TVector3 CDCGeometryPar::wireForwardPosition(int layerID, int cellID, EWirePosition set) const
{
  //  std::cout <<"cdcgeopar::fwdpos set= " << set << std::endl;
  TVector3 wPos(m_FWirPosAlign[layerID][cellID][0],
                m_FWirPosAlign[layerID][cellID][1],
                m_FWirPosAlign[layerID][cellID][2]);

  if (set == c_Misaligned) {
    wPos.SetX(m_FWirPosMisalign[layerID][cellID][0]);
    wPos.SetY(m_FWirPosMisalign[layerID][cellID][1]);
    wPos.SetZ(m_FWirPosMisalign[layerID][cellID][2]);
  } else if (set == c_Base) {
    wPos.SetX(m_FWirPos        [layerID][cellID][0]);
    wPos.SetY(m_FWirPos        [layerID][cellID][1]);
    wPos.SetZ(m_FWirPos        [layerID][cellID][2]);
  }
  return wPos;
}

const TVector3 CDCGeometryPar::wireForwardPosition(int layerID, int cellID, double z, EWirePosition set) const
{
  double yb_sag = 0.;
  double yf_sag = 0.;
  getWireSagEffect(set, layerID, cellID, z, yb_sag, yf_sag);

  TVector3 wPos(m_FWirPosAlign[layerID][cellID][0], yf_sag,
                m_FWirPosAlign[layerID][cellID][2]);
  if (set == c_Misaligned) {
    wPos.SetX(m_FWirPosMisalign[layerID][cellID][0]);
    wPos.SetZ(m_FWirPosMisalign[layerID][cellID][2]);
  } else if (set == c_Base) {
    wPos.SetX(m_FWirPos        [layerID][cellID][0]);
    wPos.SetZ(m_FWirPos        [layerID][cellID][2]);
  }
  return wPos;
}

const TVector3 CDCGeometryPar::wireBackwardPosition(int layerID, int cellID, EWirePosition set) const
{
  TVector3 wPos(m_BWirPosAlign[layerID][cellID][0],
                m_BWirPosAlign[layerID][cellID][1],
                m_BWirPosAlign[layerID][cellID][2]);

  if (set == c_Misaligned) {
    wPos.SetX(m_BWirPosMisalign[layerID][cellID][0]);
    wPos.SetY(m_BWirPosMisalign[layerID][cellID][1]);
    wPos.SetZ(m_BWirPosMisalign[layerID][cellID][2]);
  } else if (set == c_Base) {
    wPos.SetX(m_BWirPos        [layerID][cellID][0]);
    wPos.SetY(m_BWirPos        [layerID][cellID][1]);
    wPos.SetZ(m_BWirPos        [layerID][cellID][2]);
  }
  return wPos;
}

const TVector3 CDCGeometryPar::wireBackwardPosition(int layerID, int cellID, double z, EWirePosition set) const
{
  double yb_sag = 0.;
  double yf_sag = 0.;
  getWireSagEffect(set, layerID, cellID, z, yb_sag, yf_sag);

  TVector3 wPos(m_BWirPosAlign[layerID][cellID][0], yb_sag,
                m_BWirPosAlign[layerID][cellID][2]);
  if (set == c_Misaligned) {
    wPos.SetX(m_BWirPosMisalign[layerID][cellID][0]);
    wPos.SetZ(m_BWirPosMisalign[layerID][cellID][2]);
  } else if (set == c_Base) {
    wPos.SetX(m_BWirPos        [layerID][cellID][0]);
    wPos.SetZ(m_BWirPos        [layerID][cellID][2]);
  }
  return wPos;
}

double CDCGeometryPar::getWireSagCoef(EWirePosition set, int layerID, int cellID) const
{
  double coef =    m_WireSagCoef[layerID][cellID];
  if (set == c_Misaligned) {
    coef = m_WireSagCoefMisalign[layerID][cellID];
  } else if (set == c_Aligned) {
    coef = m_WireSagCoefAlign   [layerID][cellID];
  }
  return coef;
}

const double* CDCGeometryPar::innerRadiusWireLayer() const
{
  static double IRWL[MAX_N_SLAYERS] = {0};

  IRWL[0] = outerRadiusInnerWall();
  for (unsigned i = 1; i < nWireLayers(); i++)
    //IRWL[i] = (m_rSLayer[i - 1] + m_rSLayer[i]) / 2.;
    IRWL[i] = m_rFLayer[i - 1];

  return IRWL;
}

const double* CDCGeometryPar::outerRadiusWireLayer() const
{
  static double ORWL[MAX_N_SLAYERS] = {0};

  ORWL[nWireLayers() - 1] = innerRadiusOuterWall();
  for (unsigned i = 0; i < nWireLayers() - 1; i++)
    //ORWL[i] = (m_rSLayer[i] + m_rSLayer[i + 1]) / 2.;
    ORWL[i] = m_rFLayer[i];

  return ORWL;
}

unsigned CDCGeometryPar::cellId(unsigned layerId, const TVector3& position) const
{
  const unsigned nWires = m_nWires[layerId];

  double offset = m_offSet[layerId];
  //...Offset modification to be aligned to axial at z=0...
  const double phiSize = 2 * M_PI / double(nWires);
  /*{
    const double phiF = phiSize * offset
                        + phiSize * 0.5 * double(m_nShifts[layerId]);
    const double phiB = phiSize * offset;
    const TVector3 f(m_rSLayer[layerId] * cos(phiF), m_rSLayer[layerId] * sin(phiF), m_zSForwardLayer[layerId]);
    const TVector3 b(m_rSLayer[layerId] * cos(phiB), m_rSLayer[layerId] * sin(phiB), m_zSBackwardLayer[layerId]);

    const TVector3 v = f - b;
    const TVector3 u = v.Unit();
    const double beta = (0 - b.z()) / u.z();
    const TVector3 p = b + beta * u;
    double phi0 = - atan2(p.y(), p.x());
    offset += phi0 / (2 * M_PI / double(nWires));
  }*/

  unsigned j = 0;
  for (unsigned i = 0; i < 1; ++i) {
    const double phiF = phiSize * (double(i) + offset)
                        + phiSize * 0.5 * double(m_nShifts[layerId]) + m_globalPhiRotation;
    const double phiB = phiSize * (double(i) + offset)   + m_globalPhiRotation;
    const TVector3 f(m_rSLayer[layerId] * cos(phiF), m_rSLayer[layerId] * sin(phiF), m_zSForwardLayer[layerId]);
    const TVector3 b(m_rSLayer[layerId] * cos(phiB), m_rSLayer[layerId] * sin(phiB), m_zSBackwardLayer[layerId]);
    const TVector3 v = f - b;
    const TVector3 u = v.Unit();
    const double beta = (position.z() - b.z()) / u.z();
    const TVector3 p = b + beta * u;
    double dPhi = std::atan2(position.y(), position.x())
                  - std::atan2(p.y(), p.x())
                  + phiSize / 2.;
    while (dPhi < 0) dPhi += (2. * M_PI);
    j = int(dPhi / phiSize);
    while (j >= nWires) j -= nWires;
  }

  return j;
}

void CDCGeometryPar::generateXML(const string& of)
{
  //...Open xml file...
  std::ofstream ofs(of.c_str(), std::ios::out);
  if (! ofs) {
    B2ERROR("CDCGeometryPar::read !!! can not open file : "
            << of);
  }
  ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      << endl
      << "<Subdetector type=\"CDC\">"
      << endl
      << "  <Name>CDC BelleII </Name>"
      << endl
      << "  <Description>CDC geometry parameters</Description>"
      << endl
      << "  <Version>0</Version>"
      << endl
      << "  <GeoCreator>CDCBelleII</GeoCreator>"
      << endl
      << "  <Content>"
      << endl
      << "    <Rotation desc=\"Rotation of the whole cdc detector (should be the same as beampipe)\" unit=\"mrad\">0.0</Rotation>"
      << endl
      << "    <OffsetZ desc=\"The offset of the whole cdc in z with respect to the IP (should be the same as beampipe)\" unit=\"mm\">0.0</OffsetZ>"
      << endl
      << "    <Material>CDCGas</Material>"
      << endl
      << endl;

  ofs << "    <SLayers>" << endl;

  for (int i = 0; i < m_nSLayer; i++) {
    ofs << "      <SLayer id=\"" << i << "\">" << endl;
    ofs << "        <Radius desc=\"Radius of wires in this layer\" unit=\"mm\">" << senseWireR(i) << "</Radius>" << endl;
    ofs << "        <BackwardZ desc=\"z position of this wire layer at backward endplate\" unit=\"mm\">" << senseWireBZ(
          i) << "</BackwardZ>" << endl;
    ofs << "        <ForwardZ desc=\"z position of this wire layer at forward endplate\" unit=\"mm\">" << senseWireFZ(
          i) << "</ForwardZ>" << endl;
//    ofs << "        <BackwardPhi desc=\"azimuth angle of the first wire in this layer at backward endplate\" unit=\"rad\">" << wireBackwardPosition(i).phi() << "</BackwardPhi>" << endl;
//    ofs << "        <ForwardPhi desc=\"azimuth angle of the first wire in this layer at forward endplate\" unit=\"rad\">" << wireForwardPosition(i).phi() << "</ForwardPhi>" << endl;
    ofs << "        <NHoles desc=\"the number of holes in this layer, 2*(cell number)\">" << nWiresInLayer(
          i) * 2 << "</NHoles>" << endl;
    ofs << "        <NShift desc=\"the shifted hole number of each wire in this layer\">" << nShifts(i) << "</NShift>" << endl;
    ofs << "        <Offset desc=\"wire offset in phi direction at endplate\">" << m_offSet[i] << "</Offset>" << endl;
    ofs << "      </SLayer>" << endl;
  }

  ofs << "    </SLayers>" << endl;
  ofs << "    <FLayers>" << endl;

  for (int i = 0; i < m_nFLayer; i++) {
    ofs << "      <FLayer id=\"" << i << "\">" << endl;
    ofs << "        <Radius desc=\"Radius of field wires in this layer\" unit=\"mm\">" << fieldWireR(i) << "</Radius>" << endl;
    ofs << "        <BackwardZ desc=\"z position of this field wire layer at backward endplate\" unit=\"mm\">" << fieldWireBZ(
          i) << "</BackwardZ>" << endl;
    ofs << "        <ForwardZ desc=\"z position of this field wire layer at forward endplate\" unit=\"mm\">" << fieldWireFZ(
          i) << "</ForwardZ>" << endl;
    ofs << "      </FLayer>" << endl;
  }

  ofs << "    </FLayers>" << endl;

  ofs << "    <InnerWall name=\"InnerWall\">" << endl;
  ofs << "      <InnerR desc=\"Inner radius\" unit=\"mm\">" << innerRadiusInnerWall() << "</InnerR>" << endl;
  ofs << "      <OuterR desc=\"Outer radius\" unit=\"mm\">" << outerRadiusInnerWall() << "</OuterR>" << endl;
  ofs << "      <BackwardZ desc=\"z position at backward endplate\" unit=\"mm\">" << m_zWall[0][0] << "</BackwardZ>" << endl;
  ofs << "      <ForwardZ desc=\"z position at forward endplate\" unit=\"mm\">" << m_zWall[0][1] << "</ForwardZ>" << endl;
  ofs << "    </InnerWall>" << endl;

  ofs << "    <OuterWall name=\"OuterWall\">" << endl;
  ofs << "      <InnerR desc=\"Inner radius\" unit=\"mm\">" << innerRadiusOuterWall() << "</InnerR>" << endl;
  ofs << "      <OuterR desc=\"Outer radius\" unit=\"mm\">" << outerRadiusOuterWall() << "</OuterR>" << endl;
  ofs << "      <BackwardZ desc=\"z position at backward endplate\" unit=\"mm\">" << m_zWall[2][0] << "</BackwardZ>" << endl;
  ofs << "      <ForwardZ desc=\"z position at forward endplate\" unit=\"mm\">" << m_zWall[2][1] << "</ForwardZ>" << endl;
  ofs << "    </OuterWall>" << endl;

  ofs << "  </Content>"                                         << endl
      << "</Subdetector>"                                       << endl;
}

void CDCGeometryPar::getWireSagEffect(const EWirePosition set, const unsigned layerID, const unsigned cellID, const double Z,
                                      double& Yb_sag, double& Yf_sag) const
{
  //Input
  //       set    : c_Base, c_Misaligned or c_Aligned
  //       layerID: layer id (0 - 55);
  //       cellID: cell  id in the layer;
  //            Z: Z-coord. (cm) at which sense wire sag is computed.
  //
  //Output Yb_sag: Y-corrd. (cm) of intersection of a tangent and the backward endplate.
  //               Here the tangent is computed from the 1'st derivative of
  //               a paraboric wire (due to gravity) defined at Z.
  //       Yf_sag: ibid. but for forward.
  //
  //N.B.- Maybe replaced with a bit more accurate formula.
  //    - The electrostatic force effect is not included.

  double Xb = 0.;
  double Xf = 0.;
  double Yb = 0.;
  double Yf = 0.;
  double Zb = 0.;
  double Zf = 0.;
  double Coef = 0.;

  if (set == c_Aligned) {
    Coef = m_WireSagCoefAlign[layerID][cellID];
    Yb = m_BWirPosAlign[layerID][cellID][1];
    Yf = m_FWirPosAlign[layerID][cellID][1];
    if (Coef == 0.) {
      Yb_sag = Yb;
      Yf_sag = Yf;
      return;
    }
    Xb = m_BWirPosAlign[layerID][cellID][0];
    Xf = m_FWirPosAlign[layerID][cellID][0];
    Zb = m_BWirPosAlign[layerID][cellID][2];
    Zf = m_FWirPosAlign[layerID][cellID][2];

  } else if (set == c_Misaligned) {
    Coef = m_WireSagCoefMisalign[layerID][cellID];
    Yb = m_BWirPosMisalign[layerID][cellID][1];
    Yf = m_FWirPosMisalign[layerID][cellID][1];
    if (Coef == 0.) {
      Yb_sag = Yb;
      Yf_sag = Yf;
      return;
    }
    Xb = m_BWirPosMisalign[layerID][cellID][0];
    Xf = m_FWirPosMisalign[layerID][cellID][0];
    Zb = m_BWirPosMisalign[layerID][cellID][2];
    Zf = m_FWirPosMisalign[layerID][cellID][2];

  } else if (set == c_Base) {
    Coef = m_WireSagCoef[layerID][cellID];
    Yb = m_BWirPos[layerID][cellID][1];
    Yf = m_FWirPos[layerID][cellID][1];
    if (Coef == 0.) {
      Yb_sag = Yb;
      Yf_sag = Yf;
      return;
    }
    Xb = m_BWirPos[layerID][cellID][0];
    Xf = m_FWirPos[layerID][cellID][0];
    Zb = m_BWirPos[layerID][cellID][2];
    Zf = m_FWirPos[layerID][cellID][2];

  } else {
    B2FATAL("CDCGeometryPar::getWireSagEffect: called with an invalid set: " << " " << set);
  }

  const double dx = Xf - Xb;
  const double dy = Yf - Yb;
  const double dz = Zf - Zb;

  const double Zfp = sqrt(dz * dz + dx * dx); // Wire length in z-x plane since Zbp==0
  const double Zp  = (Z - Zb) * Zfp / dz;

  const double Y_sag = (Coef * (Zp - Zfp) + dy / Zfp) * Zp + Yb;
  const double dydz = (Coef * (2.*Zp - Zfp) * Zfp + dy) / dz;

  Yb_sag = Y_sag + dydz * (Zb - Z);
  Yf_sag = Y_sag + dydz * (Zf - Z);

}

void CDCGeometryPar::setDesignWirParam(const unsigned layerID, const unsigned cellID)
{
  const unsigned L = layerID;
  const unsigned C =  cellID;

  const double offset = m_offSet[L];
  //...Offset modification to be aligned to axial at z=0...
  const double phiSize = 2 * M_PI / double(m_nWires[L]);

  const double phiF = phiSize * (double(C) + offset)
                      + phiSize * 0.5 * double(m_nShifts[L]) + m_globalPhiRotation;

  m_FWirPos[L][C][0] = m_rSLayer[L] * cos(phiF);
  m_FWirPos[L][C][1] = m_rSLayer[L] * sin(phiF);
  m_FWirPos[L][C][2] = m_zSForwardLayer[L];

  const double phiB = phiSize * (double(C) + offset) + m_globalPhiRotation;

  m_BWirPos[L][C][0] = m_rSLayer[L] * cos(phiB);
  m_BWirPos[L][C][1] = m_rSLayer[L] * sin(phiB);
  m_BWirPos[L][C][2] = m_zSBackwardLayer[L];

  for (int i = 0; i < 3; ++i) {
    m_FWirPosMisalign[L][C][i] = m_FWirPos[L][C][i];
    m_BWirPosMisalign[L][C][i] = m_BWirPos[L][C][i];
    m_FWirPosAlign   [L][C][i] = m_FWirPos[L][C][i];
    m_BWirPosAlign   [L][C][i] = m_BWirPos[L][C][i];
  }

  m_WireSagCoef[L][C] = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8. * m_senseWireTension);
  //  m_WireSagCoef        [L][C] = 0.;
  m_WireSagCoefMisalign[L][C] = m_WireSagCoef[L][C];
  m_WireSagCoefAlign    [L][C] = m_WireSagCoef [L][C];

}

void CDCGeometryPar::outputDesignWirParam(const unsigned layerID, const unsigned cellID) const
{

  const unsigned L = layerID;
  const unsigned C =  cellID;

  static bool first = true;
  static ofstream ofs;
  if (first) {
    first = false;
    ofs.open("alignment.dat");
  }

  ofs << L << "  " << C;

  ofs << setiosflags(ios::showpoint | ios::uppercase);

  for (int i = 0; i < 3; ++i) ofs << "  " <<  setw(15) << setprecision(8) << m_BWirPos[L][C][i];

  for (int i = 0; i < 3; ++i) ofs << "  " <<  setw(15) << setprecision(8) << m_FWirPos[L][C][i];
  ofs << setiosflags(ios::fixed);
  ofs << "  " <<  setw(4) << setprecision(1) << m_senseWireTension;

  ofs << endl;
}

double CDCGeometryPar::getDriftV(const double time, const unsigned short iCLayer, const unsigned short lr, const double alpha,
                                 const double theta) const
{
  double dDdt = 0.;

  //convert incoming- to outgoing-lr
  unsigned short lro = getOutgoingLR(lr, alpha);

  if (!m_linearInterpolationOfXT) {
    B2FATAL("linearInterpolationOfXT = false is not allowed now !");
  }
  if (m_linearInterpolationOfXT) {
    double wal(0.);
    unsigned short ial[2] = {0};
    unsigned short ilr[2] = {lro, lro};
    getClosestAlphaPoints(alpha, wal, ial, ilr);
    double wth(0.);
    unsigned short ith[2] = {0};
    getClosestThetaPoints(alpha, theta, wth, ith);

    unsigned short jal(0), jlr(0), jth(0);
    double w = 0.;

    //compute linear interpolation (=weithed average over 4 points) in (alpha-theta) space
    for (unsigned k = 0; k < 4; ++k) {
      if (k == 0) {
        jal = ial[0];
        jlr = ilr[0];
        jth = ith[0];
        w = (1. - wal) * (1. - wth);
      } else if (k == 1) {
        jal = ial[0];
        jlr = ilr[0];
        jth = ith[1];
        w = (1. - wal) * wth;
      } else if (k == 2) {
        jal = ial[1];
        jlr = ilr[1];
        jth = ith[0];
        w = wal * (1. - wth);
      } else if (k == 3) {
        jal = ial[1];
        jlr = ilr[1];
        jth = ith[1];
        w = wal * wth;
      }

      double boundary = m_XT[iCLayer][jlr][jal][jth][6];

      if (time < boundary) {
        dDdt += w * (m_XT[iCLayer][jlr][jal][jth][1] + time
                     * (2.*m_XT[iCLayer][jlr][jal][jth][2] + time
                        * (3.*m_XT[iCLayer][jlr][jal][jth][3] + time
                           * (4.*m_XT[iCLayer][jlr][jal][jth][4] + time
                              * (5.*m_XT[iCLayer][jlr][jal][jth][5])))));
      } else {
        dDdt += w * m_XT[iCLayer][jlr][jal][jth][7];
      }
    }

  } else {
  }

  //replaced with return fabs, since dDdt < 0 rarely; why happens ???
  //  if (lr == 1) dDdt *= -1.;
  //  return dDdt;
  return fabs(dDdt);

}

double CDCGeometryPar::getDriftLength(const double time, const unsigned short iCLayer, const unsigned short lr, const double alpha,
                                      const double theta) const
{
  double dist = 0.;

  //convert incoming- to outgoing-lr
  unsigned short lro = getOutgoingLR(lr, alpha);

  //  std::cout << m_linearInterpolationOfXT << std::endl;
  //  exit(-1);
  if (!m_linearInterpolationOfXT) {
    B2FATAL("linearInterpolationOfXT = false is not allowed now !");
  }
  if (m_linearInterpolationOfXT) {
    double wal(0.);
    unsigned short ial[2] = {0};
    unsigned short ilr[2] = {lro, lro};
    getClosestAlphaPoints(alpha, wal, ial, ilr);
    double wth(0.);
    unsigned short ith[2] = {0};
    getClosestThetaPoints(alpha, theta, wth, ith);

    unsigned short jal(0), jlr(0), jth(0);
    double w = 0.;

    //    std::cout << "iCLayer,alpha,theta,lro= " << iCLayer <<" "<< (180./M_PI)*alpha <<" "<< (180./M_PI)*theta <<" "<< lro << std::endl;

    //compute linear interpolation (=weithed average over 4 points) in (alpha-theta) space
    for (unsigned k = 0; k < 4; ++k) {
      if (k == 0) {
        jal = ial[0];
        jlr = ilr[0];
        jth = ith[0];
        w = (1. - wal) * (1. - wth);
      } else if (k == 1) {
        jal = ial[0];
        jlr = ilr[0];
        jth = ith[1];
        w = (1. - wal) * wth;
      } else if (k == 2) {
        jal = ial[1];
        jlr = ilr[1];
        jth = ith[0];
        w = wal * (1. - wth);
      } else if (k == 3) {
        jal = ial[1];
        jlr = ilr[1];
        jth = ith[1];
        w = wal * wth;
      }

      //      std::cout << "k,w= " << k <<" "<< w << std::endl;
      //      std::cout << "ial[0],[1],jal,wal= " << ial[0] <<" "<< ial[1] <<" "<< jal <<" "<< wal << std::endl;
      //      std::cout << "ith[0],[1],jth,wth= " << ith[0] <<" "<< ith[1] <<" "<< jth <<" "<< wth << std::endl;

      /*
      std::cout <<"iCLayer= " << iCLayer << std::endl;
      std::cout <<"lr= " << lr << std::endl;
      std::cout <<"jal,jth= " << jal <<" "<< jth << std::endl;
      std::cout <<"wal,wth= " << wal <<" "<< wth << std::endl;
      for (int i=0; i<9; ++i) {
      std::cout <<"a= "<< i <<" "<< m_XT[iCLayer][lro][jal][jth][i] << std::endl;
      }
      */
      double boundary = m_XT[iCLayer][jlr][jal][jth][6];

      if (time < boundary) {
        dist += w * (m_XT[iCLayer][jlr][jal][jth][0] + time
                     * (m_XT[iCLayer][jlr][jal][jth][1] + time
                        * (m_XT[iCLayer][jlr][jal][jth][2] + time
                           * (m_XT[iCLayer][jlr][jal][jth][3] + time
                              * (m_XT[iCLayer][jlr][jal][jth][4] + time
                                 * (m_XT[iCLayer][jlr][jal][jth][5]))))));
      } else {
        dist += w * (m_XT[iCLayer][jlr][jal][jth][7] * (time - boundary) + m_XT[iCLayer][jlr][jal][jth][8]);
      }
      //      std::cout <<"k,w,dist= " << k <<" "<< w <<" "<< dist << std::endl;
    }

  } else {
  }

  return fabs(dist);

}

double CDCGeometryPar::getDriftTime(const double dist, const unsigned short iCLayer, const unsigned short lr, const double alpha,
                                    const double theta) const
{
  //to be replaced with a smarter algorithm...

  const double eps = 2.5e-1;
  const double maxTrials = 100;

  //  int ialpha = getAlphaBin(alpha);
  //  int itheta = getThetaBin(theta);

  //convert incoming- to outgoing-lr
  //  unsigned short lrp = getOutgoingLR(lr, alpha);

  double maxTime = 5000.; //in ns
  //  if (m_XT[iCLayer][lrp][ialpha][itheta][7] == 0.) {
  //    maxTime = m_XT[iCLayer][lrp][ialpha][itheta][6];
  //  }

  double t0 = 0.;
  double d0 = getDriftLength(t0, iCLayer, lr, alpha, theta) - dist;

  unsigned i = 0;
  double t1 = maxTime;
  double time = dist * m_nominalDriftVInv;
  while (((t1 - t0) > eps) && (i < maxTrials)) {
    time = 0.5 * (t0 + t1);
    double d1 = getDriftLength(time, iCLayer, lr, alpha, theta) - dist;
    //    std::cout <<"i,dist,t0,t1,d0,d1= " << i <<" "<< dist <<" "<< t0 <<" "<< t1 <<" "<< d0 <<" "<< d1 << std::endl;
    if (d0 * d1 > 0.) {
      t0 = time;
    } else {
      t1 = time;
    }
    ++i;
  }

  if (i >= maxTrials - 1 || time > maxTime) {
    B2WARNING("CDCGeometryPar::getDriftTime " << dist << " " << iCLayer << " " << alpha << " " << lr << " " << t0 << " " << t1 << " " <<
              time << " " << d0);
  }

  //  std::cout <<"dist0,dist1= " <<  dist <<" "<< getDriftLength(time, iCLayer, lr, alpha, theta) <<" "<< getDriftLength(time, iCLayer, lr, alpha, theta) - dist << std::endl;
  //  std::cout <<"dist0,dist1= " <<  dist <<" "<< getDriftLength(time, iCLayer, lr, 0., theta) <<" "<< getDriftLength(time, iCLayer, lr, 0., theta) - dist << std::endl;
  return time;

}

double CDCGeometryPar::getSigma(const double driftL, const unsigned short iCLayer, const unsigned short lr, const double alpha,
                                const double theta) const
{

  double sigma = 0.;

  //convert incoming- to outgoing-lr
  unsigned short lro = getOutgoingLR(lr, alpha);

  if (!m_linearInterpolationOfSgm) {
    B2FATAL("linearInterpolationOfXT = false is not allowed now !");
  }
  if (m_linearInterpolationOfSgm) {
    double wal(0.);
    unsigned short ial[2] = {0};
    unsigned short ilr[2] = {lro, lro};
    getClosestAlphaPoints4Sgm(alpha, wal, ial, ilr);
    double wth(0.);
    unsigned short ith[2] = {0};
    getClosestThetaPoints4Sgm(alpha, theta, wth, ith);

    //compute linear interpolation (=weithed average over 4 points) in (alpha-theta) space
    unsigned short jal(0), jlr(0), jth(0);
    double w = 0.;
    for (unsigned k = 0; k < 4; ++k) {
      if (k == 0) {
        jal = ial[0];
        jlr = ilr[0];
        jth = ith[0];
        w = (1. - wal) * (1. - wth);
      } else if (k == 1) {
        jal = ial[0];
        jlr = ilr[0];
        jth = ith[1];
        w = (1. - wal) * wth;
      } else if (k == 2) {
        jal = ial[1];
        jlr = ilr[1];
        jth = ith[0];
        w = wal * (1. - wth);
      } else if (k == 3) {
        jal = ial[1];
        jlr = ilr[1];
        jth = ith[1];
        w = wal * wth;
      }
      //      std::cout << "k,w= " << k <<" "<< w << std::endl;
      //      std::cout << "ial[0],[1],jal,wal= " << ial[0] <<" "<< ial[1] <<" "<< jal <<" "<< wal << std::endl;
      //      std::cout << "ith[0],[1],jth,wth= " << ith[0] <<" "<< ith[1] <<" "<< jth <<" "<< wth << std::endl;
      /*
      std::cout <<"iCLayer= " << iCLayer << std::endl;
      std::cout <<"lr= " << lr << std::endl;
      std::cout <<"jal,jth= " << jal <<" "<< jth << std::endl;
      std::cout <<"wal,wth= " << wal <<" "<< wth << std::endl;
      for (int i=0; i<9; ++i) {
      std::cout <<"a= "<< i <<" "<< m_XT[iCLayer][lro][jal][jth][i] << std::endl;
      }
      */
      const double& P0 = m_Sigma[iCLayer][jlr][jal][jth][0];
      const double& P1 = m_Sigma[iCLayer][jlr][jal][jth][1];
      const double& P2 = m_Sigma[iCLayer][jlr][jal][jth][2];
      const double& P3 = m_Sigma[iCLayer][jlr][jal][jth][3];
      const double& P4 = m_Sigma[iCLayer][jlr][jal][jth][4];
      const double& P5 = m_Sigma[iCLayer][jlr][jal][jth][5];
      const double& P6 = m_Sigma[iCLayer][jlr][jal][jth][6];

#if defined(CDC_DEBUG)
      cout << "driftL= " << driftL << endl;
      cout << "iCLayer= " << iCLayer << " " << jlr << " " << jal << " " << jth << endl;
      cout << "P0= " << P0 << endl;
      cout << "P1= " << P1 << endl;
      cout << "P2= " << P2 << endl;
      cout << "P3= " << P3 << endl;
      cout << "P4= " << P4 << endl;
      cout << "P5= " << P5 << endl;
      cout << "P6= " << P6 << endl;
#endif

      sigma += w * sqrt(P0 / (driftL * driftL + P1) + P2 * driftL + P3 +
                        P4 * exp(P5 * (driftL - P6) * (driftL - P6)));
    }
  }

  sigma = std::min(sigma, m_maxSpaceResol);
  return sigma;
}

unsigned short CDCGeometryPar::getOldLeftRight(const TVector3& posOnWire, const TVector3& posOnTrack,
                                               const TVector3& momentum) const
{
  unsigned short lr = 0;
  double wCrossT = (posOnWire.Cross(posOnTrack)).z();

  if (wCrossT < 0.) {
    lr = 0;
  } else if (wCrossT > 0.) {
    lr = 1;
  } else {
    if ((posOnTrack - posOnWire).Perp() != 0.) {
      double wCrossP = (posOnWire.Cross(momentum)).z();
      if (wCrossP > 0.) {
        if (posOnTrack.Perp() > posOnWire.Perp()) {
          lr = 0;
        } else {
          lr = 1;
        }
      } else if (wCrossP < 0.) {
        if (posOnTrack.Perp() < posOnWire.Perp()) {
          lr = 0;
        } else {
          lr = 1;
        }
      } else {
        lr = 0;
      }
    } else {
      lr = 0;
    }
  }
  return lr;
}

unsigned short CDCGeometryPar::getNewLeftRightRaw(const TVector3& posOnWire, const TVector3& posOnTrack,
                                                  const TVector3& momentum) const
{
  const double distanceCrossP = ((posOnWire - posOnTrack).Cross(momentum)).z();
  unsigned short int lr = (distanceCrossP > 0.) ? 1 : 0;
  return lr;
}

//N.B. The following alpha and theta calculations are directly implemented in CDCRecoHit.cc tentatively to avoid a circular dependence betw cdc_dataobjects and cdclib. So be careful when changing the calculations !
double CDCGeometryPar::getAlpha(const TVector3& posOnWire, const TVector3& momentum) const
{
  const double wx = posOnWire.x();
  const double wy = posOnWire.y();
  const double px = momentum.x();
  const double py = momentum.y();

  const double cross = wx * py - wy * px;
  const double dot   = wx * px + wy * py;

  return atan2(cross, dot);
}

double CDCGeometryPar::getTheta(const TVector3& momentum) const
{
  return atan2(momentum.Perp(), momentum.z());
}


unsigned short CDCGeometryPar::getOutgoingLR(const unsigned short lr, const double alpha) const
{
  unsigned short lro = (fabs(alpha) <= 0.5 * M_PI) ? lr : abs(lr - 1);
  return lro;
}


double CDCGeometryPar::getOutgoingAlpha(const double alpha) const
{
  //convert incoming- to outgoing-alpha
  double alphao = alpha;
  if (alpha >  0.5 * M_PI) {
    alphao -= M_PI;
  } else if (alpha < -0.5 * M_PI) {
    alphao += M_PI;
  }

  return alphao;
}

double CDCGeometryPar::getOutgoingTheta(const double alpha, const double theta) const
{
  //convert incoming- to outgoing-theta
  double thetao = fabs(alpha) >  0.5 * M_PI  ?  M_PI - theta  :  theta;
  //  std::cout << alpha <<" "<< thetao << std::endl;
  return thetao;
}

void CDCGeometryPar::getClosestAlphaPoints(const double alpha, double& weight, unsigned short points[2],
                                           unsigned short lrs[2]) const
{
  double alphao = getOutgoingAlpha(alpha);
  weight = 1.;

  if (alphao < m_alphaPoints[0]) {
    points[0] = m_nAlphaPoints - 1;
    points[1] = 0;
    if (m_nAlphaPoints > 1) {
      lrs[0] = abs(lrs[0] - 1); //flip lr
      weight = (alphao - (m_alphaPoints[points[0]] - M_PI)) / (m_alphaPoints[points[1]] - (m_alphaPoints[points[0]] - M_PI));
    }
  } else if (m_alphaPoints[m_nAlphaPoints - 1] <= alphao) {
    points[0] = m_nAlphaPoints - 1;
    points[1] = 0;
    if (m_nAlphaPoints > 1) {
      lrs[1] = abs(lrs[1] - 1); //flip lr
      weight = (alphao - m_alphaPoints[points[0]]) / (m_alphaPoints[points[1]] + M_PI - m_alphaPoints[points[0]]);
    }
  } else {
    for (unsigned short i = 0; i <= m_nAlphaPoints - 2; ++i) {
      if (m_alphaPoints[i] <= alphao && alphao < m_alphaPoints[i + 1]) {
        points[0] = i;
        points[1] = i + 1;
        weight = (alphao - m_alphaPoints[points[0]]) / (m_alphaPoints[points[1]] - m_alphaPoints[points[0]]);
        break;
      }
    }
  }
  //  weight = (alphao - m_alphaPoints[points[0]]) / (m_alphaPoints[points[1]] - m_alphaPoints[points[0]]);
}


void CDCGeometryPar::getClosestAlphaPoints4Sgm(const double alpha, double& weight, unsigned short points[2],
                                               unsigned short lrs[2]) const
{
  double alphao = getOutgoingAlpha(alpha);
  weight = 1.;

  if (alphao < m_alphaPoints4Sgm[0]) {
    points[0] = m_nAlphaPoints4Sgm - 1;
    points[1] = 0;
    if (m_nAlphaPoints4Sgm > 1) {
      lrs[0] = abs(lrs[0] - 1); //flip lr
      weight = (alphao - (m_alphaPoints4Sgm[points[0]] - M_PI)) / (m_alphaPoints4Sgm[points[1]] - (m_alphaPoints4Sgm[points[0]] - M_PI));
    }
  } else if (m_alphaPoints4Sgm[m_nAlphaPoints4Sgm - 1] <= alphao) {
    points[0] = m_nAlphaPoints4Sgm - 1;
    points[1] = 0;
    if (m_nAlphaPoints4Sgm > 1) {
      lrs[1] = abs(lrs[1] - 1); //flip lr
      weight = (alphao - m_alphaPoints4Sgm[points[0]]) / (m_alphaPoints4Sgm[points[1]] + M_PI - m_alphaPoints4Sgm[points[0]]);
    }
  } else {
    for (unsigned short i = 0; i <= m_nAlphaPoints4Sgm - 2; ++i) {
      if (m_alphaPoints4Sgm[i] <= alphao && alphao < m_alphaPoints4Sgm[i + 1]) {
        points[0] = i;
        points[1] = i + 1;
        weight = (alphao - m_alphaPoints4Sgm[points[0]]) / (m_alphaPoints4Sgm[points[1]] - m_alphaPoints4Sgm[points[0]]);
        break;
      }
    }
  }
}


void CDCGeometryPar::getClosestThetaPoints(const double alpha, const double theta, double& weight, unsigned short points[2]) const
{
  const double thetao = getOutgoingTheta(alpha, theta);

  if (thetao < m_thetaPoints[0]) {
    //    points[0] = 0;
    //    points[1] = 1;
    points[0] = 0;
    points[1] = 0;
    weight = 1.;
  } else if (m_thetaPoints[m_nThetaPoints - 1] <= thetao) {
    //    points[0] = m_nThetaPoints - 2;
    //    points[1] = m_nThetaPoints - 1;
    points[0] = m_nThetaPoints - 1;
    points[1] = m_nThetaPoints - 1;
    weight = 1.;
  } else {
    for (unsigned short i = 0; i <= m_nThetaPoints - 2; ++i) {
      if (m_thetaPoints[i] <= thetao && thetao < m_thetaPoints[i + 1]) {
        points[0] = i;
        points[1] = i + 1;
        weight = (thetao - m_thetaPoints[points[0]]) / (m_thetaPoints[points[1]] - m_thetaPoints[points[0]]);
        break;
      }
    }
  }
  //  weight = (thetao - m_thetaPoints[points[0]]) / (m_thetaPoints[points[1]] - m_thetaPoints[points[0]]);
}


void CDCGeometryPar::getClosestThetaPoints4Sgm(const double alpha, const double theta, double& weight,
                                               unsigned short points[2]) const
{
  const double thetao = getOutgoingTheta(alpha, theta);

  if (thetao < m_thetaPoints4Sgm[0]) {
    points[0] = 0;
    points[1] = 0;
    weight = 1.;
  } else if (m_thetaPoints4Sgm[m_nThetaPoints4Sgm - 1] <= thetao) {
    points[0] = m_nThetaPoints4Sgm - 1;
    points[1] = m_nThetaPoints4Sgm - 1;
    weight = 1.;
  } else {
    for (unsigned short i = 0; i <= m_nThetaPoints4Sgm - 2; ++i) {
      if (m_thetaPoints4Sgm[i] <= thetao && thetao < m_thetaPoints4Sgm[i + 1]) {
        points[0] = i;
        points[1] = i + 1;
        weight = (thetao - m_thetaPoints4Sgm[points[0]]) / (m_thetaPoints4Sgm[points[1]] - m_thetaPoints4Sgm[points[0]]);
        break;
      }
    }
  }
}


#if defined(CDC_DISPLACEMENT_FROM_DB)
void CDCGeometryPar::setDisplacement()
{
  //    std::cout <<"setDisplacement called" << std::endl;
  for (const auto& disp : m_displacementFromDB) {
    //    const int iLayer0 = disp.getICLayer();
    //    const int iWire0 = disp.getIWire();
    const int iLayer = WireID(disp.getEWire()).getICLayer();
    const int iWire = WireID(disp.getEWire()).getIWire();
    //    if (iLayer0 != iLayer) B2FATAL("Layer0 != Layer");
    //    if (iWire0  != iWire) B2FATAL("Wire0 != Wire");
    m_FWirPos[iLayer][iWire][0] += disp.getXFwd();
    m_FWirPos[iLayer][iWire][1] += disp.getYFwd();
    m_FWirPos[iLayer][iWire][2] += disp.getZFwd();
    m_BWirPos[iLayer][iWire][0] += disp.getXBwd();
    m_BWirPos[iLayer][iWire][1] += disp.getYBwd();
    m_BWirPos[iLayer][iWire][2] += disp.getZBwd();
    m_WireSagCoef[iLayer][iWire] = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.*
                                   (m_senseWireTension + disp.getTension()));
    //    std::cout <<"setdisp iL, iC, nominaltension, tension= " << iLayer <<" " << iWire <<" "<< m_senseWireTension <<" "<< disp.getTension() << std::endl;
  }
}
#endif


//TODO: The following lines upto the file-end are to be moved to CDCSensitiveDetector.cc
const signed short CCW = 1; ///< Constant for counterclockwise orientation
const signed short CW  = -1; ///< Constant for clockwise orientation
const signed short CW_OUT_NEIGHBOR  = 1; //Constant for clockwise outwards
const signed short CW_NEIGHBOR      = 3; //Constant for clockwise
const signed short CW_IN_NEIGHBOR   = 5; // Constant for clockwise inwards
const signed short CCW_IN_NEIGHBOR  = 7; // Constant for counterclockwise inwards
const signed short CCW_NEIGHBOR     = 9; // Constant for counterclockwise
const signed short CCW_OUT_NEIGHBOR = 11; // Constant for counterclockwise outwards

unsigned short CDCGeometryPar::areNeighbors(const WireID& wireId, const WireID& otherWireId) const
{
  //require within the same super-layer
  if (otherWireId.getISuperLayer() != wireId.getISuperLayer()) return 0;

  const signed short iWire       =      wireId.getIWire();
  const signed short iOtherWire  = otherWireId.getIWire();
  const signed short iCLayer     =      wireId.getICLayer();
  const signed short iOtherCLayer = otherWireId.getICLayer();

  //require nearby wire
  if (iWire == iOtherWire) {
  } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_nWires[iOtherCLayer])) {
  } else if ((iWire + 1) % static_cast<signed short>(m_nWires[iCLayer]) == iOtherWire) {
  } else {
    return 0;
  }
  //  std::cout <<"iCLayer,iLayer,nShifts= " << iCLayer <<" "<< iLayer <<" "<< nShifts(iCLayer) << std::endl;

  signed short iLayerDifference = otherWireId.getILayer() - wireId.getILayer();
  if (abs(iLayerDifference) > 1) return 0;

  if (iLayerDifference == 0) {
    if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_nWires[iCLayer])) return CW_NEIGHBOR;
    else if ((iWire + 1) % static_cast<signed short>(m_nWires[iCLayer]) == iOtherWire) return CCW_NEIGHBOR;
    else return 0;
  } else if (iLayerDifference == -1) {
    //    const CCWInfo deltaShift = otherLayer.getShift() - layer.getShift();
    const signed short deltaShift = m_shiftInSuperLayer[otherWireId.getISuperLayer()][otherWireId.getILayer()] -
                                    m_shiftInSuperLayer[wireId.getISuperLayer()][wireId.getILayer()];
    //    std::cout <<"in deltaShift,iOtherWire,iWire= " << deltaShift <<" "<< iOtherWire <<" "<< iWire << std::endl;
    if (iWire == iOtherWire) {
      if (deltaShift ==  CW) return  CW_IN_NEIGHBOR;
      else if (deltaShift == CCW) return CCW_IN_NEIGHBOR;
      else return 0;
    } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_nWires[iOtherCLayer])) {
      if (deltaShift == CCW) return  CW_IN_NEIGHBOR;
      else return 0;
    } else if ((iWire + 1) % static_cast<signed short>(m_nWires[iCLayer]) == iOtherWire) {
      if (deltaShift ==  CW) return CCW_IN_NEIGHBOR;
      else return 0;
    } else return 0;
  } else if (iLayerDifference == 1) {
    //    const CCWInfo deltaShift = otherLayer.getShift() - layer.getShift();
    const signed short deltaShift = m_shiftInSuperLayer[otherWireId.getISuperLayer()][otherWireId.getILayer()] -
                                    m_shiftInSuperLayer[wireId.getISuperLayer()][wireId.getILayer()];
    //    std::cout <<"out deltaShift,iOtherWire,iWire= " << deltaShift <<" "<< iOtherWire <<" "<< iWire << std::endl;
    if (iWire == iOtherWire) {
      if (deltaShift ==  CW) return  CW_OUT_NEIGHBOR;
      else if (deltaShift == CCW) return CCW_OUT_NEIGHBOR;
      else return 0;
    } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_nWires[iOtherCLayer])) {
      if (deltaShift == CCW) return  CW_OUT_NEIGHBOR;
      else return 0;
    } else if ((iWire + 1) % static_cast<signed short>(m_nWires[iCLayer]) == iOtherWire) {
      if (deltaShift ==  CW) return CCW_OUT_NEIGHBOR;
      else return 0;
    } else return 0;
  } else return 0;

}

unsigned short CDCGeometryPar::areNeighbors(unsigned short iCLayer, unsigned short iSuperLayer, unsigned short iLayer,
                                            unsigned short iWire, const WireID& otherWireId) const
{
  //require within the same super-layer
  if (otherWireId.getISuperLayer() != iSuperLayer) return 0;

  const signed short iOtherWire  = otherWireId.getIWire();
  const signed short iOtherCLayer = otherWireId.getICLayer();

  //require nearby wire
  if (iWire == iOtherWire) {
  } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_nWires[iOtherCLayer])) {
  } else if ((iWire + 1) % static_cast<signed short>(m_nWires[iCLayer]) == iOtherWire) {
  } else {
    return 0;
  }

  //  std::cout <<"iCLayer,iLayer,nShifts= " << iCLayer <<" "<< iLayer <<" "<< nShifts(iCLayer) << std::endl;
  signed short iLayerDifference = otherWireId.getILayer() - iLayer;
  if (abs(iLayerDifference) > 1) return 0;

  if (iLayerDifference == 0) {
    if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_nWires[iCLayer])) return CW_NEIGHBOR;
    else if ((iWire + 1) % static_cast<signed short>(m_nWires[iCLayer]) == iOtherWire) return CCW_NEIGHBOR;
    else return 0;
  } else if (iLayerDifference == -1) {
    //    const CCWInfo deltaShift = otherLayer.getShift() - layer.getShift();
    const signed short deltaShift = m_shiftInSuperLayer[otherWireId.getISuperLayer()][otherWireId.getILayer()] -
                                    m_shiftInSuperLayer[iSuperLayer][iLayer];
    //    std::cout <<"in deltaShift,iOtherWire,iWire= " << deltaShift <<" "<< iOtherWire <<" "<< iWire << std::endl;
    if (iWire == iOtherWire) {
      if (deltaShift ==  CW) return  CW_IN_NEIGHBOR;
      else if (deltaShift == CCW) return CCW_IN_NEIGHBOR;
      else return 0;
    } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_nWires[iOtherCLayer])) {
      if (deltaShift == CCW) return  CW_IN_NEIGHBOR;
      else return 0;
    } else if ((iWire + 1) % static_cast<signed short>(m_nWires[iCLayer]) == iOtherWire) {
      if (deltaShift ==  CW) return CCW_IN_NEIGHBOR;
      else return 0;
    } else return 0;
  } else if (iLayerDifference == 1) {
    //    const CCWInfo deltaShift = otherLayer.getShift() - layer.getShift();
    const signed short deltaShift = m_shiftInSuperLayer[otherWireId.getISuperLayer()][otherWireId.getILayer()] -
                                    m_shiftInSuperLayer[iSuperLayer][iLayer];
    //    std::cout <<"out deltaShift,iOtherWire,iWire= " << deltaShift <<" "<< iOtherWire <<" "<< iWire << std::endl;
    if (iWire == iOtherWire) {
      if (deltaShift ==  CW) return  CW_OUT_NEIGHBOR;
      else if (deltaShift == CCW) return CCW_OUT_NEIGHBOR;
      else return 0;
    } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_nWires[iOtherCLayer])) {
      if (deltaShift == CCW) return  CW_OUT_NEIGHBOR;
      else return 0;
    } else if ((iWire + 1) % static_cast<signed short>(m_nWires[iCLayer]) == iOtherWire) {
      if (deltaShift ==  CW) return CCW_OUT_NEIGHBOR;
      else return 0;
    } else return 0;
  } else return 0;

}

void CDCGeometryPar::setShiftInSuperLayer()
{
  const unsigned short nLayers[nSuperLayers] = {8, 6, 6, 6, 6, 6, 6, 6, 6}; //tentaive

  for (unsigned short SLayer = 0; SLayer < nSuperLayers; ++SLayer) {
    unsigned short firstCLayer = 0;
    for (unsigned short i = 0; i < SLayer; ++i) {
      firstCLayer += nLayers[i];
    }
    //    std::cout <<"SLayer,firstCLayer= " << SLayer <<" "<< firstCLayer << std::endl;

    TVector3 firstBPos = wireBackwardPosition(firstCLayer, 0);
    for (unsigned short Layer = 0; Layer < nLayers[SLayer]; ++Layer) {
      unsigned short CLayer = firstCLayer + Layer;

      if (CLayer == firstCLayer) {
        m_shiftInSuperLayer[SLayer][Layer] = 0;

      } else if (CLayer == firstCLayer + 1) {
        TVector3 BPos = wireBackwardPosition(CLayer, 0);
        m_shiftInSuperLayer[SLayer][Layer] = (BPos.Cross(firstBPos)).Z() > 0. ? -1 : 1;
        //  std::cout <<"CLayer,Layer,shift= " << CLayer <<" "<< Layer <<" "<< m_shiftInSuperLayer[SLayer][Layer] <<" "<< (BPos.Cross(firstBPos)).Z() << std::endl;

      } else {
        if (Layer % 2 == 0) {
          m_shiftInSuperLayer[SLayer][Layer] = 0;
        } else {
          m_shiftInSuperLayer[SLayer][Layer] = m_shiftInSuperLayer[SLayer][1];
        }
      }
      //      std::cout <<"CLayer,Layer,shift= " << CLayer <<" "<< Layer <<" "<< m_shiftInSuperLayer[SLayer][Layer] << std::endl;
    }
  }
}

signed short CDCGeometryPar::getShiftInSuperLayer(unsigned short iSuperLayer, unsigned short iLayer) const
{
  return m_shiftInSuperLayer[iSuperLayer][iLayer];
}
