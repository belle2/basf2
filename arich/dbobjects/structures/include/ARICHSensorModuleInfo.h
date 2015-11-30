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
#include <arich/dbobjects/structures/ARICHFEBoardInfo.h>
#include <arich/dbobjects/structures/ARICHHapdInfo.h>

namespace Belle2 {
  /**
  *   Sensor Module Information
  */
  class ARICHSensorModuleInfo: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHSensorModuleInfo(): m_id(0), m_FEB(NULL), m_HAPD(NULL), m_timeStamp(), m_comment("") {};

    /**
     * Constructor
     */
    ARICHSensorModuleInfo(int id, ARICHFEBoardInfo* FEB, ARICHHapdInfo* HAPD, TTimeStamp timeStamp): m_id(id), m_FEB(FEB), m_HAPD(HAPD),
      m_timeStamp(timeStamp), m_comment("") {};

    /**
     * Constructor
     */
    ARICHSensorModuleInfo(int id, ARICHFEBoardInfo* FEB, ARICHHapdInfo* HAPD, TTimeStamp timeStamp, std::string comment): m_id(id),
      m_FEB(FEB), m_HAPD(HAPD), m_timeStamp(timeStamp), m_comment(comment) {};

    /**
     * Destructor
     */
    ~ARICHSensorModuleInfo() {};

    /** Get Sensor Module Identifier
     * @return Sensor Module Identifier
     */
    int getSensorModuleID() const {return m_id; }

    /** Set Sensor Module Identifier
     * @param Sensor Module Identifier
     */
    void setSensorModuleID(int id) {m_id = id; }

    /** Get FEBoard Identifier
     * @return FEBoard Identifier
     */
    ARICHFEBoardInfo* getFEBoardID() const {return m_FEB; }

    /** Set FEBoard Identifier
     * @param FEBoard Identifier
     */
    void setFEBoardID(ARICHFEBoardInfo* FEB) {m_FEB = FEB; }

    /** Get HAPD Identifier
     * @return HAPD Identifier
     */
    ARICHHapdInfo* getHapdID() const {return m_HAPD; }

    /** Set HAPD Identifier
     * @param HAPD Identifier
     */
    void setHapdID(ARICHHapdInfo* HAPD) {m_HAPD = HAPD; }

    /** Get Production date
     * @return Production date
     */
    TTimeStamp getTimeStamp() const {return m_timeStamp; }

    /** Set Production date
     * @param Production date
     */
    void setTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }

    /** Get comment
     * @return comment
     */
    std::string getSensorModuleComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setSensorModuleComment(const std::string& comment) {m_comment = comment; }

  private:
    int m_id;                    /**< Sensor Module identifier */
    ARICHFEBoardInfo* m_FEB;     /**< HapdInfo Sensor identifier */
    ARICHHapdInfo* m_HAPD;       /**< FEBoardInfo Front End Board identifier */
    TTimeStamp m_timeStamp;      /**< Production Date */
    std::string m_comment;       /**< optional comment */

    ClassDef(ARICHSensorModuleInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2


