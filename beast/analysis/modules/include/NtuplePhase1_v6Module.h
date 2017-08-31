/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - 2016 Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NTUPLEPHASE1_V6MODULE_H
#define NTUPLEPHASE1_V6MODULE_H

#include <framework/core/Module.h>
#include <string>
#include <beast/analysis/modules/BEASTTree_v5.h>
#include "TFile.h"
#include "TChain.h"
#include "Riostream.h"
#include "TF1.h"

namespace Belle2 {

  /**
   * Read SKB PVs, simulated measurements of BEAST sensors, and write scaled simulated Ntuple in BEAST phase 1 data format
   */
  class NtuplePhase1_v6Module : public Module {

  public:

    /**
     * Constructor
     */
    NtuplePhase1_v6Module();

    /**
     * Destructor
     */
    virtual ~NtuplePhase1_v6Module();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    double m_input_Time_eqv;

    std::vector<Int_t> m_input_ts; /**< time stamp start and stop */

    std::vector<std::string> m_inputFileNames; /**< list of file names */

    Double_t m_input_Z_scaling[4]; /**< time stamp start and stop */
    std::vector<Double_t> m_input_Z; /** input Z */
    Int_t m_input_BGSol; /**< time stamp start and stop */
    Int_t m_input_ToSol; /**< time stamp start and stop */
    Int_t m_input_part; /**< which components to take into account */
    std::vector<Double_t> m_input_GasCorrection; /**< time stamp start and stop */

    std::vector<std::string> m_inputRateHistoNames; /**< list of file names */
    std::vector<std::string> m_inputDoseHistoNames; /**< list of file names */

    std::vector<std::string> m_inputRateHistoNamesVrs; /**< list of file names */
    std::vector<std::string> m_inputDoseHistoNamesVrs; /**< list of file names */

    std::vector<Double_t> m_input_LT_DIA_dose; /**< list of DIA LT dose */
    std::vector<Double_t> m_input_HT_DIA_dose; /**< list of DIA HT dose */
    std::vector<Double_t> m_input_LC_DIA_dose[12]; /**< list of DIA LC dose */
    std::vector<Double_t> m_input_HC_DIA_dose[12]; /**< list of DIA HC dose */
    std::vector<Double_t> m_input_LB_DIA_dose[12]; /**< list of DIA LB dose */
    std::vector<Double_t> m_input_HB_DIA_dose[12]; /**< list of DIA HB dose */
    std::vector<Double_t> m_input_LC_DIA_dose_av; /**< list of DIA LC dose */
    std::vector<Double_t> m_input_HC_DIA_dose_av; /**< list of DIA HC dose */
    std::vector<Double_t> m_input_LB_DIA_dose_av; /**< list of DIA LB dose */
    std::vector<Double_t> m_input_HB_DIA_dose_av; /**< list of DIA HB dose */

    std::vector<Double_t> m_input_LT_PIN_dose; /**< list of PIN LT dose */
    std::vector<Double_t> m_input_HT_PIN_dose; /**< list of PIN HT dose */
    std::vector<Double_t> m_input_LC_PIN_dose[12]; /**< list of PIN LC dose */
    std::vector<Double_t> m_input_HC_PIN_dose[12]; /**< list of PIN HC dose */
    std::vector<Double_t> m_input_LB_PIN_dose[12]; /**< list of PIN LB dose */
    std::vector<Double_t> m_input_HB_PIN_dose[12]; /**< list of PIN HB dose */
    std::vector<Double_t> m_input_LC_PIN_dose_av; /**< list of PIN LC dose */
    std::vector<Double_t> m_input_HC_PIN_dose_av; /**< list of PIN HC dose */
    std::vector<Double_t> m_input_LB_PIN_dose_av; /**< list of PIN LB dose */
    std::vector<Double_t> m_input_HB_PIN_dose_av; /**< list of PIN HB dose */

