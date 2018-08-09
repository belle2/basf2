
#ifndef __nsmparse_h__
#define __nsmparse_h__

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__dummy_close_bracket_to_cheat_emacs_auto_indent)
}
#endif

/* NSMparse */
struct NSMparse_struct {
  /* type is encoded in a char ---
     'c': int8_t,  's': int16_t,  'i': int32_t,  'l': int64_t,
     'C': uint8_t, 'S': uint16_t, 'I': uint16_t, 'L': uint64_t,
     'f': float,   'd': double, '(': nested-begin, ')' nested-end

     nested-begin has NSMparse entry with type = 0,
     size = (sizeof(nested)/sizeof(nested[0])) of nested struct,
     offset = start of nested, name = ""
  */
  char type;
  char name[256];
  int  size;       /* of array */
  int  bytes;      /* of this type */
  int  offset;     /* from the beginning */
  struct NSMparse_struct* next;
};

#ifndef __nsm2_typedef_parse__
#define __nsm2_typedef_parse__
typedef struct NSMparse_struct NSMparse;
#endif /* nsm2_typedef_parse */

NSMparse* nsmlib_parsefile(const char* datname, int revision,
                           const char* incpath, char* fmtstr, int* revisionp);

const char* nsmlib_parseerr(int* code);

#if defined(__dummy_open_bracket_to_cheat_emacs_auto_indent)
__dummy_open_bracket_to_cheat_emacs_auto_indent {
#endif
#if defined(__cplusplus)
}
#endif

#endif /* __nsmparse_h__ */
