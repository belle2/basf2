/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>


namespace Belle2 {

  /**
  * The Class for VXD global paramter payload
  */
  class VXDGlobalPar: public TObject {

  public:
    /** Constructor */
    VXDGlobalPar(float electronTolerance, float minimumElectrons, double  activeStepSize,
                 bool activeChips, bool seeNeutrons, bool onlyPrimaryTrueHits, bool onlyActiveMaterial,
                 float distanceTolerance, const std::string& defaultMaterial):
      m_electronTolerance(electronTolerance), m_minimumElectrons(minimumElectrons), m_activeStepSize(activeStepSize),
      m_activeChips(activeChips),  m_seeNeutrons(seeNeutrons),  m_onlyPrimaryTrueHits(onlyPrimaryTrueHits),
      m_onlyActiveMaterial(onlyActiveMaterial), m_distanceTolerance(distanceTolerance), m_defaultMaterial(defaultMaterial)
    {}
    /** Constructor */
    VXDGlobalPar() :
      m_electronTolerance(100), m_minimumElectrons(10), m_activeStepSize(0.005),
      m_activeChips(false),  m_seeNeutrons(false),  m_onlyPrimaryTrueHits(false),
      m_onlyActiveMaterial(false), m_distanceTolerance(0.005), m_defaultMaterial("Air")
    {}
    /** Get tolerance for the energy deposition in electrons to be merged in a single step */
    float getElectronTolerance() const { return m_electronTolerance; }
    /** Get minimum number of electrons to be deposited by a particle to be saved*/
    float getMinimumElectrons() const { return m_minimumElectrons; }
    /** Get stepsize to be used inside active volumes */
    double getActiveStepSize() const { return m_activeStepSize; }
    /** Get whether chips are sensitive  */
    bool getActiveChips() const { return m_activeChips; }
    /** Get whether sensitive detectors also see neutrons.*/
    bool getSeeNeutrons() const { return m_seeNeutrons; }
    /** Get if true only create TrueHits from primary particles and ignore secondaries */
    bool getOnlyPrimaryTrueHits() const { return m_onlyPrimaryTrueHits; }
    /** Get whether only active materials will be placed for tracking
        studies. Dead Material will be ignored */
    bool getOnlyActiveMaterial() const { return m_onlyActiveMaterial; }
    /** Get tolerance for Geant4 steps to be merged to a single step */
    float getDistanceTolerance() const { return m_distanceTolerance; }
    /** Get default material */
    std::string getDefaultMaterial() const { return m_defaultMaterial; }

  private:
    //! tolerance for the energy deposition in electrons to be merged in a single step
    float m_electronTolerance;
    //! minimum number of electrons to be deposited by a particle to be saved
    float m_minimumElectrons;
    //! Stepsize to be used inside active volumes
    double m_activeStepSize;
    //! Make also chips sensitive.
    bool m_activeChips;
    //! Make sensitive detectors also see neutrons.
    bool m_seeNeutrons;
    //! If true only create TrueHits from primary particles and ignore secondaries
    bool m_onlyPrimaryTrueHits;
    /** If this is true, only active Materials will be placed for tracking
     * studies. Dead Material will be ignored */
    bool m_onlyActiveMaterial;
    //! tolerance for Geant4 steps to be merged to a single step
    float m_distanceTolerance;
    //! default material
    std::string m_defaultMaterial;

    ClassDef(VXDGlobalPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

