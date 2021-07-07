/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once
#include <TObject.h>
#include <TTimeStamp.h>
#include <string>
#include <arich/dbobjects/ARICHMirrorInfo.h>

namespace Belle2 {
  /**
  *   Mapping of the mirrors on the detector
  */
  class ARICHMirrorMap: public TObject {
  public:
    /**
     * Default constructor
     */
    ARICHMirrorMap(): m_mirror(NULL), m_column(0), m_timeStamp(), m_comment("") {};

    /**
     * Constructor
     */
    ARICHMirrorMap(ARICHMirrorInfo* mirror, int column, TTimeStamp timeStamp): m_mirror(mirror), m_column(column),
      m_timeStamp(timeStamp), m_comment("") {};

    /**
     * Constructor
     */
    ARICHMirrorMap(ARICHMirrorInfo* mirror, int column, TTimeStamp timeStamp, const std::string& comment): m_mirror(mirror),
      m_column(column),
      m_timeStamp(timeStamp), m_comment(comment) {};

    /**
     * Destructor
     */
    ~ARICHMirrorMap() {};

    /** Return Mirror Identifier
     * @return Mirror Identifier
     */
    ARICHMirrorInfo* getMirrorID() const {return m_mirror; }

    /** Set Mirror Identifier
     * @param Mirror Identifier
     */
    void setMirrorID(ARICHMirrorInfo* mirror) {m_mirror = mirror; }

    /** Return Position on the detector
     * @return Position on the detector
     */
    int getMirrorPosition() const {return m_column; }

    /** Set Position on the detector
     * @param Position on the detector
     */
    void setMirrorPosition(int column) {m_column = column; }

    /** Return Installation date
     * @return Installation date
     */
    TTimeStamp getMirrorTimeStamp() const {return m_timeStamp; }

    /** Set Installation date
     * @param Installation date
     */
    void setMirrorTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }

    /** Return comment
     * @return comment
     */
    std::string getMirrorComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setMirrorComment(const std::string& comment) {m_comment = comment; }


  private:

    ARICHMirrorInfo* m_mirror;  /**< Mirror identifier */
    int m_column;               /**< Position on the detector */
    TTimeStamp  m_timeStamp;    /**< Installation Date */
    std::string m_comment;      /**< optional comment */

    ClassDef(ARICHMirrorMap, 1);  /**< ClassDef */
  };
} // end namespace Belle2



