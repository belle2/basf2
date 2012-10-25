/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <cmath>
#include <boost/format.hpp>
#include <ecl/geometry/ECLGeometryPar.h>
#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;
using namespace ECL;

#define PI 3.14159265358979323846


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

    m_BLThetaCrystal[iBrCry - 1] = layerContent.getAngle("K_z_TILTED") * 180 / PI;
    m_BLPhiCrystal[iBrCry - 1] = layerContent.getAngle("K_phi_TILTED") * 180 / PI;
    m_BLPreppos[iBrCry - 1] = layerContent.getLength("K_perpC") ;
    m_BLPhipos[iBrCry - 1] = layerContent.getAngle("K_phiC")  * 180 / PI;
    m_BLZpos[iBrCry - 1] = layerContent.getLength("K_zC") ;
  }



  for (int iCry = 1 ; iCry <= 132 ; ++iCry) {

    GearDir counter(content);
    counter.append((format("/EndCapCrystals/EndCapCrystal[%1%]/") % (iCry)).str());

    m_ECThetaCrystal[iCry - 1]  = counter.getAngle("K_Ptheta") * 180 / PI;
    m_ECPhiCrystal[iCry - 1] = counter.getAngle("K_Rphi2")  * 180 / PI;
    m_ECRpos[iCry - 1]  = counter.getLength("K_Pr") ;
    m_ECThetapos[iCry - 1]  = counter.getAngle("K_Ptheta") * 180 / PI;
    m_ECPhipos[iCry - 1]  = counter.getAngle("K_Pphi") * 180 / PI;
  }


}


TVector3 ECLGeometryPar::GetCrystalPos(int cid)
{

  Mapping(cid);
  TVector3 Pos;

  if (mPar_cellID < 7776 && mPar_cellID > 1151) {
    int iSector = mPar_phiID / 2;
    double phi_ang = iSector * 360 / 72  + m_BLPhipos[mPar_thetaIndex] + (mPar_phiIndex % 2) * (5 - 2.494688) ;
    TVector3 Pos_tmp(m_BLPreppos[mPar_thetaIndex] * cos(phi_ang * PI / 180) ,
                     m_BLPreppos[mPar_thetaIndex] * sin(phi_ang * PI / 180) , m_BLZpos[mPar_thetaIndex]);
    Pos = Pos_tmp;
  } else {

    double phi_ang = m_ECPhipos[mPar_phiIndex] + 360. / 16 * mPar_thetaIndex;
    double theta_ang = m_ECThetapos[mPar_phiIndex];
    TVector3 Pos_tmp(
      m_ECRpos[mPar_phiIndex] * sin(theta_ang * PI / 180) * cos(phi_ang * PI / 180),
      m_ECRpos[mPar_phiIndex] * sin(theta_ang * PI / 180) * sin(phi_ang * PI / 180),
      m_ECRpos[mPar_phiIndex] * cos(theta_ang * PI / 180));

    mPar_thetaID = 4;
    mPar_phiID = cid - 14 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 14;
    mPar_thetaIndex = mPar_phiID / 4;
    Pos = Pos_tmp;
  }

  return Pos;
}

