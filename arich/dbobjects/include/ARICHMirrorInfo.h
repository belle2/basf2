/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TGraph.h>
#include <string>

namespace Belle2 {
  /**
  *   List of MergerBoards
  */
  class ARICHMirrorInfo: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHMirrorInfo(): m_id(0), m_reflectivity(NULL), m_comment("") {};

    /**
     * Constructor
     */
    ARICHMirrorInfo(int id, TGraph* reflectivity): m_id(id), m_reflectivity(reflectivity), m_comment("") {};

    /**
     * Constructor
     */
    ARICHMirrorInfo(int id, TGraph* reflectivity, const std::string& comment): m_id(id), m_reflectivity(reflectivity),
      m_comment(comment) {};

    /** Return Mirror Identifier
     * @return Mirror Identifier
     */
    int getMirrorID() const {return m_id; }

    /** Set Mirror Identifier
     * @param Mirror Identifier
     */
    void setMirrorID(int id) {m_id = id; }

    /**
     * Return Reflectivity of the mirror
     * @return Reflectivity of the mirror
     */
    TGraph* getMirrorReflectivity() const {return m_reflectivity;}

    /**
     * Set Reflectivity of the mirror
     * @param Reflectivity of the mirror
     */
    void setMirrorReflectivity(TGraph* reflectivity) {m_reflectivity = reflectivity;}

    /** Return comment
     * @return comment
     */
    std::string getMirrorInfoComment() const {return m_comment;}

    /** Set comment
     * @param comment
     */
    void setMirrorInfoComment(const std::string& comment) {m_comment = comment; }


  private:

    int m_id;                /**< Mirror Identifier */
    TGraph* m_reflectivity;  /**< reflectivity of the mirror */
    std::string m_comment;   /**< comment */

    ClassDef(ARICHMirrorInfo, 1);  /**< ClassDef */
  };
} // end namespace Belle2
