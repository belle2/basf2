/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTMETAINFO_H_
#define EVTMETAINFO_H_

#include <framework/core/Module.h>
#include <vector>


namespace Belle2 {

  /*! The event meta data info module. */
  /*!
     This module prints the current event meta data information to
     the command line.
     (e.g. Could be used later to also send the info over TCP/IP to a GUI)
  */
  class EvtMetaInfo : public Module {

  public:

    /*! Constructor */
    EvtMetaInfo();

    /*! Destructor */
    virtual ~EvtMetaInfo();

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

  };
}

#endif /* EVTMETAINFO_H_ */
