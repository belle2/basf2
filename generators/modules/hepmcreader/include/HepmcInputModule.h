/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frrederik Krohn                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <string>

#include <mdst/dataobjects/MCParticleGraph.h>

#include <generators/hepmc/HepMCReader.h>
#include <generators/utilities/InitialParticleGeneration.h>

namespace Belle2 {

  /** The HepMCInput module.
   * Loads events from a HepMC file and stores the content
   * into the MCParticle class.
   */
  class HepMCInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    HepMCInputModule();

    /** Destructor. */
    virtual ~HepMCInputModule() {}

    /** Initializes the module. */
    virtual void initialize() override;

    /** method is called for each event. */
    virtual void event() override;

    /** method is called at end */
    virtual void terminate() override;

  protected:

    std::vector <std::string>  m_inputFileNames; /**< The list of filenames of input HepMCfile. */
    std::string m_inputFileName;   /**< The Name of the current input HepMC file. */
    uint m_iFile;                     /**< Index of the current HepMC input file. */
    int m_nVirtual;                  /**< The number of particles in each event that should be made virtual */
    std::unique_ptr<HepMCReader> m_hepmcreader;           /**< An instance of the HepMC reader. */
    MCParticleGraph m_mcParticleGraph;             /**< The MCParticle graph object. */
    bool m_useWeights;               /**< Parameter to switch on/off weight propagation */
    bool m_boost2Lab;                /**< Parameter to switch on/off boost to LAB system */
    bool m_wrongSignPz;              /**< Parameter to signal that direction of LER and HER was switched*/
    bool m_ignorereadEventNr;               /**< Count event numbers 'manually' */
    int m_runNum;                    /**< The run number that should be used if the reader acts as master */
    int m_expNum;                    /**< The experiment number that should be used if the reader acts as master */
    int m_evtNum;                    /**< The event number is needed if the reader acts as master */
    int m_minEvent;                    /**< Start at event number x. */
    int m_maxEvent;                    /**< Stop after processing n events. */
    int m_totalEvents;                    /**< totla number of events to read */
    //int m_skipNEvents;                    /**< skip events in the file */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */
  private:
    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr; /**< event meta data pointer to control event nubmer etc */
  };

} // end namespace Belle2