TVector3 ECLGeometryPar::GetCrystalVec(int cid)
{

  Mapping(cid);
  TVector3 Pos;

  if (mPar_cellID < 7776 && mPar_cellID > 1151) {
    int iSector = mPar_phiID / 2;
    double phi_ang = double(iSector) * 360. / 72.  + m_BLPhipos[mPar_thetaIndex] + double(mPar_phiIndex % 2) * (5. - 2.494688) + m_BLPhiCrystal[mPar_thetaIndex];
    double theta_ang = m_BLThetaCrystal[mPar_thetaIndex];
    TVector3 Pos_tmp(
      sin(theta_ang * PI / 180) * cos(phi_ang * PI / 180),
      sin(theta_ang * PI / 180) * sin(phi_ang * PI / 180),
      cos(theta_ang * PI / 180));
    Pos = Pos_tmp;

  } else {
    double phi_ang =   360. / 16. * mPar_thetaIndex +  m_ECPhiCrystal[mPar_phiIndex];
    double theta_ang =  m_ECThetaCrystal[mPar_phiIndex];
    TVector3 Pos_tmp(
      sin(theta_ang * PI / 180) * cos(phi_ang * PI / 180),
      sin(theta_ang * PI / 180) * sin(phi_ang * PI / 180),
      cos(theta_ang * PI / 180));
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
    mPar_cellID = 7776 + backRing[ThetaId - 59] * 16 + PhiId;
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
  } else if (cid < 3 * 16) { //Forkward start
    mPar_thetaID = 0;
    mPar_phiID = cid;
    mPar_phiIndex = mPar_phiID % 3;
    mPar_thetaIndex = mPar_phiID / 3;
  } else if (cid < 6 * 16) {
    mPar_thetaID = 1;
    mPar_phiID = cid - 3 * 16;
    mPar_phiIndex = mPar_phiID % 3 + 3;
    mPar_thetaIndex = mPar_phiID / 3;
  } else if (cid < 10 * 16) {
    mPar_thetaID = 2;
    mPar_phiID = cid - 6 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 6;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 14 * 16) {
    mPar_thetaID = 3;
    mPar_phiID = cid - 10 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 10;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 18 * 16) {
    mPar_thetaID = 4;
    mPar_phiID = cid - 14 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 14;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 24 * 16) {
    mPar_thetaID = 5;
    mPar_phiID = cid - 18 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 18;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 30 * 16) {
    mPar_thetaID = 6;
    mPar_phiID = cid - 24 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 24;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 36 * 16) {
    mPar_thetaID = 7;
    mPar_phiID = cid - 30 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 30;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 42 * 16) {
    mPar_thetaID = 8;
    mPar_phiID = cid - 36 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 36;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 48 * 16) {
    mPar_thetaID = 9;
    mPar_phiID = cid - 42 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 42;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 54 * 16) {
    mPar_thetaID = 10;
    mPar_phiID = cid - 48 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 48;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 63 * 16) {
    mPar_thetaID = 11;
    mPar_phiID = cid - 54 * 16 ;
    mPar_phiIndex = mPar_phiID % 9 + 54;
    mPar_thetaIndex = mPar_phiID / 9;
  } else if (cid < 72 * 16) {
    mPar_thetaID = 12;
    mPar_phiID = cid - 63 * 16 ;
    mPar_phiIndex = mPar_phiID % 9 + 63;
    mPar_thetaIndex = mPar_phiID / 9;
  } else if (cid < 7776) {//Barrel start
    mPar_phiID = (cid - 1152) % 144;
    mPar_thetaID = (cid - 1152) / 144 + 13;
    mPar_thetaIndex = (cid - 1152) / 144;
    mPar_phiIndex = mPar_phiID;
  } else if (cid < 7776 + 9 * 16) { //Backward start
    mPar_thetaID = 59;
    mPar_phiID =  cid - 7776 ;
    mPar_phiIndex = mPar_phiID % 9 + 72;
    mPar_thetaIndex = mPar_phiID / 9;
  } else if (cid < 7776 + 18 * 16) {
    mPar_thetaID = 60;
    mPar_phiID =   cid - 7776 - 9 * 16 ;
    mPar_phiIndex = mPar_phiID % 9 + 81;
    mPar_thetaIndex = mPar_phiID / 9;
  } else if (cid < 7776 + 24 * 16) {
    mPar_thetaID = 61;
    mPar_phiID =   cid - 7776 - 18 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 90;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 30 * 16) {
    mPar_thetaID = 62;
    mPar_phiID =   cid - 7776 - 24 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 96;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 36 * 16) {
    mPar_thetaID = 63;
    mPar_phiID =   cid - 7776 - 30 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 102;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 42 * 16) {
    mPar_thetaID = 64;
    mPar_phiID =   cid - 7776 - 36 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 108;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 48 * 16) {
    mPar_thetaID = 65;
    mPar_phiID =   cid - 7776 - 42 * 16 ;
    mPar_phiIndex = mPar_phiID % 6 + 114;
    mPar_thetaIndex = mPar_phiID / 6;
  } else if (cid < 7776 + 52 * 16) {
    mPar_thetaID = 66;
    mPar_phiID =   cid - 7776 - 48 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 120;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 7776 + 56 * 16) {
    mPar_thetaID = 67;
    mPar_phiID =   cid - 7776 - 52 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 124;
    mPar_thetaIndex = mPar_phiID / 4;
  } else if (cid < 7776 + 60 * 16) {
    mPar_thetaID = 68;
    mPar_phiID =   cid - 7776 - 56 * 16 ;
    mPar_phiIndex = mPar_phiID % 4 + 128;
    mPar_thetaIndex = mPar_phiID / 4;
  } else {
    B2ERROR("ECL ECLGeometryPar Mapping  " << cid << ". Out of range.");
  }
//      cout<<"cid "<<cid<<" mPar_thetaID "<<mPar_thetaID<<" mPar_phiID "<<mPar_phiID<<" mPar_phiIndex "<<mPar_phiIndex<<" mPar_thetaIndex "<<mPar_thetaIndex<<endl;

}


