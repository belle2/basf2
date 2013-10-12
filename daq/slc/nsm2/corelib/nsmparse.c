#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "nsm2.h"

static int  nsmparse_errcode;
static char nsmparse_errstr[256];

#define NSMENV_INCDIR "NSM2_INCDIR"

#define  NSMEPARSENOFILE (-10)
#define  NSMEPARSECOM    (-11)
#define  NSMEPARSENOREV  (-12)
#define  NSMEPARSEREV    (-13)
#define  NSMEPARSENOSTR  (-14)
#define  NSMEPARSEITEM   (-15)

nsm_data_att_t nsm_data_att_list[256];

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
  
  *filelenp = 0;

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

  filebuf = malloc(statbuf.st_size + 1);
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

  *filelenp = statbuf.st_size;
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
  for (q = p; isalnum(*q) || *q == '_'; q++);
  token = malloc(q - p + 1);
  if (! token) {
    printf("nsmparse_error: malloc error %d bytes\n", q - p + 1);
    exit(1);
  }
  strncpy(token, p, q - p);
  token[q - p] = 0;
  sprintf(nsmparse_errstr, "Error:%s:%d: ", file, line);
  sprintf(nsmparse_errstr+strlen(nsmparse_errstr), fmt, token);
  free(token);
}
/* -- nsmparse_revision ---------------------------------------------- */
static int
nsmparse_revision(const char *file, char *filebuf, const char *datname)
{
  int len = strlen(datname);
  char *p = filebuf;
  int num;

  while (p = strstr(p, datname)) {
    if ((p == filebuf || ! isalnum(*(p-1))) &&
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
/* -- nsmparse_struct ------------------------------------------------- */
static char *
nsmparse_struct(char *filebuf, const char *datname)
{
  int len = strlen(datname);
  char *p = filebuf;

  while (p = strstr(p, "struct")) {
    if ((p == filebuf || ! isalnum(*(p-1))) &&
	isspace(*(p+6)) && strncmp(p+7, datname, len) == 0 &&
	isspace(*(p+7+len)) && *(p+8+len) == '{') {
      return (isspace(*(p+9+len)) ? p+10+len : p+9+len);
    }
    p += 6;
  }
  return 0;
}
/* -- nsmparse_scan --------------------------------------------------- */
static char *
nsmparse_scan(const char *file, char *filebuf, char *start)
{
  char *ptr = start;
  struct types_t { char *name; int siz; char sym; };
  struct types_t *typep;
  static struct types_t types[] = {
    { "int64",  8, 'd' },
    { "int32",  4, 'i' },
    { "int16",  2, 's' },
    { "char",   1, 'a' },
    { "uint64", 8, 'd' },
    { "uint32", 4, 'i' },
    { "uint16", 2, 's' },
    { "uchar",  1, 'a' },
    { "byte8",  1, 'a' },
    { "double", 8, 'd' },
    { "float",  4, 'i' },
    { 0, 0, 0 }};
  char sym_prev = 0;
  int n_same = 0;
  static char fmtstr[256];
  int fmtlen = 63;

  *fmtstr = 0;

  int offset = 0;
  memset(nsm_data_att_list, 0, sizeof(nsm_data_att_list));
  nsm_data_att_t* nsm_data_att_p = nsm_data_att_list;
  while (1) {
    if (*ptr == '}') break;
    for (typep = types; typep->name; typep++) {
      int len = strlen(typep->name);
      if (strncmp(ptr, typep->name, len) == 0 && isspace(ptr[len])) {
	strcpy(nsm_data_att_p->type, typep->name);
	nsm_data_att_p->offset = offset;
	ptr += len+1;
	break;
      }
    }
    if (! typep->name) {
      nsmparse_error("'%s' is not a valid type", file, filebuf, ptr);
      return 0;
    }
    if (! isalpha(*ptr) && *ptr != '_') {
      nsmparse_error("valid name not found", file, filebuf, ptr);
      return 0;
    }
    char buf[32];
    while (isalnum(*ptr) || *ptr == '_') {
      sprintf(buf, "%c", *ptr);
      strcat(nsm_data_att_p->label, buf);
      ptr++;
    }
    if (isspace(*ptr)) ptr++;
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
      n_same++;
      nsm_data_att_p->length = -1;
      offset += typep->siz;
    } else if (*ptr == '[') { /* multi-dim array not supported yet */
      int num;
      char *q;
      if (isspace(*++ptr)) ++ptr;
      if (! isdigit(*ptr)) {
	nsmparse_error("invalid array size", file, filebuf, ptr);
	return 0;
      }
      num = strtoul(ptr, &q, 0);
      nsm_data_att_p->length = num;
      offset += typep->siz * num;
      if (isspace(*q)) q++;
      if (*q != ']') {
	nsmparse_error("invalid array size", file, filebuf, ptr);
	return 0;
      }
      if (isspace(*++q)) ++q;
      if (*q != ';') {
	nsmparse_error("semicolon not found", file, filebuf, ptr);
	return 0;
      }
      /* printf("%s %d %d\n", typep->name, typep->siz, num); */
      ptr = q+1;
      if (isspace(*ptr)) ptr++;

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
      n_same += num;
    } else {
      nsmparse_error("semicolon not found", file, filebuf, ptr);
      return 0;
    }
    nsm_data_att_p++;
  }

  if (n_same) {
    sprintf(fmtstr+strlen(fmtstr), "%c", sym_prev);
    if (n_same > 1) sprintf(fmtstr+strlen(fmtstr), "%d", n_same);
  }

  if (strlen(fmtstr) > fmtlen) {
    nsmparse_error("struct size exceeded", file, filebuf, ptr);
    return 0;
  }

  return fmtstr;
}
/* -- nsmparse_findfile ------------------------------------------- */
char *
nsmparse_findfile(const char *name, const char *incpath, off_t *filelenp)
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
	path = malloc(q - p + len + 4);
	strncpy(path, p, q-p);
	path[q-p] = '/';
	strcpy(&path[q-p+1], name);
	strcat(path, ".h");
	if (stat(path, &statbuf) == 0) {
	  *filelenp = statbuf.st_size;
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

/* -- nsmlib_parse ---------------------------------------------------- */
char *
nsmlib_parse(const char *datname, int revision, const char *incpath)
{
  off_t filelen;
  char *filepath;
  char *filebuf;
  char *datlist;
  char *strbegin;
  char *fmtstr;
  int ret;
  
  filepath = nsmparse_findfile(datname, incpath, &filelen);
  if (! filepath) {
    nsmparse_errcode = NSMEPARSENOFILE;
    sprintf(nsmparse_errstr, "%s.h not found in the include path", datname);
    return 0;
  }

  filebuf = nsmparse_readfile(filepath, &filelen);
  free(filepath);
  
  if (! filebuf) return 0;

  if (nsmparse_cleanup(filebuf, &filelen)) {
    nsmparse_errcode = NSMEPARSECOM;
    free(filebuf);
    return 0;
  }

  if ((ret = nsmparse_revision(filepath, filebuf, datname)) <= 0) {
    nsmparse_errcode = NSMEPARSENOREV;
    free(filebuf);
    return 0;
  }

  if (ret != revision) {
    nsmparse_errcode = NSMEPARSENOREV;
    sprintf(nsmparse_errstr, "revision mismatch, found %d while expecting %d",
	    ret, revision);
    free(filebuf);
    return 0;
  }
  
  
  if (! (strbegin = nsmparse_struct(filebuf, datname))) {
    nsmparse_errcode = NSMEPARSENOSTR;
    free(filebuf);
    return 0;
  }
  
  if (! (fmtstr = nsmparse_scan(filepath, filebuf, strbegin))) {
    nsmparse_errcode = NSMEPARSEITEM;
    free(filebuf);
    return 0;
  }
  
  free(filebuf);
  return fmtstr;
}
/* -- nsmlib_parse_str -------------------------------------------------- */
char *
nsmlib_parse_str(const char *filebuf_in, const char *filepath, 
		 const char *datname, int revision)
{
  off_t filelen;
  char *filebuf;
  char *strbegin;
  char *fmtstr;
  int ret;
  
  filebuf = malloc(strlen(filebuf_in));
  if (! filebuf) return 0;

  if (nsmparse_cleanup(filebuf, &filelen)) {
    nsmparse_errcode = NSMEPARSECOM;
    free(filebuf);
    return 0;
  }

  if ((ret = nsmparse_revision(filepath, filebuf, datname)) <= 0) {
    nsmparse_errcode = NSMEPARSENOREV;
    free(filebuf);
    return 0;
  }

  if (ret != revision) {
    nsmparse_errcode = NSMEPARSENOREV;
    sprintf(nsmparse_errstr, "revision mismatch, found %d while expecting %d",
	    ret, revision);
    free(filebuf);
    return 0;
  }
  
  
  if (! (strbegin = nsmparse_struct(filebuf, datname))) {
    nsmparse_errcode = NSMEPARSENOSTR;
    free(filebuf);
    return 0;
  }
  
  if (! (fmtstr = nsmparse_scan(filepath, filebuf, strbegin))) {
    nsmparse_errcode = NSMEPARSEITEM;
    free(filebuf);
    return 0;
  }
  
  free(filebuf);
  return fmtstr;
}
/* -- nsmlib_parseerr ------------------------------------------------- */
const char *
nsmlib_parseerr(int *code)
{
  if (code) *code = nsmparse_errcode;
  return nsmparse_errstr;
}
/* -- main ------------------------------------------------------------ */
#if 0
int
main(int argc, char **argv)
{
  char *datname;
  char *incpath;
  const char *fmtstr;
  int revision;
  
  if (argc < 3) {
    printf("usage: %s <data-name> <revision> [<path-list>]\n", argv[0]);
    return 1;
  }

  datname = argv[1];
  revision = atoi(argv[2]);
  incpath = (argc == 4) ? argv[3] : 0;

  if (revision <= 0) {
    printf("wrong revision %d\n", revision);
  } else if (fmtstr = nsmlib_parse(datname, revision, incpath)) {
    printf("fmtstr found: %s\n", fmtstr);
  } else {
    int errcode;
    fmtstr = nsmlib_parseerr(&errcode);
    printf("%s (code=%d)\n", fmtstr, errcode);
  }

#if 0
  filebuf = readfile(argv[1], &filelen);
  if (filebuf) {
    datname = getname(argv[1]);
    /* printf("datname = %s\n", datname); */
    parse_main(argv[1], datname, filebuf, filelen);
  }
#endif
  return 0;
}
#endif
/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
