/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/manager/RFConf.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

using namespace std;
using namespace Belle2;

RFConf::RFConf(const char* filename)
{
  m_fd = fopen(filename, "r");
  if (m_fd == NULL) {
    fprintf(stderr, "RFConf : config file not found %s\n", filename);
    exit(-1);
  }
}

RFConf::~RFConf()
{
  fclose(m_fd);
}

// Get Configuration

char* RFConf::getconf(const char* key1, const char* key2, const char* key3)
{
  char buf[1024], keybuf[256];
  char* p, *q, *keyp, *delp, *valp;
  int line;

  struct RFConf_t top, *cur;
  top.key = NULL;

  // Encode key2 and key3 in the first key if exist
  //  if (! key2) { key2 = key3; key3 = 0; }
  //  if (! key1) { key1 = key2; key2 = key3; key3 = 0; }
  if (key3)
    sprintf(keybuf, "%s.%s.%s", key1, key2, key3);
  else if (key2)
    sprintf(keybuf, "%s.%s", key1, key2);
  else if (key1)
    strcpy(keybuf, key1);
  else
    return NULL;

  // Search for the record in the configuration file
  rewind(m_fd);
  if (! top.key) { /* the first invokation */
    line = 0;
    cur = &top;
    while (fgets(buf, sizeof(buf), m_fd)) {
      line++;
      /* remove '\n' and skip too long line */
      p = strchr(buf, '\n');
      if (! p) {
        fprintf(stderr, "RFConf : line %d too long\n", line);
        while (fgets(buf, sizeof(buf), m_fd) && !strchr(buf, '\n'))
          ;
        continue;
      }
      *p = 0;
      //      printf ( "buf = %s\n", buf );

      /* sorry for this very tricky code... */
      keyp = valp = delp = 0;
      for (p = buf; *p && *p != '#'; p++) {
        if (! keyp) {
          if (! isspace(*p)) keyp = p;
        } else if (! delp) {
          if (isspace(*p)) {
            if (!isspace(*(p + 1)) && *(p + 1) != ':') {
              fprintf(stderr, "RFConf : invalid key at line %d\n", line);
              break;
            }
            *p = 0;
          } else if (*p == ':') {
            *(delp = p) = 0;
          }
        } else if (! valp) {
          if (! isspace(*p)) {
            valp = q = p;
            q++;
          }
        } else if (! isspace(*p)) {
          *q++ = *p;
        } else if (! isspace(*(p + 1))) {
          *q++ = ' ';
        }
      }
      if (valp) {
        for (*q-- = 0; isspace(*q); *q-- = 0)
          ;
      }
      if (delp) {
        cur->next = (RFConf_t*)malloc(sizeof(*cur));
        cur = cur->next;
        cur->next = 0;
        cur->key = (char*)malloc(strlen(keyp) + 1);
        strcpy(cur->key, keyp);
        if (valp) {
          cur->val = (char*)malloc(strlen(valp) + 1);
          strcpy(cur->val, valp);
        } else {
          cur->val = 0;
        }
      }
    }
  } else if (! top.val) { /* the first invokation must have failed */
    return NULL;
  }

  int nitem = 0;
  for (cur = &top; cur; cur = cur->next) {
    if (cur->key == NULL) continue;
    if (strcmp(cur->key, keybuf) == 0) return cur->val;
    nitem++;
  }
  printf("RFConf: Key %s not found\n", keybuf);
  printf("nitem = %d, keybuf = %s\n", nitem, keybuf);
  return NULL;
}

int RFConf::getconfi(const char* key1, const char* key2, const char* key3)
{
  return atoi(getconf(key1, key2, key3));
}


