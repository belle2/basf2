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
  B2INFO("Noise = " << m_obj_noise.getNoise(VxdID(3, 1, 1) , true, 0));
  //  B2INFO("Charge [e] = " << m_obj_pulseShape->getChargeFromADC(1, true, 1, 1));
  //  B2INFO("ADC counts [a.u.] = " << m_obj_pulseShape->getADCFromCharge(1, true, 1, 22500.));
  //  B2INFO("Peaking time [ns] = " << m_obj_pulseShape->getPeakTime(1, true, 1));
  //  B2INFO("Pulse width [ns] = " << m_obj_pulseShape->getWidth(1, true, 1));
  B2INFO("isBad = " << m_obj_badStrip->isBad(1, true, 1));

}
