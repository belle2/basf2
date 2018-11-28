/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/logging/Logger.h>
#include <iostream>
#include <float.h>

namespace Belle2 {

  /**
   * This class holds the prior distribution for a single particle species.
   * Its basic data members are two  2D matrix of floats (one for the probablity, one for it uncertainties)
   * plus two arrays for the bins edges.
   * The PIDPiors db object is basically a collection of 6 of these objects, one per particle species.
   */
  class PIDPriorsTable {

  public:

    /**
     * This function the position of a number in a sorted array of bin edges
     * @param val the value
     * @param array the std::vector the val has to be found
     * @returns the position of the last edge below the input value
     */
    short findBin(float val, std::vector<float> array) const
    {
      auto it = std::lower_bound(array.cbegin(),  array.cend(), val);
      return std::distance(array.cbegin(), it) - 1;
    }


    /**
     * This function the position of a number in a sorted array of bin edges
     * This implementation should be more clever than the
     * binary search implemented in findBin, but at the very end it seems to
     * be slower. It's kept in here just in case someone passes by and finds
     * a way to speed it up.
     * @param val the value
     * @param array the std::vector the val has to be found
     * @returns the position of the last edge below the input value
     */
    short findBinFast(float value, std::vector<float> array) const
    {
      // This function seraches for the first bin the axis which is above the
      // value. First it starts assuming the bins are equal, and then moves around
      // the array until the correct bin is found
      float averageBinSize = float(array[array.size() - 1] - array[0]) / (float)(array.size() - 1);
      int bin = 1 + (int)((value - array[0]) / averageBinSize);
      //adjusts forward
      while (array[bin] < value && bin < (int)array.size()) {
        bin++;
      }
      //adjusts backward
      while (array[bin - 1] > value && bin - 1 >  0) {
        bin--;
      }
      // The corresponding bin of the priors matrix is 1- the bin we found (prior[n] has, as edges, axis[n] and axis[n+1])
      return bin - 1;
    }


    /**
     * Sets the bin edges arrays. Please notice that setting the axes will also automatically
     * clear the tables.
     * @param binEdgesX the vector of bin edges for the X axis
     * @param binEdgesY the vector of bin edges for the Y axis
     */
    void setBinEdges(std::vector<float>binEdgesX, std::vector<float>binEdgesY)
    {
      m_binEdgesX = binEdgesX;
      m_binEdgesY = binEdgesY;

      // if less than two edges are provided, assumes the maximum range for floats
      if (m_binEdgesY.size() < 2) {
        B2WARNING("You provided less than 2 bin edges for the Y axis. This is not enough to create one bin, so one will be created with range [-inf, +inf]");
        m_binEdgesY.clear();
        m_binEdgesY.push_back(-FLT_MAX);
        m_binEdgesY.push_back(+FLT_MAX);
      }

      if (m_binEdgesX.size() < 2) {
        B2WARNING("You provided less than 2 bin edges for the X axis. This is not enough to create one bin, so one will be created with range [-inf, +inf]");
        m_binEdgesX.clear();
        m_binEdgesX.push_back(-FLT_MAX);
        m_binEdgesX.push_back(+FLT_MAX);
      }

      auto prevVal = m_binEdgesX[0];
      for (int iBin = 1;  iBin < (int)m_binEdgesX.size(); iBin++) {
        auto edge  = m_binEdgesX[iBin];
        if (prevVal >= edge)
          B2FATAL("Null or negative bis size found on the X axis. Please make sure that all the bis edges are sorted and non-equal");
        prevVal = edge;
      }
      prevVal = m_binEdgesY[0];
      for (int iBin = 1;  iBin < (int)m_binEdgesY.size(); iBin++) {
        auto edge = m_binEdgesY[iBin];
        if (prevVal >= edge)
          B2FATAL("Null or negative bis size found on the Y axis. Please make sure that all the bis edges are sorted and non-equal");
        prevVal = edge;
      }

      m_priors.clear();
      m_errors.clear();

      m_priors.resize(m_binEdgesY.size() - 1, std::vector<float>(m_binEdgesX.size() - 1, 0.));
      m_errors.resize(m_binEdgesY.size() - 1, std::vector<float>(m_binEdgesX.size() - 1, 0.));
      return;
    };


