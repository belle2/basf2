/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ROOT2BINARY_H
#define ROOT2BINARY_H
#include <rawdata/modules/PrintDataTemplate.h>
//#include <rawdata/modules/CprErrorMessage.h>


namespace Belle2 {

  /*! Dump basf2 objects to a binary file */

  class Root2BinaryModule : public PrintDataTemplateModule {

    // Public functions
  public:

    //! Constructor
    Root2BinaryModule();
    //! Destructor
    virtual ~Root2BinaryModule();

    //! Called at the beginning of data processing
    virtual void initialize() override;

    //! Called for each event
    virtual void event() override;

    //! Called if the current run ends
    virtual void endRun() override;

    //! Called at the end of data processing
    virtual void terminate() override;

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
