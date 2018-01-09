//+
// File : PrintData.h
// Description : Dump basf2 objects to a binary file
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef ROOT2BINARY_H
#define ROOT2BINARY_H
#include <rawdata/modules/PrintDataTemplate.h>
//#include <rawdata/modules/CprErrorMessage.h>


namespace Belle2 {

  /*! Dump basf2 objects to a binary file */

  class Root2BinaryModule : public PrintDataTemplateModule {

    // Public functions
  public:

    //! Constructor / Destructor
    Root2BinaryModule();
    virtual ~Root2BinaryModule();

    //!
    virtual void initialize();

    //!
    virtual void event();

    //!
    virtual void endRun();

    //!
    virtual void terminate();

    //! write the contents of an event
    virtual void writeEvent(RawDataBlock* raw_dblk, int* first_flag, int* break_flag,
                            int* dblk_pos, unsigned int* dblk_eve);

  protected :

    //! File descripter
    FILE* m_fp_out;

    //! Output filename
    std::string m_fname_out;

    //!
    //    CprErrorMessage print_err;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