    /**
     * Sets the priors table from a 2D std::vector
     * @param priors 2D std:vector of floats containing the prior probaiblity for each bin
     */
    void setPriorsTable(std::vector<std::vector<float>> priors)
    {
      if (m_binEdgesX.size() == 0 || m_binEdgesX.size() == 0) {
        B2ERROR("It seems you are trying to set the prior table before setting the bin edges. Please, do vice-versa and set first the arrays of the bin edges using setBinEdges(std::vector<float>binEdgesX, std::vector<float>binEdgesY ).");
      }
      if (priors.size() != m_priors.size()) {
        B2ERROR("The number of rows in the input array (" << priors.size() << ") does not match the corresponding number of bin edges (" <<
                m_priors.size() << ")");
        return;
      }
      if (priors[0].size() != m_priors[0].size()) {
        B2ERROR("The number of columns in the input array (" << priors[0].size() <<
                ") does not match the corresponding number of bin edges (" << m_priors[0].size() << ")");
        return;
      }
      m_priors = priors;
      return;
    };


    /**
     * Sets the priors error table from a 2D std::vector
     * @param errors 2D std:vector of floats containing the error on the prior probaiblity for each bin
     */
    void setErrorsTable(std::vector<std::vector<float>> errors)
    {
      if (errors.size() != m_errors.size()) {
        B2ERROR("The number of rows in the input array (" << errors.size() << ") does not match the corresponding number of bin edges (" <<
                m_errors.size() << ")");
        return;
      }
      if (errors[0].size() != m_errors[0].size()) {
        B2ERROR("The number of columns in the input array (" << errors[0].size() <<
                ") does not match the corresponding number of bin edges (" << m_errors[0].size() << ")");
        return;
      }
      m_errors = errors;
      return;
    };


    /**
     * Sets the prior value for a single bin
     * @param x the x-value of the x coordinate
     * @param y the y-value of the y coordinate
     * @param value the x-value of the prior probability associated to x and y
     */
    int setPriorValue(float x, float y, float value)
    {
      if (m_binEdgesX[0] > x || m_binEdgesX[m_binEdgesX.size() - 1] < x) {
        B2WARNING("The value " << x  << " is out of range for the X axis (" << m_binEdgesX[0] << ",  " << m_binEdgesX[m_binEdgesX.size() -
                  1] << "). Skipping.");
        return 1;
      }
      if (m_binEdgesY[0] > y || m_binEdgesY[m_binEdgesY.size() - 1] < y) {
        B2WARNING("The value " << y  << " is out of range for the Y axis (" << m_binEdgesY[0] << ",  " << m_binEdgesY[m_binEdgesY.size() -
                  1] << "). Skipping.");
        return 1;
      }
      if (value > 1. || value < 0.) {
        B2WARNING("The value " << value  << " you are trying to set for the bin (" << x << ",  " << y <<
                  ") does not look like a probability. The table will be filled, but i  will not be usable as a proper prior probability table.");
      }
      auto binX = findBin(x, m_binEdgesX);
      auto binY = findBin(y, m_binEdgesY);
      m_priors[binY][binX] = value;
      return 0;
    }


    /**
     * Sets the error on the prior value for a single bin
     * @param x the x-value of the x coordinate
     * @param y the y-value of the y coordinate
     * @param value the x-value of the prior probability error associated to x and y
     */
    int setErrorValue(float x, float y, float value)
    {
      if (m_binEdgesX[0] > x || m_binEdgesX[m_binEdgesX.size() - 1] < x) {
        B2WARNING("The value " << x  << " is out of range for the X axis (" << m_binEdgesX[0] << ",  " << m_binEdgesX[m_binEdgesX.size() -
                  1] << "). Skipping.");
        return 1;
      }
      if (m_binEdgesY[0] > y || m_binEdgesY[m_binEdgesY.size() - 1] < y) {
        B2WARNING("The value " << y  << " is out of range for the Y axis (" << m_binEdgesY[0] << ",  " << m_binEdgesY[m_binEdgesY.size() -
                  1] << "). Skipping.");
        return 1;
      }
      auto binX = findBin(x, m_binEdgesX);
      auto binY = findBin(y, m_binEdgesY);
      m_errors[binY][binX] = value;
      return 0;
    }


    /**
     * Sets axes lables
     * @param labelX the label of the X axis
     * @param labelY the label of the X axis
     */
    void setAxisLabels(std::string labelX, std::string labelY)
    {
      m_xAxisLabel = labelX;
      m_yAxisLabel = labelY;
      return ;
    }



