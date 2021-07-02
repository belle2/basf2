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

namespace Belle2 {
  /**
  *   List of MergerBoards
  */

  class ARICHMergerInfo: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHMergerInfo(): m_id(0), m_type(0), m_comment("") {};

    /**
     * Constructor
     */
    ARICHMergerInfo(int id, int type): m_id(id), m_type(type), m_comment("") {};

    /**
     * Constructor
     */
    ARICHMergerInfo(int id, int type, const std::string& comment): m_id(id), m_type(type), m_comment(comment) {};

    /**
     * Destructor
     */
    ~ARICHMergerInfo() {};

    /** Get Merger Identifier
     * @return Merger Identifier
     */
    int getMergerID() const {return m_id; }

    /** Set Merger Identifier
     * @param Merger Identifier
     */
    void setMergerID(int id) {m_id = id; }

    /** Get Merger Type
     * @return Merger Type
     */
    int getMergerType() const {return m_type; }

    /** Set Merger Type
     * @param Merger Type
     */
    void setMergerType(int type) {m_type = type; }

    /** Get comment
     * @return comment
     */
    std::string getMergerComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setMergerComment(const std::string& comment) {m_comment = comment; }

  private:
    int m_id;               /**< Merger Identifier */
    int m_type;             /**< Merger Type */
    std::string m_comment;  /**< Optional comment */

    ClassDef(ARICHMergerInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2


