/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TrueHitTesterModule_H_
#define TrueHitTesterModule_H_

#include <framework/core/Module.h>

#include <tracking/LayerWiseData.h>
//root
#include <TMatrixT.h>
#include <TTree.h>
#include <TFile.h>

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>



//// to get statistics functions of boost
//#include <boost/accumulators/accumulators.hpp>
//#include <boost/accumulators/statistics/stats.hpp>
//#include <boost/accumulators/statistics/mean.hpp>
//#include <boost/accumulators/statistics/variance.hpp>
//#include <boost/accumulators/statistics/count.hpp>

namespace Belle2 {

  class TrueHitTesterModule : public Module {

  public:

    //! Constructor
    TrueHitTesterModule();

    //! Destructor
    virtual ~TrueHitTesterModule();

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
    void registerLayerWiseData(const std::string& nameOfDataSample, const int nVarsToTest);
    void fillLayerWiseData(const std::string& nameOfDataSample, const int accuVecIndex, const std::vector<double>& newData);
    void fillTrackWiseVecData(const std::string& nameOfDataSample, const std::vector<double>& newData);
    void fillTrackWiseData(const std::string& nameOfDataSample, const double newData);
    std::vector<double> rootVecToStdVec(TMatrixT<double>& rootVector);
    //for root output
    std::string m_dataOutFileName;
    TFile* m_rootFilePtr;
    TTree* m_trueHitDataTreePtr;

    std::map<std::string, float* > m_trackWiseDataForRoot;
    std::map<std::string, std::vector<float>* > m_trackWiseVecDataForRoot;
    std::map<std::string, LayerWiseData* > m_layerWiseDataForRoot;


    int m_nLayers; // number of Si layers. That is 6 of course.
    int m_nPxdLayers; // number of PXD layer (2) so number of SVD layers will be m_nSiLayers - m_nPxdLayers
    int m_nSvdLayers;

    bool m_filter;

    int m_notPerfectCounter;


  };
}

#endif /* TrueHitTesterModule_H_ */
