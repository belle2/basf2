/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCLUSTERINFOMODULE_H
#define ECLCLUSTERINFOMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>

// root
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TMatrixFSym.h"
#include "TVector3.h"

namespace Belle2 {
  /**
   * Creates an output root file which contains info from ECL clusters. Used for b2bii conversion validation.
   *
   *    *
   */
  class ECLClusterInfoModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLClusterInfoModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void event();

    /**  */
    virtual void terminate();

  private:

    // Set parameters

    /** Name for output file.
     */
    std::string m_fileName;

    /** Name for tree.
     */
    std::string m_treeName;

    // Set variables

    TFile* m_file;
    TTree* m_tree;

    float m_E;
    float m_Theta;
    float m_Phi;
    float m_R;

    float m_Err00;
    float m_Err10;
    float m_Err11;
    float m_Err20;
    float m_Err21;
    float m_Err22;

    float m_Truth_Px;
    float m_Truth_Py;
    float m_Truth_Pz;
    float m_Truth_E;
    int m_PDG;
  };
}

#endif /* ECLCLUSTERINFOMODULE_H */
