/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/modules/PIDCalibrationWeightCreator/PIDCalibrationWeightCreatorModule.h>

// framework aux
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(PIDCalibrationWeightCreator);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PIDCalibrationWeightCreatorModule::PIDCalibrationWeightCreatorModule() : Module()

{
  setDescription("Creates the calibration weight matrix");
  addParam("matrixName", m_matrixName, "Name of the WeightMatrix");
  addParam("weightMatrix", m_weightMatrix, "6x6 WeightMatrix");
  addParam("experimentLow", m_experimentLow, "Interval of validity, ex.low");
  addParam("experimentHigh", m_experimentHigh, "Interval of validity, ex.high");
  addParam("runLow", m_runLow, "Interval of validity, run low");
  addParam("runHigh", m_runHigh, "Interval of validity, run high");
}


void PIDCalibrationWeightCreatorModule::initialize()
{
  // Check the size of m_weightMatrix. It has to be 6x6.
  bool isValid = true;
  if (m_weightMatrix.size() == 6) {
    for (const auto& vec : m_weightMatrix)
      if (vec.size() != 6) {
        isValid = false;
        break;
      }
  } else {
    isValid = false;
  }

  if (!isValid)
    B2ERROR("The given weight matrix size is invalid! It must be 6 (particle types) x 6 (detectors)!");

  Belle2::DBImportObjPtr<Belle2::PIDCalibrationWeight> importer{m_matrixName};
  importer.construct(m_weightMatrix);
  importer.import(Belle2::IntervalOfValidity(m_experimentLow, m_runLow, m_experimentHigh, m_runHigh));
}
