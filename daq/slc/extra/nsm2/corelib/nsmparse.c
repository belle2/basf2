/*
  nsmparse.c for NSM2

  20140304 nested struct trial: fmtout is ready, NSMparse is ready
  20140304 nested struct trial: fmtout is ready, NSMparse is not yet
  20140304 multi-dim array
  20140304 a simple #define can be used
  20131229 stdint.h definitions are added
  20140428 nsmparse_malloc fix
  20140902 memset fix (T.Konno)
  20140903 n_same fix for nested struct
  20140917 -1 to skip revision check
  20140918 nsmparse_struct fix (uninitialized malloc for ->next)
  20140919 bytes is added to NSMparse
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "nsmparse.h"

static int  nsmparse_errcode;
static char nsmparse_errstr[256];

#define NSMENV_INCDIR "NSM2_INCDIR"

#define  NSMEPARSENOFILE (-10)
#define  NSMEPARSECOM    (-11)
#define  NSMEPARSENOREV  (-12)
#define  NSMEPARSEREV    (-13)
#define  NSMEPARSENOSTR  (-14)
#define  NSMEPARSEITEM   (-15)

typedef struct define_struct {
  char *name;
  int val;
  struct define_struct *next;
} define_t;

define_t define_top;
define_t *define_ptr = &define_top;

#define isa_num(a) (isalnum(a) || (a) == '_')

/* -- malloc ---------------------------------------------------------- */
static char *
nsmparse_malloc(size_t siz, const char *where, const char *text)
{
  char *p = (char *)malloc(siz);
  if (! p) {
    printf("nsmparse_malloc: can't malloc %d bytes%s%s%s%s\n",
	   (int)siz,
	   where ? " in " : "", where ? where : "",
	   text  ? ": "   : "", text  ? text  : "");
    exit(1);
  }
  memset(p, 0, siz);
  return p;
}
/* -- eval ------------------------------------------------------------- *\
   this can be extended to evaluate a math expression
\* --------------------------------------------------------------------- */
static int
nsmparse_eval(const char *p)
{
  return atoi(p);
}
/* -- readfile -------------------------------------------------------- */
static char *
nsmparse_readfile(const char *file, off_t *filelenp)
{
  struct stat statbuf;
  char *filebuf;
  int fd;
  int i;
  int len;
  int blksiz = 65536;
  
  if (filelenp) *filelenp = 0;

  if (stat(file, &statbuf) < 0) {
    printf("can't stat file %s\n", file);
    return 0;
  }
  
  if (statbuf.st_size > 1024*1024) {
    printf("file %s too large (%ld bytes)\n", file, statbuf.st_size);
    return 0;
  }
  
  if ((fd = open(file, O_RDONLY)) < 0) {
    printf("can't open file %s\n", file);
    return 0;
  }

  filebuf = nsmparse_malloc(statbuf.st_size + 1, "readfile", file);
  if (! filebuf) {
    close(fd);
    printf("can't alloc %ld bytes\n", statbuf.st_size + 1);
    return 0;
  }
  
  for (i = 0; i < statbuf.st_size; i += blksiz) {
    int siz = (i + blksiz < statbuf.st_size) ? blksiz : statbuf.st_size - i;
    len = read(fd, filebuf + i, siz);
    if (len != siz) {
      printf("can't read at %d bytes\n", i + (len > 0 ? len : 0));
      close(fd);
      free(filebuf);
      return 0;
    }
  }
  close(fd);

  if (filelenp) *filelenp = statbuf.st_size;
  filebuf[statbuf.st_size] = 0;
  return filebuf;
}
/* -- nsmparse_cleanup ----------------------------------------------- */
static int
nsmparse_cleanup(char *filebuf, off_t *filelenp)
{
  char *p;
  char *q;
  char *z = filebuf + *filelenp;
  int inquote = 0;
  int addnl = 0;

  /* delete comments and quoted strings */
  
  for (p = q = filebuf; p < z; ) {
    if (p[0] == '"' && ! inquote) {
      inquote = 2;
    } else if (p[0] == '\'' && ! inquote) {
      inquote = 1;
    } else if (inquote == 1) {
      if (p[0] == '\\' && p[1]) p[1] = ' ';
      if (p[0] == '\'')
	inquote = 0;
      else if (p[0] != '\n')
	p[0] = ' ';
    } else if (inquote == 2) {
      if (p[0] == '\\' && p[1]) p[1] = ' ';
      if (p[0] == '"')
	inquote = 0;
      else if (p[0] != '\n')
	p[0] = ' ';
    } else if (p[0] == '/' && p[1] == '/') {
      while (p < z && *p != '\n') p++;
      continue;
    } else if (p[0] == '/' && p[1] == '*') {
      p += 4;
      while (p+1 < z && ! (p[0] == '*' && p[1] == '/')) {
	if (*p == '\n') addnl++;
	p++;
      }
      if (p+1 == z) return -1;
      p += 2;
      continue;
    } else if (p[0] == '\n' && addnl) {
      while (addnl-- > 0) *q++ = '\n'; /* too keep track of line number */
    }
      
    if (q != p) *q = *p;
    p++;
    q++;
  }

  if (inquote) return -1;
 
  *filelenp -= (p - q);
  *q = 0;

  /* delete duplicated spaces */
  z = filebuf + *filelenp;
  
  for (p = q = filebuf; p < z; p++, q++) {
    int isnl = 0;
    if ((p[0] == '"' || p[0] == '\'') && ! inquote) {
      inquote = 1;
    } else if (inquote) {
      if (p[0] == '"' || p[0] == '\'') inquote = 0;
    } else {
      while (p < z && isspace(p[0]) && isspace(p[1]) &&
	     (p[0] != p[1] || p[0] != '\n')) {
	if (p[0] == '\n' || p[1] == '\n') isnl = 1;
	p++;
      }
    }
    *q = isnl ? '\n' : *p;
  }
  *filelenp -= (p - q);
  *q = 0;

  return 0;
}
/* -- nsmparse_error -------------------------------------------------- */
static void
nsmparse_error(char *fmt, const char *file, char *filebuf, char *p)
{
  char *q;
  char *token;
  int line = 1;
  for (q = filebuf; q < p; q++) if (*q == '\n') line++;
  for (q = p; isa_num(*q); q++);
  token = nsmparse_malloc(q - p + 1, "error", file);
  strncpy(token, p, q - p);
  token[q - p] = 0;
  sprintf(nsmparse_errstr, "Error:%s:%d: ", file, line);
  sprintf(nsmparse_errstr+strlen(nsmparse_errstr), fmt, token);
  free(token);
}
/* -- getname ---------------------------------------------------------- *\
\* --------------------------------------------------------------------- */
char *
nsmparse_getname(const char *file, char *filebuf,
                 char *ptr, char *dest, int siz)
{
  char *q;
  if (! isalpha(*ptr) && *ptr != '_') {
    nsmparse_error("valid name not found", file, filebuf, ptr);
    return 0;
  }
  for (q = dest; isa_num(*ptr); ) {
    *q++ = *ptr++;
    if (q >= dest + siz) {
      nsmparse_error("too long variable name", file, filebuf, ptr);
      return 0;
    }
  }
  *q = 0;
  while (isspace(*ptr)) ptr++;
  return ptr;
}
/* -- nsmparse_revision ---------------------------------------------- */
static int
nsmparse_revision(const char *file, char *filebuf, const char *datname)
{
  int len = strlen(datname);
  char *p = filebuf;
  int num;

  while (p = strstr(p, datname)) {
    if ((p == filebuf || ! isa_num(*(p-1))) &&
	strncmp(p+len, "_revision", 9) == 0) {
      p += len + 9;
      if (isspace(*p)) p++;
      if (*p != '=') {
	nsmparse_error("'=' expected but not found.", file, filebuf, p);
	return -1;
      }
      if (isspace(*++p)) p++;
      num = strtoul(p, 0, 0);
      if (num <= 0) {
	sprintf(nsmparse_errstr, "Error:%s: invalid %s_revision",
		file, datname);
	return -1;
      }
      return num;
    }
    p += len;
  }
  sprintf(nsmparse_errstr, "Error:%s: %s_revision not found", file, datname);
  return -1;
}
/* -- nsmparse_struct ------------------------------------------------- *\
   parse until "struct <datname> {" is found, and also collect defines
\* -------------------------------------------------------------------- */
static char *
nsmparse_struct(char *filebuf, const char *datname)
{
  int datlen = strlen(datname);
  char *p = filebuf;
  char *structp = 0;
  char *definep = 0;

  while (p) {
    structp = strstr(p, "struct");
    definep = strstr(p, "\n#define");
    
    if (definep && definep < structp) {
      int val;
      int namelen;
      /* skip "\n#define " */
      p = definep + 8;
      while (isspace(*p))   p++;
      /* get macro name */
      definep = p;
      while (! isspace(*p)) p++;
      namelen = p - definep;
      
      while (isspace(*p))   p++;
      val = nsmparse_eval(p);
      if (val <= 0) continue;

      define_ptr->next
	= (define_t *)nsmparse_malloc(sizeof(define_t), "define", "t");
      define_ptr->name = nsmparse_malloc(namelen + 1, "define", "name");
      strncpy(define_ptr->name, definep, namelen);
      define_ptr->name[namelen] = 0;
      define_ptr->val = val;

      /* printf("name %s val %d\n", define_ptr->name, define_ptr->val); */
      define_ptr = define_ptr->next;
      
      continue;
    }
    if (! structp) break;

    /* skip struct */
    p = structp + 6;
    /* check if "struct" is not a part of a word */
    if ((p - 6) != filebuf && isa_num(*(p-7))) continue;
    if (! isspace(*p)) continue;
    /* check the struct for datname */
    if (strncmp(++p, datname, datlen) != 0) continue;
    p += datlen;
    /* skip "{" and surrounding space */
    while (isspace(*p)) p++;
    if (*p != '{') continue;
    p++;
    while (isspace(*p)) p++;
    return p;
  }

  return 0;
}
/* -- nsmparse_dimen -------------------------------------------------- *\
   
\* -------------------------------------------------------------------- */
static int
nsmparse_dimen(const char *file, char *filebuf, char *ptr, char **endp)
{
  int grandnum = 1;
  char *q = 0;
  
  while (*ptr == '[') { /* for multi-dim array */
    int num = 0;
    if (isspace(*++ptr)) ++ptr;
    if (! isdigit(*ptr)) {
      define_t *dp;
      for (dp = &define_top; dp->name; dp = dp->next) {
	int len = strlen(dp->name);
	/* printf("dp->name = %s\n", dp->name); */
	if (strncmp(dp->name, ptr, len) == 0 && ! isa_num(ptr[len])) {
	  num = dp->val;
	  q = ptr + len;
	  break;
	}
      }
      if (num == 0) {
	nsmparse_error("invalid array size", file, filebuf, ptr);
	return -1;
      }
    } else {
      num = strtoul(ptr, &q, 0); /* char **endptr = &q */
    }
    if (isspace(*q)) q++;
    if (*q != ']') {
      nsmparse_error("invalid array size", file, filebuf, ptr);
      return -1;
    }
    if (isspace(*++q)) ++q;
    grandnum *= num;
    
    if (*q == '[') {
      ptr = q;
      continue;
    }
    
    if (*q != ';') {
      nsmparse_error("semicolon not found", file, filebuf, ptr);
      return -1;
    }
  }
  
  ptr = q+1;
  while (isspace(*ptr)) ptr++;
      
  if (endp) *endp = ptr;
  return grandnum;
}

