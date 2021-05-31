/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2020 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Umberto Tamponi                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <TTree.h>
#include <TH2F.h>
#include <TFile.h>
#include <analysis/VariableManager/Manager.h>
#include <mdst/dataobjects/Track.h>

namespace Belle2 {
  /**
   * A module to plot the x-t images from TOP, and in general study the distribution of the digits associated to the particles in a particleList.
   * Starting from a particleList, it produces a tree containing any variable of choice available in the variableManager (ancing in the same way as the variablesToNtuple module),
   * plus branches containing the caracteristics of the TOPDigits in the slot where the particle is extrapolated and the histograms of the expected PDF (pixelcol VS channel)
   */
  class TOPRingPlotterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TOPRingPlotterModule();

    /**  Prepares the tree */
    void initialize() override;


    /** Fills the tree */
    void event() override;


    /**  Writes the tree */
    void terminate() override;


  private:

    /** Fills the pdf-related branches */
    void fillPDF(Belle2::Const::ChargedStable, const Track*, TH2F*, short*, float*,  int&, int&);

    /** reset the tree variables. Call it at the beginning of the event method! */
    void resetTree();


    static const int m_MaxPhotons = 5000; /**< maximum number of digits allowed per PDF */
    static const int m_MaxPDFPhotons = 5000; /**< maximum number of digits allowed per PDF */

    // steering parameters
    std::string m_particleList = "pi+:all";  /**< List of particles to be used for plotting */
    std::string m_outputName = "TOPRings.root";  /**< Name of the output file */
    int m_toyNumber = 1; /**< Number of toys used to populate the arrays of expected hits*/
    bool m_saveHistograms = false;  /**< Set true to save the histograms of the maps */

    TFile* m_outputFile = nullptr;   /**< output file */
    TTree* m_tree = nullptr;   /**< tree where data are saved. One entry per particle in m_particleList */

    /** Variable branch addresses */
    std::vector<double> m_branchAddresses = {};
    /** List of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions = {};
    /** List of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::vector<std::string> m_variables = {};

    std::vector<short> m_pdgHyp = {11, 13, 211, 321, 2212}; /**< List of pdg codes for which the PDF is sampled ansd saved. */

    float m_digitTime[m_MaxPhotons] = {0}; /**< Digit calibrated time [ns] */
    short m_digitChannel[m_MaxPhotons] = {0}; /**< SW channel (0-511) */
    short m_digitPixel[m_MaxPhotons] = {0}; /**< Pixel number (1-512)*/
    short m_digitPixelCol[m_MaxPhotons] = {0}; /**< Pixel column */
    short m_digitPixelRow[m_MaxPhotons] = {0}; /**< Pixel row */
    float m_digitAmplitude[m_MaxPhotons] = {0}; /**< Digit amplitude [ADC] */
    float m_digitWidth[m_MaxPhotons] = {0}; /**< Digit calibrated width [ns] */
    short m_digitSlot[m_MaxPhotons] = {0}; /**<Slot number (1-16) */
    short m_digitBoardstack[m_MaxPhotons] = {0}; /**< BoardStack number  */
    short m_digitCarrier[m_MaxPhotons] = {0}; /**< Carrier number  */
    short m_digitAsic[m_MaxPhotons] = {0}; /**< Asic number  */
    short m_digitQuality[m_MaxPhotons] = {0}; /**< Digit quality  */
    short m_digitASICChannel[m_MaxPhotons] = {0}; /**< ASIC channel number */
    short m_digitPMTNumber[m_MaxPhotons] = {0}; /**< Digit PMT number */
    short m_nDigits = 0; /**< Total number of digits in the slot where the track is extrapolated */

    short m_pdfPixelE[m_MaxPDFPhotons] = {0}; /**< array with the pixel location of the sampled pseudo-hits for the electron PDF*/
    float m_pdfTimeE[m_MaxPDFPhotons] = {0}; /**< array with the times of the sampled pseudo-hits for the electron PDF*/
    short m_pdfPixelMU[m_MaxPDFPhotons] = {0}; /**< array with the pixel location of the sampled pseudo-hits for the muon PDF*/
    float m_pdfTimeMU[m_MaxPDFPhotons] = {0}; /**< array with the times of the sampled pseudo-hits for the muon PDF*/
    short m_pdfPixelPI[m_MaxPDFPhotons] = {0}; /**< array with the pixel location of the sampled pseudo-hits for the pion PDF*/
    float m_pdfTimePI[m_MaxPDFPhotons] = {0}; /**< array with the times of the sampled pseudo-hits for the pion PDF*/
    short m_pdfPixelK[m_MaxPDFPhotons] = {0}; /**< array with the pixel location of the sampled pseudo-hits for the kaon PDF*/
    float m_pdfTimeK[m_MaxPDFPhotons] = {0}; /**< array with the times of the sampled pseudo-hits for the kaon PDF*/
    short m_pdfPixelP[m_MaxPDFPhotons] = {0}; /**< array with the pixel location of the sampled pseudo-hits for the proton PDF*/
    float m_pdfTimeP[m_MaxPDFPhotons] = {0}; /**< array with the times of the sampled pseudo-hits for the proton PDF*/
    int m_pdfSamplesP = 0; /**< total number of samples drawn from the proton PDF */
    int m_pdfToysP = 0; /**< total number of toys from the proton PDF */
    int m_pdfSamplesK = 0; /**< total number of samples drawn from the kaon PDF */
    int m_pdfToysK = 0; /**< total number of toys from the kaon PDF */
    int m_pdfSamplesPI = 0; /**< total number of samples drawn from the pion PDF */
    int m_pdfToysPI = 0; /**< total number of toys from the pion PDF */
    int m_pdfSamplesMU = 0; /**< total number of samples drawn from the muon PDF */
    int m_pdfToysMU = 0; /**< total number of toys from the muon PDF */
    int m_pdfSamplesE = 0; /**< total number of samples drawn from the electron PDF */
    int m_pdfToysE = 0; /**< total number of toys from the electron PDF */

    TH2F* m_hitMapMCK = nullptr; /**< x-t plot of the kaon PDF*/
    TH2F* m_hitMapMCPI =  nullptr; /**< x-t plot of the pion PDF*/
    TH2F* m_hitMapMCP =  nullptr; /**< x-t plot of the proton PDF*/
    TH2F* m_hitMapMCE =  nullptr; /**< x-t plot of the electron PDF*/
    TH2F* m_hitMapMCMU =  nullptr; /**< x-t plot of the muon PDF*/

    TH2F* m_pdfAsHisto =  nullptr; /**< histogram to hot the PDF that will be sampled */

  };
}
