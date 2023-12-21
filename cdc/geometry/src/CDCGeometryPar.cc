/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/CDCGeoControlPar.h>
#include <cdc/simulation/CDCSimControlPar.h>
#include <cdc/utilities/OpenFile.h>

//#include <float.h>

#include <cmath>
#include <boost/format.hpp>
//#include <iostream>
#include <iomanip>

#include <boost/iostreams/filtering_stream.hpp>
//#include <boost/iostreams/device/file.hpp>
//#include <boost/iostreams/filter/gzip.hpp>

#include <Math/ChebyshevPol.h>

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

  CDCGeoControlPar& gcp = CDCGeoControlPar::getInstance();

  if (gcp.getT0InputType()) {
    m_t0FromDB = new DBObjPtr<CDCTimeZeros>;
    if ((*m_t0FromDB).isValid()) {
      (*m_t0FromDB).addCallback(this, &CDCGeometryPar::setT0);
    }
  }

  if (gcp.getBwInputType()) {
    m_badWireFromDB = new DBObjPtr<CDCBadWires>;
    if ((*m_badWireFromDB).isValid()) {
      (*m_badWireFromDB).addCallback(this, &CDCGeometryPar::setBadWire);
    }
  }

  if (gcp.getPropSpeedInputType()) {
    m_propSpeedFromDB = new DBObjPtr<CDCPropSpeeds>;
    if ((*m_propSpeedFromDB).isValid()) {
      (*m_propSpeedFromDB).addCallback(this, &CDCGeometryPar::setPropSpeed);
    }
  }

  if (gcp.getTwInputType()) {
    m_timeWalkFromDB = new DBObjPtr<CDCTimeWalks>;
    if ((*m_timeWalkFromDB).isValid()) {
      (*m_timeWalkFromDB).addCallback(this, &CDCGeometryPar::setTW);
    }
  }

  if (gcp.getXtInputType()) {
    m_xtRelFromDB = new DBObjPtr<CDCXtRelations>;
    if ((*m_xtRelFromDB).isValid()) {
      (*m_xtRelFromDB).addCallback(this, &CDCGeometryPar::setXtRel);
    }
  }

  if (gcp.getSigmaInputType()) {
    m_sResolFromDB = new DBObjPtr<CDCSpaceResols>;
    if ((*m_sResolFromDB).isValid()) {
      (*m_sResolFromDB).addCallback(this, &CDCGeometryPar::setSResol);
    }
  }

  if (gcp.getFFactorInputType()) {
    m_fFactorFromDB = new DBObjPtr<CDCFudgeFactorsForSigma>;
    if ((*m_fFactorFromDB).isValid()) {
      (*m_fFactorFromDB).addCallback(this, &CDCGeometryPar::setFFactor);
    }
  }

  if (gcp.getChMapInputType()) {
    m_chMapFromDB = new DBArray<CDCChannelMap>;
    if ((*m_chMapFromDB).isValid()) {
      (*m_chMapFromDB).addCallback(this, &CDCGeometryPar::setChMap);
    }
  }

  if (gcp.getDisplacementInputType()) {
    m_displacementFromDB = new DBArray<CDCDisplacement>;
    if ((*m_displacementFromDB).isValid()) {
      (*m_displacementFromDB).addCallback(this, &CDCGeometryPar::setDisplacement);
    }
  }

  if (gcp.getAlignmentInputType()) {
    m_alignmentFromDB = new DBObjPtr<CDCAlignment>;
    if ((*m_alignmentFromDB).isValid()) {
      (*m_alignmentFromDB).addCallback(this, &CDCGeometryPar::setWirPosAlignParams);
    }
  }

  if (gcp.getMisalignment()) {
    if (gcp.getMisalignmentInputType()) {
      m_misalignmentFromDB = new DBObjPtr<CDCMisalignment>;
      if ((*m_misalignmentFromDB).isValid()) {
        (*m_misalignmentFromDB).addCallback(this, &CDCGeometryPar::setWirPosMisalignParams);
      }
    }
  }

  //TODO in future: make a new (singleton?) class and move all EDepToADC things there.
  if (gcp.getEDepToADCInputType()) {
    m_eDepToADCConversionsFromDB = new OptionalDBObjPtr<CDCEDepToADCConversions>;
    if ((*m_eDepToADCConversionsFromDB).isValid()) {
      (*m_eDepToADCConversionsFromDB).addCallback(this, &CDCGeometryPar::setEDepToADCConversions);
    }
  }

  clear();
  if (geom) {
    //    B2INFO("CDCGeometryPar: Read Geometry object");
    readFromDB(*geom);
  } else {
    //    std::cout <<"readcalled" << std::endl;
    //    read();
    //    B2FATAL("CDCGeometryPar: Strange that readFromDB is not called !");
    B2WARNING("CDCGeometryPar: Strange that readFromDB is not called! Please make sure that CDC is included in Geometry.");
  }
}

CDCGeometryPar::~CDCGeometryPar()
{
  //  B2INFO("CDCGeometryPar: destructor called");
  //  if (m_t0FromDB)           delete m_t0FromDB;
  //  if (m_badWireFromDB)      delete m_badWireFromDB;
  //  if (m_propSpeedFromDB)    delete m_propSpeedFromDB;
  //  if (m_timeWalkFromDB)     delete m_timeWalkFromDB;
  //  if (m_xtRelFromDB)        delete m_xtRelFromDB;
  //  if (m_sResolFromDB)       delete m_sResolFromDB;
  //  if (m_chMapFromDB)        delete [] m_chMapFromDB;
  //  if (m_displacementFromDB) delete [] m_displacementFromDB;
  //  if (m_chMapFromDB)        delete m_chMapFromDB;
  //  if (m_displacementFromDB) delete m_displacementFromDB;
  //  if (m_alignmentFromDB)    delete m_alignmentFromDB;
  //  if (m_misalignmentFromDB) delete m_misalignmentFromDB;
  //  B2INFO("CDCGeometryPar: destructor ended");
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

  m_tdcOffset         = 0; //not used; to be removed later
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
  for (unsigned i = 0; i < c_maxNSenseLayers; ++i) {
    m_rSLayer[i] = 0;
    m_zSForwardLayer[i] = 0;
    m_dzSForwardLayer[i] = 0;
    m_zSBackwardLayer[i] = 0;
    m_dzSBackwardLayer[i] = 0;
    m_cellSize[i] = 0;
    m_nWires[i] = 0;
    m_offSet[i] = 0;
    m_nShifts[i] = 0;
    m_propSpeedInv[i] = 0.;
  }
  for (unsigned i = 0; i < c_maxNFieldLayers; ++i) {
    m_rFLayer[i] = 0;
    m_zFForwardLayer[i] = 0;
    m_zFBackwardLayer[i] = 0;
  }

  for (unsigned L = 0; L < c_maxNSenseLayers; ++L) {
    for (unsigned C = 0; C < c_maxNDriftCells; ++C) {
      for (unsigned i = 0; i < 3; ++i) {
        m_FWirPos        [L][C][i] = 0.;
        m_BWirPos        [L][C][i] = 0.;
        m_FWirPosMisalign[L][C][i] = 0.;
        m_BWirPosMisalign[L][C][i] = 0.;
        m_FWirPosAlign   [L][C][i] = 0.;
        m_BWirPosAlign   [L][C][i] = 0.;
      }
      for (unsigned i = 0; i < 7; ++i) {
        m_eDepToADCParams[L][C][i] = 0.;
      }
      m_WireSagCoef        [L][C] = 0.;
      m_WireSagCoefMisalign[L][C] = 0.;
      m_WireSagCoefAlign   [L][C] = 0.;
      m_t0                 [L][C] = 0.;
    }
  }

  for (unsigned L = 0; L < c_maxNSenseLayers; ++L) {
    for (unsigned i = 0; i < 2; ++i) {
      for (unsigned alpha = 0; alpha < c_maxNAlphaPoints; ++alpha) {
        for (unsigned theta = 0; theta < c_maxNThetaPoints; ++theta) {
          for (unsigned xtparam = 0; xtparam < c_nXTParams; ++xtparam) {
            m_XT[L][i][alpha][theta][xtparam] = 0.;
          }

          for (unsigned sigmaparam = 0; sigmaparam < c_nSigmaParams; ++sigmaparam) {
            m_Sigma[L][i][alpha][theta][sigmaparam] = 0.;
          }
        }
      }
    }
  }

  for (unsigned board = 0; board < c_nBoards; ++board) {
    for (unsigned i = 0; i < 2; ++i) {
      m_timeWalkCoef[board][i] = 0.;
    }
    for (unsigned channel = 0; channel < 48; ++channel) {
      m_boardAndChannelToWire[board][channel] = 0.;
    }
  }

  for (unsigned superLayer = 0; superLayer < c_nSuperLayers; ++superLayer) {
    for (unsigned layer = 0; layer < 8; ++layer) {
      m_shiftInSuperLayer[superLayer][layer] = 0;
    }
  }

}