/* -- nsmparse_scan --------------------------------------------------- */
static NSMparse *
nsmparse_scan(const char *file, char *filebuf, char *start, char **endp,
	      char *fmtout, int *fmtsiz)
{
  char *ptr = start;
  char *q = 0; /* temporary pointer */
  struct types_t { char *name; int siz; char sym; char type; };
  struct types_t *typep;
  static struct types_t types[] = {
    { "int64",  8, 'd', 'l' },
    { "int32",  4, 'i', 'i' },
    { "int16",  2, 's', 's' },
    { "char",   1, 'a', 'c' },
    { "uint64", 8, 'd', 'L' },
    { "uint32", 4, 'i', 'I' },
    { "uint16", 2, 's', 'S' },
    { "uchar",  1, 'a', 'C' },
    { "byte8",  1, 'a', 'C' },
    { "int64_t",  8, 'd', 'l' },
    { "int32_t",  4, 'i', 'i' },
    { "int16_t",  2, 's', 's' },
    { "int8_t",   1, 'a', 'c' },
    { "uint64_t", 8, 'd', 'L' },
    { "uint32_t", 4, 'i', 'I' },
    { "uint16_t", 2, 's', 'S' },
    { "uint8_t",  1, 'a', 'C' },
    { "double", 8, 'd', 'd' },
    { "float",  4, 'i', 'f' },
    { 0, 0, 0 }};
  char sym_prev = 0;
  int n_same = 0;
  char fmtstr[256];
  int fmtlen = 63;
  int offset = 0;
  NSMparse *parsetop = 0;
  NSMparse *parsep = 0;

  *fmtstr = 0;
  
  while (1) {
    if (*ptr == '}') break;

    if (strncmp(ptr, "struct", 6) == 0) {
      ptr += 6;
      while (isspace(*ptr)) ptr++;

      if (parsep) {
	parsep->next = (NSMparse *)nsmparse_malloc(sizeof(NSMparse),
						   "scan", file);
	parsep = parsep->next;
      } else {
	parsetop = (NSMparse *)nsmparse_malloc(sizeof(NSMparse),
					       "scan", file);
	parsep = parsetop;
      }
      parsep->type = '(';
      parsep->bytes = 0;
      parsep->offset = offset; /* probably not correct */
      
      if (isalnum(*ptr) || *ptr == '_') {
	if (! (ptr = nsmparse_getname(file, filebuf, ptr, parsep->name,
				      sizeof(parsep->name)))) {
	  goto parseerr_return;
	}
      } else {
	parsep->name[0] = 0;
      }
      while (isspace(*ptr)) ptr++;

      if (*ptr == '{') {
	static char fmtout2[256];
	NSMparse *parse2 = 0;
	char *nestp = 0;
	int num = -1;
	int siz2 = 0;

	ptr++;
	while (isspace(*ptr)) ptr++;
	parsep->next = nsmparse_scan(file, filebuf, ptr, &ptr, fmtout2, &siz2);
	while (parsep->next) parsep = parsep->next;
	parsep->next = (NSMparse *)nsmparse_malloc(sizeof(NSMparse),
						   "scan", file);
	parsep = parsep->next;
	parsep->type = ')';
        parsep->bytes = 0;
	parsep->offset = offset; /* probably not correct */
	
	ptr++;
	while (isspace(*ptr)) ptr++;
	if (! (ptr = nsmparse_getname(file, filebuf, ptr, parsep->name,
				      sizeof(parsep->name)))) {
	  goto parseerr_return;
	}
	while (isspace(*ptr)) ptr++;
	if (*ptr == '[') {
	  num = nsmparse_dimen(file, filebuf, ptr, &ptr);
	}
	parsep->size = num;
	offset += num > 0 ? siz2 * num : siz2;
	/* printf("<offset += %d * %d = %d>\n", siz2, num, offset); */

	/* check if really nested struct is needed */
	for (nestp = &fmtout2[1]; isdigit(*nestp); nestp++)
	  ;

	if (sym_prev && (*nestp || sym_prev != fmtout2[0])) {
	  sprintf(fmtstr+strlen(fmtstr), "%c", sym_prev);
	  if (n_same > 1) sprintf(fmtstr+strlen(fmtstr), "%d", n_same);
          n_same = 0;
	}
	if (! *nestp) {
	  sym_prev = fmtout2[0];
	  n_same += (fmtout2[1] ? atoi(&fmtout2[1]) : 1) * num;
	  /* printf("<%c:%d>\n", sym_prev, n_same); */
	} else {
	  sprintf(fmtstr+strlen(fmtstr), "(%s)%d", fmtout2, num);
	  sym_prev = 0;
	  n_same = 0;
	}
	
	continue;
      }
    }
    
    while (isspace(*ptr)) ptr++;
    
    for (typep = types; typep->name; typep++) {
      int len = strlen(typep->name);
      if (strncmp(ptr, typep->name, len) == 0 && isspace(ptr[len])) {
	if (parsep) {
	  parsep->next = (NSMparse *)nsmparse_malloc(sizeof(NSMparse),
						     "scan", file);
	  parsep = parsep->next;
	} else {
	  parsetop = (NSMparse *)nsmparse_malloc(sizeof(NSMparse),
						 "scan", file);
	  parsep = parsetop;
	}
	parsep->type = typep->type;
        parsep->bytes = typep->siz;
	parsep->offset = offset;
	ptr += len+1;
	break;
      }
    }
    if (! typep->name) {
      nsmparse_error("'%s' is not a valid type", file, filebuf, ptr);
      goto parseerr_return;
    }
    
    if (! (ptr = nsmparse_getname(file, filebuf,
				  ptr, parsep->name, sizeof(parsep->name)))) {
      goto parseerr_return;
    }
    if (*ptr == ';') {
      /* printf("%s %d\n", typep->name, typep->siz); */
      if (isspace(*++ptr)) ptr++;

      if (typep->sym != sym_prev && n_same) {
	sprintf(fmtstr+strlen(fmtstr), "%c", sym_prev);
	if (n_same > 1) sprintf(fmtstr+strlen(fmtstr), "%d", n_same);
	n_same = 0;
	if (strlen(fmtstr) > fmtlen) {
	  nsmparse_error("struct size exceeded", file, filebuf, ptr);
	  return 0;
	}
      }
      sym_prev = typep->sym;
      parsep->size = -1; /* for a non-array entry */
      offset += typep->siz;
      n_same++;
    } else if (*ptr == '[') {
      int grandnum = nsmparse_dimen(file, filebuf, ptr, &ptr);
      if (grandnum < 0) goto parseerr_return;
      
      /* printf("%s %d %d\n", typep->name, typep->siz, num); */
      if (typep->sym != sym_prev && n_same) {
	sprintf(fmtstr+strlen(fmtstr), "%c", sym_prev);
	if (n_same > 1) sprintf(fmtstr+strlen(fmtstr), "%d", n_same);
	n_same = 0;
	if (strlen(fmtstr) > fmtlen) {
	  nsmparse_error("struct size exceeded", file, filebuf, ptr);
	  goto parseerr_return;
	}
      }
      sym_prev = typep->sym;
      parsep->size = grandnum; /* for a non-array entry */
      offset += typep->siz * grandnum;
      n_same += grandnum;
    } else {
      nsmparse_error("semicolon not found", file, filebuf, ptr);
      goto parseerr_return;
    }
  }

  if (n_same) {
    sprintf(fmtstr+strlen(fmtstr), "%c", sym_prev);
    if (n_same > 1) sprintf(fmtstr+strlen(fmtstr), "%d", n_same);
  }

  if (strlen(fmtstr) > fmtlen) {
    nsmparse_error("struct size exceeded", file, filebuf, ptr);
    goto parseerr_return;
  }

  /*
    fmtout must have 256 byte size
  */
  if (fmtout) strcpy(fmtout, fmtstr);
  if (fmtsiz) *fmtsiz = offset;
  if (endp) *endp = ptr;
  return parsetop;

 parseerr_return:
  while (parsetop) {
    parsep = parsetop->next;
    free(parsetop);
    parsetop = parsep;
  }
  if (fmtout) *fmtout = 0;
  return 0;
}
/* -- nsmparse_findfile ------------------------------------------- */
static char *
nsmparse_findfile(const char *name, const char *incpath)
{
  int i;
  int len = strlen(name);
  char *path;
  struct stat statbuf;
  
  for (i=0; i<3; i++) {
    if (incpath) {
      const char *p = incpath;
      const char *q;
      while (1) {
	q = strchr(p, ':');
	if (! q) q = p + strlen(p);
	path = nsmparse_malloc(q - p + len + 4, "findfile", name);
	strncpy(path, p, q-p);
	path[q-p] = '/';
	strcpy(&path[q-p+1], name);
	strcat(path, ".h");
	if (stat(path, &statbuf) == 0) {
	  return path;
	}
	free(path);
	if (! *q) break;
	p = q+1;
      }
    }
    if (i == 1)
      incpath = getenv(NSMENV_INCDIR);
    else
      incpath = ".";
  }
  return 0;
}

