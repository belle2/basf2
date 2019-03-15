//+
// File : B2BIIMCParticlesMonitorModule.h
// Description : A module to plot MCParticles monitor histograms in basf2
//
// Contributors: Hulya Atmacan
// Date : November 21, 2015
//-
#ifndef B2BIIMCPARTICLESMONITORMODULE_H
#define B2BIIMCPARTICLESMONITORMODULE_H

#include <framework/core/HistoModule.h>        // <- Substitution of HistoModule.h
#include <vector>
#include "TH1F.h"

namespace Belle2 {

  class B2BIIMCParticlesMonitorModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /**    Constructor */
    B2BIIMCParticlesMonitorModule();
    /** Destructor */
    virtual ~B2BIIMCParticlesMonitorModule();

    /** Module functions */
    virtual void initialize() override;
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
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
#endif
