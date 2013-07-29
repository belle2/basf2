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
//#include <boost/accumulators/statistics/tail.hpp>
#include <boost/accumulators/statistics/median.hpp>
//genfit stuff
#include <GFTrack.h>

//stuff for root output
#include <TTree.h>
#include <TFile.h>



namespace Belle2 {

  /*!
      This module performs a wide variety of statistical tests on the GFTrack objects created by Genfit via the GenFitter module. It is intended for tracking developers.
  */

  class TrackFitCheckerModule : public Module {


//    typedef boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::variance(boost::accumulators::lazy) > > StatisticsAccuWithMeanAndVar;
//    typedef StatisticsAccuWithMeanAndVar StatisticsContainer;
//    typedef boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::tail<boost::accumulators::right>, boost::accumulators::tag::variance(boost::accumulators::lazy) > > StatisticsAccuWithMeanVarAndTail;
//    typedef StatisticsAccuWithMeanVarAndTail StatisticsContainer;
    typedef boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::median, boost::accumulators::tag::variance(boost::accumulators::lazy) > > StatisticsAccuWithMeanMedianAndVar; /**< is a typedef for defining a Container containing information which will be stored in a root output file for statistical interpretation - long version */
    typedef StatisticsAccuWithMeanMedianAndVar StatisticsContainer; /**< is a typedef for defining a Container containing information which will be stored in a root output file for statistical interpretation - short version */

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
    /** Returns the inverted matrix of aMatrix */
    static TMatrixD invertMatrix(const TMatrixD& aMatrix) ;
  protected:

    // little helper functions for this module
    /** Calculates the χ² value from a residum (res) and the covariance matrix of this residuum (R) with χ² = res^T*R^{-1}*res */
    static double calcChi2(const TMatrixT<double>& res, const TMatrixT<double>& R);
    /** Calculates the pull values from a residuum (res) and the covariance matrix of this residuum (R) with pull_i = res_i / sqrt(R(i,i)) */
    static std::vector<double> calcPulls(const TMatrixT<double>& res, const TMatrixT<double>& R);
    /** Calculates different pull and  χ² values from a estimated track state vector defined at some point (normally a detector hit), its covariance matrix and the true state at that point */
    static std::vector<double> calcTestsWithTruthInfo(const TMatrixT<double>& state, const TMatrixT<double>& cov, const TMatrixT<double>& trueState);
//    void isMatrixCov(const TMatrixT<double>& cov);
//    static bool isSymmetric(const TMatrixT<double>& aMatrix);
    /** Simply tests if any diagonal element of aMatrix is negative*/
    static bool hasMatrixNegDiagElement(const TMatrixT<double>& aMatrix);

    // functions for dataflow inside module, registerX fillX and printX all work on the same maps whose entries are accessed by the nameOfDataSample-string which acts as a key
    void registerTrackWiseData(const std::string& nameOfDataSample); /**< used for dataflow inside module, registerX fillX and printX all work on the same maps whose entries are accessed by the nameOfDataSample-string which acts as a key, registerTrackWiseData registers new data to be stored as a single value once for each track */
    void registerTrackWiseVecData(const std::string& nameOfDataSample, const int nVarsToTest); /**< used for dataflow inside module, registerX fillX and printX all work on the same maps whose entries are accessed by the nameOfDataSample-string which acts as a key, registerTrackWiseVecData registers new data to be stored as a vector once for each track */
    void registerLayerWiseData(const std::string& nameOfDataSample, const int nVarsToTest); /**< used for dataflow inside module, registerX fillX and printX all work on the same maps whose entries are accessed by the nameOfDataSample-string which acts as a key, registerLayerWiseData registers new data to be stored as a value once for each layer */
    void registerTVector3(const std::string& nameOfDataSample); /**< to create a TVector3 branch in the custom root tree in this module's output */
    void registerInt(const std::string& nameOfDataSample); /**< used for dataflow inside module, registerX fillX and printX all work on the same maps whose entries are accessed by the nameOfDataSample-string which acts as a key, registerInt registers new data to be stored as an int */