int ECLGeometryPar::ECLVolNameToCellID(const G4String VolumeName)
{
  char temp1[10], temp2[10], temp3[10], temp4[30], temp5[30], temp6[10], temp7[10];
  int cellID = 0;
  sscanf(VolumeName.c_str(), "%[^'_']_%[^'_']_%[^'_']_%[^'_']_%[^'_']_%[^'_']_%s", temp1, temp2, temp3, temp4, temp5, temp6, temp7);

  int GSector = atoi(temp4) - 1;
  int iCry = atoi(temp6) - 1;

  if (VolumeName.c_str() == 0) {
    B2ERROR("ECL simulation cellId; Sector  " << GSector << ". Out of range.");
    return -1;
  } else if (string(VolumeName.c_str()).find("Fw") != string::npos) {

    if (iCry < 3) {
      cellID = GSector * 3 + iCry - 0;
    } else if (iCry < 6) {
      cellID = GSector * 3 + (iCry - 3) + 16 * 3;
    } else if (iCry < 10) {
      cellID = GSector * 4 + (iCry - 6) + 16 * 6;
    } else if (iCry < 14) {
      cellID = GSector * 4 + (iCry - 10) + 16 * 10;
    } else if (iCry < 18) {
      cellID = GSector * 4 + (iCry - 14) + 16 * 14;
    } else if (iCry < 24) {
      cellID = GSector * 6 + (iCry - 18) + 16 * 18;
    } else if (iCry < 30) {
      cellID = GSector * 6 + (iCry - 24) + 16 * 24;
    } else if (iCry < 36) {
      cellID = GSector * 6 + (iCry - 30) + 16 * 30;
    } else if (iCry < 42) {
      cellID = GSector * 6 + (iCry - 36) + 16 * 36;
    } else if (iCry < 48) {
      cellID = GSector * 6 + (iCry - 42) + 16 * 42;
    } else if (iCry < 54) {
      cellID = GSector * 6 + (iCry - 48) + 16 * 48;
    } else if (iCry < 63) {
      cellID = GSector * 9 + (iCry - 54) + 16 * 54;
    } else if (iCry < 72) {
      cellID = GSector * 9 + (iCry - 63) + 16 * 63;

    }
  } else if (string(VolumeName.c_str()).find("Br") != string::npos) {

    GSector = atoi(temp4) - 2 ; //atoi(temp4) = 1-144
    if (GSector == -1) GSector = 143;
    cellID = 1152 + (iCry) * 144 + GSector;

  } else {

    iCry = iCry - 72;
    if (iCry < 9) {
      cellID = (GSector) * 9 + iCry - 0 + 7776;
    } else if (iCry < 18) {
      cellID = (GSector) * 9 + (iCry - 9) + 16 * 9 + 7776;
    } else if (iCry < 24) {
      cellID = (GSector) * 6 + (iCry - 18) + 16 * 18 + 7776;
    } else if (iCry < 30) {
      cellID = (GSector) * 6 + (iCry - 24) + 16 * 24 + 7776;
    } else if (iCry < 36) {
      cellID = (GSector) * 6 + (iCry - 30) + 16 * 30 + 7776;
    } else if (iCry < 42) {
      cellID = (GSector) * 6 + (iCry - 36) + 16 * 36 + 7776;
    } else if (iCry < 48) {
      cellID = (GSector) * 6 + (iCry - 42) + 16 * 42 + 7776;
    } else if (iCry < 52) {
      cellID = (GSector) * 4 + (iCry - 48) + 16 * 48 + 7776;
    } else if (iCry < 56) {
      cellID = (GSector) * 4 + (iCry - 52) + 16 * 52 + 7776;
    } else if (iCry < 60) {
      cellID = (GSector) * 4 + (iCry - 56) + 16 * 56 + 7776;
    }
  }
  return cellID;
}






//
// constants, enums and typedefs
//

//
// static data member definitions
//

// constructors and destructor
//
EclNbr::EclNbr() :
  m_nbrs(*new std::vector< Identifier >)
{
}

EclNbr::EclNbr(const EclNbr& aNbr) :
  m_nbrs(*new std::vector< Identifier > (aNbr.m_nbrs)) ,
  m_nearSize(aNbr.m_nearSize)
{
}

EclNbr::EclNbr(
  const std::vector< Identifier >&           aNbrs     ,
  const std::vector< Identifier >::size_type aNearSize
) :
  m_nbrs(*new std::vector< Identifier > (aNbrs)) ,
  m_nearSize(aNearSize)
{
  // sort vector separately for near, nxt-near nbrs
  std::sort(m_nbrs.begin() , m_nbrs.begin() + aNearSize , std::less< Identifier >()) ;
  std::sort(m_nbrs.begin() + aNearSize ,   m_nbrs.end() , std::less< Identifier >()) ;
}

EclNbr::~EclNbr()
{
  delete &m_nbrs ;
}
std::ostream& operator<<(std::ostream& os, const EclNbr& aNbr)
{
  os << "N(" ;
  unsigned short i(0) ;
  for (std::vector< EclNbr::Identifier >::const_iterator
       iNbr(aNbr.nbrs().begin()) ;
       iNbr != aNbr.nbrs().end() ; ++iNbr) {
    ++i;
    if (iNbr != aNbr.nbrs().begin() && i != aNbr.nearSize() + 1) os << "," ;
    if (i == aNbr.nearSize() + 1) os << "|" ;
    os << std::setw(4) << (*iNbr) ;
  }
  os << ")" ;
  return os ;
}


