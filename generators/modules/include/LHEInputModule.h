/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LHEINPUTMODULE_H
#define LHEINPUTMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <generators/lhe/LHEReader.h>
#include <generators/utilities/InitialParticleGeneration.h>

namespace Belle2 {

  /** The LHEInput module.
   * Loads events from a LHE file and stores the content
   * into the MCParticle class.
   */
  class LHEInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    LHEInputModule();

    /** Destructor. */
    virtual ~LHEInputModule() {}

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

  protected:

    std::vector <std::string>  m_inputFileNames; /**< The list of filenames of input LHEfile. */
    std::string m_inputFileName;     /**< The Name of the current input LHE file. */
    uint m_iFile;                    /**< Index of the current LHE input file. */
    int m_skipEventNumber;           /**< The number of events which should be skipped at the start of reading. */
    int m_nInitial;                  /**< The number of particles in each event that should be made initial */
    int m_nVirtual;                  /**< The number of particles in each event that should be made virtual */
    LHEReader m_lhe;                 /**< An instance of the LHE reader. */
    MCParticleGraph mpg;             /**< The MCParticle graph object. */
    bool m_useWeights;               /**< Parameter to switch on/off weight propagation */
    bool m_boost2Lab;                /**< Parameter to switch on/off boost to LAB system */
    bool m_wrongSignPz;              /**< Parameter to signal that direction of LER and HER was switched*/
    bool m_makeMaster;               /**< Parameter to signal if the modul should act as master */
    int m_runNum;                    /**< The run number that should be used if the reader acts as master */
    int m_expNum;                    /**< The experiment number that should be used if the reader acts as master */
    int m_evtNum;                    /**< The event number is needed if the reader acts as master */
    double m_l0;                     /**< Dispalced distance to IP for dark photon, default to be zero */

  private:
    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */

  };

} // end namespace Belle2

#endif // HEPEVTINPUTMODULE_H
