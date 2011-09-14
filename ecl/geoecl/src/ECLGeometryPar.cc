/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>


#include <ecl/geoecl/ECLGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;



ECLGeometryPar* ECLGeometryPar::m_B4ECLGeometryParDB = 0;

ECLGeometryPar* ECLGeometryPar::Instance()
{
  if (!m_B4ECLGeometryParDB) m_B4ECLGeometryParDB = new ECLGeometryPar();
  return m_B4ECLGeometryParDB;
}

ECLGeometryPar::ECLGeometryPar()
{
  clear();
  read();
}

ECLGeometryPar::~ECLGeometryPar()
{
}

void ECLGeometryPar::clear()
{
  mPar_cellID = 0;
  mPar_thetaID = 0;
  mPar_phiID = 0;
  mPar_thetaIndex = 0;
  mPar_phiIndex = 0;

}

void ECLGeometryPar::read()
{

  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"ECL\"]/Content/");
  for (int iBrCry = 1 ; iBrCry <= 46 ; ++iBrCry) {//46=29+17

    GearDir layerContent(content);
    layerContent.append((format("/BarrelCrystals/BarrelCrystal[%1%]/") % (iBrCry)).str());

    m_BLThetaCrystal[iBrCry-1] = layerContent.getAngle("K_z_TILTED") ;
    m_BLPhiCrystal[iBrCry-1] = layerContent.getAngle("K_phi_TILTED") ;
    m_BLPreppos[iBrCry-1] = layerContent.getLength("K_perpC") ;
    m_BLPhipos[iBrCry-1] = layerContent.getAngle("K_phiC") ;
    m_BLZpos[iBrCry-1] = layerContent.getLength("K_zC") ;
  }



  for (int iCry = 1 ; iCry <= 132 ; ++iCry) {

    GearDir counter(content);
    counter.append((format("/EndCapCrystals/EndCapCrystal[%1%]/") % (iCry)).str());

    m_ECThetaCrystal[iCry-1]  = counter.getAngle("K_Ptheta") ;
    m_ECPhiCrystal[iCry-1] = counter.getAngle("K_Rphi2")  ;
    m_ECRpos[iCry-1]  = counter.getLength("K_Pr");
    m_ECThetapos[iCry-1]  = counter.getAngle("K_Ptheta") ;
    m_ECPhipos[iCry-1]  = counter.getAngle("K_Pphi") ;
  }


}


TVector3 ECLGeometryPar::GetCrystalPos(int cid)
{

  Mapping(cid);
  TVector3 Pos;

  if (mPar_cellID <7776 && mPar_cellID>1151) {
    int iSector = mPar_phiID / 2;
    double phi_ang = iSector * 360 / 72  + m_BLPhipos[mPar_thetaIndex] + (mPar_phiIndex % 2) * (5 - 2.494688) ;
    TVector3 Pos_tmp(m_BLPreppos[mPar_thetaIndex] * cos(phi_ang) ,
                     m_BLPreppos[mPar_thetaIndex] * sin(phi_ang) , m_BLZpos[mPar_thetaIndex]);
    Pos = Pos_tmp;

  } else {

    double phi_ang = m_ECPhipos[mPar_phiIndex] + 360. / 16 * mPar_thetaIndex;
    double theta_ang = m_ECThetapos[mPar_phiIndex];
    TVector3 Pos_tmp(
      m_ECRpos[mPar_phiIndex] * sin(theta_ang) * cos(phi_ang),
      m_ECRpos[mPar_phiIndex] * sin(theta_ang) * sin(phi_ang),
      m_ECRpos[mPar_phiIndex] * cos(theta_ang));
    Pos = Pos_tmp;
  }

  return Pos;
}

TVector3 ECLGeometryPar::GetCrystalVec(int cid)
{

  Mapping(cid);
  TVector3 Pos;

  if (mPar_cellID <7776 && mPar_cellID>1151) {
    int iSector = mPar_phiID / 2;
    double phi_ang = double(iSector) * 360. / 72.  + m_BLPhipos[mPar_thetaIndex] + double(mPar_phiIndex % 2) * (5. - 2.494688) + m_BLPhiCrystal[mPar_thetaIndex];
    double theta_ang = m_BLThetaCrystal[mPar_thetaIndex];
    TVector3 Pos_tmp(
      sin(theta_ang) * cos(phi_ang),
      sin(theta_ang) * sin(phi_ang),
      cos(theta_ang));
    Pos = Pos_tmp;

  } else {
    double phi_ang =   360. / 16. * mPar_thetaIndex +  m_ECPhiCrystal[mPar_phiIndex];
    double theta_ang =  m_ECThetaCrystal[mPar_phiIndex];
    TVector3 Pos_tmp(
      sin(theta_ang) * cos(phi_ang),
      sin(theta_ang) * sin(phi_ang),
      cos(theta_ang));
    Pos = Pos_tmp;
  }

  return Pos;

}



