//+
// File : B2BIIMCParticlesMonitorModule.h
// Description : A module to plot MCParticles monitor histograms in basf2
//
// Contributors: Hulya Atmacan
// Date : November 21, 2015
//-

#pragma once

#include <framework/core/HistoModule.h>        // <- Substitution of HistoModule.h
#include "TH1F.h"

namespace Belle2 {

  /** Declaration of class B2BIIMCParticlesMonitor */
  class B2BIIMCParticlesMonitorModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    B2BIIMCParticlesMonitorModule();
    /** Destructor */
    virtual ~B2BIIMCParticlesMonitorModule();

    /** Initialize the module */
    virtual void initialize() override;
    /** Called when a new run is started */
    virtual void beginRun() override;
    /** Called for each event */
    virtual void event() override;
    /** Called when the current run finished */
    virtual void endRun() override;
    /** Terminates the module */
    virtual void terminate() override;

    /** function to define histograms */
    virtual void defineHisto() override;

  private:
    TH1F* mcPDG{nullptr};  /**< histogram PDG values of MC Particles */
    TH1F* mcM{nullptr};    /**< histogram mass */
    TH1F* mcPx{nullptr};   /**< histogram momentum_x */
    TH1F* mcPy{nullptr};   /**< histogram momentum_y */
    TH1F* mcPz{nullptr};   /**< histogram momentum_z */
    TH1F* mcE{nullptr};    /**< histogram energy */
    TH1F* mcVx{nullptr};   /**< histogram vertex_x */
    TH1F* mcVy{nullptr};   /**< histogram vertex_y */
    TH1F* mcVz{nullptr};   /**< histogram vertex_z */
    TH1F* mcPiPlusMother{nullptr};   /**< histogram PDG codes of mother particles of positive pions */
    TH1F* mcPiMinusMother{nullptr};  /**< histogram PDG codes of mother particles of negative pions */
    TH1F* mcPi0Mother{nullptr};      /**< histogram PDG codes of mother particles of pi0's */
    TH1F* mcNDau{nullptr};           /**< histogram number of daughter particles of all particles */

  };
}
