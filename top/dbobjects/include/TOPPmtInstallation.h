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
#include <top/dbobjects/TOPPmtObsoleteData.h>

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
     * @param slotNumber slot number
     * @param arrayNumber number of PMT array
     * @param position position of PMT within array (module)
     * @param type PMT type
     */
    TOPPmtInstallation(const std::string& serialNumber, int moduleCNumber, int slotNumber,
                       int arrayNumber, int position, TOPPmtObsoleteData::EType type):
      m_serialNumber(serialNumber), m_moduleCNumber(moduleCNumber), m_slotNumber(slotNumber),
      m_arrayNumber(arrayNumber), m_position(position), m_type(type)
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
     * Returns slot number
     * @return slot number
     */
    int getSlotNumber() const {return m_slotNumber;}

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

    /**
     * Returns PMT type
     * @return type
     */
    TOPPmtObsoleteData::EType getType() const {return m_type;}


  private:

    std::string m_serialNumber; /**< serial number, e.g. JTxxxx */
    int m_moduleCNumber = 0;    /**< module construction number (1-based) */
    int m_slotNumber = 0;       /**< slot number (1-based) */
    int m_arrayNumber = 0;      /**< MCP array number (1-based) */
    int m_position = 0;         /**< position within the TOP module (1-based) */

    TOPPmtObsoleteData::EType m_type = TOPPmtObsoleteData::c_Unknown;   /**< type (conventional, ALD, ...) */

    ClassDef(TOPPmtInstallation, 2); /**< ClassDef */

  };

} // end namespace Belle2


