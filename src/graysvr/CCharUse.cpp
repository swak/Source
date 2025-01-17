//  CChar is either an NPC or a Player.
#include "graysvr.h"	// predef header.

bool CChar::Use_MultiLockDown( CItem * pItemTarg )
{
	ADDTOCALLSTACK("CChar::Use_MultiLockDown");
	ASSERT(pItemTarg);
	ASSERT(m_pArea);

	if ( pItemTarg->IsType(IT_KEY) || !pItemTarg->IsMovableType() || !pItemTarg->IsTopLevel() )
		return false;

	if ( !pItemTarg->m_uidLink.IsValidUID() )
	{
		// If we are in a house then lock down the item.
		pItemTarg->m_uidLink.SetPrivateUID(m_pArea->GetResourceID());
		SysMessageDefault(DEFMSG_MULTI_LOCKDOWN);
		return true;
	}
	if ( pItemTarg->m_uidLink == m_pArea->GetResourceID() )
	{
		pItemTarg->m_uidLink.InitUID();
		SysMessageDefault(DEFMSG_MULTI_LOCKUP);
		return true;
	}

	return false;
}

void CChar::Use_CarveCorpse( CItemCorpse * pCorpse )
{
	ADDTOCALLSTACK("CChar::Use_CarveCorpse");
	CREID_TYPE CorpseID = pCorpse->m_itCorpse.m_BaseID;
	CCharBase *pCorpseDef = CCharBase::FindCharBase(CorpseID);
	if ( !pCorpseDef || pCorpse->m_itCorpse.m_carved )
	{
		SysMessageDefault(DEFMSG_CARVE_CORPSE_NOTHING);
		return;
	}

	CChar *pChar = pCorpse->m_uidLink.CharFind();
	CPointMap pnt = pCorpse->GetTopLevelObj()->GetTopPoint();

	UpdateAnimate(ANIM_BOW);
	if ( pCorpse->m_TagDefs.GetKeyNum("BLOOD", true) )
	{
		CItem *pBlood = CItem::CreateBase(ITEMID_BLOOD4);
		ASSERT(pBlood);
		pBlood->SetHue(pCorpseDef->m_wBloodHue);
		pBlood->MoveToDecay(pnt, 5 * TICK_PER_SEC);
	}

	size_t iItems = 0;
	for ( size_t i = 0; i < pCorpseDef->m_BaseResources.GetCount(); i++ )
	{
		long long iQty = pCorpseDef->m_BaseResources[i].GetResQty();
		RESOURCE_ID rid = pCorpseDef->m_BaseResources[i].GetResourceID();
		if ( rid.GetResType() != RES_ITEMDEF )
			continue;

		ITEMID_TYPE id = static_cast<ITEMID_TYPE>(rid.GetResIndex());
		if ( id == ITEMID_NOTHING )
			break;

		iItems++;
		CItem *pPart = CItem::CreateTemplate(id, NULL, this);
		ASSERT(pPart);
		switch ( pPart->GetType() )
		{
			case IT_FOOD:
			case IT_FOOD_RAW:
			case IT_MEAT_RAW:
				SysMessageDefault(DEFMSG_CARVE_CORPSE_MEAT);
				//pPart->m_itFood.m_MeatType = CorpseID;
				break;
			case IT_HIDE:
				SysMessageDefault(DEFMSG_CARVE_CORPSE_HIDES);
				//pPart->m_itSkin.m_creid = CorpseID;
				if ( (g_Cfg.m_iFeatureML & FEATURE_ML_RACIAL_BONUS) && IsHuman() )	// humans always find 10% bonus when gathering hides, ores and logs (Workhorse racial trait)
					iQty = iQty * 110 / 100;
				break;
			case IT_FEATHER:
				SysMessageDefault(DEFMSG_CARVE_CORPSE_FEATHERS);
				//pPart->m_itSkin.m_creid = CorpseID;
				break;
			case IT_WOOL:
				SysMessageDefault(DEFMSG_CARVE_CORPSE_WOOL);
				//pPart->m_itSkin.m_creid = CorpseID;
				break;
			/*case IT_DRAGON_SCALE:			// TO-DO (typedef IT_DRAGON_SCALE doesn't exist yet)
				SysMessageDefault(DEFMSG_CARVE_CORPSE_SCALES);
				//pPart->m_itSkin.m_creid = CorpseID;
				break;*/
			default:
				break;
		}

		if ( iQty > 1 )
			pPart->SetAmount(static_cast<unsigned int>(iQty));

		if ( pChar && pChar->m_pPlayer )
		{
			TCHAR *pszMsg = Str_GetTemp();
			sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_CORPSE_NAME), pPart->GetName(), pChar->GetName());
			pPart->SetName(pszMsg);
			pPart->m_uidLink = pChar->GetUID();
			pPart->MoveToDecay(pnt, pPart->GetDecayTime());
			continue;
		}
		pCorpse->ContentAdd(pPart);
	}

	if ( iItems < 1 )
		SysMessageDefault(DEFMSG_CARVE_CORPSE_NOTHING);

	CheckCorpseCrime(pCorpse, false, false);
	pCorpse->m_itCorpse.m_carved = 1;			// mark as been carved
	pCorpse->m_itCorpse.m_uidKiller = GetUID();	// by you

	if ( pChar && pChar->m_pPlayer )
		pCorpse->SetTimeout(0);		// reset corpse timer to make it turn bones
}

