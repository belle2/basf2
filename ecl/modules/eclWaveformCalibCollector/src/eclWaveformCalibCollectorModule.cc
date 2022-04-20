/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module`
#include <ecl/modules/eclWaveformCalibCollector/eclWaveformCalibCollectorModule.h>

//Framework
#include <framework/gearbox/Const.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationVector.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>


#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclWaveformCalibCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------

eclWaveformCalibCollectorModule::eclWaveformCalibCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Module to export waveforms to ntuple for template calibration.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("LowEnergyThresholdGeV", m_LowEnergyThresholdGeV, "Low Energy Threshold in GeV.", 1.0);
  addParam("HighEnergyThresholdGeV", m_HighEnergyThresholdGeV, "High Energy Threshold in GeV.", 5.5);
  addParam("IncludeWaveforms", m_includeWaveforms, "Flag to save ADC information.", true);
}



/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclWaveformCalibCollectorModule::prepare()
{

}


/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclWaveformCalibCollectorModule::collect()
{
}