/* -- nsmlib_parsestr ------------------------------------------------- */
static NSMparse *
nsmlib_parsestr(const char *datname, int revision,
		const char *filebuf, const char *filepath, char *fmtstr,
                int *revisionp)
{
  char *datlist;
  char *strbegin;
  int ret;
  off_t filelen;
  char *parsebuf;
  NSMparse *parsep;

  filelen = strlen(filebuf);
  parsebuf = nsmparse_malloc(filelen + 1, "parsestr", datname);
  memcpy(parsebuf, filebuf, filelen + 1);
  
  if (nsmparse_cleanup(parsebuf, &filelen)) {
    nsmparse_errcode = NSMEPARSECOM;
    free(parsebuf);
    return 0;
  }

  if ((ret = nsmparse_revision(filepath, parsebuf, datname)) <= 0) {
    nsmparse_errcode = NSMEPARSENOREV;
    free(parsebuf);
    return 0;
  }

  if (revision != -1 && ret != revision) {
    nsmparse_errcode = NSMEPARSENOREV;
    sprintf(nsmparse_errstr, "revision mismatch, found %d while expecting %d",
	    ret, revision);
    free(parsebuf);
    return 0;
  }

  if (revisionp) *revisionp = ret;
  
  if (! (strbegin = nsmparse_struct(parsebuf, datname))) {
    nsmparse_errcode = NSMEPARSENOSTR;
    free(parsebuf);
    return 0;
  }
  
  if (! (parsep = nsmparse_scan(filepath, parsebuf, strbegin, 0, fmtstr, 0))) {
    nsmparse_errcode = NSMEPARSEITEM;
    free(parsebuf);
    return 0;
  }
  
  free(parsebuf);
  return parsep;
}
/* -- nsmlib_parse ---------------------------------------------------- */
NSMparse *
nsmlib_parsefile(const char *datname, int revision, const char *incpath,
		 char *fmtstr, int *revisionp)
{
  off_t filelen;
  char *filepath;
  char *filebuf;
  NSMparse *parsep;
  
  filepath = nsmparse_findfile(datname, incpath);
  if (! filepath) {
    nsmparse_errcode = NSMEPARSENOFILE;
    sprintf(nsmparse_errstr, "%s.h not found in the include path", datname);
    return 0;
  }

  filebuf = nsmparse_readfile(filepath, 0); /* no need to retreive length */
  
  if (! filebuf) return 0;

  parsep = nsmlib_parsestr(datname, revision, filebuf, filepath, fmtstr,
                           revisionp);
  free(filebuf);
  free(filepath);

  return parsep;
}
/* -- nsmlib_parseerr ------------------------------------------------- */
const char *
nsmlib_parseerr(int *codep)
{
  if (codep) *codep = nsmparse_errcode;
  return nsmparse_errstr;
}
/* -- main ------------------------------------------------------------ */
#if TESTPARSE
int
main(int argc, char **argv)
{
  char *datname;
  char *incpath;
  char fmtstr[256];
  int revision;
  int newrevision = -1;
  NSMparse *parsep;
  char indent[256];

  memset(fmtstr, 0, sizeof(fmtstr));
  
  if (argc < 3) {
    printf("usage: %s <data-name> <revision> [<path-list>]\n", argv[0]);
    return 1;
  }

  datname = argv[1];
  revision = atoi(argv[2]);
  incpath = (argc == 4) ? argv[3] : 0;

  if (revision <= 0) {
    printf("wrong revision %d\n", revision);
  } else if (parsep = nsmlib_parsefile(datname, revision, incpath,
				       fmtstr, &newrevision)) {
    printf("parsep found: %s\n", fmtstr);
  } else {
    int errcode;
    const char *errstr = nsmlib_parseerr(&errcode);
    printf("%s (code=%d)\n", errstr, errcode);
  }

  indent[0] = 0;
  
  while (parsep) {
    printf("parsep %s%s %c %d %d\n",
	   indent,
	   parsep->name, parsep->type, parsep->size,
	   parsep->offset);
    if (parsep->type == '(') {
      strcat(indent, "  ");
    } else if (parsep->type == ')') {
      indent[strlen(indent) - 2] = 0;
    }
    parsep = parsep->next;
  }

  return 0;
}
#endif /* TESTPARSE */
/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
