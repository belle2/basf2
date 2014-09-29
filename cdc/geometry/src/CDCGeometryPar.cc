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

CDCGeometryPar& CDCGeometryPar::Instance()
{
  if (!m_B4CDCGeometryParDB) m_B4CDCGeometryParDB = new CDCGeometryPar();
  return *m_B4CDCGeometryParDB;
}

CDCGeometryPar::CDCGeometryPar()
{
  clear();
  read();
}

CDCGeometryPar::~CDCGeometryPar()
{
}

void CDCGeometryPar::clear()
{
  m_motherInnerR = 0.;
  m_motherOuterR = 0.;
  m_motherLength = 0.;

  // T.Hara added to define the CDC mother volume (temporal)
  for (unsigned i = 0; i < 7; ++i) {
    m_momZ[i] = 0.;
    m_momRmin[i] = 0.;
  }
  //

  m_version = "unknown";
  m_nSLayer = 0;
  m_nFLayer = 0;
  m_senseWireDiameter = 0.0;
  m_senseWireTension  = 0.0;
  m_senseWireDensity  = 0.0;
  m_fieldWireDiameter = 0.0;

  m_tdcOffset         = 0;
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

void CDCGeometryPar::read()
{
  // Get the version of cdc geometry parameters
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CDC\"]/Content/");
  //m_version = gbxParams.getString("Version");

  // Get Gearbox parameters
  //gbxParams = Gearbox::Instance().getContent("CDC");

  //------------------------------
  // Get CDC geometry parameters
  //------------------------------
  GearDir innerWallParams(content, "InnerWalls/");
  m_motherInnerR = innerWallParams.getLength("InnerWall[3]/InnerR");

  GearDir outerWallParams(content, "OuterWalls/");
  m_motherOuterR = outerWallParams.getLength("OuterWall[6]/OuterR");

  int nBound = content.getNumberNodes("MomVol/ZBound");
  // Loop over to get the parameters of each boundary
  for (int iBound = 0; iBound < nBound; iBound++) {
    m_momZ[iBound] = content.getLength((format("MomVol/ZBound[%1%]/Z") % (iBound + 1)).str()) / Unit::mm;
    m_momRmin[iBound] = content.getLength((format("MomVol/ZBound[%1%]/Rmin") % (iBound + 1)).str()) / Unit::mm;
  }

  GearDir coverParams(content, "Covers/");
  double R1 = coverParams.getLength("Cover[4]/InnerR1");
  double R2 = coverParams.getLength("Cover[4]/InnerR2");
  double angle = coverParams.getLength("Cover[4]/Angle");
  double thick = coverParams.getLength("Cover[4]/Thickness");
  double zpos = coverParams.getLength("Cover[4]/PosZ");
  double length2;
  if (angle != 0) length2 = fabs(zpos + (R2 - R1) / tan(angle));
  else length2 = fabs(zpos);
  m_motherLength = (length2 + thick) * 2.;

  // Get inner wall parameters
  m_rWall[0]    = innerWallParams.getLength("InnerWall[3]/InnerR");
  m_zWall[0][0] = innerWallParams.getLength("InnerWall[1]/BackwardZ");
  m_zWall[0][1] = innerWallParams.getLength("InnerWall[1]/ForwardZ");

  m_rWall[1] = innerWallParams.getLength("InnerWall[1]/OuterR");
  m_zWall[1][0] = innerWallParams.getLength("InnerWall[1]/BackwardZ");
  m_zWall[1][1] = innerWallParams.getLength("InnerWall[1]/ForwardZ");

  // Get outer wall parameters
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
    B2INFO("CDCGeometryPar: Define all sense and field wires explicitly in the tracking volume.");
  } else {
    B2FATAL("CDCGeometryPar: Materialdefinition mode you specify is invalid.");
  }

  // Loop over all sense layers
  for (int iSLayer = 0; iSLayer < nSLayer; ++iSLayer) {


    int layerId = atoi((gbxParams.getString((format("SLayers/SLayer[%1%]/@id") % (iSLayer + 1)).str())).c_str());

    m_rSLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/Radius") % (iSLayer + 1)).str());
    m_zSBackwardLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/BackwardZ") % (iSLayer + 1)).str());
    m_zSForwardLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/ForwardZ") % (iSLayer + 1)).str());
    m_nWires[layerId] = atoi((gbxParams.getString((format("SLayers/SLayer[%1%]/NHoles") % (iSLayer + 1)).str())).c_str()) / 2;
    m_nShifts[layerId] = atoi((gbxParams.getString((format("SLayers/SLayer[%1%]/NShift") % (iSLayer + 1)).str())).c_str());
    m_offSet[layerId] = atof((gbxParams.getString((format("SLayers/SLayer[%1%]/Offset") % (iSLayer + 1)).str())).c_str());
    m_cellSize[layerId] = 2 * M_PI * m_rSLayer[layerId] / (double) m_nWires[layerId];
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
  m_tdcOffset = 0;  //to be adjusted later; set to 32768 ???
  m_tdcBinWidth = 1.0;  //in ns
  m_nominalDriftV    = 4.e-3;  //in cm/ns
  m_nominalDriftVInv = 1. / m_nominalDriftV; //in ns/cm
  m_nominalPropSpeed = 27.25;  //in cm/nsec (Belle's result, provided by iwasaki san)

  m_nominalSpaceResol = gbxParams.getLength("SenseWire/SpaceResol");
  m_maxSpaceResol = 5. * m_nominalSpaceResol;

  //Set misalignment params. (from input data)
  m_Misalignment = gbxParams.getBool("Misalignment");
  B2INFO("CDCGeometryPar: Load misalignment params.(=1)/not load(=0):" <<
         m_Misalignment);
  if (m_Misalignment) {
    readWirePositionParams(gbxParams, c_Misaligned);
  }

  //Set alignment params. (from input data)
  m_Alignment = gbxParams.getBool("Alignment");
  B2INFO("CDCGeometryPar: Load alignment params.(=1)/not load(=0):" <<
         m_Alignment);
  if (m_Alignment) {
    readWirePositionParams(gbxParams, c_Aligned);
  }

  //Set xt etc. params. for simulation
  m_XTetc = gbxParams.getBool("XTetc");
  if (m_XTetc) {
    //Read xt params.
    readXT(gbxParams);

    //Read sigma params.
    readSigma(gbxParams);

    //Read propagation speed
    readPropSpeed(gbxParams);
  }

  //Replace xt etc. with those for reconstriction
  m_XTetc4Recon = gbxParams.getBool("XTetc4Recon");
  B2INFO("CDCGeometryPar: x-t etc. for reconstruction on(=1)/off(=0):" << m_XTetc4Recon);

  if (m_XTetc4Recon) {
    readXT(gbxParams, 1);
    readSigma(gbxParams, 1);
    readPropSpeed(gbxParams, 1);
  }

  //Print();

}

