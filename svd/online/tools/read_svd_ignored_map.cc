/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <stdio.h>
#include <svd/online/SVDIgnoredStripsMap.h>
#include <vxd/dataobjects/VxdID.h>
#include <set>


int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Hello!\n");
    printf("Usage: %s filename.xml\n read filename.xml\n", argv[0]);
    return 0;
  }

  Belle2::SVDIgnoredStripsMap test(argv[1]);
  for (int plane = 3; plane < 7; ++ plane) {
    Belle2::VxdID sensorID(plane, 1, plane);
    printf("Plane %i, side v:\n", plane);
    for (unsigned short ignored : test.getIgnoredStrips(sensorID))
      printf("Ignored: %i\n", ignored);
    sensorID.setSegmentNumber(1);
    printf("Plane %i, side u:\n", plane);
    for (unsigned short ignored : test.getIgnoredStrips(sensorID))
      printf("Ignored: %i\n", ignored);
  }
}