void CDCGeometryPar::readFromDB(const CDCGeometry& geom)
{
  m_globalPhiRotation = geom.getGlobalPhiRotation();
  //  m_globalPhiRotation = geom.getGlobalOffsetC();

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
  m_debug = CDCGeoControlPar::getInstance().getDebug();
  m_nSLayer = geom.getNSenseLayers();

  m_materialDefinitionMode = CDCGeoControlPar::getInstance().getMaterialDefinitionMode();
  //  std::cout << m_materialDefinitionMode << std::endl;
  if (m_materialDefinitionMode == 0) {
    B2DEBUG(100, "CDCGeometryPar: Define a mixture of gases and wires in the tracking volume.");
  } else if (m_materialDefinitionMode == 2) {
    //    B2INFO("CDCGeometryPar: Define all sense and field wires explicitly in the tracking volume.");
    B2FATAL("CDCGeometryPar: Materialdefinition=2 is disabled for now.");
  } else {
    B2FATAL("CDCGeometryPar: Materialdefinition mode you specify is invalid.");
  }

  // Get mode for wire z-position
  m_senseWireZposMode = CDCGeoControlPar::getInstance().getSenseWireZposMode();
  //Set z corrections (from input data)
  B2DEBUG(100, "CDCGeometryPar: Sense wire z mode:" << m_senseWireZposMode);

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
    uint layerId = sense.getId();

    m_rSLayer[layerId]          = sense.getR();
    m_zSBackwardLayer[layerId]  = sense.getZbwd();
    m_zSForwardLayer[layerId]   = sense.getZfwd();
    m_nWires[layerId]           = sense.getNWires();
    m_nShifts[layerId]          = sense.getNShifts();
    m_offSet[layerId]           = sense.getOffset();
    m_cellSize[layerId]         = 2 * M_PI * m_rSLayer[layerId] / (double) m_nWires[layerId];
    m_dzSBackwardLayer[layerId] = sense.getDZbwd();
    m_dzSForwardLayer[layerId]  = sense.getDZfwd();

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
    uint layerId = field.getId();

    m_rFLayer[layerId]          = field.getR();
    m_zFBackwardLayer[layerId]  = field.getZbwd();
    m_zFForwardLayer[layerId]   = field.getZfwd();
  }

  // Get sense wire diameter
  m_senseWireDiameter = geom.getSenseDiameter();

  // Get sense wire tension
  m_senseWireTension = geom.getSenseTension();

  //  // Get sense wire density
  m_senseWireDensity = 19.3; // g/cm3  <- tentatively hard-coded here

  // Get field wire diameter
  m_fieldWireDiameter = geom.getFieldDiameter();

  // Get information on the number of (super) layers etc.
  m_nSenseWires           = geom.getNSenseWires();
  m_nFieldWires           = geom.getNFieldWires();
  m_maxNSenseLayers       = geom.getNumberOfSenseLayers();
  m_maxNFieldLayers       = geom.getNumberOfFieldLayers();
  m_maxNSuperLayers       = geom.getMaxNumberOfSuperLayers();
  m_firstLayerOffset      = geom.getOffsetOfFirstLayer();
  m_firstSuperLayerOffset = geom.getOffsetOfFirstSuperLayer();
  m_maxNCellsPerLayer     = geom.getMaxNumberOfCellsPerLayer();

  //Set various quantities (should be moved to CDC.xml later...)
  m_clockFreq4TDC = geom.getClockFrequency();
  if (not m_clockSettings.isValid())
    B2FATAL("HardwareClockSettings payloads are not valid.");
  const double officialClockFreq4TDC = 2 * m_clockSettings->getAcceleratorRF(); // in GHz
  if (abs(m_clockFreq4TDC - officialClockFreq4TDC) / m_clockFreq4TDC > 1.e-4) {
    B2WARNING("ClockFreq4TDC changed from cdclocal " << scientific << setprecision(6) << m_clockFreq4TDC << " to official " <<
              officialClockFreq4TDC << " (GHz) (difference larger than 0.01%)");
    m_clockFreq4TDC = officialClockFreq4TDC;
  }
  B2DEBUG(100, "CDCGeometryPar: Clock freq. for TDC= " << m_clockFreq4TDC << " (GHz).");
  m_tdcBinWidth = 1. / m_clockFreq4TDC;  //in ns
  B2DEBUG(100, "CDCGeometryPar: TDC bin width= " << m_tdcBinWidth << " (ns).");

  m_nominalDriftV    = 4.e-3;  //in cm/ns
  m_nominalDriftVInv = 1. / m_nominalDriftV; //in ns/cm
  m_nominalPropSpeed = 27.25;  //in cm/nsec (Belle's result, provided by iwasaki san)

  m_nominalSpaceResol = geom.getNominalSpaceResolution();
  CDCGeoControlPar& gcp = CDCGeoControlPar::getInstance();

  //Set displacement params. (from input data)
  m_displacement = CDCGeoControlPar::getInstance().getDisplacement();
  B2DEBUG(100, "CDCGeometryPar: Load displacement params. (=1); not load (=0):" << m_displacement);
  if (m_displacement) {
    if (gcp.getDisplacementInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read displacement from DB");
      setDisplacement();
    } else {
      readWirePositionParams(c_Base, &geom);
    }
  }

  //Set alignment params. (from input data)
  m_alignment = CDCGeoControlPar::getInstance().getAlignment();
  B2DEBUG(100, "CDCGeometryPar: Load alignment params. (=1); not load (=0):" <<
          m_alignment);
  if (m_alignment) {
    if (gcp.getAlignmentInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read alignment from DB");
      setWirPosAlignParams();
    } else {
      readWirePositionParams(c_Aligned, &geom);
    }
  }

  //Set misalignment params. (from input data)
  m_misalignment = CDCGeoControlPar::getInstance().getMisalignment();
  B2DEBUG(100, "CDCGeometryPar: Load misalignment params. (=1); not load (=0):" <<
          m_misalignment);
  if (m_misalignment) {
    if (gcp.getMisalignmentInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read misalignment from DB");
      setWirPosMisalignParams();
    } else {
      readWirePositionParams(c_Misaligned, &geom);
    }
  }

  // Get control params. for CDC FullSim
  m_thresholdEnergyDeposit = CDCSimControlPar::getInstance().getThresholdEnergyDeposit();
  m_minTrackLength = CDCSimControlPar::getInstance().getMinTrackLength();
  m_wireSag = CDCSimControlPar::getInstance().getWireSag();
  m_modLeftRightFlag = CDCSimControlPar::getInstance().getModLeftRightFlag();
  if (m_modLeftRightFlag) {
    B2FATAL("ModifiedLeftRightFlag = true is disabled for now; need to update a G4-related code in framework...");
  }
  //N.B. The following two lines are hard-coded since only =1 are used now.
  m_xtFileFormat = 1;
  m_sigmaFileFormat = 1;

  m_XTetc = true;
  if (m_XTetc) {
    if (gcp.getXtInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read xt from DB");
      setXtRel();  //Set xt param. (from DB)
    } else {
      readXT(gbxParams);  //Read xt params. (from file)
    }

    if (gcp.getSigmaInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read sigma from DB");
      setSResol();  //Set sigma param. (from DB)
    } else {
      readSigma(gbxParams);  //Read sigma params. (from file)
    }

    if (gcp.getFFactorInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read fudge factors from DB");
      setFFactor();  //Set fudge factors (from DB)
    } else {
      readFFactor(gbxParams);  //Read fudge factors (from file)
    }

    if (gcp.getPropSpeedInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read prop-speed from DB");
      setPropSpeed();  //Set prop-speed (from DB)
    } else {
      readPropSpeed(gbxParams);  //Read propagation speed
    }

    if (gcp.getT0InputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read t0 from DB");
      setT0();  //Set t0 (from DB)
    } else {
      readT0(gbxParams);  //Read t0 (from file)
    }

    if (gcp.getBwInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read badwire from DB");
      setBadWire();  //Set bad-wire (from DB)
    } else {
      //      readBadWire(gbxParams);  //Read bad-wire (from file)
      B2FATAL("Text file input mode for bdwires is disabled now!");
    }

    if (gcp.getChMapInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read ch-map from DB");
      setChMap();  //Set ch-map (from DB)
    } else {
      readChMap();  //Read ch-map
    }

    if (gcp.getTwInputType()) {
      B2DEBUG(100, "CDCGeometryPar: Read time-walk from DB");
      setTW();  //Set time-walk coeffs. (from DB)
    } else {
      readTW(gbxParams);  //Read time-walk coeffs. (from file)
    }
    B2DEBUG(100, "CDCGeometryPar: Time-walk param. mode= " << m_twParamMode);

    if (gcp.getEDepToADCInputType()) {
      B2DEBUG(29, "CDCGeometryPar: Read EDepToADC from DB");
      if ((*m_eDepToADCConversionsFromDB).isValid()) {
        setEDepToADCConversions(); //Set edep-to-adc (from DB)
      }
    } else {
      readEDepToADC(gbxParams);  //Read edep-to-adc params. (from file)
    }
  }

  m_XTetc4Recon = 0;
  //  B2INFO("CDCGeometryPar: Load x-t etc. params. for reconstruction (=1); not load and use the same ones for digitization (=0):" <<
  //  B2INFO("CDCGeometryPar: Use the same x-t etc. for reconstruction as those used for digitization");
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

/*
//TODO: move the following two functions to cdc/utilities <-done
// Open a file
void CDCGeometryPar::openFile(std::ifstream& ifs, const std::string& fileName0) const
{
  std::string fileName1 = "/data/cdc/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName1);

  if (fileName == "") {
    fileName = FileSystem::findFile(fileName0);
  }

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName1 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: open " << fileName1);
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName1 << " !");
  }
}
// Open a file using boost (to be able to read a gzipped file)
void CDCGeometryPar::openFile(boost::iostreams::filtering_istream& ifs, const std::string& fileName0) const
{
  std::string fileName1 = "/data/cdc/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName1);

  if (fileName == "") {
    fileName = FileSystem::findFile(fileName0);
  }

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName1 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: open " << fileName1);
    if ((fileName.rfind(".gz") != string::npos) && (fileName.length() - fileName.rfind(".gz") == 3)) {
      ifs.push(boost::iostreams::gzip_decompressor());
    }
    ifs.push(boost::iostreams::file_source(fileName));
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName1 << " !");
  }
}
*/

// Read displacement or (mis)alignment params.
//void CDCGeometryPar::readWirePositionParams(EWirePosition set,  const CDCGeometry* geom,  const GearDir gbxParams)
void CDCGeometryPar::readWirePositionParams(EWirePosition set,  const CDCGeometry* geom)
{

  std::string fileName0;
  CDCGeoControlPar& gcp = CDCGeoControlPar::getInstance();
  if (geom) {
    if (set == c_Base) {
      fileName0 = gcp.getDisplacementFile();
    } else if (set == c_Misaligned) {
      fileName0 = gcp.getMisalignmentFile();
    } else if (set == c_Aligned) {
      fileName0 = gcp.getAlignmentFile();
    }
  } else {
    if (set == c_Base) {
      fileName0 = gcp.getDisplacementFile();
    } else if (set == c_Misaligned) {
      fileName0 = gcp.getMisalignmentFile();
    } else if (set == c_Aligned) {
      fileName0 = gcp.getAlignmentFile();
    }
  }

  //  ifstream ifs;
  //  openFile(ifs, fileName0);
  boost::iostreams::filtering_istream ifs;
  openFileB(ifs, fileName0);

  uint iL(0), iC(0);
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

    if (iL < m_firstLayerOffset) {
      continue;
    }

    ++nRead;

    for (int i = 0; i < np; ++i) {
      if (set == c_Base) {
        m_BWirPos[iL][iC][i] += (iL < m_firstLayerOffset) ? 0 : back[i];
        m_FWirPos[iL][iC][i] += (iL < m_firstLayerOffset) ? 0 : fwrd[i];
      } else if (set == c_Misaligned) {
        m_BWirPosMisalign[iL][iC][i] = m_BWirPos[iL][iC][i] + ((iL < m_firstLayerOffset) ? 0 : back[i]);
        m_FWirPosMisalign[iL][iC][i] = m_FWirPos[iL][iC][i] + ((iL < m_firstLayerOffset) ? 0 : fwrd[i]);
      } else if (set == c_Aligned) {
        m_BWirPosAlign[iL][iC][i] = m_BWirPos[iL][iC][i] + ((iL < m_firstLayerOffset) ? 0 : back[i]);
        m_FWirPosAlign[iL][iC][i] = m_FWirPos[iL][iC][i] + ((iL < m_firstLayerOffset) ? 0 : fwrd[i]);
      }
    }

    //    double baseTension = 0.;

    if (set == c_Base) {
      m_WireSagCoef[iL][iC] = (iL < m_firstLayerOffset) ? 0 : M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter /
                              (8.*(m_senseWireTension + tension));
      //      std::cout <<"base iL, iC, m_senseWireTension, tension= " << iL <<" " << iC <<" "<< m_senseWireTension <<" "<< tension << std::endl;
    } else if (set == c_Misaligned) {
      double baseTension = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.* m_WireSagCoef[iL][iC]);
      m_WireSagCoefMisalign[iL][iC] = (iL < m_firstLayerOffset) ? 0 : M_PI * m_senseWireDensity * m_senseWireDiameter *
                                      m_senseWireDiameter / (8.* (baseTension + tension));
      //      std::cout <<"misa iL, iC,basetension, tension= " << iL <<" " << iC <<" "<< baseTension <<" "<< tension << std::endl;
    } else if (set == c_Aligned) {
      double baseTension = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.* m_WireSagCoef[iL][iC]);
      m_WireSagCoefAlign[iL][iC] = (iL < m_firstLayerOffset) ? 0 : M_PI * m_senseWireDensity * m_senseWireDiameter *
                                   m_senseWireDiameter / (8.*(baseTension + tension));
      //      std::cout <<"algn iL, iC,basetension, tension= " << iL <<" " << iC <<" "<< baseTension <<" "<< tension << std::endl;
    }
    //    std::cout << "baseTension,tension= " << baseTension <<" "<< tension << std::endl;

    /*
    if (m_debug) {
      std::cout << iL << " " << iC;
      for (int i = 0; i < np; ++i) cout << " " << back[i];
      for (int i = 0; i < np; ++i) cout << " " << fwrd[i];
      std::cout << " " << tension << std::endl;
    }
    */

  }

  if (nRead != m_nSenseWires) B2FATAL("CDCGeometryPar::readWirePositionParams: #lines read-in (=" << nRead <<
                                        ") is inconsistent with total #sense wires (=" << m_nSenseWires << ") !");

  //  ifs.close();
  boost::iostreams::close(ifs);
}


