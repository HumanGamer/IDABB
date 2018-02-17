#include "Blitz.h"

#include <loader.hpp>
#include <expr.hpp>
#include <typeinf.hpp>
#include <diskio.hpp>
#include <offset.hpp>

#include "Util.h"

Blitz blitz;

unsigned int getSymbolAddr(char* name)
{
	for (int i = 0; i < blitz.symbolCount; i++)
	{
		BBObject sym = blitz.symbols[i];
		if (strcmp(name, sym.name) == 0)
		{
			return sym.address;
		}
	}
	return -1;
}

unsigned int getImportAddr(char* name)
{
	for (int i = 0; i < blitz.imports.size(); i++)
	{
		BBObject imp = blitz.imports[i];
		if (strcmp(name, imp.name) == 0)
			return imp.address;
	}
	return -1;
}

void readBlitz(linput_t *li)
{
	msg("Reading File\n");

	U32 size;
	lread4bytes(li, &size, false);
	blitz.codeSize = size;

	U32 fileSize = (U32)qlsize(li);

	if (size > fileSize)
		loader_failure("Invalid code size: %d > %d", size, fileSize);

	int32 startAddr = inf.baseaddr;
	int32 endAddr = inf.baseaddr + blitz.codeSize;

	loadchunk(li, startAddr, endAddr, 0, 4, ".code", "CODE");

	char* buf = (char*)malloc(blitz.codeSize);

	for (int i = 0; i < blitz.codeSize; i++)
	{
		qlgetc(li);
	}

	U32 startSymbols = qltell(li);

	U32 symbolCount;
	lread4bytes(li, &symbolCount, false);
	blitz.symbolCount = symbolCount;

	blitz.symbols = (BBObject*)malloc(blitz.symbolCount * sizeof(BBObject));

	for (int i = 0; i < blitz.symbolCount; i++)
	{
		U32 lblAddr = qltell(li);
		char* name = readNTString(li);
		U32 addr;
		lread4bytes(li, &addr, false);

		BBObject obj(name, addr);
		blitz.symbols[i] = obj;
	}

	U32 endSymbols = qltell(li);

	int pos = qltell(li);

	U32 relRelocCount;
	lread4bytes(li, &relRelocCount, false);
	blitz.relRelocCount = relRelocCount;

	U32 startRels = qltell(li);


	blitz.relRelocs = (BBObject*)malloc(blitz.relRelocCount * sizeof(BBObject));

	for (int i = 0; i < blitz.relRelocCount; i++)
	{
		char* name = readNTString(li);
		U32 addr;
		lread4bytes(li, &addr, false);

		BBObject obj(name, addr);
		blitz.relRelocs[i] = obj;
	}
	U32 endRels = qltell(li);

	U32 absRelocCount;
	lread4bytes(li, &absRelocCount, false);
	blitz.absRelocCount = absRelocCount;

	U32 startAbs = qltell(li);

	blitz.absRelocs = (BBObject*)malloc(blitz.absRelocCount * sizeof(BBObject));

	for (int i = 0; i < blitz.absRelocCount; i++)
	{
		char* name = readNTString(li);
		U32 addr;
		lread4bytes(li, &addr, false);

		BBObject obj(name, addr);
		blitz.absRelocs[i] = obj;
	}
	//U32 endAbs = qltell(li);
}

void applyRelocs(linput_t *li)
{
	msg("Applying Relocs\n");
	const uint offset = inf.baseaddr;
	for (int i = 0; i < blitz.relRelocCount; i++)
	{
		BBObject obj = blitz.relRelocs[i];
		char* name = obj.name;
		uint addr = obj.address;

		uint old = get_32bit(addr + offset);
		add_dword(addr + offset, -(int)old);
		uint newAddr = getSymbolAddr(name);
		if (newAddr == -1)
		{
			newAddr = getImportAddr(name) - 4;
		}
		else {
			newAddr += inf.baseaddr - 4;
		}
		uint newOffset = newAddr - addr - (offset * 2);
		add_dword(addr + offset, newOffset);
	}

	for (int i = 0; i < blitz.absRelocCount; i++)
	{
		BBObject obj = blitz.absRelocs[i];
		char* name = obj.name;
		uint addr = obj.address;

		uint old = get_32bit(addr + offset);
		add_dword(addr + offset, -(int)old);
		uint newAddr = getSymbolAddr(name);
		if (newAddr == -1)
		{
			newAddr = getImportAddr(name);
		}
		else {
			newAddr += inf.baseaddr;
		}
		add_dword(addr + offset, newAddr);
	}
}