    std::vector<Double_t> m_input_LT_BGO_dose; /**< list of BGO LT dose */
    std::vector<Double_t> m_input_HT_BGO_dose; /**< list of BGO HT dose */
    std::vector<Double_t> m_input_LC_BGO_dose[12]; /**< list of BGO LC dose */
    std::vector<Double_t> m_input_HC_BGO_dose[12]; /**< list of BGO HC dose */
    std::vector<Double_t> m_input_LB_BGO_dose[12]; /**< list of BGO LB dose */
    std::vector<Double_t> m_input_HB_BGO_dose[12]; /**< list of BGO HB dose */
    std::vector<Double_t> m_input_LC_BGO_dose_av; /**< list of BGO LC dose */
    std::vector<Double_t> m_input_HC_BGO_dose_av; /**< list of BGO HC dose */
    std::vector<Double_t> m_input_LB_BGO_dose_av; /**< list of BGO LB dose */
    std::vector<Double_t> m_input_HB_BGO_dose_av; /**< list of BGO HB dose */

    std::vector<Double_t> m_input_LT_HE3_rate; /**< list of HE3 LT rate */
    std::vector<Double_t> m_input_HT_HE3_rate; /**< list of HE3 HT rate */
    std::vector<Double_t> m_input_LC_HE3_rate[12]; /**< list of HE3 LC rate */
    std::vector<Double_t> m_input_HC_HE3_rate[12]; /**< list of HE3 HC rate */
    std::vector<Double_t> m_input_LB_HE3_rate[12]; /**< list of HE3 LB rate */
    std::vector<Double_t> m_input_HB_HE3_rate[12]; /**< list of HE3 HB rate */
    std::vector<Double_t> m_input_LC_HE3_rate_av; /**< list of HE3 LC rate */
    std::vector<Double_t> m_input_HC_HE3_rate_av; /**< list of HE3 HC rate */
    std::vector<Double_t> m_input_LB_HE3_rate_av; /**< list of HE3 LB rate */
    std::vector<Double_t> m_input_HB_HE3_rate_av; /**< list of HE3 HB rate */

    std::vector<Double_t> m_input_LT_TPC_rate; /**< list of TPC LT rate */
    std::vector<Double_t> m_input_HT_TPC_rate; /**< list of TPC HT rate */
    std::vector<Double_t> m_input_LC_TPC_rate[12]; /**< list of TPC LC rate */
    std::vector<Double_t> m_input_HC_TPC_rate[12]; /**< list of TPC HC rate */
    std::vector<Double_t> m_input_LB_TPC_rate[12]; /**< list of TPC LB rate */
    std::vector<Double_t> m_input_HB_TPC_rate[12]; /**< list of TPC HB rate */
    std::vector<Double_t> m_input_LC_TPC_rate_av; /**< list of TPC LC rate */
    std::vector<Double_t> m_input_HC_TPC_rate_av; /**< list of TPC HC rate */
    std::vector<Double_t> m_input_LB_TPC_rate_av; /**< list of TPC LB rate */
    std::vector<Double_t> m_input_HB_TPC_rate_av; /**< list of TPC HB rate */

    std::vector<Double_t> m_input_LT_TPC_dose; /**< list of TPC LT dose */
    std::vector<Double_t> m_input_HT_TPC_dose; /**< list of TPC HT dose */
    std::vector<Double_t> m_input_LC_TPC_dose[12]; /**< list of TPC LC dose */
    std::vector<Double_t> m_input_HC_TPC_dose[12]; /**< list of TPC HC dose */
    std::vector<Double_t> m_input_LB_TPC_dose[12]; /**< list of TPC LB dose */
    std::vector<Double_t> m_input_HB_TPC_dose[12]; /**< list of TPC HB dose */
    std::vector<Double_t> m_input_LC_TPC_dose_av; /**< list of TPC LC dose */
    std::vector<Double_t> m_input_HC_TPC_dose_av; /**< list of TPC HC dose */
    std::vector<Double_t> m_input_LB_TPC_dose_av; /**< list of TPC LB dose */
    std::vector<Double_t> m_input_HB_TPC_dose_av; /**< list of TPC HB dose */

