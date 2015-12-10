#include "belle.h"
#include "event/BelleEvent.h"

#include "basf/module.h"
#include "basf/module_descr.h"

#include "tuple/BelleTupleManager.h"

#include "ip/IpProfile.h"

#include "particle/Particle.h"
#include "particle/utility.h"

#include "particle/PID.h"

#include MDST_H
#include "mdst/mdst.h"
#include "eid/eid.h"
#include "mdst/Muid_mdst.h"
#include "kid/atc_pid.h"
#include "kid/kid_mon_util.h"

#include <vector>

using namespace std;

#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif


  class TrackMonitor : public Module {
  public:
    TrackMonitor(void);
    ~TrackMonitor(void) {};
    void init(int* dummy) {};
    void term(void) {};
    void disp_stat(const char* cmd) {};
    void hist_def(void);
    void event(BelleEvent*, int*);
    void begin_run(BelleEvent* ptr , int* status);
    void end_run(BelleEvent* ptr, int* bla) {*bla = 0;};
    void other(int* rec, BelleEvent* ptr, int* status) {};

  private:
    // PID
    BelleHistogram* h501;
    BelleHistogram* h502;
    BelleHistogram* h503;
    BelleHistogram* h504;
    BelleHistogram* h505;
    BelleHistogram* h506;

    // 4-momentum, position
    BelleHistogram* h511;
    BelleHistogram* h512;
    BelleHistogram* h513;
    BelleHistogram* h514;
    BelleHistogram* h515;
    BelleHistogram* h516;
    BelleHistogram* h517;

    // 7*7 covariance matrix
    BelleHistogram* h521;
    BelleHistogram* h522;
    BelleHistogram* h523;
    BelleHistogram* h524;
    BelleHistogram* h525;
    BelleHistogram* h526;
    BelleHistogram* h527;
    BelleHistogram* h528;
    BelleHistogram* h529;
    BelleHistogram* h530;
    BelleHistogram* h531;
    BelleHistogram* h532;
    BelleHistogram* h533;
    BelleHistogram* h534;
    BelleHistogram* h535;
    BelleHistogram* h536;
    BelleHistogram* h537;
    BelleHistogram* h538;
    BelleHistogram* h539;
    BelleHistogram* h540;
    BelleHistogram* h541;
    BelleHistogram* h542;
    BelleHistogram* h543;
    BelleHistogram* h544;
    BelleHistogram* h545;
    BelleHistogram* h546;
    BelleHistogram* h547;
    BelleHistogram* h548;

  };

  TrackMonitor::TrackMonitor(void) {}