// Set alignment wire positions
void CDCGeometryPar::setWirPosAlignParams()
{
  // Layer alignment
  for (unsigned iL = 0; iL < c_maxNSenseLayers; ++iL) {

    if (iL < m_firstLayerOffset) {
      continue;
    }

    // wire number 511 = no wire
    auto layerID = WireID(iL, 511);

    // Alignment parameters for layer iL
    double d_layerXbwd = (*m_alignmentFromDB)->get(layerID, CDCAlignment::layerX);
    double d_layerYbwd = (*m_alignmentFromDB)->get(layerID, CDCAlignment::layerY);
    double d_layerPhiBwd = (*m_alignmentFromDB)->get(layerID, CDCAlignment::layerPhi);

    double d_layerXfwd = (*m_alignmentFromDB)->get(layerID, CDCAlignment::layerDx) + d_layerXbwd;
    double d_layerYfwd = (*m_alignmentFromDB)->get(layerID, CDCAlignment::layerDy) + d_layerYbwd;
    double d_layerPhiFwd = (*m_alignmentFromDB)->get(layerID, CDCAlignment::layerDPhi) + d_layerPhiBwd;

    for (unsigned iC = 0; iC < m_nWires[iL]; ++iC) {
      // Positions (nominal+displacement) of wire-ends of wire iC in layer iL
      double wireXbwd = m_BWirPos[iL][iC][0];
      double wireYbwd = m_BWirPos[iL][iC][1];
      double wireZbwd = m_BWirPos[iL][iC][2];

      double wireXfwd = m_FWirPos[iL][iC][0];
      double wireYfwd = m_FWirPos[iL][iC][1];
      double wireZfwd = m_FWirPos[iL][iC][2];

      // Aligned positions of wire-ends are obtained by rotating "nominal+displacement" positions and shifting them using
      // common parameters for layer rotation and shifts (at corresponding end-caps)
      m_BWirPosAlign[iL][iC][0] = d_layerXbwd + cos(d_layerPhiBwd) * wireXbwd + sin(d_layerPhiBwd) * wireYbwd;
      m_BWirPosAlign[iL][iC][1] = d_layerYbwd - sin(d_layerPhiBwd) * wireXbwd + cos(d_layerPhiBwd) * wireYbwd;
      m_BWirPosAlign[iL][iC][2] = wireZbwd;

      m_FWirPosAlign[iL][iC][0] = d_layerXfwd + cos(d_layerPhiFwd) * wireXfwd + sin(d_layerPhiFwd) * wireYfwd;
      m_FWirPosAlign[iL][iC][1] = d_layerYfwd - sin(d_layerPhiFwd) * wireXfwd + cos(d_layerPhiFwd) * wireYfwd;
      m_FWirPosAlign[iL][iC][2] = wireZfwd;
    } //end of  cell loop
  } //end of layer loop

  const int np = 3;
  double back[np], fwrd[np];

  for (unsigned iL = 0; iL < c_maxNSenseLayers; ++iL) {

    if (iL < m_firstLayerOffset) {
      continue;
    }

    for (unsigned iC = 0; iC < m_nWires[iL]; ++iC) {
      //      std::cout << "iLiC= " << iL <<" "<< iC << std::endl;
      WireID wire(iL, iC);
      back[0] = (*m_alignmentFromDB)->get(wire, CDCAlignment::wireBwdX);
      back[1] = (*m_alignmentFromDB)->get(wire, CDCAlignment::wireBwdY);
      back[2] = (*m_alignmentFromDB)->get(wire, CDCAlignment::wireBwdZ);

      fwrd[0] = (*m_alignmentFromDB)->get(wire, CDCAlignment::wireFwdX);
      fwrd[1] = (*m_alignmentFromDB)->get(wire, CDCAlignment::wireFwdY);
      fwrd[2] = (*m_alignmentFromDB)->get(wire, CDCAlignment::wireFwdZ);

      for (int i = 0; i < np; ++i) {
        // On top of the wire-end positions corrected for layer alignment, we apply possible
        // fine corrections per wire
        m_BWirPosAlign[iL][iC][i] += back[i];
        m_FWirPosAlign[iL][iC][i] += fwrd[i];
      }

      //      double baseTension = 0.;
      double baseTension = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.* m_WireSagCoef[iL][iC]);
      double tension = (*m_alignmentFromDB)->get(wire, CDCAlignment::wireTension);
      //      std::cout << back[0] <<" "<< back[1] <<" "<< back[2] <<" "<< fwrd[0] <<" "<< fwrd[1] <<" "<< fwrd[2] <<" "<< tension << std::endl;
      m_WireSagCoefAlign[iL][iC] = M_PI * m_senseWireDensity *
                                   m_senseWireDiameter * m_senseWireDiameter / (8.*(baseTension + tension));
      //    std::cout << "baseTension,tension= " << baseTension <<" "<< tension << std::endl;
    } //end of  layer loop
  } //end of cell loop
}


// Set misalignment wire positions
//TODO: merge this and setWirPosAlignParam() somehow
void CDCGeometryPar::setWirPosMisalignParams()
{
  const int np = 3;
  double back[np], fwrd[np];

  for (unsigned iL = 0; iL < c_maxNSenseLayers; ++iL) {

    if (iL < m_firstLayerOffset) {
      continue;
    }

    for (unsigned iC = 0; iC < m_nWires[iL]; ++iC) {
      //      std::cout << "iLiC= " << iL <<" "<< iC << std::endl;
      WireID wire(iL, iC);
      back[0] = (*m_misalignmentFromDB)->get(wire, CDCMisalignment::wireBwdX);
      back[1] = (*m_misalignmentFromDB)->get(wire, CDCMisalignment::wireBwdY);
      back[2] = (*m_misalignmentFromDB)->get(wire, CDCMisalignment::wireBwdZ);

      fwrd[0] = (*m_misalignmentFromDB)->get(wire, CDCMisalignment::wireFwdX);
      fwrd[1] = (*m_misalignmentFromDB)->get(wire, CDCMisalignment::wireFwdY);
      fwrd[2] = (*m_misalignmentFromDB)->get(wire, CDCMisalignment::wireFwdZ);

      for (int i = 0; i < np; ++i) {
        m_BWirPosMisalign[iL][iC][i] = m_BWirPos[iL][iC][i] + back[i];
        m_FWirPosMisalign[iL][iC][i] = m_FWirPos[iL][iC][i] + fwrd[i];
      }

      //      double baseTension = 0.;
      double baseTension = M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter / (8.* m_WireSagCoef[iL][iC]);
      double tension = (*m_misalignmentFromDB)->get(wire, CDCMisalignment::wireTension);
      //      std::cout << back[0] <<" "<< back[1] <<" "<< back[2] <<" "<< fwrd[0] <<" "<< fwrd[1] <<" "<< fwrd[2] <<" "<< tension << std::endl;
      m_WireSagCoefMisalign[iL][iC] = M_PI * m_senseWireDensity *
                                      m_senseWireDiameter * m_senseWireDiameter / (8.*(baseTension + tension));
      //    std::cout << "baseTension,tension= " << baseTension <<" "<< tension << std::endl;
    } //end of  layer loop
  } //end of cell loop
}


// Read x-t params.
void CDCGeometryPar::readXT(const GearDir& gbxParams, const int mode)
{
  if (m_xtFileFormat == 0) {
    //    oldReadXT(gbxParams, mode);
  } else {
    newReadXT(gbxParams, mode);
  }
}


// Read x-t params. (new)
void CDCGeometryPar::newReadXT(const GearDir& gbxParams, const int mode)
{
  m_linearInterpolationOfXT = true;  //must be true now

  std::string fileName0 = CDCGeoControlPar::getInstance().getXtFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("xt4ReconFileName");
  }

  boost::iostreams::filtering_istream ifs;
  openFileB(ifs, fileName0);
  //TODO: use openFile() in cdc/utilities instead of the following 18 lines <- done
  /*
  std::string fileName1 = "/data/cdc/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName1);

  if (fileName == "") {
    fileName = FileSystem::findFile(fileName0);
  }

  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName1 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: open " << fileName1);
    if ((fileName.rfind(".gz") != string::npos) && (fileName.length() - fileName.rfind(".gz") == 3)) {
      ifs.push(boost::iostreams::gzip_decompressor());
    }
    ifs.push(boost::iostreams::file_source(fileName));
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName1 << " !");
  }
  */

  //read alpha bin info.
  unsigned short nAlphaBins = 0;
  if (ifs >> nAlphaBins) {
    if (nAlphaBins == 0 || nAlphaBins > c_maxNAlphaPoints) B2FATAL("Fail to read alpha bins !");
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
    if (nThetaBins == 0 || nThetaBins > c_maxNThetaPoints) B2FATAL("CDCGeometryPar: fail to read theta bins !");
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
  const unsigned short npx = c_nXTParams - 1;
  double xtc[npx];
  double theta, alpha, dummy1;

  ifs >> m_xtParamMode >> np;
  if (m_xtParamMode < 0 || m_xtParamMode > 3) B2FATAL("CDCGeometryPar: invalid xt-parameterization mode read !");

  if (np <= 0 || np > npx) B2FATAL("CDCGeometryPar: no. of xt-params. outside limits !");

  const double epsi = 0.1;

  while (ifs >> iCL) {

    if (iCL < m_firstLayerOffset) {
      continue;
    }

    ifs >> theta >> alpha >> dummy1 >> iLR;
    for (int i = 0; i < np; ++i) {
      ifs >> xtc[i];
    }

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

    double boundT = xtc[6];
    if (m_xtParamMode == 1) {
      m_XT[iCL][iLR][ialpha][itheta][np] = ROOT::Math::Chebyshev5(boundT, xtc[0], xtc[1], xtc[2], xtc[3], xtc[4], xtc[5]);
    } else {
      m_XT[iCL][iLR][ialpha][itheta][np] =
        xtc[0] + boundT
        * (xtc[1] + boundT
           * (xtc[2] + boundT
              * (xtc[3] + boundT
                 * (xtc[4] + boundT
                    * (xtc[5])))));
    }
  }  //end of while loop

  //  ifs.close();
  boost::iostreams::close(ifs);

  //convert unit
  const double degrad = M_PI / 180.;
  for (unsigned i = 0; i < nAlphaBins; ++i) {
    m_alphaPoints[i] *= degrad;
  }
  for (unsigned i = 0; i < nThetaBins; ++i) {
    m_thetaPoints[i] *= degrad;
  }

}


// Read space resol. params.
void CDCGeometryPar::readSigma(const GearDir& gbxParams, const int mode)
{
  if (m_sigmaFileFormat == 0) {
    //    oldReadSigma(gbxParams, mode);
  } else {
    newReadSigma(gbxParams, mode);
  }
}

