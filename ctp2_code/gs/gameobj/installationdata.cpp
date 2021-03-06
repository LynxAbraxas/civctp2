//----------------------------------------------------------------------------
//
// Project      : Call To Power 2
// File type    : C++ source
// Description  : Installation data
// Id           : $Id$
//
//----------------------------------------------------------------------------
//
// Disclaimer
//
// THIS FILE IS NOT GENERATED OR SUPPORTED BY ACTIVISION.
//
// This material has been developed at apolyton.net by the Apolyton CtP2
// Source Code Project. Contact the authors at ctp2source@apolyton.net.
//
//----------------------------------------------------------------------------
//
// Compiler flags
//
// - None
//
//----------------------------------------------------------------------------
//
// Modifications from the original Activision code:
//
// - Made government modified for units work here. (July 29th 2006 Martin G�hmann)
//
//----------------------------------------------------------------------------

#include "c3.h"
#include "installation.h"
#include "ID.h"
#include "civarchive.h"
#include "network.h"

#include "UnitData.h"
#include "installationtree.h"
#include "QuadTree.h"
#include "Unit.h"
#include "World.h"
#include "player.h"
#include "Cell.h"
#include "TurnCnt.h"
#include "SelItem.h"
#include "tiledmap.h"
#include "network.h"
#include "net_info.h"
#include "UnitRecord.h"
#include "unitutil.h"
#include "terrainutil.h"

extern QuadTree<Unit> *g_theUnitTree;
extern World *g_theWorld;
extern Player **g_player;
extern TurnCount *g_turn;
extern SelectedItem *g_selected_item;
extern TiledMap *g_tiledMap;

InstallationData::InstallationData(ID id,
								   sint32 owner,
								   MapPoint &pnt,
								   sint32 type)
	: GameObj(id.m_id)
{
	m_owner = owner;
	m_point = pnt;
	m_type = type;
	m_visibility = 1 << owner;
	m_airfieldLastUsed = -1;












	ENQUEUE();
}

InstallationData::InstallationData(CivArchive &archive) : GameObj(0)
{
	Serialize(archive);
}

void InstallationData::Serialize(CivArchive &archive)
{
	if(archive.IsStoring()) {
		GameObj::Serialize(archive);
		archive.StoreChunk((uint8 *)&m_owner, ((uint8 *)&m_visibility)+sizeof(m_visibility));

		archive<<(uint32)(m_lesser != NULL);
		if (m_lesser)
			((InstallationData *)(m_lesser))->Serialize(archive) ;

		archive<<(uint32)(m_greater != NULL);
		if (m_greater)
			((InstallationData *)(m_greater))->Serialize(archive) ;

	} else {
		GameObj::Serialize(archive);
		archive.LoadChunk((uint8 *)&m_owner, ((uint8 *)&m_visibility)+sizeof(m_visibility));

		uint32	hasOld;

		archive>>hasOld;

		if (hasOld)
			m_lesser = new InstallationData(archive);

		archive>>hasOld;
		if (hasOld)
			m_greater = new InstallationData(archive);
	}
}


