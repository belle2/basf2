/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>
#include <top/dbobjects/TOPGeoModule.h>
#include <top/dbobjects/TOPGeoFrontEnd.h>
#include <top/dbobjects/TOPGeoQBB.h>
#include <top/dbobjects/TOPNominalQE.h>
#include <top/dbobjects/TOPNominalTTS.h>
#include <top/dbobjects/TOPNominalTDC.h>
#include <top/dbobjects/TOPSignalShape.h>
#include <framework/gearbox/Unit.h>
#include <vector>


namespace Belle2 {

  /**
   * Geometry parameters of TOP
   */
  class TOPGeometry: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeometry()
    {}

    /**
     * Constructor with name
     * @param name object name
     */
    explicit TOPGeometry(const std::string& name): TOPGeoBase(name)
    {}

    /**
     * Use basf2 units when returning geometry parameters
     */
    static void useBasf2Units() {s_unit = Unit::cm; s_unitName = "cm";}

    /**
     * Use Geant units when returning geometry parameters
     */
    static void useGeantUnits() {s_unit = Unit::mm; s_unitName = "mm";}

    /**
     * Appends module (if its ID differs from already appended modules)
     * @param module module geometry parameters
     */
    void appendModule(const TOPGeoModule& module);

    /**
     * Sets front-end
     * @param frontEnd front-end geometry parameters
     * @param num number of board stacks per module
     */
    void setFrontEnd(const TOPGeoFrontEnd& frontEnd, unsigned num = 4)
    {
      m_frontEnd = frontEnd;
      m_numBoardStacks = num;
    }

    /**
     * Sets quartz bar box
     * @param QBB quartz bar box geometry parameters
     */
    void setQBB(const TOPGeoQBB& QBB) {m_QBB = QBB;}

    /**
     * Sets nominal quantum efficiency of PMT
     * @param nominalQE nominal quantum efficiency
     */
    void setNominalQE(const TOPNominalQE& nominalQE) {m_nominalQE = nominalQE;}

    /**
     * Sets nominal time transition spread of PMT
     * @param nominalTTS nominal TTS
     */
    void setNominalTTS(const TOPNominalTTS& nominalTTS) {m_nominalTTS = nominalTTS;}

    /**
     * Sets nominal time-to-digit conversion parameters
     * @param nominalTDC nominal TDC parameters
     */
    void setNominalTDC(const TOPNominalTDC& nominalTDC) {m_nominalTDC = nominalTDC;}

    /**
     * Sets single photon signal shape
     * @param signalShape signal shape
     */
    void setSignalShape(const TOPSignalShape& signalShape) {m_signalShape = signalShape;}

    /**
     * Sets calibration pulse shape
     * @param shape calibration pulse shape
     */
    void setCalPulseShape(const TOPSignalShape& shape) {m_calPulseShape = shape;}

    /**
     * Returns number of modules
     * @return number of modules
     */
    unsigned getNumModules() const {return m_modules.size();}

    /**
     * Returns module
     * @param moduleID valid module ID (1-based)
     * @return module geometry parameters
     */
    const TOPGeoModule& getModule(int moduleID) const;

    /**
     * Returns all modules
     * @return modules
     */
    const std::vector<TOPGeoModule>& getModules() const {return m_modules;}

    /**
     * Checks if module exists in m_modules
     * @param moduleID module ID (1-based)
     * @return true if exists
     */
    bool isModuleIDValid(int moduleID) const;

    /**
     * Returns front-end
     * @return front-end geometry parameters
     */
    const TOPGeoFrontEnd& getFrontEnd() const {return m_frontEnd;}

    /**
     * Returns number of boardstacks per module
     * @return number of boardstacks per module
     */
    unsigned getNumBoardStacks() const {return m_numBoardStacks;}

    /**
     * Returns quartz bar box
     * @return quartz bar box geometry parameters
     */
    const TOPGeoQBB& getQBB() const {return m_QBB;}

    /**
     * Returns nominal quantum efficiency of PMT
     * @return nominal quantum efficiency
     */
    const TOPNominalQE& getNominalQE() const {return m_nominalQE;}

    /**
     * Returns nominal time transition spread of PMT
     * @return nominal TTS
     */
    const TOPNominalTTS& getNominalTTS() const {return m_nominalTTS;}

    /**
     * Returns nominal time-to-digit conversion parameters
     * @return nominal TDC parameters
     */
    const TOPNominalTDC& getNominalTDC() const {return m_nominalTDC;}

    /**
     * Returns single photon signal shape
     * @return signal shape
     */
    const TOPSignalShape& getSignalShape() const {return m_signalShape;}

    /**
     * Returns calibration pulse shape
     * @return calibration pulse shape
     */
    const TOPSignalShape& getCalPulseShape() const {return m_calPulseShape;}

    /**
     * Returns inner radius of the volume devoted to TOP counter
     * @return inner radius
     */
    double getInnerRadius() const;

    /**
     * Returns outer radius of the volume devoted to TOP counter
     * @return outer radius
     */
    double getOuterRadius() const;

    /**
     * Returns average radius of modules
     * @return radius
     */
    double getRadius() const;

    /**
     * Returns backward z of the volume devoted to TOP counter
     * @return minimal z
     */
    double getBackwardZ() const;

    /**
     * Returns forward z of the volume devoted to TOP counter
     * @return maximal z
     */
    double getForwardZ() const;

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "TOP geometry parameters") const;


  private:

    std::vector<TOPGeoModule> m_modules; /**< geometry parameters of modules */
    TOPGeoFrontEnd m_frontEnd;  /**< geometry parameters of front-end electronics */
    TOPGeoQBB m_QBB;  /**< geometry parameters of quartz bar box */
    unsigned m_numBoardStacks = 0;  /**< number of boardstacks per module */
    TOPNominalQE m_nominalQE; /**< nominal quantum efficiency of PMT */
    TOPNominalTTS m_nominalTTS; /**< nominal time transition spread of PMT */
    TOPNominalTDC m_nominalTDC; /**< nominal time-to-digit conversion parameters */
    TOPSignalShape m_signalShape; /**< shape of single photon signal */
    TOPSignalShape m_calPulseShape; /**< shape of the calibration pulse */

    ClassDef(TOPGeometry, 5); /**< ClassDef */

  };

} // end namespace Belle2