    void fillTrackWiseData(const std::string& nameOfDataSample, const double newData); /**< used for dataflow inside module, registerX fillX and printX all work on the same maps whose entries are accessed by the nameOfDataSample-string which acts as a key, fillTrackWiseData stores new data as a single value once for each track */
    void fillTrackWiseVecData(const std::string& nameOfDataSample, const std::vector<double>& newData); /**< used for dataflow inside module, registerX fillX and printX all work on the same maps whose entries are accessed by the nameOfDataSample-string which acts as a key, fillTrackWiseVecData stores new data as a single value once for each track */
    void fillLayerWiseData(const std::string& nameOfDataSample, const int accuVecIndex, const std::vector<double>& newData); /**< used for dataflow inside module, registerX fillX and printX all work on the same maps whose entries are accessed by the nameOfDataSample-string which acts as a key, fillLayerWiseData new data as a single value once for each track */
    void fillTVector3(const std::string& nameOfDataSample, const TVector3& newData); /**< stores a TVector3 into a TVector3 branch in the custom root tree in this module's output */
    void fillInt(const std::string& nameOfDataSample, const int newData); /**< used for dataflow inside module, registerX fillX and printX all work on the same maps whose entries are accessed by the nameOfDataSample-string which acts as a key, fillInt stores new data as an int  */

    // for text- or console-output, nameOfDataSample is fileName,therefore only internally set within code
    void printTrackWiseStatistics(const std::string& nameOfDataSample, const bool count = false); /**< for text- or console-output of track wise data, nameOfDataSample is fileName,therefore only internally set within code */
    void printTrackWiseVecStatistics(const std::string& nameOfDataSample, const std::vector<std::string>& trackWiseVarNames, const  bool count = false); /**< or text- or console-output of trackwise data which is stored as a vector, nameOfDataSample is fileName,therefore only internally set within code */
    void printLayerWiseStatistics(const std::string& nameOfDataSample,  const std::vector<std::string>& layerWiseVarNames, int madVars, const bool count = true); /**< or text- or console-output for layer wise data, nameOfDataSample is fileName,therefore only internally set within code */
    void printLRResData(const std::string& nameOfDataSample, const std::vector<std::string>& layerWiseVarNames); /**< print the data of left-right-residuals? */

    //When no layer wise tests are done, the layer numbers are set to 0 in this module.
    int m_nSiLayers; /**< number of Si layers.  m_nSiLayers = m_nPxdLayers + m_nSvdLayers */
    int m_nPxdLayers; /**< number of PXD layer, 2 normally */
    int m_nSvdLayers; /**< number of SVD layer, 4 normally */
    int m_nCdcLayers; /**< number of CDC layer, 56 normally */
    int m_nLayers; /**< m_nLayers = m_nPxdLayers + m_nSvdLayers + m_nCdcLayers */

    std::vector<std::string> m_layerWiseTruthTestsVarNames; /**< holds the names of the rows for the text file output table produced by the tests made by truthTests()*/
    std::vector<std::string> m_vertexTestsVarNames; /**< holds the names of the rows for the text file output table produced by the track wise (vec) tests using the residuals at the true vertex position*/


    // the following maps should not be accessed directly but only with the corresponding "register" "fill" and "print" functions
    //the following maps will be filled with the test data so that statistical quantities like mean and variance can be calculated
    std::map<std::string, StatisticsContainer > m_trackWiseDataSamples; /**< */
    std::map<std::string, std::vector<StatisticsContainer> > m_trackWiseVecDataSamples; /**< */
    std::map<std::string, std::vector<std::vector<StatisticsContainer> > > m_layerWiseDataSamples; /**< */

    //the following maps will be filled with the test data so they can be written into a root file for further analysis with root
    std::map<std::string, float > m_trackWiseDataForRoot; /**< the map will be filled with the track wise test data so they can be written into a root file for further analysis with root*/
    std::map<std::string, std::vector<float>* > m_trackWiseVecDataForRoot; /**< the map will be filled with the track wise vector test data so they can be written into a root file for further analysis with root */
    std::map<std::string, std::vector< std::vector <float> >* > m_layerWiseDataForRoot; /**< the map will be filled with the layer wise test data so they can be written into a root file for further analysis with root */
    std::map<std::string, TVector3* > m_TVector3ForRoot; /**< this one is to store the mcparticle truth info for position and momentum*/
    std::map<std::string, int > m_intForRoot; /**< this one is to store some auxiliary (like if track was fitted successful) info about the tracks into the root file */