void EclNbr::printNbr()
{
  unsigned short Nri(0) ;
  cout << "(";
  for (std::vector< EclNbr::Identifier >::const_iterator
       iNbr(m_nbrs.begin()) ;
       iNbr != m_nbrs.end() ; ++iNbr) {
    ++Nri;
    if (iNbr != m_nbrs.begin() && Nri != m_nearSize + 1) cout << "," ;
    if (Nri == m_nearSize + 1) cout << "|" ;
    cout << std::setw(4) << (*iNbr) ;
  }
  cout << ")" << endl;
}
//
// assignment operators


const EclNbr& EclNbr::operator=(const EclNbr& aNbr)
{
  if (this != &aNbr) {
    m_nbrs     = aNbr.m_nbrs     ;
//     delete &m_nbrs;
//     m_nbrs     = *new vector< Identifier > ( aNbr.m_nbrs );
    m_nearSize = aNbr.m_nearSize ;
  }
  return *this ;
}

//
// member functions
//

//
// const member functions
//

const std::vector< EclNbr::Identifier >&
EclNbr::nbrs()      const
{
  return m_nbrs ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nearBegin() const
{
  return m_nbrs.begin() ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nearEnd()   const
{
  return m_nbrs.begin() + m_nearSize ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nextBegin() const
{
  return m_nbrs.begin() + m_nearSize ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nextEnd()   const
{
  return m_nbrs.end() ;
}

std::vector< EclNbr::Identifier >::size_type
EclNbr::nearSize()  const
{
  return m_nearSize ;
}

std::vector< EclNbr::Identifier >::size_type
EclNbr::nextSize()  const
{
  return (m_nbrs.size() - m_nearSize) ;
}


int EclNbr::GetCellID(int ThetaId, int PhiId)
{
  int forwRing[13] = {0, 3, 6, 10, 14, 18, 24, 30, 36, 42, 48, 54, 63 };
  int backRing[10] = {0, 9, 18, 24, 30, 36, 42, 48, 52, 56} ;

/// 0-12  forward
/// 13-58 barrel
/// 59-68 backward
  if (ThetaId < 13) {
    mNbr_cellID = forwRing[ThetaId] * 16 + PhiId;
  } else if (ThetaId > 58) {
    mNbr_cellID = 7776 + backRing[ThetaId - 59] * 16 + PhiId;
  } else if (ThetaId > 12 && ThetaId < 59) {
    mNbr_cellID = 1152 + 144 * (ThetaId - 13)  + PhiId;
  } else     B2ERROR("ECL ECLGeometryNbr::CellID int ThetaId " << ThetaId << " int PhiId " << PhiId << ". Out of range.");

  mNbr_thetaID = ThetaId;
  mNbr_phiID =  PhiId ;
  return mNbr_cellID;
}

void EclNbr::Mapping(int cid)
{


  mNbr_cellID = cid;
  if (cid < 0) {
    B2ERROR("ECL ECLGeometryNbr Mapping  " << cid << ". Out of range.");
  } else if (cid < 3 * 16) { //Forkward start
    mNbr_thetaID = 0;
    mNbr_phiID = cid;
  } else if (cid < 6 * 16) {
    mNbr_thetaID = 1;
    mNbr_phiID = cid - 3 * 16;
  } else if (cid < 10 * 16) {
    mNbr_thetaID = 2;
    mNbr_phiID = cid - 6 * 16 ;
  } else if (cid < 14 * 16) {
    mNbr_thetaID = 3;
    mNbr_phiID = cid - 10 * 16 ;
  } else if (cid < 18 * 16) {
    mNbr_thetaID = 4;
    mNbr_phiID = cid - 14 * 16 ;
  } else if (cid < 24 * 16) {
    mNbr_thetaID = 5;
    mNbr_phiID = cid - 18 * 16 ;
  } else if (cid < 30 * 16) {
    mNbr_thetaID = 6;
    mNbr_phiID = cid - 24 * 16 ;
  } else if (cid < 36 * 16) {
    mNbr_thetaID = 7;
    mNbr_phiID = cid - 30 * 16 ;
  } else if (cid < 42 * 16) {
    mNbr_thetaID = 8;
    mNbr_phiID = cid - 36 * 16 ;
  } else if (cid < 48 * 16) {
    mNbr_thetaID = 9;
    mNbr_phiID = cid - 42 * 16 ;
  } else if (cid < 54 * 16) {
    mNbr_thetaID = 10;
    mNbr_phiID = cid - 48 * 16 ;
  } else if (cid < 63 * 16) {
    mNbr_thetaID = 11;
    mNbr_phiID = cid - 54 * 16 ;
  } else if (cid < 72 * 16) {
    mNbr_thetaID = 12;
    mNbr_phiID = cid - 63 * 16 ;
  } else if (cid < 7776) {//Barrel start
    mNbr_phiID = (cid - 1152) % 144;
    mNbr_thetaID = (cid - 1152) / 144 + 13;
  } else if (cid < 7776 + 9 * 16) { //Backward start
    mNbr_thetaID = 59;
    mNbr_phiID =  cid - 7776 ;
  } else if (cid < 7776 + 18 * 16) {
    mNbr_thetaID = 60;
    mNbr_phiID =   cid - 7776 - 9 * 16 ;
  } else if (cid < 7776 + 24 * 16) {
    mNbr_thetaID = 61;
    mNbr_phiID =   cid - 7776 - 18 * 16 ;
  } else if (cid < 7776 + 30 * 16) {
    mNbr_thetaID = 62;
    mNbr_phiID =   cid - 7776 - 24 * 16 ;
  } else if (cid < 7776 + 36 * 16) {
    mNbr_thetaID = 63;
    mNbr_phiID =   cid - 7776 - 30 * 16 ;
  } else if (cid < 7776 + 42 * 16) {
    mNbr_thetaID = 64;
    mNbr_phiID =   cid - 7776 - 36 * 16 ;
  } else if (cid < 7776 + 48 * 16) {
    mNbr_thetaID = 65;
    mNbr_phiID =   cid - 7776 - 42 * 16 ;
  } else if (cid < 7776 + 52 * 16) {
    mNbr_thetaID = 66;
    mNbr_phiID =   cid - 7776 - 48 * 16 ;
  } else if (cid < 7776 + 56 * 16) {
    mNbr_thetaID = 67;
    mNbr_phiID =   cid - 7776 - 52 * 16 ;
  } else if (cid < 7776 + 60 * 16) {
    mNbr_thetaID = 68;
    mNbr_phiID =   cid - 7776 - 56 * 16 ;
  } else {
    B2ERROR("ECL ECLGeometryNbr Mapping  " << cid << ". Out of range.");
  }
}
EclNbr
EclNbr::getNbr(const Identifier aCellId)
{
  // generate nbr lists. always easier here to work with theta-phi

  //const EclThetaPhiId thetaPhiId ( ids()[ aCellId -1 ].thetaPhiId() );
  //const EclThetaPhiId::Identifier thetaId ( thetaPhiId.thetaId() );
  //const EclThetaPhiId::Identifier phiId   ( thetaPhiId.phiId()   );



  const int cellID = aCellId;
  Mapping(cellID);
  const int thetaId = GetThetaID();
  const int phiId = GetPhiID();
  std::vector< EclNbr::Identifier >::size_type nearSize(0);
  std::vector< EclNbr::Identifier > vNbr;

  vNbr.reserve(24) ;   // except for extreme endcaps, always 24

  int t00 = thetaId;
  int tm1 = thetaId - 1;
  int tm2 = thetaId - 2;
  int tp1 = thetaId + 1;
  int tp2 = thetaId + 2;

  if (aCellId > 1151 && aCellId < 7776) {
    // barrel
    //
    //   12 13 14 15 16      ^ theta
    //   11  2  3  4 17      |
    //   10  1  0  5 18      +--> phi    X--+    view from inside
    //    9  8  7  6 19                     |    (foot pointing e- dir)
    //   24 23 22 21 20                     Z
    int f00 = phiId;
    int fm1 = (phiId + 143) % 144;
    int fp1 = (phiId + 1) % 144;
    int fm2 = (phiId + 142) % 144;
    int fp2 = (phiId + 2) % 144;

    vNbr.push_back(GetCellID(t00 , fm1));
    vNbr.push_back(GetCellID(tp1 , fm1));
    vNbr.push_back(GetCellID(tp1 , f00));
    vNbr.push_back(GetCellID(tp1 , fp1));
    vNbr.push_back(GetCellID(t00 , fp1));
    vNbr.push_back(GetCellID(tm1 , fp1));
    vNbr.push_back(GetCellID(tm1 , f00));
    vNbr.push_back(GetCellID(tm1 , fm1));
    /*
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp1 , fm1 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp1 , f00 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp1 , fp1 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( t00 , fp1 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm1 , fp1 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm1 , f00 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm1 , fm1 )));
    */
    nearSize = vNbr.size();

    vNbr.push_back(GetCellID(tm1 , fm2));
    vNbr.push_back(GetCellID(t00 , fm2));
    vNbr.push_back(GetCellID(tp1 , fm2));
    vNbr.push_back(GetCellID(tp2 , fm2));
    vNbr.push_back(GetCellID(tp2 , fm1));
    vNbr.push_back(GetCellID(tp2 , f00));
    vNbr.push_back(GetCellID(tp2 , fp1));
    vNbr.push_back(GetCellID(tp2 , fp2));
    vNbr.push_back(GetCellID(tp1 , fp2));
    vNbr.push_back(GetCellID(t00 , fp2));
    vNbr.push_back(GetCellID(tm1 , fp2));
    vNbr.push_back(GetCellID(tm2 , fp2));
    vNbr.push_back(GetCellID(tm2 , fp1));
    vNbr.push_back(GetCellID(tm2 , f00));
    vNbr.push_back(GetCellID(tm2 , fm1));
    vNbr.push_back(GetCellID(tm2 , fm2));
    /*
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm1 , fm2 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( t00 , fm2 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp1 , fm2 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp2 , fm2 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp2 , fm1 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp2 , f00 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp2 , fp1 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp2 , fp2 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tp1 , fp2 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( t00 , fp2 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm1 , fp2 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm2 , fp2 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm2 , fp1 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm2 , f00 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm2 , fm1 )));
         vNbr.push_back( mappings().cellId( EclThetaPhiId( tm2 , fm2 )));
    */
  }//if( aCellId > 1152 && aCellId < 7777 )
  else {
    // endcap -- not always 24!
    int n00 = 1000;
    int np1 = 1000;
    int np2 = 1000;
    int nm1 = 1000;
    int nm2 = 1000;
    if (aCellId < 1153) { // forward
      const EclIdentifier mPerRingForward[]
        = { 48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, 144, 144 };
      if (thetaId > 1) nm2 = mPerRingForward[ thetaId - 2 ];
      if (thetaId > 0) nm1 = mPerRingForward[ thetaId - 1 ];
      n00 = mPerRingForward[ thetaId     ];
      np1 = mPerRingForward[ thetaId + 1 ];
      np2 = mPerRingForward[ thetaId + 2 ];
    } else { // backward
      const EclIdentifier mPerRingBackward[]
        = { 64, 64, 64, 96, 96, 96, 96, 96, 144, 144, 144, 144 };
      if (thetaId < 67) np2 = mPerRingBackward[ 66 - thetaId ];
      if (thetaId < 68) np1 = mPerRingBackward[ 67 - thetaId ];
      n00 = mPerRingBackward[ 68 - thetaId ];
      nm1 = mPerRingBackward[ 69 - thetaId ];
      nm2 = mPerRingBackward[ 70 - thetaId ];
    }
    // f-- are phi's, t-- are thetas
    // all calculations should be integer arith - pcs
    // f(th,phi)
    // criterion: center -> next bin
    int f0000 = phiId;
    int fp100 = (f0000 * np1 + np1 / 2) / n00;
    int fp200 = (f0000 * np2 + np2 / 2) / n00;
    int fm100 = (f0000 * nm1 + nm1 / 2) / n00;
    int fm200 = (f0000 * nm2 + nm2 / 2) / n00;

    int f00m1 = (f0000 + n00 - 1) % n00;  // should be exact
    int f00m2 = (f0000 + n00 - 2) % n00;
    int f00p1 = (f0000 + 1) % n00;
    int f00p2 = (f0000 + 2) % n00;

    int fp1m1 = (fp100 + np1 - 1) % np1;
    int fp1m2 = (fp100 + np1 - 2) % np1;
    int fp1p1 = (fp100 + 1) % np1;
    int fp1p2 = (fp100 + 2) % np1;

    int fm1m1 = (fm100 + nm1 - 1) % nm1;
    int fm1m2 = (fm100 + nm1 - 2) % nm1;
    int fm1p1 = (fm100 + 1) % nm1;
    int fm1p2 = (fm100 + 2) % nm1;

    int fp2m1 = (fp200 + np2 - 1) % np2;
    int fp2m2 = (fp200 + np2 - 2) % np2;
    int fp2p1 = (fp200 + 1) % np2;
    int fp2p2 = (fp200 + 2) % np2;

    int fm2m1 = (fm200 + nm2 - 1) % nm2;
    int fm2m2 = (fm200 + nm2 - 2) % nm2;
    int fm2p1 = (fm200 + 1) % nm2;
    int fm2p2 = (fm200 + 2) % nm2;
    int delta = n00 / 16;
//     int sector = phiId/delta;  // 0..15
    int nth = phiId % delta;

    switch (thetaId) {
      case 0:
        if (nth == 1)
          fp2p2 = 1000;
        break;
      case 1:
        if (nth == 1) {
          fp2p2 = 1000;
          fp1p2 = fp1p1;
          fp1p1 = 1000;
        }
        break;
      case 2:
        if ((nth == 0) || (nth == 1)) {
          fm2p2 = 1000;
          fm1p2 = fm1p1;
          fm1p1 = 1000;
        } else if ((nth == 2) || (nth == 3)) {
          fm2m2 = 1000;
          fm1m2 = fm1m1;
          fm1m1 = 1000;
        }
        break;
      case 3:
        if ((nth == 0) || (nth == 3)) {
          fm2p2 = fm2m2 = 1000;
        } else if (nth == 1) {
          fm2p2 = 1000;
        } else if (nth == 2) {
          fm2m2 = 1000;
        }
        break;
      case 5:
        if ((nth == 2) || (nth == 5)) {
          fm2m2 = 1000;
          fm1m2 = fm1m1;
          fm1m1 = 1000;
        } else {
          fm2p2 = 1000;
          fm1p2 = fm1p1;
          fm1p1 = 1000;
        }
        break;
      case 6:
        fm2p2 = 1000;
        if ((nth == 0) || (nth == 2) || (nth == 3) || (nth == 5)) {
          fm2m2 = 1000;
        }
        break;
      case 11:
        if ((nth == 2) || (nth == 5) || (nth == 8)) {
          fm2m2 = 1000;
          fm1m2 = fm1m1;
          fm1m1 = 1000;
        } else {
          fm2p2 = 1000;
          fm1p2 = fm1p1;
          fm1p1 = 1000;
        }
        break;
      case 12:
        fm2p2 = 1000;
        if ((nth == 0) || (nth == 2) || (nth == 3)
            || (nth == 5) || (nth == 6) || (nth == 8))
          fm2m2 = 1000;
        break;
      case 65:
        if ((nth == 2) || (nth == 5)) {
          fp2m2 = 1000;
          fp1m2 = fp1m1;
          fp1m1 = 1000;
        } else {
          fp2p2 = 1000;
          fp1p2 = fp1p1;
          fp1p1 = 1000;
        }
        break;
      case 64:
        fp2p2 = 1000;
        if ((nth ==  0) || (nth == 2) || (nth == 3) || (nth == 5))
          fp2m2 = 1000;
        break;
      case 60:
        if ((nth == 2) || (nth == 5) || (nth == 8)) {
          fp2m2 = 1000;
          fp1m2 = fp1m1;
          fp1m1 = 1000;
        } else {
          fp2p2 = 1000;
          fp1p2 = fp1p1;
          fp1p1 = 1000;
        }
        break;
      case 59:
        fp2p2 = 1000;
        if ((nth ==  0) || (nth == 2) || (nth == 3) || (nth == 5)
            || (nth == 6) || (nth == 8))
          fp2m2 = 1000;
        break;
    }//switch

    // insert near-nbrs
    vNbr.push_back(GetCellID(t00, f00m1));
    vNbr.push_back(GetCellID(t00, f00p1));
    if (nm1 < 999) {
      vNbr.push_back(GetCellID(tm1 , fm100));
      if (fm1m1 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1m1));
      if (fm1p1 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1p1));
    }
    if (np1 < 999) {
      vNbr.push_back(GetCellID(tp1 , fp100));
      if (fp1m1 < 999)
        vNbr.push_back(GetCellID(tp1 , fp1m1));
      if (fp1p1 < 999)
        vNbr.push_back(GetCellID(tp1 , fp1p1));
    }
    nearSize = vNbr.size() ;
    /*
         // insert near-nbrs
         vNbr.push_back(mappings().cellId(EclThetaPhiId(t00, f00m1)));
         vNbr.push_back(mappings().cellId(EclThetaPhiId(t00, f00p1)));
         if(nm1 < 999) {
            vNbr.push_back(mappings().cellId(EclThetaPhiId(tm1 ,fm100)));
            if (fm1m1 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tm1 ,fm1m1)));
            if (fm1p1 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tm1 ,fm1p1)));
         }
         if(np1 < 999) {
            vNbr.push_back(mappings().cellId(EclThetaPhiId(tp1 ,fp100)));
            if (fp1m1 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tp1 ,fp1m1)));
            if (fp1p1 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tp1 , fp1p1)));
         }
         nearSize = vNbr.size() ;
    */

    // now on to next-near neighbors
    if (nm2 < 999) {
      vNbr.push_back(GetCellID(tm2 , fm200));
      if (fm2m1 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2m1));
      if (fm2p1 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2p1));
      if (fm2m2 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2m2));
      if (fm2p2 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2p2));
    }
    if (nm1 < 999) {
      if (fm1m2 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1m2));
      if (fm1p2 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1p2));
    }
    vNbr.push_back(GetCellID(t00 , f00m2));
    vNbr.push_back(GetCellID(t00 , f00p2));
    if (np1 < 999) {
      if (fp1m2 < 999)
        vNbr.push_back(GetCellID(tp1, fp1m2));
      if (fp1p2 < 999)
        vNbr.push_back(GetCellID(tp1, fp1p2));
    }
    if (np2 < 999) {
      vNbr.push_back(GetCellID(tp2, fp200));
      if (fp2m1 < 999)
        vNbr.push_back(GetCellID(tp2, fp2m1));
      if (fp2p1 < 999)
        vNbr.push_back(GetCellID(tp2, fp2p1));
      if (fp2m2 < 999)
        vNbr.push_back(GetCellID(tp2, fp2m2));
      if (fp2p2 < 999)
        vNbr.push_back(GetCellID(tp2, fp2p2));
    }

    /*
         // now on to next-near neighbors
         if(nm2 < 999) {
            vNbr.push_back(mappings().cellId(EclThetaPhiId(tm2 ,fm200)));
            if (fm2m1 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tm2 ,fm2m1)));
            if (fm2p1 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tm2 ,fm2p1)));
            if (fm2m2 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tm2 ,fm2m2)));
            if (fm2p2 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tm2 ,fm2p2)));
         }
         if(nm1 < 999) {
            if (fm1m2 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tm1 ,fm1m2)));
            if (fm1p2 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tm1 ,fm1p2)));
         }
         vNbr.push_back(mappings().cellId(EclThetaPhiId(t00 ,f00m2)));
         vNbr.push_back(mappings().cellId(EclThetaPhiId(t00 ,f00p2)));
         if(np1 < 999) {
            if (fp1m2 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tp1, fp1m2)));
            if (fp1p2 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tp1, fp1p2)));
         }
         if(np2 < 999) {
            vNbr.push_back(mappings().cellId(EclThetaPhiId(tp2, fp200)));
            if (fp2m1 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tp2, fp2m1)));
            if (fp2p1 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tp2, fp2p1)));
            if (fp2m2 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tp2, fp2m2)));
            if (fp2p2 < 999)
               vNbr.push_back(mappings().cellId(EclThetaPhiId(tp2, fp2p2)));
         }

    */
  }//else( aCellId > 1152 && aCellId < 7777 )
  return
    EclNbr(vNbr, nearSize);
}

