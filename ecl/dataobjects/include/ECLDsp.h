/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *               Savino Longo                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDSP_H
#define ECLDSP_H

#include <framework/datastore/RelationsObject.h>
namespace Belle2 {

  /*! Class to store ECL ShaperDSP waveform ADC data.
   *
   * For MC filled in ecl/modules/eclDigitizer/src/ECLDigitizerModule.cc
   * For data filled in ecl/modules/eclUnpacker/src/eclUnpackerModule.cc
   *
   */

  class ECLDsp : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLDsp() : m_DspAVector(31, 0)
    {
      m_CellId = 0;    /**< cell id */
      m_TwoComponentTotalAmp = -1;  /**< Offline two component total amplitude */
      m_TwoComponentHadronAmp = -1; /**< Offline two component hadron amplitude */
      m_TwoComponentChi2 = -1;  /**< Offline two component chi2*/
      m_TwoComponentTime = 1;  /**< Offline two component time */
      m_TwoComponentBaseline = 1;  /**< Offline two component baseline */
      m_isData = false;  /**< Data = true MC = false */
    }

    /** Constructor for data*/
    ECLDsp(int CellId, int NADCPoints, int* ADCData, bool flag)
    {
      m_CellId     = CellId;
      m_DspAVector.assign(ADCData, ADCData + NADCPoints);
      m_isData = flag;
    }

    /** Constructor for data*/
    ECLDsp(int CellId, std::vector<int> ADCData, bool flag)
    {
      m_CellId     = CellId;
      m_DspAVector = ADCData;
      m_isData = flag;
    }

    /*! Set Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set Data or MC flag. MC = false. Data = true
     */
    void setisData(bool flag) { m_isData = flag; }

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

    /*! Set two comp total amp
     */
    void setTwoComponentTotalAmp(double input) {  m_TwoComponentTotalAmp = input; }

    /*! Set two comp hadron amp
     */
    void setTwoComponentHadronAmp(double input) { m_TwoComponentHadronAmp = input; }

    /*! Set two comp chi2
     */
    void setTwoComponentChi2(double input) {      m_TwoComponentChi2 = input; }

    /*! Set two comp time
     */
    void setTwoComponentTime(double input) {      m_TwoComponentTime = input; }

    /*! Set two comp baseline
     */
    void setTwoComponentBaseline(double input) {  m_TwoComponentBaseline = input; }

    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get Data or MC flag
     * @return Data or MC flag. MC=false Data=true;
     */
    bool getisData() const { return m_isData; }

    /*! Get Dsp Array
     * @return Dsp Array 0~31
     */
    void getDspA(int  DspArray[31]) const
    {
      for (int i = 0; i < 31; i++)
        DspArray[i] = m_DspAVector[i];
    }


    /*! get two comp total amp
     * @return two comp total amp
     */
    double getTwoComponentTotalAmp() const { return m_TwoComponentTotalAmp; }

    /*! get two comp hadron amp
     * @return two comp hadron amp
     */
    double getTwoComponentHadronAmp() const { return m_TwoComponentHadronAmp; }

    /*! get two comp chi2
     * @return two comp chi2
     */
    double getTwoComponentChi2() const { return m_TwoComponentChi2; }

    /*! get two comp time
     * @return two comp time
     */
    double getTwoComponentTime() const { return m_TwoComponentTime; }

    /*! get two comp baseline
     * @return two comp baseline
     */
    double getTwoComponentBaseline() const { return m_TwoComponentBaseline; }


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
    bool m_isData;      /**< Data = true, MC = false*/
    double m_TwoComponentTotalAmp; /**< Two comp total amp */
    double m_TwoComponentHadronAmp;   /**< Two comp hadron amp */
    double m_TwoComponentChi2; /**< Two comp chi2 */
    double m_TwoComponentTime; /**< Two comp time*/
    double m_TwoComponentBaseline; /**< Two comp baseline*/
    std::vector <int> m_DspAVector; /**< Dsp array vith variable length for calibration, tests, etc.  */

    /*2 dspa array with variable length*/
    /*3 Add two component variables*/
    ClassDef(ECLDsp, 3);

  };
} // end namespace Belle2

#endif
