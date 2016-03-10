/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLWAVEFORMDATA_H
#define ECLWAVEFORMDATA_H

#include <cassert>
#include <vector>
#include <utility>
#include <TObject.h>

namespace Belle2 {

  /** ECLWaveformData - container for inverse covariant matrix and shape parameters for time and amplitude fit of ecl crystal waveform
   *
   * Objects of this class keep the independent matrix elements
   * of the inverse covariant matrix to perform waveform fit and
   * the parameters defining the waveform shape.
   */

  class ECLWaveformData : public TObject {
  public:

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
    /** Getter method for all matrix as one dimentional array */

    void getArray(float WF[136]) const  {for (int i = 0; i < 136; i++) { WF[i] = (float) m_matrixElement[i];}   }

    /** Getter method for all matrix as two dimentional array */

    void getMatrix(float M[16][16]) const
    {
      const float* A = m_matrixElement;
      for (int i = 0; i < 16; i++) {
        for (int j = 0; j < i; j++) M[i][j] = M[j][i] = *A++;
        M[i][i] = *A++;
      }
    }

    void getMatrix(double M[16][16]) const
    {
      const float* A = m_matrixElement;
      for (int i = 0; i < 16; i++) {
        for (int j = 0; j < i; j++) M[i][j] = M[j][i] = *A++;
        M[i][i] = *A++;
      }
    }

//    void storeMatrix(const float M[16][16])
    void storeMatrix(const std::array<std::array<float, 16>, 16>& M) // using std::array for intel compiler (TF)
    {
      float* A = m_matrixElement;
      for (int i = 0; i < 16; i++) {
        for (int j = 0; j < i; j++) *A++ = M[i][j];
        *A++ = M[i][i];
      }
    }

//    void storeMatrix(const double M[16][16])
    void storeMatrix(const std::array<std::array<double, 16>, 16>& M) // using std::array for intel compiler (TF)
    {
      float* A = m_matrixElement;
      for (int i = 0; i < 16; i++) {
        for (int j = 0; j < i; j++) *A++ = static_cast<float>(M[i][j]);
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

    /** Getter method for waveform shape parameters as one dimentional array */
    void getWaveformParArray(float P[10]) const
    {
      for (int i = 0; i < 10; i++)
        P[i] = (float) m_waveformPar[i];
    }


    void getWaveformParArray(double P[10]) const
    {
      for (int i = 0; i < 10; i++)
        P[i] = m_waveformPar[i];
    }


    /** print-out function for debugging purpose*/

    void print() const;


    static const size_t c_nElements = 136; /** number of independent matrix elements*/
    static const size_t c_nParams = 10; /** number of parameters defining the waveform shape*/
  private:
    Float_t m_matrixElement[c_nElements];
    Float_t m_waveformPar[c_nParams];

    ClassDef(ECLWaveformData, 1);
  };

  /** Container for constant parameters used in waveform fits */

  class ECLWFAlgoParams : public TObject {
  public:

    Int_t ka;               /** multipliers power of 2 for fg31,fg41 */
    Int_t kb;               /** multipliers power of 2 for fg32 */
    Int_t kc;               /** multipliers power of 2 for fg33,fg43 */
    Int_t y0Startr;         /** start point for pedestal calculation */
    Int_t chiThresh;        /** chi2 threshold for quality bit */
    Int_t k1Chi;            /** multipliers power of 2 for f */
    Int_t k2Chi;            /** multipliers power of 2 for chi2 calculation */
    Int_t hitThresh;        /** hardware threshold(to start digitization) */
    Int_t lowAmpThresh;     /** threshold to calculate time */
    Int_t skipThresh;       /** threshold to send data to collector */
    int getka() const {return (int) ka;}
    int getkb() const {return (int) kb;}
    int getkc() const {return (int) kc;}
    int gety0s()  const {return (int) y0Startr;}
    int getcT() const {return (int) chiThresh;}
    int getk1() const {return (int) k1Chi;}
    int getk2() const {return (int) k2Chi;}
    int gethT() const {return (int) hitThresh;}
    int getlAT() const {return (int) lowAmpThresh;}
    int getsT()  const {return (int) skipThresh;}

    ClassDef(ECLWFAlgoParams, 1)
  };

  /** Container for constant matrix used to generate electronic noise */
  class ECLNoiseData : public TObject {
  public:
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

    /** Getter method for matrix as one dimentional array */

    void getArray(float NoiseData[496]) const  {for (int i = 0; i < 496; i++) { NoiseData[i] = (float) m_matrixElement[i];}   }

    /** Getter method for matrix as two dimentional array */

    void getMatrix(float M[31][31]) const
    {
      const float* A = m_matrixElement;
      for (int i = 0; i < 31; i++) {
        for (int j = 0; j < 31; j++) {
          if (j > i) { // fill only the lower triangle
            M[i][j] = 0.f;
          } else {
            M[i][j] = *A++;
          }
        }
      }
    }

    // sampling a random vector x from the 31-dimensional multivariate normal distribution with covariance matrix C
    void generateCorrelatedNoise(const float z[31], float x[31]) const
    {
      // z = (z0, ..., z30) is a inpute vector whose components are 31 independent standard normal variates
      // the output vector x is x0 + A*z, where x0 is 0 at the moment
      // A*A^T = C where C is a positive definite covariance matrix, A is a lower triangular matrix
      const float* A = m_matrixElement;
      for (int i = 0; i < 31; i++) {
        float sum = 0;
        for (int j = 0; j <= i; j++) sum += z[j] * (*A++);
        x[i] = sum;
      }
    }

    static const size_t c_nElements = 496; /** number of independent elements */

    Float_t m_matrixElement[c_nElements]; /** electronic noise matrix */

    ClassDef(ECLNoiseData, 1)
  };


  class ECLLookupTable : public TObject {
  public:
    ECLLookupTable() : m_content(8736, 0) {}

    unsigned int operator[](unsigned int key) const
    { return m_content[ key - 1 ]; }
    unsigned int& operator[](unsigned int key)
    { return m_content[ key - 1 ]; }

  private:
    std::vector<unsigned int> m_content; /** index to index lookup table */
    ClassDef(ECLLookupTable, 1);
  };

}
#endif
