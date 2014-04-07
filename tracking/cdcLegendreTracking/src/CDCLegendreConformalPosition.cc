/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreConformalPosition.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

CDCLegendreConformalPosition* CDCLegendreConformalPosition::s_cdcLegendreConformalPosition = 0;

CDCLegendreConformalPosition& CDCLegendreConformalPosition::Instance()
{
  if (!s_cdcLegendreConformalPosition) s_cdcLegendreConformalPosition = new CDCLegendreConformalPosition();
  return *s_cdcLegendreConformalPosition;
}


CDCLegendreConformalPosition& CDCLegendreConformalPosition::InstanceTrusted()
{
  return *s_cdcLegendreConformalPosition;
}

void CDCLegendreConformalPosition::clearPointers()
{
  delete s_cdcLegendreConformalPosition;
}

CDCLegendreConformalPosition::CDCLegendreConformalPosition()
{
  //Get the position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  int nWires;
  TVector3 wireBegin;
  TVector3 wireEnd;
  double zReference = 25.852;
  double fraction;

  //Initialize look-up table
  double bin_width = m_PI / m_nbinsTheta;
  m_sin_theta = new double[m_nbinsTheta + 1];
  m_cos_theta = new double[m_nbinsTheta + 1];

  for (int i = 0; i <= m_nbinsTheta; ++i) {
    m_sin_theta[i] = sin(i * bin_width);
    m_cos_theta[i] = cos(i * bin_width);
  }



  double x, y, x_con, y_con;

  for (int layerId = 0; layerId < nLayers_max; layerId++) {

    if ((layerId >= 0) && (layerId <= 13)) {
      nWires = 160;
    } else if ((layerId >= 14) && (layerId <= 19)) {
      nWires = 192;
    } else if ((layerId >= 20) && (layerId <= 25)) {
      nWires = 224;
    } else if ((layerId >= 26) && (layerId <= 31)) {
      nWires = 256;
    } else if ((layerId >= 32) && (layerId <= 37)) {
      nWires = 288;
    } else if ((layerId >= 38) && (layerId <= 43)) {
      nWires = 320;
    } else if ((layerId >= 44) && (layerId <= 49)) {
      nWires = 352;
    } else if ((layerId >= 50) && (layerId <= 55)) {
      nWires = 384;
    }

    for (int wireId = 0; wireId < nWires; wireId++) {

      wireBegin = cdcg.wireForwardPosition(layerId, wireId);
      wireEnd   = cdcg.wireBackwardPosition(layerId, wireId);

      fraction = (zReference - wireBegin.z()) / (wireEnd.z() - wireBegin.z());

      x = wireBegin.x() + fraction * (wireEnd.x() - wireBegin.x());
      y = wireBegin.y() + fraction * (wireEnd.y() - wireBegin.y());

      x_con = 2 * x / (x * x + y * y);
      y_con = 2 * y / (x * x + y * y);

      for (int binTheta = 0; binTheta <= m_nbinsTheta; ++binTheta) {
        double r_temp = x_con * m_cos_theta[binTheta] +
                        y_con * m_sin_theta[binTheta];
        m_lookupR[layerId][wireId][binTheta] = r_temp;
      }
    }
  }

}

CDCLegendreConformalPosition::~CDCLegendreConformalPosition()
{
  if ((s_cdcLegendreConformalPosition) && (this != s_cdcLegendreConformalPosition)) {
    delete s_cdcLegendreConformalPosition;
    s_cdcLegendreConformalPosition = NULL;
  }

  delete[] m_sin_theta;
  delete[] m_cos_theta;

}
