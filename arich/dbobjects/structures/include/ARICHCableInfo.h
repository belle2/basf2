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
   *   List of Cables
   */

  class ARICHCableInfo: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHCableInfo(): m_id(0), m_type(0), m_length(0), m_comment("") {};

    /**
     * Constructor
     */
    ARICHCableInfo(int id, int type, float length): m_id(id), m_type(type), m_length(length), m_comment("") {};

    /**
     * Constructor
     */
    ARICHCableInfo(int id, int type, float length, std::string comment): m_id(id), m_type(type), m_length(length),
      m_comment(comment) {};

    /**
     * Destructor
     */
    ~ARICHCableInfo() {};

    /** Return Cable Identifier
     * @return Cable Identifier
     */
    int getCableID() const {return m_id; }

    /** Set Cable Identifier
     * @param Cable Identifier
     */
    void setCableID(int id) {m_id = id; }

    /** Return Cable Type
     * @return Cable Type
     */
    int getCableType() const {return m_type; }

    /** Set Cable Type
     * @param Cable Type
     */
    void setCableType(int type) {m_type = type; }

    /** Return Cable Type
     * @return Cable Type
     */
    float getCableLength() const {return m_length; }

    /** Set Cable Length
     * @param Cable Length
     */
    void setCableLength(float length) {m_length = length; }

    /** Return comment
     * @return comment
     */
    std::string getCableComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setCableComment(const std::string& comment) {m_comment = comment; }


  private:
    int m_id;         /**< Cable Identifier */
    int m_type;         /**< Cable Type */
    float m_length;     /**< Length of the Cable */
    std::string m_comment;  /**< comment */

    ClassDef(ARICHCableInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2
