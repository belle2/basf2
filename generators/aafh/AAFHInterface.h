/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef GENERATORS_AAFH_INTERFACE_H
#define GENERATORS_AAFH_INTERFACE_H

#include <vector>
#include <mdst/dataobjects/MCParticleGraph.h>


namespace Belle2 {

  /** Class to inferface AAFH/DIAG36 Generator written in Fortran.
   * Almost all parameters can be set using this class. First all parameters
   * should be set, then call initialize(), an arbitary number of
   * generateEvent() and then finish(). After finish(), the total cross section
   * can be obained using getTotalCrossSection()
   */
  class AAFHInterface {
  public:
    /** Generation mode */
    enum EMode {
      /** E+E- -> Mu+Mu-L+L- where L can be anything, defaults to tau */
      c_MuonParticle = 1,
      /** E+E- -> Mu+Mu-Mu+Mu- */
      c_MuonMuon = 2,
      /** E+E- -> E+E-Mu+Mu- */
      c_ElectonMuon = 3,
      /** E+E- -> E+E-L+L- where L can be anything, defaults to tau */
      c_ElectronParticle = 4,
      /** E+E- -> E+E-E+E- */
      c_ElectonElectron = 5,
    };

    /** Rejection mode */
    enum ERejection {
      /** Apply rejection only once for the final event */
      c_Once = 1,
      /** Apply rejection first for each sub generator and then for the final
       * event */
      c_Twice = 2,
    };

    /** Default Constructor */
    AAFHInterface() = default;
    /** Set the maximum number of tries to generate an event */
    void setMaxTries(int tries);
    /** Set the relative weights for the sub generators */
    void setGeneratorWeights(const std::vector<double>& weights);
    /** Set the maximum expected weights for the rejection method. If the
     * weights are to large generation will be ineffective, if they are to
     * small it will generate Error messages */
    void setMaxWeights(double subgeneratorWeight, double finalWeight);
    /** Set the suppression limits when calculation the matrix elements */
    void setSupressionLimits(std::vector<double> limits);
    /** Set the minimum invariant mass for the generated event */
    void setMinimumMass(double minMass) { m_minMass = minMass; }
    /** Set the particle type for modes c_MuonParticle and c_ElectronParticle */
    void setParticle(const std::string& particle)
    {
      m_particle = particle;
    }

    /** Get the maximum number of tries to generate an event */
    int getMaxTries() const;
    /** Get the relative weights for the sub generators */
    std::vector<double> getGeneratorWeights() const;
    /** Get the maximum expected final weight for the rejection method */
    double getMaxSubGeneratorWeight() const;
    /** Get the maximum expected subgenerator weight for the rejection method */
    double getMaxFinalWeight() const;
    /** Get suppression limits */
    std::vector<double> getSuppressionLimits() const;
    /** Get the minimum invariant mass for the generated event */
    double getMinimumMass() const { return m_minMass; }
    /** Get the particle type for modes c_MuonParticle and c_ElectronParticle */
    std::string getParticle() const { return m_particle; }
    /** Return total cross section */
    double getTotalCrossSection() const;
    /** Return error on the total cross section */
    double getTotalCrossSectionError() const;

    /** initialize the generator */
    void initialize(double beamEnergy, EMode mode, ERejection rejection);
    /** generate one event and add it to the graph in CMS */
    void generateEvent(MCParticleGraph& mpg);
    /** calculate total cross section */
    void finish();
  private:
    /** update particle with generated values
     * @param p MCParticle to be updated
     * @param q 4-Vector + mass from the generator
     * @param pdg pdgcode of the particle
     * @param isInitial incoming beam particles
     */
    void updateParticle(MCParticleGraph::GraphParticle& p, double* q, int pdg, bool isInitial = false);
    /** minimum invariant mass */
    double m_minMass {0};
    /** name of the particle for modes c_MuonParticle and c_ElectronParticle */
    std::string m_particle {"tau-"};
    /** pdg of the particle for modes c_MuonParticle and c_ElectronParticle,
     * gets set in initialize() */
    int m_particlePDG {0};
  };

} //Belle2 namespace
#endif // GENERATORS_AAFH_INTERFACE_H
