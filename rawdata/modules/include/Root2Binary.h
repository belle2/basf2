//+
// File : PrintData.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef ROOT2BINARY_H
#define ROOT2BINARY_H
#include <rawdata/modules/PrintDataTemplate.h>
//#include <rawdata/modules/CprErrorMessage.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Root2BinaryModule : public PrintDataTemplateModule {

    // Public functions
  public:

    //! Constructor / Destructor
    Root2BinaryModule();
    virtual ~Root2BinaryModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();

    //!
    virtual void endRun();

    //!
    virtual void terminate();

    //!
    virtual void writeEvent(RawDataBlock* raw_dblk, int* first_flag, int* break_flag,
                            int* dblk_pos, unsigned int* dblk_eve);

  protected :
    //!
    FILE* m_fp_out;

    //!
    std::string m_fname_out;

    //!
    //    CprErrorMessage print_err;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
