/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTMETAGEN_H_
#define EVTMETAGEN_H_

#include <framework/core/Module.h>
#include <vector>


namespace Belle2 {

  //! The event meta data generator module.
  /*!
     This module generates the event meta data information
     like experiment, run and event number. It is meant to
     be used in cases where no module generating this information
     (e.g. an input module loading a file) is available.
  */
  class EvtMetaGen : public Module {

  public:

    //! Macro which adds a method to return a new instance of the module.
    NEW_MODULE(EvtMetaGen)

    //! Constructor
    EvtMetaGen();

    //! Destructor
    virtual ~EvtMetaGen();

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

    int m_evtNumber; /*!< The current event number. */
    int m_colIndex;  /*!< The current index for the exp and run lists. */

    std::vector<int> m_expList;      /*!< The list (column) of experiments. */
    std::vector<int> m_runList;      /*!< The list (column) of runs. */
    std::vector<int> m_evtStartList; /*!< The list (column) of event numbers to start with. */
    std::vector<int> m_evtEndList;   /*!< The list (column) of event numbers to end with. */
  };
}

#endif /* EVTMETAGEN_H_ */
