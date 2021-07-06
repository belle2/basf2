/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include <TGraph.h>


namespace Belle2 {
  //! TOP backgound module.
  /*
   * This module analyses the data for beam backgound studies
   */
  class TOPBackgroundModule : public Module {

  public:


    //! Constructor.
    TOPBackgroundModule();

    //! Destructor.
    virtual ~TOPBackgroundModule();

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     *
     * Convert TOPSimHits of the event to TOPHits.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

    /**
     *Prints module parameters.
     */
    void printModuleParams() const;

    /**
     *Print histogram 1D, helper function.
     */
    void myprint(TH1F* histo, const char* path, const char* xtit, const char* ytit, double tresh);



  private:

    std::string  m_BkgType;         /**< Type of background */
    std::string  m_OutputFileName;  /**< Output filename */
    double m_TimeOfSimulation;      /**< Time of the simulation of the input file */

    TFile* m_rootFile;  /**< root file for saving histograms */
    TTree* origingamma;  /**< tree for saving origingamma points */
    TTree* originpe;  /**< tree for saving originpe points */

    TH1F* peflux;      /**< pe flux */
    TH1F* nflux;       /**< neutron flux */
    TH1F* rdose;       /**<  radiation dose*/
    TH1F* zdist;       /**<  z distribution of primaries*/
    TH1F* genergy;     /**< energy of gamma that hits the bar */
    TH1F* genergy2;    /**< energy of gamma that hits the bar */

    TH1F* zdistg;      /**< z distribution of the photoelectron flux */
    TH1F* originpt;    /**< pt of electrons*/

    TH2F* nflux_bar;   /**< neutron flux on bar */
    TH2F* gflux_bar;   /**< gamma flux on bar */
    TH2F* cflux_bar;   /**< charged flux on bar */

    TH1F* norigin;     /**< nutron origin */
    TH1F* gorigin;     /**< gamma origin */
    TH1F* corigin;     /**< charged origin */

    TH1F* nprim;       /**< neutron primery */
    TH1F* gprim;       /**< gamma primery */
    TH1F* cprim;       /**< charged primary */

    TGraph* origin_zx;    /**<  grapch zx */
    TGraph* origin_zy;    /**<  graph zy */

    TGraph* prim_zx;     /**<  grapch zx */
    TGraph* prim_zy;     /**<  graph zy */
    TGraph* module_occupancy;     /**<  graph zy */

    double PCBmass;      /**<  constant */
    double PCBarea;      /**<  constant */
    double yearns;       /**<  constant */
    double evtoJ;        /**<  constant */
    double mtoc;         /**<  constant */
    int count;           /**<  counter */
    int count_occ;           /**<  counter */
    double origingamma_x;  /**< points from origin_zx and _zy graphs: x */
    double origingamma_y;  /**< points from origin_zx and _zy graphs: y */
    double origingamma_z;  /**< points from origin_zx and _zy graphs: z */
    double originpe_x;     /**< points for origin of mother of photoelectrons: x*/
    double originpe_y;     /**< points for origin of mother of photoelectrons: y*/
    double originpe_z;     /**< points for origin of mother of photoelectrons: z (=zdist)*/
  };

} // Belle2 namespace
