/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING: Do not try to fry it with water. Use only olive oil.          *
 **************************************************************************/

#pragma once

#include <svd/dbobjects/SVDLocalConfigParameters.h>
#include <svd/dbobjects/SVDGlobalConfigParameters.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <string>

namespace Belle2 {

  /** This class defines the dbobject and the methods to access the
   * parameters to configure SVD for data taking with:
   * - float zero suppression value (global run)
   * - int latency (global run)
   * - string: date and time time of the calibration run
   * - float injected charge (local cal run)
   * - time units (local cal run)
   * - masking bitmap (local cal run)
   *
   */
  class SVDDetectorConfiguration {
  public:

    static std::string svdLocalConfig_name; /**< name of the SVDLocalConfigParameters payload*/
    static std::string svdGlobalConfig_name; /**< name of the SVDGlobalConfigParameters payload*/

    typedef SVDLocalConfigParameters t_svdLocalConfig_payload; /**< typedef of the SVDLocalConfigParameters payload*/
    typedef SVDGlobalConfigParameters t_svdGlobalConfig_payload; /**< typedef of the SVDGlobalConfigParameters payload*/



    /** Constructor, no input argument is required */
    SVDDetectorConfiguration()
      : m_svdLocalConfig_aDBObjPtr(svdLocalConfig_name)
      , m_svdGlobalConfig_aDBObjPtr(svdGlobalConfig_name)
    {

      m_svdGlobalConfig_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2INFO("SVDDetectorConfiguration, GLOBAL run parameters: from now on we are using " <<
        this->m_svdGlobalConfig_aDBObjPtr -> get_uniqueID()); });

      m_svdLocalConfig_aDBObjPtr.addCallback([ this ](const std::string&) -> void {
        B2INFO("SVDDetectorConfiguration, LOCAL run parameters: from now on we are using " <<
        this->m_svdLocalConfig_aDBObjPtr -> get_uniqueID()); });
    }

    /** LOCAL CONFIGURATION PARAMETERS:
     * Return the charge injected on each strip for the pulse
     *  shape calibration during the calibration local runs
     *
     * Input:
     * no input parameters are required since it is a detector based payload
     *
     * Output: float corresponding to the injected charge [e]
     */
    // cppcheck-suppress unusedPrivateFunction
    float getInjectedCharged()
    {
      return m_svdLocalConfig_aDBObjPtr->getInjectedCharge();
    }


    /** LOCAL CONFIGURATION PARAMETERS:
     * Return the time units of the measured peak time of the pulse shape given in term of accelerator RFC
     *
     * Input:
     * no input parameters are required since it is a detector based payload
     *
     * Output: float corresponding to the time units [RFC]
     */
    float getCalibrationTimeInRFCUnits()
    {
      return m_svdLocalConfig_aDBObjPtr->getCalibrationTimeInRFCUnits();
    }


    /** LOCAL CONFIGURATION PARAMETERS:
     * Return the time stamp (date, hour) of the calibration
     *
     * Input:
     * no input parameters are required since it is a detector based payload
     *
     * Output: string corresponding to the date_hour (yyyymmdd_hhmm) of when the calibration has been taken
     */
    std::string getCalibDate()
    {
      return m_svdLocalConfig_aDBObjPtr->getCalibDate();
    }

    /** GLOBAL CONFIGURATION PARAMETERS:
     * Return the zero suppression cut applied during data taking
     *
     * Input:
     * no input parameters are required since it is a detector based payload
     *
     * Output: float corresponding to the zero suppression cut applied during data taking
     */
    float getZeroSuppression()
    {
      return m_svdGlobalConfig_aDBObjPtr->getZeroSuppression();
    }

    /** GLOBAL CONFIGURATION PARAMETERS:
     * Return the latency during applied during data taking
     *
     * Input:
     * no input parameters are required since it is a detector based payload
     *
     * Output: int corresponding to the latency cut applied during data taking
     */
    int getLatency()
    {
      return m_svdGlobalConfig_aDBObjPtr->getLatency();
    }


    /**  GLOBAL CONFIGURATION PARAMETERS:
     * Return the masking bitmap applied to mask strips at FADC level (only taken into account for cm-section)
     *
     * Input:
     * no input parameters are required since it is a detector based payload
     *
     * Output: int corresponding to the filtering mask applied to the field <masks>
     */
    float getMaskFilter()
    {
      return m_svdGlobalConfig_aDBObjPtr->getMaskFilter();
    }


    /** GLOBAL CONFIGURATION PARAMETERS:
     * Return the APV clock units [ns] in term of accelerator RFC converted to ns
     *
     * Input:none
     *
     * Output: float corresponding to the APV Clock units [ns]
     */
    float getAPVClockInRFCUnits()
    {
      return m_svdGlobalConfig_aDBObjPtr->getAPVClockInRFCUnits();
    }

    /** GLOBAL CONFIGURATION PARAMETERS:
     * Return the HV applied during data taking
     *
     * Input:
     * no input parameters are required since it is a detector based payload
     *
     * Output: float corresponding to the HV = Vbias/2 applied during data taking
     */
    float getHV()
    {
      return m_svdGlobalConfig_aDBObjPtr->getHV();
    }

    /** GLOBAL CONFIGURATION PARAMETERS:
     * Return the relative time shift in units of APV clock /4 between the 3- and 6- sample acquired events
     *
     * Input:
     * no input parameters are required since it is a detector based payload
     *
     * Output: int corresponding to the relative time shift in units of APV clock /4 between the 3- and 6- sample acquired events
     */
    float getRelativeTimeShift()
    {
      return m_svdGlobalConfig_aDBObjPtr->getRelativeTimeShift();
    }

    /** GLOBAL CONFIGURATION PARAMETER:
     * Return the int corresponding to the sumber of samples (3 -> 3-sample DAQ, 6-> 6-sample DAQ, 9 ->3-mixed-6 DAQ)
     */
    int getNrFrames()
    {
      return m_svdGlobalConfig_aDBObjPtr->getNrFrames();
    }

    /** returns the unique ID of the payload */
    //    TString getUniqueID() { return m_svdLocalConfig_aDBObjPtr->get_uniqueID(); }

    /** returns true if the m_aDBObtPtr is valid in the requested IoV */
    bool isValid()
    {
      //      return m_svdLocalConfig_aDBObjPtr.isValid();
      return m_svdGlobalConfig_aDBObjPtr.isValid();
    }



  private:
    DBObjPtr< t_svdLocalConfig_payload > m_svdLocalConfig_aDBObjPtr; /**< SVDLocalConfigParameters payload*/
    DBObjPtr< t_svdGlobalConfig_payload > m_svdGlobalConfig_aDBObjPtr;  /**< SVDGlobalConfigParameters payload*/


  };
}
