/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann (marcel.hohmann@unimelb.edu.au)           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/gearbox/Unit.h>
#include <framework/database/DBObjPtr.h>

//MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <analysis/dataobjects/Particle.h> // could be avoided by jsut recalculating the gamma 4 vector from the clsuter
//ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/modules/eclShowerShape/ECLShowerShapeModule.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLGeometryPar.h>

//Root
#include "TFile.h"
#include "TTree.h"



namespace Belle2 {

  class ECLChargedPIDMVAExpertModule : public Module {

  public:

    /**
     * Constructor, for setting module description and parameters.
     */
    ECLChargedPIDMVAExpertModule();

    /**
     * Destructor, use to clean up anything you created in the constructor.
     */
    virtual ~ECLChargedPIDMVAExpertModule();

    /**
     * Use this to initialize resources or memory your module needs.
     *
     * Also register any outputs of your module (StoreArrays, RelationArrays,
     * StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize() override;

    /**
     * Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun() override;

    /**
     * Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

    /**
     * Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun() override;

    /**
     * Clean up anything you created in initialize().
     */
    virtual void terminate() override;

  private:

    /**
     * Array of Track objects.
     */
    StoreArray<Track> m_tracks;

    /** filename for the root  file */
    std::string m_filename;
    int m_pdg_code;
    bool m_keep_only_truth_matched;

    /**output file for training data */
    TFile* m_TFile;

    /**training data tree */
    TTree* m_TTree;


    // MVA Variables
    Float_t m_E1E9;
    Float_t m_E9E21;
    Float_t m_S2;
    Float_t m_E;
    Float_t m_EoP;
    Float_t m_EoPBremsCorrected;
    Float_t m_Z40;
    Float_t m_Z51;
    Float_t m_ZMVA;
    Float_t m_PSDMVA;
    Float_t m_DeltaL;
    Float_t m_LAT;

    Float_t m_p;
    Float_t m_pBremsCorrected;
    Float_t m_clusterTheta;
    Float_t m_theta;
    Int_t m_charge;
    Int_t m_mcPDG;
    Int_t m_mcMotherPDG;

    Float_t m_Z00;
    Float_t m_Z10;
    Float_t m_Z11;
    Float_t m_Z20;
    Float_t m_Z21;
    Float_t m_Z22;
    Float_t m_Z30;
    Float_t m_Z31;
    Float_t m_Z32;
    Float_t m_Z33;
    //Float_t m_Z40;
    Float_t m_Z41;
    Float_t m_Z42;
    Float_t m_Z43;
    Float_t m_Z44;
    Float_t m_Z50;
    //Float_t m_Z51;
    Float_t m_Z52;
    Float_t m_Z53;
    Float_t m_Z54;
    Float_t m_Z55;
    Float_t m_Z60;
    Float_t m_Z61;
    Float_t m_Z62;
    Float_t m_Z63;
    Float_t m_Z64;
    Float_t m_Z65;
    Float_t m_Z66;

    Float_t m_PSD0HadronEnergy;
    Float_t m_PSD0OnlineEnergy;
    Float_t m_PSD0HadronEnergyFraction;
    Float_t m_PSD0DigitWeight;
    Float_t m_PSD0DigitFitType;
    Float_t m_PSD0R;
    Float_t m_PSD0CosTheta;
    Float_t m_PSD0Phi;

    Float_t m_PSD1HadronEnergy;
    Float_t m_PSD1OnlineEnergy;
    Float_t m_PSD1HadronEnergyFraction;
    Float_t m_PSD1DigitWeight;
    Float_t m_PSD1DigitFitType;
    Float_t m_PSD1R;
    Float_t m_PSD1CosTheta;
    Float_t m_PSD1Phi;

    Float_t m_PSD2HadronEnergy;
    Float_t m_PSD2OnlineEnergy;
    Float_t m_PSD2HadronEnergyFraction;
    Float_t m_PSD2DigitWeight;
    Float_t m_PSD2DigitFitType;
    Float_t m_PSD2R;
    Float_t m_PSD2CosTheta;
    Float_t m_PSD2Phi;

    Float_t m_PSD3HadronEnergy;
    Float_t m_PSD3OnlineEnergy;
    Float_t m_PSD3HadronEnergyFraction;
    Float_t m_PSD3DigitWeight;
    Float_t m_PSD3DigitFitType;
    Float_t m_PSD3R;
    Float_t m_PSD3CosTheta;
    Float_t m_PSD3Phi;

    Float_t m_PSD4HadronEnergy;
    Float_t m_PSD4OnlineEnergy;
    Float_t m_PSD4HadronEnergyFraction;
    Float_t m_PSD4DigitWeight;
    Float_t m_PSD4DigitFitType;
    Float_t m_PSD4R;
    Float_t m_PSD4CosTheta;
    Float_t m_PSD4Phi;

