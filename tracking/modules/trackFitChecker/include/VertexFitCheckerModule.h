/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Moritz Nadler                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VERTEXFITCHECKER_H
#define VERTEXFITCHECKER_H

#include <framework/core/Module.h>
#include <string>
#include <fstream>
#include <sstream>
//root I/O
#include <TTree.h>
#include <TFile.h>

//root
#include <TMatrixT.h>

// to get statistics functions of boost
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/count.hpp>
//#include <boost/accumulators/statistics/tail.hpp>
#include <boost/accumulators/statistics/median.hpp>

namespace Belle2 {

  /** This module checks the correctness of the fitted vertices produced by the Vertexer module (uses GFRave) by conducting several statistical tests on them
  */

  class VertexFitCheckerModule : public Module {

    typedef boost::accumulators::accumulator_set < double, boost::accumulators::stats < boost::accumulators::tag::mean, boost::accumulators::tag::median, boost::accumulators::tag::variance(boost::accumulators::lazy) > > StatisticsAccuWithMeanMedianAndVar;
    typedef StatisticsAccuWithMeanMedianAndVar StatisticsContainer;

  public:
    VertexFitCheckerModule();
    ~VertexFitCheckerModule() {}

    void initialize();
    void beginRun();
    void event();
    void endRun();
    void terminate();

  protected:



    void registerVertexWiseData(const std::string& nameOfDataSample);
    void fillVertexWiseData(const std::string& nameOfDataSample, const double newData);
    void printVertexWiseStatistics(const std::string& nameOfDataSample, const bool count = false);

    void registerVertexWiseVecData(const std::string& nameOfDataSample, const int nVarsToTest);
    void fillVertexWiseVecData(const std::string& nameOfDataSample, const std::vector<double>& newData);
    void printVertexWiseVecStatistics(const std::string& nameOfDataSample, const std::vector<std::string>& trackWiseVarNames, const  bool count = false);

    void registerInt(const std::string& nameOfDataSample);
    void fillInt(const std::string& nameOfDataSample, const int newData);
    double calcChi2(const TMatrixT<double>& res, const TMatrixT<double>& R) const;

    std::map<std::string, StatisticsContainer > m_vertexWiseDataSamples;
    std::map<std::string, std::vector<StatisticsContainer> > m_vertexWiseVecDataSamples;

    std::map<std::string, std::vector<float>* > m_vertexWiseVecDataForRoot;
    std::map<std::string, float > m_vertexWiseDataForRoot;
    std::map<std::string, int > m_intForRoot; //this one is to store some auxiliary (like if vertex was fitted successful) info about the tracks into the root file


    bool m_robust;
    std::map<std::string, std::vector<double > > m_vertexWiseData;
    std::map<std::string, std::vector<std::vector<double> > > m_vertexWiseVecData;
    std::map<std::string, double > m_madScalingFactors; //scaling factor the mad to make it compariable to the standard deviation
    double calcMad(const std::vector<double>& data, const double& median);
    int countOutliers(const std::vector<double>& dataSample, const double mean, const double sigma, const double widthScaling);
    //double calcMedian(std::vector<double> data);

    std::string m_dataOutFileName; //common part of all names of output files

    std::stringstream m_textOutput;

    //stuff for root output
    bool m_writeToFile;
    bool m_writeToRootFile;

    TTree* m_statDataTreePtr;
    TFile* m_rootFilePtr;
    int m_processedVertices;
    int m_badVertexPValueVertices;
    int m_badTrackPValueVertices;
    int m_vertexNotPureCounter;

    double  m_trackPValueCut;
    double  m_vertexPValueCut;


  };
}

#endif
