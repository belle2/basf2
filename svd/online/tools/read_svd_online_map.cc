#include <stdio.h>
#include <svd/online/SVDOnlineToOfflineMap.h>
#include <svd/dataobjects/SVDDigit.h>
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
  Belle2::SVDDigit* digit = test.NewDigit(129, 6, 119, 10000, 100);
  printf("Layer/Ladder/Sensor: %i/%i/%i ",
         digit->getSensorID().getLayerNumber(),
         digit->getSensorID().getLadderNumber(),
         digit->getSensorID().getSensorNumber());
  printf("%s", digit->isUStrip() ? "u" : "v");
  printf("-strip: %i ", digit->getCellID());
  printf("Charge: %f ", digit->getCharge());
  printf("Time: %i ", digit->getIndex());
  printf("\n");

  printf("Find (layer, ladder, sensor, side, strip) for (FADC, APV, channel)\n");
  auto info = test.getChipInfo(129, 6);
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