void CChar::Use_MoonGate( CItem * pItem )
{
	ADDTOCALLSTACK("CChar::Use_MoonGate");
	ASSERT(pItem);

	bool fQuiet = (pItem->m_itTelepad.m_fQuiet != 0);
	CPointMap ptTeleport = pItem->m_itTelepad.m_pntMark;
	if ( pItem->IsType(IT_MOONGATE) )
	{
		// RES_MOONGATES
		// What gate are we at ?
		size_t iCount = g_Cfg.m_MoonGates.GetCount();
		size_t i = 0;
		for ( ; ; i++ )
		{
			if ( i >= iCount )
				return;
			if ( GetTopPoint().GetDist(g_Cfg.m_MoonGates[i]) <= UO_MAP_VIEW_SIZE )
				break;
		}

		// Set it's current destination based on the moon phases.
		// ensure iTrammelPhrase isn't smaller than iFeluccaPhase, to avoid uint underflow in next calculation
		unsigned int iTrammelPhase = g_World.GetMoonPhase(false) % iCount;
		unsigned int iFeluccaPhase = g_World.GetMoonPhase(true) % iCount;
		if ( iTrammelPhase < iFeluccaPhase )
			iTrammelPhase += iCount;

		size_t iMoongateIndex = (i + (iTrammelPhase - iFeluccaPhase)) % iCount;
		ASSERT(g_Cfg.m_MoonGates.IsValidIndex(iMoongateIndex));
		ptTeleport = g_Cfg.m_MoonGates[iMoongateIndex];
	}

	if ( m_pNPC )
	{
		if ( pItem->m_itTelepad.m_fPlayerOnly )
			return;
		if ( m_pNPC->m_Brain == NPCBRAIN_GUARD )	// guards won't leave the guarded region
		{
			CRegionWorld *pArea = dynamic_cast<CRegionWorld *>(ptTeleport.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
			if ( !pArea || !pArea->IsGuarded() )
				return;
		}
		if ( Noto_IsCriminal() )	// criminals won't enter on guarded regions
		{
			CRegionWorld *pArea = dynamic_cast<CRegionWorld *>(ptTeleport.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
			if ( !pArea || pArea->IsGuarded() )
				return;
		}
	}

	// Teleport me. and take a step.
	Spell_Teleport(ptTeleport, true, pItem->IsAttr(ATTR_DECAY) ? true : false, !fQuiet);
}

bool CChar::Use_Kindling( CItem * pKindling )
{
	ADDTOCALLSTACK("CChar::Use_Kindling");
	ASSERT(pKindling);
	if ( !pKindling->IsTopLevel() )
	{
		SysMessageDefault(DEFMSG_ITEMUSE_KINDLING_CONT);
		return false;
	}

	if ( !Skill_UseQuick(SKILL_CAMPING, Calc_GetRandLLVal(30)) )
	{
		SysMessageDefault(DEFMSG_ITEMUSE_KINDLING_FAIL);
		return false;
	}

	pKindling->SetID(ITEMID_CAMPFIRE);
	pKindling->SetAttr(ATTR_MOVE_NEVER|ATTR_CAN_DECAY);
	pKindling->SetTimeout((4 + pKindling->GetAmount()) * 60 * TICK_PER_SEC);
	pKindling->SetAmount(1);	// all kindling is set to one fire
	pKindling->m_itLight.m_pattern = LIGHT_LARGE;
	pKindling->Update();
	pKindling->Sound(0x226);
	return true;
}

bool CChar::Use_Cannon_Feed( CItem * pCannon, CItem * pFeed )
{
	ADDTOCALLSTACK("CChar::Use_Cannon_Feed");
	if ( pFeed && pCannon && pCannon->IsType(IT_CANNON_MUZZLE) )
	{
		if ( !CanUse(pCannon, false) )
			return false;
		if ( !CanUse(pFeed, true) )
			return false;

		if ( pFeed->GetID() == ITEMID_REAG_SA )
		{
			if ( pCannon->m_itCannon.m_Load & 1 )
			{
				SysMessageDefault(DEFMSG_ITEMUSE_CANNON_HPOWDER);
				return false;
			}
			pCannon->m_itCannon.m_Load |= 1;
			SysMessageDefault(DEFMSG_ITEMUSE_CANNON_LPOWDER);
			return true;
		}

		if ( pFeed->IsType(IT_CANNON_BALL) )
		{
			if ( pCannon->m_itCannon.m_Load & 2 )
			{
				SysMessageDefault(DEFMSG_ITEMUSE_CANNON_HSHOT);
				return false;
			}
			pCannon->m_itCannon.m_Load |= 2;
			SysMessageDefault(DEFMSG_ITEMUSE_CANNON_LSHOT);
			return true;
		}
	}

	SysMessageDefault(DEFMSG_ITEMUSE_CANNON_EMPTY);
	return false;
}

bool CChar::Use_Train_Dummy( CItem * pItem, bool fSetup )
{
	ADDTOCALLSTACK("CChar::Use_Train_Dummy");
	// IT_TRAIN_DUMMY
	// Dummy animation timer prevents over dclicking.

	ASSERT(pItem);
	SKILL_TYPE skill = Fight_GetWeaponSkill();
	if ( g_Cfg.IsSkillFlag(skill, SKF_RANGED) )		// do not allow archery training on dummys
	{
		SysMessageDefault(DEFMSG_ITEMUSE_TDUMMY_ARCH);
		return false;
	}

	char skilltag[32];
	sprintf(skilltag, "OVERRIDE.PracticeMax.SKILL_%d", skill &~0xD2000000);
	CVarDefCont *pSkillTag = pItem->GetKey(skilltag, true);
	int iMaxSkill = pSkillTag ? static_cast<int>(pSkillTag->GetValNum()) : g_Cfg.m_iSkillPracticeMax;

	if ( Skill_GetBase(skill) > iMaxSkill )
	{
		SysMessageDefault(DEFMSG_ITEMUSE_TDUMMY_SKILL);
		return false;
	}
	if ( !pItem->IsTopLevel() )
	{
	baddumy:
		SysMessageDefault(DEFMSG_ITEMUSE_TDUMMY_P);
		return false ;
	}

	// Check location
	int dx = GetTopPoint().m_x - pItem->GetTopPoint().m_x;
	int dy = GetTopPoint().m_y - pItem->GetTopPoint().m_y;

	if ( pItem->GetDispID() == ITEMID_DUMMY1 )
	{
		if ( !(!dx && abs(dy) < 2) )
			goto baddumy;
	}
	else
	{
		if ( !(!dy && abs(dx) < 2) )
			goto baddumy;
	}

	if ( fSetup )
	{
		if ( Skill_GetActive() == NPCACT_TRAINING )
			return true;
		UpdateAnimate(ANIM_ATTACK_WEAPON);
		m_Act_TargPrv = m_uidWeapon;
		m_Act_Targ = pItem->GetUID();
		Skill_Start(NPCACT_TRAINING);
	}
	else
	{
		pItem->SetAnim(static_cast<ITEMID_TYPE>(pItem->GetID() + 1), 3 * TICK_PER_SEC);
		pItem->Sound(0x033);
		Skill_UseQuick(skill, Calc_GetRandLLVal(40));
	}
	return true;
}

bool CChar::Use_Train_PickPocketDip( CItem *pItem, bool fSetup )
{
	ADDTOCALLSTACK("CChar::Use_Train_PickPocketDip");
	// IT_TRAIN_PICKPOCKET
	// Train dummy.

	ASSERT(pItem);
	if ( Skill_GetBase(SKILL_STEALING) > g_Cfg.m_iSkillPracticeMax )
	{
		SysMessageDefault(DEFMSG_ITEMUSE_PDUMMY_SKILL);
		return true;
	}
	if ( !pItem->IsTopLevel() )
	{
	badpickpocket:
		SysMessageDefault(DEFMSG_ITEMUSE_PDUMMY_P);
		return true;
	}

	int dx = GetTopPoint().m_x - pItem->GetTopPoint().m_x;
	int dy = GetTopPoint().m_y - pItem->GetTopPoint().m_y;

	bool fNS = (pItem->GetDispID() == ITEMID_PICKPOCKET_NS || pItem->GetDispID() == ITEMID_PICKPOCKET_NS2);
	if ( fNS )
	{
		if ( !(!dx && abs(dy) < 2) )
			goto badpickpocket;
	}
	else
	{
		if ( !(!dy && abs(dx) < 2) )
			goto badpickpocket;
	}

	if ( fSetup )
	{
		if ( Skill_GetActive() == NPCACT_TRAINING )
			return true;
		m_Act_TargPrv = m_uidWeapon;
		m_Act_Targ = pItem->GetUID();
		Skill_Start(NPCACT_TRAINING);
	}
	else if ( !Skill_UseQuick(SKILL_STEALING, Calc_GetRandLLVal(40)) )
	{
		pItem->Sound(0x041);
		pItem->SetAnim(fNS ? ITEMID_PICKPOCKET_NS_FX : ITEMID_PICKPOCKET_EW_FX, 3 * TICK_PER_SEC);
		UpdateAnimate(ANIM_ATTACK_WEAPON);
	}
	else
	{
		SysMessageDefault(DEFMSG_ITEMUSE_PDUMMY_OK);
		//pItem->Sound(0x033);
	}
	return true;
}

bool CChar::Use_Train_ArcheryButte( CItem * pButte, bool fSetup )
{
	ADDTOCALLSTACK("CChar::Use_Train_ArcheryButte");
	// IT_ARCHERY_BUTTE
	ASSERT(pButte);

	ITEMID_TYPE AmmoID;
	if ( GetDist(pButte) < 2 )		// if we are standing right next to the butte, retrieve the arrows/bolts
	{
		if ( pButte->m_itArcheryButte.m_AmmoCount == 0 )
		{
			SysMessageDefault(DEFMSG_ITEMUSE_ARCHB_EMPTY);
			return true;
		}

		AmmoID = pButte->m_itArcheryButte.m_AmmoType;
		CItemBase *pAmmoDef = CItemBase::FindItemBase(AmmoID);
		if ( pAmmoDef )
		{
			TCHAR *pszMsg = Str_GetTemp();
			sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_REM), pAmmoDef->GetName(), (pButte->m_itArcheryButte.m_AmmoCount == 1) ? "" : g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_REMS));
			Emote(pszMsg);

			CItem *pRemovedAmmo = CItem::CreateBase(AmmoID);
			ASSERT(pRemovedAmmo);
			pRemovedAmmo->SetAmount(pButte->m_itArcheryButte.m_AmmoCount);
			ItemBounce(pRemovedAmmo);
		}

		// Clear the target
		pButte->m_itArcheryButte.m_AmmoType = ITEMID_NOTHING;
		pButte->m_itArcheryButte.m_AmmoCount = 0;
		return true;
	}

	SKILL_TYPE skill = Fight_GetWeaponSkill();
	if ( !g_Cfg.IsSkillFlag(skill, SKF_RANGED) )
	{
		SysMessageDefault(DEFMSG_ITEMUSE_ARCHB_WS);
		return true;
	}
	if ( Skill_GetBase(skill) > g_Cfg.m_iSkillPracticeMax )
	{
		SysMessageDefault(DEFMSG_ITEMUSE_ARCHB_SKILL);
		return true;
	}

	// Make sure we have some ammo
	CItem *pWeapon = m_uidWeapon.ItemFind();
	ASSERT(pWeapon);
	const CItemBase *pWeaponDef = pWeapon->Item_GetDef();

	// Determine ammo type
	CVarDefCont *pVarAmmoType = pWeapon->GetDefKey("AMMOTYPE", true);
	RESOURCE_ID_BASE rid;
	LPCTSTR t_Str;

	if ( pVarAmmoType )
	{
		t_Str = pVarAmmoType->GetValStr();
		rid = static_cast<RESOURCE_ID_BASE>(g_Cfg.ResourceGetID(RES_ITEMDEF, t_Str));
	}
	else
	{
		rid = pWeaponDef->m_ttWeaponBow.m_idAmmo;
	}
	AmmoID = static_cast<ITEMID_TYPE>(rid.GetResIndex());

	// If there is a different ammo type on the butte currently, tell us to remove the current type first
	if ( (pButte->m_itArcheryButte.m_AmmoType != ITEMID_NOTHING) && (pButte->m_itArcheryButte.m_AmmoType != AmmoID) )
	{
		SysMessageDefault(DEFMSG_ITEMUSE_ARCHB_X);
		return true;
	}

	// We need to be correctly aligned with the target before we can use it
	// For the south facing butte, we need to have the same X value and a Y > 2
	// For the east facing butte, we need to have the same Y value and an X > 2
	if ( !pButte->IsTopLevel() )
	{
	badalign:
		SysMessageDefault(DEFMSG_ITEMUSE_ARCHB_P);
		return true;
	}

	int targDistX = GetTopPoint().m_x - pButte->GetTopPoint().m_x;
	int targDistY = GetTopPoint().m_y - pButte->GetTopPoint().m_y;

	if ( (pButte->GetID() == ITEMID_ARCHERYBUTTE_S) || (pButte->GetID() == ITEMID_MONGBATTARGET_S) )
	{
		if ( !(targDistX == 0 && targDistY > 2) )
			goto badalign;
	}
	else
	{
		if ( !(targDistY == 0 && targDistX > 2) )
			goto badalign;
	}

	if ( !CanSeeLOS(pButte, LOS_NB_WINDOWS) )	//we should be able to shoot through a window
	{
		SysMessageDefault(DEFMSG_ITEMUSE_ARCHB_BLOCK);
		return true;
	}

	if ( fSetup )
	{
		if ( Skill_GetActive() == NPCACT_TRAINING )
			return true;
		UpdateAnimate(ANIM_ATTACK_WEAPON);
		m_Act_TargPrv = m_uidWeapon;
		m_Act_Targ = pButte->GetUID();
		Skill_Start(NPCACT_TRAINING);
		return true;
	}

	CVarDefCont *pCont = pWeapon->GetDefKey("AMMOCONT",true);

	if ( m_pPlayer && AmmoID )
	{
		int iFound = 1;
		if ( pCont )
		{
			//check for UID
			CGrayUID uidCont = static_cast<DWORD>(pCont->GetValNum());
			CItemContainer *pNewCont = dynamic_cast<CItemContainer*>(uidCont.ItemFind());
			if ( !pNewCont )	//if no UID, check for ITEMID_TYPE
			{
				t_Str = pCont->GetValStr();
				RESOURCE_ID_BASE rContid = static_cast<RESOURCE_ID_BASE>(g_Cfg.ResourceGetID(RES_ITEMDEF, t_Str));
				ITEMID_TYPE ContID = static_cast<ITEMID_TYPE>(rContid.GetResIndex());
				if ( ContID )
					pNewCont = dynamic_cast<CItemContainer*>(ContentFind(rContid));
			}

			if ( pNewCont )
				iFound = pNewCont->ContentConsume(RESOURCE_ID(RES_ITEMDEF, AmmoID));
			else
				iFound = ContentConsume(RESOURCE_ID(RES_ITEMDEF, AmmoID));
		}
		else
			iFound = ContentConsume(RESOURCE_ID(RES_ITEMDEF, AmmoID));
		if ( iFound )
		{
			SysMessageDefault(DEFMSG_ITEMUSE_ARCHB_NOAMMO);
			return(true);
		}
	}

	// OK...go ahead and fire at the target
	// Check the skill
	bool fSuccess = Skill_UseQuick(skill, Calc_GetRandLLVal(40));

	// determine animation parameters
	CVarDefCont *pVarAnim = pWeapon->GetDefKey("AMMOANIM", true);
	CVarDefCont *pVarAnimColor = pWeapon->GetDefKey("AMMOANIMHUE", true);
	CVarDefCont *pVarAnimRender = pWeapon->GetDefKey("AMMOANIMRENDER", true);
	ITEMID_TYPE AmmoAnim;
	DWORD AmmoHue;
	DWORD AmmoRender;

	if ( pVarAnim )
	{
		t_Str = pVarAnim->GetValStr();
		rid = static_cast<RESOURCE_ID_BASE>(g_Cfg.ResourceGetID(RES_ITEMDEF, t_Str));
		AmmoAnim = static_cast<ITEMID_TYPE>(rid.GetResIndex());
	}
	else
		AmmoAnim = static_cast<ITEMID_TYPE>(pWeaponDef->m_ttWeaponBow.m_idAmmoX.GetResIndex());

	AmmoHue = pVarAnimColor ? static_cast<DWORD>(pVarAnimColor->GetValNum()) : 0;
	AmmoRender = pVarAnimRender ? static_cast<DWORD>(pVarAnimRender->GetValNum()) : 0;
	
	pButte->Effect(EFFECT_BOLT, AmmoAnim, this, 16, 0, false, AmmoHue, AmmoRender);
	pButte->Sound(0x224);

	// Did we destroy the ammo?
	const CItemBase *pAmmoDef = NULL;
	if ( AmmoID )
		pAmmoDef = CItemBase::FindItemBase(AmmoID);

	if ( !fSuccess )
	{
		// Small chance of destroying the ammo
		if ( pAmmoDef && !Calc_GetRandVal(10) )
		{
			TCHAR *pszMsg = Str_GetTemp();
			sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_DEST), pAmmoDef->GetName());
			Emote(pszMsg, NULL, true);
			return true;
		}

		static LPCTSTR const sm_Txt_ArcheryButte_Failure[] =
		{
			g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_MISS_1),
			g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_MISS_2),
			g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_MISS_3),
			g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_MISS_4)
		};
		Emote(sm_Txt_ArcheryButte_Failure[Calc_GetRandVal(COUNTOF(sm_Txt_ArcheryButte_Failure))]);
	}
	else
	{
		// Very small chance of destroying another arrow
		if ( pAmmoDef && !Calc_GetRandVal(50) && pButte->m_itArcheryButte.m_AmmoCount )
		{
			TCHAR *pszMsg = Str_GetTemp();
			sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_SPLIT), pAmmoDef->GetName());
			Emote(pszMsg, NULL, true);
			return true;
		}

		static LPCTSTR const sm_Txt_ArcheryButte_Success[] =
		{
			g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_HIT_1),
			g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_HIT_2),
			g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_HIT_3),
			g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_ARCHB_HIT_4)
		};
		Emote(sm_Txt_ArcheryButte_Success[Calc_GetRandVal(COUNTOF(sm_Txt_ArcheryButte_Success))]);
	}

	// Update the target
	if ( AmmoID )
	{
		pButte->m_itArcheryButte.m_AmmoType = AmmoID;
		pButte->m_itArcheryButte.m_AmmoCount++;
	}
	return true;
}

