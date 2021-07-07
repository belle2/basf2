/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
void CreateClass(TString FileName,TString TreeName,TString ClassName)
{
  TFile *File = new TFile(FileName);
  TTree *Tree = (TTree*)File->Get(TreeName);
  Tree        ->MakeClass(ClassName);
}
