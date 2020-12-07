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
#include <top/reconstruction/TOPreco.h>

using namespace Belle2::Variable;

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
    void fillPDF(TH2F* histo,  Belle2::TOP::TOPreco reco);

    std::string m_particleList = "pi+:all";  /**< List of particles to be used for plotting */
    std::string m_outputName = "TOPRings.root";  /**< Name of the output file */
    int m_toyNumber = 1;  /**< Number of toys used to fill the PDF maps */
    bool m_saveHistograms = false;  /**< Set true to save the histograms fo the maps */
    bool m_saveDigitTree = true;  /**< Set true to Save the branch with the digits */

    TFile* m_outputFile = nullptr;   /**< output file */
    TTree* m_tree = nullptr;   /**< tree where data are saved. One entry per particle in m_particleList */

    short m_maxDigits =  5000;  /**< Maximum number of digit in the slot where the particles is extrapolated*/

    /** Variable branch addresses */
    std::vector<double> m_branchAddresses;
    /** List of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;
    /** List of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::vector<std::string> m_variables;

    float m_digitTime[5000] = {0}; /**< Digit calibrated time [ns] */
    short m_digitChannel[5000] = {0}; /**< SW channel (0-511) */
    short m_digitPixelCol[5000] = {0}; /**< Pixel column */
    short m_digitPixelRow[5000] = {0}; /**< Pixel row */
    float m_digitAmplitude[5000] = {0}; /**< Digit amplitude [ADC] */
    float m_digitWidth[5000] = {0}; /**< Digit calibrated width [ns] */
    short m_digitASICChannel[5000] = {0}; /**< ASIC channel number (0-7) */
    short m_digitPMTNumber[5000] = {0}; /**< Digit PMT number */
    short m_nDigits = 0; /**< Total number of digits in the slot where the track is extrapolated */



    TH2F* m_hitMapMCK = new TH2F("hitMapMCK", "hitMapMCK", 64, 0, 64, 500, 0, 50); /**< x-t plot of the kaon PDF*/
    TH2F* m_hitMapMCPi = new TH2F("hitMapMCPi", "hitMapMCPi", 64, 0, 64, 500, 0, 50); /**< x-t plot of the pion PDF*/
    TH2F* m_hitMapMCP = new TH2F("hitMapMCP", "hitMapMCP", 64, 0, 64, 500, 0, 50); /**< x-t plot of the proton PDF*/
    TH2F* m_hitMapMCE = new TH2F("hitMapMCE", "hitMapMCE", 64, 0, 64, 500, 0, 50); /**< x-t plot of the electron PDF*/
    TH2F* m_hitMapMCMU = new TH2F("hitMapMCMU", "hitMapMCMU", 64, 0, 64, 500, 0, 50); /**< x-t plot of the muon PDF*/



  };
}