void addImportSection()
{
	msg("Adding Import Section\n");
	uint baseAddr = getBaseAddress();
	uint funcSize = getFuncSize();
	uint sectionSize = blitz.imports.size() * funcSize;

	idc_value_t args[6];
	args[0] = baseAddr;
	args[1] = baseAddr + sectionSize;
	args[2] = "DATA";
	args[3] = 4;
	args[4] = ".imports";
	args[5] = 1;

	//msg("Section Addr: %d\n", baseAddr);
	//msg("Section Size: %d\n", sectionSize);

	callIDCFunc("SetupSection", 6, args);
}

void addImports()
{
	msg("Adding Imports\n");
	for (int i = 0; i < blitz.imports.size(); i++)
	{
		BBObject obj = blitz.imports[i];
		char* name = obj.name;
		uint addr = obj.address;

		addImport(addr, name);
	}
}

void buildCoreImports()
{
	msg("Building Core Imports\n");
	uint baseAddr = getBaseAddress();
	uint funcSize = getFuncSize();

	int importCount = 0;
	for (int i = 0; i < blitz.relRelocCount; i++)
	{
		BBObject obj = blitz.relRelocs[i];
		char* name = obj.name;

		if (getSymbolAddr(name) != -1)
			continue;
		if (getImportAddr(name) != -1)
			continue;

		uint addr = baseAddr + (funcSize * importCount);

		blitz.imports.emplace_back(name, addr);

		importCount++;
	}
}

extern void setType(const char* importName, const char* decl)
{
	U32 addr = getImportAddr((char*)importName);
	setType(addr, decl);
}

ea_t getAddr(char* name)
{
	int addr = getSymbolAddr(name);
	if (addr == -1)
		return getImportAddr(name);
	return addr;
}

void applyTypes()
{
	/*U32 addr = getImportAddr("__bbStrConst");

	til_t til;
	apply_cdecl2(&til, addr, "BBStr* (__stdcall *)(const char* s);");*/

	for (int i = 0; i < blitz.absRelocCount; i++)
	{
		BBObject obj = blitz.absRelocs[i];
		//op_t op;
		//op.addr = obj.address;
		//ua_add_off_drefs(op, dr_O);
		//set_refinfo
		//op_offset(obj.address, OPND_ALL, REF_OFF32);
		//flags_t flag = 0;
		//opinfo_t ti;
		//ti.ri.target = getAddr(obj.name);
		//set_opinfo(obj.address, OPND_ALL, flag, &ti);
		//set_offset(obj.address, 1, 0);

		/*refinfo_t refinfo;
		refinfo.init(REF_OFF32);
		refinfo.target = getAddr(obj.name);
		op_offset_ex(obj.address, OPND_ALL, &refinfo);*/

		/*xrefpos_t xrefpos;
		xrefpos.ea = obj.address;
		xrefpos.type = 0;
		set_xrefpos(getAddr(obj.name), &xrefpos);*/

		//flags_t F;

		//op_adds_xrefs(F, OPND_ALL);
		//set_op_type(obj.address, offflag(), OPND_ALL);
		//set_forced_operand(0x12, OPND_ALL, obj.name);
		//set_manual_insn(obj.address, obj.name);
		/*if (create_xrefs_from(obj.address) == 0)
		{
			msg("Ooops: %d, %s\n", obj.address, obj.name);
		}*/

		//add_cref(obj.address, getAddr(obj.name), fl_U);
		//set_op_type(0x13, offflag(), 0);
		//set_op_type(0x13, offflag(), 1);
		//set_op_type(0x13, offflag(), -1);

		op_offset(obj.address, 0, REF_OFF32);

		// Temp Working
		add_dref(obj.address, getAddr(obj.name), dr_U);
	}
	setType("__bbStrConst", "void* (__stdcall *)(const char* s);");
}

unsigned int getBaseAddress()
{
	return 0x10000000;
}