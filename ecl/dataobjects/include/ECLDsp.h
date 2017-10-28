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
    ECLDsp() : m_DspAVector(31, 0)
    {
      m_CellId = 0;    /**< cell id */

    }

    /** Constructor for data*/
    ECLDsp(int CellId, int NADCPoints, int* ADCData)
    {
      m_CellId     = CellId;
      m_DspAVector.assign(ADCData, ADCData + NADCPoints);
    }

    /** Constructor for data*/
    ECLDsp(int CellId, std::vector<int> ADCData)
    {
      m_CellId     = CellId;
      m_DspAVector = ADCData;
    }

    /*! Set Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set Dsp array
     */
    void setDspA(int  DspArray[31])
    {
      m_DspAVector.assign(DspArray, DspArray + 31);
    }

    /*! Set Dsp array
     */
    void setDspA(std::vector <int> DspArrayVector)
    {
      m_DspAVector = DspArrayVector;
    }

    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get Dsp Array
     * @return Dsp Array 0~31
     */
    void getDspA(int  DspArray[31]) const
    {
      for (int i = 0; i < 31; i++)
        DspArray[i] = m_DspAVector[i];
    }

    /*! Get Dsp Array
     * @return Dsp Array of variable length
     */
    std::vector <int> getDspA() const
    {
      return m_DspAVector;
    }

    /*! Get number of ADC points
     * @return number of ADC points
     */
    int getNADCPoints() const
    {
      return m_DspAVector.size();
    }

  private:

    int m_CellId;      /**< Cell ID */
    std::vector <int> m_DspAVector; /**< Dsp array vith variable length for calibration, tests, etc.  */

    ClassDef(ECLDsp, 2);/* dspa array with variable length*/

  };
} // end namespace Belle2

#endif