    /**
     * Returns the prior probability for a given value of (x,y). If the bin is outside of the table boundaries, 0 will be returned.
     * @param x the x-value of the x coordinate
     * @param y the y-value of the y coordinate
     * @param value the prior probability value
     */
    float getPriorValue(float x, float y) const
    {
      if (m_binEdgesX[0] > x || m_binEdgesX[m_binEdgesX.size() - 1] < x) {
        B2WARNING("The value " << x  << " is out of range for the X axis (" << m_binEdgesX[0] << ",  " << m_binEdgesX[m_binEdgesX.size() -
                  1] << "). Returning 0.");
        return 0.;
      }
      if (m_binEdgesY[0] > y || m_binEdgesY[m_binEdgesY.size() - 1] < y) {
        B2WARNING("The value " << y  << " is out of range for the Y axis (" << m_binEdgesY[0] << ",  " << m_binEdgesY[m_binEdgesY.size() -
                  1] << "). Returning 0.");
        return 0.;
      }
      auto binX = findBin(x, m_binEdgesX);
      auto binY = findBin(y, m_binEdgesY);
      return m_priors[binY][binX];
    }

    /**
     * Returns the error on the prior probability for a given value of (x,y). If the bin is outside of the table boundaries, 0 will be returned.
     * @param x the x-value of the x coordinate
     * @param y the y-value of the y coordinate
     * @param value the error on the prior probability value
     */
    float getErrorValue(int binX, int binY) const
    {
      if (binX > (int)m_binEdgesX.size() - 1  || binX < 0) {
        B2WARNING("The bin " <<  binX  << " is out of range for the X axis which has  " << m_binEdgesX.size() << " elements. Returning 0.");
        return 0.;
      }
      if (binY > (int)m_binEdgesY.size() - 1  || binY < 0) {
        B2WARNING("The bin " <<  binY  << " is out of range for the Y axis which has  " << m_binEdgesY.size() << " elements. Returning 0.");
        return 0.;
      }
      return m_errors[binY][binX];
    }


    /**
     * Returns the X axis label
     * @return the X axis label
     */
    std::string getXAxisLabel() const
    {
      return m_xAxisLabel;
    }

    /**
     * Returns the Y axis label
     * @return the Y axis label
     */
    std::string getYAxisLabel() const
    {
      return m_yAxisLabel;
    }


    /**
     * Prints the content of the table and the axes
     */
    void printPrior() const
    {
      std::cout << " --- Prior summary --- " << std::endl;
      std::cout << " Size :  " << m_priors[0].size() << " X  " << m_priors.size() << std::endl;
      std::cout << " X axis:  " ;
      for (auto edge : m_binEdgesX) {
        std::cout << " "  << edge << " " ;
      }
      std::cout << " " << std::endl;
      std::cout << " Y axis ";
      for (auto edge : m_binEdgesY) {
        std::cout << " " << edge << " " ;
      }
      std::cout << " " << std::endl;
      std::cout << " Values " << std::endl;
      for (int iY = m_priors.size() - 1; iY >= 0; iY--) {
        for (int iX = 0; iX < (int)m_priors[0].size(); iX++) {
          std::cout << " " << m_priors[iY][iX] << " ";
        }
        std::cout << " " << std::endl;
      }
      std::cout << " --- End of prior summary --- " << std::endl;
    }


    /**
     * Prints the content of the error table  and the axes
     */
    void printError() const
    {
      std::cout << " --- Prior summary --- " << std::endl;
      std::cout << " Size :  " << m_priors[0].size() << " X  " << m_priors.size() << std::endl;
      std::cout << " X axis:  " ;
      for (auto edge : m_binEdgesX) {
        std::cout << " "  << edge << " " ;
      }
      std::cout << " " << std::endl;
      std::cout << " Y axis ";
      for (auto edge : m_binEdgesY) {
        std::cout << " " << edge << " " ;
      }
      std::cout << " " << std::endl;
      std::cout << " Values " << std::endl;
      for (int iY = m_priors.size() - 1; iY >= 0; iY--) {
        for (int iX = 0; iX < (int)m_priors[0].size(); iX++) {
          std::cout << " " << m_errors[iY][iX] << " ";
        }
        std::cout << " " << std::endl;
      }
      std::cout << " --- End of prior summary --- " << std::endl;
    }


  private:

    std::vector<float> m_binEdgesX = {}; /**< The array containing the bin edges for the X axis */
    std::vector<float> m_binEdgesY = {}; /**< The array containing the bin edges for the Y axis */
    std::vector<std::vector<float>> m_priors; /**< The matrix with the prior probabilities */
    std::vector<std::vector<float>> m_errors; /**< The the matrix with the errors on the prior probabilities */
    std::string m_xAxisLabel = "" ; /**< label of the X axis, indicating which variable is represented here */
    std::string m_yAxisLabel = "" ; /**< label of the Y axis, indicating which variable is represented here */
  };
} // Belle2 namespace
