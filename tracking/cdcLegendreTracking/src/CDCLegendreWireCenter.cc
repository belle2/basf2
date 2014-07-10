/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/cdcLegendreTracking/CDCLegendreWireCenter.h>

//using namespace std;
using namespace Belle2;
using namespace TrackFinderCDCLegendre;

WireCenter* WireCenter::s_cdcLegendreWireCenter = 0;

WireCenter::WireCenter()
{
  m_wireCentersArray[0] = -0.54;
  m_wireCentersArray[1] = -0.57;
  m_wireCentersArray[2] = -0.60;
  m_wireCentersArray[3] = -0.64;
  m_wireCentersArray[4] = -0.68;
  m_wireCentersArray[5] = -0.71;
  m_wireCentersArray[6] = -0.74;
  m_wireCentersArray[7] = -0.78;
  m_wireCentersArray[8] = -0.82;
  m_wireCentersArray[9] = -0.91;
  m_wireCentersArray[10] = -0.95;
  m_wireCentersArray[11] = -0.99;
  m_wireCentersArray[12] = -1.08;
  m_wireCentersArray[13] = -1.16;
  m_wireCentersArray[14] = -1.21;
  m_wireCentersArray[15] = -1.27;
  m_wireCentersArray[16] = -1.33;
  m_wireCentersArray[17] = -1.39;
  m_wireCentersArray[18] = -1.45;
  m_wireCentersArray[19] = -1.52;
  m_wireCentersArray[20] = -1.61;
  m_wireCentersArray[21] = -1.65;
  m_wireCentersArray[22] = -1.71;
  m_wireCentersArray[23] = -1.75;
  m_wireCentersArray[24] = -1.80;
  m_wireCentersArray[25] = -1.85;
  m_wireCentersArray[26] = -1.96;
  m_wireCentersArray[27] = -2.02;
  m_wireCentersArray[28] = -2.09;
  m_wireCentersArray[29] = -2.15;
  m_wireCentersArray[30] = -2.22;
  m_wireCentersArray[31] = -2.28;
  m_wireCentersArray[32] = -2.33;
  m_wireCentersArray[33] = -2.41;
  m_wireCentersArray[34] = -2.49;
  m_wireCentersArray[35] = -2.57;
  m_wireCentersArray[36] = -2.65;
  m_wireCentersArray[37] = -2.71;
  m_wireCentersArray[38] = -2.74;
  m_wireCentersArray[39] = -2.80;
  m_wireCentersArray[40] = -2.86;
  m_wireCentersArray[41] = -2.93;
  m_wireCentersArray[42] = -3.00;
  m_wireCentersArray[43] = -3.07;
  m_wireCentersArray[44] = -3.12;
  m_wireCentersArray[45] = -3.18;
  m_wireCentersArray[46] = -3.25;
  m_wireCentersArray[47] = -3.32;
  m_wireCentersArray[48] = -3.38;
  m_wireCentersArray[49] = -3.45;
  m_wireCentersArray[50] = -3.53;
  m_wireCentersArray[51] = -3.59;
  m_wireCentersArray[52] = -3.66;
  m_wireCentersArray[53] = -3.73;
  m_wireCentersArray[54] = -3.82;
  m_wireCentersArray[55] = -3.89;

}




WireCenter& WireCenter::Instance()
{
  if (!s_cdcLegendreWireCenter) s_cdcLegendreWireCenter = new WireCenter();
  return *s_cdcLegendreWireCenter;
}

WireCenter::~WireCenter()
{
  if ((s_cdcLegendreWireCenter) && (this != s_cdcLegendreWireCenter)) {
    delete s_cdcLegendreWireCenter;
    s_cdcLegendreWireCenter = 0;
  }

}