// Read (mis)alignment params.
void CDCGeometryPar::readWirePositionParams(const GearDir gbxParams, EWirePosition set)
{
  std::string fileName0 = gbxParams.getString("misalignmentFileName");
  if (set == c_Aligned) {
    fileName0 = gbxParams.getString("alignmentFileName");
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
    ifs >> tension;

    if (ifs.eof()) break;

    ++nRead;

    for (int i = 0; i < np; ++i) {
      if (set == c_Misaligned) {
        m_BWirPosMisalign[iL][iC][i] = m_BWirPos[iL][iC][i] + back[i];
        m_FWirPosMisalign[iL][iC][i] = m_FWirPos[iL][iC][i] + fwrd[i];
      } else if (set == c_Aligned) {
        m_BWirPosAlign[iL][iC][i] = m_BWirPos[iL][iC][i] + back[i];
        m_FWirPosAlign[iL][iC][i] = m_FWirPos[iL][iC][i] + fwrd[i];
      }
    }

    double baseTension = M_PI * m_senseWireDensity *
                         m_senseWireDiameter * m_senseWireDiameter / (8.* m_WireSagCoef[iL][iC]);
    if (set == c_Misaligned) {
      m_WireSagCoefMisalign[iL][iC] = M_PI * m_senseWireDensity *
                                      m_senseWireDiameter * m_senseWireDiameter / (8.*(baseTension + tension));
    } else if (set == c_Aligned) {
      m_WireSagCoefAlign[iL][iC] = M_PI * m_senseWireDensity *
                                   m_senseWireDiameter * m_senseWireDiameter / (8.*(baseTension + tension));
    }

    if (m_debug) {
      std::cout << iL << " " << iC;
      for (int i = 0; i < np; ++i) cout << " " << back[i];
      for (int i = 0; i < np; ++i) cout << " " << fwrd[i];
      std::cout << " " << tension << std::endl;
    }

  }

  if (nRead != nSenseWires) B2FATAL("CDCGeometryPar::readWirePositionParams: #lines read-in (=" << nRead << ") is inconsistent with total #sense wires (=" << nSenseWires << ") !");

  ifs.close();
}


