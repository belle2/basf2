#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/SectorTFInfo.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/sectorMapTools/SectorTools.h>

#include "tracking/dataobjects/FullSecID.h"

// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>


#include <vector>
#include <utility>
#include <TVector3.h>


using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class SectorToolsTest : public ::testing::Test {




  protected:
  };


  /** ***** searchSector4Hit ***** **/
/// searches for sectors fitting current hit coordinates, returns blank string if nothing could be found
  unsigned short searchSector4Hit(VxdID aVxdID,
                                  TVector3 localHit,
                                  TVector3 sensorSize,
                                  vector<float>& uConfig,
                                  vector<float>& vConfig)
  {
    unsigned short aSecID = numeric_limits<unsigned short>::max();;
    unsigned int aFullSecID = numeric_limits<unsigned int>::max();

//   B2INFO ("searchSector4Hit: localHit: " << localHit.X() << "/" << localHit.Y() << "/" << localHit.Z());
//   B2INFO ("searchSector4Hit: sensorSize: " << sensorSize.X() << "/" << sensorSize.Y() << "/" << sensorSize.Z());

    for (int j = 0; j != int(uConfig.size() - 1); ++j) {
//     B2INFO( "uCuts(j)*uSize: " << uConfig.at(j)*sensorSize[0] << " uCuts(j+1)*uSize: " << uConfig.at(j + 1)*sensorSize[0]);

      //  if (localHit[0] >= (uConfig[j]*sensorSize[0] * 2.) && localHit[0] <= (uConfig[j + 1]*sensorSize[0] * 2.)) {
      if (localHit[0] >= (uConfig[j]*sensorSize[0]) && localHit[0] <= (uConfig[j + 1]*sensorSize[0])) {
        for (int k = 0; k != int(vConfig.size() - 1); ++k) {
//         B2INFO (" vCuts(k)*vSize: " << vConfig.at(k)*sensorSize[1] << " vCuts(k+1)*vSize: " << vConfig.at(k + 1)*sensorSize[1]);

          // if (localHit[1] >= (vConfig[k]*sensorSize[1] * 2.) && localHit[1] <= (vConfig[k + 1]*sensorSize[1] * 2.)) {

          if (localHit[1] >= (vConfig[k]*sensorSize[1]) && localHit[1] <= (vConfig[k + 1]*sensorSize[1])) {

//    B2INFO (" FOUND: " << (vConfig[k]*sensorSize[1] * 2.) << " vCuts(k+1)*vSize: " << (vConfig[k + 1]*sensorSize[1] * 2.) );


            aSecID = k + 1 + j * (vConfig.size() - 1);
//    B2INFO (" FOUND: k: " << k << ", j: " << j);

            aFullSecID = FullSecID(aVxdID, false, aSecID).getFullSecID();
            aFullSecID = aFullSecID;
          }
        }
      }
    } //sector-searching loop

    return aSecID;
    //return aFullSecID;
  }


  /** Test simple Setters and Getters. */
//   TEST_F(SectorToolsTest, testEmptyTool)
//   {
//     SectorTools aTool = SectorTools();
//
//   }

  TEST_F(SectorToolsTest, testPointsSectorTF)
  {
    TVector3 testPoint(2, 1, 3);

    SectorTFInfo aSector = SectorTFInfo();
    aSector.setSectorID(12);
    aSector.setPoint(0, testPoint);

    ASSERT_EQ(12., aSector.getSectorID());
    ASSERT_EQ(testPoint, aSector.getPoint(0));

  }

  TEST_F(SectorToolsTest, testConvert)
  {

    TVector3 testPoint, testPointGlobal;
    std::pair<float, float> hitLocalTest;

    //Coor asecid: 990511105, position Hit: 3.02141/2.42389/-4.97057, uCoord: 1.66035, vCoord: 4.43043, aVxdID: 24896
    hitLocalTest.first = 1.66035;
    hitLocalTest.second = 4.43043;

    VxdID aVxdID(24896);

    VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, 3.852, 12.002, 0.032, 0, 0);

    testPoint = SpacePoint::getGlobalCoordinates(hitLocalTest, aVxdID, &sensorInfoBase);