namespace Belle2 {
  int ECLG4VolNameToCellID(const G4String VolumeName)
  {
    char temp1[10], temp2[10], temp3[10], temp4[30], temp5[30], temp6[10], temp7[10];
    int cellID = 0;
    sscanf(VolumeName.c_str(), "%[^'_']_%[^'_']_%[^'_']_%[^'_']_%[^'_']_%[^'_']_%s", temp1, temp2, temp3, temp4, temp5, temp6, temp7);
    int GSector = (atoi(temp4) - 1) ;
    int iCry = atoi(temp6) - 1;

    if (VolumeName.c_str() == 0) {
      B2ERROR("ECL simulation cellId; Sector  " << GSector << ". Out of range.");
      return -1;
    } else if (string(VolumeName.c_str()).find("Fw") != string::npos) {

      if (iCry < 3) {
        cellID = GSector * 3 + iCry - 0;
      } else if (iCry < 6) {
        cellID = GSector * 3 + (iCry - 3) + 16 * 3;
      } else if (iCry < 10) {
        cellID = GSector * 4 + (iCry - 6) + 16 * 6;
      } else if (iCry < 14) {
        cellID = GSector * 4 + (iCry - 10) + 16 * 10;
      } else if (iCry < 18) {
        cellID = GSector * 4 + (iCry - 14) + 16 * 14;
      } else if (iCry < 24) {
        cellID = GSector * 6 + (iCry - 18) + 16 * 18;
      } else if (iCry < 30) {
        cellID = GSector * 6 + (iCry - 24) + 16 * 24;
      } else if (iCry < 36) {
        cellID = GSector * 6 + (iCry - 30) + 16 * 30;
      } else if (iCry < 42) {
        cellID = GSector * 6 + (iCry - 36) + 16 * 36;
      } else if (iCry < 48) {
        cellID = GSector * 6 + (iCry - 42) + 16 * 42;
      } else if (iCry < 54) {
        cellID = GSector * 6 + (iCry - 48) + 16 * 48;
      } else if (iCry < 63) {
        cellID = GSector * 9 + (iCry - 54) + 16 * 54;
      } else if (iCry < 72) {
        cellID = GSector * 9 + (iCry - 63) + 16 * 63;

      }
    } else if (string(VolumeName.c_str()).find("Br") != string::npos) {
      GSector = GSector - 1;
      if (GSector == -1) GSector = 143;
      cellID = 1152 + (iCry) * 144 + GSector;

    } else {

      iCry = iCry - 72;
      if (iCry < 9) {
        cellID = (GSector) * 9 + iCry - 0 + 7776;
      } else if (iCry < 18) {
        cellID = (GSector) * 9 + (iCry - 9) + 16 * 9 + 7776;
      } else if (iCry < 24) {
        cellID = (GSector) * 6 + (iCry - 18) + 16 * 18 + 7776;
      } else if (iCry < 30) {
        cellID = (GSector) * 6 + (iCry - 24) + 16 * 24 + 7776;
      } else if (iCry < 36) {
        cellID = (GSector) * 6 + (iCry - 30) + 16 * 30 + 7776;
      } else if (iCry < 42) {
        cellID = (GSector) * 6 + (iCry - 36) + 16 * 36 + 7776;
      } else if (iCry < 48) {
        cellID = (GSector) * 6 + (iCry - 42) + 16 * 42 + 7776;
      } else if (iCry < 52) {
        cellID = (GSector) * 4 + (iCry - 48) + 16 * 48 + 7776;
      } else if (iCry < 56) {
        cellID = (GSector) * 4 + (iCry - 52) + 16 * 52 + 7776;
      } else if (iCry < 60) {
        cellID = (GSector) * 4 + (iCry - 56) + 16 * 56 + 7776;
      }
    }
    return cellID;
  }
}//Belle2



