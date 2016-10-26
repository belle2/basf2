/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/utility/ARICHChannelHist.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>
#include <TVector2.h>
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHModulesInfo.h>
#include <framework/database/DBObjPtr.h>

using namespace std;
using namespace Belle2;

ARICHChannelHist::ARICHChannelHist(const char* name, const char* title, int type) : TH2Poly()
{

  SetName(name);
  SetTitle(title);

  m_hapd2binMap.assign(420, 0);

  DBObjPtr<ARICHGeometryConfig> arichGeo;
  DBObjPtr<ARICHChannelMapping> chMap;
  DBObjPtr<ARICHModulesInfo> modInfo;

  float size = arichGeo->getHAPDGeometry().getPadSize() / 2. - 0.01;
  if (type == 1) size = arichGeo->getHAPDGeometry().getSizeX() / 2. - 0.5;

  float X[5], Y[5], globX[5], globY[5];
  X[0] = -size;  Y[0] = -size;
  X[1] =  size;  Y[1] = -size;
  X[2] =  size;  Y[2] =  size;
  X[3] = -size;  Y[3] =  size;
  X[4] = -size;  Y[4] = -size;

  int nhapd = 1;
  // HAPD bins
  if (type == 1) {
    for (int hapdID = 1; hapdID < 421; hapdID++) {
      if (!modInfo->isActive(hapdID)) continue;
      m_hapd2binMap[hapdID - 1] = nhapd;
      nhapd++;
      float fi = arichGeo->getDetectorPlane().getSlotPhi(hapdID);
      float r  = arichGeo->getDetectorPlane().getSlotR(hapdID);
      TVector2 centerPos(r * cos(fi), r * sin(fi));
      for (int i = 0; i < 5; i++) {
        float rotX = X[i] * cos(fi) - Y[i] * sin(fi);
        float rotY = X[i] * sin(fi) + Y[i] * cos(fi);
        globX[i] = rotX + centerPos.X();
        globY[i] = rotY + centerPos.Y();
      }
      TGraph* mybox = new TGraph(5, globX, globY);

      mybox->SetName((to_string(hapdID)).c_str());
      AddBin(mybox);
    }
  } else if (type == 0) {
    for (int hapdID = 1; hapdID < 421; hapdID++) {
      if (!modInfo->isActive(hapdID)) continue;
      m_hapd2binMap[hapdID - 1] = nhapd;
      nhapd++;
      for (int chID = 0; chID < 144; chID++) {
        float fi = arichGeo->getDetectorPlane().getSlotPhi(hapdID);
        int chX, chY;
        chMap->getXYFromAsic(chID, chX, chY);
        TVector2 centerPos = arichGeo->getChannelPosition(hapdID, chX, chY);
        for (int i = 0; i < 5; i++) {
          float rotX = X[i] * cos(fi) - Y[i] * sin(fi);
          float rotY = X[i] * sin(fi) + Y[i] * cos(fi);
          globX[i] = rotX + centerPos.X();
          globY[i] = rotY + centerPos.Y();
        }
        TGraph* mybox = new TGraph(5, globX, globY);

        mybox->SetName((to_string(hapdID)).c_str());
        AddBin(mybox);
      }
    }
  } else std::cout << "Invalid histogram type! use 0 for channel bins or 1 for HAPD bins" << std::endl;
  SetOption("colz");

}

void ARICHChannelHist::fillBin(unsigned hapdID, unsigned chID)
{

  unsigned chIndex = (m_hapd2binMap[hapdID - 1] - 1) * 144 + chID + 1;
  SetBinContent(chIndex, GetBinContent(chIndex) + 1);
}

void ARICHChannelHist::setBinContent(unsigned hapdID, unsigned chID, double value)
{

  unsigned chIndex = (hapdID - 1) * 144 + chID + 1;
  SetBinContent(chIndex, value);
}

void ARICHChannelHist::setBinContent(unsigned hapdID, double value)
{
  SetBinContent(hapdID, value);
}

void ARICHChannelHist::fillBin(unsigned hapdID)
{
  SetBinContent(hapdID, GetBinContent(hapdID) + 1);
}
