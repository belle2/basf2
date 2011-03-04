/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* This module checks if a track hits a user given SVD layer and writes the initial
 * momentum and theta angle to a file.
 * It will only selcet tracks that hit every SVD layer until the given one and the
 * given layer too OR
 * if the option notEven is given the module will selcet every particle that does NOT
 * hit the stated SVD layer and also non of the following layers.
 * IMPORTANT for performance reasons this modules assumes that in the mcParticleArray all
 * the particles that were created by the particle gun come first that means before the particles
 * form created by material effects. If this stops beeing the case a few lines
 * of code have to be deleted.
 * IMPORTANT this modules was only tested with ONE event
 */

#ifndef svdHitCounterModule_H_
#define svdHitCounterModule_H_

#include <framework/core/Module.h>
//#include <cdc/hitcdc/CDCSimHit.h>
#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>
#include <framework/dataobjects/Relation.h>
//#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <framework/gearbox/Unit.h>
//#include <vector>
#include <fstream>
#include <string>
//#include <iostream>
//#include <sstream>

namespace Belle2 {

  //! Exercise 1 module.
  /*!
      Add here a description what your module does.
  */
  class svdHitCounterModule : public Module {

  public:

    //! Constructor
    svdHitCounterModule();

    //! Destructor
    virtual ~svdHitCounterModule();

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

    //std::string m_cdcSimHitColName;
    //std::string m_pxdSimHitColName;
    std::string m_svdSimHitColName;
    std::string m_mcPartColName;

  private:
    std::string m_dataOutFileName;
    std::ofstream m_dataOut;
    //std::ofstream m_dataOut2;
    int m_nTracks;
    int m_svdLayerId;
    std::string m_option;
    bool m_atLeast;

    //std::vector<double> m_testVec;


  };
}

#endif /* svdHitCounterModule_H_ */
