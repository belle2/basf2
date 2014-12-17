/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *               Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <tracking/vectorTools/B2Vector3.h>


using namespace std;

namespace Belle2 {

  namespace B2Vector3Tests {

    /** Tests Sanity of B2Vector3 and compares compatibility with TVector3 */
    class B2Vector3Test : public ::testing::Test {
    public:
    protected:
    };


    /** Test naming.
    */
    TEST_F(B2Vector3Test, testNaming)
    {
      EXPECT_EQ("Belle2::B2Vector3<int>" , B2Vector3<int>().name());
      EXPECT_EQ("Belle2::B2Vector3<bool>" , B2Vector3<bool>().name());
      EXPECT_EQ("Belle2::B2Vector3<double>" , B2Vector3<double>().name());
      EXPECT_EQ("Belle2::B2Vector3<double>" , B2Vector3D().name());
      EXPECT_EQ("Belle2::B2Vector3<float>" , B2Vector3F().name());
    }


    /** Test constructors and essential member access
    */
    TEST_F(B2Vector3Test, testConstructors)
    {
      // empty constructor:
      EXPECT_FALSE(B2Vector3<bool>().X()); // filled 3 times false
      EXPECT_EQ(0, B2Vector3<unsigned int>().Y());
      EXPECT_EQ(0., B2Vector3<double>().Z());

      // constructor passing 3 coordinates
      EXPECT_FLOAT_EQ(1.2, B2Vector3<float>(1.2, 2.3, 3.4).X());
      EXPECT_FALSE(B2Vector3<bool>(false, false, false).Y());
      EXPECT_TRUE(B2Vector3<bool>(true, true, true).Z());


      // constructor with hard copy:
      B2Vector3<int> testB2Vector({1, 2, 3});
      EXPECT_EQ(3, testB2Vector.z());

      // constructor with passing by reference
      float testArray[3] = {1, 2, 3} ;
      B2Vector3<float> testB2Vector2(testArray);
      EXPECT_EQ(2.f, testB2Vector2.y());

      // constructor with passing by pointer
      B2Vector3<float> testB2Vector3(&testArray);
      EXPECT_EQ(3.f, testB2Vector3.z());

      // constructor accepting a TVector3, or a pointer to it
      TVector3 testTVector3(1. / 3., 1, 0);
      EXPECT_DOUBLE_EQ(testTVector3.x(), B2Vector3<double>(testTVector3).x());
      EXPECT_FLOAT_EQ(testTVector3.y(), B2Vector3<float>(&testTVector3).y());
      EXPECT_TRUE(B2Vector3<bool>(testTVector3).Y());
      EXPECT_FALSE(B2Vector3<bool>(testTVector3).Z());

      // constructor accepting another B2Vector3 of the same type, or a pointer to it
      B2Vector3D testB2Vector3D(1. / 3., 1, 0);
      EXPECT_DOUBLE_EQ(testB2Vector3D.x(), B2Vector3<double>(testB2Vector3D).x());
      EXPECT_DOUBLE_EQ(testB2Vector3D.y(), B2Vector3<double>(&testB2Vector3D).y());

      // constructor accepting another B2Vector3 carrying a different type
      EXPECT_FLOAT_EQ(B2Vector3<double>(testB2Vector3D).x(), B2Vector3<float>(testB2Vector3D).x());
      EXPECT_NE(B2Vector3<double>(testB2Vector3D).x(), B2Vector3<float>(testB2Vector3D).x());
      EXPECT_TRUE(B2Vector3<bool>(testB2Vector3D).X());
      EXPECT_TRUE(B2Vector3<bool>(testB2Vector3D).Y());
      EXPECT_FALSE(B2Vector3<bool>(testB2Vector3D).Z());
    }



    /** Test access and manipulating coordinates of the B2Vector3
    */
    TEST_F(B2Vector3Test, testGetterSetter)
    {
      TVector3 testTVector3(1. / 3., 1, 0);

      EXPECT_EQ(testTVector3, B2Vector3D(testTVector3).GetTVector3());

      B2Vector3D testXYZ = B2Vector3D();

      testXYZ.SetX(23.);
      EXPECT_EQ(23., testXYZ.X());
      EXPECT_EQ(testXYZ.X(), testXYZ.at(0));

      testXYZ.SetY(42.);
      EXPECT_EQ(42., testXYZ.Y());
      EXPECT_EQ(testXYZ.Y(), testXYZ(1));

      testXYZ.SetZ(0.5);
      EXPECT_EQ(0.5, testXYZ.Z());
      EXPECT_EQ(testXYZ.Z(), testXYZ[2]);

      testXYZ.SetXYZ(1, 2, 3);
      EXPECT_EQ(2, testXYZ.Y());

      testXYZ.SetXYZ(testTVector3);
      EXPECT_EQ(testTVector3, testXYZ.GetTVector3());

      testXYZ.SetXYZ(&testTVector3);
      EXPECT_EQ(testTVector3, testXYZ.GetTVector3());

      EXPECT_B2FATAL(testXYZ.at(-1));

      EXPECT_B2FATAL(testXYZ(-1));

      EXPECT_B2FATAL(testXYZ[-1]);
    }



    /** Test operators of the B2Vector3
    */
    TEST_F(B2Vector3Test, testOperators)
    {
      //test assignment and comparison and convertibility to TVector3:
      TVector3 tVecA(1. / 3., 23., 42.);
      B2Vector3D vecA(tVecA.X(), tVecA.Y(), tVecA.Z());
      B2Vector3D vecB(vecA);
      B2Vector3D vecC = vecA;
      B2Vector3D vecD(tVecA);
      TVector3 tVecB(vecA);

      EXPECT_EQ(vecA, vecB);
      EXPECT_EQ(vecA, vecC);
      EXPECT_EQ(vecA, vecD);
      EXPECT_EQ(vecA, tVecA);
      EXPECT_EQ(vecD, tVecA);
      EXPECT_EQ(tVecA, vecA);
      EXPECT_EQ(tVecA, vecD);
      EXPECT_EQ(tVecA, tVecB);

      vecD.SetZ(3.14);
      EXPECT_NE(vecA, vecD);

      vecD.SetXYZ(vecA); // reset

// //     EXPECT_DOUBLE_EQ( 2.* vecA, vecA + vecA);
// //     EXPECT_DOUBLE_EQ( vecA * 2., vecA + vecA);
// //     EXPECT_DOUBLE_EQ( tVecA + tVecA, vecA + vecA);
      EXPECT_DOUBLE_EQ(tVecA * tVecA, vecA * vecA);

    }
  }  // namespace B2Vector3Tests

}  // namespace Belle2