// Read x-t params.
void CDCGeometryPar::readXT(const GearDir gbxParams, const int mode)
{
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
  unsigned nalpha = 19; //to be moved to appropriate place...
  unsigned ntheta = 7;  //to be moved to appropriate place...
  unsigned nRead = 0;

  while (true) {

    //
    // Read a line of xt-parameter from Garfield calculations.
    //

    ifs >> iL >> theta >> alpha >> dummy1 >> lr;
    for (int i = 0; i < np - 1; ++i) {
      ifs >> xt[i];
    }

    if (ifs.eof()) break;

    ++nRead;

    const int ialpha = alpha / 10. + 9;

    int itheta = 0;
    if (theta ==  40.) {
      itheta = 1;
    } else if (theta ==  60.) {
      itheta = 2;
    } else if (theta ==  90.) {
      itheta = 3;
    } else if (theta == 120.) {
      itheta = 4;
    } else if (theta == 130.) {
      itheta = 5;
    } else if (theta == 149.) {
      itheta = 6;
    }

    for (int i = 0; i < np - 1; ++i) {
      m_XT[iL][lr][ialpha][itheta][i] = xt[i];
    }

    //    m_XT[iL][lr][ialpha][itheta][6] *= -1;
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

    //convert unit, microsec -> nsec  <- tentative
    i = 1;
    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-3;
    i = 2;
    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-6;
    i = 3;
    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-9;
    i = 4;
    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-12;
    i = 5;
    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-15;
    i = 6;
    m_XT[iL][lr][ialpha][itheta][i] *= 1.e3;
    i = 7;
    m_XT[iL][lr][ialpha][itheta][i] *= 1.e-3;

  }

  if (nRead != 2 * 18 * 7 * MAX_N_SLAYERS) B2FATAL("CDCGeometryPar::readXT: #lines read-in (=" << nRead << ") is inconsistent with 2*18*7 x total #layers (=" << 2 * 18 * 7 * MAX_N_SLAYERS << ") !");

  ifs.close();

  //set xt(L/R,alpha=-90deg) = xt(R/L,alpha=90deg)
  for (unsigned iL = 0; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      int lrp = 0;
      if (lr == 0) lrp = 1;
      for (unsigned itheta = 0; itheta < ntheta; ++itheta) {
        for (int i = 0; i < np; ++i) {
          m_XT[iL][lr][0][itheta][i] = m_XT[iL][lrp][18][itheta][i];
        }
      }
    }
  }

  //set xt(theta= 18) = xt(theta= 40) for the layers >= 20, since xt(theta=18) for these layers are unavailable
  for (unsigned iL = 20; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      for (unsigned ialpha = 0; ialpha < nalpha; ++ialpha) {
        for (int i = 0; i < np; ++i) {
          m_XT[iL][lr][ialpha][0][i] = m_XT[iL][lr][ialpha][1][i];
        }
      }
    }
  }

  //set xt(theta=130) = xt(theta=120) for the layers >= 37, since xt(theta=130) for these layers are unavailable
  for (unsigned iL = 37; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      for (unsigned ialpha = 0; ialpha < nalpha; ++ialpha) {
        for (int i = 0; i < np; ++i) {
          m_XT[iL][lr][ialpha][5][i] = m_XT[iL][lr][ialpha][4][i];
        }
      }
    }
  }

  //set xt(theta=149) = xt(theta=130) for the layers >= 13, since xt(theta=149) for these layers are unavailable
  for (unsigned iL = 13; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      for (unsigned ialpha = 0; ialpha < nalpha; ++ialpha) {
        for (int i = 0; i < np; ++i) {
          m_XT[iL][lr][ialpha][6][i] = m_XT[iL][lr][ialpha][5][i];
        }
      }
    }
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

