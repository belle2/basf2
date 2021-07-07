/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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

    /** Offline two component fit type */
    enum TwoComponentFitType {
      poorChi2 = -1,  /**< All offline fit attempts were greater than chi2 threshold */
      photonHadron = 0,  /**< photon + hadron template fit */
      photonHadronBackgroundPhoton = 1,  /**< photon + hadron template + pile-up photon fit */
      photonDiodeCrossing = 2  /**< photon + diode template fit */
    };

    /** default constructor for ROOT */
    ECLDsp() : m_DspAVector(31, 0) {}

    /** Constructor for data*/
    ECLDsp(int CellId, int NADCPoints, int* ADCData)
    {
      m_CellId     = CellId;
      m_DspAVector.assign(ADCData, ADCData + NADCPoints);
    }

    /** Constructor for data*/
    ECLDsp(int CellId, const std::vector<int>& ADCData) :
      m_CellId(CellId), m_DspAVector(ADCData) {}

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
    void setDspA(const std::vector <int>& DspArrayVector)
    {
      m_DspAVector = DspArrayVector;
    }

    /*! Set two comp total amp
     */
    void setTwoComponentTotalAmp(double input) {  m_TwoComponentTotalAmp = input; }

    /*! Set two comp hadron amp
     */
    void setTwoComponentHadronAmp(double input) { m_TwoComponentHadronAmp = input; }

    /*! Set two comp diode amp
     */
    void setTwoComponentDiodeAmp(double input) { m_TwoComponentDiodeAmp = input; }

    /*! Set two comp chi2
     */
    void setTwoComponentChi2(double input) { m_TwoComponentChi2 = input; }

    /*! Set two comp chi2 for a fit type
     *   see enum TwoComponentFitType in ECLDsp.h for description of fit types.
     */
    void setTwoComponentSavedChi2(TwoComponentFitType FitTypeIn, double input)
    {
      unsigned int index = FitTypeIn ;
      m_TwoComponentSavedChi2[index] = input;
    }

    /*! Set two comp time
     */
    void setTwoComponentTime(double input) { m_TwoComponentTime = input; }

    /*! Set two comp baseline
     */
    void setTwoComponentBaseline(double input) {  m_TwoComponentBaseline = input; }

    /*! Set pile-up photon energy
     */
    void setbackgroundPhotonEnergy(double input) {  m_backgroundPhotonEnergy = input; }

    /*! Set pile-up photon time
     */
    void setbackgroundPhotonTime(double input) {  m_backgroundPhotonTime = input; }

    /*! Set fit type
     */
    void setTwoComponentFitType(TwoComponentFitType ft) { m_TwoComponentFitType = ft; }

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


    /*! get two comp total amp
     * @return two comp total amp
     */
    double getTwoComponentTotalAmp() const { return m_TwoComponentTotalAmp;}

    /*! get two comp hadron amp
     * @return two comp hadron amp
     */
    double getTwoComponentHadronAmp() const { return m_TwoComponentHadronAmp; }

    /*! get two comp diode amp
     * @return two comp diode amp
     */
    double getTwoComponentDiodeAmp() const { return m_TwoComponentDiodeAmp; }

    /*! get two comp chi2
     * @return two comp chi2
     */
    double getTwoComponentChi2() const { return m_TwoComponentChi2; }

    /*! get two comp chi2 for a fit type
     *  see enum TwoComponentFitType in ECLDsp.h for description of fit types.
     * @return two comp chi2 for fit type
     */
    double getTwoComponentSavedChi2(TwoComponentFitType FitTypeIn) const
    {
      unsigned int index = FitTypeIn ;
      return m_TwoComponentSavedChi2[index];
    }

    /*! get two comp time
     * @return two comp time
     */
    double getTwoComponentTime() const { return m_TwoComponentTime; }

    /*! get two comp baseline
     * @return two comp baseline
     */
    double getTwoComponentBaseline() const { return m_TwoComponentBaseline; }

    /*! get two comp fit type
     * @return two comp fit type
     */
    TwoComponentFitType getTwoComponentFitType() const { return m_TwoComponentFitType; }

    /*! get pile up photon energy
     * @return pile up photon energy
     */
    double getbackgroundPhotonEnergy() const { return m_backgroundPhotonEnergy; }

    /*! get pile up photon time
     * @return pile up photon time
     */
    double getbackgroundPhotonTime() const { return m_backgroundPhotonTime; }

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

    int m_CellId{0};                      /**< Cell ID */
    double m_TwoComponentTotalAmp{ -1};   /**< Two comp total amp */
    double m_TwoComponentHadronAmp{ -1};  /**< Two comp hadron amp */
    double m_TwoComponentDiodeAmp{ -1};   /**< Two comp diode amp */
    double m_TwoComponentChi2{ -1};       /**< Two comp chi2 */
    double m_TwoComponentSavedChi2[3] = { -1, -1, -1}; /**< Two comp chi2 for each fit tried in reconstruction */
    double m_TwoComponentTime{1};         /**< Two comp time*/
    double m_TwoComponentBaseline{1};     /**< Two comp baseline*/
    double m_backgroundPhotonEnergy{ -1}; /**< Pile-up photon energy*/
    double m_backgroundPhotonTime{ -1};   /**< Pile-up photon time*/
    TwoComponentFitType m_TwoComponentFitType{poorChi2};  /**< offline fit hypothesis.*/
    std::vector <int> m_DspAVector;       /**< Dsp array vith variable length for calibration, tests, etc.  */

    /** 2 dspa array with variable length*/
    /** 3 Add two component variables*/
    /** 4 Add diode and pile-up photon offline fit hypothesis*/
    /** 5 Added m_TwoComponentSavedChi2[3] to save chi2 for each fit tried */
    /** 6 removed IsData member */
    ClassDef(ECLDsp, 6);

  };
} // end namespace Belle2

