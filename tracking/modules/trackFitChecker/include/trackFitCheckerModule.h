/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* this is some kind of alpha version of a track fit tester. It is _planned_ that
 * this module will _at least_ be albe to calculate the p values of reconstructed
 * tracks (should be uniformly distributet) from the total chi2 values, the chi2
 * values of the smoother with respect to the measurements for
 * every sillicon layer (should be chi2(2) distributed for every layer), the
 * normalized residuals of the smoother for every layer (should ben standart
 * normal distributed) and the chi2 values of the smoothed track parameters with
 * respect to the true track parameters from geant4 (should be chi2(5)
 * distributed for every layer).
 */

#ifndef trackFitCheckerModule_H_
#define trackFitCheckerModule_H_

#include <framework/core/Module.h>
#include <cdc/hitcdc/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>
#include <framework/dataobjects/Relation.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDHit.h>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>

#include "GFTrack.h"
#include "GFTools.h"

// to get statistics functions of boost
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

namespace Belle2 {

  //! Exercise 1 module.
  /*!
      Add here a description what your module does.
  */
  class trackFitCheckerModule : public Module {

  public:

    //! Constructor
    trackFitCheckerModule();

    //! Destructor
    virtual ~trackFitCheckerModule();

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

    std::string m_cdcSimHitColName;
    std::string m_pxdSimHitColName;
    std::string m_svdSimHitColName;
    std::string m_mcParticleColName;
    std::string m_fittedTracksName;
  private:
    std::string m_dataOutFileName;
    std::string m_dataOutFileName2;
    std::ofstream m_dataOut;
    std::ofstream m_dataOut2;
    std::vector<double> m_testVec;
    boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::variance(boost::accumulators::lazy) > > m_pValues;
    boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::variance(boost::accumulators::lazy) > > m_absMoms;

    boost::accumulators::accumulator_set<double, boost::accumulators::stats< boost::accumulators::tag::mean> > m_vertexX;
    boost::accumulators::accumulator_set<double, boost::accumulators::stats< boost::accumulators::tag::mean> > m_vertexY;
    boost::accumulators::accumulator_set<double, boost::accumulators::stats< boost::accumulators::tag::mean> > m_vertexZ;

  };
}

#endif /* trackFitCheckerModule_H_ */
