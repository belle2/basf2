/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Susanne Koblitz                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>

#include <mdst/dataobjects/MCParticleGraph.h>

#include <generators/hepevt/HepevtReader.h>
#include <generators/utilities/InitialParticleGeneration.h>

namespace Belle2 {

  /** The HepevtInput module.
   * Loads events from a HepEvt file and stores the content
   * into the MCParticle class.
   */
  class HepevtInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    HepevtInputModule();

    /** Destructor. */
    virtual ~HepevtInputModule() {}

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

  protected:

    std::vector <std::string>  m_inputFileNames; /**< The list of filenames of input HepEvtfile. */
    std::string m_inputFileName;   /**< The Name of the current input HepEvt file. */
    uint m_iFile;                     /**< Index of the current HepEvt input file. */
    int m_skipEventNumber;           /**< The number of events which should be skipped at the start of reading. */
    int m_nVirtual;                  /**< The number of particles in each event that should be made virtual */
    HepevtReader m_hepevt;           /**< An instance of the HepEvt reader. */
    MCParticleGraph mpg;             /**< The MCParticle graph object. */
    bool m_useWeights;               /**< Parameter to switch on/off weight propagation */
    bool m_boost2Lab;                /**< Parameter to switch on/off boost to LAB system */
    bool m_wrongSignPz;              /**< Parameter to signal that direction of LER and HER was switched*/
    bool m_makeMaster;               /**< Parameter to signal if the modul should act as master */
    int m_runNum;                    /**< The run number that should be used if the reader acts as master */
    int m_expNum;                    /**< The experiment number that should be used if the reader acts as master */
    int m_evtNum;                    /**< The event number is needed if the reader acts as master */

  private:
    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */

  };

} // end namespace Belle2

