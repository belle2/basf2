/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <array>

namespace Belle2 {

  /** DB object for parameters to create variable clusterTimeNorm90  */
  class ECLTimingNormalization: public TObject {


    //---------------------------------------------------
  public:

    /** Constructor. */
    ECLTimingNormalization() :
      m_timeWalkPar(),
      m_backgroundPar(),
      m_energyPar(),
      m_minTNormalization()
    { };


    //---------------------------------------------------
    //..Getters for the parameters

    /** Get the parameters to correct for time walk
     E0 / bias at E0 / lowE slope / highE slope / curvature / Emin / Emax */
    const std::array< std::array<float, 7>,  8736>& getTimeWalkPar() const {return m_timeWalkPar;}

    /** Get the parameters to correct for dependence on background level
     intercept / slope / p2 / min ootCrys / max ootCrys */
    const std::array< std::array<float, 5>,  8736>& getBackgroundPar() const {return m_backgroundPar;}

    /** Get the parameters to correct for dependence on crystal energy
     E0 / res at E0 / lowE slope / highE slope / curvature / Emin / Emax */
    const std::array< std::array<float, 7>,  8736>& getEnergyPar() const {return m_energyPar;}

    /** Get the minimum value for the time normalization*/
    float getMinTNormalization() const {return m_minTNormalization;}


    //---------------------------------------------------
    //..Setters for the parmeters

    /** Set the parameters to correct for time walk*/
    void setTimeWalkPar(const std::array< std::array<float, 7>,  8736>& timeWalkPar) {m_timeWalkPar = timeWalkPar;}

    /** Set the parameters to correct for dependence on background level*/
    void setBackgroundPar(const std::array< std::array<float, 5>,  8736>& backgroundPar) {m_backgroundPar = backgroundPar;}

    /** Set the parameters to correct for dependence on crystal energy*/
    void setEnergyPar(const std::array< std::array<float, 7>,  8736>& energyPar) {m_energyPar = energyPar;}

    /** Set the minimum value for the time normalization*/
    void setMinTNormalization(const float minTNormalization) {m_minTNormalization = minTNormalization;}


    //---------------------------------------------------
  private:

    std::array< std::array<float, 7>,  8736> m_timeWalkPar;  /**< time walk correction */
    std::array< std::array<float, 5>,  8736> m_backgroundPar;  /**< dependence on background level */
    std::array< std::array<float, 7>,  8736> m_energyPar;  /**< dependence on energy */
    float m_minTNormalization; /**< minimum value for tNormalization (ns) */

    ClassDef(ECLTimingNormalization, 1); /**< ClassDef */
  };
}


