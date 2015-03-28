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
    static const size_t c_nElements = 136; /** number of independent elements */
    Float_t m_matrixElement[c_nElements]; /** electronic noise matrix */

    ClassDef(ECLNoiseData, 1)
  };


  class ECLLookupTable : public TObject {
  public:
    ECLLookupTable() : m_content(8736, 0) {}

    int operator[](int key) const
    { return m_content[ key ]; }
    int& operator[](int key)
    { return m_content[ key ]; }

  private:
    std::vector<int> m_content; /** index to index lookup table */
    ClassDef(ECLLookupTable, 1);
  };

}
#endif
