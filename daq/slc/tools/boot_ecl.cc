#include <cstdlib>
#include <cstdio>
#include <cstring>

int cpr_ecl01[] = {5001, 5002, 5003, 5004, 5005, 5006, 5007, 5008, 5009, 5010, 5011, 5012, 5013};
//int cpr_ecl02[] = {5014,5015,5016,5017,5018};
int cpr_ecl02[] = {5014, 5015, 5016, 6001, 5018};

int main(int argc, char** argv)
{
  bool nsm = false;
  bool cpr = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-nsm") == 0) {
      nsm = true;
      continue;
    } else if (strcmp(argv[i], "-cpr") == 0) {
      cpr = true;
      continue;
    }
  }

  for (int i = 1; i < argc; i++) {
    char s[256];
    if (strcmp(argv[i], "ecl01") == 0) {
      if (nsm) {
        system("ssh ecl01 killall nsmd2");
        for (unsigned int i = 0; i < sizeof(cpr_ecl01) / sizeof(int); i++) {
          sprintf(s, "ssh cpr%d killall nsmd2", cpr_ecl01[i]);
          system(s);
        }
        system("ssh ecl01 \"~/.bash_profile; bootnsmd2; bootnsmd2 -g\"");
        for (unsigned int i = 0; i < sizeof(cpr_ecl01) / sizeof(int); i++) {
          sprintf(s, "ssh cpr%d \"~/.bash_profile; bootnsmd2 cpr%d\"",
                  cpr_ecl01[i], cpr_ecl01[i]);
          system(s);
        }
      }
      if (cpr) {
        system("ssh ecl01 \"~/.bash_profile; runcontrold ecl01 -d; rocontrold ropc501 -d\"");
        for (unsigned int i = 0; i < sizeof(cpr_ecl01) / sizeof(int); i++) {
          sprintf(s, "ssh cpr%d \"killall -9 cprcontrold basf2; "
                  "source ~/.bash_profile; cprcontrold cpr%d\"",
                  cpr_ecl01[i], cpr_ecl01[i]);
          system(s);
        }
      }
    } else if (strcmp(argv[i], "ecl02") == 0) {
      if (nsm) {
        system("ssh ecl02 killall nsmd2");
        for (unsigned int i = 0; i < sizeof(cpr_ecl02) / sizeof(int); i++) {
          sprintf(s, "ssh ecl02 \"ssh cpr%d killall nsmd2\"", cpr_ecl02[i]);
          system(s);
        }
        system("ssh ecl02 \"~/.bash_profile; bootnsmd2; bootnsmd2 -g\"");
        for (unsigned int i = 0; i < sizeof(cpr_ecl02) / sizeof(int); i++) {
          sprintf(s, "ssh ecl02 \"ssh cpr%d ~/.bash_profile; bootnsmd2 cpr%d\"",
                  cpr_ecl02[i], cpr_ecl02[i]);
          system(s);
        }
      }
      if (cpr) {
        system("ssh ecl02 \"~/.bash_profile; runcontrold ecl02 -d; rocontrold ropc501 -d\"");
        for (unsigned int i = 0; i < sizeof(cpr_ecl02) / sizeof(int); i++) {
          sprintf(s, "ssh ecl02 \"ssh cpr%d killall -9 cprcontrold basf2; "
                  "source ~/.bash_profile; cprcontrold cpr%d\"",
                  cpr_ecl02[i], cpr_ecl02[i]);
          system(s);
        }
      }
    }
  }
  return 0;
}