bool CChar::Use_Item_Web( CItem * pItemWeb )
{
	ADDTOCALLSTACK("CChar::Use_Item_Web");
	// IT_WEB
	// IT_EQ_STUCK
	// Try to break out of the web.
	// Or just try to damage it.
	//
	// RETURN: true = held in place.
	//  false = walk thru it.

	if ( GetDispID() == CREID_GIANT_SPIDER || !pItemWeb || !pItemWeb->IsTopLevel() || IsStatFlag(STATF_DEAD|STATF_Insubstantial) || IsPriv(PRIV_GM) )
		return false;	// just walk through it

	// Try to break it.
	int iStr = pItemWeb->m_itWeb.m_Hits_Cur;
	if ( iStr == 0 )
		iStr = pItemWeb->m_itWeb.m_Hits_Cur = 60 + Calc_GetRandVal(250);

	// Since broken webs become spider silk, we should get out of here now if we aren't in a web.
	CItem *pFlag = LayerFind(LAYER_FLAG_Stuck);
	if ( CanMove(pItemWeb, false) )
	{
		if ( pFlag )
			pFlag->Delete();
		return false;
	}

	if ( pFlag )
	{
		if ( pFlag->IsTimerSet() )	// don't allow me to try to damage it too often
			return true;
	}

	int iDmg = pItemWeb->OnTakeDamage(Stat_GetAdjusted(STAT_STR), this);
	switch ( iDmg )
	{
		case 0:			// damage blocked
		case 1:			// web survived
		default:		// unknown
			if ( GetTopPoint() == pItemWeb->GetTopPoint() )		// is character still stuck on the web?
				break;

		case 2:			// web turned into silk
		case INT_MAX:	// web destroyed
			if ( pFlag )
				pFlag->Delete();
			return false;
	}

	// Stuck in it still.
	if ( !pFlag )
	{
		if ( iDmg < 0 )
			return false;

		// First time message.
		pFlag = CItem::CreateBase(ITEMID_WEB1_1);
		ASSERT(pFlag);
		pFlag->SetType(IT_EQ_STUCK);
		pFlag->m_uidLink = pItemWeb->GetUID();
		LayerAdd(pFlag, LAYER_FLAG_Stuck);
	}
	else
	{
		if ( iDmg < 0 )
		{
			pFlag->Delete();
			return false;
		}
		SysMessagef(g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SWEB_STUCK), pItemWeb->GetName());
	}

	pFlag->SetTimeout(TICK_PER_SEC);	// don't check it too often
	return true;
}

