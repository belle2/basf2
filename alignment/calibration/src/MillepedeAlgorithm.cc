#include <alignment/calibration/MillepedeAlgorithm.h>

#include <alignment/PedeApplication.h>
#include <alignment/PedeResult.h>
#include <alignment/dataobjects/PedeSteering.h>
#include <alignment/dbobjects/VXDAlignment.h>
#include <alignment/dbobjects/CDCCalibration.h>
#include <alignment/GlobalLabel.h>

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

  if (mille.getFiles().empty()) {
    B2INFO("No binary files.");
    return c_Failure;
  }

  m_steering.getFiles() = mille.getFiles();

  PedeApplication pede;
  m_result = pede.calibrate(m_steering);

  if (!pede.success() || !m_result.isValid()) {
    B2INFO(pede.getExitMessage());
    return c_Failure;
  }

  auto vxdAlignment = new VXDAlignment();
  auto cdcCalibration = new CDCCalibration();

  double maxCorrectionPull = 0.;
  int maxCorrectionPullLabel = 0;

  for (int ipar = 0; ipar < m_result.getNoParameters(); ipar++) {
    if (!m_result.isParameterDetermined(ipar)) continue;

    GlobalLabel param(m_result.getParameterLabel(ipar));
    double correction = m_result.getParameterCorrection(ipar);
    double error = m_result.getParameterError(ipar);
    double pull = fabs(correction) / error;

    if (pull > maxCorrectionPull) {
      maxCorrectionPull = pull;
      maxCorrectionPullLabel = param.label();
    }

    if (param.isVXD())
      vxdAlignment->set(param.getVxdID(), param.getParameterId() - 1, correction, error);

    if (param.isCDC())
      cdcCalibration->set(param.getWireID(), param.getParameterId() - 1, correction, error);

  }

  saveCalibration(vxdAlignment, "VXDAlignment");
  saveCalibration(cdcCalibration, "CDCCalibration");

  // Iterate until corrections are larger than their errors
  //TODO: We do not update parameters yet, only replace them, so iterations have no effect
  if (maxCorrectionPull >= 1.) {
    B2INFO("Largest correction/error is " << maxCorrectionPull << " for parameter with label " << maxCorrectionPullLabel <<
           ". Requesting iteration.");
    return c_Iterate;
  }

  return c_OK;
}

