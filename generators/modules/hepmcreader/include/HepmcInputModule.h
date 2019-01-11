/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frrederik Krohn                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HEPMCINPUTMODULE_H
#define HEPMCINPUTMODULE_H

#include <framework/core/Module.h>

#include <string>

#include <mdst/dataobjects/MCParticle.h>
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

    /** Method is called for each event. */
    virtual void event() override;

  protected:

    std::vector <std::string>  m_inputFileNames; /**< The list of filenames of input HepMCfile. */
    std::string m_inputFileName;   /**< The Name of the current input HepMC file. */
    uint m_iFile;                     /**< Index of the current HepMC input file. */
    int m_nVirtual;                  /**< The number of particles in each event that should be made virtual */
    HepMCReader m_hepmcreader;           /**< An instance of the HepMC reader. */
    MCParticleGraph mcParticleGraph;             /**< The MCParticle graph object. */
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

#endif // HEPMCINPUTMODULE_H