int CChar::Use_PlayMusic( CItem * pInstrument, int iDifficultyToPlay )
{
	ADDTOCALLSTACK("CChar::Use_PlayMusic");
	// SKILL_ENTICEMENT, SKILL_MUSICIANSHIP,
	// ARGS:
	//	iDifficultyToPlay = 0-100
	// RETURN:
	//  >=0 = success
	//  -1 = too hard for u.
	//  -2 = can't play. no instrument.

	if ( !pInstrument )
	{
		pInstrument = ContentFind(RESOURCE_ID(RES_TYPEDEF, IT_MUSICAL), 0, 1);
		if ( !pInstrument )
		{
			SysMessageDefault(DEFMSG_MUSICANSHIP_NOTOOL);
			return -2;
		}
	}

	bool fSuccess = Skill_UseQuick(SKILL_MUSICIANSHIP, iDifficultyToPlay, (Skill_GetActive() != SKILL_MUSICIANSHIP));
	Sound(pInstrument->Use_Music(fSuccess));
	if ( fSuccess )
		return iDifficultyToPlay;	// success

	// Skill gain for SKILL_MUSICIANSHIP failure will need to be triggered
	// manually, since Skill_UseQuick isn't going to do it for us in this case
	if ( Skill_GetActive() == SKILL_MUSICIANSHIP )
		Skill_Experience(SKILL_MUSICIANSHIP, -iDifficultyToPlay);

	SysMessageDefault(DEFMSG_MUSICANSHIP_POOR);
	return -1;		// fail
}

bool CChar::Use_Repair( CItem * pItemArmor )
{
	ADDTOCALLSTACK("CChar::Use_Repair");
	// Attempt to repair the item.
	// If it is repairable.

	if ( !pItemArmor || !pItemArmor->Armor_IsRepairable() )
	{
		SysMessageDefault(DEFMSG_REPAIR_NOT);
		return false;
	}

	if ( pItemArmor->IsItemEquipped() )
	{
		SysMessageDefault(DEFMSG_REPAIR_WORN);
		return false;
	}
	if ( !CanUse(pItemArmor, true) )
	{
		SysMessageDefault(DEFMSG_REPAIR_REACH);
		return false;
	}

	// Quickly use arms lore skill, but don't gain any skill until later on
	int iArmsLoreDiff = Calc_GetRandVal(30);
	if ( !Skill_UseQuick(SKILL_ARMSLORE, iArmsLoreDiff, false) )
	{
		// apply arms lore skillgain for failure
		Skill_Experience(SKILL_ARMSLORE, -iArmsLoreDiff);
		SysMessageDefault(DEFMSG_REPAIR_UNK);
		return false;
	}

	if ( pItemArmor->m_itArmor.m_Hits_Cur >= pItemArmor->m_itArmor.m_Hits_Max )
	{
		SysMessageDefault(DEFMSG_REPAIR_FULL);
		return false;
	}

	m_Act_p = g_World.FindItemTypeNearby(GetTopPoint(), IT_ANVIL, 2, false);
	if ( !m_Act_p.IsValidPoint() )
	{
		SysMessageDefault(DEFMSG_REPAIR_ANVIL);
		return false;
	}

	CItemBase *pItemDef = pItemArmor->Item_GetDef();
	ASSERT(pItemDef);

	// Use up some raw materials to repair.
	int iTotalHits = pItemArmor->m_itArmor.m_Hits_Max;
	int iDamageHits = pItemArmor->m_itArmor.m_Hits_Max - pItemArmor->m_itArmor.m_Hits_Cur;
	int iDamagePercent = IMULDIV(100, iDamageHits, iTotalHits);

	size_t iMissing = ResourceConsumePart(&(pItemDef->m_BaseResources), 1, iDamagePercent / 2, true);
	if ( iMissing != pItemDef->m_BaseResources.BadIndex() )
	{
		// Need this to repair.
		const CResourceDef *pCompDef = g_Cfg.ResourceGetDef(pItemDef->m_BaseResources.GetAt(iMissing).GetResourceID());
		SysMessagef(g_Cfg.GetDefaultMsg(DEFMSG_REPAIR_LACK_1), pCompDef ? pCompDef->GetName() : g_Cfg.GetDefaultMsg(DEFMSG_REPAIR_LACK_2));
		return false;
	}

	UpdateDir(m_Act_p);
	UpdateAnimate(ANIM_ATTACK_1H_SLASH);

	// quarter the skill to make it.
	// + more damaged items should be harder to repair.
	// higher the percentage damage the closer to the skills to make it.

	size_t iRes = pItemDef->m_SkillMake.FindResourceType(RES_SKILL);
	if ( iRes == pItemDef->m_SkillMake.BadIndex() )
		return false;

	CResourceQty RetMainSkill = pItemDef->m_SkillMake[iRes];
	int iSkillLevel = static_cast<int>(RetMainSkill.GetResQty()) / 10;
	int iDifficulty = IMULDIV(iSkillLevel, iDamagePercent, 100);
	if ( iDifficulty < iSkillLevel / 4 )
		iDifficulty = iSkillLevel / 4;

	// apply arms lore skillgain now
	LPCTSTR pszText;
	Skill_Experience(SKILL_ARMSLORE, iArmsLoreDiff);
	bool fSuccess = Skill_UseQuick(static_cast<SKILL_TYPE>(RetMainSkill.GetResIndex()), iDifficulty);
	if ( fSuccess )
	{
		pItemArmor->m_itArmor.m_Hits_Cur = static_cast<WORD>(iTotalHits);
		pszText = g_Cfg.GetDefaultMsg(DEFMSG_REPAIR_1);
	}
	else
	{
		/*****************************
		// not sure if this is working!
		******************************/
		// Failure
		if ( !Calc_GetRandVal(6) )
		{
			pszText = g_Cfg.GetDefaultMsg(DEFMSG_REPAIR_2);
			pItemArmor->m_itArmor.m_Hits_Max--;
			pItemArmor->m_itArmor.m_Hits_Cur--;
		}
		else if ( !Calc_GetRandVal(3) )
		{
			pszText = g_Cfg.GetDefaultMsg(DEFMSG_REPAIR_3);
			pItemArmor->m_itArmor.m_Hits_Cur--;
		}
		else
			pszText = g_Cfg.GetDefaultMsg( DEFMSG_REPAIR_4 );

		iDamagePercent = Calc_GetRandVal(iDamagePercent);	// some random amount
	}

	ResourceConsumePart(&(pItemDef->m_BaseResources), 1, iDamagePercent / 2, false);
	if ( pItemArmor->m_itArmor.m_Hits_Cur <= 0 )
		pszText = g_Cfg.GetDefaultMsg(DEFMSG_REPAIR_5);

	TCHAR *pszMsg = Str_GetTemp();
	sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_REPAIR_MSG), pszText, pItemArmor->GetName());
	Emote(pszMsg);

	if ( pItemArmor->m_itArmor.m_Hits_Cur <= 0 )
		pItemArmor->Delete();
	else
		pItemArmor->UpdatePropertyFlag(AUTOTOOLTIP_FLAG_DURABILITY);

	return fSuccess;
}

void CChar::Use_EatQty( CItem * pFood, int iQty )
{
	ADDTOCALLSTACK("CChar::Use_EatQty");
	// low level eat
	ASSERT(pFood);
	if ( iQty <= 0 )
		return;

	if ( iQty > pFood->GetAmount() )
		iQty = pFood->GetAmount();

	int iRestore = 0;
	if ( pFood->m_itFood.m_foodval )
		iRestore = static_cast<int>(pFood->m_itFood.m_foodval);
	else
		iRestore = pFood->Item_GetDef()->GetVolume();	// some food should have more value than other !

	if ( iRestore < 1 )
		iRestore = 1;

	int iSpace = Stat_GetMax(STAT_FOOD) - Stat_GetVal(STAT_FOOD);
	if ( iSpace <= 0 )
		return;

	if ( iQty > 1 && (iRestore * iQty > iSpace) )
		iQty = maximum(1, iSpace / iRestore);

	switch ( pFood->GetType() )
	{
		case IT_FRUIT:
		case IT_FOOD:
		case IT_FOOD_RAW:
		case IT_MEAT_RAW:
			if ( pFood->m_itFood.m_poison_skill )	// was the food poisoned?
				SetPoison(pFood->m_itFood.m_poison_skill * 10, 1 + (pFood->m_itFood.m_poison_skill / 50), this);
	}

	UpdateDir(pFood);
	EatAnim(pFood->GetName(), iRestore * iQty);
	pFood->ConsumeAmount(iQty);
}

