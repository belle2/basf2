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

#include <vector>
#include <string>

namespace Belle2 {

  /**
   * This class holds the prior distribution for a single particle species.
   * Its basic data members are two 2D matrices of floats (one for the probability, one for it uncertainties)
   * plus two arrays for the bins edges. All these elements, including the matrices, are implemented as
   * 1 dimensional vectors.
   * The PIDPriors class that is stored in the database is basically nothing but a collection of 6 of these objects, one per particle species.
   */
  class PIDPriorsTable {

  public:

    /**
     * Sets the bin edges arrays. Please notice that setting the axes will also automatically
     * clear the tables.
     * @param binEdgesX the vector of bin edges for the X axis
     * @param binEdgesY the vector of bin edges for the Y axis
     */
    void setBinEdges(const std::vector<float>& binEdgesX, const std::vector<float>& binEdgesY);


    /**
     * Sets the priors table from a 2D std::vector
     * @param priors 2D std:vector of floats containing the prior probability for each bin
     */
    void setPriorsTable(const std::vector<float>& priors)
    {
      m_priors = priors;
      return;
    };


    /**
     * Sets the priors error table from a 2D std::vector
     * @param errors 2D std:vector of floats containing the error on the prior probability for each bin
     */
    void setErrorsTable(const std::vector<float>& errors)
    {
      m_errors = errors;
      return;
    };


    /**
     * Sets the prior value for a single bin
     * @param x the x-value of the x coordinate
     * @param y the y-value of the y coordinate
     * @param value the x-value of the prior probability associated to x and y
     */
    void setPriorValue(float x, float y, float value);


    /**
     * Sets the error on the prior value for a single bin
     * @param x the x-value of the x coordinate
     * @param y the y-value of the y coordinate
     * @param value the x-value of the prior probability error associated to x and y
     */
    void setErrorValue(float x, float y, float value);


    /**
     * Sets axes labels
     * @param labelX the label of the X axis
     * @param labelY the label of the X axis
     */
    void setAxisLabels(const std::string& labelX, const std::string& labelY)
    {
      m_xAxisLabel = labelX;
      m_yAxisLabel = labelY;
      return ;
    }


    /**
     * Returns the prior probability for a given bin. If the bin is outside of the table boundaries, 0 will be returned.
     * @param iX bin number on the X axis
     * @param iY bin number on the Y axis
     * @return value the prior probability value
     */
    float getPriorInBin(int iX, int iY) const
    {
      return m_priors[iX + (m_binEdgesX.size() - 1) * iY];
    };


    /**
     * Returns the error on prior probability for a given bin. If the bin is outside of the table boundaries, 0 will be returned.
     * @param iX bin number on the X axis
     * @param iY bin number on the Y axis
     * @return the error on the prior probability value
     */
    float getErrorInBin(int iX, int iY) const
    {
      return m_errors[iX + (m_binEdgesX.size() - 1) * iY];
    };


    /**
     * Returns the prior probability for a given value of (x,y). If the bin is outside of the table boundaries, 0 will be returned.
     * @param x the x-value of the x coordinate
     * @param y the y-value of the y coordinate
     * @return the value the prior probability value
     */
    float getPriorValue(float x, float y) const;


    /**
     * Returns the error on the prior probability for a given value of (x,y). If the bin is outside of the table boundaries, 0 will be returned.
     * @param x the x-value of the x coordinate
     * @param y the y-value of the y coordinate
     * @return value the error on the prior probability value
     */
    float getErrorValue(float x, float y) const;


    /**
     * Returns the X axis label
     * @return the X axis label
     */
    std::string getXAxisLabel() const
    {
      return m_xAxisLabel;
    };

    /**
     * Returns the Y axis label
     * @return the Y axis label
     */
    std::string getYAxisLabel() const
    {
      return m_yAxisLabel;
    };


    /**
     * Prints the content of the table and the axes
     */
    void printPrior() const;

    /**
     * Prints the content of the error table  and the axes
     */
    void printError() const;


  private:

    std::vector<float> m_binEdgesX = {}; /**< The array containing the bin edges for the X axis */
    std::vector<float> m_binEdgesY = {}; /**< The array containing the bin edges for the Y axis */
    std::vector<float> m_priors; /**< The matrix with the prior probabilities */
    std::vector<float> m_errors; /**< The matrix with the errors on the prior probabilities */
    std::string m_xAxisLabel = "" ; /**< label of the X axis, indicating which variable is represented here */
    std::string m_yAxisLabel = "" ; /**< label of the Y axis, indicating which variable is represented here */


    /**
     * Checks if a value is within the range of an array
     * @param text text to display in the warning message
     * @param val the value
     * @param edges the vector of bin edge values the val has to be found in
     * @returns the position of the last edge below the input value
     */
    bool checkRange(const std::string& text, float val, const std::vector<float>& edges) const;

    /**
     * This function returns the position of a number in a sorted array of bin edges
     * @param val the value
     * @param array the std::vector the val has to be found
     * @returns the position of the last edge below the input value
     */
    short findBin(float val, std::vector<float> array) const;

    /**
     * This function returns the position of a number in a sorted array of bin edges
     * This implementation should be more clever than the
     * binary search implemented in findBin, but at the very end it seems to
     * be slower. It's kept in here just in case someone passes by and finds
     * a way to speed it up.
     * @param val the value
     * @param array the std::vector the val has to be found
     * @returns the position of the last edge below the input value
     */
    // cppcheck-suppress unusedPrivateFunction
    short findBinFast(float val, std::vector<float> array) const;

    /**
     * This function returns the position of a number in a sorted array of bin edges,
     * assuming that the latter are equally spaced.
     * @param val the value
     * @param array the std::vector the val has to be found
     * @returns the position of the last edge below the input value
     */
    // cppcheck-suppress unusedPrivateFunction
    short findBinWithFixedWidth(float val, std::vector<float> array) const;


  };
} // Belle2 namespace
