/* ---------------------------------------------------------------------- *\
   nsmhash.c

   hash table calculation library for NSM2

   20130117 ---- initial version
   20140614 1933 check nsmd_sysp pointer before using
\* ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> /* for ntoha etc */
#include <netinet/in.h> /* for ntoha etc */
#include <arpa/inet.h>  /* for ntoha etc */

#include "nsm2.h"

#define HASHMAX 2039

static char *hashtable[HASHMAX];
static int deltatable[HASHMAX];

struct NSMsys;
#define SYSPOS(ptr) ((char *)(ptr) - (char *)nsmd_sysp)
#define SYSPTR(pos) (((char *)nsmd_sysp)+pos)

/* -- nsmlib_hashcode ------------------------------------------------ */
int
nsmlib_hashcode(const char *key, int hashmax)
{
  int num = 0;

  while (*key) {
    int shft = (*key % 11);
    num = (num << shft) + (num >> (11 - shft)) + *key;
    num %= hashmax;
    key++;
  }
  
  return num;
}

/* -- nsmlib_hash ---------------------------------------------------- */
/*
  hashtable is an array of SYSPOS to a struct with size = hashmax,
  and the top of the struct is a char string that can be compared with key.

  create == 1  to create a hash code
  create == 0  to retrieve the hash code
  create == -1 to remove a hash code

  When create == -1, the return value is the number of hash keys that
  have to be shifted.  This information is used to update the hash table.
 */
int
nsmlib_hash(struct NSMsys *nsmd_sysp, int32 *hashtable, int hashmax,
	    const char *key, int create)
{
  int code = nsmlib_hashcode(key, hashmax);
  int num = code;
  int delta = 0;

  if (! nsmd_sysp) return -1;
  
  while (hashtable[num] && strcmp(SYSPTR(ntohl(hashtable[num])), key) != 0) {
    num = (num + 1) % hashmax;
    delta++;
    if (delta == hashmax - 1) {
      /* at least one less to have a gap, so it should not happen */
      return -1;
    }
  }
  if (hashtable[num]) {
    /*
    if (create > 0) {
      printf("old hash found at %d for %s delta=%d\n", num, s, delta);
    }
    */
    if (create < 0) {
      int nshift = 1;
      
      hashtable[num] = 0;

      int next = (num + 1) % hashmax;
      while (1) {
	while (hashtable[next] &&
	       nsmlib_hashcode(SYSPTR(ntohl(hashtable[next])),
			       hashmax) != code) {
	  next++;
	  nshift++;
	}
	if (! hashtable[next]) break;
	
	hashtable[num] = hashtable[next];
	hashtable[next] = 0;
	num = next;
	next = (num + 1) % hashmax;
	nshift++;
      }
      
      return nshift;
    }
  } else if (create < 0) {
    /* printf("hash for %s to remove not found\n", s); */
    return -1;
  } else if (create > 0) {
    hashtable[num] = htonl(SYSPOS(key));
  } else {
    return -1; /* not found */
  }
  return num;
}