bool CChar::Use_Eat( CItem * pItemFood, int iQty )
{
	ADDTOCALLSTACK("CChar::Use_Eat");
	// What we can eat should depend on body type.
	// How much we can eat should depend on body size and current fullness.
	//
	// ??? monsters should be able to eat corpses / raw meat
	// IT_FOOD or IT_FOOD_RAW
	// NOTE: Some foods like apples are stackable !

	if ( !CanMove(pItemFood) )
	{
		SysMessageDefault(DEFMSG_FOOD_CANTMOVE);
		return false;
	}

	if ( Stat_GetMax(STAT_FOOD) == 0 )
	{
		SysMessageDefault(DEFMSG_FOOD_CANTEAT);
		return false;
	}

	// Is this edible by me ?
	if ( !Food_CanEat(pItemFood) )
	{
		SysMessageDefault(DEFMSG_FOOD_RCANTEAT);
		return false;
	}

	if ( Stat_GetVal(STAT_FOOD) >= Stat_GetMax(STAT_FOOD) )
	{
		SysMessageDefault(DEFMSG_FOOD_CANTEATF);
		return false;
	}

	Use_EatQty(pItemFood, iQty);

	LPCTSTR pMsg;
	int index = IMULDIV(Stat_GetVal(STAT_FOOD), 5, Stat_GetMax(STAT_FOOD));
	switch ( index )
	{
		case 0:
			pMsg = g_Cfg.GetDefaultMsg(DEFMSG_FOOD_FULL_1);
			break;
		case 1:
			pMsg = g_Cfg.GetDefaultMsg(DEFMSG_FOOD_FULL_2);
			break;
		case 2:
			pMsg = g_Cfg.GetDefaultMsg(DEFMSG_FOOD_FULL_3);
			break;
		case 3:
			pMsg = g_Cfg.GetDefaultMsg(DEFMSG_FOOD_FULL_4);
			break;
		case 4:
			pMsg = g_Cfg.GetDefaultMsg(DEFMSG_FOOD_FULL_5);
			break;
		case 5:
		default:
			pMsg = g_Cfg.GetDefaultMsg(DEFMSG_FOOD_FULL_6);
			break;
	}
	SysMessage(pMsg);
	return true;
}

void CChar::Use_Drink( CItem * pItem )
{
	ADDTOCALLSTACK("CChar::Use_Drink");
	// IT_POTION:
	// IT_DRINK:
	// IT_PITCHER:
	// IT_WATER_WASH:
	// IT_BOOZE:

	if ( !CanMove(pItem) )
	{
		SysMessageDefault(DEFMSG_DRINK_CANTMOVE);
		return;
	}

	const CItemBase *pItemDef = pItem->Item_GetDef();
	ITEMID_TYPE idbottle = static_cast<ITEMID_TYPE>(RES_GET_INDEX(pItemDef->m_ttDrink.m_idEmpty));

	if ( pItem->IsType(IT_BOOZE) )
	{
		// Beer wine and liquor. vary strength of effect. m_itBooze.m_EffectStr
		int iStrength = Calc_GetRandVal(300) + 10;

		// Create ITEMID_PITCHER if drink a pitcher.
		// GLASS or MUG or Bottle ?

		// Get you Drunk, but check to see if we already are drunk
		CItem *pDrunkLayer = LayerFind(LAYER_FLAG_Drunk);
		if ( pDrunkLayer )
		{
			// lengthen/strengthen the effect
			Spell_Effect_Remove(pDrunkLayer);
			pDrunkLayer->m_itSpell.m_spellcharges += 10;
			if ( pDrunkLayer->m_itSpell.m_spelllevel < 500 )
				pDrunkLayer->m_itSpell.m_spelllevel += static_cast<WORD>(iStrength);
			Spell_Effect_Add(pDrunkLayer);
		}
		else
		{
			CItem *pSpell = Spell_Effect_Create(SPELL_Liquor, LAYER_FLAG_Drunk, iStrength, 15 * TICK_PER_SEC, this);
			pSpell->m_itSpell.m_spellcharges = 10;	// how long to last.
		}
	}

	if ( pItem->IsType(IT_POTION) )
	{
		// Time limit on using potions.
		if ( LayerFind(LAYER_FLAG_PotionUsed) )
		{
			SysMessageDefault(DEFMSG_DRINK_POTION_DELAY);
			return;
		}

		// Convey the effect of the potion.
		int iSkillQuality = pItem->m_itPotion.m_skillquality;
		if ( g_Cfg.m_iFeatureAOS & FEATURE_AOS_UPDATE_B )
		{
			int iEnhance = static_cast<int>(GetDefNum("EnhancePotions", false));
			if ( iEnhance )
				iSkillQuality += IMULDIV(iSkillQuality, iEnhance, 100);
		}

		OnSpellEffect(static_cast<SPELL_TYPE>(RES_GET_INDEX(pItem->m_itPotion.m_Type)), this, iSkillQuality, pItem);

		// Give me the marker that i've used a potion.
		Spell_Effect_Create(SPELL_NONE, LAYER_FLAG_PotionUsed, iSkillQuality, 15 * TICK_PER_SEC, this);
	}

	if ( pItem->IsType(IT_DRINK) && IsSetOF(OF_DrinkIsFood) )
	{
		int iRestore = 0;
		if ( pItem->m_itDrink.m_foodval )
			iRestore = static_cast<int>(pItem->m_itDrink.m_foodval);
		else
			iRestore = pItem->Item_GetDef()->GetVolume();

		if ( iRestore < 1 )
			iRestore = 1;

		if ( Stat_GetVal(STAT_FOOD) >= Stat_GetMax(STAT_FOOD) )
		{
			SysMessageDefault(DEFMSG_DRINK_FULL);
			return;
		}

		Stat_SetVal(STAT_FOOD, Stat_GetVal(STAT_FOOD) + iRestore);
		if ( pItem->m_itFood.m_poison_skill )
			SetPoison(pItem->m_itFood.m_poison_skill * 10, 1 + (pItem->m_itFood.m_poison_skill / 50), this);
	}

	//Sound(sm_DrinkSounds[Calc_GetRandVal(COUNTOF(sm_DrinkSounds))]);
	UpdateAnimate(ANIM_EAT);
	pItem->ConsumeAmount();

	// Create the empty bottle ?
	if ( idbottle != ITEMID_NOTHING )
		ItemBounce(CItem::CreateScript(idbottle, this));
}

CChar * CChar::Use_Figurine( CItem * pItem, bool bCheckFollowerSlots )
{
	ADDTOCALLSTACK("CChar::Use_Figurine");
	// NOTE: The figurine is NOT destroyed.
	bool bCreatedNewNpc = false;
	if ( !pItem )
		return NULL;

	if ( pItem->m_uidLink.IsValidUID() && pItem->m_uidLink.IsChar() && pItem->m_uidLink != GetUID() && !IsPriv(PRIV_GM) )
	{
		SysMessageDefault(DEFMSG_MSG_FIGURINE_NOTYOURS);
		return NULL;
	}

	// Create a new NPC if there's no one linked to this figurine 
	CChar *pPet = pItem->m_itFigurine.m_UID.CharFind();
	if ( !pPet )
	{
		CREID_TYPE id = pItem->m_itFigurine.m_ID;
		if ( !id )
		{
			id = CItemBase::FindCharTrack(pItem->GetID());
			if ( !id )
			{
				DEBUG_ERR(("FIGURINE id 0%x, no creature\n", pItem->GetDispID()));
				return NULL;
			}
		}
		bCreatedNewNpc = true;
		pPet = CreateNPC(id);
		ASSERT(pPet);
		pPet->SetName(pItem->GetName());
		if ( pItem->GetHue() )
		{
			pPet->m_prev_Hue = pItem->GetHue();
			pPet->SetHue(pItem->GetHue());
		}
	}

	if ( bCheckFollowerSlots && IsSetOF(OF_PetSlots) )
	{
		if ( !FollowersUpdate(pPet, static_cast<short>(maximum(1, pPet->GetDefNum("FOLLOWERSLOTS", true, true))), true) )
		{
			SysMessageDefault(DEFMSG_PETSLOTS_TRY_CONTROL);
			if ( bCreatedNewNpc )
				pPet->Delete();
			return NULL;
		}
	}

	if ( pPet->IsDisconnected() )
		pPet->StatFlag_Clear(STATF_Ridden);		// pull the creature out of IDLE space

	pItem->m_itFigurine.m_UID.InitUID();
	pPet->m_dirFace = m_dirFace;
	pPet->NPC_PetSetOwner(this);
	pPet->MoveToChar(pItem->GetTopLevelObj()->GetTopPoint());
	pPet->Update();
	pPet->Skill_Start(SKILL_NONE);	// was NPCACT_RIDDEN
	pPet->SoundChar(CRESND_RAND1);
	return pPet;
}