//     B2INFO ("HALLO : " << testPoint.X() << " / " << testPoint.Y() << " / " << testPoint.Z());

    testPointGlobal = sensorInfoBase.pointToGlobal(testPoint);

    ASSERT_EQ(testPoint, testPointGlobal);
  }



  TEST_F(SectorToolsTest, testSectorCalcID)
  {
    SectorTools aTool = SectorTools();

    // Config from the TrackFinder
    std::vector<float> uConfigTest = {0, 0.5, 1};
    std::vector<float> vConfigTest = {0, 0.33, 0.67, 1};

//  getUSize: 3.852
//  getVSize: 12.002
    TVector3 sensorSize(3.852, 12.002, 0);

    std::pair<float, float> aRelCoor2;
    unsigned short aSecID;

    // Test Cases
    // TVector3       aSecID  aVxdID
    //     3.65526  4.43185 0 5 24896
    //     3.65526  8.22384 0 6 24896
    //     2.4668 11.7496 0 6 25152
    //     3.26142  2.25991 0 4 25888
    //     1.88061  1.92809 0 1 26144
    //     1.88061  5.65315 0 2 26144

    std::vector<unsigned short> compareID = {24896, 24896, 25152, 25888, 26144, 26144};

    std::vector<std::pair<float, float>> aRelCoor = {{3.65526, 4.43185}, {3.65526, 8.22384}, {2.4668, 11.7496}, {3.26142, 2.25991}, {1.88061, 1.92809}, {1.88061, 5.65315}};

    std::vector<unsigned short> compareSecID = {5, 6, 6, 4, 1, 2};

    for (uint currentTest = 0; currentTest < compareID.size(); currentTest++) {

      //       B2INFO ("currentTest: " << currentTest);

      VxdID aVxdID(compareID.at(currentTest));

      //  getUSize: 3.852
      //  getVSize: 12.002
      //  getThickness: 0.032
      VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, 3.852, 12.002, 0.032, 0, 0);

      //     B2INFO ("aSecID Start: " << aRelCoor.first << "/" << aRelCoor.second);

      // std::pair<float, float> SpacePoint::convertToLocalCoordinates(const std::pair<float, float>& hitNormalized, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo)
      //aRelCoor = SpacePoint::convertToLocalCoordinates (aRelCoor, aVxdID, &sensorInfoBase);
      TVector3 m_position = sensorInfoBase.pointToGlobal(TVector3(aRelCoor.at(currentTest).first, aRelCoor.at(currentTest).second, 0));

      //     B2INFO ("aSecID Global Corr: " << m_position.X() << "/" << m_position.Y() << "/" << m_position.Z());

      unsigned short withSearchHit = searchSector4Hit(aVxdID, m_position, sensorSize, uConfigTest, vConfigTest);

      //      B2INFO ("withSearchHit: " << withSearchHit);

      aRelCoor2 = SpacePoint::convertToLocalCoordinates(aRelCoor.at(currentTest), aVxdID, &sensorInfoBase);

      aRelCoor2 = SpacePoint::convertToNormalizedCoordinates(aRelCoor2, aVxdID, &sensorInfoBase);

      //     B2INFO ("aSecID X: " << aRelCoor2.first << "/" << aRelCoor2.second);

      aSecID = aTool.calcSecID(uConfigTest, vConfigTest, aRelCoor2);

      // Compare searchSector4Hit <=> calcSecID
      ASSERT_EQ(withSearchHit, aSecID);

      // Compare Stored SecID <=> calcSecID
      ASSERT_EQ(compareSecID.at(currentTest) , aSecID);

      //      B2INFO ("aSecID X: " << aSecID);
    }

  }




  TEST_F(SectorToolsTest, testSectorCoordinates)
  {
    SectorTools aTool = SectorTools();

    //  std::pair<float, float> = RelCoords;
    std::pair<float, float> aRelCoor_corner1 = {0, 0};
    std::pair<float, float> aRelCoor_corner2 = {0, 1};
    std::pair<float, float> aRelCoor_corner3 = {1, 0};
    std::pair<float, float> aRelCoor_corner4 = {1, 1};

    std::pair<float, float> aRelCoor = {3.65526, 4.43185}, aRelCoor2;
    unsigned short aSectorID;

    std::vector<float> uConfigTest = {0, 0.5, 1};
    std::vector<float> vConfigTest = {0, 0.33, 0.67, 1};

    // static NormCoords calcNormalizedSectorPoint(const std::vector<float>& uConfig, const std::vector<float>& vConfig, uShort secID, NormCoords coords)

    // Coor asecid: 990511105
    // Point 1: (7.325, -1.367, -5.326) Point 2: (7.325, -1.367, -3.400) Point 3: (4.353, 1.251, -5.326) Point 4: (4.353, 1.251, -3.400)
    // Coor asecid: 990511105, position Hit: 3.02134/2.42394/-4.96915, uCoord: 1.66044, vCoord: 4.43185, aVxdID: 24896
    aSectorID = 24896;
    VxdID aVxdID(aSectorID);

    VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, 3.852, 12.002, 0.032, 0, 0);

    aRelCoor2 = SpacePoint::convertToLocalCoordinates(aRelCoor, aVxdID, &sensorInfoBase);
    aRelCoor2 = SpacePoint::convertToNormalizedCoordinates(aRelCoor2, aVxdID, &sensorInfoBase);

    unsigned short aSecID = aTool.calcSecID(uConfigTest, vConfigTest, aRelCoor2);


    aRelCoor = aTool.calcNormalizedSectorPoint(uConfigTest, vConfigTest, aSecID, aRelCoor_corner1);
//     B2INFO ("COOR: u: " << aRelCoor.first << ", v: " << aRelCoor.second);
    ASSERT_EQ(float(0.5), aRelCoor.first);
    ASSERT_EQ(float(0.33), aRelCoor.second);

    aRelCoor = aTool.calcNormalizedSectorPoint(uConfigTest, vConfigTest, aSecID, aRelCoor_corner2);
//     B2INFO ("COOR: u: " << aRelCoor.first << ", v: " << aRelCoor.second);

    ASSERT_EQ(float(0.5), aRelCoor.first);
    ASSERT_EQ(float(0.67), aRelCoor.second);

    aRelCoor = aTool.calcNormalizedSectorPoint(uConfigTest, vConfigTest, aSecID, aRelCoor_corner3);
//     B2INFO ("COOR: u: " << aRelCoor.first << ", v: " << aRelCoor.second);

    ASSERT_EQ(float(1.), aRelCoor.first);
    ASSERT_EQ(float(0.33), aRelCoor.second);

    aRelCoor = aTool.calcNormalizedSectorPoint(uConfigTest, vConfigTest, aSecID, aRelCoor_corner4);
//     B2INFO ("COOR: u: " << aRelCoor.first << ", v: " << aRelCoor.second);

    ASSERT_EQ(float(1.), aRelCoor.first);
    ASSERT_EQ(float(0.67), aRelCoor.second);

  }

}
