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

  class MethodMockPlugin : public MethodBase {

  public:
    MethodMockPlugin(const TString& jobName,
                     const TString& methodTitle,
                     DataSetInfo& theData,
                     const TString& theOption = "",
                     TDirectory* theTargetDir = 0);

    MethodMockPlugin(DataSetInfo& theData,
                     const TString& theWeightFile,
                     TDirectory* theTargetDir = NULL);

    virtual ~MethodMockPlugin();

    virtual Bool_t HasAnalysisType(Types::EAnalysisType type, UInt_t numberClasses, UInt_t numberTargets);
    void Train();
    void Init();
    void Reset();


    void AddWeightsXMLTo(void* parent) const;
    void ReadWeightsFromXML(void* parent);

    using MethodBase::ReadWeightsFromStream;
    void ReadWeightsFromStream(std::istream& istr);

    Double_t GetMvaValue(Double_t* err = 0, Double_t* errUpper = 0);
    const Ranking* CreateRanking();

    void DeclareOptions();
    void ProcessOptions();
    void GetHelpMessage() const;

  private:

    ClassDef(MethodMockPlugin, 0)
  };
}