bool CChar::FollowersUpdate( CChar * pChar, short iFollowerSlots, bool bCheckOnly )
{
	ADDTOCALLSTACK("CChar::FollowersUpdate");
	// Attemp to update followers on this character based on pChar
	// bSustract = true for pet's release, shrink, etc ...
	// This is supossed to be called only when OF_PetSlots is enabled, so no need to check it here.

	if ( !bCheckOnly && IsTrigUsed(TRIGGER_FOLLOWERSUPDATE) )
	{
		CScriptTriggerArgs Args;
		Args.m_iN1 = (iFollowerSlots > 0) ? 0 : 1;
		Args.m_iN2 = abs(iFollowerSlots);
		if ( OnTrigger(CTRIG_FollowersUpdate, pChar, &Args) == TRIGRET_RET_TRUE )
			return false;

		iFollowerSlots = static_cast<short>(Args.m_iN2);
	}

	short iCurFollower = static_cast<short>(GetDefNum("CURFOLLOWER", true, true));
	short iMaxFollower = static_cast<short>(GetDefNum("MAXFOLLOWER", true, true));
	short iSetFollower = iCurFollower + iFollowerSlots;

	if ( (iSetFollower > iMaxFollower) && !IsPriv(PRIV_GM) )
		return false;

	if ( !bCheckOnly )
	{
		SetDefNum("CURFOLLOWER", maximum(iSetFollower, 0));
		UpdateStatsFlag();
	}
	return true;
}

bool CChar::Use_Key( CItem * pKey, CItem * pItemTarg )
{
	ADDTOCALLSTACK("CChar::Use_Key");
	ASSERT(pKey);
	ASSERT(pKey->IsType(IT_KEY));
	if ( !pItemTarg )
	{
		SysMessageDefault(DEFMSG_MSG_KEY_TARG);
		return false;
	}

	if ( pKey != pItemTarg && pItemTarg->IsType(IT_KEY) )
	{
		// We are trying to copy a key ?
		if ( !CanUse(pItemTarg, true) )
		{
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_REACH);
			return false;
		}

		if ( !pKey->m_itKey.m_lockUID && !pItemTarg->m_itKey.m_lockUID )
		{
			SysMessageDefault(DEFMSG_MSG_KEY_BLANKS);
			return false;
		}
		if ( pItemTarg->m_itKey.m_lockUID && pKey->m_itKey.m_lockUID )
		{
			SysMessageDefault(DEFMSG_MSG_KEY_NOTBLANKS);
			return false;
		}

		// Need tinkering tools ???
		if ( !Skill_UseQuick(SKILL_TINKERING, 30 + Calc_GetRandLLVal(40)) )
		{
			SysMessageDefault(DEFMSG_MSG_KEY_FAILC);
			return false;
		}
		if ( pItemTarg->m_itKey.m_lockUID )
			pKey->m_itKey.m_lockUID = pItemTarg->m_itKey.m_lockUID;
		else
			pItemTarg->m_itKey.m_lockUID = pKey->m_itKey.m_lockUID;
		return true;
	}

	if ( !pKey->m_itKey.m_lockUID )
	{
		SysMessageDefault(DEFMSG_MSG_KEY_ISBLANK);
		return false;
	}
	if ( pKey == pItemTarg )	// rename the key
	{
		if ( IsClient() )
			GetClient()->addPromptConsole(CLIMODE_PROMPT_NAME_KEY, g_Cfg.GetDefaultMsg(DEFMSG_MSG_KEY_SETNAME), pKey->GetUID());
		return false;
	}

	if ( !CanUse(pItemTarg, false) )
	{
		SysMessageDefault(DEFMSG_MSG_KEY_CANTREACH);
		return false;
	}

	if ( m_pArea->GetResourceID() == pKey->m_itKey.m_lockUID )
	{
		if ( Use_MultiLockDown(pItemTarg) )
			return true;
	}

	if ( !pItemTarg->m_itContainer.m_lockUID )	// or m_itContainer.m_lockUID
	{
		SysMessageDefault(DEFMSG_MSG_KEY_NOLOCK);
		return false;
	}
	if ( !pKey->IsKeyLockFit(pItemTarg->m_itContainer.m_lockUID) )	// or m_itKey
	{
		SysMessageDefault(DEFMSG_MSG_KEY_WRONGLOCK);
		return false;
	}

	return Use_KeyChange(pItemTarg);
}

bool CChar::Use_KeyChange( CItem * pItemTarg )
{
	ADDTOCALLSTACK("CChar::Use_KeyChange");
	// Lock or unlock the item.
	switch ( pItemTarg->GetType() )
	{
		case IT_SIGN_GUMP:
			// We may rename the sign.
			if ( IsClient() )
				GetClient()->addPromptConsole(CLIMODE_PROMPT_NAME_SIGN, g_Cfg.GetDefaultMsg(DEFMSG_MSG_KEY_TARG_SIGN), pItemTarg->GetUID());
			return true;
		case IT_CONTAINER:
			pItemTarg->SetType(IT_CONTAINER_LOCKED);
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_CONT_LOCK);
			break;
		case IT_CONTAINER_LOCKED:
			pItemTarg->SetType(IT_CONTAINER);
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_CONT_ULOCK);
			break;
		case IT_SHIP_HOLD:
			pItemTarg->SetType(IT_SHIP_HOLD_LOCK);
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_HOLD_LOCK);
			break;
		case IT_SHIP_HOLD_LOCK:
			pItemTarg->SetType(IT_SHIP_HOLD);
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_HOLD_ULOCK);
			break;
		case IT_DOOR:
		case IT_DOOR_OPEN:
			pItemTarg->SetType(IT_DOOR_LOCKED);
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_DOOR_LOCK);
			break;
		case IT_DOOR_LOCKED:
			pItemTarg->SetType(IT_DOOR);
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_DOOR_ULOCK);
			break;
		case IT_SHIP_TILLER:
			if ( IsClient() )
				GetClient()->addPromptConsole(CLIMODE_PROMPT_NAME_SHIP, g_Cfg.GetDefaultMsg(DEFMSG_MSG_SHIPNAME_PROMT), pItemTarg->GetUID());
			return true;
		case IT_SHIP_PLANK:
			pItemTarg->Ship_Plank(false);	// just close it.
			if ( pItemTarg->GetType() == IT_SHIP_SIDE_LOCKED )
			{
				pItemTarg->SetType(IT_SHIP_SIDE);
				SysMessageDefault(DEFMSG_MSG_KEY_TARG_SHIP_ULOCK);
				break;
			}
			// Then fall thru and lock it.
		case IT_SHIP_SIDE:
			pItemTarg->SetType(IT_SHIP_SIDE_LOCKED);
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_SHIP_LOCK);
			break;
		case IT_SHIP_SIDE_LOCKED:
			pItemTarg->SetType(IT_SHIP_SIDE);
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_SHIP_ULOCK);
			break;
		default:
			SysMessageDefault(DEFMSG_MSG_KEY_TARG_NOLOCK);
			return false;
	}

	pItemTarg->Sound(0x049);
	return true;
}

bool CChar::Use_Seed( CItem * pSeed, CPointMap * pPoint )
{
	ADDTOCALLSTACK("CChar::Use_Seed");
	// Use the seed at the current point on the ground or some new point that i can touch.
	// IT_SEED from IT_FRUIT

	ASSERT(pSeed);
	CPointMap pt;
	if ( pPoint )
		pt = *pPoint;
	else if ( pSeed->IsTopLevel() )
		pt = pSeed->GetTopPoint();
	else
		pt = GetTopPoint();

	if ( !CanTouch(pt) )
	{
		SysMessageDefault(DEFMSG_MSG_SEED_REACH);
		return false;
	}

	// is there soil here ? IT_DIRT
	if ( !IsPriv(PRIV_GM) && !g_World.IsItemTypeNear(pt, IT_DIRT, 0, false) )
	{
		SysMessageDefault(DEFMSG_MSG_SEED_TARGSOIL);
		return(false);
	}

	const CItemBase *pItemDef = pSeed->Item_GetDef();
	ITEMID_TYPE idReset = static_cast<ITEMID_TYPE>(RES_GET_INDEX(pItemDef->m_ttFruit.m_idReset));
	if ( idReset == 0 )
	{
		SysMessageDefault(DEFMSG_MSG_SEED_NOGOOD);
		return false;
	}

	// Already a plant here ?
	CWorldSearch AreaItems(pt);
	for (;;)
	{
		CItem *pItem = AreaItems.GetItem();
		if ( !pItem )
			break;
		if ( pItem->IsType(IT_TREE) || pItem->IsType(IT_FOLIAGE) )		// there's already a tree here
		{
			SysMessageDefault(DEFMSG_MSG_SEED_ATREE);
			return false;
		}
		if ( pItem->IsType(IT_CROPS) )		// there's already a plant here
			pItem->Delete();
	}

	// plant it and consume the seed.

	CItem *pPlant = CItem::CreateScript(idReset, this);
	ASSERT(pPlant);

	pPlant->MoveToUpdate(pt);
	if ( pPlant->IsType(IT_CROPS) || pPlant->IsType(IT_FOLIAGE) )
	{
		pPlant->m_itCrop.m_ReapFruitID = pSeed->GetID();
		pPlant->Plant_CropReset();
	}
	else
	{
		pPlant->SetDecayTime(10 * g_Cfg.m_iDecay_Item);
	}

	pSeed->ConsumeAmount();
	return true;
}

