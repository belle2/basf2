/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/logging/Logger.h>

#include <framework/utilities/TestHelpers.h>

#include <framework/gearbox/Gearbox.h>

#include <TVector3.h>
#include <filesystem>

#include <boost/math/special_functions/sign.hpp>

#include <cmath>
#include <vector>

using namespace Belle2::TestHelpers;
using namespace std::filesystem;

void TestWithGearbox::SetUpTestCase()
{
  //Setup the gearbox
  Gearbox& gearbox = Gearbox::getInstance();

  std::vector<std::string> backends;
  backends.emplace_back("file:");
  gearbox.setBackends(backends);

  B2INFO("Start open gearbox.");
  gearbox.open("geometry/Belle2.xml");
  B2INFO("Finished open gearbox.");
}

void TestWithGearbox::TearDownTestCase()
{
  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.close();
}

TempDirCreator::TempDirCreator():
  m_oldpwd(current_path().string())
{
  char* temporaryDirName = strdup((temp_directory_path() / "basf2_XXXXXX").c_str());
  auto directory = mkdtemp(temporaryDirName);
  if (!directory) {
    B2ERROR("Cannot create temporary directory: " << strerror(errno));
    free(temporaryDirName);
    return;
  }
  path tmpdir = directory;
  current_path(tmpdir);
  m_tmpdir = tmpdir.string();
  free(temporaryDirName);
}

TempDirCreator::~TempDirCreator()
{
  current_path(m_oldpwd);
  remove_all(m_tmpdir);
}

std::string TempDirCreator::getTempDir() const
{
  return m_tmpdir;
}

bool Belle2::TestHelpers::angleNear(double expected, double actual, double absError)
{
  return fabs(remainder(expected - actual, 2 * M_PI)) < absError;
}

bool Belle2::TestHelpers::sameSign(double expected, double actual)
{
  if (std::isnan(expected) or std::isnan(actual)) return false;
  using boost::math::sign;
  int expectedSign = sign(expected);
  int actualSign = sign(actual);
  return expectedSign == actualSign;
}

bool Belle2::TestHelpers::isPositive(double expected)
{
  return expected > 0;
}

bool Belle2::TestHelpers::isNegative(double expected)
{
  return expected < 0;
}

template<>
bool Belle2::TestHelpers::allNear<ROOT::Math::XYZVector>(const ROOT::Math::XYZVector& expected,
                                                         const ROOT::Math::XYZVector& actual,
                                                         double tolerance)
{
  bool xNear = std::fabs(expected.X() - actual.X()) < tolerance;
  bool yNear = std::fabs(expected.Y() - actual.Y()) < tolerance;
  bool zNear = std::fabs(expected.Z() - actual.Z()) < tolerance;
  return xNear and yNear and zNear;
}

void Belle2::TestHelpers::PrintTo(const TVector3& tVector3, ::std::ostream& output)
{
  output
      << "TVector3("
      << tVector3.X() << ", "
      << tVector3.Y() << ", "
      << tVector3.Z() << ")";
}
