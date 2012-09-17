/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TrueHitTesterModule_H_
#define TrueHitTesterModule_H_

#include <framework/core/Module.h>

//root
#include <TMatrixT.h>
//stuff for root output
#include <TTree.h>
#include <TFile.h>

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>


namespace Belle2 {
  /** This module gets the VXDTrueHits form the datastore and extracts information from the them in a format that is same as
   * the local coordinates used by Genfit. In this way the information form the simulation can be compared for every layer with the
   * smoother information from he track fitter. Additionally the Energy loss and scattering angles are extracted form the true hits
   * if can be inspected with root afterwards. Basically this module presents you the simulation data directly before it enters the
   * the fitter.
   */
  class TrueHitTesterModule : public Module {

  public:

    //! Constructor
    TrueHitTesterModule();

    //! Destructor
    ~TrueHitTesterModule();

    //! Initialize the Module
    /*! Function is called only once at the beginning of your job at the beginning of the corresponding module.
        Things that can be done here, should be done here, as it is relatively cheap in terms of CPU time.

        This method has to be implemented by subclasses.
    */
    void initialize();

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.

        This method has to be implemented by subclasses.
    */
    void beginRun();

    //! Running over all events
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.

        This method has to be implemented by subclasses.
    */

    void event();

    //! Is called after processing the last event of a run
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.

        This method has to be implemented by subclasses.
    */
    void endRun();

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.

        This method has to be implemented by subclasses.
    */
    void terminate();


  protected:
    void registerLayerWiseData(const std::string& nameOfDataSample, const int nVarsToTest);  /**< function to create std vector<vector<float> > branches in a root file*/
    void fillLayerWiseData(const std::string& nameOfDataSample, const int accuVecIndex, const std::vector<double>& newData); /**< function to put data into the root branches created by registerLayerWiseData */

    //for root output
    std::string m_dataOutFileName; /**< name of the output file*/
    TFile* m_rootFilePtr; /**< the root file object for the custom root output*/
    TTree* m_trueHitDataTreePtr; /**< the root tree object for the custom root output. Use only accessed via fillLayerWiseData*/

    std::map<std::string, std::vector<std::vector<float> >* > m_layerWiseDataForRoot; /**< holds the branch varaibles for the costum root output. Only accessed via fillLayerWiseData*/


    int m_nLayers; /**< number of VXD layers. That is PXD + SVD layers*/
    int m_nPxdLayers; /**< number of PXD layers*/
    int m_nSvdLayers; /**< number of SVD layers*/

    bool m_filter; /**< this flag determins if only tracks with exaclty m_nLayers (=6) hits are used*/

    int m_notPerfectCounter; /**< if m_filter is true this holds the number of tracks not having m_nLayers (=6) hits*/


  };
}

#endif /* TrueHitTesterModule_H_ */
