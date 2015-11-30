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
#include <string>
#include <TTimeStamp.h>
#include <arich/dbobjects/structures/ARICHAsicInfo.h>

namespace Belle2 {
  /**
  *   Front End Electronics Boards
  */

  class ARICHFEBoardInfo: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHFEBoardInfo(): m_id(0), m_timeStamp(), m_currentLocation("")
    {
      for (unsigned i = 0; i < 4; i++) m_asicInfo[i] = NULL;
      for (unsigned i = 0; i < 4; i++) m_asicsFEB[i] = ("");
    };

    /**
     * Constructor
     */
    ARICHFEBoardInfo(int id, ARICHAsicInfo** asicInfo, std::string asicsFEB, TTimeStamp timeStamp): m_id(id), m_timeStamp(timeStamp),
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
    int getFEBoardID() const {return m_id; }

    /** Set Front End Board Identifier
     * @param Front End Board Identifier
     */
    void setFEBoardID(int id) {m_id = id; }

    /** Get Asic Info
     * @return Asic Info
     */
    ARICHAsicInfo* getAsicInfo(unsigned int i) const;

    /** Set Asic Info
     * @param Asic Info
     */
    void setAsicInfo(unsigned int i, ARICHAsicInfo* asic);

    /** Get Asic Info
     * @return Asic Info
     */
    std::string getAsicPosition(unsigned int i) const;

    /** Set Asic Info
     * @param Asic Info
     */
    void setAsicPosition(unsigned int i, std::string asicsFEB);

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
    ARICHAsicInfo* m_asicInfo[4];    /**< id of the i-th chip in the FEB */
    std::string m_asicsFEB[4];       /**< serial numbers of ASICs on FEB board */
    TTimeStamp m_timeStamp;          /**< Production Date */
    std::string m_currentLocation;   /**< Current Location of the FEB */

    ClassDef(ARICHFEBoardInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2