// Read space reso. params.
void CDCGeometryPar::readSigma(const GearDir gbxParams, const int mode)
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
  const int np = 6;
  double sigma[np];
  unsigned nRead = 0;

  while (true) {
    ifs >> iL;
    for (int i = 0; i < np; ++i) {
      ifs >> sigma[i];
    }
    if (ifs.eof()) break;

    ++nRead;

    for (int i = 0; i < np; ++i) {
      m_Sigma[iL][i] = sigma[i];
    }

    m_Sigma[iL][np] = 0.5 * m_cellSize[iL] - 0.75;

    if (m_debug) {
      cout << iL;
      for (int i = 0; i < np + 1; ++i) {
        cout << " " << m_Sigma[iL][i];
      }
      cout << endl;
    }
  }

  if (nRead != MAX_N_SLAYERS) B2FATAL("CDCGeometryPar::readSigma: #lines read-in (=" << nRead << ") is inconsistent with total #layers (=" << MAX_N_SLAYERS << ") !");

  ifs.close();
}

// Read propagation speed param.
void CDCGeometryPar::readPropSpeed(const GearDir gbxParams, const int mode)
{
  std::string fileName0 = gbxParams.getString("propSpeedFileName");
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

    m_PropSpeedInv[iL] = 1. / speed;

    if (m_debug) cout << iL << " " << speed << endl;
  }

  if (nRead != MAX_N_SLAYERS) B2FATAL("CDCGeometryPar::readPropSpeed: #lines read-in (=" << nRead << ") is inconsistent with total #layers (=" << MAX_N_SLAYERS << ") !");

  ifs.close();
}

void CDCGeometryPar::Print() const
{}