//------------------ code -----------------------------------------------
  void TrackMonitor::begin_run(BelleEvent*, int* status)
  {
    *status = 0;
    IpProfile::begin_run();
    eid::init_data();
  }

  void TrackMonitor::event(BelleEvent*, int* status)
  {
    *status = 0;

    atc_pid sel_kapi(3, 1, 5, 3, 2);
    atc_pid sel_prka(3, 1, 5, 4, 3);
    atc_pid sel_prpi(3, 1, 5, 4, 2);

    Mdst_charged_Manager& chgmgr = Mdst_charged_Manager::get_manager();

    vector<Particle> pions;

    for (std::vector<Mdst_charged>::const_iterator it = chgmgr.begin();
         it != chgmgr.end(); it++) {

      if (it->charge() > 0) {
        Particle pi(*it, Ptype(+211));
        pions.push_back(pi);
      } else {
        Particle pi(*it, Ptype(-211));
        pions.push_back(pi);
      }

    }

    for (unsigned i = 0; i < pions.size(); i++) {

      Particle PI = pions[i];

      double pid_kapi = sel_kapi.prob(&PI.mdstCharged());
      double pid_prka = sel_prka.prob(&PI.mdstCharged());
      double pid_prpi = sel_prpi.prob(&PI.mdstCharged());
      eid sel_e(PI.mdstCharged());
      Muid_mdst sel_mu(PI.mdstCharged());
      double e_like = sel_e.prob(3, -1, 5);
      double m_like = sel_mu.Muon_likelihood();

      HepSymMatrix err(7, 0);
      err = PI.momentum().dpx();

      h501->accumulate(pid_kapi,   1.0);
      h502->accumulate(pid_prka,   1.0);
      h503->accumulate(pid_prpi,   1.0);
      h504->accumulate(e_like,     1.0);
      if (sel_mu.Chi_2() != -1)
        h505->accumulate(m_like,     1.0);
      else
        h505->accumulate(0,          1.0);
      if (sel_mu.Chi_2() > 0)
        h506->accumulate(1,          1.0);
      else
        h506->accumulate(0,          1.0);

      h511->accumulate(PI.px(),    1.0);
      h512->accumulate(PI.py(),    1.0);
      h513->accumulate(PI.pz(),    1.0);
      h514->accumulate(PI.e(),     1.0);
      h515->accumulate(PI.x().x(), 1.0);
      h516->accumulate(PI.x().y(), 1.0);
      h517->accumulate(PI.x().z(), 1.0);

      h521->accumulate(err[0][0],  1.0);
      h522->accumulate(err[0][1],  1.0);
      h523->accumulate(err[0][2],  1.0);
      h524->accumulate(err[0][3],  1.0);
      h525->accumulate(err[0][4],  1.0);
      h526->accumulate(err[0][5],  1.0);
      h527->accumulate(err[0][6],  1.0);
      h528->accumulate(err[1][1],  1.0);
      h529->accumulate(err[1][2],  1.0);
      h530->accumulate(err[1][3],  1.0);
      h531->accumulate(err[1][4],  1.0);
      h532->accumulate(err[1][5],  1.0);
      h533->accumulate(err[1][6],  1.0);
      h534->accumulate(err[2][2],  1.0);
      h535->accumulate(err[2][3],  1.0);
      h536->accumulate(err[2][4],  1.0);
      h537->accumulate(err[2][5],  1.0);
      h538->accumulate(err[2][6],  1.0);
      h539->accumulate(err[3][3],  1.0);
      h540->accumulate(err[3][4],  1.0);
      h541->accumulate(err[3][5],  1.0);
      h542->accumulate(err[3][6],  1.0);
      h543->accumulate(err[4][4],  1.0);
      h544->accumulate(err[4][5],  1.0);
      h545->accumulate(err[4][6],  1.0);
      h546->accumulate(err[5][5],  1.0);
      h547->accumulate(err[5][6],  1.0);
      h548->accumulate(err[6][6],  1.0);

    }

  }

  void TrackMonitor::hist_def(void)
  {
    extern BelleTupleManager* BASF_Histogram;

    h501 = (*BASF_Histogram).histogram("PID kapi", 100,   0.0,  1.0, 501);
    h502 = (*BASF_Histogram).histogram("PID prka", 100,   0.0,  1.0, 502);
    h503 = (*BASF_Histogram).histogram("PID prpi", 100,   0.0,  1.0, 503);
    h504 = (*BASF_Histogram).histogram("PID e",    100,   0.0,  1.0, 504);
    h505 = (*BASF_Histogram).histogram("PID mu",   100,   0.0,  1.0, 505);
    h506 = (*BASF_Histogram).histogram("PID mu Q", 100,   0.0,  1.0, 506);

    h511 = (*BASF_Histogram).histogram("px",       100,  -3.0,  3.0, 511);
    h512 = (*BASF_Histogram).histogram("py",       100,  -3.0,  3.0, 512);
    h513 = (*BASF_Histogram).histogram("pz",       100,  -3.0,  3.0, 513);
    h514 = (*BASF_Histogram).histogram("E",        100,   0.0,  4.0, 514);
    h515 = (*BASF_Histogram).histogram("x",        100,  -5.0,  5.0, 515);
    h516 = (*BASF_Histogram).histogram("y",        100,  -5.0,  5.0, 516);
    h517 = (*BASF_Histogram).histogram("z",        100,  -5.0,  5.0, 517);

    h521 = (*BASF_Histogram).histogram("Cov 00",   100,  -0.0001, 0.0001, 521);
    h522 = (*BASF_Histogram).histogram("Cov 01",   100,  -0.0001, 0.0001, 522);
    h523 = (*BASF_Histogram).histogram("Cov 02",   100,  -0.0001, 0.0001, 523);
    h524 = (*BASF_Histogram).histogram("Cov 03",   100,  -0.0001, 0.0001, 524);
    h525 = (*BASF_Histogram).histogram("Cov 04",   100,  -0.0001, 0.0001, 525);
    h526 = (*BASF_Histogram).histogram("Cov 05",   100,  -0.0001, 0.0001, 526);
    h527 = (*BASF_Histogram).histogram("Cov 06",   100,  -0.0001, 0.0001, 527);
    h528 = (*BASF_Histogram).histogram("Cov 11",   100,  -0.0001, 0.0001, 528);
    h529 = (*BASF_Histogram).histogram("Cov 12",   100,  -0.0001, 0.0001, 529);
    h530 = (*BASF_Histogram).histogram("Cov 13",   100,  -0.0001, 0.0001, 530);
    h531 = (*BASF_Histogram).histogram("Cov 14",   100,  -0.0001, 0.0001, 531);
    h532 = (*BASF_Histogram).histogram("Cov 15",   100,  -0.0001, 0.0001, 532);
    h533 = (*BASF_Histogram).histogram("Cov 16",   100,  -0.0001, 0.0001, 533);
    h534 = (*BASF_Histogram).histogram("Cov 22",   100,  -0.0001, 0.0001, 534);
    h535 = (*BASF_Histogram).histogram("Cov 23",   100,  -0.0001, 0.0001, 535);
    h536 = (*BASF_Histogram).histogram("Cov 24",   100,  -0.0001, 0.0001, 536);
    h537 = (*BASF_Histogram).histogram("Cov 25",   100,  -0.0001, 0.0001, 537);
    h538 = (*BASF_Histogram).histogram("Cov 26",   100,  -0.0001, 0.0001, 538);
    h539 = (*BASF_Histogram).histogram("Cov 33",   100,  -0.0001, 0.0001, 539);
    h540 = (*BASF_Histogram).histogram("Cov 34",   100,  -0.0001, 0.0001, 540);
    h541 = (*BASF_Histogram).histogram("Cov 35",   100,  -0.0001, 0.0001, 541);
    h542 = (*BASF_Histogram).histogram("Cov 36",   100,  -0.0001, 0.0001, 542);
    h543 = (*BASF_Histogram).histogram("Cov 44",   100,  -0.0001, 0.0001, 543);
    h544 = (*BASF_Histogram).histogram("Cov 45",   100,  -0.0001, 0.0001, 544);
    h545 = (*BASF_Histogram).histogram("Cov 46",   100,  -0.0001, 0.0001, 545);
    h546 = (*BASF_Histogram).histogram("Cov 55",   100,  -0.0001, 0.0001, 546);
    h547 = (*BASF_Histogram).histogram("Cov 56",   100,  -0.0001, 0.0001, 547);
    h548 = (*BASF_Histogram).histogram("Cov 66",   100,  -0.0001, 0.0001, 548);

  }

  extern "C" Module_descr* mdcl_TrackMonitor()
  {
    TrackMonitor* module = new TrackMonitor;
    Module_descr* dscr = new Module_descr("TrackMonitor", module);
    basf_save_mdsc(dscr);
    return dscr;
  }
#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
