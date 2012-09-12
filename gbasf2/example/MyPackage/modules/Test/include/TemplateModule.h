#ifndef TEMPLATEMOD_H
#define TEMPLATEMOD_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

//! Module TemplateModule
  /*!
    Please add here some description of your module for doxygen.
  */
  class TemplateModule : public Module {

  public:
    //! Constructor
    TemplateModule(bool selfReg = true);

    //! Destructor
    virtual ~TemplateModule();

    //! Initialize the Module
    /*! This method is called only once before the actual event processing starts.
        Use this method to initialize variables, open files etc.
    */
    virtual void initialize();

    //! Called when entering a new run
    /*! Called at the beginning of each run, the method gives you the chance to change run dependent constants like alignment parameters, etc.
    */
    virtual void beginRun();

    //! This method is the core of the module.
    /*! This method is called for each event. All processing of the event has to take place in this method.
    */
    virtual void event();

    //! This method is called if the current run ends.
    /*! Use this method to store information, which should be aggregated over one run.
    */
    virtual void endRun();

    //! This method is called at the end of the event processing.
    /*! This method is called only once after the event processing finished.
        Use this method for cleaning up, closing files, etc.
    */
    virtual void terminate();

  protected:

  private:

  };
} // end namespace Belle2
#endif

