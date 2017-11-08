/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Alessandro Gaz                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * PMT position within a module
   */
  class TOPPmtInstallation : public TObject {
  public:
    /**
     * number of PMT's in a PMT array (TOP module)
     */
    enum {c_NumPMTs = 32};

    /**
     * Default constructor
     */
    TOPPmtInstallation()
    {}

    /**
     * Useful constructor
     * @param serialNumber serial number
     * @param moduleCNumber module construction number
     * @param arrayNumber number of PMT array
     * @param position position of PMT within array (module)
     */
    TOPPmtInstallation(const std::string& serialNumber, int moduleCNumber, int arrayNumber, int position):
      m_serialNumber(serialNumber), m_moduleCNumber(moduleCNumber), m_arrayNumber(arrayNumber), m_position(position)
    {}

    /**
     * Returns PMT serial number
     * @return serial number
     */
    const std::string& getSerialNumber() const {return m_serialNumber;}

    /**
     * Returns construction module number
     * @return construction module number
     */
    int getModuleCNumber() const {return m_moduleCNumber;}

    /**
     * Returns PMT array number
     * @return PMT array number
     */
    int getArrayNumber() const {return m_arrayNumber;}

    /**
     * Returns PMT position within array/module
     * @return PMT position
     */
    int getPosition() const {return m_position;}


  private:

    std::string m_serialNumber; /**< serial number, e.g. JTxxxx */
    int m_moduleCNumber = 0;    /**< module construction number (1-based) */
    int m_arrayNumber = 0;      /**< MCP array number (1-based) */
    int m_position = 0;         /**< position within the TOP module (1-based) */

    ClassDef(TOPPmtInstallation, 1); /**< ClassDef */

  };

} // end namespace Belle2


