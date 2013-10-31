#include "TTDStatus.h"

extern "C" {
#include <nsm2.h>
#include "pocket_ttd.h"
}

#include "base/NSMNode.h"

#include "base/Debugger.h"

#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace Belle2;

TTDStatus::TTDStatus(const std::string& data_name) throw()
  : NSMData(data_name, "pocket_ttd", pocket_ttd_revision)
{
  _status = malloc(sizeof(pocket_ttd));
}

TTDStatus::~TTDStatus() throw()
{
  free(_status);
}

void TTDStatus::read(NSMNode*) throw(NSMHandlerException)
{
  const pocket_ttd* status = (const pocket_ttd*)get();
  /*
  std::cout << __FILE__ << ":" << __LINE__ << std::endl
      << " exp_number = " << status->exp_number << std::endl
      << " run_number = " << status->run_number << std::endl
      << " evt_number = " << status->evt_number << std::endl
      << " evt_total  = " << status->evt_total << std::endl
      << " ftswid     = " << status->ftswid << std::endl
      << " isrunning  = " << (status->isrunning?"running":"not running") << std::endl
      << " utime      = " << status->utime << std::endl
      << " ctime      = " << status->ctime << std::endl
      << " udead      = " << status->udead << std::endl
      << " cdead      = " << status->cdead << std::endl;
  */
  memcpy(_status, status, sizeof(pocket_ttd));
}

void TTDStatus::write(NSMNode*) throw(NSMHandlerException)
{
  pocket_ttd* status = (pocket_ttd*)get();
  memcpy(status, _status, sizeof(pocket_ttd));
}

void TTDStatus::serialize(NSMNode* node, int& npar, int* pars, std::string& data)
throw(IOException)
{
  data = "";
  npar = 0;
  pars[npar++] = node->getIndex();
  pocket_ttd* ttd_status = (pocket_ttd*)_status;
  pars[npar++] = ttd_status->isrunning;
  /*
  pars[npar++] = ttd_status->sigpipe;
  pars[npar++] = ttd_status->rsv2;
  pars[npar++] = ttd_status->rsv3;
  pars[npar++] = ttd_status->b2ebport;
  pars[npar++] = ttd_status->b2ebsock;
  pars[npar++] = ttd_status->b2ebip;
  pars[npar++] = ttd_status->myip;
  */
  pars[npar++] = ttd_status->exp_number;
  pars[npar++] = ttd_status->run_number;
  pars[npar++] = ttd_status->run_count;
  pars[npar++] = ttd_status->evt_number;
  pars[npar++] = ttd_status->evt_total;
  pars[npar++] = ttd_status->ftswid;
  /*
  pars[npar++] = ttd_status->cpldver;
  pars[npar++] = ttd_status->conf;
  pars[npar++] = ttd_status->cclk;
  pars[npar++] = ttd_status->fpgaid;
  pars[npar++] = ttd_status->fpgaver;
  pars[npar++] = ttd_status->setutim;
  pars[npar++] = ttd_status->clkfreq;
  */
  pars[npar++] = ttd_status->utime;
  pars[npar++] = ttd_status->ctime;
  pars[npar++] = ttd_status->udead;
  pars[npar++] = ttd_status->cdead;
  /*
  pars[npar++] = ttd_status->omask;
  pars[npar++] = ttd_status->stamisc;
  pars[npar++] = ttd_status->jtag;
  pars[npar++] = ttd_status->jtdo;
  pars[npar++] = ttd_status->jctl;
  pars[npar++] = ttd_status->jreg;
  pars[npar++] = ttd_status->jrst;
  pars[npar++] = ttd_status->jsta;
  pars[npar++] = ttd_status->reset;
  pars[npar++] = ttd_status->utimrst;
  pars[npar++] = ttd_status->ctmirst;
  pars[npar++] = ttd_status->utimerr;
  pars[npar++] = ttd_status->ctmierr;
  pars[npar++] = ttd_status->errsrc;
  pars[npar++] = ttd_status->tluctrl;
  pars[npar++] = ttd_status->tlustat;
  pars[npar++] = ttd_status->trgset;
  pars[npar++] = ttd_status->tlimit;
  pars[npar++] = ttd_status->tincnt;
  pars[npar++] = ttd_status->toutcnt;
  pars[npar++] = ttd_status->tlast;
  pars[npar++] = ttd_status->stafifo;
  pars[npar++] = ttd_status->enstat;
  pars[npar++] = ttd_status->revopos;
  pars[npar++] = ttd_status->revoin;
  pars[npar++] = ttd_status->setaddr;
  pars[npar++] = ttd_status->setcmd;
  pars[npar++] = ttd_status->lckfreq;
  pars[npar++] = ttd_status->stat;
  pars[npar++] = ttd_status->linkup;
  pars[npar++] = ttd_status->error;
  pars[npar++] = ttd_status->acksig;
  for (int i = 0; i < 10; i++) {
    pars[npar++] = ttd_status->staa[i];
    pars[npar++] = ttd_status->stab[i];
    pars[npar++] = ttd_status->maxt[i];
  }
  */
}

void TTDStatus::deserialize(NSMNode* node, int npar, int* pars, const std::string& data)
throw(IOException)
{

}

int TTDStatus::getEventNumber()
{
  return ((pocket_ttd*)_status)->evt_number;
}

int TTDStatus::getEventTotal()
{
  return ((pocket_ttd*)_status)->evt_total;
}

bool TTDStatus::isRunning()
{
  return ((pocket_ttd*)_status)->isrunning;
}
