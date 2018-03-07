//+
// File : HistoModule.h
// Description : Base class of module with histograms
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 30 - Jul - 2010
//-

#pragma once

#include <framework/core/Module.h>
#include <framework/pcore/RbTuple.h>

#define REG_HISTOGRAM   Belle2::RbTupleManager::Instance().register_module ( this );

namespace Belle2 {

  // Derived Module class  to replace Module class
  /**
      HistoModule.h is supposed to be used instead of Module.h for the modules
      with histogram definitions to be managed by HistoManager.
      A module HistoManager has to be registered on top of the module list for
      the management.
  */
  class HistoModule : public Module {
  public:
    /** Constructor */
    HistoModule() {};
    /** Destructor */
    virtual ~HistoModule() {};

    // Member functions (for event processing)
    /** Function for dynamic initialization of module */
    virtual void initialize() {};
    /** Function to process begin_run record */
    virtual void beginRun() {};
    /** Function to process event record */
    virtual void event() {};
    /** Function to process end_run record */
    virtual void endRun() {};
    /** Function to terminate module */
    virtual void terminate() {};

    /** Function to define histograms */
    /**
       This function is hooked to HistoManager by calling
               RbTupleManager::Instance().register_module ( this )
       or using a macro REG_HISTOGRAM.
       It is supposed to be done in initialize() function.
    */
    virtual void defineHisto() {};
  };

}
