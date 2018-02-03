/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Oberhof                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLPURECSIINFO_H
#define ECLPURECSIINFO_H

#include <framework/datastore/RelationsObject.h>
namespace Belle2 {

  /*! Class to store ECL crystal type relation to ECLDigit
   * for the simulation pure CsI upgrade option
   * filled in ecl/modules/eclDigitizer/src/ECLDigitizerPureCsIModule.cc
   */

  class ECLPureCsIInfo : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLPureCsIInfo()
    {
      m_CellId = 0;    /**< Cell ID */
      m_isPureCsI = 0; /**< is pure CsI crystal? */
      ;
    }

    /*! Set  Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set isPureCsI
    */
    void setPureCsI(bool isPureCsI) { m_isPureCsI = isPureCsI; }

    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get isPureCsI
     * @return isPureCsI
    */
    bool getPureCsI() const { return m_isPureCsI; }

  private:

    int m_CellId;      /**< Cell ID */
    bool m_isPureCsI;   /**< is Pure CsI? */

    ClassDef(ECLPureCsIInfo, 2);/**< ClassDef */

  };
} // end namespace Belle2

#endif
