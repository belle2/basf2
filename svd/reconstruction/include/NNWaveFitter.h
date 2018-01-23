/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef _SVD_RECONSTRUCTION_NNwAVEFITTER_H
#define _SVD_RECONSTRUCTION_NNWAVEFITTER_H

#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <svd/reconstruction/NNWaveFitTool.h>
#include <svd/simulation/SVDSimulationTools.h>
#include <framework/gearbox/Unit.h>
#include <Eigen/Dense>

namespace Belle2 {
  namespace SVD {

// ==============================================================================
// The NNWaveFitter class - neural network fitter of APV25 waveforms
// ------------------------------------------------------------------------------
    /*
    * The class uses a neural network to find a probability distribution
    * of arrival times for a sextet of APX25 signal samples.
    * The input to the network are sample S/N ratios, i.e., we expect
    * \sigma^2 = 1, plus width of the waveform from calibration data.
    * The network(s) is currently trained using Python's scikit-learn
    * package and results are saved in a PMML-like xml file.
    * The NNWaveFitter reads the contents of this xml and on request provides
    * the following data to the calling routine:
    * 1. Signal's arrival time estimate and its error,
    * 2. Signal's amplitude and its error,
    * 3. chi2 of the fit
    * 4. (Binned) probability distribution for arrival time.
    *
    * The network is a multiclass classifier and the time bin probabilities
    * are its main output. Time and amplitude are calculated from this
    * distribution. The probability distribution is useful by itself in
    * background suppression and u/v hit pairing.
    *
    * The input xml file has, with a few extensions,  the official PMML
    * format (http://dmg.org), which can be created and read by most
    * machine learning libraries.
    *
    * The fitter generates a NNWaveFit object to aid in computations with
    * bin probabilities, such as shifting, multiplication, calculations of
    * time shift, amplitude and their errors.
    */

    class NNWaveFitter {

    public:


      /** Bounds type used to hold network parameter bounds used in
       * training the network. Also, this is the range guaranteed
       * network applicability.
       */
      typedef std::pair<double, double> nnBoundsType;

      /** Cnnstructor
       * constructs the wavefitter from data in xml file.
       * @param xmlData string containing the network definition xml tree
       */
      NNWaveFitter(std::string xmlData = "");

      /**Set proper network definition file.
       * @param xmlData string containing the network definition xml tree
       */
      void setNetwrok(const std::string& xmlData);

      /** Fitting method
       * Send data and get rseult structure.
       * If the fitter is not properly initialized, empty structure with
       * valid = false will be returned, no warning - warning is only given
       * for unsuccessful initialization.
       * @param samples Array of 6 apv samples
       * @param tau The wave width for the current strip (unencoded ns)
       * @return Pointer to array of bin probabilities.
       */
      std::shared_ptr<nnFitterBinData> getFit(const apvSamples& samples, double tau);

      /** Get a handle to a NNWaveFit object.
       * The object provides a set of tools to maniulate probabilities and calculate things
       * from probability distributions.
       */
      const NNWaveFitTool& getFitTool() const { return *m_fitTool; }

      /** Get bin times of the network output
       * @return array of mean bin times (averaged over training data),
       * properly sorted.
       */
      const nnFitterBinData& getBinCenters() const { return m_binCenters; }

      /** Get bins of netwrok output
       * @return array of bin edges (length of timebase + 1)
       */
      const nnFitterBins& getBins() const { return m_bins; }

      /** Get width bounds
       * Width bounds are minimum and maximum width used in training the
       * network. The bounds are used to encode width on input to the
       * fitter.
       */
      nnBoundsType getWaveWidthBounds() const { return m_waveWidthBounds; }

      /** Get amplitude bounds
       * @return std::pair of amplitude bounds used in simulation
       */
      nnBoundsType getAmplitudeBounds() const { return m_amplitudeBounds; }

      /** Get time shift bounds
       * @return std::pair of time shift bounds used in simulation
       */
      nnBoundsType getTimeShiftBounds() const { return m_timeShiftBounds; }

      /** Is this fitter working?
       * Return false if the fitter was not initialized properly.
       */
      bool isValid() const { return m_isValid; }

      /** Check NN data against a dump from Python.
       * @param dumpname Filename of a text dump of network coefficients.
       * @param tol Tolerance for float comparisons
       * @return true if all comparisons pass.
       */
      bool checkCoefficients(const std::string& dumpname, double tol = 1.0e-10);

    private:

      /** Activation functions type */
      typedef std::function<double(double)> activationType;
      /** Rectifier activation */
      activationType relu =
        [](double x) -> double { return std::max(double(0.0), x); };
      /** Sigmoid activation */
      activationType sigmoid =
        [](double x) -> double { double e = std::exp(x); return e / (1.0 + e); };

      /** Softmax function, normalization for the network's output layer.
       * This is a strange softmax that maps zero values back to
       * zero, but this is what scikit-learn returns.
       */
      Eigen::VectorXd softmax(const Eigen::VectorXd& input)
      {
        auto output = input.array().unaryExpr(
                        [](double x)->double { return ((x > 0.0) ? exp(x) : 0.0); }
                      );
        double norm = output.sum();
        return output / norm;
      }

      /** The method that actually reads the xml file.
       * @param xmlFileName Name of the source xml file with network data
       * @return 0: OK, -1: something went wrong.
       */
      int readNetworkData(const std::string& xmlFileName);

      /** We use map to store network layers since we don't know if we'll
       * be reading them in correct order.
       */
      typedef std::map< size_t, std::pair< Eigen::MatrixXd, Eigen::VectorXd > >
      networkWeightsType;

      /** Storage for internal states. */
      typedef std::vector<Eigen::VectorXd> layerStatesType;

      // Data members
      bool m_isValid; /**< true if fitter was properly initialized */
      std::size_t m_nLayers; /**< number of NN layers, read from xml */
      nnFitterBinData m_binCenters;  /**< centers of NN time bins */
      nnFitterBins m_bins; /**< NN time bin boundaries */
      layerStatesType m_layerStates; /**< vectors of layer states */
      std::vector<std::size_t> m_layerSizes; /**< NN layer sizes */
      networkWeightsType m_networkCoefs; /**< NN weights and intercepts */
      activationType m_activation; /**< Network activation function */
      nnBoundsType m_amplitudeBounds; /**< Amplitude range of the network */
      nnBoundsType m_waveWidthBounds; /**< Waveform width range of the network */
      nnBoundsType m_timeShiftBounds; /**< Time shift range of the network */
      TauEncoder m_tauCoder; /**< Tau encoder class instance to scale tau values */
      waveFunction m_wave; /**< Wave function used in training the network. */
      std::shared_ptr<NNWaveFitTool> m_fitTool; /**< FitterTool object allowing calculations on network fits. */

    }; // class NNWaveFitter

  } // namespace SVD
} // namespace Belle2

#endif