const TVector3 CDCGeometryPar::wireForwardPosition(int layerID, int cellID, EWirePosition set) const
{
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
  for (unsigned i = 0; i < 1; i++) {
    const double phiF = phiSize * (double(i) + offset)
                        + phiSize * 0.5 * double(m_nShifts[layerId]);
    const double phiB = phiSize * (double(i) + offset);
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
    while (j > nWires) j -= nWires;
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
  ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                                << endl
      << "<Subdetector type=\"CDC\">"                                                                                                                << endl
      << "  <Name>CDC BelleII </Name>"                                                                                                               << endl
      << "  <Description>CDC geometry parameters</Description>"                                                                                      << endl
      << "  <Version>0</Version>"                                                                                                                    << endl
      << "  <GeoCreator>CDCBelleII</GeoCreator>"                                                                                                     << endl
      << "  <Content>"                                                                                                                               << endl
      << "    <Rotation desc=\"Rotation of the whole cdc detector (should be the same as beampipe)\" unit=\"mrad\">0.0</Rotation>"                   << endl
      << "    <OffsetZ desc=\"The offset of the whole cdc in z with respect to the IP (should be the same as beampipe)\" unit=\"mm\">0.0</OffsetZ>"  << endl
      << "    <Material>CDCGas</Material>"                                                                                                           << endl
      << endl;

  ofs << "    <SLayers>" << endl;

  for (int i = 0; i < m_nSLayer; i++) {
    ofs << "      <SLayer id=\"" << i << "\">" << endl;
    ofs << "        <Radius desc=\"Radius of wires in this layer\" unit=\"mm\">" << senseWireR(i) << "</Radius>" << endl;
    ofs << "        <BackwardZ desc=\"z position of this wire layer at backward endplate\" unit=\"mm\">" << senseWireBZ(i) << "</BackwardZ>" << endl;
    ofs << "        <ForwardZ desc=\"z position of this wire layer at forward endplate\" unit=\"mm\">" << senseWireFZ(i) << "</ForwardZ>" << endl;
//    ofs << "        <BackwardPhi desc=\"azimuth angle of the first wire in this layer at backward endplate\" unit=\"rad\">" << wireBackwardPosition(i).phi() << "</BackwardPhi>" << endl;
//    ofs << "        <ForwardPhi desc=\"azimuth angle of the first wire in this layer at forward endplate\" unit=\"rad\">" << wireForwardPosition(i).phi() << "</ForwardPhi>" << endl;
    ofs << "        <NHoles desc=\"the number of holes in this layer, 2*(cell number)\">" << nWiresInLayer(i) * 2 << "</NHoles>" << endl;
    ofs << "        <NShift desc=\"the shifted hole number of each wire in this layer\">" << nShifts(i) << "</NShift>" << endl;
    ofs << "        <Offset desc=\"wire offset in phi direction at endplate\">" << m_offSet[i] << "</Offset>" << endl;
    ofs << "      </SLayer>" << endl;
  }

  ofs << "    </SLayers>" << endl;
  ofs << "    <FLayers>" << endl;

  for (int i = 0; i < m_nFLayer; i++) {
    ofs << "      <FLayer id=\"" << i << "\">" << endl;
    ofs << "        <Radius desc=\"Radius of field wires in this layer\" unit=\"mm\">" << fieldWireR(i) << "</Radius>" << endl;
    ofs << "        <BackwardZ desc=\"z position of this field wire layer at backward endplate\" unit=\"mm\">" << fieldWireBZ(i) << "</BackwardZ>" << endl;
    ofs << "        <ForwardZ desc=\"z position of this field wire layer at forward endplate\" unit=\"mm\">" << fieldWireFZ(i) << "</ForwardZ>" << endl;
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

void CDCGeometryPar::getWireSagEffect(const EWirePosition set, const unsigned layerID, const unsigned cellID, const double Z, double& Yb_sag, double& Yf_sag) const
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
                      + phiSize * 0.5 * double(m_nShifts[L]);

  m_FWirPos[L][C][0] = m_rSLayer[L] * cos(phiF);
  m_FWirPos[L][C][1] = m_rSLayer[L] * sin(phiF);
  m_FWirPos[L][C][2] = m_zSForwardLayer[L];

  const double phiB = phiSize * (double(C) + offset);

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
  m_WireSagCoefMisalign[L][C] = m_WireSagCoef[L][C];
  m_WireSagCoefAlign   [L][C] = m_WireSagCoef[L][C];

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

double CDCGeometryPar::getDriftV(const double time, const unsigned short iCLayer, const unsigned short lr, const double alpha, const double theta) const
{

  double dDdt;

  int ialpha = getAlphaBin(alpha);
  int itheta = getThetaBin(theta);

  const double boundary = m_XT[iCLayer][lr][ialpha][itheta][6];

  if (time < boundary) {
    dDdt =    m_XT[iCLayer][lr][ialpha][itheta][1] + time
              * (2.*m_XT[iCLayer][lr][ialpha][itheta][2] + time
                 * (3.*m_XT[iCLayer][lr][ialpha][itheta][3] + time
                    * (4.*m_XT[iCLayer][lr][ialpha][itheta][4] + time
                       * (5.*m_XT[iCLayer][lr][ialpha][itheta][5]))));
  } else {
    dDdt = m_XT[iCLayer][lr][ialpha][itheta][7];
  }

  //replaced with return fabs, since dDdt < 0 rarely; why happens ???
  //  if (lr == 1) dDdt *= -1.;
  //  return dDdt;
  return fabs(dDdt);

}

double CDCGeometryPar::getDriftLength(const double time, const unsigned short iCLayer, const unsigned short lr, const double alpha, const double theta) const
{

  double dist = 0.;

  int ialpha = getAlphaBin(alpha);
  int itheta = getThetaBin(theta);

  //  std::cout <<"iCLayer= " << iCLayer << std::endl;
  //  std::cout <<"lr= " << lr << std::endl;
  //  std::cout <<"alpha,ialpha= " << alpha <<" "<< ialpha << std::endl;
  const double boundary = m_XT[iCLayer][lr][ialpha][itheta][6];
  //  std::cout <<"boundary= " << boundary << std::endl;

  if (time < boundary) {
    dist = m_XT[iCLayer][lr][ialpha][itheta][0] + time
           * (m_XT[iCLayer][lr][ialpha][itheta][1] + time
              * (m_XT[iCLayer][lr][ialpha][itheta][2] + time
                 * (m_XT[iCLayer][lr][ialpha][itheta][3] + time
                    * (m_XT[iCLayer][lr][ialpha][itheta][4] + time
                       * (m_XT[iCLayer][lr][ialpha][itheta][5])))));
  } else {
    dist = m_XT[iCLayer][lr][ialpha][itheta][7] * (time - boundary) + m_XT[iCLayer][lr][ialpha][itheta][8];
  }

  //  if (lr == 1) dist *= -1.;
  //  std::cout <<"dist= " << dist << std::endl;
  //tentative  return std::max(0., dist);
  return fabs(dist);

}

double CDCGeometryPar::getDriftTime(const double dist, const unsigned short iCLayer, const unsigned short lr, const double alpha, const double theta) const
{
  //to be replaced with a smarter algorithm...

  const double eps = 2.5e-1;
  const double maxTrials = 100;

  int ialpha = getAlphaBin(alpha);
  int itheta = getThetaBin(theta);

  double maxTime = 5000.; //in ns
  if (m_XT[iCLayer][lr][ialpha][itheta][7] == 0.) {
    maxTime = m_XT[iCLayer][lr][ialpha][itheta][6];
  }

  double t0 = 0.;
  double d0 = getDriftLength(t0, iCLayer, lr, alpha, theta) - dist;

  unsigned i = 0;
  double t1 = maxTime;
  double time = dist * m_nominalDriftVInv;
  while (((t1 - t0) > eps) && (i < maxTrials)) {
    time = 0.5 * (t0 + t1);
    double d1 = getDriftLength(time, iCLayer, lr, alpha, theta) - dist;
    if (d0 * d1 > 0.) {
      t0 = time;
    } else {
      t1 = time;
    }
    ++i;
  }

  if (i >= maxTrials - 1 || time > maxTime) {
    B2WARNING("CDCGeometryPar::getDriftTime " << dist << " " << iCLayer << " " << alpha << " " << lr << " " << t0 << " " << t1 << " " << time << " " << d0);
  }

  return time;

}

double CDCGeometryPar::getSigma(const double driftL, const unsigned short iCLayer) const
{

  const double P0 = m_Sigma[iCLayer][0];
  const double P1 = m_Sigma[iCLayer][1];
  const double P2 = m_Sigma[iCLayer][2];
  const double P3 = m_Sigma[iCLayer][3];
  const double P4 = m_Sigma[iCLayer][4];
  const double P5 = m_Sigma[iCLayer][5];
  const double P6 = m_Sigma[iCLayer][6];

  double sigma = sqrt(P0 / (driftL * driftL + P1) + P2 * driftL + P3 +
                      P4 * exp(P5 * (driftL - P6) * (driftL - P6)));
  sigma = std::min(sigma, m_maxSpaceResol);

#if defined(CDC_DEBUG)
  cout << "driftL= " << driftL << endl;
  cout << "iCLayer= " << iCLayer << endl;
  cout << "P0= " << P0 << endl;
  cout << "P1= " << P1 << endl;
  cout << "P2= " << P2 << endl;
  cout << "P3= " << P3 << endl;
  cout << "P4= " << P4 << endl;
  cout << "P5= " << P5 << endl;
  cout << "P6= " << P6 << endl;
  cout << "sigma= " << sigma << endl;
#endif

  return sigma;
}

unsigned short CDCGeometryPar::getOldLeftRight(const TVector3& posOnWire, const TVector3& posOnTrack, const TVector3& momentum) const
{
  unsigned short lr = 1;
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
        lr = 1;
      }
    } else {
      lr = 1;
    }
  }
  return lr;
}