    std::vector<Double_t> m_input_LT_TPC_angular_rate; /**< list of TPC_angular LT rate */
    std::vector<Double_t> m_input_HT_TPC_angular_rate; /**< list of TPC_angular HT rate */
    std::vector<Double_t> m_input_LC_TPC_angular_rate[12]; /**< list of TPC_angular LC rate */
    std::vector<Double_t> m_input_HC_TPC_angular_rate[12]; /**< list of TPC_angular HC rate */
    std::vector<Double_t> m_input_LB_TPC_angular_rate[12]; /**< list of TPC_angular LB rate */
    std::vector<Double_t> m_input_HB_TPC_angular_rate[12]; /**< list of TPC_angular HB rate */
    std::vector<Double_t> m_input_LC_TPC_angular_rate_av; /**< list of TPC_angular LC rate */
    std::vector<Double_t> m_input_HC_TPC_angular_rate_av; /**< list of TPC_angular HC rate */
    std::vector<Double_t> m_input_LB_TPC_angular_rate_av; /**< list of TPC_angular LB rate */
    std::vector<Double_t> m_input_HB_TPC_angular_rate_av; /**< list of TPC_angular HB rate */

    std::vector<Double_t> m_input_LT_TPC_angular_dose; /**< list of TPC_angular LT dose */
    std::vector<Double_t> m_input_HT_TPC_angular_dose; /**< list of TPC_angular HT dose */
    std::vector<Double_t> m_input_LC_TPC_angular_dose[12]; /**< list of TPC_angular LC dose */
    std::vector<Double_t> m_input_HC_TPC_angular_dose[12]; /**< list of TPC_angular HC dose */
    std::vector<Double_t> m_input_LB_TPC_angular_dose[12]; /**< list of TPC_angular LB dose */
    std::vector<Double_t> m_input_HB_TPC_angular_dose[12]; /**< list of TPC_angular HB dose */
    std::vector<Double_t> m_input_LC_TPC_angular_dose_av; /**< list of TPC_angular LC dose */
    std::vector<Double_t> m_input_HC_TPC_angular_dose_av; /**< list of TPC_angular HC dose */
    std::vector<Double_t> m_input_LB_TPC_angular_dose_av; /**< list of TPC_angular LB dose */
    std::vector<Double_t> m_input_HB_TPC_angular_dose_av; /**< list of TPC_angular HB dose */

    std::vector<Double_t> m_input_LT_CSI_dose; /**< list of CSI LT dose */
    std::vector<Double_t> m_input_HT_CSI_dose; /**< list of CSI HT dose */
    std::vector<Double_t> m_input_LC_CSI_dose[12]; /**< list of CSI LC dose */
    std::vector<Double_t> m_input_HC_CSI_dose[12]; /**< list of CSI HC dose */
    std::vector<Double_t> m_input_LB_CSI_dose[12]; /**< list of CSI LB dose */
    std::vector<Double_t> m_input_HB_CSI_dose[12]; /**< list of CSI HB dose */
    std::vector<Double_t> m_input_LC_CSI_dose_av; /**< list of CSI LC dose */
    std::vector<Double_t> m_input_HC_CSI_dose_av; /**< list of CSI HC dose */
    std::vector<Double_t> m_input_LB_CSI_dose_av; /**< list of CSI LB dose */
    std::vector<Double_t> m_input_HB_CSI_dose_av; /**< list of CSI HB dose */

    std::vector<Double_t> m_input_LT_CSI_dose_binE; /**< list of CSI LT dose_binE */
    std::vector<Double_t> m_input_HT_CSI_dose_binE; /**< list of CSI HT dose_binE */
    std::vector<Double_t> m_input_LC_CSI_dose_binE[12]; /**< list of CSI LC dose_binE */
    std::vector<Double_t> m_input_HC_CSI_dose_binE[12]; /**< list of CSI HC dose_binE */
    std::vector<Double_t> m_input_LB_CSI_dose_binE[12]; /**< list of CSI LB dose_binE */
    std::vector<Double_t> m_input_HB_CSI_dose_binE[12]; /**< list of CSI HB dose_binE */
    std::vector<Double_t> m_input_LC_CSI_dose_binE_av; /**< list of CSI LC dose_binE */
    std::vector<Double_t> m_input_HC_CSI_dose_binE_av; /**< list of CSI HC dose_binE */
    std::vector<Double_t> m_input_LB_CSI_dose_binE_av; /**< list of CSI LB dose_binE */
    std::vector<Double_t> m_input_HB_CSI_dose_binE_av; /**< list of CSI HB dose_binE */

