/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDSP_H
#define ECLDSP_H

#include <framework/datastore/RelationsObject.h>
namespace Belle2 {

  /*! Class to store ECL simulated rawdata of Dsp array for fit
   *  before digitization fit (output of ECLDsp)
   * relation to ECLHit
   * filled in ecl/modules/eclDigitizer/src/ECLDigitizerModule.cc
   */

  class ECLDsp : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLDsp() {
      m_CellId = 0;    /**< Cell ID */
      for (int i = 0; i < 31; i++)m_DspA[i] = 0; /**< Dsp Array 0~31 for fit   */
      ;
    }

    /*! Set Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set Dsp array
     */
    void setDspA(int  DspArray[31]) { for (int i = 0; i < 31; i++) { m_DspA[i] = DspArray[i];} }


    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get Dsp Array
     * @return Dsp Array 0~31
     */
    void getDspA(int  DspArray[31]) const { for (int i = 0; i < 31; i++) {  DspArray[i] = m_DspA[i] ;} }


  private:

    int m_CellId;      /**< Cell ID */
    int m_DspA[31];    /**< Dsp Array 0~31 for fit   */

    ClassDef(ECLDsp, 1);/**< ClassDef */

  };
} // end namespace Belle2

#endif
