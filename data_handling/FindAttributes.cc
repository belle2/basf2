// -* Data Handling  for Belle II Experimental -*-
//
// Description: registeration tool
//
// Author : JungHyun, Kim
// Created: 2010. 08. 18.
//


/****************************************************************************************

[File-level attributes] (from B2TDR)

*: This attribute can be found in the mdc file. If we have to add the
   attribute into the mdc file, we will assign the given name to it.

=========================================================================================
Attribute   Type           description                     mdc name        parameter
-----------------------------------------------------------------------------------------
guid        varchar(40)    grid unique ID (hex format)     GUID*
lfn         varchar(1024)  logical file name               LFN             PhysicalFileName
status      varchar(16)    good/bad/and so on              Run status      (output in mdc)
events      int            total number of events          Events*         max_evt
datasetid   int            dataset ID                      Dataset ID*
stream      int            stream number                   Stream          _stream
runH        int            highest run number              Max run*        RunNo
eventH      int            highest event number            Max evt         max_evt
runL        int            lowest run number               Min run*        RunNo
eventL      int            lowest event number             Min evt         min_evt
parentid    int(128)       IDs of parent files             Parent ID
softwareid  int            ID of software version          Software ID*
siteid      int            ID of site where it was created Site ID*
userid      int            ID of a user who created it     User ID*
log guid    varchar(40)    GUID of log file (hex format)   Log GUID*
==============================================================================

[Dataset attributes]
====================================================================================
Attribute   Type           description                     mdc name       parameter
------------------------------------------------------------------------------------
id          int            unique ID                       Dataset ID*
name        varchar(32)    unique name
description varchar(1024)  description
type        char(1)        'M' or 'R':MonteCarlo or Real   Data type     Mc_Real_Sel
level       varchar(16)    uds, charm, charged, mixed                    _mctype
mode        varchar(16)    on_resonance, continuum                       (not ready)
skim        varchar(32)    skim name
userid      int            ID of user who added it         User ID*
date        timestamp      date when it was added          Create date   _create_date
------------------------------------------------------------------------------

[Software release version attributes]
=========================================================================================
Attribute   Type           description                     mdc name              parameter
-----------------------------------------------------------------------------------------
id          int            unique ID                       Software ID*
name        varchar(64)    software name
version     varchar(16)    release version                Belle library version  _belle_version
description varchar(1024)  description
----------------------------------------------------------------------------------------

[Path information attributes]
=========================================================================================
Attribute   Type           description                     mdc name      parameter
-----------------------------------------------------------------------------------------
type        char(1)        'M' or 'R':MonteCarlo or Real   Data type      Mc_Real_Sel
exp         int            experiment number               Exp No         ExpNo
path        varchar(1024)  AMGA directory path
------------------------------------------------------------------------------------------

[Event-level attributes]
==============================================================================
Attribute   Type           description                     mdc name
------------------------------------------------------------------------------
nmichargedtrk int          # of - charged track
nplchargedtrk int          # of + charged track
nks           int          # of K_S 2
nkl           int          # of K_L 2
skimid        int          skim number
r2            int          R2 value(0.0 ~ 1.0)
------------------------------------------------------------------------------

[Skim attributes]
==============================================================================
Attribute   Type           description                     mdc name
------------------------------------------------------------------------------
id          smallint       unique ID
name        varchar(32)    unique name
description varchar(1024)  description
userid      int            ID of user who added it
date        timestamp      date when it was added
------------------------------------------------------------------------------

[User attributes]
==============================================================================
Attribute   Type           description                     mdc name
------------------------------------------------------------------------------
id          int            unique ID
name        varchar(32)    unique name
description varchar(1024)  description
date        timestamp      date when it was added
------------------------------------------------------------------------------

[Site attributes]
===================================================================================================
Attribute   Type           description                     mdc name      parameter
---------------------------------------------------------------------------------------------------
id          int            unique ID                       Site ID*     hostname(define as string)
name        varchar(64)    unique name
description varchar(1024)  description
date        timestamp      date when it was added
has_exp_fr  varbit(128)    file-level, real data
has_exp_fm  varbit(128)    file-level, montecarlo
------------------------------------------------------------------------------

The following list shows mdc names that are not described in the above tables.
------------------------------------------------------------------------------
attribute description      type            parameter         code description
------------------------------------------------------------------------------

                                                             Starting(check the spending time with end)
            ?                                              File No
            int?                                           ID No
            ?                                              Exp date
            ?                                              Exp time
            ?                                            Belle detector version
            ?                                              End
------------------------------------------------------------------------------

 */

//-------------------------------------------------------------------
// Class definition
// ------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include "time.h"
#include "stdlib.h"
#include "stdio.h"
#include "belle_amga.h"

#include "basf/module.h"
#include "basf/module_descr.h"
#include "basf/basfout.h"
#include "event/BelleEvent.h"
#include "tuple/BelleTupleManager.h"
#include "panther/panther.h"
#include "particle/Particle.h"
#include "particle/utility.h"
#include "mdst/mdst.h"
#include "ip/IpProfile.h"     //about dr dz 
#include "tables/mctype.h"
#include "basf/basfshm.h"

