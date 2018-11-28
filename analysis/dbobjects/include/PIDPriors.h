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
#include <TMath.h>
#include <TH2F.h>
#include <TAxis.h>

#include <framework/gearbox/Unit.h>
#include <analysis/dbobjects/PIDPriorsTable.h>

namespace Belle2 {

  /**
   * A database class to hold the prior probability for
   * the particle identification. The piors are defined as the
   * fraction of particle of each species in a given bin of the kinematic variables.
   * This prior for each particle species are stored as PIDPriorsTable object. This class provides
   * a container for 6 PIDPriorsTable objects, some wrappers areound the most used getters and setteres, and
   * some setters that allow the user to load the priors starting from a TH2 object.
   */
  class PIDPriors : public TObject {

  public:

    /**
     * Default constructor.
     */
    PIDPriors()
    {
      for (int iHyp = 0; iHyp < 6; iHyp++)
        m_priors[iHyp] = PIDPriorsTable();
    };


    /**
     * A small function that converts the PDG code into the internal code (i.e. the index in the array of TOPPriorsTable)
     * used to store the prior: e=0, mu=1, pi=2, K=3, p=4, d=5.
     * If the PDG code does not correspond to any of the "stable" particles,
     * -1 is retured
     * @param pdg the PDG code
     * @return  the internal code corresponding to the PDG code
     */
    short parsePDGForPriors(int pdg) const
    {
      pdg = TMath::Abs(pdg);
      if (pdg == 11) return 0;
      else if (pdg == 13) return 1;
      else if (pdg == 211) return 2;
      else if (pdg == 321) return 3;
      else if (pdg == 2212) return 4;
      else if (pdg == 1000010020) return 5;
      else return -1;
    };


