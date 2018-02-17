#include "Util.h"

#include <loader.hpp>
#include <expr.hpp>
#include <typeinf.hpp>
#include <xref.hpp>
#include <diskio.hpp>
#include <offset.hpp>
#include <auto.hpp>

til_t til;

char* readNTString(linput_t* li)
{
	char res[255];

	int index = 0;
	while (true)
	{
		if (index >= 255)
		{
			loader_failure("Failed to load string! %s", res);
			break;
		}
		res[index] = '\0';
		char c = qlgetc(li);
		if (c == EOF)
		{
			warning("End of file while reading string!");
			break;
		}
		char *s = (char*)malloc(2);
		s[0] = c;
		s[1] = '\0';
		strcat_s(res, s);
		if (c == '\0')
			break;
		index++;
	}

	int len = strlen(res) + 1;
	char *ret = (char*)malloc(len);

	memcpy_s(ret, len, res, len);

	return ret;
}

bool callIDCFunc(const char* filename, const char* func, int argc, const idc_value_t *argv, idc_value_t* result)
{
	//const char* idcPath = "idc";// get_idc_name();

	//char buf[1024];
	//sprintf_s(buf, 1024, "%s/%s", idcPath, filename);

	char buf[1024];
	get_idc_filename(buf, 1024, filename);

	qstring errbuf;

	if (!exec_idc_script(result, buf, func, argv, argc, &errbuf))
	{
		warning("Unable to call script function \"%s\" from \"%s\"\n\n%s", func, buf, errbuf.c_str());
		return false;
	}
	return true;
}

idc_value_t callIDCFunc(const char* func, int argc, const idc_value_t *argv)
{
	idc_value_t result;
	callIDCFunc("blitzbasic_imports.idc", func, argc, argv, &result);
	return result;
}

void add_var(U32 addr, const char *name)
{
	//if (addr != 0)
	{
		set_name(addr, name, SN_NON_AUTO);
		op_offset(addr, OPND_MASK, REF_OFF32);
	}
}

void add_string(U32 addr, const char *name)
{
	//if (addr != 0)
	{
		set_name(addr, name, SN_NON_AUTO);
		//make_ascii_string(addr, 0, ASCSTR_C);
		// TODO: FIX
	}
}

void add_label(U32 addr, const char *name)
{
	//if (addr != 0)
	{
		set_name(addr, name, SN_NON_AUTO);
	}
}

void add_function(U32 addr, const char *name)
{
	//if (addr != 0)
	{
		set_name(addr, name, SN_NON_AUTO);

		auto_make_code(addr);
		auto_make_proc(addr);
	}
}

void add_internal(U32 addr, const char* name)
{
	if ((strlen(name) > 2 && strcmp(&name[2], "_begin") != 0) || strcmp(name, "__MAIN") != 0)
	{
		add_label(addr, name);
	}
	else
	{
		for (int i = 0; i < 255; i++)
		{
			char c = get_byte(addr);
			if (!isalnum(c) && !ispunct(c))
			{
				add_label(addr, name);
				return;
			}
		}

		add_string(addr, name);
	}
}

void add_object(U32 addr, const char *name)
{
	const char *newName = &name[0];//2
	if (name[0] == '_')
	{
		switch (name[1])
		{
		case 'l': // Label
			add_label(addr, newName);
			break;
		case 'a': // Array
			add_var(addr, newName);
			break;
		case 'v': // Var
			add_var(addr, newName);
			break;
		case '_': // Internal
			add_label(addr, name);
			break;
		case 't': // Type
			add_var(addr, newName);
			break;
		case 'f': // Function
			add_function(addr, newName);
			add_label(addr, newName);
			break;
		default:
			if (isdigit(name[1]))
			{
				add_internal(addr, name);
			}
			else {
				warning("Unknown Type: %s", name);
			}
			break;
		}
	}
}

void addImport(U32 funcAddr, char* funcName)
{
	set_name(funcAddr, funcName, SN_NOWARN);
}

void loadchunk(linput_t *li, ea_t ea, ea_t end, ea_t paragraphs, int32 filepos, const char *name, const char *sclass)
{
	int32 position = qltell(li);
	int startAddr = ea;// +inf.baseaddr;
	int endAddr = end;// +inf.baseaddr;
	file2base(li, filepos, startAddr, endAddr, FILEREG_PATCHABLE);
	add_segm(paragraphs, startAddr, endAddr, name, sclass);
	qlseek(li, position);
}

void addXRef(U32 fromAddr, U32 toAddr)
{
	dref_t type;
	type = dr_O;
	
	if (!add_dref(fromAddr, toAddr, type))
	{
		msg("Failed to add ref: %d->%d\n", fromAddr, toAddr);
	}
}

extern void setType(ea_t ea, const char* decl)
{
	//apply_cdecl2(&til, ea, decl);
	apply_cdecl(&til, ea, decl);
}

bool stringEquals(char* str1, char* str2)
{
	return strcmp(str1, str2) == 0;
}

uint getFuncSize()
{
	return 0x1;
}