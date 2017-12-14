/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CHANGEPXDDIGITSTOBINARYMODULE_H
#define CHANGEPXDDIGITSTOBINARYMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDDigit.h>

namespace Belle2 {

  /**
   * This module changes specified PXD sensors to binary readout to emulate EUDET telescopes.
   * Must be run before PXDClusterizer.
   *
   * For sensors specified in "SensorIDs", all PXDDigits' charges are changed
   * to "BinaryValue" if charge is above "Threshold", to 0 otherwise.
   * This is for emulation of binary readout for EUDET telescopes made of PXD sensors.
   *
   */
  class ChangePXDDigitsToBinaryModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ChangePXDDigitsToBinaryModule();

    /**  Module initialization */
    virtual void initialize();

    /**  Loop over PXDDigits and change them to ON/OFF binary pixels in specified modules */
    virtual void event();

  private:

    double m_binaryValue;  /**< Charge for pixel ON */
    double m_threshold;  /**< Minimum charge for pixel to be ON */
    std::vector<std::string> m_sensorIDs;  /**< List of sensor IDs */
    std::vector<VxdID> m_sensorIDsVXD; /**< List of sensorIDs converted to VxdID */

    std::string m_storeDigitsName;  /**< Name of the collection to use for the PXDDigits */
  };
}

#endif /* CHANGEPXDDIGITSTOBINARYMODULE_H */