void CDCGeometryPar::newReadSigma(const GearDir& gbxParams, const int mode)
{
  m_linearInterpolationOfSgm = true; //must be true now

  std::string fileName0 = CDCGeoControlPar::getInstance().getSigmaFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("sigma4ReconFileName");
  }

  ifstream ifs;
  //  openFile(ifs, fileName0);
  openFileA(ifs, fileName0);

  //read alpha bin info.
  unsigned short nAlphaBins = 0;
  if (ifs >> nAlphaBins) {
    if (nAlphaBins == 0 || nAlphaBins > c_maxNAlphaPoints) B2FATAL("Fail to read alpha bins !");
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
    if (nThetaBins == 0 || nThetaBins > c_maxNThetaPoints) B2FATAL("CDCGeometryPar: fail to read theta bins !");
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
  double sigma[c_nSigmaParams]; // cppcheck-suppress constVariable
  double theta, alpha;

  ifs >> m_sigmaParamMode >> np;
  //  std:: cout << m_sigmaParamMode <<" "<< np << std::endl;
  if (m_sigmaParamMode < 0 || m_sigmaParamMode > 4) B2FATAL("CDCGeometryPar: invalid sigma-parameterization mode read !");

  if (np > c_nSigmaParams) B2FATAL("CDCGeometryPar: no. of sigma-params. outside limits !");

  ifs >> m_maxSpaceResol;

  const double epsi = 0.1;

  while (ifs >> iCL) {

    if (iCL < m_firstLayerOffset) {
      continue;
    }

    ifs >> theta >> alpha >> iLR;
    //    std::cout << iCL <<" "<< theta <<" "<< alpha <<" "<< iLR << std::endl;
    for (int i = 0; i < np; ++i) {
      ifs >> sigma[i];
    }

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

  ifs.close();

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


// Read fudge factors
void CDCGeometryPar::readFFactor(const GearDir& gbxParams, const int mode)
{
  std::string fileName0 = CDCGeoControlPar::getInstance().getFFactorFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("fudgeFactorFileName");
  }
  B2WARNING("readFFactor is not ready! " << fileName0);
  //TODO; implement the following part.
}


// Read propagation speed param.
void CDCGeometryPar::readPropSpeed(const GearDir& gbxParams, const int mode)
{
  std::string fileName0 = CDCGeoControlPar::getInstance().getPropSpeedFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("propSpeed4ReconFileName");
  }

  ifstream ifs;
  //  openFile(ifs, fileName0);
  openFileA(ifs, fileName0);

  uint iL;
  double speed;
  unsigned nRead = 0;

  while (true) {
    ifs >> iL >> speed;
    if (ifs.eof()) break;

    ++nRead;

    m_propSpeedInv[iL] = (iL < m_firstLayerOffset) ? 0 : 1. / speed;

    if (m_debug) B2DEBUG(150, iL << " " << speed);
  }

  if (nRead != c_maxNSenseLayers) B2FATAL("CDCGeometryPar::readPropSpeed: #lines read-in (=" << nRead <<
                                            ") is inconsistent with total #layers (=" << c_maxNSenseLayers << ") !");

  ifs.close();
}

/*
// Read deltaz params.
void CDCGeometryPar::readDeltaz(const GearDir gbxParams)
{
  std::string fileName0 = gbxParams.getString("deltazFileName");
  fileName0 = "/data/cdc/" + fileName0;
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

  if (nRead != c_maxNSenseLayers) B2FATAL("CDCGeometryPar::readDeltaz: #lines read-in (=" << nRead <<
                                        ") is inconsistent with total #layers (=" << c_maxNSenseLayers << ") !");

  ifs.close();
}
*/


// Read t0 params.
void CDCGeometryPar::readT0(const GearDir& gbxParams, int mode)
{
  std::string fileName0 = CDCGeoControlPar::getInstance().getT0File();
  if (mode == 1) {
    fileName0 = gbxParams.getString("t04ReconFileName");
  }

  ifstream ifs;
  //  openFile(ifs, fileName0);
  openFileA(ifs, fileName0);

  uint iL(0), iC(0);
  float t0(0);
  unsigned nRead = 0;

  while (true) {
    ifs >> iL >> iC >> t0;

    if (iL < m_firstLayerOffset) {
      continue;
    }

    if (ifs.eof()) break;

    ++nRead;

    m_t0[iL][iC] = (iL < m_firstLayerOffset) ? 0. : t0;

    if (m_debug) {
      B2DEBUG(150, iL << " " << iC << " " << t0);
    }
  }

  if (nRead != m_nSenseWires) B2FATAL("CDCGeometryPar::readT0: #lines read-in (=" << nRead <<
                                        ") is inconsistent with total #sense wires (=" << m_nSenseWires << ") !");

  ifs.close();

  calcMeanT0();
}


/*
// Read bad-wires.
void CDCGeometryPar::readBadWire(const GearDir gbxParams, int mode)
{
  std::string fileName0 = CDCGeoControlPar::getInstance().getBwFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("bw4ReconFileName");
  }

  ifstream ifs;
  //  openFile(ifs, fileName0);
  openFileA(ifs, fileName0);

  int iCL(0), iW(0);
  unsigned nRead = 0;

  while (true) {
    ifs >> iCL >> iW;

    if (ifs.eof()) break;

    ++nRead;

    m_badWire.push_back(WireID(iCL, iW));

    if (m_debug) {
      B2DEBUG(150, iCL << " " << iW);
    }
  }

  if (nRead > m_nSenseWires) B2FATAL("CDCGeometryPar::readBadWire: #lines read-in (=" << nRead <<
                                     ") is larger than the total #sense wires (=" << m_nSenseWires << ") !");

  ifs.close();
}
*/


// Read time-walk parameters
void CDCGeometryPar::readTW(const GearDir& gbxParams, const int mode)
{
  std::string fileName0 = CDCGeoControlPar::getInstance().getTwFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("tw4ReconFileName");
  }

  ifstream ifs;
  //  openFile(ifs, fileName0);
  openFileA(ifs, fileName0);

  unsigned short nPars(0);
  ifs >> m_twParamMode >> nPars;
  if (m_twParamMode > 1) {
    B2FATAL("CDCGeometryPar::readTW: invalid mode specified!");
  }
  if (nPars > 2) {
    B2FATAL("CDCGeometryPar::readTW: invalid #params specified!");
  }

  unsigned iBoard = 0;
  unsigned nRead = 0;
  // Read board id and coefficients
  while (ifs >> iBoard) {
    for (unsigned short i = 0; i < nPars; ++i) {
      ifs >> m_timeWalkCoef[iBoard][i];
    }
    ++nRead;
  }

  if (nRead != c_nBoards) B2FATAL("CDCGeometryPar::readTW: #lines read-in (=" << nRead << ") is inconsistent with #boards (=" <<
                                    c_nBoards
                                    << ") !");

  ifs.close();
}


// Read ch-map
//void CDCGeometryPar::readChMap(const GearDir gbxParams)
void CDCGeometryPar::readChMap()
{
  std::string fileName0 = CDCGeoControlPar::getInstance().getChMapFile();

  ifstream ifs;
  //  openFile(ifs, fileName0);
  openFileA(ifs, fileName0);

  unsigned short iSL, iL, iW, iB, iC;
  unsigned nRead = 0;

  while (true) {
    // Read a relation
    ifs >> iSL >> iL >> iW >> iB >> iC;
    if (ifs.eof()) break;
    if (iSL >= c_nSuperLayers or iSL < m_firstSuperLayerOffset) continue;

    ++nRead;
    WireID wID(iSL, iL, iW);
    m_wireToBoard.insert(pair<WireID, unsigned short>(wID, iB));
  }

  if (nRead != m_nSenseWires) B2FATAL("CDCGeometryPar::readChMap: #lines read-in (=" << nRead <<
                                        ") is inconsistent with #sense-wires (="
                                        << m_nSenseWires << ") !");

  ifs.close();
}


// Read edep-to-adc
void CDCGeometryPar::readEDepToADC(const GearDir& gbxParams, const int mode)
{
  //  B2WARNING("CDCGeometryPar: readEDepToADC is not ready!");
  std::string fileName0 = CDCGeoControlPar::getInstance().getEDepToADCFile();
  if (mode == 1) {
    fileName0 = gbxParams.getString("fudgeFactorFileName");
  }

  ifstream ifs;
  //  openFileA(ifs, fileName0);
  std::string fileName1 = "/data/cdc/" + fileName0;
  std::string fileName = FileSystem::findFile(fileName1, true);

  if (fileName == "") {
    fileName = FileSystem::findFile(fileName0, true);
  }

  if (fileName == "") {
    B2FATAL("CDC::openFile: " << fileName0 << " not exist!");
  } else {
    //    B2INFO("CDC::openFile: open " << fileName);
    B2DEBUG(29, "CDC::openFile: open " << fileName);
    ifs.open(fileName.c_str());
    if (!ifs) B2FATAL("CDC::openFile: cannot open " << fileName << " !");
  }

  unsigned short paramMode(0), nParams(0);
  ifs >> paramMode >> nParams;
  if (paramMode > 1) B2FATAL("Param mode > 1!");
  if (nParams > 7)   B2FATAL("No. of params. > 7!");
  unsigned short groupId(0);
  ifs >> groupId;
  B2DEBUG(29, paramMode << " " << nParams << " " << groupId);
  if (groupId > 0) B2FATAL("GgroupId > 0!");


  unsigned short cLMin[c_nSuperLayers], cLMax[c_nSuperLayers]; //min and max clayer per super-layer
  cLMin[0] = 0;
  cLMax[0] = 7;
  for (unsigned int sl = 1; sl < c_nSuperLayers; ++sl) {
    cLMin[sl] = cLMax[0] + 6 * sl - 5;
    cLMax[sl] = cLMax[0] + 6 * sl;
  }

  unsigned short id = 0;
  double coef = 0.;
  unsigned short nRead = 0;
  while (ifs >> id) {
    for (unsigned short i = 0; i < nParams; ++i) {
      ifs  >> coef;
      for (unsigned short cL = cLMin[id]; cL <= cLMax[id]; ++cL) { //clayer loop
        for (unsigned short cell = 0; cell < m_nWires[cL]; ++cell) { //cell loop
          m_eDepToADCParams[cL][cell][i] = (cL < m_firstLayerOffset) ? 0 : coef;
          //    B2DEBUG(29, "cL,cell,i,coef= "<< cL <<" "<< cell <<" "<< i <<" "<< coef);
        }
      }
    }
    ++nRead;
    if (nRead > c_nSuperLayers) B2FATAL("No. of read in lines > " << c_nSuperLayers << " !");
  }

  ifs.close();
}


// Set t0 (from DB)
void CDCGeometryPar::setT0()
{
  for (unsigned short iCL = 0; iCL < c_maxNSenseLayers; ++iCL) {
    for (unsigned short iW = 0; iW < c_maxNDriftCells; ++iW) {
      m_t0[iCL][iW] = 0.;
    }
  }

  for (auto const& ent : (*m_t0FromDB)->getT0s()) {
    const WireID wid = WireID(ent.first);
    const unsigned short iCL = wid.getICLayer();
    const unsigned short iW  = wid.getIWire();
    m_t0[iCL][iW]            = (iCL < m_firstLayerOffset) ? 0. : ent.second;
  }

  calcMeanT0();
}


// Calculate mean t0
void CDCGeometryPar::calcMeanT0(double minT0, double maxT0, int maxIt, double nStdv, double epsi)
{
  double oldMeanT0 = 0;
  unsigned short it1 = 0;
  for (unsigned short it = 0; it < maxIt; ++it) {
    it1 = it;
    double effiSum = 0.;
    m_meanT0 = 0.;
    double stdvT0 = 0;
    for (unsigned short iCL = 0; iCL < c_maxNSenseLayers; ++iCL) {
      for (unsigned short iW = 0; iW < m_nWires[iCL]; ++iW) {
        if (m_t0[iCL][iW] < minT0 || m_t0[iCL][iW] > maxT0) continue;
        const WireID wid = WireID(iCL, iW);
        if (isHotWire(wid)) continue;
        if (isBadWire(wid)) continue;
        double effi = 1.;
        isDeadWire(wid, effi);
        effiSum += effi;
        m_meanT0 += (iCL < m_firstLayerOffset) ? 0. : effi * m_t0[iCL][iW];
        stdvT0   += (iCL < m_firstLayerOffset) ? 0. : effi * m_t0[iCL][iW] * m_t0[iCL][iW];
      }
    }
    if (effiSum > 0.) {
      m_meanT0 /= effiSum;
      stdvT0   /= effiSum;
      stdvT0 = sqrt(fabs(stdvT0 - m_meanT0 * m_meanT0));
      B2DEBUG(29, it << " " << effiSum << " " << m_meanT0 << " " << stdvT0);
      if (fabs(m_meanT0 - oldMeanT0) < epsi) break;
      oldMeanT0 = m_meanT0;
      minT0 = m_meanT0 - nStdv * stdvT0;
      maxT0 = m_meanT0 + nStdv * stdvT0;
    } else {
      B2FATAL("Wire efficiency sum <= 0!");
    }
  }
  if (it1 == maxIt - 1) B2WARNING("Max. iterations(=" << maxIt << ") needed to calculte the mean t0. Strange.");
}


