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
  class ECLWaveformDataUp : public TObject {
  public:
    // Compile-time fixed sizes for the sampled signal matrix (64x64 -> 2080 independent elements)
    static const size_t c_nADCsignal = 64; /**< number of ADC samples in the signal fit */
    static const size_t c_nElements = c_nADCsignal * (c_nADCsignal + 1) / 2; /**< number of independent elements */
    static const size_t c_nParams = 14; /**< number of parameters defining the waveform shape */

    /** Setter method for independent matrix element */
    void setMatrixElement(size_t i, float value)
    {
      assert(i < c_nElements);
      m_matrixElement[i] = value;
    }
    /** Getter method for independent matrix element */
    float getMatrixElement(size_t i) const
    {
      assert(i < c_nElements);
      return m_matrixElement[i];
    }

    /** Getter method for independent matrix element */
    float getMatrixElement(size_t i, size_t j) const
    {
      if (i < j) std::swap(i, j);
      return m_matrixElement[ i * (i + 1) / 2 + j];
    }

    /** Getter method for all matrix as one dimensional array */
    void getArray(float WF[c_nElements]) const  {for (size_t i = 0; i < c_nElements; i++) { WF[i] = (float) m_matrixElement[i];}   }

    /** Getter method for all matrix as two dimensional array (floats) */
    void getMatrix(float M[c_nADCsignal][c_nADCsignal]) const
    {
      const float* A = m_matrixElement;
      for (size_t i = 0; i < c_nADCsignal; i++) {
        for (size_t j = 0; j < i; j++) M[i][j] = M[j][i] = *A++;
        M[i][i] = *A++;
      }
    }

    /** Getter method for all matrix as two dimensional array (doubles) */
    void getMatrix(double M[c_nADCsignal][c_nADCsignal]) const
    {
      const float* A = m_matrixElement;
      for (size_t i = 0; i < c_nADCsignal; i++) {
        for (size_t j = 0; j < i; j++) M[i][j] = M[j][i] = *A++;
        M[i][i] = *A++;
      }
    }

    /** Setter for matrix from std::array of floats */
    void storeMatrix(const std::array<std::array<float, c_nADCsignal>, c_nADCsignal>& M) // using std::array for intel compiler (TF)
    {
      float* A = m_matrixElement;
      for (size_t i = 0; i < c_nADCsignal; i++) {
        for (size_t j = 0; j < i; j++) *A++ = M[i][j];
        *A++ = M[i][i];
      }
    }

    /** Setter for matrix from std::array of doubles */
    void storeMatrix(const std::array<std::array<double, c_nADCsignal>, c_nADCsignal>& M) // using std::array for intel compiler (TF)
    {
      float* A = m_matrixElement;
      for (size_t i = 0; i < c_nADCsignal; i++) {
        for (size_t j = 0; j < i; j++) *A++ = static_cast<float>(M[i][j]);
        *A++ = M[i][i];
      }
    }

    /** Setter method for waveform shape parameter */
    void setWaveformPar(size_t i, float value)
    {
      assert(i < c_nParams);
      m_waveformPar[i] = value;
    }


    /** Getter method for waveform shape parameter */
    float getWaveformPar(size_t i) const
    {
      assert(i < c_nParams);
      return m_waveformPar[i];
    }

    /** Getter method for waveform shape parameters as one dimensional array of
     * floats */
    void getWaveformParArray(float P[c_nParams]) const
    {
      for (size_t i = 0; i < c_nParams; i++)
        P[i] = (float) m_waveformPar[i];
    }

    /** Getter method for waveform shape parameters as one dimensional array of
     * doubles */
    void getWaveformParArray(double P[c_nParams]) const
    {
      for (size_t i = 0; i < c_nParams; i++)
        P[i] = m_waveformPar[i];
    }

    /** print-out function for debugging purpose*/
    void print() const;

  private:
    Float_t m_matrixElement[c_nElements]; /**< the matrix elements */
    Float_t m_waveformPar[c_nParams]; /**< the waveform parameters */

    ClassDef(ECLWaveformDataUp, 1); /**< ClassDef */
  };

  /** Container for constant parameters used in waveform fits */

  class ECLWFAlgoParamsUp : public TObject {
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

    ClassDef(ECLWFAlgoParamsUp, 1); /**< ClassDef */
  };

  /** Container for constant matrix used to generate electronic noise */
  class ECLNoiseDataUp : public TObject {
  public:
    // Noise is defined for full ADC window (fixed at 64 samples)
    static const size_t c_nADC = 64; /**< number of ADC samples */
    static const size_t c_nElements = c_nADC * (c_nADC + 1) / 2; /**< number of independent elements */
    /** Setter method for independent matrix element */
    void setMatrixElement(size_t i, float value)
    {
      assert(i < c_nElements);
      m_matrixElement[i] = value;
    }
    /** Getter method for independent matrix element */
    float getMatrixElement(size_t i) const
    {
      assert(i < c_nElements);
      return m_matrixElement[i];
    }

    /** Getter method for matrix as one dimensional array */
    void getArray(float NoiseData[c_nElements]) const  { for (size_t i = 0; i < c_nElements; ++i) { NoiseData[i] = m_matrixElement[i]; } }

    /** Getter method for matrix as two dimensional array */
    void getMatrix(float M[c_nADC][c_nADC]) const
    {
      const float* A = m_matrixElement;
      for (size_t i = 0; i < c_nADC; ++i) {
        for (size_t j = 0; j < c_nADC; ++j) {
          if (j > i) { // fill only the lower triangle
            M[i][j] = 0.f;
          } else {
            M[i][j] = *A++;
          }
        }
      }
    }

    /** sampling a random vector x from the 64-dimensional multivariate normal distribution with covariance matrix C */
    void generateCorrelatedNoise(const float z[c_nADC], float x[c_nADC]) const
    {
      // z = (z0, ..., z63) is a inpute vector whose components are 64 independent standard normal variates
      // the output vector x is x0 + A*z, where x0 is 0 at the moment
      // A*A^T = C where C is a positive definite covariance matrix, A is a lower triangular matrix
      const float* A = m_matrixElement;
      for (size_t i = 0; i < c_nADC; ++i) {
        float sum = 0.f;
        for (size_t j = 0; j <= i; ++j) sum += z[j] * (*A++);
        x[i] = sum;
      }
    }
    Float_t m_matrixElement[c_nElements]; /**< electronic noise matrix */

    ClassDef(ECLNoiseDataUp, 1); /**< ClassDef */
  };


  /** Class for a lookup table */
  class ECLLookupTableUp : public TObject {
  public:
    /** Constructor */
    ECLLookupTableUp() : m_content(ECLElementNumbers::c_NCrystals, 0) {}

    /** Array-like access operator */
    unsigned int operator[](unsigned int key) const
    { return m_content[ key - 1 ]; }
    /** Array-like access operator */
    unsigned int& operator[](unsigned int key)
    { return m_content[ key - 1 ]; }

  private:
    std::vector<unsigned int> m_content; /**< index to index lookup table */
    ClassDef(ECLLookupTableUp, 1); /**< ClassDef */
  };

}
