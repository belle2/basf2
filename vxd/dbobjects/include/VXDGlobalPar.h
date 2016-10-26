/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <TObject.h>
#include <string>


namespace Belle2 {

  class GearDir;


  /**
  * The Class for VXD global paramter payload
  */

  class VXDGlobalPar: public TObject {

  public:
    //! Default constructor
    VXDGlobalPar() {}
    //! Constructor using Gearbox
    explicit VXDGlobalPar(const GearDir& content) { read(content); }
    //! Destructor
    ~VXDGlobalPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
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
    float m_electronTolerance {100};
    //! minimum number of electrons to be deposited by a particle to be saved
    float m_minimumElectrons {10};
    //! Stepsize to be used inside active volumes
    double m_activeStepSize {0.005};
    //! Make also chips sensitive.
    bool m_activeChips {false};
    //! Make sensitive detectors also see neutrons.
    bool m_seeNeutrons {false};
    //! If true only create TrueHits from primary particles and ignore secondaries
    bool m_onlyPrimaryTrueHits {false};
    /** If this is true, only active Materials will be placed for tracking
     * studies. Dead Material will be ignored */
    bool m_onlyActiveMaterial {false};
    //! tolerance for Geant4 steps to be merged to a single step
    float m_distanceTolerance {0.005};
    //! default material
    std::string m_defaultMaterial;

    ClassDef(VXDGlobalPar, 5);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

