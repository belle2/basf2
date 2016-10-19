/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Calibration constants for the ECL single crystal energy calibration.   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGITTIMECONSTANTS_H
#define ECLDIGITTIMECONSTANTS_H
#include <TObject.h>

namespace Belle2 {

  /**
   *   Time and time resolution calibration constants per digit
   */

  class ECLDigitTimeConstants: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLDigitTimeConstants(): m_cellID(0), m_offset(0) {};

    /**
     * Constructor
     */
    ECLDigitTimeConstants(int cellid, float offset): m_cellID(cellid), m_offset(offset) {};

    /**
     * Destructor
     */
    ~ECLDigitTimeConstants() {};

    /** Return calibration offset
     * @return calibration offset
     */
    float getOffset() const {return m_offset; };

    /** Return cell ID
     * @return cell ID
     */
    int getCellID() const {return m_cellID; };

    /** Set calibration offset
     * @param calibration offset
     */
    void setOffset(float offset) {m_offset = offset; };

    /** Set cell ID
     * @param cell ID
     */
    void setCellID(int cellid) {m_cellID = cellid; };

  private:
    int m_cellID;       /**< ECL cell ID */
    float m_offset;  /**< time offset 'a' */

    // 2: changed getter getCelleID from float to int
    ClassDef(ECLDigitTimeConstants, 2); /**< ClassDef */
  };
} // end namespace Belle2
#endif