// Set bad-wire (from DB)
void CDCGeometryPar::setBadWire()
{
  //  m_badWire = (*m_badWireFromDB)->getWires();
  calcMeanT0();
}


// Set prop.-speed (from DB)
void CDCGeometryPar::setPropSpeed()
{
  for (unsigned short iCL = 0; iCL < (*m_propSpeedFromDB)->getEntries(); ++iCL) {
    m_propSpeedInv[iCL] = (iCL < m_firstLayerOffset) ? 0. : 1. / (*m_propSpeedFromDB)->getSpeed(iCL);
  }
}


// Set time-walk coefficient (from DB)
void CDCGeometryPar::setTW()
{
  //  (*m_timeWalkFromDB)->dump();
  m_twParamMode = (*m_timeWalkFromDB)->getTwParamMode();

  for (unsigned short iBd = 0; iBd < (*m_timeWalkFromDB)->getEntries(); ++iBd) {
    int np = ((*m_timeWalkFromDB)->getTimeWalkParams(iBd)).size();
    for (int i = 0; i < np; ++i) {
      m_timeWalkCoef[iBd][i] = ((*m_timeWalkFromDB)->getTimeWalkParams(iBd))[i];
    }
  }
}


// Set xt params. (from DB)
void CDCGeometryPar::setXtRel()
{
  m_linearInterpolationOfXT = true;  //must be true now

  //  std::cout <<"setXtRelation called" << std::endl;
  m_nAlphaPoints = (*m_xtRelFromDB)->getNoOfAlphaBins();
  for (unsigned short i = 0; i < m_nAlphaPoints; ++i) {
    m_alphaPoints[i] = (*m_xtRelFromDB)->getAlphaPoint(i);
    //    std::cout << m_alphaPoints[i]*180./M_PI << std::endl;
  }

  m_nThetaPoints = (*m_xtRelFromDB)->getNoOfThetaBins();
  for (unsigned short i = 0; i < m_nThetaPoints; ++i) {
    m_thetaPoints[i] = (*m_xtRelFromDB)->getThetaPoint(i);
    //    std::cout << m_thetaPoints[i]*180./M_PI << std::endl;
  }

  m_xtParamMode = (*m_xtRelFromDB)->getXtParamMode();

  for (unsigned short iCL = 0; iCL < c_maxNSenseLayers; ++iCL) {
    if (iCL < m_firstLayerOffset) {
      // m_XT is initialized to 0, but reading
      // (*m_xtRelFromDB)->getXtParams(iCL, iLR, iA, iT)
      // could fail if iCL < m_firstLayerOffset, thus continue as m_XT would be set to 0 in this case anyway
      continue;
    }
    for (unsigned short iLR = 0; iLR < 2; ++iLR) {
      for (unsigned short iA = 0; iA < m_nAlphaPoints; ++iA) {
        for (unsigned short iT = 0; iT < m_nThetaPoints; ++iT) {
          const std::vector<float> params = (*m_xtRelFromDB)->getXtParams(iCL, iLR, iA, iT);
          unsigned short np = params.size();
          //    std::cout <<"np4xt= " << np << std::endl;
          for (unsigned short i = 0; i < np; ++i) {
            m_XT[iCL][iLR][iA][iT][i] = params[i];
          }

          double boundT = m_XT[iCL][iLR][iA][iT][6];
          if (m_xtParamMode == 1) {
            m_XT[iCL][iLR][iA][iT][np] = ROOT::Math::Chebyshev5(boundT, m_XT[iCL][iLR][iA][iT][0], m_XT[iCL][iLR][iA][iT][1],
                                                                m_XT[iCL][iLR][iA][iT][2], m_XT[iCL][iLR][iA][iT][3], m_XT[iCL][iLR][iA][iT][4], m_XT[iCL][iLR][iA][iT][5]);
          } else {
            m_XT[iCL][iLR][iA][iT][np] =
              m_XT[iCL][iLR][iA][iT][0] + boundT
              * (m_XT[iCL][iLR][iA][iT][1] + boundT
                 * (m_XT[iCL][iLR][iA][iT][2] + boundT
                    * (m_XT[iCL][iLR][iA][iT][3] + boundT
                       * (m_XT[iCL][iLR][iA][iT][4] + boundT
                          * (m_XT[iCL][iLR][iA][iT][5])))));
          }
        }
      }
    }
  }

}


// Set sigma params. (from DB)
void CDCGeometryPar::setSResol()
{
  m_linearInterpolationOfSgm = true; //must be true now

  //  std::cout <<"setSResol called" << std::endl;
  m_nAlphaPoints4Sgm = (*m_sResolFromDB)->getNoOfAlphaBins();
  for (unsigned short i = 0; i < m_nAlphaPoints4Sgm; ++i) {
    m_alphaPoints4Sgm[i] = (*m_sResolFromDB)->getAlphaPoint(i);
    //    std::cout << m_alphaPoints4Sgm[i]*180./M_PI << std::endl;
  }

  m_nThetaPoints4Sgm = (*m_sResolFromDB)->getNoOfThetaBins();
  for (unsigned short i = 0; i < m_nThetaPoints4Sgm; ++i) {
    m_thetaPoints4Sgm[i] = (*m_sResolFromDB)->getThetaPoint(i);
    //    std::cout << m_thetaPoints4Sgm[i]*180./M_PI << std::endl;
  }

  //  std::cout << "m_nAlphaPoints4Sgm= " << m_nAlphaPoints4Sgm << std::endl;
  //  std::cout << "m_nThetaPoints4Sgm= " << m_nThetaPoints4Sgm << std::endl;

  m_sigmaParamMode = (*m_sResolFromDB)->getSigmaParamMode();

  m_maxSpaceResol = (*m_sResolFromDB)->getMaxSpaceResol();

  for (unsigned short iCL = 0; iCL < c_maxNSenseLayers; ++iCL) {
    for (unsigned short iLR = 0; iLR < 2; ++iLR) {
      for (unsigned short iA = 0; iA < m_nAlphaPoints4Sgm; ++iA) {
        for (unsigned short iT = 0; iT < m_nThetaPoints4Sgm; ++iT) {
          const std::vector<float> params = (*m_sResolFromDB)->getSigmaParams(iCL, iLR, iA, iT);
          unsigned short np = params.size();
          //    std::cout <<"np4sigma= " << np << std::endl;
          for (unsigned short i = 0; i < np; ++i) {
            m_Sigma[iCL][iLR][iA][iT][i] = (iCL < m_firstLayerOffset) ? 0. : params[i];
          }
        }
      }
    }
  }

}


// Set fudge factors (from DB)
void CDCGeometryPar::setFFactor()
{
  unsigned short groupId = (*m_fFactorFromDB)->getGroupID();
  unsigned short nEnt    = (*m_fFactorFromDB)->getEntries();
  B2DEBUG(29, "setFFactor called: groupId,nEnt= " << groupId << " " << nEnt);

  if (groupId == 0) { //per all-layers mode
  } else {
    B2FATAL("CDCGeometryPar:: Invalid group-id " << groupId << " specified!");
  }

  for (unsigned short id = 0; id < nEnt; ++id) {
    unsigned short np = ((*m_fFactorFromDB)->getFactors(id)).size();
    if (np != 3) B2FATAL("CDCGeometryPar:: No. of fudge factors != 3!");
    for (unsigned short i = 0; i < np; ++i) {
      m_fudgeFactorForSigma[i] = ((*m_fFactorFromDB)->getFactors(id))[i];
      B2DEBUG(29, i << " " << m_fudgeFactorForSigma[i]);
    }
  }

  CDCGeoControlPar& gcp = CDCGeoControlPar::getInstance();
  m_fudgeFactorForSigma[0] *= gcp.getAddFudgeFactorForSigmaForData();
  m_fudgeFactorForSigma[1] *= gcp.getAddFudgeFactorForSigmaForMC();
  B2DEBUG(29, "fudge factors= " << m_fudgeFactorForSigma[0] << " " << m_fudgeFactorForSigma[1] << " " << m_fudgeFactorForSigma[2]);
}


// Set ch-map (from DB)
void CDCGeometryPar::setChMap()
{
  for (const auto& cm : (*m_chMapFromDB)) {
    const unsigned short isl = cm.getISuperLayer();
    if (isl >= c_nSuperLayers or isl < m_firstSuperLayerOffset) continue;
    const uint il  = cm.getILayer();
    const int iw  = cm.getIWire();
    const int iBd = cm.getBoardID();
    const WireID wID(isl, il, iw);
    m_wireToBoard.insert(pair<WireID, unsigned short>(wID, iBd));
    const int iCh = cm.getBoardChannel();
    m_wireToChannel.insert(pair<WireID, unsigned short>(wID, iCh));
    m_boardAndChannelToWire[iBd][iCh] = wID.getEWire();
  }
}

// Set edep-to-ADC conversion params. (from DB)
void CDCGeometryPar::setEDepToADCConversions()
{
  unsigned short groupId = (*m_eDepToADCConversionsFromDB)->getGroupID();
  unsigned short nEnt    = (*m_eDepToADCConversionsFromDB)->getEntries();
  if (groupId == 0) { //per super-layer mode
    if (nEnt > c_nSuperLayers) B2FATAL("CDCGeometryPar:: group-id " << groupId << " and #entries " << nEnt << " are inconsistent!");
  } else if (groupId == 1) { //per layer mode
    if (nEnt > c_maxNSenseLayers) B2FATAL("CDCGeometryPar:: group-id " << groupId << " and #entries " << nEnt << " are inconsistent!");
  } else {
    B2FATAL("CDCGeometryPar:: Invalid group-id " << groupId << " specified !");
  }

  unsigned short cLMin[c_nSuperLayers], cLMax[c_nSuperLayers]; //min and max clayer per super-layer
  cLMin[0] = 0;
  cLMax[0] = 7;
  for (unsigned int sl = 1; sl < c_nSuperLayers; ++sl) {
    cLMin[sl] = cLMax[0] + 6 * sl - 5;
    cLMax[sl] = cLMax[0] + 6 * sl;
  }

  for (unsigned short id = 0; id < nEnt; ++id) {
    unsigned short np = ((*m_eDepToADCConversionsFromDB)->getParams(id)).size();
    if (np > 7) B2FATAL("CDCGeometryPar:: No. of edep-to-ADC conversion params. > 7");
    if (groupId == 0) { //per super-layer; id=super-layer
      for (unsigned short cL = cLMin[id]; cL <= cLMax[id]; ++cL) { //clayer loop
        for (unsigned short cell = 0; cell < m_nWires[cL]; ++cell) { //cell loop
          for (unsigned short i = 0; i < np; ++i) {
            m_eDepToADCParams[cL][cell][i] = (cL < m_firstLayerOffset) ? 0. : ((*m_eDepToADCConversionsFromDB)->getParams(id))[i];
          }
        }
      }
    } else if (groupId == 1) { //per clayer; id=clayer
      for (unsigned short cell = 0; cell < m_nWires[id]; ++cell) { //cell loop
        for (unsigned short i = 0; i < np; ++i) {
          m_eDepToADCParams[id][cell][i] = (id < m_firstLayerOffset) ? 0. : ((*m_eDepToADCConversionsFromDB)->getParams(id))[i];
        }
      }
    } else if (groupId == 2) { //per wire
      //not ready
      B2FATAL("CDCGeometryPar::setEDepToADCConversions(): groupId=2 not ready!");
    }
  }
}