    // now the data itself also in c++ vectors so this module can use custom implemented estimators instead just the one provided by root and
    bool m_robust; /**< if this flag is true the robust tests (= median and MAD and truncated mean and std) will be executed */
    std::map<std::string, std::vector<double > > m_trackWiseData; /**< now the track wise data itself also in c++ vectors so this module can use custom implemented estimators instead just the one provided by root and */
    std::map<std::string, std::vector<std::vector<double> > > m_trackWiseVecData; /**< now the track wise vector data itself also in c++ vectors so this module can use custom implemented estimators instead just the one provided by root and */
    std::map<std::string, std::vector<std::vector< std::vector <double> > > > m_layerWiseData; /**< now the layer wise data itself also in c++ vectors so this module can use custom implemented estimators instead just the one provided by root and*/


    /** function to calculated the MAD or "median absolute deviation" for given data sample (data). One also has to provide the median of the sample */
    static double calcMad(const std::vector<double>& data, const double& median);
    std::map<std::string, double > m_madScalingFactors; /**< scaling factors for the MAD to make it comparable to the standard deviation. If a key does not exist, no MAD will be calculated for the data sample using that key*/

    /** returns the median of the data sample "data" */
    static double calcMedian(std::vector<double> data);
    //void calcMedianAndMad(std::vector<double> data, double& median, double& mad);
    /** calculates the truncated mean and standard deviation of the data sample "data". cutRatio is the truncation ratio. If symmetric is true the smallest and largest values will be ignored. If false only the largest values. The return value is the number of data points that were truncated */
    static int truncatedMeanAndStd(std::vector<double> data, const double cutRatio, const bool symmetric, double& mean, double& std);
    std::map<std::string, double > m_truncationRatios; /**<holds the ratio how many of the data of a named sample (name = key) should be cut away in the trunctatedMeanAndStd function. If a key does not exist, trunctatedMeanAndStd will not be calculated for the data sample using that key */
    /** returns the number of outliers in dataSample. An outlier is defined as being widthScaling*sigma away from mean */
    static int countOutliers(const std::vector<double>& dataSample, const double mean, const double sigma, const double widthScaling);

    // this maps will hold the names of the test data variables that have more then one component like the residuals of the origin position and momentum
    //std::map<std::string, std::vector<std::string>* > namesOfTestVars;

    int m_processedTracks; /**< holds number of tracks used in the tests */
    double m_pvalueCut; /**< tracks with p-value smaller than m_pvalueCut will not be used in the tests */
    bool m_fillOnlyInRootFile; /**< this flag is set to true for a track with p-value lower than m_pvalueCut. While this track will be ignored in the in the test resutls printed by the print functions it will sill contribute to the data written to the root file */
    double m_truncationRatio; /**< the truncation ratio passed to truncatedMeanAndStd in the print functions */
    int m_nCutawayTracks; /**< holds the number of tracks with p-value larger than m_pvalueCut */
    int m_extrapFailed; /**< holds the number of tracks that cannot be extrapolated to their true vertex position by Genfit */
    // counters holding the number of the covariance matrices (R) of the resuduals that have negative diagonal elements
    int m_badR_fCounter; /**< holds the number of times the covariance matrix of the forward updated residual had negative diagonal elements */
    int m_badR_bCounter; /**< holds the number of times the covariance matrix of the backward updated residual had negative diagonal elements */
    int m_badR_smCounter; /**< holds the number of times the covariance matrix of the smoothed residual had negative diagonal elements */
    bool m_testSi; /**< flag determines if the layer wise tests for PXD and SVD hits should be executed */
    bool m_testCdc; /**< flag determines if the layer wise tests for CDC hits should be executed */
//    int m_unSymmetricCounter;
//    int m_notPosDefCounter;
//    int m_nDigits;
    // module input parameters
    //bool m_useTruthInfo;
    bool m_testPrediction; /**< flag determines if the layer wise tests will also the predicted states in addition to the updated states are tested */
//    bool m_testDaf;
    bool m_truthAvailable; /**< flag determines if the layer wise tests will also the truth info for every hit (from the TrueHit objects) */
    bool m_testLRRes; /**< flag determines if the resolution of the left right ambiguity of CDC hits is tested */
//    int m_inspectTracks;
    std::string m_dataOutFileName; /**< this string will be added as a prefix to all output files of this module */

