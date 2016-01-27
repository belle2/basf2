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

const char* feename(int hwtype, int fwtype);
int openfn(int fd, int readonly, char* program);
int readfn(int fd, int adr);
int writefn(int fd, int adr, int val);
int readfn32(int fd, int adr);
int writefn32(int fd, int adr, int val);
int hswait_quiet(int fd);
int hswait(int fin, int fd);
int readfee8(int fd, int adr);
int writefee8(int fd, int adr, int val);
int readfee32(int fd, int adr, int* valp);
int writefee32(int fd, int adr, int val);
int writestream(int fd, char* filename);
const char* hslberr();

#if defined(__dummy_open_bracket_to_cheat_emacs_auto_indent)
__dummy_open_bracket_to_cheat_emacs_auto_indent {
#endif
#if defined(__cplusplus)
}
#endif
