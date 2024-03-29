/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

  Int_t plot(TString ahName){
    ARICHAerogelHist *ah = (ARICHAerogelHist*)_file0->Get(ahName.Data());
    ah->DrawHisto("ZCOLOT text same","");
    return 0;
  }

}

Int_t plotTestARICHAerogelHist() {

  //plotTest();

  plot("h2_aerogel_up_n");
  plot("h2_aerogel_up_transmL");
  plot("h2_aerogel_up_thick");

  plot("h2_aerogel_down_n");
  plot("h2_aerogel_down_transmL");
  plot("h2_aerogel_down_thick");

  return 0;
  
}