    std::vector<Double_t> m_input_LT_CSI_rate; /**< list of CSI LT rate */
    std::vector<Double_t> m_input_HT_CSI_rate; /**< list of CSI HT rate */
    std::vector<Double_t> m_input_LC_CSI_rate[12]; /**< list of CSI LC rate */
    std::vector<Double_t> m_input_HC_CSI_rate[12]; /**< list of CSI HC rate */
    std::vector<Double_t> m_input_LB_CSI_rate[12]; /**< list of CSI LB rate */
    std::vector<Double_t> m_input_HB_CSI_rate[12]; /**< list of CSI HB rate */
    std::vector<Double_t> m_input_LC_CSI_rate_av; /**< list of CSI LC rate */
    std::vector<Double_t> m_input_HC_CSI_rate_av; /**< list of CSI HC rate */
    std::vector<Double_t> m_input_LB_CSI_rate_av; /**< list of CSI LB rate */
    std::vector<Double_t> m_input_HB_CSI_rate_av; /**< list of CSI HB rate */

    std::vector<Double_t> m_input_LT_CLAWS_rate; /**< list of CLAWS_rate LT  */
    std::vector<Double_t> m_input_HT_CLAWS_rate; /**< list of CLAWS_rate HT  */
    std::vector<Double_t> m_input_LC_CLAWS_rate[12]; /**< list of CLAWS_rate LC  */
    std::vector<Double_t> m_input_HC_CLAWS_rate[12]; /**< list of CLAWS_rate HC  */
    std::vector<Double_t> m_input_LB_CLAWS_rate[12]; /**< list of CLAWS_rate LB  */
    std::vector<Double_t> m_input_HB_CLAWS_rate[12]; /**< list of CLAWS_rate HB  */
    std::vector<Double_t> m_input_LC_CLAWS_rate_av; /**< list of CLAWS_rate LC  */
    std::vector<Double_t> m_input_HC_CLAWS_rate_av; /**< list of CLAWS_rate HC  */
    std::vector<Double_t> m_input_LB_CLAWS_rate_av; /**< list of CLAWS_rate LB  */
    std::vector<Double_t> m_input_HB_CLAWS_rate_av; /**< list of CLAWS_rate HB  */

    std::vector<Double_t> m_input_LT_QCSS_rate; /**< list of QCSS_rate LT  */
    std::vector<Double_t> m_input_HT_QCSS_rate; /**< list of QCSS_rate HT  */
    std::vector<Double_t> m_input_LC_QCSS_rate[12]; /**< list of QCSS_rate LC  */
    std::vector<Double_t> m_input_HC_QCSS_rate[12]; /**< list of QCSS_rate HC  */
    std::vector<Double_t> m_input_LB_QCSS_rate[12]; /**< list of QCSS_rate LB  */
    std::vector<Double_t> m_input_HB_QCSS_rate[12]; /**< list of QCSS_rate HB  */
    std::vector<Double_t> m_input_LC_QCSS_rate_av; /**< list of QCSS_rate LC  */
    std::vector<Double_t> m_input_HC_QCSS_rate_av; /**< list of QCSS_rate HC  */
    std::vector<Double_t> m_input_LB_QCSS_rate_av; /**< list of QCSS_rate LB  */
    std::vector<Double_t> m_input_HB_QCSS_rate_av; /**< list of QCSS_rate HB  */

    std::vector<Double_t> m_input_I_HER; /**< HER current and error */
    std::vector<Double_t> m_input_I_LER; /**< LER current and error */

    std::vector<Double_t> m_input_P_LER; /**< LER pressure and error */
    std::vector<Double_t> m_input_P_HER; /**< HER pressure and error */

    std::vector<Double_t> m_input_bunchNb_LER; /**< LER bunch number and error */
    std::vector<Double_t> m_input_bunchNb_HER; /**< HER bunch number and error */

