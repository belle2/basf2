/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <generators/hepevt/HepevtReader.h>
#include <mdst/dataobjects/MCParticleGraph.h>

/* C++ headers. */
#include <string>

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
    bool m_wrongSignPz;              /**< Parameter to signal that direction of LER and HER was switched*/
    bool m_createEventMetaData;               /**< Parameter to allow the module to create EventMetaData and set event info  */
    int m_runNum;                    /**< The run number that should be used if the reader handles EventMetaData */
    int m_expNum;                    /**< The experiment number that should be used if the reader handles EventMetaData */
    int m_evtNum;                    /**< The event number is needed if the reader handles EventMetaData */

  private:

    StoreObjPtr<EventMetaData> m_eventMetaData; /**< Event meta data */

  };

} // end namespace Belle2

