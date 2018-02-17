#include <loader.hpp>
#include <frame.hpp>
#include <expr.hpp>
#include <typeinf.hpp>
#include <string>

#include "Util.h"
#include "Blitz.h"
#include "xref.h"

#define MAX_FILE_FORMAT_NAME QMAXPATH

compiler_info_t compiler;

//int idaapi accept_file(linput_t *li, char fileformatname[MAX_FILE_FORMAT_NAME], int n)
int idaapi accept_file(qstring *fileformatname, qstring *processor, linput_t *li, const char *filename)
{
	//if (n)
	//	return 0;

	char rf[QMAXFILE];
	get_root_filename(rf, sizeof(rf));
	const char *ext = get_file_ext(rf);

	if (ext == NULL || stricmp(ext, "bbc") != 0)
		return 0;
	
	//qstrncpy(fileformatname, "BlitzBasic Resource", MAX_FILE_FORMAT_NAME);
	*fileformatname = "BlitzBasic Resource";

	return 1;
}

void setupProperties()
{
	msg("Setting Properties\n");

	if (ph.id != PLFM_386)
		set_processor_type("80386r", SETPROC_LOADER);

	compiler.id = COMP_MS;
	compiler.defalign = 0;
	compiler.cm = CM_CC_STDCALL | CM_M_NN | CM_N32_F48;
	compiler.size_i = 4;
	compiler.size_b = 4;
	compiler.size_e = 4;
	compiler.size_s = 2;
	compiler.size_l = 4;
	compiler.size_ll = 8;

	set_compiler(compiler, SETCOMP_OVERRIDE);

	inf.baseaddr = 0;
	inf.start_cs = 0;
	inf.start_ip = inf.baseaddr;
}

void idaapi load_file(linput_t *li, ushort neflag, const char* fileformatname)
{
	setupProperties();
	readBlitz(li);
	buildCoreImports();
	addImportSection();
	addImports();
	applyRelocs(li);
	//addAllXRefs();
	applyTypes();

	// Process Symbols

	for (int i = 0; i < blitz.symbolCount; i++)
	{
		BBObject sym = blitz.symbols[i];
		add_object(sym.address + inf.baseaddr, sym.name);
	}
}

extern "C" _declspec(dllexport) loader_t LDSC =
{
	IDP_INTERFACE_VERSION,
	0,
	accept_file,
	load_file,
	NULL,
	NULL,
	NULL,
};
