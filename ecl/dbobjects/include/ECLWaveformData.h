/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <algorithm>
#include <array>
#include <cassert>
#include <utility>
#include <vector>

namespace Belle2 {

  /** ECLWaveformData - container for inverse covariant matrix and shape parameters for time and amplitude fit of ecl crystal waveform
   *
   * Objects of this class keep the independent matrix elements
   * of the inverse covariant matrix to perform waveform fit and
   * the parameters defining the waveform shape.
   */
  class ECLWaveformData : public TObject {
  public:
    // Default sizes for the current ECL electronics:
    // 16x16 sampled signal matrix -> 136 independent elements,
    // and 10 parameters defining the waveform shape.
    static constexpr size_t c_nADCsignal = 16; /**< number of ADC samples in the signal fit */
    static constexpr size_t c_nElements = c_nADCsignal * (c_nADCsignal + 1) / 2;  /**< number of independent matrix elements */
    static constexpr size_t c_nParams = 10; /**< number of parameters defining the waveform shape */

    /** Constructor.
     * @param nADCsignal number of ADC samples used in the waveform fit
     * @param nParams number of waveform-shape parameters
     */
    explicit ECLWaveformData(size_t nADCsignal = c_nADCsignal, size_t nParams = c_nParams):
      m_nADCsignal(nADCsignal),
      m_nParams(nParams),
      m_matrixElement(nADCsignal * (nADCsignal + 1) / 2),
      m_waveformPar(nParams)
    {}

    /** Setter method for independent matrix element */
    void setMatrixElement(size_t i, float value)
    {
      assert(i < m_matrixElement.size());
      m_matrixElement[i] = value;
    }

    /** Getter method for independent matrix element */
    float getMatrixElement(size_t i) const
    {
      assert(i < m_matrixElement.size());
      return m_matrixElement[i];
    }

    /** Getter method for independent matrix element */
    float getMatrixElement(size_t i, size_t j) const
    {
      assert(i < m_nADCsignal);
      assert(j < m_nADCsignal);
      if (i < j)
        std::swap(i, j);
      return m_matrixElement[i * (i + 1) / 2 + j];
    }

    /** Getter method for all matrix as one dimensional array */
    void getArray(float* WF) const
    {
      std::copy(m_matrixElement.begin(),
                m_matrixElement.end(),
                WF);
    }

    /** Getter method for all matrix as two dimensional array (floats) */
    template <size_t N>
    void getMatrix(float (&M)[N][N]) const
    {
      assert(N == m_nADCsignal);
      auto it = m_matrixElement.begin();
      for (size_t i = 0; i < m_nADCsignal; ++i) {
        for (size_t j = 0; j < i; ++j)
          M[i][j] = M[j][i] = *it++;
        M[i][i] = *it++;
      }
    }

    /** Getter method for all matrix as two dimensional array (doubles) */
    template <size_t N>
    void getMatrix(double (&M)[N][N]) const
    {
      assert(N == m_nADCsignal);
      auto it = m_matrixElement.begin();
      for (size_t i = 0; i < m_nADCsignal; ++i) {
        for (size_t j = 0; j < i; ++j)
          M[i][j] = M[j][i] = *it++;
        M[i][i] = *it++;
      }
    }

    /** Setter for matrix from std::array */
    template <typename T, size_t N>
    void storeMatrix(const std::array<std::array<T, N>, N>& M)
    {
      assert(N == m_nADCsignal);
      auto it = m_matrixElement.begin();
      for (size_t i = 0; i < m_nADCsignal; ++i) {
        for (size_t j = 0; j < i; ++j)
          *it++ = static_cast<float>(M[i][j]);
        *it++ = static_cast<float>(M[i][i]);
      }
    }

    /** Setter method for waveform shape parameter */
    void setWaveformPar(size_t i, float value)
    {
      assert(i < m_waveformPar.size());
      m_waveformPar[i] = value;
    }

    /** Getter method for waveform shape parameter */
    float getWaveformPar(size_t i) const
    {
      assert(i < m_waveformPar.size());
      return m_waveformPar[i];
    }

    /** Getter method for waveform shape parameters as one dimensional array of
     * floats */
    void getWaveformParArray(float* P) const
    {
      std::copy(m_waveformPar.begin(),
                m_waveformPar.end(),
                P);
    }

    /** Getter method for waveform shape parameters as one dimensional array of
     * doubles */
    void getWaveformParArray(double* P) const
    {
      std::copy(m_waveformPar.begin(),
                m_waveformPar.end(),
                P);
    }

    /** Return the number of ADC samples used in the waveform fit. */
    size_t getNADCsignal() const { return m_nADCsignal; }

    /** Return the number of waveform-shape parameters. */
    size_t getNParams() const { return m_nParams; }

    /** Return the number of stored independent matrix elements. */
    size_t getNElements() const { return m_matrixElement.size(); }

    /** print-out function for debugging purpose*/
    void print() const;

  private:
    UInt_t m_nADCsignal{c_nADCsignal};  /**< Number of ADC samples used in the waveform fit. */
    UInt_t m_nParams{c_nParams};  /**< Number of waveform-shape parameters. */

    std::vector<float> m_matrixElement; /**< the matrix elements */
    std::vector<float> m_waveformPar; /**< the waveform parameters */

