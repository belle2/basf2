

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef KlongValidationModule_H
#define KlongValidationModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <TTree.h>
#include <TFile.h>
#include <string>
#include <TEfficiency.h>
#include <TH1F.h>
#include <TH2.h>
#include <TGraphAsymmErrors.h>



namespace Belle2 {

  class KlongValidationModule : public Module {

  public:

    KlongValidationModule();

    virtual ~KlongValidationModule();

    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    virtual void terminate();

  protected:

  private:

    double m_momentum;
    double m_theta;
    double m_phi;
    double m_isKl;
    double m_isBeamBKG;
    bool   m_passed;
    bool   m_faked;
    bool   m_reconstructedAsKl;

    //use TEfficiency histogramms to calculate efficiency
    // name, title;xtitle;ytitle, bins (noch herausfinden)
    TEfficiency* m_effPhi;
    TEfficiency* m_effTheta;
    TEfficiency* m_effMom;
    TEfficiency* m_fakePhi;
    TEfficiency* m_fakeTheta;
    TEfficiency* m_fakeMom;

    std::string m_outPath = "KlongValidationData.root";

    /* root tree etc.  **/
    TFile* m_f = nullptr;  //
    TTree* m_tree = nullptr;

  }; // end class
} // end namespace Belle2

#endif
