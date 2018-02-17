#include "xref.h"

#include "Blitz.h"
#include "Util.h"

#include <loader.hpp>
#include <ua.hpp>
//#include <kernwin.hpp>

void addAllXRefs()
{
	msg("Adding XRefs\n");
	for (int i = 0; i < blitz.absRelocCount; i++)
	{
		BBObject obj = blitz.absRelocs[i];
		int fromAddr = obj.address;
		int toAddr = getSymbolAddr(obj.name);
		if (toAddr == -1)
		{
			toAddr = getImportAddr(obj.name);
		}
		//addXRef(fromAddr, toAddr);

		//set_op_type(fromAddr, offflag(), 1);

		//setFlags(fromAddr, )

		int from = fromAddr - 2;
		

		flags_t flags = get_flags(from);

		if (is_off(flags, 1))
		{
			//set_refinfo
			//msg("Unknown Reloc: %s\n", obj.name);
		}
	}
}