    ClassDef(ECLWaveformData, 2); /**< ClassDef */
  };

  /** Container for constant parameters used in waveform fits */

  class ECLWFAlgoParams : public TObject {
  public:

    Int_t ka;               /**< multipliers power of 2 for fg31,fg41 */
    Int_t kb;               /**< multipliers power of 2 for fg32 */
    Int_t kc;               /**< multipliers power of 2 for fg33,fg43 */
    Int_t y0Startr;         /**< start point for pedestal calculation */
    Int_t chiThresh;        /**< chi2 threshold for quality bit */
    Int_t k1Chi;            /**< multipliers power of 2 for f */
    Int_t k2Chi;            /**< multipliers power of 2 for chi2 calculation */
    Int_t hitThresh;        /**< hardware threshold(to start digitization) */
    Int_t lowAmpThresh;     /**< threshold to calculate time */
    Int_t skipThresh;       /**< threshold to send data to collector */
    int getka() const {return (int) ka;}            /**< getter for multipliers power of 2 for fg31 fg41 */
    int getkb() const {return (int) kb;}            /**< getter for multipliers power of 2 for fg32 */
    int getkc() const {return (int) kc;}            /**< getter for multipliers power of 2 for fg33,fg43 */
    int gety0s()  const {return (int) y0Startr;}    /**< getter for the start point for pedestal calculation */
    int getcT() const {return (int) chiThresh;}     /**< getter for the chi2 threshold for quality bit */
    int getk1() const {return (int) k1Chi;}         /**< getter for the multipliers power of 2 for f */
    int getk2() const {return (int) k2Chi;}         /**< getter for multipliers power of 2 for chi2 calculation */
    int gethT() const {return (int) hitThresh;}     /**< getter for the hardware threshold */
    int getlAT() const {return (int) lowAmpThresh;} /**< getter for the threshold to calculate time */
    int getsT()  const {return (int) skipThresh;}   /**< getter for the threshold to send data to collector */

    ClassDef(ECLWFAlgoParams, 1); /**< ClassDef */
  };

  /** Container for constant matrix used to generate electronic noise */
  class ECLNoiseData : public TObject {
  public:
    // Noise is defined for full ADC window (fixed at 31 samples for current ECL electronics)
    static constexpr size_t c_nADC = 31; /**< number of ADC samples */
    static constexpr size_t c_nElements =  c_nADC * (c_nADC + 1) / 2; /**< number of independent elements */

    /** Constructor.
     * @param nADC number of ADC samples
     */
    explicit ECLNoiseData(size_t nADC = c_nADC):
      m_nADC(nADC),
      m_matrixElement(nADC * (nADC + 1) / 2)
    {}

    /** Setter method for independent matrix element */
    void setMatrixElement(size_t i, float value)
    {
      assert(i < m_matrixElement.size());
      m_matrixElement[i] = value;
    }

    /** Getter method for independent matrix element */
    float getMatrixElement(size_t i) const
    {
      assert(i < m_matrixElement.size());
      return m_matrixElement[i];
    }

    /** Getter method for matrix as one dimensional array */
    void getArray(float* NoiseData) const
    {
      std::copy(m_matrixElement.begin(),
                m_matrixElement.end(),
                NoiseData);
    }

    /** Getter method for matrix as two dimensional array */
    template<size_t N>
    void getMatrix(float (&M)[N][N]) const
    {
      assert(N == m_nADC);
      auto it = m_matrixElement.begin();
      for (size_t i = 0; i < m_nADC; ++i) {
        for (size_t j = 0; j < m_nADC; ++j) {
          if (j > i) {
            M[i][j] = 0.f;
          } else {
            M[i][j] = *it++;
          }
        }
      }
    }

    /** sampling a random vector x from the N-dimensional multivariate normal distribution with covariance matrix C */
    void generateCorrelatedNoise(const float* z, float* x) const
    {
      auto it = m_matrixElement.begin();
      for (size_t i = 0; i < m_nADC; ++i) {
        float sum = 0.f;
        for (size_t j = 0; j <= i; ++j)
          sum += z[j] * (*it++);
        x[i] = sum;
      }
    }

    /** Return the number of ADC samples. */
    size_t getNADC() const { return m_nADC; }

    /** Return the number of stored independent matrix elements. */
    size_t getNElements() const { return m_matrixElement.size(); }

  private:
    UInt_t m_nADC{c_nADC};  /**< Number of ADC samples. */

    std::vector<float> m_matrixElement; /**< electronic noise matrix */

    ClassDef(ECLNoiseData, 2); /**< ClassDef */
  };


  /** Class for a lookup table */
  class ECLLookupTable : public TObject {
  public:
    /** Constructor */
    ECLLookupTable() : m_content(ECLElementNumbers::c_NCrystals, 0) {}

    /** Array-like access operator */
    unsigned int operator[](unsigned int key) const
    { return m_content[ key - 1 ]; }
    /** Array-like access operator */
    unsigned int& operator[](unsigned int key)
    { return m_content[ key - 1 ]; }

  private:
    std::vector<unsigned int> m_content; /**< index to index lookup table */
    ClassDef(ECLLookupTable, 1); /**< ClassDef */
  };

}