bool CChar::Use_BedRoll( CItem * pItem )
{
	ADDTOCALLSTACK("CChar::Use_BedRoll");
	// IT_BEDROLL

	ASSERT(pItem);
	switch ( pItem->GetDispID() )
	{
		case ITEMID_BEDROLL_C:
			if ( !pItem->IsTopLevel() )
			{
			putonground:
				SysMessageDefault(DEFMSG_ITEMUSE_BEDROLL);
				return true;
			}
			pItem->SetID(Calc_GetRandVal(2) ? ITEMID_BEDROLL_O_EW : ITEMID_BEDROLL_O_NS);
			pItem->Update();
			return true;
		case ITEMID_BEDROLL_C_NS:
			if ( !pItem->IsTopLevel() )
				goto putonground;
			pItem->SetID(ITEMID_BEDROLL_O_NS);
			pItem->Update();
			return true;
		case ITEMID_BEDROLL_C_EW:
			if ( !pItem->IsTopLevel() )
				goto putonground;
			pItem->SetID(ITEMID_BEDROLL_O_EW);
			pItem->Update();
			return true;
		case ITEMID_BEDROLL_O_EW:
		case ITEMID_BEDROLL_O_NS:
			pItem->SetID(ITEMID_BEDROLL_C);
			pItem->Update();
			return true;
		default:
			return false;
	}
}

bool CChar::Use_Item( CItem * pItem, bool fLink )
{
	ADDTOCALLSTACK("CChar::Use_Item");
	// An NPC could use these as well.
	// don't check distance here.
	// Could be a switch or something.
	// ARGS:
	//   fLink = this is the result of a linked action.
	// RETURN:
	//   true = it activated.

	if ( !pItem )
		return false;

	if ( m_pNPC && (IsTrigUsed(TRIGGER_DCLICK) || IsTrigUsed(TRIGGER_ITEMDCLICK)) )		// for players, DClick was called before this function
	{
		if ( pItem->OnTrigger(ITRIG_DCLICK, this) == TRIGRET_RET_TRUE )
			return false;
	}

	CItemSpawn *pSpawn = static_cast<CItemSpawn*>(pItem->m_uidSpawnItem.ItemFind());
	if ( pSpawn )
		pSpawn->DelObj(pItem->GetUID());	// remove this item from it's spawn when DClicks it

	bool fAction = true;
	switch ( pItem->GetType() )
	{
		case IT_ITEM_STONE:
		{
			// Give them this item
			if ( pItem->m_itItemStone.m_wAmount == USHRT_MAX )
			{
				SysMessageDefault(DEFMSG_MSG_IT_IS_DEAD);
				return true;
			}
			if ( pItem->m_itItemStone.m_wRegenTime )
			{
				if ( pItem->IsTimerSet() )
				{
					SysMessagef(g_Cfg.GetDefaultMsg(DEFMSG_MSG_STONEREG_TIME), pItem->GetTimerDiff() / TICK_PER_SEC);
					return true;
				}
				pItem->SetTimeout(pItem->m_itItemStone.m_wRegenTime * TICK_PER_SEC);
			}
			ItemBounce(CItem::CreateTemplate(pItem->m_itItemStone.m_ItemID, GetPackSafe(), this));
			if ( pItem->m_itItemStone.m_wAmount != 0 )
			{
				pItem->m_itItemStone.m_wAmount--;
				if ( pItem->m_itItemStone.m_wAmount == 0 )
					pItem->m_itItemStone.m_wAmount = USHRT_MAX;
			}
			return true;
		}

		case IT_SEED:
			return Use_Seed(pItem, NULL);

		case IT_BEDROLL:
			return Use_BedRoll(pItem);

		case IT_KINDLING:
			return Use_Kindling(pItem);

		case IT_SPINWHEEL:
		{
			if ( fLink )
				return false;

			// Just make them spin
			pItem->SetAnim(static_cast<ITEMID_TYPE>(pItem->GetID() + 1), 2 * TICK_PER_SEC);
			SysMessageDefault(DEFMSG_ITEMUSE_SPINWHEEL);
			return true;
		}

		case IT_TRAIN_DUMMY:
		{
			if ( fLink )
				return false;

			Use_Train_Dummy(pItem, true);
			return true;
		}

		case IT_TRAIN_PICKPOCKET:
		{
			if ( fLink )
				return false;

			Use_Train_PickPocketDip(pItem, true);
			return true;
		}

		case IT_ARCHERY_BUTTE:
		{
			if ( fLink )
				return false;

			Use_Train_ArcheryButte(pItem, true);
			return true;
		}

		case IT_LOOM:
		{
			if ( fLink )
				return false;

			SysMessageDefault(DEFMSG_ITEMUSE_LOOM);
			return true;
		}

		case IT_BEE_HIVE:
		{
			if ( fLink )
				return false;

			// Get honey from it
			ITEMID_TYPE id = ITEMID_NOTHING;
			if ( !pItem->m_itBeeHive.m_honeycount )
				SysMessageDefault(DEFMSG_ITEMUSE_BEEHIVE);
			else
			{
				switch ( Calc_GetRandVal(3) )
				{
					case 1:
						id = ITEMID_JAR_HONEY;
						break;
					case 2:
						id = ITEMID_BEE_WAX;
						break;
				}
			}
			if ( id )
			{
				ItemBounce(CItem::CreateScript(id, this));
				pItem->m_itBeeHive.m_honeycount--;
			}
			else
			{
				SysMessageDefault(DEFMSG_ITEMUSE_BEEHIVE_STING);
				OnTakeDamage(Calc_GetRandVal(5), this, DAMAGE_POISON|DAMAGE_GENERAL);
			}
			pItem->SetTimeout(15 * 60 * TICK_PER_SEC);
			return true;
		}

		case IT_MUSICAL:
		{
			if ( !Skill_Wait(SKILL_MUSICIANSHIP) )
			{
				m_Act_Targ = pItem->GetUID();
				Skill_Start(SKILL_MUSICIANSHIP);
			}
			break;
		}

		case IT_CROPS:
		case IT_FOLIAGE:
		{
			// Pick cotton/hay/etc
			fAction = pItem->Plant_Use(this);
			break;
		}

		case IT_FIGURINE:
		{
			// Create the creature here
			fAction = (Use_Figurine(pItem) != NULL);
			if ( fAction )
				pItem->Delete();
			return true;
		}

		case IT_TRAP:
		case IT_TRAP_ACTIVE:
		{
			// Activate the trap (plus any linked traps)
			int iDmg = pItem->Use_Trap();
			if ( CanTouch(pItem->GetTopLevelObj()->GetTopPoint()) )
				OnTakeDamage(iDmg, NULL, DAMAGE_HIT_BLUNT|DAMAGE_GENERAL);
			break;
		}

		case IT_SWITCH:
		{
			// Switches can be linked to gates and doors and such.
			// Flip the switch graphic.
			pItem->SetSwitchState();
			break;
		}

		case IT_PORT_LOCKED:
			if ( !fLink && !IsPriv(PRIV_GM) )
			{
				SysMessageDefault(DEFMSG_ITEMUSE_PORT_LOCKED);
				return true;
			}
		case IT_PORTCULIS:
			// Open a metal gate vertically
			pItem->Use_Portculis();
			break;

		case IT_DOOR_LOCKED:
			if ( !ContentFindKeyFor(pItem) )
			{
				SysMessageDefault(DEFMSG_MSG_KEY_DOORLOCKED);
				if ( !pItem->IsTopLevel() )
					return false;
				if ( pItem->IsAttr(ATTR_MAGIC) )	// show it's magic face
				{
					ITEMID_TYPE id = (GetDispID() & DOOR_NORTHSOUTH) ? ITEMID_DOOR_MAGIC_SI_NS : ITEMID_DOOR_MAGIC_SI_EW;
					CItem *pFace = CItem::CreateBase(id);
					ASSERT(pFace);
					pFace->MoveToDecay(pItem->GetTopPoint(), 4 * TICK_PER_SEC);
				}
				if ( !IsPriv(PRIV_GM) )
					return true;
			}
		case IT_DOOR_OPEN:
		case IT_DOOR:
			{
				bool fOpen = pItem->Use_DoorNew(fLink);
				if ( fLink || !fOpen )	// don't link if we are just closing the door
					return true;
			}
			break;

		case IT_SHIP_PLANK:
		{
			// Close the plank if I'm inside the ship
			if ( m_pArea->IsFlag(REGION_FLAG_SHIP) && m_pArea->GetResourceID() == pItem->m_uidLink )
			{
				if ( pItem->m_itShipPlank.m_itSideType == IT_SHIP_SIDE_LOCKED && !ContentFindKeyFor(pItem) )
				{
					SysMessageDefault(DEFMSG_ITEMUSE_SHIPSIDE);
					return true;
				}
				return pItem->Ship_Plank(false);
			}
			else if ( pItem->IsTopLevel() )
			{
				// Teleport to plank if I'm outside the ship
				CPointMap pntTarg = pItem->GetTopPoint();
				pntTarg.m_z++;
				Spell_Teleport(pntTarg, true, false, false);
			}
			return true;
		}

		case IT_SHIP_SIDE_LOCKED:
			if ( !ContentFindKeyFor(pItem) )
			{
				SysMessageDefault(DEFMSG_ITEMUSE_SHIPSIDE);
				return true;
			}
		case IT_SHIP_SIDE:
			// Open the plank
			pItem->Ship_Plank(true);
			return true;

		case IT_GRAIN:
		case IT_GRASS:
		case IT_GARBAGE:
		case IT_FRUIT:
		case IT_FOOD:
		case IT_FOOD_RAW:
		case IT_MEAT_RAW:
		{
			if ( fLink )
				return false;

			Use_Eat(pItem);
			return true;
		}

		case IT_POTION:
		case IT_DRINK:
		case IT_PITCHER:
		case IT_WATER_WASH:
		case IT_BOOZE:
		{
			if ( fLink )
				return false;

			Use_Drink(pItem);
			return true;
		}

		case IT_LIGHT_OUT:		// can the light be lit?
		case IT_LIGHT_LIT:		// can the light be doused?
			fAction = pItem->Use_Light();
			break;

		case IT_CLOTHING:
		case IT_ARMOR:
		case IT_ARMOR_LEATHER:
		case IT_SHIELD:
		case IT_WEAPON_MACE_CROOK:
		case IT_WEAPON_MACE_PICK:
		case IT_WEAPON_MACE_SMITH:
		case IT_WEAPON_MACE_SHARP:
		case IT_WEAPON_SWORD:
		case IT_WEAPON_FENCE:
		case IT_WEAPON_BOW:
		case IT_WEAPON_AXE:
		case IT_WEAPON_XBOW:
		case IT_WEAPON_MACE_STAFF:
		case IT_JEWELRY:
		case IT_WEAPON_THROWING:
		{
			if ( fLink )
				return false;

			return ItemEquip(pItem);
		}

		case IT_WEB:
		{
			if ( fLink )
				return false;

			Use_Item_Web(pItem);
			return true;
		}

		case IT_SPY_GLASS:
		{
			if ( fLink )
				return false;

			// Spyglass will tell you the moon phases
			static LPCTSTR const sm_sPhases[8] =
			{
				g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_M1),
				g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_M2),
				g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_M3),
				g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_M4),
				g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_M5),
				g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_M6),
				g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_M7),
				g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_M8)
			};
			SysMessagef(g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_TR), sm_sPhases[g_World.GetMoonPhase(false)]);
			SysMessagef(g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SPYGLASS_FE), sm_sPhases[g_World.GetMoonPhase(true)]);

			if ( m_pArea && m_pArea->IsFlag(REGION_FLAG_SHIP) )
				ObjMessage(pItem->Use_SpyGlass(this), this);
			return true;
		}

		case IT_SEXTANT:
		{
			if ( fLink )
				return false;

			if ( (GetTopPoint().m_map <= 1) && (GetTopPoint().m_x > UO_SIZE_X_REAL) )	// dungeons and T2A lands
				ObjMessage(g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SEXTANT_T2A), this);
			else
			{
				TCHAR *pszMsg = Str_GetTemp();
				sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_ITEMUSE_SEXTANT), m_pArea->GetName(), pItem->Use_Sextant(GetTopPoint()));
				ObjMessage(pszMsg, this);
			}
			return true;
		}

		default:
			fAction = false;
	}

	// Try to follow the link as well (if it has one)
	CItem *pLinkItem = pItem->m_uidLink.ItemFind();
	if ( pLinkItem && (pLinkItem != pItem) )
	{
		CItem *pItemFirst = NULL;		// watch out for loops
		size_t iCount = 0;
		if ( fLink )
		{
			if ( pItemFirst == pItem )
				return true;	// kill the loop
			if ( ++iCount > 64 )
				return true;
		}
		else
		{
			pItemFirst = pItem;
			iCount = 0;
		}
		fAction |= Use_Item(pLinkItem, true);
	}

	return fAction;
}

