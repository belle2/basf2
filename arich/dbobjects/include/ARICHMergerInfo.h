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

namespace Belle2 {
  /**
  *   List of MergerBoards
  */

  class ARICHMergerInfo: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHMergerInfo(): m_id(0), m_type(0) {};

    /**
     * Constructor
     */
    ARICHMergerInfo(int id, int type)
    {
      m_id = id;
      m_type = type;
    }

    /**
     * Destructor
     */
    ~ARICHMergerInfo() {};

    /** Get Merger Identifier
     * @return Merger Identifier
     */
    int getMergerID() const {return m_id; }

    /** Get Merger Type
     * @return Merger Type
     */
    int getMergerType() const {return m_type; }

    /** Get comment
     * @return comment
     */
    std::string getMergerComment();

    /** Set Merger Identifier
     * @param Merger Identifier
     */
    void setMergerID(int id) {m_id = id; }

    /** Set Merger Type
     * @param Merger Type
     */
    void setMergerType(int type) {m_type = type; }

    /** Set comment
     * @param comment
     */
    void setMergerComment();

  private:
    int m_id;             /**< Merger Identifier */
    int m_type;             /**< Merger Type */
    std::string m_comment;  /**< Optional comment */

    ClassDef(ARICHMergerInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2


