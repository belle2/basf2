/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Susanne Koblitz                           *
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

      /** The mode the HepEvtInputModule is running in. */
      enum EInputMode {
        c_NotSet,         /**< The mode was not set yet. */
        c_EvtNumExternal, /**< The event number is taken from an external source (e.g. EvtMetaGen). */
        c_EvtNumFile      /**< The event number is taken from the HepEvt file.. */
      };

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

      EInputMode m_inputMode;      /**< The input mode of the module. Defines where the event number is taken from. */
      std::string m_inputFileName; /**< The filename of the input HepEvt file. */
      int m_skipEventNumber;       /**< The number of events which should be skipped at the start of reading. */
      int m_Nvirtual;              /**< The number of particles in each event that should be made virtual */
      HepevtReader m_hepevt;       /**< An instance of the HepEvt reader. */
      MCParticleGraph mpg;         /**< The MCParticle graph object. */
      bool m_useWeights;           /**< Parameter to switch on/off weight propagation */
      bool m_boost2LAB;           /**< Parameter to switch on/off boost to LAB system */
      bool m_wrongSignPz;           /**< Parameter to signal that direction of LER and HER was switched*/
    };

  }//end namespace Generators

} // end namespace Belle2

#endif // HEPEVTINPUTMODULE_H
