/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//STL
#include <vector>

//Framework
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>


class TH1F;
class TH2F;

namespace Belle2 {

  class ECLSimHit;
  class MCParticle;
  class ECLShower;
  class BeamBackHit;
  class ECLCrystalData;

#ifdef DOARICH
  class ARICHGeometryPar;
#endif

  /** A module to study background campaigns and produce histograms */
  class ECLBackgroundModule : public HistoModule {

  public:

    /** Constructor.*/
    ECLBackgroundModule();

    /** Destructor.*/
    virtual ~ECLBackgroundModule();

    /** Initialize variables. */
    virtual void initialize() override;

    /** beginRun */
    virtual void beginRun() override;

    /** Event method  */
    virtual void event() override;

    /** endRun */
    virtual void endRun() override;

    /** terminate */
    virtual void terminate() override;

    /** Initalize the histograms*/
    virtual void defineHisto() override;

  private:

    /** Store array: ECLSimHit. */
    StoreArray<ECLSimHit> m_eclArray;

    /** Store array: MCParticle. */
    StoreArray<MCParticle> m_mcParticles;

    /** Store array: BeamBackHit. */
    StoreArray<BeamBackHit> m_BeamBackArray;

    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowerArray;

    /** length of sample in us */
    int m_sampleTime;

    /** Whether or not the ARICH plots are produced */
    bool m_doARICH;

    /** Cell ID of crystal(s) of interest */
    std::vector<int> m_CryInt;

    /** Event counter */
    int m_nEvent{0};

    /** ECL Sim Hits */
    TH1F* h_nECLSimHits{nullptr};

    /** Crystal Radiation Dose, actual Theta*/
    TH1F* h_CrystalRadDoseTheta{nullptr};
    /** Crystal Radiation Dose*/
    TH1F* h_CrystalRadDose{nullptr};
    /** Crystal Radiation Dose, ThetaID=2*/
    TH1F* h_CrystalThetaID2{nullptr};
    /** Crystal Radiation Dose, ThetaID=67*/
    TH1F* h_CrystalThetaID67{nullptr};
    /** Hit locations*/
    TH2F* h_HitLocations{nullptr};
    /** Crystal Radiation Dose in Barrel, 12<thetaID<59*/
    TH1F* h_BarrelDose{nullptr};

    /** Energy averaged per ring */
    TH1F* hEdepPerRing{nullptr};

    /** Event counter averaged per ring (theta-id)*/
    TH1F* hNevtPerRing{nullptr};



    /**Diode Radiation Dose */
    TH1F* h_DiodeRadDose{nullptr};
    /** Neutron Flux in Diodes*/
    TH1F* h_NeutronFlux{nullptr};
    /** Neutron flux in Diodes, ThetaID=2*/
    TH1F* h_NeutronFluxThetaID2{nullptr};
    /** Neutron flux in Diodes, ThetaID=67*/
    TH1F* h_NeutronFluxThetaID67{nullptr};
    /** Neutron Energy*/
    TH1F* h_NeutronE{nullptr};
    /** Neutron Energy, First Crystal*/
    TH1F* h_NeutronEThetaID0{nullptr};
    /** Photon Energy */
    TH1F* h_PhotonE{nullptr};


    /** Shower Energy distribution vs theta*/
    TH2F* h_ShowerVsTheta{nullptr};
    /** Shower Energy distribution*/
    TH1F* h_Shower{nullptr};

    /** Production Vertex*/
    TH1F* h_ProdVert{nullptr};
    /** Production Vertex vs thetaID*/
    TH2F* h_ProdVertvsThetaId{nullptr};

    /** us in a year*/
    const double usInYr = 1e13;
    /**Joules in a GeV */
    const double GeVtoJ = 1.6e-10;

    /**  Store crystal geometry and mass data*/
    ECLCrystalData* Crystal[8736] {0};

    /** Populate ARICH HAPD dose and flux histograms (from the BeamBack hits array)**/
    int FillARICHBeamBack(BeamBackHit* aBBHit);
    /** Builds geometry (fill Crystal look-up arrays) **/
    int BuildECL();
    // Transform Histograms //

