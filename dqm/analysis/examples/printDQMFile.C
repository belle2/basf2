/**
 * This example prints the content of the DQM file with MonitoringObjects 
 * if "name" is set it only prints content of MonitoringObject with this name 
 * if "opt" is set to "draw" all canvases are shown
 *
 * run as: root -l intputfile.root printDQMFile.C
 *
 * Author: Luka Santelj, Feb. 2020 
 * (luka.santelj@ijs.si)
 */

void printDQMFile(TString name="",TString opt=""){
  
  TIter next(_file0->GetListOfKeys());
  TKey* key;
  Belle2::DQMFileMetaData* meta;
  std::vector<Belle2::MonitoringObject*> monobj;
  while((key=(TKey*)next())){
    std::cout << key->GetClassName() << std::endl;
    if( TString(key->GetClassName()) == "Belle2::DQMFileMetaData") meta = (Belle2::DQMFileMetaData*)key->ReadObj();
    if( TString(key->GetClassName()) == "Belle2::MonitoringObject") monobj.push_back((Belle2::MonitoringObject*)key->ReadObj());
  }

  std::cout << "DQM file content" << std::endl << std::endl;
  
  meta->Print("all");
  
  for(auto mon : monobj){    
    if(name.Length() && name!=mon->GetName()) continue;
    mon->print();    
    if(opt=="draw"){
      const std::vector<TCanvas*> canvs = mon->getListOfCanvases();
      for(auto canv : canvs) canv->Draw();    
    }
  }
 
}