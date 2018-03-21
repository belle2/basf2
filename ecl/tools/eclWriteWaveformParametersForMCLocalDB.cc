#include <framework/database/DBImportObjPtr.h>
#include <ecl/dbobjects/ECLDigitWaveformParametersForMC.h>
#include <iostream>
#include <fstream>
/*
MC photon parameters are given as averaged value.
Diode and hadron template parameters are computed using
eclComputePulseTemplates_StepX.cc  scripts, consistent with procedure for data parameters.
*/

int main()
{

  Belle2::DBImportObjPtr<Belle2::ECLDigitWaveformParametersForMC> importer("ECLDigitWaveformParametersForMC");
  importer.construct();

  const float PhotonMCPar11[11] = {27.7221, 0.5, 0.648324, 0.401711, 0.374167, 0.849417, 0.00144548, 4.70722, 0.815639, 0.555605, 0.2752};
  const float HadronMCPar11[11] = {29.5092, 0.542623, 0.929354, 0.556139, 0.446967, 0.140175, 0.0312971, 3.12842, 0.791012, 0.619416, 0.385621};
  const float DiodeMCPar11[11] = {28.7801, 0.578214, 0.00451387, 0.663087, 0.501441, 0.12073, 0.029675, 3.0666, 0.643883, 0.756048, 0.509381};

  importer->setTemplateParameters(PhotonMCPar11, HadronMCPar11, DiodeMCPar11);
  importer.import(Belle2::IntervalOfValidity::always());

  std::cout << "Successfully wrote payload ECLDigitWaveformParametersForMC to local DB" << std::endl;

  return 1;

}
