/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDDatabaseTestModule.h>
#include <framework/logging/Logger.h>

//#include <svd/dbobjects/SVDPulseShapeCalibrations.h>
//#include <svd/dbobjects/SVDLocalRunBadStrips.h>

#include <vxd/dataobjects/VxdID.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDatabaseTest)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDatabaseTestModule::SVDDatabaseTestModule() : Module()
{
  //Set module properties
  setDescription("The SVD Test Module for the database interface");

}

void SVDDatabaseTestModule::initialize()
{ }

/** Get access to calibration results retreived from database */
void SVDDatabaseTestModule::event()
{

  /**The calibration constants stored in the specific
   * dbobjects are accessed by the specific DBObjPtr
   *
   */

  /** Get default values for noise, charge and ADC counts and
   * print them
   */
  VxdID sensorID(3, 1, 1);
  /*
  B2INFO("Noise L3_1_1 side V strip 0 = " << m_obj_noise.getNoise(sensorID, false, 0));
  B2INFO("Noise L3_1_1 side U strip 0 = " << m_obj_noise.getNoise(sensorID , true, 0));
  B2INFO("Test the retrieval of the noise in electrons:");
  B2INFO("Noise L3_1_1 side V strip 0 [electrons]= " << m_obj_noise.getNoiseInElectrons(sensorID, false, 0));
  B2INFO("Noise L3_1_1 side U strip 0 [electrons]= " << m_obj_noise.getNoiseInElectrons(sensorID , true, 0));

  VxdID sensorID_L4(4, 1, 1);
  B2INFO("Noise L4_1_1 side V strip 0 = " << m_obj_noise.getNoise(sensorID_L4, false, 0));
  B2INFO("Noise L4_1_1 side U strip 0 = " << m_obj_noise.getNoise(sensorID_L4 , true, 0));
  B2INFO("Test the retrieval of the noise in electrons:");
  B2INFO("Noise L4_1_1 side V strip 0 [electrons]= " << m_obj_noise.getNoiseInElectrons(sensorID_L4, false, 0));
  B2INFO("Noise L4_1_1 side U strip 0 [electrons]= " << m_obj_noise.getNoiseInElectrons(sensorID_L4 , true, 0));

  B2INFO("~~~~~~~~~~~~~~\n");

  B2INFO("Charge [e]        L3_1_1 side V strip 0 = " << m_obj_pulseShape.getChargeFromADC(sensorID, false, 0, 60));
  B2INFO("ADC counts [a.u.] L3_1_1 side V strip 0 = " << m_obj_pulseShape.getADCFromCharge(sensorID, false, 0, 22500.));
  B2INFO("Peaking time [ns] L3_1_1 side V strip 0 = " << m_obj_pulseShape.getPeakTime(sensorID, false, 0));
  B2INFO("Pulse width [ns]  L3_1_1 side V strip 0 = " << m_obj_pulseShape.getWidth(sensorID, false, 0));
  B2INFO("Time Correction [ns]  L3_1_1 side V strip 0 = " << m_obj_pulseShape.getTimeShiftCorrection(sensorID, false, 0));
  B2INFO("~~~~~~~~~~~~~~\n");

  B2INFO("Charge [e]        L3_1_1 side U strip 0 = " << m_obj_pulseShape.getChargeFromADC(sensorID, true, 0, 60));
  B2INFO("ADC counts [a.u.] L3_1_1 side U strip 0 = " << m_obj_pulseShape.getADCFromCharge(sensorID, true, 0, 22500.));
  B2INFO("Peaking time [ns] L3_1_1 side U strip 0 = " << m_obj_pulseShape.getPeakTime(sensorID, true, 0));
  B2INFO("Pulse width [ns]  L3_1_1 side U strip 0 = " << m_obj_pulseShape.getWidth(sensorID, true, 0));
  B2INFO("Time Correction [ns]  L3_1_1 side V strip 0 = " << m_obj_pulseShape.getTimeShiftCorrection(sensorID, true, 0));
  B2INFO("~~~~~~~~~~~~~~\n");
  */

  float beta = m_obj_cog.getCorrectedTime(sensorID, true, 0, 0, 0); //t_RAW = 0
  float alfa = m_obj_cog.getCorrectedTime(sensorID, true, 0, 1, 0) - beta; //t_RAW = 1 - beta
  B2INFO("CoG, alfa = " << alfa << ", beta = " << beta);

  /*
  B2INFO("~~~~~~~~~~~~~~\n");
  B2INFO("isBad = " << m_obj_badStrip->isBad(1, true, 1));
  */


}
