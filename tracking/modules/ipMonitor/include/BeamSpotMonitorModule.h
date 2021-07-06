/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>

#include <mdst/dbobjects/BeamSpot.h>

#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>

namespace Belle2 {
  /**
   * Module for the monitoring of the BeamSpot position and size
   */
  class BeamSpotMonitorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    BeamSpotMonitorModule();

    /** initialize the TTree*/
    virtual void initialize() override;

    /** check BeamSpot payload validity*/
    virtual void beginRun() override;

    /** fill trees */
    virtual void event() override;

    /** print the payloads uniqueID and write tree to the rootfile  */
    virtual void terminate() override;

  private:

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */
    TTree* m_tree = nullptr; /**<pointer to the tree */

    //branches
    TBranch* b_exp = nullptr; /**< experiment number*/
    TBranch* b_run = nullptr; /**< run number*/
    TBranch* b_x = nullptr; /**< X position of the beam spot*/
    TBranch* b_y = nullptr; /**< Y position of the beam spot*/
    TBranch* b_z = nullptr; /**< Z position of the beam spot*/
    TBranch* b_xErr = nullptr; /**< X position error of the beam spot*/
    TBranch* b_yErr = nullptr; /**< Y position error of the beam spot*/
    TBranch* b_zErr = nullptr; /**< Z position error of the beam spot*/
    TBranch* b_xSize = nullptr; /**< X position size of the beam spot*/
    TBranch* b_ySize = nullptr; /**< Y position size of the beam spot*/
    TBranch* b_zSize = nullptr; /**< Z position size of the beam spot*/

    //branch variables
    int m_run = -1; /**< run number*/
    int m_exp = -1; /**< experiment number*/
    double m_x = 0; /**< X position of the beam spot*/
    double m_y = 0; /**< Y position of the beam spot*/
    double m_z = 0; /**< Z position of the beam spot*/
    double m_xErr = 0; /**< X position error of the beam spot*/
    double m_yErr = 0; /**< Y position error of the beam spot*/
    double m_zErr = 0; /**< Z position error of the beam spot*/
    double m_xSize = 0; /**< X position size of the beam spot*/
    double m_ySize = 0; /**< Y position size of the beam spot*/
    double m_zSize = 0; /**< Z position size of the beam spot*/


    DBObjPtr<BeamSpot> m_BeamSpotDB; /**< beam spot payload from the database*/
    BeamSpot m_BeamSpot; /**< beam spot payload */

  };
}

