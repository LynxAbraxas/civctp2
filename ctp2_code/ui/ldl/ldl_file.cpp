










#include "c3.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif defined(WIN32)
#include <direct.h>
#endif

#include "ldl_file.hpp"
#include "ldl_attr.hpp"
#include "ldl_data.hpp"
#include "ldl_memmap.h"

#include "ldlif.h"



ldl::ldl(const char *fname, const char *outputDir, BOOL bUseInternalMemmap )
{
	ReadData( fname );
}

ldl::~ldl( void )
{
	ldlif_deallocate_stuff();
}

BOOL ldl::ReadData( const char *fname )
{
	char *name = strrchr(fname, FILE_SEPC);
	if (NULL == name) {
		name = const_cast<char *>(fname);
	} else {
		name++;
	}
	
	ldlif_parse(name);

	return TRUE;
	
}


ldl_datablock *ldl::FindDataBlock( char *szName, ldl_datablock *dbParent )
{
	return ldlif_find_block(szName);
}

BOOL ldl::WriteData()
{
	return TRUE;
}
