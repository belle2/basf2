/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Timofey Uglov                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/





#ifndef EKLMDIGITIZATIONMODULE_H
#define EKLMDIGITIZATIONMODULE_H

#include <framework/core/Module.h>
#include <string>



namespace Belle2 {

  //! Module EKLMDigitizationModule
  /*!
    Simple module for reading EKLM hits
  */
  class EKLMDigitizationModule : public Module {

  public:

    //! Constructor
    /*!
      Create and allocate memory for variables here.
      Add the module parameters in this method.
      \param selfRegisterType True if this module is self-registering,
      otherwise false.
    */
    EKLMDigitizationModule();

    //! Destructor
    /*!
      Use the destructor to release the memory you allocated in the constructor.
    */
    virtual ~EKLMDigitizationModule();

    //! Initialize the Module
    /*! This method is called only once before the actual event
        processing starts.
        Use this method to initialize variables, open files etc.
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! Called at the beginning of each run, the method gives you the chance
        to change run dependent constants like alignment parameters, etc.
    */
    virtual void beginRun();

    //! This method is the core of the module.
    /*! This method is called for each event.
        All processing of the event has to take place in this method.
    */
    virtual void event();

    //! This method is called if the current run ends.
    /*! Use this method to store information,
        which should be aggregated over one run.
    */
    virtual void endRun();

    //! This method is called at the end of the event processing.
    /*! This method is called only once after the event processing finished.
        Use this method for cleaning up, closing files, etc.
    */
    virtual void terminate();

  protected:
  private:

    /**
     * Strip hits with npe lower this value will be marked as bad
     */
    double m_discriminatorThreshold;

    /**
     * Strip transformation file
     */
    std::string m_stripInfromationDBFile;

  };
} // end namespace Belle2
#endif

