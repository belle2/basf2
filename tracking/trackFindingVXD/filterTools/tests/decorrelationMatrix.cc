/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <gtest/gtest.h>

#include <tracking/trackFindingVXD/filterTools/DecorrelationMatrix.h>
#include <tracking/trackFindingVXD/filterTools/DecorrelationMatrixHelper.h>

#include <Eigen/Dense>

#include <fstream>
#include <cstdio>

using namespace std;
using namespace Belle2;

namespace VXDTFFilterTest {

  /** typedef for less typing effort */
  using TestMatrix = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>;

  /** MATLAB generated random vector */
  const std::vector<double> v1 = {
    0.537667139546100,
    1.833885014595086,
    -2.258846861003648,
    0.862173320368121,
    0.318765239858981,
    -1.307688296305273,
    -0.433592022305684,
    0.342624466538650,
    3.578396939725760,
    2.769437029884877
  };

  /** MATLAB generated random vector */
  const std::vector<double> v2 = {
    -1.349886940156521,
    3.034923466331855,
    0.725404224946106,
    -0.063054873189656,
    0.714742903826096,
    -0.204966058299775,
    -0.124144348216312,
    1.489697607785465,
    1.409034489800479,
    1.417192413429614
  };

  /** MATLAB generated random vector */
  const std::vector<double> v3 = {
    0.671497133608080,
    -1.207486922685038,
    0.717238651328838,
    1.630235289164729,
    0.488893770311789,
    1.034693009917860,
    0.726885133383238,
    -0.303440924786016,
    0.293871467096658,
    -0.787282803758638
  };

  /// Test Class
  class DecorrelationMatrixTest : public ::testing::Test {
  protected:
    /**
     * Fills the data into the internal data structure that is used for testing.
     *
     * NOTE: data created with MATLAB and imported here hardcoded
     */
    virtual void SetUp()
    {
      m_testData = {{v1, v2, v3}};

      TestMatrix tmpMatrix;
      tmpMatrix << 3.132492133948475, 0.974827954209597, -0.761264020048923,
                0.974827954209597, 1.486186070946439, -0.840189849104485,
                -0.761264020048923, -0.840189849104485, 0.739017883637750;

      // MATLAB calculates the empirical covariance matrix with a prefactor (n-1)^-1, but we do with a prefactor of n^-1 so this has
      // to be corrected (n=3 in the test cases)
      m_covMatrix = (double)(v3.size() - 1) / (double)v3.size() * tmpMatrix;
    }

    /** data that is used in the tests*/
    std::array<std::vector<double>, 3> m_testData;

    /** covariance matrix of the data as calculated via MATLAB */
    TestMatrix m_covMatrix;

    /** provide the identity matrix as class-member since it is used in many testcases */
    TestMatrix m_identity = TestMatrix::Identity();
  };

  /** basic test of correct SetUp */
  TEST_F(DecorrelationMatrixTest, TestSetUp)
  {
    EXPECT_EQ(3, m_testData.size());
    for (const auto& vec : m_testData) { EXPECT_EQ(10, vec.size()); }
  }

  /** test if the default constructor initializes the internal matrix to the identity matrix */
  TEST_F(DecorrelationMatrixTest, TestConstructor)
  {
    DecorrelationMatrix<3> matrix{};
    const auto& internalMatrix = matrix.getMatrix();

    for (auto i = 0; i < internalMatrix.outerSize(); ++i) {
      for (auto j = 0; j < internalMatrix.innerSize(); ++j) {
        EXPECT_DOUBLE_EQ(m_identity(i, j), internalMatrix(i, j));
      }
    }
  }

  /** test if the decorrelation matrix is correctly calculated from a given covariance matrix */
  TEST_F(DecorrelationMatrixTest, TestCalculateDecorrMatrix)
  {
    DecorrelationMatrix<3> matrix{};
    matrix.calculateDecorrMatrix(m_testData);

    // test that the size of the input and the output vector does not change
    std::vector<double> testVec = { m_testData[0][0], m_testData[1][0], m_testData[2][0] };
    auto outputVec = matrix.decorrelate(testVec);
    EXPECT_EQ(testVec.size(), outputVec.size());

    // test that the covariance matrix of the decorrelated test data is indeed the identity (at least numerically)
    auto outputData = matrix.decorrelate(m_testData);
    auto covMat = calculateCovMatrix(outputData);
    for (auto i = 0; i < covMat.outerSize(); ++i) {
      for (auto j = 0; j < covMat.innerSize(); ++j) {
        if (i == j) {
          EXPECT_FLOAT_EQ(m_identity(i, j), covMat(i, j)); // the diagonal should be ones, even with considering numerics
        } else {
          EXPECT_NEAR(m_identity(i, j), covMat(i, j), 1e-15); // EXPECT_FLOAT_EQ is to stringent for the numerical calculations
        }
      }
    }

    // COULDDO: test non-normalized version as well
  }

  /** test if the calculation of the covariance matrix is correct */
  TEST_F(DecorrelationMatrixTest, TestCaluclateCovMatrix)
  {
    // test if the correct covariance matrix gets calculated
    const TestMatrix covMat = calculateCovMatrix(m_testData);
    EXPECT_EQ(covMat.outerSize(), 3);
    EXPECT_EQ(covMat.innerSize(), 3);
    for (auto i = 0; i < covMat.outerSize(); ++i) {
      for (auto j = 0; j < covMat.innerSize(); ++j) {
        EXPECT_DOUBLE_EQ(m_covMatrix(i, j), covMat(i, j));
      }
    }

    // check if an identity matrix is returned if the input is incorrect
    auto badTestData = m_testData;
    badTestData[0].erase(badTestData[0].begin()); // remove first element from the first vector in the array
    const TestMatrix badMat = calculateCovMatrix(badTestData);
    EXPECT_EQ(3, badMat.outerSize());
    EXPECT_EQ(3, badMat.innerSize());
    EXPECT_TRUE(badMat == m_identity);
  }

  /** test if the IO methods work as advertised.
   * First write to and then read in again from a temporary file and then compare the written out and read in matrix
   */
  TEST_F(DecorrelationMatrixTest, TestDecorrelationMatrixIO)
  {
    const char* filename = "tmp_matrix_testoutput.dat";

    ofstream ofs(filename);
    DecorrelationMatrix<3> covMatrix(m_covMatrix);
    ofs << covMatrix.print() << std::endl;
    ofs.close();

    ifstream ifs(filename);
    DecorrelationMatrix<3> inMatrix{};
    EXPECT_TRUE(inMatrix.readFromStream(ifs));
    ifs.close();

    const TestMatrix& inMat = inMatrix.getMatrix();
    for (auto i = 0; i < m_covMatrix.outerSize(); ++i) {
      for (auto j = 0; j < m_covMatrix.innerSize(); ++j) {
        EXPECT_DOUBLE_EQ(m_covMatrix(i, j), inMat(i, j));
      }
    }

    ASSERT_EQ(0, remove(filename)); // assert that the temporarily created file gets deleted again
  }
}
