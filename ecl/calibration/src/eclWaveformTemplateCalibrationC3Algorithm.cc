/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclWaveformTemplateCalibrationC3Algorithm.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>

/* ROOT headers. */
#include <TFile.h>
#include <TTree.h>

/* C++ headers. */
#include <cstdlib>

using namespace std;
using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclWaveformTemplateCalibrationC3Algorithm::eclWaveformTemplateCalibrationC3Algorithm():
  CalibrationAlgorithm("eclWaveformTemplateCalibrationC2Collector")
{
  setDescription(
    "Perform the hadron and diode template shape calibration using photon template shapes from stage C2"
  );
}

CalibrationAlgorithm::EResult eclWaveformTemplateCalibrationC3Algorithm::calibrate()
{

  B2INFO("RUNNING" << Form("eclComputePulseTemplates_Step2.py %d %d", (m_firstCellID), (m_lastCellID)));
  system(Form("eclComputePulseTemplates_Step2.py %d %d", (m_firstCellID), (m_lastCellID)));

  B2INFO("RUNNING " << Form("eclComputePulseTemplates_Step3 %d %d", m_firstCellID, m_lastCellID));
  system(Form("eclComputePulseTemplates_Step3 %d %d", m_firstCellID, m_lastCellID));

  //save to db
  ECLDigitWaveformParameters* HadronDiodeParameters = new ECLDigitWaveformParameters();

  TFile* TempFile = new TFile(Form("HadronPars_Low%d_High%d.root", m_firstCellID, m_lastCellID), "READ");
  TTree* TempTree = (TTree*)  TempFile->Get("HadronWaveformInfo");
  double tHadronShapePars_A[11];
  double tDiodeShapePars_A[11];
  double tMaxResidualHadron_A;
  double tMaxResidualDiode_A;
  double tMaxValDiode_A;
  double tMaxValHadron_A;
  TempTree->SetBranchAddress("TempHadronPar11_A", &tHadronShapePars_A);
  TempTree->SetBranchAddress("TempDiodePar11_A", &tDiodeShapePars_A);
  TempTree->SetBranchAddress("MaxResHadron_A", &tMaxResidualHadron_A);
  TempTree->SetBranchAddress("MaxResDiode_A", &tMaxResidualDiode_A);
  TempTree->SetBranchAddress("MaxValDiode_A", &tMaxValDiode_A);
  TempTree->SetBranchAddress("MaxValHadron_A", &tMaxValHadron_A);

  int batch = m_lastCellID - m_firstCellID;
  for (int j = 0; j <= batch; j++) {
    int tCellID = m_firstCellID + j;
    if (tCellID > ECLElementNumbers::c_NCrystals)continue;
    B2INFO("tCellID=" << tCellID << " j=" << j);
    TempTree->GetEntry(j);
    float tHadronShapePars_float[11];
    float tDiodeShapePars_float[11];
    for (int p = 0; p < 11; p++) {

      if (tHadronShapePars_A[p] > 100 || tHadronShapePars_A[p] < -100) {
        B2INFO("Warning  large parameter for: " << tCellID << " " << tHadronShapePars_A[p]);
        for (int h = 0; h < 11; h++) B2INFO(tHadronShapePars_A[h]);
      }

      tHadronShapePars_float[p] = (float)tHadronShapePars_A[p];
      tDiodeShapePars_float[p] = (float)tDiodeShapePars_A[p];

    }

    B2INFO("tCellID tHadronShapePars_float[0]" << tCellID << " " << tHadronShapePars_float[0]);

    HadronDiodeParameters->setTemplateParameters(tCellID, tHadronShapePars_float, tHadronShapePars_float, tDiodeShapePars_float);
  }
  TempFile->Close();

  saveCalibration(HadronDiodeParameters, Form("HadronDiodeParameters_CellID%d_CellID%d", m_firstCellID, m_lastCellID));
  return c_OK;
}