    //stuff for text file output
    std::stringstream m_textOutput; /**< this object collects all results of the statistical tests so they can be written to the terminal and/or to a text file via the m_dataOut object */
    std::ofstream m_dataOut; /**< file stream object to write the contend of m_textOutput into a text file */

//    bool m_spMode;
//    std::ofstream m_piStuffOut;

    bool m_wAndPredPresentsTested; /**< this flag is set true after it was checked if predicted states and/or DAF weights are present */

    //stuff for the text output for rave developers
    bool m_exportTracksForRaveDeveloper; /**< if true a text file is created with track parameters. This file is intended as input for Wolfgang Waltenberger's stand alone Rave version to debug vertexing. */
    std::ofstream m_forRaveOut; /**< the output file stream to create a text file with with track parameters intended as input for Wolfgang Waltenberger's stand alone Rave version to debug vertexing */

    bool m_writeToFile; /**< If true a text file with the results of all statistical tests will be created */
    //stuff for root output
    bool m_writeToRootFile; /**< If true a root file where all the data extracted form the tested tracks will be saved */
    TTree* m_statDataTreePtr; /**< TTree for this module's own ROOT output */
    TFile* m_rootFilePtr; /**< TFile for this module's own ROOT output */

    /** struct to store data needed by normalTests() and truthTests() for one track. */
    struct TrackData {

      int nHits; /**<number of hits in GFTrack object */

      //stuff for the normal layer wise tests:

      std::vector<int> accuVecIndices; /**< layer indices of hits shifted to start at 0 no matter what detectors are active (any combination of PXD SVD CDC) */
      std::vector<int> detIds; /**< detector ids of hits */

      std::vector<TMatrixD> ms; /**< Measurements from RecoHits */
      std::vector<TMatrixD> Hs; /**< H matrix from RecoHitts (H maps the 5D state onto the measurement */
      std::vector<TMatrixD> Vs; /**< covariance matrices of measurement errors from RecoHits */
      std::vector<TMatrixD> states_fu; /**< 5D forward updated state at hit positions */
      std::vector<TMatrixD> covs_fu; /**< 5x5 forward updated covariance matrices at hit positions */
      std::vector<TMatrixD> states_bu; /**< 5D backward updated state at hit positions */
      std::vector<TMatrixD> covs_bu; /**< 5x5 backward updated covariance matrices at hit positions */
      std::vector<TMatrixD> states_sm; /**< 5D smoothed state at hit positions */
      std::vector<TMatrixD> covs_sm; /**< 5x5 smoothed covariance matrices at hit positions */

      //aditional stuff for the tests that need truth info
      std::vector<TMatrixD> states_t; /**< 5D true state at hit positions. Extracted form TrueHit. Only needed by truthTests not by normalTests*/

      //aditional stuff for the tests that need predicted state
      std::vector<TMatrixD> states_fp; /**< 5D forward predicted state at hit positions */
      std::vector<TMatrixD> covs_fp; /**< 5x5 forward predicted covariance matrices at hit positions */
      std::vector<TMatrixD> states_bp; /**< 5D backward predicted state at hit positions */
      std::vector<TMatrixD> covs_bp; /**< 5x5 backward predicted covariance matrices at hit positions */

    };
    TrackData m_trackData; /**< holds data needed by normalTests() and truthTests() for one track. */
    /** Reads all data from aTrackPtr that are needed for the layer wise tests. The extracted data of one track will be written into m_trackData*/
    void extractTrackData(GFTrack* const aTrackPtr, const double charge);

//    void testDaf(GFTrack* const aTrackPtr); // implementation of testDaf currently broken
    /** Checks if the left/right ambiguity of all CDC hits in aTrackPtr was resolved correct or not during the track fit */
    void testLRAmbiResolution(GFTrack* const aTrackPtr);
    /** Calculates pulls and χ² of the residuals res = m -H*state for all hits. So it does _not_ need the true track state (TrueHits). Reads in the data from m_trackData that was prepared by extractTrackData*/
    void normalTests();
    /** Calculates pulls and χ² of the residuals res = state - state_true for all hits. So it needs the true track state (TrueHits). Reads in the data from m_trackData that was prepared by extractTrackData*/
    void truthTests();

    //void inspectTracks(double chi2tot_fu, double vertexAbsMom);

  };


}

#endif /* TrackFitCheckerModule_H_ */
