/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
namespace Belle2 {

  /*! Class to store ECL ShaperDSP waveform ADC data and additional information for ML studies
   *
   * For MC filled in ecl/modules/eclDigitizer/src/ECLDigitizerModule.cc
   * For data filled in ecl/modules/eclUnpacker/src/eclUnpackerModule.cc
   *
   */

  class ECLDspWithExtraMCInfo : public ECLDsp {
  public:
    /*! Set total true deposited energy
    */
    void setEnergyDep(double input) { m_energyDep = input; }

    /*! Set total true deposited hadron energy
    */
    void setHadronEnergyDep(double input) { m_hadronEnergyDep = input; }

    /*! Set true flight time (weighted by true deposited energy)
    */
    void setFlightTime(double input) { m_flighttime = input; }

    /*! Set timeshift (weighted by true deposited energy)
    */
    void setTimeShift(double input) { m_timeshift = input; }

    /*! Set time to sensor (weighted by true deposited energy)
    */
    void setTimeToSensor(double input) { m_timetosensor = input; }

    /*! Set energy conversion factor
    */
    void setEnergyConversion(double input) { m_energyConversion = input; }

    /*! get true deposited hadron energy
    * @returntrue deposited hadron energy
    */
    double getHadronEnergyDep() const { return m_hadronEnergyDep; }

    /*! get true deposited energy
    * @returntrue deposited energy
    */
    double getEnergyDep() const { return m_energyDep; }

    /*! get energy conversion factor
    * @return energy conversion factor
    */
    double getEnergyConversion() const { return m_energyConversion; }

    /*! get true flight time
    * @return true flight time
    */
    double getFlightTime() const { return m_flighttime; }

    /*! get time shift
    * @return  time shift
    */
    double getTimeShift() const { return m_timeshift; }

    /*! get time to sensor
    * @return time to sensor
    */
    double getTimeToSensor() const { return m_timetosensor; }


  private:

    double m_hadronEnergyDep{0};   /**< True deposited hadron energy*/
    double m_energyDep{0};   /**< True deposited energy*/
    double m_energyConversion{0};   /**< Conversion from energy to ADC*/
    double m_flighttime{0};   /**< True flighttime (weighted by true energy) */
    double m_timeshift{0};   /**< Time shift (weighted by true energy) */
    double m_timetosensor{0};   /**< time to sensor (weighted by true energy) */

    ClassDef(ECLDspWithExtraMCInfo, 1);

  };
} // end namespace Belle2