signed short CDCGeometryPar::getNewLeftRightRaw(const TVector3& posOnWire, const TVector3& posOnTrack, const TVector3& momentum) const
{
  double distanceCrossP = ((posOnWire - posOnTrack).Cross(momentum)).z();
  short int lr = 1;
  if (distanceCrossP < 0.) lr = -1;
  return lr;
}

double CDCGeometryPar::getAlpha(const TVector3& posOnWire, const TVector3& momentum) const
{
  const double wx = posOnWire.x();
  const double wy = posOnWire.y();
  const double px = momentum.x();
  const double py = momentum.y();

  double sinalpha = (wx * py - wy * px) / sqrt((wx * wx + wy * wy) * (px * px + py * py));

  const double dot = wx * px + wy * py;
  if (dot < 0.) sinalpha *= -1.;

  return asin(sinalpha);
}

double CDCGeometryPar::getTheta(const TVector3& momentum) const
{
  return atan2(momentum.Perp(), momentum.z());
}

int CDCGeometryPar::getAlphaBin(const double alpha) const
{
  //tentative
  int ialpha = (alpha >= 0.) ? (alpha * 180. / M_PI + 5.) / 10. : (alpha * 180. / M_PI - 5.) / 10.;
  ialpha += 9;
  ialpha = std::max(0, ialpha);
  ialpha = std::min(18, ialpha);
  return ialpha;
}

