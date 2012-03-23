// High-level AMGA Library for Belle II
// Author: Jae-Hyuck Kwak (jhkwak@kisti.re.kr)
// Created: 2012-02-27 Initial release

#include <MDClient.h>
#include <iostream>
#include <ctime>
using namespace std;

#define __BELLE_AMGA_DEBUG__

struct struct_attr_filelvl {
  int id;
  char guid[40];
  char lfn[1024];
  char status[16];
  int events;
  int datasetid;
  int stream;
  int runH;
  int eventH;
  int runL;
  int eventL;
  int parentid; /* FIXME: int(128) */
  int softwareid;
  int siteid;
  int userid;
  char log_guid[40];
};

struct struct_attr_dataset {
  int id;
  char name[32];
  char description[1024];
  char type;
  char level[16];
  char mode[16];
  char skim[32];
  int userid;
  time_t date;
};

struct struct_attr_software {
  int id;
  char name[64];
  char version[16];
  char description[1024];
};

struct struct_attr_path {
  char type;
  int exp;
  char path[1024];
};


struct struct_attr_evtlvl {
  int nmichargedtrk;
  int nplchargedtrk;
  int nks;
  int nkl;
  int skimid;
  int r2;
};

struct struct_attr_skim {
  int id;
  char name[32];
  char description[1024];
  int userid;
  time_t date;
};

struct struct_attr_user {
  int id;
  char name[32];
  char description[1024];
  time_t date;
};

struct struct_attr_site {
  int id;
  char name[64];
  char description[1024];
  time_t date;
  unsigned char has_exp_fr[16]; /* FIXME: varbit(128) */
  unsigned char has_exp_fm[16]; /* FIXME: varbit(128) */
};

enum SCHEMAID {
  FILELVL = 1,
  DATASET,
  SOFTWARE,
  PATH,
  EVTLVL,
  SKIM,
  USER,
  SITE
};

class belle_amga {
private:
  MDClient client;

  struct_attr_filelvl filelvl;
  struct_attr_dataset dataset;
  struct_attr_software software;
  struct_attr_path path;
  struct_attr_evtlvl evtlvl;
  struct_attr_skim skim;
  struct_attr_user user;
  struct_attr_site site;

  int write_attr_filelvl(char* entry_name, struct_attr_filelvl& attr_filelvl_list);
  int write_attr_dataset(char* entry_name, struct_attr_dataset& attr_dataset_list);
  int write_attr_software(char* entry_name, struct_attr_software& attr_software_list);
  int write_attr_path(char* entry_name, struct_attr_path& attr_path_list);
  int write_attr_evtlvl(char* entry_name, struct_attr_evtlvl& attr_evtlvl_list);
  int write_attr_skim(char* entry_name, struct_attr_skim& attr_skim_list);
  int write_attr_user(char* entry_name, struct_attr_user& attr_user_list);
  int write_attr_site(char* entry_name, struct_attr_site& attr_site_list);

public:
  belle_amga() {
#ifdef __BELLE_AMGA_DEBUG__
    cout << "[BELLE_AMGA DEBUG] belle_amga constructor called" << endl;
#endif
  }

  belle_amga& getInstance();

  int write_belle_amga(int schemaid, char* entry_name, void* attr_struct);

  ~belle_amga() {
#ifdef __BELLE_AMGA_DEBUG__
    cout << "[BELLE_AMGA DEBUG] belle_amga destructor called" << endl;
#endif
  }
};

