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

#ifndef ECLDIGITENERGYCONSTANTS_H
#define ECLDIGITENERGYCONSTANTS_H
#include <TObject.h>

namespace Belle2 {

  /**
   *   Energy calibration constants per digit.
   */

  class ECLDigitEnergyConstants: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLDigitEnergyConstants(): m_cellID(0), m_amplitude(0), m_energy(0) {};

    /**
     * Constructor
     */
    ECLDigitEnergyConstants(int cellid, float amplitude, float energy): m_cellID(cellid), m_amplitude(amplitude), m_energy(energy) {};

    /**
     * Destructor
     */
    ~ECLDigitEnergyConstants() {};

    /** Return calibration amplitude
     * @return calibration amplitude
     */
    float getAmplitude() const {return m_amplitude; };

    /** Return calibration energy
     * @return calibration energy
     */
    float getEnergy() const {return m_energy; };

    /** Return cell ID
     * @return cell ID
     */
    int getCellID() const {return m_cellID; };

    /** Set calibration amplitude (measured)
     * @param calibration amplitude
     */
    void setConstant(float amplitude) {m_amplitude = amplitude; };

    /** Set calibration energy (known from physics, e.g. ee->gg kinematics)
     * @param calibration energy
     */
    void setEnergy(float energy) {m_energy = energy; };

    /** Set cell ID
     * @param cell ID
     */
    void setCellID(int cellid) {m_cellID = cellid; };

  private:
    int m_cellID;       /**< ECL cell ID */
    float m_amplitude;  /**< Calibration amplitude */
    float m_energy;     /**< Calibration energy */

    // 2: changed getter getCelleID from float to int
    ClassDef(ECLDigitEnergyConstants, 2); /**< ClassDef */
  };
} // end namespace Belle2
#endif
