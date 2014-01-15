//
// Copyright (c) 2005, Wei Mingzhi <whistler@openoffice.org>
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA
//

#include "main.h"

typedef struct textmessage_s
{
   char *name;
   char *message;
} textmessage_t;

#define  MAX_MESSAGES     512

static int gMessageTableCount = 0;
static textmessage_t gMessageTable[MAX_MESSAGES];

void FreeTextMessage()
{
   for (int i = 0; i < gMessageTableCount; i++) {
      if (gMessageTable[i].name != NULL) {
         free(gMessageTable[i].name);
      }
      gMessageTable[i].name = NULL;

      if (gMessageTable[i].message != NULL) {
         free(gMessageTable[i].message);
      }
      gMessageTable[i].message = NULL;
   }

   gMessageTableCount = 0;
}

void InitTextMessage()
{
   FILE *fp = fopen(DATA_DIR "titles.txt", "r");
   if (fp == NULL) {
      fprintf(stderr, "WARNING: cannot load data/titles.txt!\n");
      return;
   }

   char buf[256], buf1[256], msgbuf[1024];
   enum { NAME, TEXT } state = NAME;
   int linenumber = 0;

   FreeTextMessage();

   while (fgets(buf, 256, fp) != NULL) {
      linenumber++;
      strncpy(buf1, buf, 256);
      trim(buf);

      if (buf[0] == '\0' || buf[0] == '#' || buf[0] == '/')
         continue; // skip empty or comment lines

      switch (state) {
         case NAME:
            if (strcmp(buf, "}") == 0) {
               fclose(fp);
               TerminateOnError("Unexpected \"}\" found in titles.txt, line %d", linenumber);
            } else if (strcmp(buf, "{") == 0) {
               state = TEXT;
               msgbuf[0] = '\0';
            } else {
               gMessageTable[gMessageTableCount].name = strdup(buf);
            }
            break;

         case TEXT:
            if (strcmp(buf, "{") == 0) {
               fclose(fp);
               TerminateOnError("Unexpected \"{\" found in titles.txt, line %d", linenumber);
            } else if (strcmp(buf, "}") == 0) {
               char *p = msgbuf;
               p += strlen(p) - 1;
               while (*p == '\n' || *p == '\r') {
                  *p-- = '\0'; // remove trailing linefeeds
               }
               state = NAME;
               gMessageTable[gMessageTableCount].message = strdup(msgbuf);
               gMessageTableCount++;
               if (gMessageTableCount >= MAX_MESSAGES) {
                  fprintf(stderr, "WARNING: TOO MANY MESSAGES IN TITIES.TXT, MAX IS %d\n", MAX_MESSAGES);
                  goto end;
               }
            } else {
               strncat(msgbuf, buf1, 1024);
            }
            break;
      }
   }

end:
   fclose(fp);
}

const char *msg(const char *name)
{
   for (int i = 0; i < gMessageTableCount; i++) {
      if (strcmp(gMessageTable[i].name, name) == 0)
         return gMessageTable[i].message;
   }
   return name;
}

