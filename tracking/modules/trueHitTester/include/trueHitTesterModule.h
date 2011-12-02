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
 * this module will _at least_ be able to calculate the p values of reconstructed
 * tracks (should be uniformly distributed) from the total chi2 values, the chi2
 * values of the smoother with respect to the measurements for
 * every silicon layer (should be chi2(2) distributed for every layer), the
 * normalized residuals of the smoother for every layer (should be standard
 * normal distributed) and the chi2 values of the smoothed track parameters with
 * respect to the true track parameters from geant4 (should be chi2(5)
 * distributed for every layer).
 */

#ifndef trueHitTesterModule_H_
#define trueHitTesterModule_H_

#include <framework/core/Module.h>
//#include <cdc/hitcdc/CDCSimHit.h>
//#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>

#include <framework/datastore/RelationIndex.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/VxdID.h>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>

// #include <tracking/dataobjects/MscAndEnLossData.h>

#include "GFTrack.h"
#include "GFTools.h"

#include <TRandom.h>

// to get statistics functions of boost
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/count.hpp>

namespace Belle2 {
  namespace BA = boost::accumulators;
  //! Exercise 1 module.
  /*!
      Add here a description what your module does.
  */

  class trueHitTesterModule : public Module {

  public:

    //! Constructor
    trueHitTesterModule();

    //! Destructor
    virtual ~trueHitTesterModule();

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

    void event();

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
    int m_nSiLayers; // number of Si layers. That is 6 of course.
    int m_nPxdLayers; // number of PXD layer (2) so number of SVD layers will be m_nSiLayers - m_nPxdLayers
    int m_nSvdLayers;
    std::vector<std::vector<float> > m_deltaEsSiLayers;
    std::vector<std::vector<double> >m_deltadudwsSiLayers;
    std::vector<std::vector<double> >m_deltadvdwsSiLayers;
    //std::vector<std::vector<double> >m_deltadudwsSiLayers;
    //std::vector<std::vector<double> >m_deltadvdwsSiLayers;
    std::string m_dataOutFileName;



  };
}

#endif /* trueHitTesterModule_H_ */
