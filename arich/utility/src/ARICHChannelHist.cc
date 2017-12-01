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
#include <math.h>
#include <iostream>
#include <algorithm>
#include <TVector2.h>

using namespace std;
using namespace Belle2;

ARICHChannelHist::ARICHChannelHist(const char* name, const char* title, int type,
                                   const std::vector<unsigned>& moduleIDs) : TH2Poly()
{

  SetName(name);
  SetTitle(title);

  m_hapd2binMap.assign(420, 0);

  // positions of HAPDs and channel mapping (avoid using DB classes...)
  double rs[7] = {57.35, 65.81, 74.37, 82.868, 91.305, 99.794, 108.185};
  unsigned nhapds[7] = {42, 48, 54, 60, 66, 72, 78};
  unsigned chmap[144] = {88, 86, 96, 87, 75, 72, 97, 108, 73, 74, 98, 109, 84, 85, 120, 110, 76, 77, 132, 121, 136, 124, 99, 133, 125, 113, 122, 111, 101, 137, 134, 123, 89, 112, 100, 135, 52, 28, 3, 40, 41, 5, 15, 2, 17, 29, 27, 14, 4, 16, 1, 26, 53, 65, 0, 13, 48, 49, 39, 12, 61, 62, 25, 38, 63, 60, 24, 37, 64, 50, 51, 36, 91, 115, 140, 103, 102, 138, 128, 141, 126, 114, 116, 129, 139, 127, 142, 117, 90, 78, 143, 130, 95, 94, 104, 131, 82, 81, 118, 105, 80, 83, 119, 106, 79, 93, 92, 107, 55, 57, 47, 56, 68, 71, 46, 35, 70, 69, 45, 34, 59, 58, 23, 33, 67, 66, 11, 22, 7, 19, 44, 10, 18, 30, 21, 32, 42, 6, 9, 20, 54, 31, 43, 8};
  double chns[12] = { -2.88, -2.37, -1.86, -1.35, -0.84, -0.33, 0.33, 0.84, 1.35, 1.86, 2.37, 2.88};

  float size = 0.5 / 2. - 0.01;
  if (type == 1) size = 7.0 / 2. - 0.5;


  float X[5], Y[5], globX[5], globY[5];
  X[0] = -size;  Y[0] = -size;
  X[1] =  size;  Y[1] = -size;
  X[2] =  size;  Y[2] =  size;
  X[3] = -size;  Y[3] =  size;
  X[4] = -size;  Y[4] = -size;

  int nhapd = 1;
  unsigned iring = 0;
  unsigned ihapd = 0;

  std::vector<unsigned> ids;
  if (moduleIDs.size() > 0) ids = moduleIDs;
  else {
    for (int hapdID = 1; hapdID < 421; hapdID++) {
      ids.push_back(hapdID);
    }
  }

  // HAPD bins
  if (type == 1) {
    for (int hapdID = 1; hapdID < 421; hapdID++) {
      //for (unsigned hapdID : ids) {
      //m_hapd2binMap[hapdID - 1] = nhapd;
      //nhapd++;
      float r = rs[iring];
      float dphi = 2.*M_PI / nhapds[iring];
      float fi = dphi / 2. + ihapd * dphi;
      TVector2 centerPos(r * cos(fi), r * sin(fi));
      for (int i = 0; i < 5; i++) {
        float rotX = X[i] * cos(fi) - Y[i] * sin(fi);
        float rotY = X[i] * sin(fi) + Y[i] * cos(fi);
        globX[i] = rotX + centerPos.X();
        globY[i] = rotY + centerPos.Y();
      }
      if (std::find(ids.begin(), ids.end(), hapdID) != ids.end()) {
        m_hapd2binMap[hapdID - 1] = nhapd;
        nhapd++;
        TGraph* mybox = new TGraph(5, globX, globY);
        mybox->SetName((to_string(hapdID)).c_str());
        AddBin(mybox);
      }
      ihapd++;
      if (ihapd == nhapds[iring]) { iring++; ihapd = 0;}
    }

  } else if (type == 0) {
    for (int hapdID = 1; hapdID < 421; hapdID++) {
      //for (unsigned hapdID : ids) {
      //  m_hapd2binMap[hapdID - 1] = nhapd;
      //  nhapd++;
      float dphi = 2.*M_PI / nhapds[iring];
      float fi = dphi / 2. + ihapd * dphi;
      float r = rs[iring];
      for (int chID = 0; chID < 144; chID++) {

        unsigned chX = chmap[chID] % 12;
        unsigned chY = chmap[chID] / 12;
        TVector2 hapdPos(r * cos(fi), r * sin(fi));
        TVector2 locPos(chns[chX], chns[chY]);
        TVector2 centerPos = hapdPos + locPos.Rotate(fi);

        for (int i = 0; i < 5; i++) {
          float rotX = X[i] * cos(fi) - Y[i] * sin(fi);
          float rotY = X[i] * sin(fi) + Y[i] * cos(fi);
          globX[i] = rotX + centerPos.X();
          globY[i] = rotY + centerPos.Y();
        }
        if (std::find(ids.begin(), ids.end(), hapdID) != ids.end()) {
          m_hapd2binMap[hapdID - 1] = nhapd;
          if (chID == 143) nhapd++;
          TGraph* mybox = new TGraph(5, globX, globY);
          mybox->SetName((to_string(hapdID)).c_str());
          AddBin(mybox);
        }
      }
      ihapd++;
      if (ihapd == nhapds[iring]) { iring++; ihapd = 0;}
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

  unsigned chIndex = (m_hapd2binMap[hapdID - 1] - 1) * 144 + chID + 1;
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
