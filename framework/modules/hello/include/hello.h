// This is a template header file module for the basf2 framework
// it contains a class that should be derived from some basf2 module base class

#ifndef MODULEHELLO_H
#define MODULEHELLO_H

#include <framework/fwcore/Module.h>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include <string>
#include <vector>
#include <TRandom3.h>

#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/SimpleVec.h>
#include <framework/datastore/Relation.h>
#include <framework/datastore/EventMetaData.h>



namespace Belle2 {

  //! Module Hello
  /*!
  */
  class ModuleHello : public Module {

  public:

    //! Macro which adds a method to return a new instance of the module.
    NEW_MODULE(ModuleHello)

    //! Constructor
    ModuleHello();

    //! Destructor
    virtual ~ModuleHello();

    //! Initialize the Module
    /*! Function is called only once at the beginning of your job at the beginning of the corresponding module.
        Things that can be done here, should be done here, as it is relatively cheap in terms of CPU time.

        This method has to be implemented by subclasses.
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! At the beginning of each run, the function gives you the chance to change run dependent constants like alignment parameters, etc.

        This method has to be implemented by subclasses.
    */
    virtual void beginRun();

    //! Running over all events
    /*! Function is called for each evRunning over all events
        This means, this function is called very often, and good performance of the code is of strong interest.

        This method has to be implemented by subclasses.
    */
    virtual void event();

    //! Is called after processing the last event of a run
    /*! Good e.g. for storing stuff, that you want to aggregate over one run.

        This method has to be implemented by subclasses.
    */
    virtual void endRun();

    //! Is called at the end of your Module
    /*! Function is called only once at the end of your job at the end of the corresponding module.
        This function is for cleaning up, closing files, etc.

        This method has to be implemented by subclasses.
    */
    virtual void terminate();


  protected:


  private:

    int m_evtNum;

    int m_testParamInt;
    double m_testParamDouble;
    std::string m_testParamString;
    bool m_testParamBool;

    TRandom3 random;

    std::vector<int> m_testParamIntList;
    std::vector<double> m_testParamDoubleList;
    std::vector<std::string> m_testParamStringList;
    std::vector<bool> m_testParamBoolList;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