#include EVTCLS_H       //Hadron flag
#include MDST_H
#include HEPEVT_H
#include BELLETDF_H

#if defined(BELLE_NAMESPACE)
using namespace std;
namespace Belle {
#endif

  unsigned setGenHepInfoR(Particle&);   //set genhepinevt. to
  //reconstructed particle

  extern BasfSharedMem* BASF_Sharedmem;

  struct minmax_evt {
    int min;
    int max;
  };

  class BelleEvent;
  class BasfOutput;

  class FindAttributes
      : public Module {
  public:
    FindAttributes();

    // Constructor
    ~FindAttributes();            // Destructor

    void init(int*);            // Initialize
    void term(void);            // Terminates
    void disp_stat(const char*);    // Displays status (dummied)
    void hist_def(void);      // Defines histogram and ntuples
    void begin_run(BelleEvent*, int*); // Beginning of run handling
    void event(BelleEvent*, int*);  // Processes an event
    void end_run(BelleEvent*, int*); // End of run handling (dummied)
    void other(int*, BelleEvent*, int*);  // Others (dummied)

  public:
    //Def select real data or MC data
    int Mc_Real_Sel, ExpNo, RunNo, EvtNo, ExpTime, ExpDate, ExpDetver;
    int _mctype, _stream;

    ofstream _file ;

    //---
    // flags for which types of events are written.
    //---

    //--
    // file names.
    //---
    char PhysicalFileName[256];
    char DataType[256];

    int  OutputFlag;

    char _temp_file[256];
    char _evt_file[256] ;
    char _file_ID[256];
    char _hostname[256] , _create_date[256] ;

  private:
    BelleTuple* nt_meta;         //Def of histogram
    bool isfirst;
  };

/////////////////////////////////////////////////////////////////////
// BASF Interface Function
/////////////////////////////////////////////////////////////////////

  extern "C" Module_descr* mdcl_FindAttributes()
  {
    FindAttributes* module = new FindAttributes;
    Module_descr* dscr = new Module_descr("FindAttributes", module);

    dscr->define_param("DataType",
                       "Data Type(on_resonance:(4S) off_resonance: continuum",
                       256, module->DataType);
    dscr->define_param("OutputFlag",
                       "turn on output(=1:do skim/=0:no output)",
                       &module->OutputFlag);
    dscr->define_param("PhysicalFileName",
                       "physical file name",
                       256, module->PhysicalFileName);

    return dscr;
  }

//////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////

  FindAttributes::FindAttributes(void)
  {
    OutputFlag = 1;
    isfirst = true;
  }

//////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////

  FindAttributes::~FindAttributes()
  {
  }

//////////////////////////////////////////////////////////////////////
// Initializes FindAttributes
//////////////////////////////////////////////////////////////////////

  void FindAttributes::init(int* status)
  {

    time_t t;
    time(&t);

    BASF_Sharedmem->allocate("minmax_evt", sizeof(minmax_evt));

    string str(PhysicalFileName);
    int first = str.find_last_of("/");
    int last = str.find_last_of(".mdst");

    sprintf(_temp_file, "_%s.mdc", str.substr(first + 1, last - (first + 5)).c_str());

    _file.open(_temp_file);

    _file << "LFN : " << PhysicalFileName << endl;

    _file << "Starting : " << t << endl;
  }

//////////////////////////////////////////////////////////////////////
// Terminates FindAttributes
//////////////////////////////////////////////////////////////////////

  void FindAttributes::term()
  {
    time_t t;
    time(&t);

    _file << "End : " << t << endl;
    _file << "Run status : good" << endl;

    gethostname(_hostname, 255);
    _file << "Hostname : " << _hostname << endl;

    strftime(_create_date, 255, "%a %b %d %H:%M:%S %Z %Y",
             localtime(&t));
    _file << "Create date : " << _create_date << endl;

    int size;
    minmax_evt* mm_evt
      = (minmax_evt*)BASF_Sharedmem->get_pointer(0, "minmax_evt", &size);
    int min_evt = mm_evt->min;
    int max_evt = mm_evt->max;
    int nproc = BASF_Sharedmem->nprocess();
    for (int i = 1; i < nproc; i++) {
      mm_evt = (minmax_evt*)BASF_Sharedmem->get_pointer(i, "minmax_evt", &size);
      if (mm_evt->min < min_evt) min_evt = mm_evt->min;
      if (mm_evt->max > max_evt) max_evt = mm_evt->max;
    }
    _file << "Min evt : " << min_evt << endl;
    _file << "Max evt : " << max_evt << endl;

    _file.close();

    /* AMGA developing team
    */
//------------------------------------------------------






//-------------------------------------------------------
  }

//////////////////////////////////////////////////////////////////////
// Displays  status
//////////////////////////////////////////////////////////////////////

  void FindAttributes::disp_stat(const char*)
  {
  }

//////////////////////////////////////////////////////////////////////
// Defines histograms
//////////////////////////////////////////////////////////////////////

