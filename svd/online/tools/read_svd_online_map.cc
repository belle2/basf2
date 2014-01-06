#include <stdio.h>
#include <svd/online/SVDOnlineToOfflineMap.h>
#include <svd/dataobjects/SVDDigit.h>

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
  printf("Strip: %i ", digit->getCellID());
  printf("Charge: %f ", digit->getCharge());
  printf("Time: %f ", digit->getTime());
  printf("\n");
}
