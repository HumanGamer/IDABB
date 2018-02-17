#ifndef __UTIL_H_
#define __UTIL_H_

#include "types.h"

#include <pro.h>
#include <typeinf.hpp>

class linput_t;
class idc_value_t;

extern til_t til;

extern char* readNTString(linput_t* li);

extern bool callIDCFunc(const char* filename, const char* func, int argc = NULL, const idc_value_t *argv = NULL, idc_value_t* result = NULL);
extern idc_value_t callIDCFunc(const char* func, int argc = NULL, const idc_value_t *argv = NULL);

extern void add_var(U32 addr, const char *name);
extern void add_string(U32 addr, const char *name);
extern void add_label(U32 addr, const char *name);
extern void add_function(U32 addr, const char *name);
extern void add_internal(U32 addr, const char* name);
extern void add_object(U32 addr, const char *name);
extern void addImport(U32 funcAddr, char* funcName);
extern void addXRef(U32 fromAddr, U32 toAddr);
extern void loadchunk(linput_t *li, ea_t ea, ea_t end, ea_t paragraphs, int32 filepos, const char *name, const char *sclass);
extern void setType(ea_t ea, const char* decl);

extern bool stringEquals(char* str1, char* str2);

extern unsigned int getFuncSize();

#endif // __UTIL_H_