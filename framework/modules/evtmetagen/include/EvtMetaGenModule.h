/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTMETAGENMODULE_H_
#define EVTMETAGENMODULE_H_

#include <framework/core/Module.h>
#include <vector>


namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   */

  /**
   * The event meta data generator module.
   *
   * This module generates the event meta data information
   * like experiment, run and event number. It is meant to
   * be used in cases where no module generating this information
   * (e.g. an input module loading a file) is available.
   */
  class EvtMetaGenModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    EvtMetaGenModule();

    /** Destructor. */
    virtual ~EvtMetaGenModule();

    /** Initializes the Module.
     *
     * Does a check of the parameters and gives an error of the parameters
     * were not correctly set by the user.
     */
    virtual void initialize();

    /** Stores the event meta data into the DataStore.
     *
     * Based on the parameters set by the user the current event meta data
     * is calculated (event number, run number, experiment number) and stored
     * into the event part of the DataStore. If a new run is started the
     * endRun process type followed by the beginRun process type is returned.
     * Once the end of the processing is reached, the end of data process type
     * is returned.
     */
    virtual void event();


  protected:


  private:

    unsigned long m_evtNumber; /**< The current event number. */
    int m_colIndex;  /**< The current index for the exp and run lists. */

    std::vector<int> m_expList;      /**< The list (column) of experiments. */
    std::vector<int> m_runList;      /**< The list (column) of runs. */
    std::vector<int> m_evtNumList;   /**< The list (column) of the number of events which should be processed. */
  };
  /*! @} */
}

#endif /* EVTMETAGENMODULE_H_ */
