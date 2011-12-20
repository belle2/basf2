/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef trackFitCheckerModule_H_
#define trackFitCheckerModule_H_

#include <framework/core/Module.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <iostream>
#include <iomanip>

#include <TMatrixT.h>

// to get statistics functions of boost
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/count.hpp>

//stuff for root output
#include <TTree.h>
#include <TFile.h>
#include <tracking/TrackWiseDataStruct.h>
#include <tracking/TrackWiseVecDataStruct.h>
#include <tracking/LayerWiseData.h>


namespace Belle2 {

  /*!
      This module calculates the resudals, pulls and chi^2 for a sample of tracks both track wise and layer wise tests are used. If availiable truth info from simulation in form of "TrueHits" is used.
  */

  class trackFitCheckerModule : public Module {


    typedef boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::variance(boost::accumulators::lazy) > > StatisticsAccuWithMeanAndVar;
    typedef StatisticsAccuWithMeanAndVar StatisticsContainer;
  public:

    //! Constructor
    trackFitCheckerModule();

    //! Destructor
    ~trackFitCheckerModule();

    //! Initialize the Module
    /*! Function is called only once at the beginning of your job at the beginning of the corresponding module.
        Things that can be done here, should be done here, as it is relatively cheap in terms of CPU time.

        This method has to be implemented by subclasses.
    */
    void initialize();

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.

        This method has to be implemented by subclasses.
    */
    void beginRun();

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
    void endRun();

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.

        This method has to be implemented by subclasses.
    */
    void terminate();


  protected:

    // little helper functions for this module
    double calcChi2(const TMatrixT<double>& res, const TMatrixT<double>& R);
    std::vector<double> calcZs(const TMatrixT<double>& res, const TMatrixT<double>& R);
    std::vector<double> calcTestsWithTruthInfo(const TMatrixT<double>& state, const TMatrixT<double>& cov, const TMatrixT<double>& trueState);
    void isMatrixCov(const TMatrixT<double>& cov);
    bool isSymmetric(const TMatrixT<double>& aMatrix);
    bool hasMatrixNegDiagElement(const TMatrixT<double>& aMatrix);
    void printLayerWiseStatistics(const std::string& nameOfDataSample,  const std::vector<std::string>& layerWiseVarNames);
    void resizeLayerWiseData(const std::string& nameOfDataSample, const int nVarsToTest);
    void printTrackWiseStatistics(const std::string& nameOfDataSample);
    void printTrackWiseVecStatistics(const std::string& nameOfDataSample, const std::vector<std::string>& trackWiseVarNames);
    void fillLayerWiseData(const std::string& nameOfDataSample, const int accuVecIndex, const std::vector<double>& newData);
    void fillTrackWiseVecData(const std::string& nameOfDataSample, const std::vector<double>& newData);
    void fillTrackWiseData(const std::string& nameOfDataSample, const double newData);

    int m_nSiLayers; // number of Si layers. That is 6 of course.
    int m_nPxdLayers; // number of PXD layer (2) so number of SVD layers will be m_nSiLayers - m_nPxdLayers
    int m_nSvdLayers;
    int m_nCdcLayers;
    int m_nLayers;
    std::string m_dataOutFileName;
    std::string m_dataOutFileName2;
    std::ofstream m_dataOut;
    std::ofstream m_dataOut2;
    std::vector<std::string> m_layerWiseTruthTestsVarNames;
    std::vector<std::string> m_vertexTestsVarNames;

    //test first layer with the smearing I set:

    /*    statisticsAccuWithMeanAndVar m_z_pf_qOverPsLayer1;
        statisticsAccuWithMeanAndVar m_z_pf_dudwsLayer1;
        statisticsAccuWithMeanAndVar m_z_pf_dvdwsLayer1;
        statisticsAccuWithMeanAndVar m_z_pf_usLayer1;
        statisticsAccuWithMeanAndVar m_z_pf_vsLayer1;
        statisticsAccuWithMeanAndVar m_pf_chi2sLayer1;
    */


    std::map<std::string, StatisticsContainer > m_trackWiseDataSamples;
    std::map<std::string, std::vector<StatisticsContainer> > m_trackWiseVecDataSamples;
    std::map<std::string, std::vector<std::vector<StatisticsContainer> > > m_layerWiseDataSamples;

    std::map<std::string, float* > m_trackWiseDataForRoot;
//    std::map<std::string, TrackWiseVecDataStruct* > m_trackWiseVecDataForRoot;
    std::map<std::string, std::vector<float>* > m_trackWiseVecDataForRoot;
    std::map<std::string, LayerWiseData* > m_layerWiseDataForRoot;
    //bool m_testGeant3;
    int m_failedSmootherCounter;
    int m_processedTracks;
    double m_totalChi2Cut;
    int m_nCutawayTracks;
    // counters holding the number of the covariance matrices (R) of the resuduals that have negative diagonal elements
    int m_badR_fCounter;
    int m_badR_bCounter;
    int m_badR_smCounter;
    bool m_testSi;
    bool m_testCdc;
    int m_unSymmetricCounter;
    int m_notPosDefCounter;
    int m_nDigits;
    bool m_useTruthInfo;
    bool m_testPrediction;
    //output
    //bool m_writeToB2info;
    std::string m_testOutputFileName;
    std::stringstream m_textOutput;

    bool m_writeToFile;
    bool m_writeToRootFile;

    //stuff for root output
    TTree* m_statDataTreePtr;
    TFile* m_rootFilePtr;

  };
}

#endif /* trackFitCheckerModule_H_ */
