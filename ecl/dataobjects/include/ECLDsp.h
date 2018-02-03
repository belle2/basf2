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
      m_TwoCompTotalAmp = -1;  /**< Offline two component total amplitude */
      m_TwoCompHadronAmp = -1; /**< Offline two component hadron amplitude */
      m_TwoCompChi2 = -1;  /**< Offline two component chi2*/
      m_TwoCompTime = 1;  /**< Offline two component time */
      m_TwoCompBaseline = 1;  /**< Offline two component baseline */
      m_DataMCFlag = false;  /**< Data = true MC = false */
    }

    /** Constructor for data*/
    ECLDsp(int CellId, int NADCPoints, int* ADCData, bool flag)
    {
      m_CellId     = CellId;
      m_DspAVector.assign(ADCData, ADCData + NADCPoints);
      m_DataMCFlag = flag;
    }

    /** Constructor for data*/
    ECLDsp(int CellId, std::vector<int> ADCData, bool flag)
    {
      m_CellId     = CellId;
      m_DspAVector = ADCData;
      m_DataMCFlag = flag;
    }

    /*! Set Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set Data or MC flag. MC = false. Data = true
     */
    void setDataMCFlag(bool flag) { m_DataMCFlag = flag; }

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
    void setTwoCompTotalAmp(double input) {  m_TwoCompTotalAmp = input; }

    /*! Set two comp hadron amp
     */
    void setTwoCompHadronAmp(double input) { m_TwoCompHadronAmp = input; }

    /*! Set two comp chi2
     */
    void setTwoCompChi2(double input) {      m_TwoCompChi2 = input; }

    /*! Set two comp time
     */
    void setTwoCompTime(double input) {      m_TwoCompTime = input; }

    /*! Set two comp baseline
     */
    void setTwoCompBaseline(double input) {  m_TwoCompBaseline = input; }

    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get Data or MC flag
     * @return Data or MC flag. MC=false Data=true;
     */
    bool getDataMCFlag() const { return m_DataMCFlag; }

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
    double getTwoCompTotalAmp() const { return m_TwoCompTotalAmp; }

    /*! get two comp hadron amp
     * @return two comp hadron amp
     */
    double getTwoCompHadronAmp() const { return m_TwoCompHadronAmp; }

    /*! get two comp chi2
     * @return two comp chi2
     */
    double getTwoCompChi2() const { return m_TwoCompChi2; }

    /*! get two comp time
     * @return two comp time
     */
    double getTwoCompTime() const { return m_TwoCompTime; }

    /*! get two comp baseline
     * @return two comp baseline
     */
    double getTwoCompBaseline() const { return m_TwoCompBaseline; }


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
    bool m_DataMCFlag;      /**< Data = true, MC = false*/
    double m_TwoCompTotalAmp; /**< Two comp total amp */
    double m_TwoCompHadronAmp;   /**< Two comp hadron amp */
    double m_TwoCompChi2; /**< Two comp chi2 */
    double m_TwoCompTime; /**< Two comp time*/
    double m_TwoCompBaseline; /**< Two comp baseline*/
    std::vector <int> m_DspAVector; /**< Dsp array vith variable length for calibration, tests, etc.  */

    /*2 dspa array with variable length*/
    /*3 Add two component variables*/
    ClassDef(ECLDsp, 3);

  };
} // end namespace Belle2

#endif