    /**
     * Sets the prior table for a particle species from a PIDPriorsTable object
     * @param PDGCode the PDG code of the particle
     * @param table the priors table
     */
    void setPriors(int PDGCode, PIDPriorsTable table)
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
        return;
      }
      m_priors[index] = table;
      return;
    }


    /**
     * Sets the prior table for a particle species from a the std::vectors of the bin edges and the probability values
     * @param PDGCode the PDG code of the particle
     * @param xAxisEdge the std::vector<float> containing the bin edges of the X axis
     * @param yAxisEdge the std::vector<float> containing the bin edges of the Y axis
     * @param priorsTable the 2D std::vector<float> containing the prior probabilities
     * @param errorsTable the 2D std::vector<float> containing the errors on prior probabilities
     */
    void setPriors(int PDGCode, std::vector<float> xAxisEdges, std::vector<float> yAxisEdges,
                   std::vector<std::vector<float>> priorsTable, std::vector<std::vector<float>> errorsTable)
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
        return;
      }
      m_priors[index].setBinEdges(xAxisEdges, yAxisEdges);
      m_priors[index].setBinEdges(xAxisEdges, yAxisEdges);
      m_priors[index].setPriorsTable(priorsTable);
      m_priors[index].setErrorsTable(errorsTable);
      return;
    }


    /**
     * Sets the prior table for a particle species from a TH2F.
     * The bin edges are taken form the TH2 axes, the prior probability from the bins content and the errors from the bins error
     * @param PDGCode the PDG code of the particle
     * @param priorHistogram the prior for the particle specie
     */
    void setPriors(int PDGCode, TH2F* priorHistogram);


    /**
     * Sets the prior table for a particle species starting from a TH2F of the counts and a TH2F of the normalization.
     * The counts histogram is divided by the normalization one to get the probabilities and the errors
     * The bin edges are taken form the TH2 axes, the prior probability from the bins content and the errors from the bins error
     * @param PDGCode the PDG code of the particle
     * @param priorHistogram the prior for the particle species
     */
    void setPriors(int PDGCode, TH2F* counts, TH2F* normalization);


    /**
     * Sets the axes for the priors table of the selected species.
     * Effectively this is just a wrapper around PIDPriorsTable::setBinEdges()
     * @param PDGCode the PDG code of the prior's species
     * @param xAxisEdge the vector of edges of the X axis
     * @param yAxisEdge the vector of edges of the Y axis
     */
    void setPriorsAxes(int PDGCode, std::vector<float> xAxisEdges, std::vector<float> yAxisEdges)
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
        return;
      }
      m_priors[index].setBinEdges(xAxisEdges, yAxisEdges);
      m_priors[index].setBinEdges(xAxisEdges, yAxisEdges);
      return;
    }


    /**
     * Sets the probability table for the priors of the selected particle species.
     * Effectively this is just a wrapper around PIDPriorsTable::setPriorsTable()
     * @param PDGCode the PDG code of the prior's species
     * @param priorsTable a 2D vector containing the prior probabilities
     */
    void setPriorsTable(int PDGCode, std::vector<std::vector<float>> priorsTable)
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
        return;
      }
      m_priors[index].setPriorsTable(priorsTable);
      return;
    }


    /**
     * Sets the probability error table for the priors of the selected particle species.
     * Effectively this is just a wrapper around PIDPriorsTable::setErrorssTable()
     * @param PDGCode the PDG code of the prior's species
     * @param errorsTable a 2D vector containing the prior probabilities
     */
    void setErrorsTable(int PDGCode, std::vector<std::vector<float>>errorsTable)
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
        return;
      }
      m_priors[index].setErrorsTable(errorsTable);
      return;
    }


    /**
     * Sets the axis labels for the priors of the selected particle species.
     * @param PDGCode the PDG code of the prior's species
     * @param xAxisLabel the label of the X axis
     * @param yAxisLabel the label of the Y axis
     */
    void setAxisLabels(int PDGCode, std::string xAxisLabel, std::string yAxisLabel)
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
        return;
      }
      m_priors[index].setAxisLabels(xAxisLabel, yAxisLabel);
      return;
    }


    /**
     * Returns the priors table of the selected particle species.
     * @param PDGCode the PDG code of the prior's species
     * @return the priors table
     */
    PIDPriorsTable getPriorsTable(int PDGCode) const
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
      }
      return m_priors[index];
    }


    /**
     * Returns the prior probability associated to a particle with defined species and parameters.
     * @param PDGCode the PDG code of the prior's species
     * @param x the value of the x-axis coordinate
     * @param y the value of the y-axis coordinate
     * @return the prior probability value
     */
    float getPriorValue(int PDGCode, float x, float y) const
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
        return NAN;
      }
      return m_priors[index].getPriorValue(x, y);
    }


    /**
     * Returns the error on the prior probability associated to a particle with defined species and parameters.
     * @param PDGCode the PDG code of the prior's species
     * @param x the value of the x-axis coordinate
     * @param y the value of the y-axis coordinate
     * @return the prior probability error
     */
    float getPriorError(int PDGCode, float x, float y) const
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
        return NAN;
      }
      return m_priors[index].getErrorValue(x, y);
    }


    /**
     * Returns the X axis label of the prior
     * @param PDGCode the PDG code of the prior's species
     * @return the X axis label for the prior tabel of the selected particle species
     */
    std::string getXAxisLabel(int PDGCode)
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
      }
      return m_priors[index].getXAxisLabel();
    }


    /**
     * Returns the Y axis label of the prior
     * @param PDGCode the PDG code of the prior's species
     * @return the Y axis label for the prior tabel of the selected particle species
     */
    std::string getYAxisLabel(int PDGCode)
    {
      auto index = parsePDGForPriors(PDGCode);
      if (index < 0) {
        B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
      }
      return m_priors[index].getYAxisLabel();
    }

  private:
    PIDPriorsTable m_priors[6]; /**< The array of PIDPiorsTable, one per particle species. */
  };

} // Belle2 namespace