double CDCGeometryPar::getEDepToADCConvFactor(unsigned short iCL, unsigned short iW, double edep, double dx, double costh)
{
  //  double convF = (100.0 / 3.2); //keV -> count
  //Model assumed here is from CLEO-c:
  //Igen = Imea * [1 + alf*Imea/cth] / [1 + gam*Imea/cth];
  //cth = |costh| + dlt;
  //Igen: original dE/dx; Imea: measured dE/dx with space-charge effect
  const double  mainF = (iCL < m_firstLayerOffset) ? 0. : m_eDepToADCParams[iCL][iW][0];
  const double& alf   = (iCL < m_firstLayerOffset) ? 0. : m_eDepToADCParams[iCL][iW][1];
  const double& gam   = (iCL < m_firstLayerOffset) ? 0. : m_eDepToADCParams[iCL][iW][2];
  const double& dlt   = (iCL < m_firstLayerOffset) ? 0. : m_eDepToADCParams[iCL][iW][3];
  const double& a     = (iCL < m_firstLayerOffset) ? 0. : m_eDepToADCParams[iCL][iW][4];
  const double& b     = (iCL < m_firstLayerOffset) ? 0. : m_eDepToADCParams[iCL][iW][5];
  const double cth  = fabs(costh) + dlt;
  const double iGen = edep / dx; // keV/cm
  const double tmp  = cth - gam * iGen;
  const double disc = tmp * tmp + 4.*alf * cth * iGen;

  double iMea = 0.;
  if (alf == 0.) {
    iMea = cth * iGen / tmp;
  } else if (disc >= 0.) {
    iMea = (-tmp + sqrt(disc)) / (2.*alf);
    //    if (alf < 0.) {
    //      B2INFO("alf<0:CHECK");
    //      B2INFO(-tmp + sqrt(disc)) / (2.*alf));
    //      B2INFO(-tmp - sqrt(disc)) / (2.*alf));
    //    }
  }

  double convF = mainF;
  if (iMea > 0.) {
    convF = mainF * std::min(iMea / iGen, 1.);
    //    if (iMea > iGen) B2DEBUG(29, "iMea > iGen " << iMea <<" "<< iGen);
  } else {
    //TODO: check the following issue more
    //    B2WARNING("CDCGeometryPar: Measured dE/dx <= 0!");
    //    B2DEBUG(29, "CDCGeometryPar: Measured dE/dx <= 0!");
    //    B2DEBUG(29, "iGen,iMea= " << std::setw(15) << std::scientific << std::setprecision(8) << iGen <<" "<< iMea);
    //    B2DEBUG(29, "dx,mainF,alf,gam,dlt,cth,tmp,disc= " << dx <<" "<< mainF <<" "<< alf <<" "<< gam <<" "<< dlt <<" "<<" "<< tmp <<" "<< disc);
  }
  convF *= 1. + a * (costh - b);
  return convF;
}


void CDCGeometryPar::Print() const
{}

