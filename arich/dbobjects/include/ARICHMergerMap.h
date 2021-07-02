/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <arich/dbobjects/ARICHMergerInfo.h>
#include <arich/dbobjects/ARICHB2LinkInfo.h>
#include <arich/dbobjects/ARICHCableInfo.h>

namespace Belle2 {
  /**
  *   Mapping of the merger board to the detector
  */
  class ARICHMergerMap: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHMergerMap(): m_position(0), m_sextant(0), m_merger(Belle2::ARICHMergerInfo()), m_b2Link(Belle2::ARICHB2LinkInfo()),
      m_cable(Belle2::ARICHCableInfo()) {};

    /**
     * Constructor
     */
    ARICHMergerMap(int position, int sextant): m_position(position), m_sextant(sextant), m_merger(Belle2::ARICHMergerInfo()),
      m_b2Link(Belle2::ARICHB2LinkInfo()), m_cable(Belle2::ARICHCableInfo()) {};

    /**
     * Constructor
     */
    ARICHMergerMap(int position, int sextant, ARICHMergerInfo merger, ARICHB2LinkInfo b2Link,
                   ARICHCableInfo cable): m_position(position), m_sextant(sextant), m_merger(merger), m_b2Link(b2Link), m_cable(cable) {};

    /**
     * Destructor
     */
    ~ARICHMergerMap() {};

    /** Return Merger Position
     * @return Merger Position
     */
    int getMergerPosition() const {return m_position; }

    /** Set Merger Position
     * @param Merger Position
     */
    void setMergerPosition(int position) {m_position = position; }

    /** Return Merger sextant
     * @return Merger sextant
     */
    int getMergerSextant() const {return m_sextant; }

    /** Set Merger sextant
     * @param Merger sextant
     */
    void setMergerSextant(int sextant) {m_sextant = sextant; }

    /** Return Merger board Identifier
     * @return Merger board Identifier
     */

    ARICHMergerInfo getMergerID() const {return m_merger; }

    /** Set Merger board Identifier
     * @param Merger board Identifier
     */
    void setMergerID(ARICHMergerInfo merger) {m_merger = merger; }


    /** Return B2Link Identifier
     * @return B2Link Identifier
     */

    ARICHB2LinkInfo getB2LinkID() const {return m_b2Link; }

    /** Set B2Link Identifier
     * @param B2Link Identifier
     */
    void setB2LinkID(ARICHB2LinkInfo b2Link) {m_b2Link = b2Link; }


    /** Return board Identifier
     * @return board Identifier
     */

    ARICHCableInfo getCableID() const {return m_cable; }

    /** Set cable Identifier
     * @param cable Identifier
     */
    void setCableID(ARICHCableInfo cable) {m_cable = cable; }



  private:
    int m_position;             /**< Position in the detector */
    int m_sextant;              /**< Sextant identification   */
    ARICHMergerInfo m_merger;   /**< Merger Board identifier */
    ARICHB2LinkInfo m_b2Link;   /**< B2Link Identifier */
    ARICHCableInfo  m_cable;    /**< Cable Identifier */

    ClassDef(ARICHMergerMap, 1);  /**< ClassDef */
  };
} // end namespace Belle2

