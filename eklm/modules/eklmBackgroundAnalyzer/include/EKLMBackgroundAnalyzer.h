#ifndef EKLMBACKGROUNDANALYZERMODULE_H
#define EKLMBACKGROUNDANALYZERMODULE_H

#include <framework/datastore/StoreObjPtr.h>

#include <framework/datastore/StoreArray.h>

#include <framework/core/Module.h>
#include <string>

#include <eklm/geoeklm/G4PVPlacementGT.h>
#include <eklm/eklmhit/EKLMBackHit.h>




namespace Belle2 {

  //!  EklmBackgroundAnalyzer
  /*!
    Simple module for reading EKLM hits
  */
  class EKLMBackgroundAnalyzerModule : public Module  {

  public:

    //! Constructor
    /*!
      Create and allocate memory for variables here.
      Add the module parameters in this method.
      \param selfRegisterType True if this module is self-registering,
      otherwise false.
    */
    EKLMBackgroundAnalyzerModule();

    //! Destructor
    /*!
      Use the destructor to release the memory you allocated in the constructor.
    */
    virtual ~EKLMBackgroundAnalyzerModule();

    //! Initialize the module
    /*! This method is called only once before the actual event
        processing starts.
        Use this method to initialize variables, open files etc.
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! Called at the beginning of each run, the method gives you the chance
        to change run dependent constants like alignment parameters, etc.
    */
    virtual void beginRun();

    //! This method is the core of the module.
    /*! This method is called for each event.
        All processing of the event has to take place in this method.
    */
    virtual void event();

    //! This method is called if the current run ends.
    /*! Use this method to store information,
        which should be aggregated over one run.
    */
    virtual void endRun();

    //! This method is called at the end of the event processing.
    /*! This method is called only once after the event processing finished.
        Use this method for cleaning up, closing files, etc.
    */
    virtual void terminate();


  protected:
  private:
    //! std::map for hits sorting according G4PVPlacementGT
    std::map<std::string , std::vector<EKLMBackHit*> > m_HitVolumeMap;

    /*! Function to read hits from the starage and sort them according to the PV
     *  clone of those in EKLMDigitizer Should it be template function?
     */
    void readAndSortBkgHits();

    void makeGraphs();


  };
} // end namespace Belle2
#endif

