/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leonid Burmistrov                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

using namespace Belle2;

namespace {

  Int_t plotTest(){
    TString h2name = "h2_aerogel_Hist";
    TString h2title = "ARICHAerogelHist";
    ARICHAerogelHist *h2_aerogel = new ARICHAerogelHist( h2name.Data(), h2title.Data());
    cout<<"h2_aerogel->GetNcells() = "<<h2_aerogel->GetNcells()<<endl;
    for(Int_t i = 1;i<=h2_aerogel->GetNcells();i++){
      h2_aerogel->SetBinContent(i,i);
    }
    h2_aerogel->DrawHisto("ZCOLOT text same","");
    return 0;
  }

}

Int_t plotTestARICHAerogelHist() {

  plotTest();

  //Build the plots    
  //plot( elementsAM, elementsAI, "up", "n");
  //plot( elementsAM, elementsAI, "down", "n");
  //plot( elementsAM, elementsAI, "up", "transmL");
  //plot( elementsAM, elementsAI, "down", "transmL");
  //plot( elementsAM, elementsAI, "up", "thick");
  //plot( elementsAM, elementsAI, "down", "thick");
  //Int_t verboseLevel = 0;
  //printMy (elementsAM, elementsAI, verboseLevel);

  return 0;
  
}
