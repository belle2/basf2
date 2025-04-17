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
#include <arich/dbobjects/ARICHFEBoardInfo.h>
#include <arich/dbobjects/ARICHHapdInfo.h>
#include <arich/dbobjects/ARICHModuleTest.h>

namespace Belle2 {

  /**
   * Sensor-module information.
   */
  class ARICHSensorModuleInfo: public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHSensorModuleInfo(): m_id(0), m_FEBserial(0), m_FEB(NULL), m_HAPDserial(""), m_HAPD(NULL), m_module(NULL), m_HVboard(0),
      m_timeStamp(), m_comment("") {};

    /**
     * Constructor.
     */
    ARICHSensorModuleInfo(int id, ARICHFEBoardInfo* FEB, ARICHHapdInfo* HAPD, TTimeStamp timeStamp): m_id(id), m_FEBserial(0),
      m_FEB(FEB), m_HAPDserial(""), m_HAPD(HAPD), m_module(NULL), m_HVboard(0), m_timeStamp(timeStamp), m_comment("") {};

    /**
     * Constructor.
     */
    ARICHSensorModuleInfo(int id, ARICHFEBoardInfo* FEB, ARICHHapdInfo* HAPD, TTimeStamp timeStamp,
                          const std::string& comment): m_id(id),
      m_FEBserial(0), m_FEB(FEB), m_HAPDserial(""), m_HAPD(HAPD), m_module(NULL), m_HVboard(0), m_timeStamp(timeStamp),
      m_comment(comment) {};

    /**
     * Destructor.
     */
    ~ARICHSensorModuleInfo() {};

    /**
     * Get sensor-module identifier.
     * @return Sensor-module identifier.
     */
    int getSensorModuleID() const {return m_id; }

    /**
     * Set Sensor-module identifier.
     * @param[in] id Sensor-module identifier.
     */
    void setSensorModuleID(int id) {m_id = id; }

    /**
     * Get FEB serial number.
     * @return FEB serial number.
     */
    int getFEBserial() const {return m_FEBserial; }

    /**
     * Set FEB serial number.
     * @param[in] FEBserial FEB serial number.
     */
    void setFEBserial(int FEBserial) {m_FEBserial = FEBserial; }

    /**
     * Get FEBoard identifier.
     * @return FEBoard identifier.
     */
    ARICHFEBoardInfo* getFEBoardID() const {return m_FEB; }

    /**
     * Set FEBoard identifier.
     * @param[in] FEB FEBoard identifier.
     */
    void setFEBoardID(ARICHFEBoardInfo* FEB) {m_FEB = FEB; }

    /**
     * Get HAPD serial number.
     * @return HAPD serial number.
     */
    std::string getHAPDserial() const {return m_HAPDserial; }

    /**
     * Set HAPD serial number.
     * @param[in] HAPDserial HAPD serial number.
     */
    void setHAPDserial(const std::string& HAPDserial) {m_HAPDserial = HAPDserial; }

    /**
     * Get HAPD Identifier.
     * @return HAPD Identifier.
     */
    ARICHHapdInfo* getHapdID() const {return m_HAPD; }

    /**
     * Set HAPD Identifier.
     * @param[in] HAPD HAPD Identifier.
     */
    void setHapdID(ARICHHapdInfo* HAPD) {m_HAPD = HAPD; }

    /**
     * Get module test identifier.
     * @return Module test identifier.
     */
    ARICHModuleTest* getModuleTest() const {return m_module; }

    /**
     * Set module test identifier.
     * @param[in] module Module test Identifier.
     */
    void setModuleTest(ARICHModuleTest* module) {m_module = module; }

    /**
     * Get HV board identifier.
     * @return HV board identifier.
     */
    int getHVboardID() const {return m_HVboard; }

    /**
     * Set HV board identifier.
     * @param[in] hv HV board identifier.
     */
    void setHVboardID(int hv) {m_HVboard = hv; }

    /**
     * Get production date.
     * @return Production date
     */
    TTimeStamp getTimeStamp() const {return m_timeStamp; }

    /**
     * Set production date.
     * @param[in] timeStamp Production date.
     */
    void setTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }

    /**
     * Get comment.
     * @return comment.
     */
    std::string getSensorModuleComment() const {return m_comment; }

    /**
     * Set comment.
     * @param[in] comment Comment.
     */
    void setSensorModuleComment(const std::string& comment) {m_comment = comment; }

  private:
    int m_id;                    /**< Sensor Module identifier: m_id = n_ring * nModulesInPreviousRing + n_column */
    int m_FEBserial;             /**< FEB serial number */
    ARICHFEBoardInfo* m_FEB;     /**< FEBoardInfo Front End Board identifier */
    std::string m_HAPDserial;    /**< HAPD serial number */
    ARICHHapdInfo* m_HAPD;       /**< HapdInfo Sensor identifier */
    ARICHModuleTest* m_module;   /**< Module test identifier */
    int m_HVboard;               /**< HV board serial number */
    TTimeStamp m_timeStamp;      /**< Production Date */
    std::string m_comment;       /**< optional comment */

    ClassDef(ARICHSensorModuleInfo, 2);  /**< ClassDef */
  };
} // end namespace Belle2


