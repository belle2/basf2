/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*****************************************************************
* Simple root script that adds TStrings to tree which 
* can be used to store additional information. The tree 
* will be stored in the root file.
*
* example: 
*****************************************************************/
#include <TString.h>
#include <TFile.h>
#include <TTree.h>

#include <iostream>
#include <time.h>

/** @param: filename : the name of the file 
    @param: delete_tree: if true the old tree will be deleted, else a new entry will be added to the tree
    @param: tree_name: name of the tree
    @return: return true if everything works as expected, false else
*/

const TString branchname = "entry";

bool AddDescriptionToRootFile(TString filename, TString description, bool delete_tree = false, TString tree_name = "Descriptions"); 

bool
AddDescriptionToRootFile(TString filename, TString description, bool delete_tree, TString tree_name){
  
  TFile f(filename , "UPDATE");
  if( !f.IsOpen() ) {
    std::cout << "ERROR: file is not open! filename: " << filename << std::endl;
    return false;
  } 

  // get the pointer to the old tree
  TTree * tree = (TTree*)f.Get(tree_name);

  // create a new tree in this case
  if( delete_tree && tree != nullptr) {
    std::cout << "Deleting old tree" << std::endl;
    delete tree;
    tree = nullptr;
  }
    
  TString * content = new TString("");
  // if no tree create a new one
  if( tree == nullptr ){
    std::cout << "Creating new tree" << std::endl;
    tree = new TTree(tree_name, "tree containing descriptions/information of the contents of this file (can have more than 1 entry)"); 
    tree->Branch(branchname, content);
  }

  if( tree->GetBranch(branchname) == nullptr ) {
    std::cout << "ERROR: branch not found! branch name: " << branchname << " in tree: " << tree_name << std::endl;
    return false;
  }
  
  tree->SetBranchAddress(branchname, &content);

  // add the time stamp to all entries
  time_t rawtime;
  struct tm * timeinfo;
  char buffer [80];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
  std::string timestring(buffer);


  *content = timestring + " : " + description; 
  tree->Fill();

  tree->Write();

  f.Close();

  return true;
}
