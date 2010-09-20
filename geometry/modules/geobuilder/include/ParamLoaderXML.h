/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARAMLOADERXML_H_
#define PARAMLOADERXML_H_

#include <framework/core/Module.h>

#include <string>


namespace Belle2 {

  //! The ParamLoaderXML module.
  /*!
     This module loads the parameters from a XML document.

     Creates a GearboxIO object which reads the parameters
     for the Belle II detector geometry from XML files.
     Starting from the basic XML file, which includes all other
     XML files (e.g. for each subdetector one file), this method
     allows accessing all parameters from locally stored XML documents.
  */
  class ParamLoaderXML : public Module {

  public:

    //! Macro which adds a method to return a new instance of the module.
    NEW_MODULE(ParamLoaderXML)

    //! Constructor
    ParamLoaderXML();

    //! Destructor
    virtual ~ParamLoaderXML();

    //! Initialize the Module
    /*! Function is called only once at the beginning of your job at the beginning of the corresponding module.
        Things that can be done here, should be done here, as it is relatively cheap in terms of CPU time.

        This method has to be implemented by subclasses.
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.

        This method has to be implemented by subclasses.
    */
    virtual void beginRun();

    //! Running over all events
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.

        This method has to be implemented by subclasses.
    */
    virtual void event();

    //! Is called after processing the last event of a run
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.

        This method has to be implemented by subclasses.
    */
    virtual void endRun();

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.

        This method has to be implemented by subclasses.
    */
    virtual void terminate();


  protected:


  private:

    std::string m_filenameXML;   /*!< The filename of the XML file. */
    bool m_paramCheck; /*!< If set to true, performs a check of the path/parameter each time it is accessed. */

  };
}

#endif /* PARAMLOADERXML_H_ */
