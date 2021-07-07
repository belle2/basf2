/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <stdio.h>
#include <svd/online/SVDOnlineToOfflineMap.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <vxd/dataobjects/VxdID.h>

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Hello! Hello!\n");
    printf("Usage: %s filename.xml\n read filename.xml\n", argv[0]);
    return 0;
  }

  Belle2::SVDOnlineToOfflineMap test(argv[1]);
  short sample[6] = {0, 0, 0, 10000, 0, 0};
  Belle2::SVDShaperDigit* digit = test.NewShaperDigit(129, 6, 119, sample, 100);
  printf("Layer/Ladder/Sensor: %i/%i/%i ",
         digit->getSensorID().getLayerNumber(),
         digit->getSensorID().getLadderNumber(),
         digit->getSensorID().getSensorNumber());
  printf("%s", digit->isUStrip() ? "u" : "v");
  printf("-strip: %i ", digit->getCellID());
  printf("MaxCharge: %f ", double(digit->getMaxADCCounts()));
  //printf("Time: %i ", digit->getIndex());
  printf("\n");

  printf("Find (layer, ladder, sensor, side, strip) for (FADC, APV, channel)\n");
  auto info = test.getSensorInfo(129, 6);
  short strip = test.getStripNumber(119, info);
  printf("Layer/Ladder/Sensor: %i/%i/%i ",
         info.m_sensorID.getLayerNumber(),
         info.m_sensorID.getLadderNumber(),
         info.m_sensorID.getSensorNumber()
        );
  printf("%s", info.m_uSide ? "u" : "v");
  printf("-strip: %i ", strip);
  printf("\n");

}
