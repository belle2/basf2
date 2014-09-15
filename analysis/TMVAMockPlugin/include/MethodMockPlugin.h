/*
 * Thomas Keck 2014
 */

#pragma once

#ifndef ROOT_TMVA_MethodBase
#include "TMVA/MethodBase.h"
#endif

// Dirty hack see linkdef.h
namespace Belle2 {
  class MethodMockPlugin {};
}

namespace TMVA {

  /**
   * Mock TMVA Plugin for writing tests
   */
  class MethodMockPlugin : public MethodBase {

  public:
    /**
     * Constructor
     */
    MethodMockPlugin(const TString& jobName,
                     const TString& methodTitle,
                     DataSetInfo& theData,
                     const TString& theOption = "",
                     TDirectory* theTargetDir = 0);

    /**
     * Constructor
     */
    MethodMockPlugin(DataSetInfo& theData,
                     const TString& theWeightFile,
                     TDirectory* theTargetDir = NULL);

    /**
     * Destructor
     */
    virtual ~MethodMockPlugin();

    /**
     * Method
     */
    virtual Bool_t HasAnalysisType(Types::EAnalysisType type, UInt_t numberClasses, UInt_t numberTargets);

    /**
     * Method
     */
    void Train();

    /**
     * Method
     */
    void Init();

    /**
     * Method
     */
    void Reset();

    /**
     * Method
     */
    void AddWeightsXMLTo(void* parent) const;

    /**
     * Method
     */
    void ReadWeightsFromXML(void* parent);

    using MethodBase::ReadWeightsFromStream;

    /**
     * Method
     */
    void ReadWeightsFromStream(std::istream& istr);


    /**
     * Method
     */
    Double_t GetMvaValue(Double_t* err = 0, Double_t* errUpper = 0);

    /**
     * Method
     */
    const Ranking* CreateRanking();


    /**
     * Method
     */
    void DeclareOptions();

    /**
     * Method
     */
    void ProcessOptions();

    /**
     * Method
     */
    void GetHelpMessage() const;

  private:

    ClassDef(MethodMockPlugin, 0)
  };
}

