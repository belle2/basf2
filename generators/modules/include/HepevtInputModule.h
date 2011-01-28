/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HEPEVTINPUTMODULE_H
#define HEPEVTINPUTMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <generators/dataobjects/MCParticle.h>
#include <generators/dataobjects/MCParticleGraph.h>
#include <generators/hepevt/HepevtReader.h>

namespace Belle2 {

  namespace Generators {

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
      virtual void initialize();

      /** Method is called for each event. */
      virtual void event();

    protected:

      int m_evtNumber;             /**< The current event number, used in the case that the HepEvt file doesn't provide it. */
      bool m_evtIDUsed;            /**< True if the event number from the HepEvt file was used and therefore the internal event number can't be used anymore. */
      std::string m_inputFileName; /**< The filename of the input HepEvt file. */
      int m_skipEventNumber;       /**< The number of events which should be skipped at the start of reading. */
      HepevtReader m_hepevt;       /**< An instance of the HepEvt reader. */
      MCParticleGraph mpg;         /**< The MCParticle graph object. */
      bool m_useWeights;           /**< Parameter to switch on/off weight propagation */
    };

  }//end namespace Generators

} // end namespace Belle2

#endif // HEPEVTINPUTMODULE_H
