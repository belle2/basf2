/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVD_PAR_H_
#define SVD_PAR_H_

#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  /* This namespace provides basic parameters in SVD setup.
   */

  namespace SVDPar {

    const unsigned short maxLayer    = 5;
    const unsigned short minLayer    = 2;
    const unsigned short nLayer      = maxLayer - minLayer + 1;
    const unsigned short nLadder_L3  = 7;
    const unsigned short nLadder_L4  = 10;
    const unsigned short nLadder_L5  = 12;
    const unsigned short nLadder_L6  = 16;
    const unsigned short nLadder[nLayer] =
    {nLadder_L3, nLadder_L4, nLadder_L5, nLadder_L6};
    const unsigned short nSensor_L3  = 2;
    const unsigned short nSensor_L4  = 3;
    const unsigned short nSensor_L5  = 4;
    const unsigned short nSensor_L6  = 5;
    const unsigned short nSensor[nLayer] =
    {nSensor_L3, nSensor_L4, nSensor_L5, nSensor_L6};
    //const unsigned short nSegment    = 2;
    const unsigned short nPN         = 2;
    const unsigned short maxStrip    = 768;

    const unsigned short maxSample   = 6;

    const unsigned short NApvPerHybrid   = 6;

    const unsigned short nSensorID   =
      (unsigned short)(nSensor_L3 * nLadder_L3 +
                       nSensor_L4 * nLadder_L4 +
                       nSensor_L5 * nLadder_L5 +
                       nSensor_L6* nLadder_L6) * nPN;


    short getSVDSensorID(const unsigned short layer,
                         const unsigned short ladder,
                         const unsigned short sensor,
                         const unsigned short pn)
    {

      if (layer < minLayer || maxLayer < layer || 2 <= pn) {
        return -1;
      }

      short sensor_id_base = 0;

      if (layer == 2) {
        if (nLadder[0] <= ladder || nSensor[0] <= sensor) return -2;
        return nPN * nSensor[0] * ladder + nPN * sensor + pn;
      }

      sensor_id_base += nPN * nSensor[0] * nLadder[0];
      if (layer == 3) {
        if (nLadder[1] <= ladder || nSensor[1] <= sensor) return -3;
        return sensor_id_base + nPN * nSensor[1] * ladder + nPN * sensor + pn;
      }

      sensor_id_base += nPN * nSensor[1] * nLadder[1];
      if (layer == 4) {
        if (nLadder[2] <= ladder || nSensor[2] <= sensor) return -4;
        return sensor_id_base + nPN * nSensor[2] * ladder + nPN * sensor + pn;
      }

      sensor_id_base += nPN * nSensor[2] * nLadder[2];
      if (layer == 5) {
        if (nLadder[3] <= ladder || nSensor[3] <= sensor) return -5;
        return sensor_id_base + nPN * nSensor[3] * ladder + nPN * sensor + pn;
      }

      return -9;
    }

    short getSVDSensorID(VxdID id, bool is_u)
    {
      unsigned short layer   = id.getLayerNumber() - 1;
      unsigned short ladder  = id.getLadderNumber();
      //unsigned short sensor  = id.getSensorNumber();
      unsigned short sensor  = 0; // workround solution for DESY beam data
      //unsigned short segment = id.getSegmentNumber();

      unsigned short pn      = (is_u) ? 0 : 1; // 0: p-side, 1: n-side

      //std::cout << "VxdID: " << id.getID() << " = layer: " << layer << ", ladder: " << ladder << ", sensor: " << sensor << ", segment: " << segment << std::endl;

      return getSVDSensorID(layer, ladder, sensor, pn);
    }

  } // namespace SVDPar

} // namespace Belle2

#endif