    Float_t m_PSD5HadronEnergy;
    Float_t m_PSD5OnlineEnergy;
    Float_t m_PSD5HadronEnergyFraction;
    Float_t m_PSD5DigitWeight;
    Float_t m_PSD5DigitFitType;
    Float_t m_PSD5R;
    Float_t m_PSD5CosTheta;
    Float_t m_PSD5Phi;

    Float_t m_PSD6HadronEnergy;
    Float_t m_PSD6OnlineEnergy;
    Float_t m_PSD6HadronEnergyFraction;
    Float_t m_PSD6DigitWeight;
    Float_t m_PSD6DigitFitType;
    Float_t m_PSD6R;
    Float_t m_PSD6CosTheta;
    Float_t m_PSD6Phi;

    Float_t m_PSD7HadronEnergy;
    Float_t m_PSD7OnlineEnergy;
    Float_t m_PSD7HadronEnergyFraction;
    Float_t m_PSD7DigitWeight;
    Float_t m_PSD7DigitFitType;
    Float_t m_PSD7R;
    Float_t m_PSD7CosTheta;
    Float_t m_PSD7Phi;

    Float_t m_PSD8HadronEnergy;
    Float_t m_PSD8OnlineEnergy;
    Float_t m_PSD8HadronEnergyFraction;
    Float_t m_PSD8DigitWeight;
    Float_t m_PSD8DigitFitType;
    Float_t m_PSD8R;
    Float_t m_PSD8CosTheta;
    Float_t m_PSD8Phi;

    Float_t m_PSD9HadronEnergy;
    Float_t m_PSD9OnlineEnergy;
    Float_t m_PSD9HadronEnergyFraction;
    Float_t m_PSD9DigitWeight;
    Float_t m_PSD9DigitFitType;
    Float_t m_PSD9R;
    Float_t m_PSD9CosTheta;
    Float_t m_PSD9Phi;

    Float_t m_PSD10HadronEnergy;
    Float_t m_PSD10OnlineEnergy;
    Float_t m_PSD10HadronEnergyFraction;
    Float_t m_PSD10DigitWeight;
    Float_t m_PSD10DigitFitType;
    Float_t m_PSD10R;
    Float_t m_PSD10CosTheta;
    Float_t m_PSD10Phi;

    Float_t m_PSD11HadronEnergy;
    Float_t m_PSD11OnlineEnergy;
    Float_t m_PSD11HadronEnergyFraction;
    Float_t m_PSD11DigitWeight;
    Float_t m_PSD11DigitFitType;
    Float_t m_PSD11R;
    Float_t m_PSD11CosTheta;
    Float_t m_PSD11Phi;

    Float_t m_PSD12HadronEnergy;
    Float_t m_PSD12OnlineEnergy;
    Float_t m_PSD12HadronEnergyFraction;
    Float_t m_PSD12DigitWeight;
    Float_t m_PSD12DigitFitType;
    Float_t m_PSD12R;
    Float_t m_PSD12CosTheta;
    Float_t m_PSD12Phi;

    Float_t m_PSD13HadronEnergy;
    Float_t m_PSD13OnlineEnergy;
    Float_t m_PSD13HadronEnergyFraction;
    Float_t m_PSD13DigitWeight;
    Float_t m_PSD13DigitFitType;
    Float_t m_PSD13R;
    Float_t m_PSD13CosTheta;
    Float_t m_PSD13Phi;

    Float_t m_PSD14HadronEnergy;
    Float_t m_PSD14OnlineEnergy;
    Float_t m_PSD14HadronEnergyFraction;
    Float_t m_PSD14DigitWeight;
    Float_t m_PSD14DigitFitType;
    Float_t m_PSD14R;
    Float_t m_PSD14CosTheta;
    Float_t m_PSD14Phi;

    Float_t m_PSD15HadronEnergy;
    Float_t m_PSD15OnlineEnergy;
    Float_t m_PSD15HadronEnergyFraction;
    Float_t m_PSD15DigitWeight;
    Float_t m_PSD15DigitFitType;
    Float_t m_PSD15R;
    Float_t m_PSD15CosTheta;
    Float_t m_PSD15Phi;

    Float_t m_PSD16HadronEnergy;
    Float_t m_PSD16OnlineEnergy;
    Float_t m_PSD16HadronEnergyFraction;
    Float_t m_PSD16DigitWeight;
    Float_t m_PSD16DigitFitType;
    Float_t m_PSD16R;
    Float_t m_PSD16CosTheta;
    Float_t m_PSD16Phi;

