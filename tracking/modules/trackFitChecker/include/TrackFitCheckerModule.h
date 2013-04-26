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
#include <boost/accumulators/statistics/tail.hpp>
#include <boost/accumulators/statistics/median.hpp>
//genfit stuff
#include <GFTrack.h>

//stuff for root output
#include <TTree.h>
#include <TFile.h>



namespace Belle2 {

  /*!
      This module calculates the residuals, pulls and chi^2 for a sample of tracks both track wise and layer wise tests are used. If available truth info from simulation in form of "TrueHits" is used.
  */

  class TrackFitCheckerModule : public Module {


//    typedef boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::variance(boost::accumulators::lazy) > > StatisticsAccuWithMeanAndVar;
//    typedef StatisticsAccuWithMeanAndVar StatisticsContainer;
//    typedef boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::tail<boost::accumulators::right>, boost::accumulators::tag::variance(boost::accumulators::lazy) > > StatisticsAccuWithMeanVarAndTail;
//    typedef StatisticsAccuWithMeanVarAndTail StatisticsContainer;
    typedef boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::median, boost::accumulators::tag::variance(boost::accumulators::lazy) > > StatisticsAccuWithMeanMedianAndVar;
    typedef StatisticsAccuWithMeanMedianAndVar StatisticsContainer;

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

    static TMatrixD invertMatrix(const TMatrixD& aMatrix) ;
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
    void registerTVector3(const std::string& nameOfDataSample); // to create a TVector3 branch in the costum root tree in this module's output
    void registerInt(const std::string& nameOfDataSample);

    void fillTrackWiseData(const std::string& nameOfDataSample, const double newData);
    void fillTrackWiseVecData(const std::string& nameOfDataSample, const std::vector<double>& newData);
    void fillLayerWiseData(const std::string& nameOfDataSample, const int accuVecIndex, const std::vector<double>& newData);
    void fillTVector3(const std::string& nameOfDataSample, const TVector3& newData);
    void fillInt(const std::string& nameOfDataSample, const int newData);

    void printTrackWiseStatistics(const std::string& nameOfDataSample, const bool count = false);
    void printTrackWiseVecStatistics(const std::string& nameOfDataSample, const std::vector<std::string>& trackWiseVarNames, const  bool count = false);
    void printLayerWiseStatistics(const std::string& nameOfDataSample,  const std::vector<std::string>& layerWiseVarNames, int madVars, const bool count = true);
    void printLRResData(const std::string& nameOfDataSample, const std::vector<std::string>& layerWiseVarNames);

    int m_nSiLayers; // number of Si layers. That is 6 of course.
    int m_nPxdLayers; // number of PXD layer (2) so number of SVD layers will be m_nSiLayers - m_nPxdLayers
    int m_nSvdLayers;
    int m_nCdcLayers;
    int m_nLayers;

    std::vector<std::string> m_layerWiseTruthTestsVarNames;
    std::vector<std::string> m_vertexTestsVarNames;


    // the following maps should not be accessed directly but only with the corresponding "register" "fill" and "print" functions
    //the following maps will be filled with the test data so that statistical quantities like mean and variance can be calculated
    std::map<std::string, StatisticsContainer > m_trackWiseDataSamples;
    std::map<std::string, std::vector<StatisticsContainer> > m_trackWiseVecDataSamples;
    std::map<std::string, std::vector<std::vector<StatisticsContainer> > > m_layerWiseDataSamples;

    //the following maps will be filled with the test data so they can be written into a root file for further analysis with root
    std::map<std::string, float > m_trackWiseDataForRoot;
    std::map<std::string, std::vector<float>* > m_trackWiseVecDataForRoot;
    std::map<std::string, std::vector< std::vector <float> >* > m_layerWiseDataForRoot;
    std::map<std::string, TVector3* > m_TVector3ForRoot; //this one is to store the mcparticle truth info for position and momentum
    std::map<std::string, int > m_intForRoot; //this one is to store some auxiliary (like if track was fitted successful) info about the tracks into the root file

//    // now the data itself also in c++ vectors so this module can use custom implemented estimators instead just the one provided by root and
    bool m_robust;
    std::map<std::string, std::vector<double > > m_trackWiseData;
    std::map<std::string, std::vector<std::vector<double> > > m_trackWiseVecData;
    std::map<std::string, std::vector<std::vector< std::vector <double> > > > m_layerWiseData;


    /** function to calculated the MAD or Median absolute deviation for given data sample. One has also provide the median of the sample (because boost has already median calculation implemented so I did not implement it myself) */
    double calcMad(const std::vector<double>& data, const double& median);
    std::map<std::string, double > m_madScalingFactors; //scaling factor the mad to make it compariable to the standard deviation
    double calcMedian(std::vector<double> data);
    //void calcMedianAndMad(std::vector<double> data, double& median, double& mad);
    int trunctatedMeanAndStd(std::vector<double> data, const double cutRatio, const bool symmetric, double& mean, double& std);
    std::map<std::string, double > m_trunctationRatios; //holds the ratio how many of the data of a named sample should be cut away in the trunctatedMeanAndStd function
    int countOutliers(const std::vector<double>& dataSample, const double mean, const double sigma, const double widthScaling);

    // this maps will hold the names of the test data variables that have more then one component like the residuals of the origin position and momentum
    std::map<std::string, std::vector<std::string>* > namesOfTestVars;

    int m_processedTracks;
    double m_pvalueCut;
    bool m_fillOnlyInRootFile;
    double m_trunctationRatio;
    int m_nCutawayTracks;
    int m_extrapFailed; //counter for the number of tracks that cannot be extrapolated to thier true vertex position by Genfit
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
    bool m_testLRRes;
    int m_inspectTracks;
    std::string m_dataOutFileName; //common part of all names of output files

    //stuff for text file output
    std::stringstream m_textOutput;
    std::ofstream m_dataOut;



    bool m_wAndPredPresentsTested;
    //stuff for the text output for rave developers
    bool m_exportTracksForRaveDeveloper;
    std::ofstream m_forRaveOut;

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

    void extractTrackData(GFTrack* const aTrackPtr, const double charge);

    void testDaf(GFTrack* const aTrackPtr);
    //void testDafWithBG(GFTrack *const aTrackPtr);
    //void fillDafWithBGData(const std::string& nameOfDataSample, const std::vector<std::vector<float> >& allWeights);

    void testLRAmbiResolution(GFTrack* const aTrackPtr);

    void normalTests();

    void truthTests();

    void inspectTracks(double chi2tot_fu, double vertexAbsMom);

  };
}

#endif /* TrackFitCheckerModule_H_ */
