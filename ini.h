//
// Copyright (c) 2005, Wei Mingzhi <whistler_wmz@users.sf.net>
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

#ifndef INI_H
#define INI_H

// uncomment the next line to enable hashing
//#define WITH_HASH 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

#if defined(_MSC_VER)
// Remove warnings from Level 4
#pragma warning (disable : 4244)
#pragma warning (disable : 4514)
#pragma warning (disable : 4706)
#endif

#if defined(_WIN32) && !defined(__BORLANDC__)
#ifndef strcasecmp
#define strcasecmp strcmpi
#endif
#endif

#ifdef WITH_HASH
static const int INI_HASH_KEY_SIZE = 1024; // size of the key hash table
static const int INI_HASH_VALUE_SIZE = 1024; // size of the value hash table of each key
#endif

static const int INI_SIZE_INCREMENT = 10; // increment of memory allocation

typedef struct {
   char           *value_name;
   char           *value;
} ini_value_t;

typedef struct {
   char               *key_name;
   int                 value_count;
   int                 current_size;
   ini_value_t        *values;
#ifdef WITH_HASH
   ini_value_t        *hash[INI_HASH_VALUE_SIZE];
#endif
} ini_key_t;

class CIniFile
{
public:
   CIniFile();
   CIniFile(const char *filename);
   virtual ~CIniFile();

   void Load(const char *filename);
   void Save(const char *filename);

   const char *Get(const char *key, const char *value, const char *def = "");
   void Set(const char *key, const char *value, const char *set);

   inline bool Valid(void) { return (ini != NULL); }

private:
   ini_key_t      *ini;
   int             key_count;
   int             current_size;

   void            FreeAllTheStuff(void);

   // utility functions
   void            trim(char *str);
#ifdef WITH_HASH
   unsigned short  GetHashValue(const char *sz);

   ini_key_t     **m_Hash;
#endif
};

#endif

