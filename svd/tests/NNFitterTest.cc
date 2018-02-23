/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dataobjects/VxdID.h>
#include <svd/simulation/SVDSimulationTools.h>
#include <svd/reconstruction/NNWaveFitter.h>
#include <svd/reconstruction/NNWaveFitTool.h>
#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <cmath>
#include <functional>

using namespace std;

namespace Belle2 {
  namespace SVD {

    /**
     * Compare NN fitter parameter dump from Python with internal data
     * representation in C++ to verify that the network coefficients were
     * transferred without a significant loss of accuracy.
     * FIXME (Oct 12,2017: This test fails on buildbot, but ran successfully
     * on all machines I tried.
     */
    TEST(NNTimeFitter, DISABLED_CompareNetworkCoefficient)
    {
      // Create an instance of the NN fitter
      NNWaveFitter fitter("svd/data/SVDTimeNet.xml");
      EXPECT_TRUE(fitter.checkCoefficients("svd/data/classifier.txt", 1.0e-6));
    }

    /**
     * Read a sample of fits from the Python NN suite and check that the C++ NN fitter produces
     * the same results.
     * This is the result of select_t0_fits.columns:
     * Index([ 'test', 'amplitude', 't0', 'tau', 'sigma', 's1', 's2', 's3', 's4', 's5', 's6',
     * 'normed_tau', 't0_bin', 'abin', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
     * 16, 17, 18, 19, 20, 21, 22, 23, 24, 'a_fit', 'a_sigma', 'chi2_ndf', 't_fit', 't_sigma',
     * 't_true', 'tau', 'a_true', 't_bin', 'a_bin', 't_diff','a_diff'],
     * dtype='object')
     * Columns labeled by numerals contain probabilities.
     */
    TEST(NNTimeFitter, DISABLED_CompareFits)
    {
      const size_t max_lines = 100; // maximum number of lines to be read

      // Create an instance of the NN fitter and the fitter tool.
      NNWaveFitter fitter("SVDTimeNet_6samples");
      auto fitTool = fitter.getFitTool();
      size_t nProbs = fitTool.getBinCenters().size();

      ifstream infile("svd/data/test_sample.csv");

      // Read the rows one by one and compare results
      string line;
      getline(infile, line);

      for (size_t i_line = 0; i_line < max_lines; i_line++) {

        getline(infile, line);
        if (line.size() < 10) break;
        istringstream sline(line);

        // Parse header. We want the dimennsion of the probability array.
        // not needed
        string cell;
        getline(sline, cell, ','); // index
        getline(sline, cell, ','); // test

// Declare variables as unused and suppress compiler warnings.
#define UNUSED(x) [&x]{}()

        // true values
        getline(sline, cell, ',');
        double true_amp = stod(cell);
        UNUSED(true_amp); // Read from the fule, though not used.
        getline(sline, cell, ',');
        double true_t0 = stod(cell);
        UNUSED(true_t0);
        getline(sline, cell, ',');
        double width = stod(cell);
        getline(sline, cell, ',');
        double noise = stod(cell);
        UNUSED(noise);

        // normalized samples
        apvSamples normedSamples;
        for (size_t iSample = 0; iSample < nAPVSamples; ++iSample) {
          getline(sline, cell, ',');
          normedSamples[iSample] = stod(cell);
        }

        // not needed
        getline(sline, cell, ',');
        getline(sline, cell, ',');
        getline(sline, cell, ',');

        // probabilities
        nnFitterBinData ProbsPy(nProbs);
        for (size_t iSample = 0; iSample < nProbs; ++iSample) {
          getline(sline, cell, ',');
          ProbsPy[iSample] = stod(cell);
        }

        // fit results
        getline(sline, cell, ',');
        double fitPy_amp = stod(cell);
        getline(sline, cell, ',');
        double fitPy_ampSigma = stod(cell);
        getline(sline, cell, ',');
        double fitPy_chi2 = stod(cell);
        UNUSED(fitPy_chi2); // Read from file but not used.
        getline(sline, cell, ',');
        double fitPy_t0 = stod(cell);
        getline(sline, cell, ',');
        double fitPy_t0Sigma = stod(cell);

#undef UNUSED

        // now do the Cpp fit
        const shared_ptr<nnFitterBinData> ProbsCpp = fitter.getFit(normedSamples, width);
        for (size_t iBin = 0; iBin < nProbs; ++iBin)
          EXPECT_NEAR((*ProbsCpp)[iBin], ProbsPy[iBin], 5.0e-3);

        double t0_cpp, t0_err_cpp;
        tie(t0_cpp, t0_err_cpp) = fitTool.getTimeShift(*ProbsCpp);
        EXPECT_NEAR(t0_cpp, fitPy_t0, 5);
        EXPECT_NEAR(t0_err_cpp, fitPy_t0Sigma, 2);

        double amp_cpp, amp_err_cpp, chi2_cpp;
        tie(amp_cpp, amp_err_cpp, chi2_cpp) = fitTool.getAmplitudeChi2(normedSamples, t0_cpp, width);
        EXPECT_NEAR(amp_cpp, fitPy_amp, 1.0);
        EXPECT_NEAR(amp_err_cpp, fitPy_ampSigma, 0.1);
        // FIXME: This is calculated slightly differently in Python, and it shows.
        // EXPECT_NEAR(chi2_cpp, fitPy_chi2, 1);
      }
    }

  } // namespace SVD
}  // namespace Belle2
