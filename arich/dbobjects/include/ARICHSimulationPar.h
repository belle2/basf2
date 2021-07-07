/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>

namespace Belle2 {
  //! The Class for ARICH simulation parameters
  /*! This class provides ARICH paramters for simulation, such as QE vs. wavelength curve for HAPDs,
      parameters to describe negative polarity crosstalk of channels of APD, ...
  */

  class ARICHSimulationPar : public TObject {

  public:

    //! Default constructor
    ARICHSimulationPar() {};

    /**
     * Get QE at given energy
     * @param e photon energy [eV]
     * @return QE
     */
    double getQE(double e) const;

    /**
    * Get HAPD collection efficiency
    * @return  collection efficiency
    */
    double getColEff() const {return (double)m_colEff;}

    /**
     * Get QE scaling factor for photons internally reflected in HAPD window
     */
    double getQEScaling() const {return (double)m_qeScale;}

    /**
     * Get absorbtion probability for photons internally reflected in HAPD window
     */
    double getWindowAbsorbtion() const {return (double)m_windowAbsorbtion;}

    /**
     * Get factor to suppress p.e. detection efficiency due to negative polarity crosstalk among chip channels
     */
    double getChipNegativeCrosstalk() const {return (double)m_chipNegativeCrosstalk;}

    /**
     * Get number of flat background hits/hapd/event to be added in digitizer
     */
    double getNBkgHits() const {return (double)m_nBkgHits;}

    /**
     * Set QE curve
     * @param lambdaFirst curve starting wavelength in nm
     * @param lambdaStep step between points
     * @param qe vector of QE values
     */
    void setQECurve(float lambdaFirst, float lambdaStep, const std::vector<float>& qe);

    /**
     * Set number of flat background hits/hapd/event to be added in digitizer
     * @param number of background hits/hapd/event
     */
    void setNBkgHits(float nbkg) { m_nBkgHits = nbkg;}

    /**
    * Set QE scaling factor for photons internally reflected in HAPD window
    * @param qescale QE scaling
    */
    void setQEScaling(double qescale) { m_qeScale = (float)qescale;}

    /**
    * Set absorbtion probability for photons internally reflected in HAPD window
    * @param abs absorbtion probability
    */
    void setWindowAbsorbtion(double abs) { m_windowAbsorbtion = (float)abs;}

    /**
    * Set factor to suppress p.e. detection efficiency due to negative polarity crosstalk among chip channels
    * @param cross level of negative crosstalk
    */
    void setChipNegativeCrosstalk(double cross) { m_chipNegativeCrosstalk = (float)cross;}

    /**
     * Set HAPD collection efficiency
     * @param colEff collection efficiency
     */
    void setCollectionEff(double colEff) {m_colEff = (float)colEff;}

    /**
     * Set peak QE value (for HAPD with maximal QE, at later stage HAPD dependent scaling is done)
     * @param peak QE at peak
     */
    void setPeakQE(double peak) {m_peakQE = (float)peak;}

    /**
     * print class content
     */
    void print() const;

  private:

    float m_qeScale = 0;                   /*!< QE scale factor for photons internally reflected in HAPD window */
    float m_windowAbsorbtion = 0;          /*!< absorbtion probability for photons internally reflected in HAPD window */
    float m_chipNegativeCrosstalk = 0;     /*!< to simulate opposite polarity crosstalk among channels on chip */
    float m_peakQE = 0;                    /*!< maximal peak QE of all HAPDs */

    std::vector<float> m_qe;               /*!< quantum efficiency curve */
    float m_colEff = 0;                    /*!< collection efficiency */
    float m_lambdaFirst = 0;               /*!< wavelength [nm]: first QE data point */
    float m_lambdaStep = 0;                /*!< wavelength [nm]: step */
    float m_nBkgHits = 0;                    /*!< number if flat background hits/hapd/event to be added in digitizer */

    ClassDef(ARICHSimulationPar, 2);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