  void FindAttributes::hist_def()
  {
    extern BelleTupleManager* BASF_Histogram;
    BelleTupleManager& tm = *BASF_Histogram;

    char* meta =
      // should leave blank at last line
      //                             |
      //                             \/
      "expnum runnum evtnum random "
      "deltae mbc "
      "v11_px v11_py v11_pz "
      "v12_px v12_py v12_pz "
      "v21_px v21_py v21_pz ksmass "
      "v22_px v22_py v22_pz "
      "v1_px v1_py v1_pz rhomass "
      "v2_px v2_py v2_pz kstmass "
      "cosb dz lr1 lr2 lr3 qr charge "
      "pid1 pid2 pid0 pid3 pid4 "
      "helrho helkst "
      "a0 a1 a2 a3 a4 "
      "b0 b1 b2 b3 b4 "
      "c0 c1 c2 c3 c4 "
      "d0 d1 d2 d3 d4 "
      "e0 e1 e2 e3 e4 "
      "f0 f1 f2 f3 f4 ";

    nt_meta = tm.ntuple("Meta-data Catalog information", meta, 500);


  }

//////////////////////////////////////////////////////////////////////
// Beginning of run handling
//////////////////////////////////////////////////////////////////////

  void FindAttributes::begin_run(BelleEvent*, int*)
  {
    // Set IP profile
    IpProfile::begin_run();   //about dr dz cut
    IpProfile::dump();
    const char* RealOrMC = " RM";

    // MC or Real selection
    Belle_runhead_Manager& RunMgr = Belle_runhead_Manager::get_manager();
    Mc_Real_Sel = RunMgr[0].ExpMC();

    //_file << "Data type[real(1)/MC(2)] : " << Mc_Real_Sel << endl;
    // According to the dataset attributes table of TDR, Data type is 'R' or 'M'.
    // FIXME : is it certain that Mc_Real_Sel = 1 for REAL and 2 for MC?
    _file << "Data type : " << RealOrMC[Mc_Real_Sel] << endl;

    if (Mc_Real_Sel == 2)  {
      Mctype_Manager& McMgr = Mctype_Manager::get_manager();
      _mctype = McMgr[0].type();
      _stream = McMgr[0].stream();

      _file << "MC type : " << _mctype << endl;
      _file << "Stream : " << _stream << endl;
    }

    ExpNo = RunMgr[0].ExpNo();
    RunNo = RunMgr[0].RunNo();
    ExpDate = RunMgr[0].Date();
    ExpTime = RunMgr[0].Time();
    ExpDetver = RunMgr[0].DetVer();
    //               EvtNo = RunMgr[0].EvtNo();
    _file << "Exp No : " << ExpNo << endl;
    _file << "Run No : " << RunNo << endl;

    sprintf(_file_ID, "f%d", RunNo);
    _file << "File No : " << _file_ID << endl;
    _file << "ID No : " << RunNo << endl;
    _file << "Exp date : " << ExpDate << endl;
    _file << "Exp time : " << ExpTime << endl;
    _file << "Parent ID : " << endl;

    string str(PhysicalFileName);
    int first = str.find_last_of("-");
    int last = str.find_last_of(".mdst");

    string _belle_version =  str.substr(first + 1, last - (first + 5));
    _file << "Belle library version : "
          << _belle_version << endl;
    _file << "Belle detector version : " << ExpDetver << endl;
  }

//////////////////////////////////////////////////////////////////////
// Processes an event
//////////////////////////////////////////////////////////////////////

  void FindAttributes::event(BelleEvent*, int* status)
  {
    *status = -1 ;

    // scale_momenta

    Belle_event_Manager& header_mgr
      = Belle_event_Manager::get_manager();
    Belle_event& header(header_mgr[0]);
    int expnum  = header.ExpNo();
    int runnum  = header.RunNo();
    int evtnum  = header.EvtNo();
    float eher1 = header.EHER();
    float eler1 = header.ELER();
    int mask = 0xfffffff;
    evtnum = evtnum & mask;

    int size;
    minmax_evt* mm_evt
      = (minmax_evt*)BASF_Sharedmem->get_pointer(BASF_Sharedmem->get_id(),
                                                 "minmax_evt", &size);

    if (isfirst) {
      mm_evt->min = evtnum;
      mm_evt->max = evtnum;
      isfirst = false;
    } else {
      if (evtnum < mm_evt->min) mm_evt->min = evtnum;
      if (evtnum > mm_evt->max) mm_evt->max = evtnum;
    }
  }

//////////////////////////////////////////////////////////////////////
// End of run handling
//////////////////////////////////////////////////////////////////////

  void FindAttributes::end_run(BelleEvent*, int*)
  {
  }

//////////////////////////////////////////////////////////////////////
// Others
//////////////////////////////////////////////////////////////////////

  void FindAttributes::other(int*, BelleEvent*, int*)
  {
  }

#if defined(BELLE_NAMESPACE)
}
#endif