    Float_t m_PSD17HadronEnergy;
    Float_t m_PSD17OnlineEnergy;
    Float_t m_PSD17HadronEnergyFraction;
    Float_t m_PSD17DigitWeight;
    Float_t m_PSD17DigitFitType;
    Float_t m_PSD17R;
    Float_t m_PSD17CosTheta;
    Float_t m_PSD17Phi;

    Float_t m_PSD18HadronEnergy;
    Float_t m_PSD18OnlineEnergy;
    Float_t m_PSD18HadronEnergyFraction;
    Float_t m_PSD18DigitWeight;
    Float_t m_PSD18DigitFitType;
    Float_t m_PSD18R;
    Float_t m_PSD18CosTheta;
    Float_t m_PSD18Phi;

    Float_t m_PSD19HadronEnergy;
    Float_t m_PSD19OnlineEnergy;
    Float_t m_PSD19HadronEnergyFraction;
    Float_t m_PSD19DigitWeight;
    Float_t m_PSD19DigitFitType;
    Float_t m_PSD19R;
    Float_t m_PSD19CosTheta;
    Float_t m_PSD19Phi;

    Float_t m_logL_SVD_electron;
    Float_t m_logL_SVD_muon;
    Float_t m_logL_SVD_pion;
    Float_t m_logL_SVD_proton;
    Float_t m_logL_SVD_deuteron;
    Float_t m_logL_SVD_kaon;

    Float_t m_logL_ECL_electron;
    Float_t m_logL_ECL_muon;
    Float_t m_logL_ECL_pion;
    Float_t m_logL_ECL_proton;
    Float_t m_logL_ECL_deuteron;
    Float_t m_logL_ECL_kaon;

    Float_t m_logL_CDC_electron;
    Float_t m_logL_CDC_muon;
    Float_t m_logL_CDC_pion;
    Float_t m_logL_CDC_proton;
    Float_t m_logL_CDC_deuteron;
    Float_t m_logL_CDC_kaon;

    Float_t m_logL_TOP_electron;
    Float_t m_logL_TOP_muon;
    Float_t m_logL_TOP_pion;
    Float_t m_logL_TOP_proton;
    Float_t m_logL_TOP_deuteron;
    Float_t m_logL_TOP_kaon;

    Float_t m_logL_ARICH_electron;
    Float_t m_logL_ARICH_muon;
    Float_t m_logL_ARICH_pion;
    Float_t m_logL_ARICH_proton;
    Float_t m_logL_ARICH_deuteron;
    Float_t m_logL_ARICH_kaon;

    Float_t m_logL_KLM_electron;
    Float_t m_logL_KLM_muon;
    Float_t m_logL_KLM_pion;
    Float_t m_logL_KLM_proton;
    Float_t m_logL_KLM_deuteron;
    Float_t m_logL_KLM_kaon;

    /**
     * The event information.
     */
    StoreObjPtr<EventMetaData> m_eventMetaData;

    // copy pasted frankenstein's monster
  private:
    /**Struct used to hold information of the digits projected to a plane perpendicular to the shower direction */
    struct ProjectedECLDigit {

      /** weighted energy */
      double energy;

      /** radial distance */
      double rho;

      /** polar angel */
      double alpha;
    };


    /** Compute the absolute value of the complex Zernike moment Znm.
        The moments are computed in a plane perpendicular to the direction of the shower.
        The plane's origin is at the intersection of the shower direction with the plane.
        The origin is at a distance from the interaction point equal to the shower distance from the interaction point.

        n, m - are the Zernike polynomial rank
        R0 - is a scaling factor used to normalize the distances in the described plane.
        It also sets the maximum distance from the origin (the Zernike polynomials are defined only on the unit circle).
        All points in the plane with a distance larger than R0 from the origin are ignored.

        Valid values of n,m are n,m >= 0, m <= n.
        If n or m are invalid the function returns 0.0.
        */

    double computeAbsZernikeMoment(const std::vector<ProjectedECLDigit>& projectedDigits, const double totalEnergy, const int n,
                                   const int m,
                                   const double rho) const;

    /** Compute projections of the ECLCalDigits to the perpendicular plane */
    std::vector<ProjectedECLDigit> projectECLDigits(const ECLShower& shower) const;

    /** The radial part of the Zernike polynomial
     * n,m - Zernike polynomial rank
     * rho - radial distance             */
    double Rnm(const int n, const int m, const double rho) const;

    /** Return the complex value of the Zernike polynomial of rank n,m.
    Znm(rho,alpha) = Rnm(rho) * exp(i*m*alpha)
    rho - radial distance
    alpha - polar angle */
    std::complex<double> zernikeValue(const double rho, const double alpha, const int n, const int m) const;

  };

} //Belle2



