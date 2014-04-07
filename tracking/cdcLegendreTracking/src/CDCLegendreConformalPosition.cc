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

CDCLegendreConformalPosition::CDCLegendreConformalPosition(int ninsTheta) :
  m_ninsTheta(ninsTheta)
{
  //Get the position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  int nLayers = 56;
  int nWires[56];
  TVector3 wireBegin;
  TVector3 wireEnd;
  double zReference = 25.852;
  double fraction;

  double x, y, x_con, y_con;

  for (int layerId = 0; layerId < nLayers; layerId++) {

    if ((layerId >= 0) && (layerId <= 13)) {
      nWires[layerId] = 160;

      for (int wireId = 0; wireId < nWires[layerId]; wireId++) {

        wireBegin = cdcg.wireForwardPosition(layerId, wireId);
        wireEnd   = cdcg.wireBackwardPosition(layerId, wireId);

        fraction = (zReference - wireBegin.z()) / (wireEnd.z() - wireBegin.z());

        x = wireBegin.x() + fraction * (wireEnd.x() - wireBegin.x());
        y = wireBegin.y() + fraction * (wireEnd.y() - wireBegin.y());

        x_con = 2 * x / (x * x + y * y);
        y_con = 2 * y / (x * x + y * y);



      }



    }



  }

}


CDCLegendreConformalPosition& CDCLegendreConformalPosition::Instance(int ninsTheta)
{
  if (!s_cdcLegendreConformalPosition) s_cdcLegendreConformalPosition = new CDCLegendreConformalPosition(ninsTheta);
  return *s_cdcLegendreConformalPosition;
}
