#include "analysis/VertexFitting/TreeFitter/FitParams.h"
#include <Eigen/Dense>

#include <gtest/gtest.h>


namespace Belle2 {
  /** Test fixture. */
  class TreeFitterFitparTest : public ::testing::Test {
  protected:
  };

  TEST_F(TreeFitterFitparTest, Constructor)
  {
    TreeFitter::FitParams fitParDim3(3);

    /** all elments of the statevector initialised to 0  */
    EXPECT_TRUE((fitParDim3.getStateVector().array() == 0.0).all());

    /** all covariance matrix elemtns initalised to 0 */
    EXPECT_TRUE((fitParDim3.getCovariance().array() == 0.0).all());
  }

  TEST_F(TreeFitterFitparTest, Functions)
  {
    TreeFitter::FitParams fitParDim3(3);

    fitParDim3.getStateVector() << 1, 2, 3;
    fitParDim3.resetStateVector();
    EXPECT_TRUE((fitParDim3.getStateVector().array() == 0.0).all());


    fitParDim3.getCovariance() << 1, 2, 3, 4, 5, 6, 7, 8, 9;
    fitParDim3.resetCovariance();
    EXPECT_TRUE((fitParDim3.getCovariance().array() == 0.0).all());

    EXPECT_FALSE(fitParDim3.testCovariance());

    fitParDim3.getCovariance() = Eigen::MatrixXd::Random(3, 3);

  }

}  // namespace
