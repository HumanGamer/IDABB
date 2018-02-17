#ifndef __BLITZ_H_
#define __BLITZ_H_

#include "types.h"

class linput_t;

#include <cstdint>
#include <vector>

struct BBObject
{
	char* name;
	U32 address;
	BBObject(char* Name, U32 Address) : name(Name), address(Address) { }
};

struct Blitz
{
	U32 codeSize;
	void* code;

	U32 symbolCount;
	BBObject* symbols;
	
	U32 relRelocCount;
	BBObject* relRelocs;

	U32 absRelocCount;
	BBObject* absRelocs;

	std::vector<BBObject> imports;
};

extern unsigned int getSymbolAddr(char* name);
extern unsigned int getImportAddr(char* name);
extern Blitz blitz;

extern void applyRelocs(linput_t *li);
extern void readBlitz(linput_t *li);
extern void addImportSection();
extern void addImports();
extern void buildCoreImports();
extern void setType(const char* importName, const char* decl);
extern void applyTypes();

extern unsigned int getBaseAddress();

#endif // __BLITZ_H_
