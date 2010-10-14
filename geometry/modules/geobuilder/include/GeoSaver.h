/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOSAVER_H_
#define GEOSAVER_H_

#include <framework/core/Module.h>

#include <string>


namespace Belle2 {

  /*! The GeoSaver module. */
  /*!
     Saves the current detector geometry to a root file
  */
  class GeoSaver : public Module {

  public:

    /*! Constructor */
    GeoSaver();

    /*! Destructor */
    virtual ~GeoSaver();

    /*! Initialize the Module */
    /*! Function is called only once at the beginning of your job at the beginning of the corresponding module.
        Things that can be done here, should be done here, as it is relatively cheap in terms of CPU time.

        This method has to be implemented by subclasses.
    */
    virtual void initialize();

    /*! Called when entering a new run */
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.

        This method has to be implemented by subclasses.
    */
    virtual void beginRun();

    /*! Running over all events */
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.

        This method has to be implemented by subclasses.
    */
    virtual void event();

    /*! Is called after processing the last event of a run */
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.

        This method has to be implemented by subclasses.
    */
    virtual void endRun();

    /*! Is called at the end of your Module */
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.

        This method has to be implemented by subclasses.
    */
    virtual void terminate();


  protected:


  private:

    std::string m_filenameROOT; /*!< The filename of the ROOT file into which the geometry is saved. */

  };
}

#endif /* GEOSAVER_H_ */
