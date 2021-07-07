/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <string>
#include <TTimeStamp.h>
#include <arich/dbobjects/ARICHAsicInfo.h>

namespace Belle2 {
  /**
  *   Front End Electronics Boards
  */

  class ARICHFEBoardInfo: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHFEBoardInfo(): m_serial(0), m_timeStamp(), m_currentLocation("")
    {
      for (unsigned i = 0; i < 4; i++) m_asicInfo[i] = NULL;
      for (unsigned i = 0; i < 4; i++) m_asicsFEB[i] = ("");
    };

    /**
     * Constructor
     */
    ARICHFEBoardInfo(int serial, ARICHAsicInfo** asicInfo, const std::string& asicsFEB, TTimeStamp timeStamp): m_serial(serial),
      m_timeStamp(timeStamp),
      m_currentLocation("")
    {
      for (unsigned i = 0; i < 4; i++) m_asicInfo[i] = asicInfo[i];
      for (unsigned i = 0; i < 4; i++) m_asicsFEB[i] = asicsFEB[i];
    };

    /**
     * Destructor
     */
    ~ARICHFEBoardInfo() {};

    /** Get Front End Board Identifier
     * @return Front End Board Identifier
     */
    int getFEBoardSerial() const {return m_serial; }

    /** Set Front End Board Identifier
     * @param Front End Board Identifier
     */
    void setFEBoardSerial(int serial) {m_serial = serial; }

    /** Get Asic Info
     * @return Asic Info
     */
    ARICHAsicInfo* getAsicInfo(unsigned int i) const;

    /** Set Asic Info
     * @param Asic Info
     */
    void setAsicInfo(unsigned int i, ARICHAsicInfo* asic);

    /** Get Asic position
     * @return Asic position
     */
    std::string getAsicPosition(unsigned int i) const;

    /** Set Asic position
     * @param Asic position
     */
    void setAsicPosition(unsigned int i, const std::string& asicsFEB);

    /** Get delivery date
     * @return delivery date
     */
    TTimeStamp getTimeStamp() const {return m_timeStamp; }

    /** Set delivery date
     * @param delivery date
     */
    void setTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }

    /** Get current location
     * @return current location
     */
    std::string getFEBoardLocation();

    /** Set current location
     * @param current location
     */
    void setFEBoardLocation(const std::string& currentLocation) {m_currentLocation = currentLocation; }

  private:
    int m_serial;                    /**< FrontEndBoard Identfier */
    ARICHAsicInfo* m_asicInfo[4];    /**< id of the i-th chip in the FEB */
    std::string m_asicsFEB[4];       /**< serial numbers of ASICs on FEB board */
    TTimeStamp m_timeStamp;          /**< Delivery Date */
    std::string m_currentLocation;   /**< Current Location of the FEB */

    ClassDef(ARICHFEBoardInfo, 2);  /**< ClassDef */
  };
} // end namespace Belle2
