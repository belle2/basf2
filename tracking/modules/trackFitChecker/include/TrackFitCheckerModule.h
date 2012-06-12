/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef TrackFitCheckerModule_H_
#define TrackFitCheckerModule_H_

#include <framework/core/Module.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <cdc/dataobjects/CDCRecoHit.h>


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

//genfit stuff
#include <GFTrack.h>

//stuff for root output
#include <TTree.h>
#include <TFile.h>



namespace Belle2 {

  /*!
      This module calculates the resudals, pulls and chi^2 for a sample of tracks both track wise and layer wise tests are used. If availiable truth info from simulation in form of "TrueHits" is used.
  */

  class TrackFitCheckerModule : public Module {


    typedef boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::variance(boost::accumulators::lazy) > > StatisticsAccuWithMeanAndVar;
    typedef StatisticsAccuWithMeanAndVar StatisticsContainer;
  public:

    //! Constructor
    TrackFitCheckerModule();

    //! Destructor
    ~TrackFitCheckerModule();

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
    double calcChi2(const TMatrixT<double>& res, const TMatrixT<double>& R) const;
    std::vector<double> calcZs(const TMatrixT<double>& res, const TMatrixT<double>& R) const;
    std::vector<double> calcTestsWithTruthInfo(const TMatrixT<double>& state, const TMatrixT<double>& cov, const TMatrixT<double>& trueState) const;
    void isMatrixCov(const TMatrixT<double>& cov);
    bool isSymmetric(const TMatrixT<double>& aMatrix) const;
    bool hasMatrixNegDiagElement(const TMatrixT<double>& aMatrix) const;

    // functions for dataflow inside module
    void registerTrackWiseData(const std::string& nameOfDataSample);
    void registerTrackWiseVecData(const std::string& nameOfDataSample, const int nVarsToTest);
    void registerLayerWiseData(const std::string& nameOfDataSample, const int nVarsToTest);

    void fillTrackWiseData(const std::string& nameOfDataSample, const double newData);
    void fillTrackWiseVecData(const std::string& nameOfDataSample, const std::vector<double>& newData);
    void fillLayerWiseData(const std::string& nameOfDataSample, const int accuVecIndex, const std::vector<double>& newData);

    void printTrackWiseStatistics(const std::string& nameOfDataSample, const bool count = false);
    void printTrackWiseVecStatistics(const std::string& nameOfDataSample, const std::vector<std::string>& trackWiseVarNames, const  bool count = false);
    void printLayerWiseStatistics(const std::string& nameOfDataSample,  const std::vector<std::string>& layerWiseVarNames, const bool count = true);

    int m_nSiLayers; // number of Si layers. That is 6 of course.
    int m_nPxdLayers; // number of PXD layer (2) so number of SVD layers will be m_nSiLayers - m_nPxdLayers
    int m_nSvdLayers;
    int m_nCdcLayers;
    int m_nLayers;

    std::vector<std::string> m_layerWiseTruthTestsVarNames;
    std::vector<std::string> m_vertexTestsVarNames;


    // the following 6 maps should not be accessed dirctely but only with the corresponding "register" "fill" and "print" fucntions
    //the following maps will be filled with the test data so that statistical quantities like mean and variance can be calculated
    std::map<std::string, StatisticsContainer > m_trackWiseDataSamples;
    std::map<std::string, std::vector<StatisticsContainer> > m_trackWiseVecDataSamples;
    std::map<std::string, std::vector<std::vector<StatisticsContainer> > > m_layerWiseDataSamples;

    //the following maps will be filled with the test data so they can be written into a root file for further analysis with root
    std::map<std::string, float > m_trackWiseDataForRoot;
    std::map<std::string, std::vector<float>* > m_trackWiseVecDataForRoot;
    std::map<std::string, std::vector< std::vector <float> >* > m_layerWiseDataForRoot;

    // this maps will hold the names of the test data variables that have more then one component like the residuals of the origin position and momentum
    std::map<std::string, std::vector<std::string>* > namesOfTestVars;

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
    // module input parameters
    bool m_useTruthInfo;
    bool m_testPrediction;
    bool m_testDaf;
    bool m_truthAvailable;
    bool m_inspectTracks;
    std::string m_dataOutFileName; //common part of all names of output files


    //stuff for text file output
    std::stringstream m_textOutput;
    std::ofstream m_dataOut;



    bool m_wAndPredPresentsTested;
    //stuff for root output
    bool m_writeToFile;
    bool m_writeToRootFile;

    TTree* m_statDataTreePtr;
    TFile* m_rootFilePtr;

    //hold all the data needed for the layer wise tests of one track
    struct TrackData {

      int nHits; //number of hits in GFTrack object

      //stuff for the normal layer wise tests:

      std::vector<int> accuVecIndices;
      std::vector<int> detIds;
      std::vector<int> hitDims;

      std::vector<TMatrixD> ms;
      std::vector<TMatrixD> Hs;
      std::vector<TMatrixD> Vs;
      std::vector<TMatrixD> states_fu;
      std::vector<TMatrixD> covs_fu;
      std::vector<TMatrixD> states_bu;
      std::vector<TMatrixD> covs_bu;
      std::vector<TMatrixD> states_sm;
      std::vector<TMatrixD> covs_sm;

      //aditional stuff for the tests that need truth info
      std::vector<TMatrixD> states_t;

      //aditional stuff for the tests that need predicted state
      std::vector<TMatrixD> states_fp;
      std::vector<TMatrixD> covs_fp;
      std::vector<TMatrixD> states_bp;
      std::vector<TMatrixD> covs_bp;

    };
    TrackData m_trackData;

    void setTrackData(GFTrack* const aTrackPtr, const double charge);

    void testDaf(GFTrack* const aTrackPtr);
    //void testDafWithBG(GFTrack *const aTrackPtr);
    //void fillDafWithBGData(const std::string& nameOfDataSample, const std::vector<std::vector<float> >& allWeights);
    void normalTests();

    void truthTests();

    void inspectTracks(double chi2tot_fu, double vertexAbsMom);

  };
}

#endif /* TrackFitCheckerModule_H_ */
