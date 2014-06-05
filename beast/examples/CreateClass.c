void CreateClass(TString FileName,TString TreeName,TString ClassName)
{
  TFile *File = new TFile(FileName);
  TTree *Tree = (TTree*)File->Get(TreeName);
  Tree        ->MakeClass(ClassName);
}
