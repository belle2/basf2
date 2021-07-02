/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>

namespace Belle2 {

  /**
   *   Flag for beam background study
   */
  class ECLBeamBackgroundStudy: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLBeamBackgroundStudy() : m_isBeamBackgroundStudy(false) {}

    /**
     * Destructor
     */
    ~ECLBeamBackgroundStudy() {}

    /** Return beam background study flag
     * @return beam background study flag
     */
    bool getBeamBackgroundStudy() const {return m_isBeamBackgroundStudy; }

    /** Set beam background study flag
     * @param beam background study flag
     */
    void setBeamBackgroundStudy(bool study) { m_isBeamBackgroundStudy = study; }
  private:
    bool m_isBeamBackgroundStudy; /**< Flag */

    ClassDef(ECLBeamBackgroundStudy, 1); /**< ClassDef */
  };
} // end namespace Belle2
