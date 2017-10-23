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
#include <svd/dbobjects/SVDLocalRunBadStrips.h>

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
  VxdID sensorIDOnFourthLayer(4, 1, 3);
  VxdID sensorIDOnSixthLayer(6, 6, 4);

  /************************The Noises **********************/

  B2INFO("Noise L3_1_1 side V strip 0 = " << m_obj_noise.getNoise(sensorID, false, 0));
  B2INFO("Noise L3_1_1 side U strip 0 = " << m_obj_noise.getNoise(sensorID , true, 0));
  B2INFO("~~~~~~~~~~~~~~\n");

  B2INFO("Noise L4_1_3 side V strip 0 = " << m_obj_noise.getNoise(sensorIDOnFourthLayer, false, 0));
  B2INFO("Noise L4_1_3 side U strip 0 = " << m_obj_noise.getNoise(sensorIDOnFourthLayer , true, 0));
  B2INFO("~~~~~~~~~~~~~~\n");


  B2INFO("Noise L6_6_4 side V strip 0 = " << m_obj_noise.getNoise(sensorIDOnSixthLayer, false, 0));
  B2INFO("Noise L6_6_4 side U strip 0 = " << m_obj_noise.getNoise(sensorIDOnSixthLayer , true, 0));
  B2INFO("~~~~~~~~~~~~~~\n");

  /********************Pulse Shape Properties*******************/

  B2INFO("Charge [e]        L3_1_1 side V strip 0 = " << m_obj_pulseShape.getChargeFromADC(sensorID, false, 0, 60));
  B2INFO("ADC counts [a.u.] L3_1_1 side V strip 0 = " << m_obj_pulseShape.getADCFromCharge(sensorID, false, 0, 22500.));
  B2INFO("Peaking time [ns] L3_1_1 side V strip 0 = " << m_obj_pulseShape.getPeakTime(sensorID, false, 0));
  B2INFO("Pulse width [ns]  L3_1_1 side V strip 0 = " << m_obj_pulseShape.getWidth(sensorID, false, 0));
  B2INFO("~~~~~~~~~~~~~~\n");

  B2INFO("Charge [e]        L3_1_1 side U strip 0 = " << m_obj_pulseShape.getChargeFromADC(sensorID, true, 0, 60));
  B2INFO("ADC counts [a.u.] L3_1_1 side U strip 0 = " << m_obj_pulseShape.getADCFromCharge(sensorID, true, 0, 22500.));
  B2INFO("Peaking time [ns] L3_1_1 side U strip 0 = " << m_obj_pulseShape.getPeakTime(sensorID, true, 0));
  B2INFO("Pulse width [ns]  L3_1_1 side U strip 0 = " << m_obj_pulseShape.getWidth(sensorID, true, 0));
  B2INFO("~~~~~~~~~~~~~~\n");

  B2INFO("Charge [e]        L6_6_4 side V strip 0 = " << m_obj_pulseShape.getChargeFromADC(sensorIDOnSixthLayer, false, 0, 60));
  B2INFO("ADC counts [a.u.] L6_6_4 side V strip 0 = " << m_obj_pulseShape.getADCFromCharge(sensorIDOnSixthLayer, false, 0, 22500.));
  B2INFO("Peaking time [ns] L6_6_4 side V strip 0 = " << m_obj_pulseShape.getPeakTime(sensorIDOnSixthLayer, false, 0));
  B2INFO("Pulse width [ns]  L6_6_4 side V strip 0 = " << m_obj_pulseShape.getWidth(sensorIDOnSixthLayer, false, 0));
  B2INFO("~~~~~~~~~~~~~~\n");

  B2INFO("Charge [e]        L6_6_4 side U strip 0 = " << m_obj_pulseShape.getChargeFromADC(sensorIDOnSixthLayer, true, 0, 60));
  B2INFO("ADC counts [a.u.] L6_6_4 side U strip 0 = " << m_obj_pulseShape.getADCFromCharge(sensorIDOnSixthLayer, true, 0, 22500.));
  B2INFO("Peaking time [ns] L6_6_4 side U strip 0 = " << m_obj_pulseShape.getPeakTime(sensorIDOnSixthLayer, true, 0));
  B2INFO("Pulse width [ns]  L6_6_4 side U strip 0 = " << m_obj_pulseShape.getWidth(sensorIDOnSixthLayer, true, 0));

  //  B2INFO("isBad = " << m_obj_badStrip->isBad(1, true, 1));

}
