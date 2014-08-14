/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLTRIG_H
#define ECLTRIG_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /*! Class to store ECLTrig, still need to be study
   * relation to ECLHit
   * filled in ecl/modules/eclDigitizer/src/ECLDigitizerModule.cc
   */

  class ECLTrig : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLTrig() {
      m_CellId = 0;    /**< Cell ID */
      m_TimeTrig = 0; /**< Trig Time */
    }


    /*! Set  Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }
    /*! Set Trig Time
     */
    void setTimeTrig(double TimeTrig) { m_TimeTrig = TimeTrig; }
    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get Trig Time
     * @return Trig Time
     */
    double getTimeTrig() const { return m_TimeTrig; }

  private:
    int m_CellId;      /**< Cell ID */
    double m_TimeTrig; /**< Trig Time */


    ClassDef(ECLTrig, 1);/**< ClassDef */

  };
} // end namespace Belle2

#endif
