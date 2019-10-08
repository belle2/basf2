{

  TFile f1("trggdlDQM.root");
  TRG->cd();
  gDirectory->ls();

  TCanvas* canvas = new TCanvas("canvas");
  hGDL_gdlL1TocomL1->Draw();
  canvas->Print("trggdlDQM.pdf(","gdlL1TocomL1");
  hGDL_rvcoutTogdlL1->Draw();
  canvas->Print("trggdlDQM.pdf","rvcoutTogdlL1");
  hGDL_eclTogdlL1->Draw();
  canvas->Print("trggdlDQM.pdf","eclTogdlL1");
  hGDL_ecl8mToGDL->Draw();
  canvas->Print("trggdlDQM.pdf","ecl8mToGDL");
  hGDL_eclToGDL->Draw();
  canvas->Print("trggdlDQM.pdf","eclToGDL");
  hGDL_ns_cdcTocomL1->Draw();
  canvas->Print("trggdlDQM.pdf","cdcTocomL1");
  hGDL_ns_cdcTogdlL1->Draw();
  canvas->Print("trggdlDQM.pdf","cdcTogdlL1");
  hGDL_ns_topToecl->Draw();
  canvas->Print("trggdlDQM.pdf","topToecl");
  hGDL_ns_topTocdc->Draw();
  canvas->Print("trggdlDQM.pdf","topTocdc");
  hGDL_cdcToecl->Draw();
  canvas->Print("trggdlDQM.pdf","cdcToecl");
  hGDL_ns_cdcToecl->Draw();
  canvas->Print("trggdlDQM.pdf","cdcToecl");
  hGDL_inp->Draw();
  canvas->Print("trggdlDQM.pdf","input bits");
  hGDL_itd->Draw();
  canvas->Print("trggdlDQM.pdf","ITD bits");
  hGDL_ftd->Draw();
  canvas->Print("trggdlDQM.pdf","FTD bits");
  hGDL_psn->Draw();
  canvas->Print("trggdlDQM.pdf","PSNM bits");
  hGDL_timtype->Draw();
  canvas->Print("trggdlDQM.pdf)","TimType bits");

}