const B2Vector3D CDCGeometryPar::wireForwardPosition(uint layerID, int cellID, EWirePosition set) const
{
  // return early in case of empty layer, i.e. layerID < m_firstLayerOffset
  if (layerID < m_firstLayerOffset) {
    return B2Vector3D(0, 0, 0);
  }

  //  std::cout <<"cdcgeopar::fwdpos set= " << set << std::endl;
  B2Vector3D wPos(m_FWirPosAlign[layerID][cellID][0],
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

const B2Vector3D CDCGeometryPar::wireForwardPosition(uint layerID, int cellID, double z, EWirePosition set) const
{
  // return early in case of empty layer, i.e. layerID < m_firstLayerOffset
  if (layerID < m_firstLayerOffset) {
    return B2Vector3D(0, 0, 0);
  }

  double yb_sag = 0.;
  double yf_sag = 0.;
  getWireSagEffect(set, layerID, cellID, z, yb_sag, yf_sag);

  B2Vector3D wPos(m_FWirPosAlign[layerID][cellID][0], yf_sag,
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

const B2Vector3D CDCGeometryPar::wireBackwardPosition(uint layerID, int cellID, EWirePosition set) const
{
  // return early in case of empty layer, i.e. layerID < m_firstLayerOffset
  if (layerID < m_firstLayerOffset) {
    return B2Vector3D(0, 0, 0);
  }

  B2Vector3D wPos(m_BWirPosAlign[layerID][cellID][0],
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

const B2Vector3D CDCGeometryPar::wireBackwardPosition(uint layerID, int cellID, double z, EWirePosition set) const
{
  // return early in case of empty layer, i.e. layerID < m_firstLayerOffset
  if (layerID < m_firstLayerOffset) {
    return B2Vector3D(0, 0, 0);
  }

  double yb_sag = 0.;
  double yf_sag = 0.;
  getWireSagEffect(set, layerID, cellID, z, yb_sag, yf_sag);

  B2Vector3D wPos(m_BWirPosAlign[layerID][cellID][0], yb_sag,
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

double CDCGeometryPar::getWireSagCoef(EWirePosition set, uint layerID, int cellID) const
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
  static double IRWL[c_maxNSenseLayers] = {0};

  IRWL[0] = outerRadiusInnerWall();
  for (unsigned i = 1; i < nWireLayers(); i++)
    //IRWL[i] = (m_rSLayer[i - 1] + m_rSLayer[i]) / 2.;
    IRWL[i] = (i == m_firstLayerOffset) ? outerRadiusInnerWall() : m_rFLayer[i - 1];

  return IRWL;
}

const double* CDCGeometryPar::outerRadiusWireLayer() const
{
  static double ORWL[c_maxNSenseLayers] = {0};

  ORWL[nWireLayers() - 1] = innerRadiusOuterWall();
  for (unsigned i = 0; i < nWireLayers() - 1; i++)
    //ORWL[i] = (m_rSLayer[i] + m_rSLayer[i + 1]) / 2.;
    ORWL[i] = m_rFLayer[i];

  return ORWL;
}

unsigned CDCGeometryPar::cellId(unsigned layerId, const B2Vector3D& position) const
{
  if (layerId < m_firstLayerOffset) {
    return 0;
  }

  const unsigned nWires = m_nWires[layerId];

  double offset = m_offSet[layerId];
  //...Offset modification to be aligned to axial at z=0...
  const double phiSize = 2 * M_PI / double(nWires);
  /*{
    const double phiF = phiSize * offset
                        + phiSize * 0.5 * double(m_nShifts[layerId]);
    const double phiB = phiSize * offset;
    const B2Vector3D f(m_rSLayer[layerId] * cos(phiF), m_rSLayer[layerId] * sin(phiF), m_zSForwardLayer[layerId]);
    const B2Vector3D b(m_rSLayer[layerId] * cos(phiB), m_rSLayer[layerId] * sin(phiB), m_zSBackwardLayer[layerId]);

    const B2Vector3D v = f - b;
    const B2Vector3D u = v.Unit();
    const double beta = (0 - b.Z()) / u.Z();
    const B2Vector3D p = b + beta * u;
    double phi0 = - atan2(p.Y(), p.X());
    offset += phi0 / (2 * M_PI / double(nWires));
  }*/

  unsigned j = 0;
  for (unsigned i = 0; i < 1; ++i) {
    const double phiF = phiSize * (double(i) + offset)
                        + phiSize * 0.5 * double(m_nShifts[layerId]) + m_globalPhiRotation;
    const double phiB = phiSize * (double(i) + offset)   + m_globalPhiRotation;
    const B2Vector3D f(m_rSLayer[layerId] * cos(phiF), m_rSLayer[layerId] * sin(phiF), m_zSForwardLayer[layerId]);
    const B2Vector3D b(m_rSLayer[layerId] * cos(phiB), m_rSLayer[layerId] * sin(phiB), m_zSBackwardLayer[layerId]);
    const B2Vector3D v = f - b;
    const B2Vector3D u = v.Unit();
    const double beta = (position.Z() - b.Z()) / u.Z();
    const B2Vector3D p = b + beta * u;
    double dPhi = std::atan2(position.Y(), position.X())
                  - std::atan2(p.Y(), p.X())
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

  // return early in case of empty layer, i.e. layerID < m_firstLayerOffset
  if (layerID < m_firstLayerOffset) {
    Yb_sag = 0.;
    Yf_sag = 0.;
    return;
  }

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

  m_FWirPos[L][C][0] = (L < m_firstLayerOffset) ? 0. : m_rSLayer[L] * cos(phiF);
  m_FWirPos[L][C][1] = (L < m_firstLayerOffset) ? 0. : m_rSLayer[L] * sin(phiF);
  m_FWirPos[L][C][2] = (L < m_firstLayerOffset) ? 0. : m_zSForwardLayer[L];

  const double phiB = phiSize * (double(C) + offset) + m_globalPhiRotation;

  m_BWirPos[L][C][0] = (L < m_firstLayerOffset) ? 0. : m_rSLayer[L] * cos(phiB);
  m_BWirPos[L][C][1] = (L < m_firstLayerOffset) ? 0. : m_rSLayer[L] * sin(phiB);
  m_BWirPos[L][C][2] = (L < m_firstLayerOffset) ? 0. : m_zSBackwardLayer[L];

  for (int i = 0; i < 3; ++i) {
    m_FWirPosMisalign[L][C][i] = (L < m_firstLayerOffset) ? 0. : m_FWirPos[L][C][i];
    m_BWirPosMisalign[L][C][i] = (L < m_firstLayerOffset) ? 0. : m_BWirPos[L][C][i];
    m_FWirPosAlign   [L][C][i] = (L < m_firstLayerOffset) ? 0. : m_FWirPos[L][C][i];
    m_BWirPosAlign   [L][C][i] = (L < m_firstLayerOffset) ? 0. : m_BWirPos[L][C][i];
  }

  m_WireSagCoef[L][C] = (L < m_firstLayerOffset) ? 0. : M_PI * m_senseWireDensity * m_senseWireDiameter * m_senseWireDiameter /
                        (8. * m_senseWireTension);
  //  m_WireSagCoef        [L][C] = 0.;
  m_WireSagCoefMisalign[L][C] = (L < m_firstLayerOffset) ? 0. : m_WireSagCoef[L][C];
  m_WireSagCoefAlign   [L][C] = (L < m_firstLayerOffset) ? 0. : m_WireSagCoef[L][C];

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
  if (iCLayer < m_firstLayerOffset) {
    return 0.;
  }

  double dDdt = 0.;

  //calculate min. drift time
  double minTime = getMinDriftTime(iCLayer, lr, alpha, theta);
  double delta = time - minTime;

  //convert incoming- to outgoing-lr
  unsigned short lro = getOutgoingLR(lr, alpha);

  if (!m_linearInterpolationOfXT) {
    B2FATAL("linearInterpolationOfXT = false is not allowed now !");
  } else {
    double wal(0.);
    unsigned short ial[2] = {0};
    unsigned short ilr[2] = {lro, lro};
    getClosestAlphaPoints(alpha, wal, ial, ilr);
    double wth(0.);
    unsigned short ith[2] = {0};
    getClosestThetaPoints(alpha, theta, wth, ith);

    unsigned short jal(0), jlr(0), jth(0);
    double w = 0.;

    //use xt reversed at (x=0,t=tmin) for delta<0 ("negative drifttime")
    double timep = delta < 0. ? minTime - delta : time;

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

      if (timep < boundary) {
        if (m_xtParamMode == 1) {
          const double& c1 = m_XT[iCLayer][jlr][jal][jth][1];
          const double& c2 = m_XT[iCLayer][jlr][jal][jth][2];
          const double& c3 = m_XT[iCLayer][jlr][jal][jth][3];
          const double& c4 = m_XT[iCLayer][jlr][jal][jth][4];
          const double& c5 = m_XT[iCLayer][jlr][jal][jth][5];
          dDdt += w * ROOT::Math::Chebyshev4(timep, c1 + 3.*c3 + 5.*c5, 4.*c2 + 8.*c4, 6.*c3 + 10.*c5, 8.*c4, 10.*c5);
        } else {
          dDdt += w * (m_XT[iCLayer][jlr][jal][jth][1] + timep
                       * (2.*m_XT[iCLayer][jlr][jal][jth][2] + timep
                          * (3.*m_XT[iCLayer][jlr][jal][jth][3] + timep
                             * (4.*m_XT[iCLayer][jlr][jal][jth][4] + timep
                                * (5.*m_XT[iCLayer][jlr][jal][jth][5])))));
        }
      } else {
        dDdt += w * m_XT[iCLayer][jlr][jal][jth][7];
      }
    } //end of weighted mean loop
  }

  dDdt = fabs(dDdt);
  //n.b. following line not needed since dDdt > 0 even for delta < 0
  //  if (delta < 0.) dDdt *= -1.;
  return dDdt;

}

//TODO: mearge with getDriftLength
double CDCGeometryPar::getDriftLength0(const double time, const unsigned short iCLayer, const unsigned short lr, const double alpha,
                                       const double theta) const
{
  if (iCLayer < m_firstLayerOffset) {
    return 0.;
  }

  double dist = 0.;

  //convert incoming- to outgoing-lr
  unsigned short lro = getOutgoingLR(lr, alpha);

  //  std::cout << m_linearInterpolationOfXT << std::endl;
  //  exit(-1);
  if (!m_linearInterpolationOfXT) {
    B2FATAL("linearInterpolationOfXT = false is not allowed now !");
  } else {
    double wal(0.);
    unsigned short ial[2] = {0};
    unsigned short ilr[2] = {lro, lro};
    getClosestAlphaPoints(alpha, wal, ial, ilr);
    double wth(0.);
    unsigned short ith[2] = {0};
    getClosestThetaPoints(alpha, theta, wth, ith);

    unsigned short jal(0), jlr(0), jth(0);
    double w = 0.;

    //use xt reversed at (x=0,t=tmin) for delta<0 ("negative drifttime")
    double timep = time;
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

      double boundary = m_XT[iCLayer][jlr][jal][jth][6];

      if (timep < boundary) {
        if (m_xtParamMode == 1) {
          dist += w * ROOT::Math::Chebyshev5(timep, m_XT[iCLayer][jlr][jal][jth][0], m_XT[iCLayer][jlr][jal][jth][1],
                                             m_XT[iCLayer][jlr][jal][jth][2], m_XT[iCLayer][jlr][jal][jth][3], m_XT[iCLayer][jlr][jal][jth][4], m_XT[iCLayer][jlr][jal][jth][5]);
        } else {
          dist += w * (m_XT[iCLayer][jlr][jal][jth][0] + timep
                       * (m_XT[iCLayer][jlr][jal][jth][1] + timep
                          * (m_XT[iCLayer][jlr][jal][jth][2] + timep
                             * (m_XT[iCLayer][jlr][jal][jth][3] + timep
                                * (m_XT[iCLayer][jlr][jal][jth][4] + timep
                                   * (m_XT[iCLayer][jlr][jal][jth][5]))))));
        }
      } else {
        dist += w * (m_XT[iCLayer][jlr][jal][jth][7] * (timep - boundary) + m_XT[iCLayer][jlr][jal][jth][8]);
      }
      //      std::cout <<"k,w,dist= " << k <<" "<< w <<" "<< dist << std::endl;
    } //end of weighted mean loop
  }

  //  dist = fabs(dist);
  return dist;

}

double CDCGeometryPar::getDriftLength(const double time, const unsigned short iCLayer, const unsigned short lr, const double alpha,
                                      const double theta,
                                      const bool calculateMinTime,
                                      const double inputMinTime) const
{
  if (iCLayer < m_firstLayerOffset) {
    return 0.;
  }

  double dist = 0.;

  //calculate min. drift time
  double minTime = calculateMinTime ? getMinDriftTime(iCLayer, lr, alpha, theta) : inputMinTime;
  double delta = time - minTime;

  //convert incoming- to outgoing-lr
  unsigned short lro = getOutgoingLR(lr, alpha);

  //  std::cout << m_linearInterpolationOfXT << std::endl;
  //  exit(-1);
  if (!m_linearInterpolationOfXT) {
    B2FATAL("linearInterpolationOfXT = false is not allowed now !");
  } else {
    double wal(0.);
    unsigned short ial[2] = {0};
    unsigned short ilr[2] = {lro, lro};
    getClosestAlphaPoints(alpha, wal, ial, ilr);
    double wth(0.);
    unsigned short ith[2] = {0};
    getClosestThetaPoints(alpha, theta, wth, ith);

    unsigned short jal(0), jlr(0), jth(0);
    double w = 0.;

    //use xt reversed at (x=0,t=tmin) for delta<0 ("negative drifttime")
    double timep = delta < 0. ? minTime - delta : time;

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

      if (timep < boundary) {
        if (m_xtParamMode == 1) {
          dist += w * ROOT::Math::Chebyshev5(timep, m_XT[iCLayer][jlr][jal][jth][0], m_XT[iCLayer][jlr][jal][jth][1],
                                             m_XT[iCLayer][jlr][jal][jth][2], m_XT[iCLayer][jlr][jal][jth][3], m_XT[iCLayer][jlr][jal][jth][4], m_XT[iCLayer][jlr][jal][jth][5]);
        } else {
          dist += w * (m_XT[iCLayer][jlr][jal][jth][0] + timep
                       * (m_XT[iCLayer][jlr][jal][jth][1] + timep
                          * (m_XT[iCLayer][jlr][jal][jth][2] + timep
                             * (m_XT[iCLayer][jlr][jal][jth][3] + timep
                                * (m_XT[iCLayer][jlr][jal][jth][4] + timep
                                   * (m_XT[iCLayer][jlr][jal][jth][5]))))));
        }
      } else {
        dist += w * (m_XT[iCLayer][jlr][jal][jth][7] * (timep - boundary) + m_XT[iCLayer][jlr][jal][jth][8]);
      }
      //      std::cout <<"k,w,dist= " << k <<" "<< w <<" "<< dist << std::endl;
    } //end of weighted mean loop
  }

  dist = fabs(dist);
  if (delta < 0.) dist *= -1.;
  return dist;

}

double CDCGeometryPar::getMinDriftTime(const unsigned short iCLayer, const unsigned short lr, const double alpha,
                                       const double theta) const
{
  if (iCLayer < m_firstLayerOffset) {
    return 0.;
  }

  double minTime = 0.;

  //convert incoming- to outgoing-lr
  unsigned short lro = getOutgoingLR(lr, alpha);

  if (!m_linearInterpolationOfXT) {
    B2FATAL("linearInterpolationOfXT = false is not allowed now !");
  } else {
    double wal(0.);
    unsigned short ial[2] = {0};
    unsigned short ilr[2] = {lro, lro};
    getClosestAlphaPoints(alpha, wal, ial, ilr);
    double wth(0.);
    unsigned short ith[2] = {0};
    getClosestThetaPoints(alpha, theta, wth, ith);

    unsigned short jal(0), jlr(0), jth(0);
    double w = 0.;

    double c[6] = {0.}, a[6] = {0.};
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

      for (int i = 0; i < 5; ++i) {
        c[i] += w * m_XT[iCLayer][jlr][jal][jth][i];
      }
    }

    if (m_xtParamMode == 1) { //convert c to coeff for normal-poly if Chebyshev
      a[0] = c[0] -    c[2] +    c[4];
      a[1] = c[1] - 3.*c[3] + 5.*c[5];
      a[2] =        2.*c[2] - 8.*c[4];
      a[3] =        4.*c[3] - 20.*c[5];
      a[4] =        8.*c[4];
      a[5] =       16.*c[5];
    } else { //normal-poly
      for (int i = 0; i < 5; ++i) a[i] = c[i];
    }

    //estimate an initial value
    //    bool check = false;
    //    bool nointersection = false;
    if (a[2] != 0.) {  //2nd-order approx. near t=0
      const double det = a[1] * a[1] - 4.*a[2] * a[0];
      if (det >= 0.) {
        //Choose the solution with dx/dt > 0 which gives x=0
        minTime = (-a[1] + sqrt(det)) / (2.*a[2]);
      } else {
        //Choose the solution with smallest x
        //  nointersection = true;
        minTime = -a[1]  / (2.*a[2]);
        //  cout <<"smallest-x solution= " << minTime << endl;
      }
    } else if (a[1] != 0.) {
      minTime = -a[0] / a[1];  //1st-order approx.
    } else {
      B2WARNING("CDCGeometryPar::getMinDriftTime: minDriftTime not determined; assume zero.\n" << "layer(#0-55),lr,alpha(rad),theta= " <<
                iCLayer << " " << lr << " " << alpha << " " << theta);
      return minTime;
    }

    //    double minTime0 = minTime;
    //higher-order corr. using Newton method; trial to minimize x^2
    double  edm; //  = 10.;   //(cm)  (SG: fix to avoid cpp-check warning)
    //      const double epsi4t = 0.01; //(ns)
    //    const double epsi4x = 1.e-5; //(cm)
    const double epsi4x = 5.e-6; //(cm)
    //    const unsigned short maxIter = 4;
    const unsigned short maxIter = 8;
    const double maxDt = 20.; //(ns)
    unsigned short nIter = 0;
    double minXsq = 1.e10; //(cm^2)
    double minMinTime = minTime;
    //      double told = minTime + 1000.*epsi4t;
    //      while (fabs(minTime - told) > epsi && nIter <= maxIter) {
    for (nIter = 0; nIter <= maxIter; ++nIter) {
      //  told = minTime;
      double t = minTime;
      double x   = a[0] + t * (a[1] + t * (a[2] + t * (a[3] + t * (a[4] + t * a[5]))));
      double x2 = x * x;
      if (x2 < minXsq) {
        minXsq = x2;
        minMinTime = t;
      }
      double xp  = a[1] + t * (2 * a[2] + t * (3 * a[3] + t * (4 * a[4] + t * 5 * a[5])));
      double xpp = 2 * a[2] + t * (6 * a[3] + t * (12 * a[4] + t * 20 * a[5]));
      double den = xp * xp + x * xpp;
      if (den <= 0.) {
        den = xp * xp;
      }

      if (den > 0.) {
        //estimated distance to min.
        edm = fabs(x * xp) / sqrt(den); //not in distance^2 but in distance
        if (edm < epsi4x) break; //converged
      }

      double dt = 1.; //dt for den=0 (ns)
      if (den != 0.) {
        dt = x * xp / den;
        if (dt >= 0.) {
          dt = std::min(dt,  maxDt);
        } else {
          dt = std::max(dt, -maxDt);
        }
      } else {
        B2WARNING("CDCGeometryPar::getMinDriftTime: den = 0\n" << "layer(#0-55),lr,alpha(rad),theta= " <<
                  iCLayer << " "
                  << lr <<
                  " " << alpha << " " << theta);
      }
      minTime -= dt;
    } //end of iteration loop

    //choose minMinTime for not-converged case
    if (nIter == (maxIter + 1)) minTime = minMinTime;

    /*
    if (fabs(minTime) > 20.) {
      B2WARNING("CDCGeometryPar::getMinDriftTime: |minDriftTime| > 20ns. Ok ?\n" << "layer(#0-55),lr,alpha(rad),theta,minTime(ns)= " <<
                iCLayer << " "
                << lr <<
                " " << alpha << " " << theta << " " << minTime);
    }
    if (nointersection) {
      cout <<"final minTime= " << minTime << endl;
      cout <<"final minx   = " << a[0] + a[1] * minTime + a[2] *pow(minTime,2) + a[3] *pow(minTime,3) + a[4] *pow(minTime,4) + a[5] *pow(minTime,5) << endl;
    }
    */
    /*
    if (check) {
      double dmin = 999.;
      double tmin = 999.;
      for (int i = -10000; i < 10000; ++i) {
        double ti = 0.01 * i;
        double dl = fabs(getDriftLength0(ti, iCLayer, lr, alpha, theta));
        if (dl < dmin) {
          dmin = dl;
          tmin = ti;
        }
      }

      double smartd = getDriftLength0(minTime, iCLayer, lr, alpha, theta);
      if (check) {
    //      if (fabs(smartd) > dmin && minTime < tmin && fabs(minTime - tmin) > 0.1) {
        B2WARNING("CDCGeometryPar::getMinDriftTime \n" << "layer(#0-55),lr,alpha(rad),theta= " <<
                  iCLayer << " "
                  << lr <<
                  " " << alpha << " " << theta);
        B2INFO("det, minTime0= " << det << " " << minTime0);
        B2INFO("direct search n,tmin,dmin= " << nIter << " " << tmin << " " << dmin);
        B2INFO(" smart search n,tmin,dmin= " << nIter << " " << minTime << " " << getDriftLength0(minTime, iCLayer, lr, alpha, theta));

        for (int i=-200; i < 200; ++i) {
          double ti = 0.25*i;
          double dl = getDriftLength0(ti, iCLayer, lr, alpha, theta);
          std::cout << ti <<" "<< dl << std::endl;
        }
        exit(-1);
      }
    }
    */
  }

  return minTime;
}

double CDCGeometryPar::getDriftTime(const double dist, const unsigned short iCLayer, const unsigned short lr, const double alpha,
                                    const double theta) const
{
  if (iCLayer < m_firstLayerOffset) {
    return 0.;
  }

  //to be replaced with a smarter algorithm...

  const double eps = 2.5e-1;
  const double maxTrials = 100;

  //  int ialpha = getAlphaBin(alpha);
  //  int itheta = getThetaBin(theta);

  //convert incoming- to outgoing-lr
  //  unsigned short lrp = getOutgoingLR(lr, alpha);

  double maxTime = 2000.; //in ns (n.b. further reduction, 2->1us could be ok)
  //  if (m_XT[iCLayer][lrp][ialpha][itheta][7] == 0.) {
  //    maxTime = m_XT[iCLayer][lrp][ialpha][itheta][6];
  //  }

  double minTime = getMinDriftTime(iCLayer, lr, alpha, theta);
  double t0 = minTime;
  //  std::cout << "minTime,x= " << t0 <<" "<< getDriftLength(t0, iCLayer, lr, alpha, theta) << std::endl;
  const bool calMinTime = false;
  //  double d0 = getDriftLength(t0, iCLayer, lr, alpha, theta, calMinTime, minTime) - dist;
  double d0 = - dist;

  unsigned i = 0;
  double t1 = maxTime;
  double time = dist * m_nominalDriftVInv;
  while (((t1 - t0) > eps) && (i < maxTrials)) {
    time = 0.5 * (t0 + t1);
    double d1 = getDriftLength(time, iCLayer, lr, alpha, theta, calMinTime, minTime) - dist;
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

double CDCGeometryPar::getSigma(const double DriftL0, const unsigned short iCLayer, const unsigned short lr, const double alpha,
                                const double theta) const
{
  if (iCLayer < m_firstLayerOffset) {
    return 0.;
  }


  double sigma = 0.;
  //DriftL0 < 0 for the hit w/driftTime < 0; use |DriftL0| to avoid sigma=nan
  const double driftL = fabs(DriftL0);

  //convert incoming- to outgoing-lr
  unsigned short lro = getOutgoingLR(lr, alpha);

  if (!m_linearInterpolationOfSgm) {
    B2FATAL("linearInterpolationOfSgm = false is not allowed now !");
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
      const double P7 = m_sigmaParamMode == 0 ? DBL_MAX : m_Sigma[iCLayer][jlr][jal][jth][7];

      if (driftL < P7) {
        sigma += w * sqrt(P0 / (driftL * driftL + P1) + P2 * driftL + P3 +
                          P4 * exp(P5 * (driftL - P6) * (driftL - P6)));
      } else {
        double forthTermAtP7 = P4 * exp(P5 * (P7 - P6) * (P7 - P6));
        const double& P8 = m_Sigma[iCLayer][jlr][jal][jth][8];
        if (m_sigmaParamMode == 1) {
          double sigmaAtP7 = sqrt(P0 / (P7 * P7 + P1) + P2 * P7 + P3 + forthTermAtP7);
          sigma += w * (P8 * (driftL - P7) + sigmaAtP7);
        } else if (m_sigmaParamMode == 2) {
          double onePls4AtP7 = sqrt(P0 / (P7 * P7 + P1) + forthTermAtP7);
          const double onePls4 = P8 * (driftL - P7) + onePls4AtP7;
          sigma += w * sqrt(P2 * driftL + P3 + onePls4 * onePls4);
        } else if (m_sigmaParamMode == 3) {
          forthTermAtP7 = sqrt(forthTermAtP7);
          const double forthTerm = P8 * (driftL - P7) + forthTermAtP7;
          sigma += w * sqrt(P0 / (driftL * driftL + P1) + P2 * driftL + P3 +
                            forthTerm * forthTerm);
        } //end of mode
      } // end of driftL
    } //end of for loop
  }

  sigma = std::min(sigma, m_maxSpaceResol);
  return sigma;
}

unsigned short CDCGeometryPar::getOldLeftRight(const B2Vector3D& posOnWire, const B2Vector3D& posOnTrack,
                                               const B2Vector3D& momentum) const
{
  unsigned short lr = 0;
  double wCrossT = (posOnWire.Cross(posOnTrack)).Z();

  if (wCrossT < 0.) {
    lr = 0;
  } else if (wCrossT > 0.) {
    lr = 1;
  } else {
    if ((posOnTrack - posOnWire).Perp() != 0.) {
      double wCrossP = (posOnWire.Cross(momentum)).Z();
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

unsigned short CDCGeometryPar::getNewLeftRightRaw(const B2Vector3D& posOnWire, const B2Vector3D& posOnTrack,
                                                  const B2Vector3D& momentum) const
{
  const double distanceCrossP = ((posOnWire - posOnTrack).Cross(momentum)).Z();
  unsigned short int lr = (distanceCrossP > 0.) ? 1 : 0;
  return lr;
}

//N.B. The following alpha and theta calculations are directly implemented in CDCRecoHit.cc tentatively to avoid a circular dependence betw cdc_dataobjects and cdclib. So be careful when changing the calculations !
double CDCGeometryPar::getAlpha(const B2Vector3D& posOnWire, const B2Vector3D& momentum) const
{
  const double wx = posOnWire.X();
  const double wy = posOnWire.Y();
  const double px = momentum.X();
  const double py = momentum.Y();

  const double cross = wx * py - wy * px;
  const double dot   = wx * px + wy * py;

  return atan2(cross, dot);
}

double CDCGeometryPar::getTheta(const B2Vector3D& momentum) const
{
  return atan2(momentum.Perp(), momentum.Z());
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
    const float* upper = std::upper_bound(m_alphaPoints,
                                          m_alphaPoints + m_nAlphaPoints, alphao);
    points[1] = upper - m_alphaPoints;
    points[0] = points[1] - 1;
    weight = (alphao - m_alphaPoints[points[0]]) / (m_alphaPoints[points[1]] - m_alphaPoints[points[0]]);
  }
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
    const float* upper = std::upper_bound(m_alphaPoints4Sgm,
                                          m_alphaPoints4Sgm + m_nAlphaPoints4Sgm, alphao);
    points[1] = upper - m_alphaPoints4Sgm;
    points[0] = points[1] - 1;
    weight = (alphao - m_alphaPoints4Sgm[points[0]]) / (m_alphaPoints4Sgm[points[1]] - m_alphaPoints4Sgm[points[0]]);
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
    const float* upper = std::upper_bound(m_thetaPoints,
                                          m_thetaPoints + m_nThetaPoints, thetao);
    points[1] = upper - m_thetaPoints;
    points[0] = points[1] - 1;
    weight = (thetao - m_thetaPoints[points[0]]) / (m_thetaPoints[points[1]] - m_thetaPoints[points[0]]);
  }
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
    const float* upper = std::upper_bound(m_thetaPoints4Sgm,
                                          m_thetaPoints4Sgm + m_nThetaPoints4Sgm, thetao);
    points[1] = upper - m_thetaPoints4Sgm;
    points[0] = points[1] - 1;
    weight = (thetao - m_thetaPoints4Sgm[points[0]]) / (m_thetaPoints4Sgm[points[1]] - m_thetaPoints4Sgm[points[0]]);
  }
}


void CDCGeometryPar::setDisplacement()
{
  //    std::cout <<"setDisplacement called" << std::endl;
  for (const auto& disp : (*m_displacementFromDB)) {
    //    const int iLayer0 = disp.getICLayer();
    //    const int iWire0 = disp.getIWire();
    const int iLayer = WireID(disp.getEWire()).getICLayer();
    const int iWire = WireID(disp.getEWire()).getIWire();
    //    if (iLayer0 != iLayer) B2FATAL("Layer0 != Layer");
    //    if (iWire0  != iWire) B2FATAL("Wire0 != Wire");
    m_FWirPos[iLayer][iWire][0] += (iLayer < m_firstLayerOffset) ? 0. : disp.getXFwd();
    m_FWirPos[iLayer][iWire][1] += (iLayer < m_firstLayerOffset) ? 0. : disp.getYFwd();
    m_FWirPos[iLayer][iWire][2] += (iLayer < m_firstLayerOffset) ? 0. : disp.getZFwd();
    m_BWirPos[iLayer][iWire][0] += (iLayer < m_firstLayerOffset) ? 0. : disp.getXBwd();
    m_BWirPos[iLayer][iWire][1] += (iLayer < m_firstLayerOffset) ? 0. : disp.getYBwd();
    m_BWirPos[iLayer][iWire][2] += (iLayer < m_firstLayerOffset) ? 0. : disp.getZBwd();
    m_WireSagCoef[iLayer][iWire] = (iLayer < m_firstLayerOffset) ? 0. : M_PI * m_senseWireDensity * m_senseWireDiameter *
                                   m_senseWireDiameter / (8.*
                                                          (m_senseWireTension + disp.getTension()));
    //    std::cout <<"setdisp iL, iC, nominaltension, tension= " << iLayer <<" " << iWire <<" "<< m_senseWireTension <<" "<< disp.getTension() << std::endl;
  }
}


void CDCGeometryPar::setShiftInSuperLayer()
{
  const unsigned short nLayers[c_nSuperLayers] = {8, 6, 6, 6, 6, 6, 6, 6, 6}; //tentaive

  for (unsigned short SLayer = 0; SLayer < c_nSuperLayers; ++SLayer) {
    unsigned short firstCLayer = 0;
    for (unsigned short i = 0; i < SLayer; ++i) {
      firstCLayer += nLayers[i];
    }
    //    std::cout <<"SLayer,firstCLayer= " << SLayer <<" "<< firstCLayer << std::endl;

    B2Vector3D firstBPos = wireBackwardPosition(firstCLayer, 0);
    for (unsigned short Layer = 0; Layer < nLayers[SLayer]; ++Layer) {
      unsigned short CLayer = firstCLayer + Layer;

      if (CLayer == firstCLayer) {
        m_shiftInSuperLayer[SLayer][Layer] = 0;

      } else if (CLayer == firstCLayer + 1) {
        B2Vector3D BPos = wireBackwardPosition(CLayer, 0);
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
