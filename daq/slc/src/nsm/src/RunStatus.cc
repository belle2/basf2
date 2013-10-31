#include "RunStatus.h"

extern "C" {
#include <nsm2.h>
#include "run_status.h"
}

#include <cstring>

using namespace Belle2;

RunStatus::RunStatus(const std::string& data_name) throw()
  : NSMData(data_name, "run_status", run_status_revision)
{
  _serial = 0;
  _exp_no = 0;
  _run_no = 0;
  _start_time = 0;
  _end_time = 0;
  _event_number = 0;
  _event_total = 0;
}

void RunStatus::read() throw(NSMHandlerException)
{
  const run_status* status = (const run_status*)get();
  _exp_no = status->exp_no;
  _run_no = status->run_no;
  _start_time = status->start_time;
  _end_time = status->end_time;
  _event_number = status->event_number;
  _event_total = status->event_total;
}

void RunStatus::write() throw(NSMHandlerException)
{
  run_status* status = (run_status*)get();
  status->exp_no = _exp_no;
  status->run_no = _run_no;
  status->start_time = _start_time;
  status->end_time = _end_time;
  status->event_number = _event_number;
  status->event_total = _event_total;
}

