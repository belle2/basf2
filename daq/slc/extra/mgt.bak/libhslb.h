/*
  libhslb.h

  HSLB and FEE register read/write functions
*/

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__dummy_close_bracket_to_cheat_emacs_auto_indent)
}
#endif

#define M012_SERIAL      7
#define M012_SELECTMAP   6

const char* hslberr();
int openfn(int fd, int readonly);
int readfn(int fd, int adr);
int writefn(int fd, int adr, int val);
int readfn32(int fd, int adr);
int writefn32(int fd, int adr, int val);
int hswait_quiet(int fd);
int hswait(int fd);
int readfee8(int fd, int adr);
int writefee8(int fd, int adr, int val);
int readfee32(int fd, int adr, int* valp);
int writefee32(int fd, int adr, int val);
int writestream(int fd, char* filename);
void writefpga(int fndev, int m012, int ch, int n);
void dumpfpga(int conf, char* str);
int bootfpga(int fndev, const char* file,
             int verbose, int forced, int m012);
int checkfee(hslb_info* hslb);
int check_hslb(hslb_info* hslb, char* msg);

#if defined(__dummy_open_bracket_to_cheat_emacs_auto_indent)
__dummy_open_bracket_to_cheat_emacs_auto_indent {
#endif
#if defined(__cplusplus)
}
#endif
