/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/utilities/ConditionalGaussGenerator.h>
#include <TRandom.h>


using namespace Belle2;
using Eigen::MatrixXd;
using Eigen::VectorXd;


/** transform matrix into vector of columns */
static std::vector<VectorXd> toVectors(const MatrixXd& mat)
{
  std::vector<VectorXd> vList;
  for (int j = 0; j < mat.cols(); ++j)
    vList.push_back(mat.col(j));

  return vList;
}

/** transform vector of vectors (of columns) into single matrix */
static MatrixXd toMatrix(const std::vector<VectorXd>& vList)
{
  if (vList.size() == 0)
    return MatrixXd();

  MatrixXd mat(vList[0].rows(), vList.size());
  for (unsigned j = 0; j < vList.size(); ++j)
    mat.col(j) = vList[j];

  return mat;
}

/** get the vectors defining linear span orthogonal to vector v0 */
static std::vector<VectorXd> getOrthogonalSpace(VectorXd v0)
{
  if (v0.dot(v0) == 0)
    return toVectors(MatrixXd::Identity(v0.rows(), v0.rows()));

  VectorXd v0Norm = v0.normalized();

  std::vector<VectorXd> vList;
  vList.push_back(v0Norm);

  while (int(vList.size()) < v0.rows()) {

    // Let's first create an empty vector and then fill it with gRandom
    // We can NOT use VectorXd::Random for any reasons
    VectorXd v(v0.rows());
    for (int i = 0; i < v.rows(); ++i)
      v(i) = gRandom->Uniform(-1, 1);

    for (const VectorXd& vi : vList)
      v -= vi.dot(v) * vi / vi.dot(vi);

    if (v.dot(v) > 0) {
      vList.push_back(v.normalized());
    }
  }

  vList.erase(vList.begin()); // note len(vList) >= 1
  return vList;
}

ConditionalGaussGenerator::ConditionalGaussGenerator(const VectorXd& mu, const MatrixXd& covMat) : m_mu(mu), m_covMat(covMat)
{
  Eigen::SelfAdjointEigenSolver<MatrixXd> sol(m_covMat);
  VectorXd vals = sol.eigenvalues();
  MatrixXd vecs = sol.eigenvectors();

  std::vector<VectorXd> vBase;
  //keep only vectors with positive eigenvalue
  for (int i = 0; i < vals.rows(); ++i)
    if (vals[i] > 0) {
      vBase.push_back(sqrt(vals[i]) * vecs.col(i));
    }

  if (vBase.size() == 0) // matrix is zero, no smearing
    return;

  m_vBaseMat = toMatrix(vBase);

  // get the longitudinal space
  VectorXd v0 = m_vBaseMat.row(0);

  // get the orthogonal space
  m_cOrt = getOrthogonalSpace(v0);

  m_v0norm =  v0.dot(v0) > 0 ?  v0 / v0.dot(v0) : v0; //normalize, or keep it zero
}

VectorXd ConditionalGaussGenerator::generate(double x0) const
{
  double dx0 = x0 - m_mu[0];

  // in case of zero cov matrix
  if (m_vBaseMat.cols() == 0)
    return m_mu;

  VectorXd x = m_v0norm * dx0;

  for (const VectorXd& co : m_cOrt) {
    double rr = gRandom->Gaus();
    x += rr * co;
  }

  VectorXd vec = m_vBaseMat * x;

  return (m_mu + vec);
}