void InstallationData::DoVision()
{
	MapPoint topleft = m_point;

	double maxVisionRange = terrainutil_GetMaxVisionRange();
	double myVisionRange = terrainutil_GetVisionRange(m_type, m_point);
	sint32 maxrsq = sint32((maxVisionRange+0.5) * (maxVisionRange+0.5));
	sint32 myrsq = sint32((myVisionRange+0.5)*(myVisionRange+0.5));

	if(m_owner >= 0) {
		m_visibility = 1 << m_owner;
	} else {
		m_visibility = 0;
	}

	topleft.x -= static_cast<sint16>(maxVisionRange);
	DynamicArray<Installation> instArray;

	g_theInstallationTree->SearchRect(instArray, topleft,
									  static_cast<sint16>(maxVisionRange) * 2 + 1,
									  static_cast<sint16>(maxVisionRange) * 2 + 1,
									  m_owner >= 0 ? ~(1 << m_owner) : 0xffffffff);
	sint32 in = instArray.Num();
	sint32 i;
	for(i = 0; i < in; i++) {
		InstallationData *oinst = instArray[i].AccessData();
		double hisr = terrainutil_GetVisionRange(oinst->m_type, oinst->GetLocation()) + 0.5;
		sint32 hisrsq = sint32(hisr * hisr);
		sint32 ls = GetDistance(this, oinst, sint32(maxVisionRange));
		if(ls > maxrsq)
			continue;

		if(ls <= hisrsq && !(m_visibility & (1 << oinst->m_owner))) {
			if(g_theTerrainImprovementDB->Get(oinst->m_type)->GetCanSee() &
			   g_theTerrainImprovementDB->Get(m_type)->GetCanSee()) {
				m_visibility |= (1 << oinst->m_owner);
			}
		}

		if(ls <= myrsq && !(oinst->m_visibility & (1 << m_owner))) {

			if (g_theTerrainImprovementDB->Get(oinst->m_type)->GetCanSee() &
				g_theTerrainImprovementDB->Get(m_type)->GetCanSee()) {
				oinst->m_visibility |= 1 << m_owner;
			}
		}
	}

	if(unitutil_GetMaxVisionRange() > maxVisionRange) {
		maxVisionRange = unitutil_GetMaxVisionRange();
	}
	maxrsq = sint32((maxVisionRange+0.5) * (maxVisionRange+0.5));
	topleft = m_point;
	topleft.x -= static_cast<sint16>(maxVisionRange);
	DynamicArray<Unit> unitArray;

	g_theUnitTree->SearchRect(unitArray, topleft,
							  static_cast<sint16>(maxVisionRange) * 2 + 1,
							  static_cast<sint16>(maxVisionRange) * 2 + 1,
							  ~(1 << m_owner));
	sint32 un = unitArray.Num();
	for(i = 0; i < un; i++) {
		UnitData *ud = unitArray[i].AccessData();
		double hisr = ud->GetDBRec()->GetVisionRange() + 0.5;
		sint32 hisrsq = sint32(hisr*hisr);
		sint32 ls = UnitData::GetDistance(Installation(m_id),
										  ud, sint32(maxVisionRange));
		if(ls >= maxrsq)
			continue;

		if(ls <= hisrsq && !(m_visibility & (1 << ud->GetOwner()))) {
			m_visibility |= (1 << ud->GetOwner());
		}

		if(ls <= myrsq && !(ud->GetRealVisibility() & (1 << m_owner))) {
			if(ud->GetDBRec()->GetVisionClass() &
			   g_theTerrainImprovementDB->Get(m_type)->GetCanSee()) {
				if(m_owner >= 0) {
					ud->SetVisible((PLAYER_INDEX)m_owner);
				}
			}
		}
	}
}