    /** Create 2D histograms indicating the position of each crystals */
    int SetPosHistos(TH1F* h, TH2F* hFWD, TH2F* hBAR, TH2F* hBWD);
    /** Convert histogram vs crystal index to geometrical positions */
    TH2F* BuildPosHisto(TH1F* h, const char* sub);
    /** Convert histogram vs crystal index to *average* per theta-ID  (wide binning)*/
    TH1F* BuildThetaIDWideHisto(TH1F* h_cry);
    /**Convert histogram vs ARICH channel ID to *average* per ring ID */
    TH1F* BuildARICHringIDHisto(TH1F* h_cell);
    /** Get ARICH ring ID from the module index */
    int ARICHmod2row(int modID);

    //ECL channels
    /** Total number of ECL crystals   */
    static const int nECLCrystalTot = 8736;
    /** Number of Barrel ECL crystals  */
    const int nECLCrystalBAR = 6624;
    /** Number of FWD ECL end-capcrystals */
    const int nECLCrystalECF = 1152;
    /** Number of BWD ECL end-capcrystals */
    const int nECLCrystalECB =  960;
    /** Number of thetaID values */
    static const int nECLThetaID = 69;

    //ECL Pin Diodes
    /** Frontal area [cm*cm] of Diodes */
    const double DiodeArea = 2 * 2;
    /** Thickness [cm] of Diodes */
    const double DiodeThk = 0.1;
    /** Density (silicium) [kg*cm^{-3}] of Si*/
    const double SiRho = 2.33e-3;
    /** Mass [kg] of Diodes */
    const double DiodeMass = DiodeArea * DiodeThk * SiRho;;


    /** ARICH geometry paramaters */
#ifdef DOARICH
    ARICHGeometryPar* m_arichgp {0};   /**< Geometry parameters of ARICH. */
#endif

    //Below are the density for the BOARDS (not the HAPD themselves)
    //The values are Taken from Luka Šantelj's BeamBack_arich.cc

    /**ARICH: Area (cm^2) of the HAPD boards*/
    const double HAPDarea      = 7.5 * 7.5;
    /**ARICH: Thickness (cm) of the HAPD boards*/
    const double HAPDthickness = 0.2;
    /**ARICH:  Mass (kg) of the HAPD boards */
    const double HAPDmass      = 47.25e-3;

    //ARICH channels
    /** ARICH parameter */
    const int nHAPD = 420;
    /** ARICH parameter */
    const int nHAPDrings = 7;
    /** ARICH parameter */
    const int nHAPDperRing[7] = {42, 48, 54, 60, 66, 72, 78};

    // For shield studies

    /** Radiation Dose per cell*/
    TH1F* hEMDose{nullptr};

    /** Energy per cell */
    TH1F* hEnergyPerCrystal{nullptr};

    /**Diode Neutron Flux per cell */
    TH1F* hDiodeFlux{nullptr};

    /**Log Spectrum of the photons hitting the crystals / 1 MeV */
    TH1F* hEgamma{nullptr};
    /**Log Spectrum of the neutrons hitting the diodes / 1 MeV */
    TH1F* hEneu{nullptr};


    /** ARICH Yearly dose (rad) vs module index. Based on energy of all BeamBackgrounds */
    TH1F* hARICHDoseBB{nullptr};
    /** ARICH Yearly neutron flux vs module index. Based on energy of all BeamBackgrounds */
    TH1F* hHAPDFlux{nullptr};

    /**Energy per crystal Forward Calorimeter*/
    TH2F* hEnergyPerCrystalECF{nullptr};
    /**Energy per crystal Backward Calorimeter*/
    TH2F* hEnergyPerCrystalECB{nullptr};
    /**Energy per crystal Barrel*/
    TH2F* hEnergyPerCrystalBAR{nullptr};
    /**Energy per crystal Wide bins*/
    TH1F* hEnergyPerCrystalWideTID{nullptr};


    /**Radiation Dose Forward Calorimeter*/
    TH2F* hEMDoseECF{nullptr};
    /**Radiation Dose Backward Calorimeter*/
    TH2F* hEMDoseECB{nullptr};
    /**Radiation Dose Barrel*/
    TH2F* hEMDoseBAR{nullptr};
    /**Radiation Dose Wide bins*/
    TH1F* hEMDoseWideTID{nullptr};

    /**Diode Neutron Flux Forward Calorimeter*/
    TH2F* hDiodeFluxECF{nullptr};
    /**Diode Neutron Flux Backward Calorimeter*/
    TH2F* hDiodeFluxECB{nullptr};
    /**Diode Neutron Flux Barrel*/
    TH2F* hDiodeFluxBAR{nullptr};
    /**Diode Neutron Flux Wide bins*/
    TH1F* hDiodeFluxWideTID{nullptr};

  };

} // end of Belle2 namespace
