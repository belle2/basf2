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
   * List of cables.
   */
  class ARICHCableInfo: public TObject {
  public:
    /**
     * Default constructor.
     */
    ARICHCableInfo(): m_id(0), m_type(0), m_length(0), m_comment("") {};

    /**
     * Constructor.
     */
    ARICHCableInfo(int id, int type, float length): m_id(id), m_type(type), m_length(length), m_comment("") {};

    /**
     * Constructor.
     */
    ARICHCableInfo(int id, int type, float length, const std::string& comment): m_id(id), m_type(type), m_length(length),
      m_comment(comment) {};

    /**
     * Destructor.
     */
    ~ARICHCableInfo() {};

    /**
     * Get cable identifier.
     * @return Cable identifier.
     */
    int getCableID() const {return m_id; }

    /**
     * Set cable identifier.
     * @param[in] id Cable identifier.
     */
    void setCableID(int id) {m_id = id; }

    /**
     * Get cable type.
     * @return Cable yype.
     */
    int getCableType() const {return m_type; }

    /**
     * Set cable type.
     * @param[in] type Cable type.
     */
    void setCableType(int type) {m_type = type; }

    /**
     * Get cable length.
     * @return Cable length.
     */
    float getCableLength() const {return m_length; }

    /**
     * Set cable length.
     * @param[in] length Cable length.
     */
    void setCableLength(float length) {m_length = length; }

    /**
     * Get comment.
     * @return Comment.
     */
    std::string getCableComment() const {return m_comment; }

    /**
     * Set comment.
     * @param[in] comment Comment.
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