int CDCGeometryPar::getThetaBin(const double theta) const
{
  double th = 180. * theta / M_PI;
  //hard-coded tentatively
  int itheta = 0;
  if (th < 29.0) {
  } else if (th <  50.0) {
    itheta = 1;
  } else if (th <  75.0) {
    itheta = 2;
  } else if (th < 105.0) {
    itheta = 3;
  } else if (th < 125.0) {
    itheta = 4;
  } else if (th < 139.5) {
    itheta = 5;
  } else {
    itheta = 6;
  }
  return itheta;
}

double CDCGeometryPar::ClosestApproach(const TVector3 bwp, const TVector3 fwp, const TVector3 posIn, const TVector3 posOut, TVector3& hitPosition, TVector3& wirePosition) const
{
  //----------------------------------------------------------
  /* For two lines r=r1+t1.v1 & r=r2+t2.v2
     the closest approach is d=|(r2-r1).(v1 x v2)|/|v1 x v2|
     the point where closest approach are
     t1=(v1 x v2).[(r2-r1) x v2]/[(v1 x v2).(v1 x v2)]
     t2=(v1 x v2).[(r2-r1) x v1]/[(v1 x v2).(v1 x v2)]
     if v1 x v2=0 means two lines are parallel
     d=|(r2-r1) x v1|/|v1|
  */

  double t1, t2, distance, dInOut, dHitIn, dHitOut;

  //--------------------------
  // Get wirepoint @ endplate
  //--------------------------
  /*  CDCGeometryPar& cdcgp = CDCGeometryPar::Instance();
  TVector3 tfwp = cdcgp.wireForwardPosition(layerId, cellId);
  G4ThreeVector fwp(tfwp.x(), tfwp.y(), tfwp.z());
  TVector3 tbwp = cdcgp.wireBackwardPosition(layerId, cellId);
  G4ThreeVector bwp(tbwp.x(), tbwp.y(), tbwp.z());
  */

  TVector3 wireLine = fwp - bwp;
  TVector3 hitLine = posOut - posIn;

  TVector3 hitXwire = hitLine.Cross(wireLine);
  TVector3 wire2hit = fwp - posOut;

  //----------------------------------------------------------------
  // Hitposition is the position on hit line where closest approach
  // of two lines, but it may out the area from posIn to posOut
  //----------------------------------------------------------------
  if (hitXwire.Mag() == 0) {
    distance = wireLine.Cross(wire2hit).Mag() / wireLine.Mag();
    hitPosition = posIn;
    t2 = (posIn - fwp).Dot(wireLine) / wireLine.Mag2();
  } else {
    t1 = hitXwire.Dot(wire2hit.Cross(wireLine)) / hitXwire.Mag2();
    hitPosition = posOut + t1 * hitLine;
    t2 = hitXwire.Dot(wire2hit.Cross(hitLine)) / hitXwire.Mag2();

    dInOut = (posOut - posIn).Mag();
    dHitIn = (hitPosition - posIn).Mag();
    dHitOut = (hitPosition - posOut).Mag();
    if (dHitIn <= dInOut && dHitOut <= dInOut) { //Between point in & out
      distance = fabs(wire2hit.Dot(hitXwire) / hitXwire.Mag());
    } else if (dHitOut > dHitIn) { // out posIn
      distance = wireLine.Cross(posIn - fwp).Mag() / wireLine.Mag();
      hitPosition = posIn;
      t2 = (posIn - fwp).Dot(wireLine) / wireLine.Mag2();
    } else { // out posOut
      distance = wireLine.Cross(posOut - fwp).Mag() / wireLine.Mag();
      hitPosition = posOut;
      t2 = (posOut - fwp).Dot(wireLine) / wireLine.Mag2();
    }
  }

  wirePosition = fwp + t2 * wireLine;

  return distance;

}
