/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#include "tracking/trackFindingVXD/sectorMapTools/CompactSecIDs.h"
#include "tracking/dataobjects/FullSecID.h"

#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <list>
#include <utility>

using namespace std;
using namespace Belle2;


TEST(CompactSecIDs, basicCheck)
{
  CompactSecIDs compactSecIds;

  vector< float > uSup = { .25, .5 , .75 };
  vector< float > vSup = { .25, .5 , .75 , .8};

  vector< vector< FullSecID > > sectors;

  sectors.resize(uSup.size() + 1);

  int counter = 0;
  for (unsigned int i = 0; i < uSup.size() + 1 ; i++) {
    sectors[i].resize(vSup.size() + 1);
    for (unsigned int j = 0; j < vSup.size() + 1; j++) {
      sectors[i][j] = FullSecID(VxdID(6, 3 , 2), false, 12 + counter);
      counter ++;
    }
  }
  // succesfully insert
  EXPECT_EQ(sectors.size()*sectors[0].size(),
            compactSecIds.addSectors(uSup, vSup, sectors)) ;

  // succesfully retrieve
  EXPECT_EQ(1, compactSecIds.getCompactID(sectors[0][0]));
  EXPECT_EQ(2, compactSecIds.getCompactIDFastAndDangerous(sectors[0][1]));

  // unsuccesfully retrieve
  EXPECT_EQ(0, compactSecIds.getCompactID(FullSecID(VxdID(6, 3 , 2), false, 1)));
  EXPECT_EQ(0, compactSecIds.getCompactID(FullSecID(VxdID(7, 3 , 2), false, 1)));
  EXPECT_EQ(0, compactSecIds.getCompactID(FullSecID(VxdID(5, 3 , 2), false, 1)));

  // cannot insert twice
  EXPECT_EQ(0, compactSecIds.addSectors(uSup, vSup, sectors));



}


TEST(CompactSecIDs, stressTest)
{

  CompactSecIDs compactSecIds;
  vector<int> layers  = { 1, 2, 3, 4, 5, 6};
  vector<int> ladders = { 8, 12, 7, 10, 12, 16};
  vector<int> sensors = { 2, 2, 2, 3, 4, 5};


  vector< float > uSup = { .25, .5 , .75 };
  vector< float > vSup = { .2, .4 , .6 , .8};


  vector< vector< FullSecID > > sectors;

  sectors.resize(uSup.size() + 1);


  for (auto layer : layers)
    for (int ladder = 1 ; ladder <= ladders[layer - 1] ; ladder++)
      for (int sensor = 1 ; sensor <=  sensors[layer - 1] ; sensor++) {
        int counter = 0;
        for (unsigned int i = 0; i < uSup.size() + 1; i++) {
          sectors[i].resize(vSup.size() + 1);
          for (unsigned int j = 0; j < vSup.size() + 1 ; j++) {
            sectors[i][j] = FullSecID(VxdID(layer, ladder , sensor),
                                      false, counter);
            counter ++;
          }
        }

        // succesfully insert
        EXPECT_EQ(sectors.size()*sectors[0].size()  , compactSecIds.addSectors(uSup , vSup, sectors));
      }

  for (auto layer : layers)
    for (int ladder = 1 ; ladder <= ladders[layer - 1] ; ladder++)
      for (int sensor = 1 ; sensor <=  sensors[layer - 1] ; sensor++) {
        int counter = 0;
        for (unsigned int i = 0; i < uSup.size() + 1 ; i++) {
          sectors[i].resize(vSup.size() + 1);
          for (unsigned int j = 0; j < vSup.size() + 1; j++) {
            sectors[i][j] = FullSecID(VxdID(layer, ladder , sensor),
                                      false, counter);
            counter ++;
          }
        }

        // un succesfully insert
        EXPECT_EQ(0  , compactSecIds.addSectors(uSup , vSup, sectors));
      }

  int expected = 0;

  for (auto layer : layers)
    for (int ladder = 1 ; ladder <= ladders[layer - 1] ; ladder++)
      for (int sensor = 1 ; sensor <=  sensors[layer - 1] ; sensor++)
        for (unsigned int sector = 0 ; sector < (uSup.size() + 1) * (vSup.size() + 1) ; sector ++) {
          FullSecID aSector(VxdID(layer, ladder , sensor), true, sector);
          // succesfully retrieve
          EXPECT_EQ(++ expected, compactSecIds.getCompactID(aSector));

        }

  //  list< pair< float, float> >;

  for (float u = 0. ; u < 1.; u += .25) {
    for (float v = 0.; v < 1.; v += .2)
      cout << compactSecIds.getFullID(VxdID(1, 1 , 1), u, v) << "\t";
    cout << endl;
  }

  for (float u = 1.e-6 ; u < 1.3; u += .25) {
    for (float v = 1.e-6; v < 1.3; v += .2)
      cout << compactSecIds.getFullID(VxdID(1, 1 , 1), u, v) << "\t";
    cout << endl;
  }

  for (float u = .25 ; u < 1.25; u += .25) {
    for (float v = 0.2; v < 1.2; v += .2)
      cout << compactSecIds.getFullID(VxdID(1, 1 , 1), u, v) << "\t";
    cout << endl;
  }

}
