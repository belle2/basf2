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

#include <TObject.h>

#include <analysis/dbobjects/PIDPriorsTable.h>
#include <framework/gearbox/Const.h>

class TH2F;

namespace Belle2 {

  /**
   * A database class to hold the prior probability for
   * the particle identification.
   * This prior probability  for each particle species is stored as PIDPriorsTable object, that allows to bin it in 2 dimensions defined by two kinematic variables.
   * This class provides a container for 6 PIDPriorsTable objects, some wrappers around the most used getters and setters, and
   * some setters that allow the user to load the priors starting from a TH2 object.
   */
  class PIDPriors : public TObject {

    /**
     * Class revision number
     */
    ClassDef(PIDPriors, 1);


  public:

    /**
     * Default constructor.
     */
    PIDPriors() {};


    /**
     * Sets the prior table for a particle species from a PIDPriorsTable object
     * @param particle the charged stable type of the particle
     * @param table the priors table
     */
    void setPriors(const Const::ChargedStable& particle, const PIDPriorsTable& table)
    {
      auto index = particle.getIndex();
      m_priors[index] = table;
      return;
    }


    /**
     * Sets the prior table for a particle species from the std::vectors of the bin edges and the probability values
     * @param particle the charged stable type of the particle
     * @param xAxisEdges the std::vector<float> containing the bin edges of the X axis
     * @param yAxisEdges the std::vector<float> containing the bin edges of the Y axis
     * @param priorsTable the 2D std::vector<float> containing the prior probabilities
     * @param errorsTable the 2D std::vector<float> containing the errors on prior probabilities
     */
    void setPriors(const Const::ChargedStable& particle, std::vector<float> xAxisEdges, std::vector<float> yAxisEdges,
                   std::vector<float> priorsTable, std::vector<float> errorsTable)
    {
      auto index = particle.getIndex();
      m_priors[index].setBinEdges(xAxisEdges, yAxisEdges);
      m_priors[index].setBinEdges(xAxisEdges, yAxisEdges);
      m_priors[index].setPriorsTable(priorsTable);
      m_priors[index].setErrorsTable(errorsTable);
      return;
    }


    /**
     * Sets the prior table for a particle species from a TH2F.
     * The bin edges are taken form the TH2 axes, the prior probability from the bins content and the errors from the bins error
     * @param particle the charged stable type of the particle
     * @param priorHistogram the prior for the particle specie
     */
    void setPriors(const Const::ChargedStable& particle, TH2F* priorHistogram);


    /**
     * Sets the prior table for a particle species starting from a TH2F of the counts and a TH2F of the normalization.
     * The counts histogram is divided by the normalization one to get the probabilities and the errors
     * The bin edges are taken form the TH2 axes, the prior probability from the bins content and the errors from the bins error
     * @param particle the charged stable type of the particle
     * @param counts histogram of counts
     * @param normalization histogram used for normalization
     */
    void setPriors(const Const::ChargedStable& particle, TH2F* counts, TH2F* normalization);


    /**
     * Sets the axes for the priors table of the selected species.
     * Effectively this is just a wrapper around PIDPriorsTable::setBinEdges()
     * @param particle the charged stable type of the prior's species
     * @param xAxisEdges the vector of edges of the X axis
     * @param yAxisEdges the vector of edges of the Y axis
     */
    void setPriorsAxes(const Const::ChargedStable& particle, std::vector<float> xAxisEdges, std::vector<float> yAxisEdges)
    {
      auto index = particle.getIndex();
      m_priors[index].setBinEdges(xAxisEdges, yAxisEdges);
      m_priors[index].setBinEdges(xAxisEdges, yAxisEdges);
      return;
    }


    /**
     * Sets the probability table for the priors of the selected particle species.
     * Effectively this is just a wrapper around PIDPriorsTable::setPriorsTable()
     * @param particle the charged stable type of the prior's species
     * @param priorsTable a 2D vector containing the prior probabilities
     */
    void setPriorsTable(const Const::ChargedStable& particle, std::vector<float> priorsTable)
    {
      auto index = particle.getIndex();
      m_priors[index].setPriorsTable(priorsTable);
      return;
    }


    /**
     * Sets the probability error table for the priors of the selected particle species.
     * Effectively this is just a wrapper around PIDPriorsTable::setErrorssTable()
     * @param particle the charged stable type of the prior's species
     * @param errorsTable a 2D vector containing the prior probabilities
     */
    void setErrorsTable(const Const::ChargedStable& particle, std::vector<float> errorsTable)
    {
      auto index = particle.getIndex();
      m_priors[index].setErrorsTable(errorsTable);
      return;
    }


    /**
     * Sets the axis labels for the priors of the selected particle species.
     * @param particle the charged stable type of the prior's species
     * @param xAxisLabel the label of the X axis
     * @param yAxisLabel the label of the Y axis
     */
    void setAxisLabels(const Const::ChargedStable& particle, std::string xAxisLabel, std::string yAxisLabel)
    {
      auto index = particle.getIndex();
      m_priors[index].setAxisLabels(xAxisLabel, yAxisLabel);
      return;
    }


    /**
     * Returns the priors table of the selected particle species.
     * @param particle the charged stable type of the prior's species
     * @return the priors table
     */
    PIDPriorsTable getPriorsTable(const Const::ChargedStable& particle) const
    {
      auto index = particle.getIndex();
      return m_priors[index];
    }


    /**
     * Returns the prior probability associated to a particle with defined species and parameters.
     * @param particle the charged stable type of the prior's species
     * @param x the value of the x-axis coordinate
     * @param y the value of the y-axis coordinate
     * @return the prior probability value
     */
    float getPriorValue(const Const::ChargedStable& particle, float x, float y) const
    {
      auto index = particle.getIndex();
      return m_priors[index].getPriorValue(x, y);
    }


    /**
     * Returns the error on the prior probability associated to a particle with defined species and parameters.
     * @param particle the charged stable type of the prior's species
     * @param x the value of the x-axis coordinate
     * @param y the value of the y-axis coordinate
     * @return the prior probability error
     */
    float getPriorError(const Const::ChargedStable& particle, float x, float y) const
    {
      auto index = particle.getIndex();
      return m_priors[index].getErrorValue(x, y);
    }


    /**
     * Returns the X axis label of the prior
     * @param particle the charged stable type of the prior's species
     * @return the X axis label for the prior table of the selected particle species
     */
    std::string getXAxisLabel(const Const::ChargedStable& particle) const
    {
      auto index = particle.getIndex();
      return m_priors[index].getXAxisLabel();
    }


    /**
     * Returns the Y axis label of the prior
     * @param particle the charged stable type of the prior's species
     * @return the Y axis label for the prior table of the selected particle species
     */
    std::string getYAxisLabel(const Const::ChargedStable& particle) const
    {
      auto index = particle.getIndex();
      return m_priors[index].getYAxisLabel();
    }

  private:
    PIDPriorsTable m_priors[Const::ChargedStable::c_SetSize]; /**< The array of PIDPiorsTable, one per particle species. */


  };


} // Belle2 namespace