    Double_t m_input_data_bunchNb_LER; /**< LER bunch number and error */
    Double_t m_input_data_bunchNb_HER; /**< HER bunch number and error */
    std::string m_input_data_SingleBeam; /**< LER or HER or Both*/

    std::vector<Double_t> m_input_sigma_x_LER; /**< LER beam size and errors */
    std::vector<Double_t> m_input_sigma_x_HER; /**< HER beam size and errors */
    std::vector<Double_t> m_input_sigma_y_LER; /**< LER beam size and errors */
    std::vector<Double_t> m_input_sigma_y_HER; /**< HER beam size and errors */

    std::vector<Double_t> m_input_LB_SAD_RLR; /**< list of SAD_RLR LB dose */
    std::vector<Double_t> m_input_HB_SAD_RLR; /**< list of SAD_RLR HB dose */
    std::vector<Double_t> m_input_LC_SAD_RLR; /**< list of SAD_RLR LC dose */
    std::vector<Double_t> m_input_HC_SAD_RLR; /**< list of SAD_RLR HC dose */
    std::vector<Double_t> m_input_LB_SAD_RLR_av; /**< list of SAD_RLR LB dose */
    std::vector<Double_t> m_input_HB_SAD_RLR_av; /**< list of SAD_RLR HB dose */
    std::vector<Double_t> m_input_LC_SAD_RLR_av; /**< list of SAD_RLR LC dose */
    std::vector<Double_t> m_input_HC_SAD_RLR_av; /**< list of SAD_RLR HC dose */
    std::vector<Double_t> m_input_LT_SAD_RLR; /**< list of SAD_RLR LT dose */
    std::vector<Double_t> m_input_HT_SAD_RLR; /**< list of SAD_RLR HT dose */

    std::vector<Double_t> m_input_LT_DOSI; /**< list of PIN LT dose */
    std::vector<Double_t> m_input_HT_DOSI; /**< list of PIN HT dose */
    std::vector<Double_t> m_input_LC_DOSI[12]; /**< list of PIN LC dose */
    std::vector<Double_t> m_input_HC_DOSI[12]; /**< list of PIN HC dose */
    std::vector<Double_t> m_input_LB_DOSI[12]; /**< list of PIN LB dose */
    std::vector<Double_t> m_input_HB_DOSI[12]; /**< list of PIN HB dose */
    std::vector<Double_t> m_input_LC_DOSI_av; /**< list of PIN LC dose */
    std::vector<Double_t> m_input_HC_DOSI_av; /**< list of PIN HC dose */
    std::vector<Double_t> m_input_LB_DOSI_av; /**< list of PIN LB dose */
    std::vector<Double_t> m_input_HB_DOSI_av; /**< list of PIN HB dose */

    Double_t m_input_PIN_width; /**< PIN width */
    std::vector<Double_t> m_input_HE3_EfCor; /**< HE3 inefficiency correction */

    TF1* fctRate_HB;  /** fct HB */
    TF1* fctRate_HC;  /** fct HC */
    TF1* fctRate_LB;  /** fct LB */
    TF1* fctRate_LC;  /** fct LC */

    TTree* m_treeBEAST = 0;   /**< BEAST tree pointer */
    TTree* m_treeTruth = 0;   /**< Truth tree pointer */
    TChain* m_tree = 0;            /**< tree pointer */
    std::string m_inputFileName;  /**< input file name */
    std::vector<std::string> m_inputHistoFileNames; /**< list of histo. file names */
    std::string m_outputFileName; /**< output file name */
    TFile* m_file;        /**< TFile */
    BEAST_v5::BEASTTree_v5 m_beast; /** BEAST structure */

    int m_numEntries;     /**< number of ntuple entries */
    int m_entryCounter;   /**< entry counter */
    unsigned m_numEvents = 0;      /**< number of events (tree entries) in the sample */
    unsigned m_eventCount = 0;     /**< current event (tree entry) */
    unsigned m_exp = 0;            /**< Date of the day */
    Int_t m_DayBin;
  };

} // Belle2 namespace

#endif