void InstallationData::CheckVision(sint32 owner)
{
	MapPoint topleft = m_point;

	double maxVisionRange = terrainutil_GetMaxVisionRange();
	sint32 maxrsq = sint32((maxVisionRange+0.5) * (maxVisionRange+0.5));

	topleft.x -= static_cast<sint16>(maxVisionRange);
	DynamicArray<Installation> instArray;

	g_theInstallationTree->SearchRect(instArray, topleft,
									  static_cast<sint16>(maxVisionRange) * 2 + 1,
									  static_cast<sint16>(maxVisionRange) * 2 + 1,
									  1 << owner);
	sint32 in = instArray.Num();
	BOOL canBeSeen = FALSE;
	sint32 i;
	for(i = 0; i < in; i++) {
		InstallationData *oinst = instArray[i].AccessData();
		sint32 ls = GetDistance(this, oinst, sint32(maxVisionRange));
		if(ls > maxrsq)
			continue;

		double hisr = terrainutil_GetVisionRange(oinst->m_type, oinst->GetLocation()) + 0.5;
		sint32 hisrsq = sint32(hisr * hisr);
		if(ls <= hisrsq) {
			if(g_theTerrainImprovementDB->Get(m_type)->GetCanSee() &
			   g_theTerrainImprovementDB->Get(oinst->m_type)->GetCanSee()) {
				canBeSeen = TRUE;
				break;
			}
		}
	}

	if(!canBeSeen) {

		maxVisionRange = unitutil_GetMaxVisionRange();
		maxrsq = sint32((maxVisionRange+0.5) * (maxVisionRange+0.5));
		topleft = m_point;
		topleft.x -= static_cast<sint16>(maxVisionRange);
		DynamicArray<Unit> unitArray;
		g_theUnitTree->SearchRect(unitArray, topleft,
								  static_cast<sint16>(maxVisionRange) * 2 + 1,
								  static_cast<sint16>(maxVisionRange) * 2 + 1,
								  1 << owner);
		sint32 un = unitArray.Num();
		for(i = 0; i < un; i++) {
			UnitData *ud = unitArray[i].AccessData();
			sint32 ls = UnitData::GetDistance(Installation(m_id),
											  ud, sint32(maxVisionRange));
			if(ls > maxrsq)
				continue;

			double hisr = ud->GetDBRec()->GetVisionRange()+0.5;
			sint32 hisrsq = sint32(hisr*hisr);
			if(ls <= hisrsq) {
				if(g_theTerrainImprovementDB->Get(m_type)->GetCanSee() &
				   ud->GetDBRec()->GetCanSee()) {
					canBeSeen = TRUE;
					break;
				}
			}
		}
	}

	if(canBeSeen) {
		m_visibility |= (1 << owner);
	}
}

sint32 InstallationData::GetDistance(InstallationData* inst1,
									 InstallationData* inst2,
									 sint32 wrapRange)
{
	sint32 dx, dy;
	dx = inst1->m_point.x - inst2->m_point.x;
	dy = inst1->m_point.y - inst2->m_point.y;
	dx = abs(dx);
	dy = abs(dy);
	if(dx > wrapRange) {
		if(inst1->m_point.x > inst2->m_point.x) {
			dx = (inst1->m_point.x - g_theWorld->GetXWidth()) - inst2->m_point.x;
		} else {
			dx = inst1->m_point.x - (inst2->m_point.x - g_theWorld->GetXWidth());
		}
		dx = abs(dx);
	}
	dy -= dx;

	return (dx * dx) + (dy * dy);
}

void InstallationData::RebuildQuadTree()
{
	g_theInstallationTree->Insert(Installation(m_id));
	if(m_lesser)
		((InstallationData*)m_lesser)->RebuildQuadTree();

	if(m_greater)
		((InstallationData*)m_greater)->RebuildQuadTree();
}

void InstallationData::UseAirfield()
{
	m_airfieldLastUsed = g_turn->GetRound();
}

sint32 InstallationData::AirfieldLastUsed() const
{
	return m_airfieldLastUsed;
}

void InstallationData::ChangeOwner(sint32 toOwner)
{
	if(g_network.IsHost()) {
		g_network.Enqueue(new NetInfo(NET_INFO_CODE_CHANGE_INSTALLATION_OWNER,
									  m_id, m_owner, toOwner));
	}

	if(m_owner >= 0 && g_player[m_owner] != NULL)
	{
		g_player[m_owner]->RemoveInstallationReferences(Installation(m_id));

		double visionRange = terrainutil_GetVisionRange(m_type, m_point);
		g_player[m_owner]->m_vision->RemoveVisible(m_point, visionRange);

		if(visionRange > 0)
		{
			g_player[m_owner]->RemoveUnitVision(m_point, visionRange);
		}
	}

	if(toOwner >= 0)
	{
		g_player[toOwner]->AddInstallation(Installation(m_id));
	}

	m_owner = toOwner;

	if(toOwner >= 0)
	{
		DoVision();
	}
}

void InstallationData::SetVisible(sint32 player)
{
	m_visibility |= (1 << player);
	g_tiledMap->RedrawTile(&m_point);
}