int ECLGeometryPar::GetCellID(int ThetaId, int PhiId)
{
  int forwRing[13] = {0, 3, 6, 10, 14, 18, 24, 30, 36, 42, 48, 54, 63 };
  int backRing[10] = {0, 9, 18, 24, 30, 36, 42, 48, 52, 56} ;

/// 0-12  forward
/// 13-58 barrel
/// 59-68 backward
  if (ThetaId < 13) {
    mPar_cellID = forwRing[ThetaId] * 16 + PhiId;
  } else if (ThetaId > 58) {
    mPar_cellID = 7776 + backRing[ThetaId-59] * 16 + PhiId;
  } else if (ThetaId > 12 && ThetaId < 59) {
    mPar_cellID = 1152 + 144 * (ThetaId - 13)  + PhiId;
  } else     B2ERROR("ECL ECLGeometryPar::CellID int ThetaId " << ThetaId << " int PhiId " << PhiId << ". Out of range.");

  mPar_thetaID = ThetaId;
  mPar_phiID =  PhiId ;
  return mPar_cellID;
}

void ECLGeometryPar::Mapping(int cid)
{


  mPar_cellID = cid;
  if (cid < 0) {
    B2ERROR("ECL ECLGeometryPar Mapping  " << cid << ". Out of range.");
  } else if (cid < 3*16) {//Forkward start
    mPar_thetaID = 0;
    mPar_phiID = cid;
    mPar_phiIndex = mPar_phiID % 3;
    mPar_thetaIndex = mPar_phiID / 3;
  } else if (cid < 6*16) {
    mPar_thetaID = 1;
    mPar_phiID = cid - 3 * 16;
    mPar_phiIndex = mPar_phiID % 3 + 3;
    mPar_thetaIndex = mPar_phiID / 3;
  } else if (cid < 10*16) {
    mPar_thetaID = 2;
    mPar_phiID = cid - 6 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 6;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 14*16) {
    mPar_thetaID = 3;
    mPar_phiID = cid - 10 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 10;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 18*16) {
    mPar_thetaID = 4;
    mPar_phiID = cid - 14 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 14;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 24*16) {
    mPar_thetaID = 5;
    mPar_phiID = cid - 18 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 18;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 30*16) {
    mPar_thetaID = 6;
    mPar_phiID = cid - 24 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 24;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 36*16) {
    mPar_thetaID = 7;
    mPar_phiID = cid - 30 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 30;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 42*16) {
    mPar_thetaID = 8;
    mPar_phiID = cid - 36 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 36;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 48*16) {
    mPar_thetaID = 9;
    mPar_phiID = cid - 42 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 42;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 54*16) {
    mPar_thetaID = 10;
    mPar_phiID = cid - 48 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 48;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 63*16) {
    mPar_thetaID = 11;
    mPar_phiID = cid - 54 * 16 ;
    mPar_phiIndex = mPar_phiID % 9 + 54;
    mPar_thetaIndex = mPar_phiID / 9;
  } else if (cid < 72*16) {
    mPar_thetaID = 12;
    mPar_phiID = cid - 63 * 16 ;
    mPar_phiIndex = mPar_phiID % 9 + 63;
    mPar_thetaIndex = mPar_phiID / 9;
  } else if (cid < 7776) {//Barrel start
    mPar_phiID = (cid - 1152) % 144;
    mPar_thetaID = (cid - 1152) / 144 + 13;
    mPar_thetaIndex = (cid - 1152) / 144;
    mPar_phiIndex = mPar_phiID;
  } else if (cid < 7776 + 9*16) {//Backward start
    mPar_thetaID = 59;
    mPar_phiID =  cid - 7776 ;
    mPar_phiIndex = mPar_phiID % 9 + 72;
    mPar_thetaIndex = mPar_phiID / 9;
  } else if (cid < 7776 + 18*16) {
    mPar_thetaID = 60;
    mPar_phiID =   cid - 7776 - 9 * 16 ;
    mPar_phiIndex = mPar_phiID % 9 + 81;
    mPar_thetaIndex = mPar_phiID / 9;
  } else if (cid < 7776 + 24*16) {
    mPar_thetaID = 61;
    mPar_phiID =   cid - 7776 - 18 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 90;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 30*16) {
    mPar_thetaID = 62;
    mPar_phiID =   cid - 7776 - 24 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 96;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 36*16) {
    mPar_thetaID = 63;
    mPar_phiID =   cid - 7776 - 30 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 102;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 42*16) {
    mPar_thetaID = 64;
    mPar_phiID =   cid - 7776 - 36 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 108;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 48*16) {
    mPar_thetaID = 65;
    mPar_phiID =   cid - 7776 - 42 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 114;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 52*16) {
    mPar_thetaID = 66;
    mPar_phiID =   cid - 7776 - 48 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 120;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 7776 + 56*16) {
    mPar_thetaID = 67;
    mPar_phiID =   cid - 7776 - 52 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 124;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 7776 + 60*16) {
    mPar_thetaID = 68;
    mPar_phiID =   cid - 7776 - 56 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 128;
    mPar_thetaIndex = mPar_phiID / 4;
  } else {
    B2ERROR("ECL ECLGeometryPar Mapping  " << cid << ". Out of range.");
  }

}



