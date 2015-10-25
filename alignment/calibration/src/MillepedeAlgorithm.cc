#include <alignment/calibration/MillepedeAlgorithm.h>

#include <alignment/PedeApplication.h>
#include <alignment/PedeResult.h>
#include <alignment/dataobjects/PedeSteering.h>

using namespace std;
using namespace Belle2;
using namespace alignment;

MillepedeAlgorithm::MillepedeAlgorithm() : CalibrationAlgorithm("MillepedeCollector")
{
  setDescription("Millepede calibration & alignment algorithm");
}

CalibrationAlgorithm::EResult MillepedeAlgorithm::calibrate()
{
  auto& mille = getObject<MilleData>("mille");

  m_steering.getFiles() = mille.getFiles();

  PedeApplication pede;
  m_result = pede.calibrate(m_steering);

  B2INFO("Millepede exit code: " << pede.getExitCode());
  B2INFO(pede.getExitMessage());

  if (pede.success())
    return c_OK;
  else
    return c_Failure;
}

