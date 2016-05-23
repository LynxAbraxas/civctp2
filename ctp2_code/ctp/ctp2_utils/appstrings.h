/**
 * $Id$
 */
#ifdef HAVE_PRAGMA_ONCE
#pragma once
#endif

#ifndef __APPSTRINGS_H__
#define __APPSTRINGS_H__

enum APPSTR {
	APPSTR_LOCALE,
	APPSTR_FONTS,
	APPSTR_DIRECTX,
	APPSTR_DIRECTMEDIA,
	APPSTR_INITIALIZE,
	APPSTR_CDROM,
	APPSTR_MEMORY,
	APPSTR_NOWINDOWSDIR,
	APPSTR_REINSTALLDIRECTX,
	APPSTR_NO16BIT,
	APPSTR_NEEDDIRECTX,
	APPSTR_CANTFINDFILE,
	APPSTR_INSERTCDROM,
	APPSTR_NEEDCDROM,
	APPSTR_NEEDDIRECTMEDIA,
	APPSTR_LOWMEMORY1,
	APPSTR_LOWMEMORY2,
	APPSTR_NEEDMOREMEMORY,

	APPSTR_MAX
};

void appstrings_Initialize(void);
void appstrings_Cleanup(void);

MBCHAR *appstrings_GetString(APPSTR stringID);

#endif
