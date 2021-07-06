/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <vector>
#include <cmath>
#include <framework/logging/Logger.h>
#include <iostream>

namespace Belle2 {

  /**
  *   dE/dx eletronic ADC non-linearity correction for
  *   highly ionising particles (used in offline hadron saturation calibration)
  *   paramters are for X vs Y relation and sep for inner and outer layer
  *   vector array 0,1 for inner and 2,3 for outer layers
  */

  class CDCDedxADCNonLinearity: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCDedxADCNonLinearity(): m_nonlADC() {};

    /**
     * Constructor
     */
    explicit CDCDedxADCNonLinearity(const std::vector<std::vector<double>>& nonlinearity): m_nonlADC(nonlinearity) {};

    /**
     * Destructor
     */
    ~CDCDedxADCNonLinearity() {};

    /**
     * Get the number of bins for the non-linearity angle correction
     * @param layer outer vs inner 0 to 56 indexing
     * @param axis 0 for X and 1 for Y
     */
    unsigned int getSize(int layer, int axis) const
    {

      int mylayer = 0;
      if (layer < 0 || layer > 55) {
        B2ERROR("CDCDedxADCNonLinearity:CDC layer out of range, choose between 0-55");
        return 0;
      } else {
        if (layer >= 8) mylayer = 2;
      }

      if (axis < 0 || axis > 1) {
        B2ERROR("CDCDedxADCNonLinearity:Problem with X/Y axis: choose 0/1 for X/Y");
        return 0;
      } else {
        mylayer = mylayer + axis;
      }

      return m_nonlADC[mylayer].size();
    };


    /** Return corrected ADC with given parameters
     * @param ADC uncorrected version
     * @param layer outer vs inner 0 to 56 indexing
     */
    double getCorrectedADC(const double& ADC, const int layer)const
    {

      int mylayer = 0;
      if (layer < 0 || layer > 55) {
        B2WARNING("CDCDedxADCNonLinearity:returning uncorrected ADC as input layer is out of range: must be 0-55");
        return ADC;
      } else {
        if (layer >= 8) mylayer = 2;
      }

      std::vector<double> tempX = m_nonlADC[mylayer];//inner or outer X array
      std::vector<double> tempY = m_nonlADC[mylayer + 1];//inner or outer Y array

      if (tempX.size() != tempY.size()) {
        B2WARNING("CDCDedxADCNonLinearity:returning uncorrected ADC as parameters range don't match: X=Y in bins");
        return ADC;
      }

      //Find bin for ADC correction
      unsigned int ibin = TMath::BinarySearch(tempY.size(), tempY.data(), double(ADC));

      if (ibin >= tempY.size() - 1)ibin = tempY.size() - 2; //overflow to last bin
      if (ibin >= tempY.size() - 1) {
        B2WARNING("CDCDedxADCNonLinearity:returning uncorrected ADC as bins are not in range");
        return ADC;
      }

      double slope = (tempY[ibin + 1] - tempY[ibin]) / (tempX[ibin + 1] - tempX[ibin]);
      return  std::round(tempX[ibin] + (ADC - tempY[ibin]) / slope);
    }

    /** return specific hadron parameter
     * @param layer outer(8-55) vs inner(0-7)
     * @param axis 0 for X and 1 for Y
     * @param par number starts from 0
     */
    double getNonLinearityPar(int layer, int axis, unsigned int par) const
    {

      int mylayer = 0;
      if (layer < 0 || layer > 55) {
        B2ERROR("CDCDedxADCNonLinearity:CDC layer out of range, choose between 0-55");
        return -99.0;
      } else {
        if (layer >= 8) mylayer = 2;
      }

      if (axis < 0 || axis > 1) {
        B2ERROR("CDCDedxADCNonLinearity:Problem with X/Y axis: choose 0/1 for X/Y");
        return -99.0;
      } else {
        mylayer = mylayer + axis;
      }

      if (par >= m_nonlADC[mylayer].size()) {
        B2ERROR("CDCDedxADCNonLinearity:Problem with par index: choose 0 and " << m_nonlADC[mylayer].size()); //
        return -99.0;
      }

      return m_nonlADC[mylayer][par];
    };

    /** print requested hadron parameter array
     * @param layer outer vs inner
     * @param axis 0 for X and 1 for Y
     */
    void printNonLinearityPars(int layer, int axis) const
    {

      int mylayer = 0;
      if (layer < 0 || layer > 55) {
        B2ERROR("CDCDedxADCNonLinearity:CDC layer out of range, choose between 0-55");
        return;
      } else {
        if (layer >= 8) mylayer = 2;
      }

      if (axis < 0 || axis > 1) {
        B2ERROR("CDCDedxADCNonLinearity:Problem with X/Y axis: choose 0/1 for X/Y");
        return;
      } else {
        mylayer = mylayer + axis;
      }

      B2INFO("Printing parameters for layer: " << layer << ", axis: " << axis << ", nPars:" << m_nonlADC[mylayer].size());
      for (unsigned int iPar = 0; iPar < m_nonlADC[mylayer].size(); iPar++)
        std::cout << "Par # " << iPar << ": " << m_nonlADC[mylayer][iPar] << std::endl;

    };

    /** set specific hadron parameter
     * @param layer outer(8-55) vs inner(0-7)
     * @param axis 0 for X and 1 for Y
     * @param par number starts from 0
     * @param value of parameter to set
     */
    void setNonLinearityPar(unsigned int layer, unsigned int axis, unsigned int par, double value)
    {
      int mylayer = 0;
      if (layer > 55) {
        B2ERROR("CDCDedxADCNonLinearity:CDC layer out of range, choose between 0-55");
        return;
      } else {
        if (layer >= 8) mylayer = 2;
      }

      if (axis > 1) {
        B2ERROR("CDCDedxADCNonLinearity:Problem with X/Y axis: choose 0/1 for X/Y");
        return;
      } else {
        mylayer = mylayer + axis;
      }

      if (par >= m_nonlADC[mylayer].size()) {
        B2ERROR("CDCDedxADCNonLinearity:Problem with par index: choose 0 and " << m_nonlADC[mylayer].size()); //
      }

      //here set parameter to requested value
      m_nonlADC[mylayer][par] = value;

    };


  private:
    std::vector<std::vector<double>> m_nonlADC; /**< ADC vs corrected ADC mapping */
    ClassDef(CDCDedxADCNonLinearity, 1); /**< ClassDef */
  };
} // end namespace Belle2
