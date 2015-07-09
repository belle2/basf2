/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TList.h>
#include <string>
#include <TTimeStamp.h>
class ARICHAsicInfo;

namespace Belle2 {
  /**
  *   Front End Electronics Boards
  */

  class ARICHFEBoardInfo: public TObject {
    /**
     * Default constructor
     */
    ARICHFEBoardInfo(): m_id(0), m_timeStamp(0, 0, 0, kTRUE, 0) {};

    /**
     * Constructor
     */
    ARICHFEBoardInfo(int id, TTimeStamp timeStamp)
    {
      m_id = id;
      m_timeStamp = timeStamp;
    }

    /**
     * Destructor
     */
    ~ARICHFEBoardInfo() {};

    /** Get Front End Board Identifier
     * @return Front End Board Identifier
     */
    int getFEBoardID() const {return m_id; }

    /** Set Front End Board Identifier
     * @param Front End Board Identifier
     */
    void setFEBoardID(int id) {m_id = id; }

    /** Get Asic Identifier
     * @return Asic Identifier
     */
    ARICHAsicInfo* getAsicID() const {return m_asicInfo; }

    /** Set HAPD Identifier
     * @param HAPD Identifier
     */
    void setAsicID(ARICHAsicInfo* Asic) {m_asicInfo = Asic; }

    /** Get Production date
     * @return Production date
     */
    TTimeStamp getTimeStamp() const {return m_timeStamp; }

    /** Set Production date
     * @param Production date
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
    int m_id;                        /**< FrontEndBoard Identfier */
    ARICHAsicInfo* m_asicInfo;       /**< id of the i-th chip in the FEB */
    TTimeStamp m_timeStamp;          /**< Production Date */
    std::string m_currentLocation;   /**< Current Location of the FEB */

    ClassDef(ARICHFEBoardInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2



