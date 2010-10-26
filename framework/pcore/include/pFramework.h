//+
// File : pFramework.h
// Description : Framework class for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - May - 2010
//-
#ifndef PFRAMEWORK_H_
#define PFRAMEWORK_H_

#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/exception_translator.hpp>

#include <framework/core/Path.h>
#include <framework/core/Module.h>
#include <framework/core/Framework.h>

#include <framework/pcore/pEventProcessor.h>

#include <string>
#include <list>

namespace Belle2 {

  class pFramework : public Framework {
  public:
    static pFramework& Instance();

    pFramework(void);
    ~pFramework(void);

    //  void process_parallel ( PathPtr startpath, unsigned long maxevt );
    void process_parallel(PathPtr startpath);

    static void exposePythonAPI();

    static int nprocess(void);
    void nprocess(int);

  private:

    pEventProcessor* m_pEventProcessor; /*!< The event processor, which loops over the events and calls the modules. */

  };

} // End of Belle2

#endif