bool CChar::Use_Obj( CObjBase * pObj, bool fTestTouch, bool fScript  )
{
	ADDTOCALLSTACK("CChar::Use_Obj");
	if ( !pObj )
		return false;
	if ( IsClient() )
		return GetClient()->Event_DoubleClick(pObj->GetUID(), false, fTestTouch, fScript);
	else
		return Use_Item(dynamic_cast<CItem*>(pObj), fTestTouch);
}

bool CChar::ItemEquipArmor( bool fForce )
{
	ADDTOCALLSTACK("CChar::ItemEquipArmor");
	// Equip ourselves as best as possible.

	CCharBase *pCharDef = Char_GetDef();
	CItemContainer *pPack = GetPack();
	if ( !pPack || !pCharDef || !pCharDef->Can(CAN_C_EQUIP) )
		return false;

	int iBestScore[LAYER_HORSE];
	memset(iBestScore, 0, sizeof(iBestScore));
	CItem *pBestArmor[LAYER_HORSE];
	memset(pBestArmor, 0, sizeof(pBestArmor));

	if ( !fForce )
	{
		// Block those layers that are already used
		for ( size_t i = 0; i < COUNTOF(iBestScore); i++ )
		{
			pBestArmor[i] = LayerFind(static_cast<LAYER_TYPE>(i));
			if ( pBestArmor[i] != NULL )
				iBestScore[i] = INT_MAX;
		}
	}

	for ( CItem *pItem = pPack->GetContentHead(); pItem != NULL; pItem = pItem->GetNext() )
	{
		int iScore = pItem->Armor_GetDefense();
		if ( !iScore )	// might not be armor
			continue;

		// Can I even equip this?
		LAYER_TYPE layer = CanEquipLayer(pItem, LAYER_QTY, NULL, true);
		if ( layer == LAYER_NONE )
			continue;

		if ( iScore > iBestScore[layer] )
		{
			iBestScore[layer] = iScore;
			pBestArmor[layer] = pItem;
		}
	}

	// Equip all the stuff we found
	for ( size_t i = 0; i < COUNTOF(iBestScore); i++ )
	{
		if ( pBestArmor[i] )
			ItemEquip(pBestArmor[i], this);
	}

	return true;
}

bool CChar::ItemEquipWeapon( bool fForce )
{
	ADDTOCALLSTACK("CChar::ItemEquipWeapon");
	// Find my best weapon and equip it
	if ( !fForce && m_uidWeapon.IsValidUID() )	// we already have a weapon equipped
		return true;

	CCharBase *pCharDef = Char_GetDef();
	CItemContainer *pPack = GetPack();

	if ( !pPack || !pCharDef || !pCharDef->Can(CAN_C_USEHANDS) )
		return false;

	// Loop through all my weapons and come up with a score for it's usefulness

	CItem *pBestWeapon = NULL;
	int iWeaponScoreMax = NPC_GetWeaponUseScore(NULL);	// wrestling

	for ( CItem *pItem = pPack->GetContentHead(); pItem != NULL; pItem = pItem->GetNext() )
	{
		int iWeaponScore = NPC_GetWeaponUseScore(pItem);
		if ( iWeaponScore > iWeaponScoreMax )
		{
			iWeaponScoreMax = iWeaponScore;
			pBestWeapon = pItem;
		}
	}

	if ( pBestWeapon )
		return ItemEquip(pBestWeapon);

	return true;
}
