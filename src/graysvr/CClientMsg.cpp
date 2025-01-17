// Game server messages. (No login stuff)
#include "graysvr.h"	// predef header.
#include "CClient.h"
#include "../network/network.h"
#include "../network/send.h"

// Simple string hashing algorithm function
// Founded by D. J. Bernstein
// Original code found at: http://www.cse.yorku.ca/~oz/hash.html
unsigned long HashString(LPCTSTR str, size_t length)
{
    unsigned long hash = 5381;
    for (size_t i = 0; i < length; i++)
	    hash = ((hash << 5) + hash) + *str++;

    return hash;
}

/////////////////////////////////////////////////////////////////
// -CClient stuff.

void CClient::resendBuffs()
{
	// These checks are in addBuff too, but it would be useless to call it so many times
	if ( !IsSetOF(OF_Buffs) )
		return;
	if ( PacketBuff::CanSendTo(GetNetState()) == false )
		return;

	// Skills
	CChar *pChar = GetChar();
	ASSERT(pChar);

	if ( pChar->IsStatFlag(STATF_Hidden|STATF_Insubstantial) )
		addBuff( BI_HIDDEN, 1075655, 1075656 );

	// Spells
	CContainer *Cont = dynamic_cast<CContainer*>(pChar);
	ASSERT(Cont);

	TCHAR NumBuff[7][8];
	LPCTSTR pNumBuff[7] = { NumBuff[0], NumBuff[1], NumBuff[2], NumBuff[3], NumBuff[4], NumBuff[5], NumBuff[6] };

	short iStatEffect = 0;
	short iTimerEffect = 0;

	for ( size_t i = 0; i < Cont->GetCount(); ++i )
	{
		CItem *pSpell = Cont->GetAt(i);
		if ( !pSpell || !pSpell->IsType(IT_SPELL) )
			continue;

		iStatEffect = static_cast<short>(pSpell->m_itSpell.m_spelllevel);
		iTimerEffect = static_cast<short>(pSpell->GetTimerAdjusted());
		
		// TO-DO: Fix buff icons not showing correct timer value on client login
		// Strangely the buff works fine when added (cast spell), but on resend (client login)
		// it always use timer=0 even if the timer value is correct and added properly

		switch( pSpell->m_itSpell.m_spell )
		{
		case SPELL_Night_Sight:
			addBuff( BI_NIGHTSIGHT, 1075643, 1075644, iTimerEffect );
			break;
		case SPELL_Clumsy:
			ITOA(iStatEffect, NumBuff[0], 10);
			addBuff( BI_CLUMSY, 1075831, 1075832, iTimerEffect, pNumBuff, 1 );
			break;
		case SPELL_Weaken:
			ITOA(iStatEffect, NumBuff[0], 10);
			addBuff( BI_WEAKEN, 1075837, 1075838, iTimerEffect, pNumBuff, 1 );
			break;
		case SPELL_Feeblemind:
			ITOA(iStatEffect, NumBuff[0], 10);
			addBuff( BI_FEEBLEMIND, 1075833, 1075834, iTimerEffect, pNumBuff, 1 );
			break;
		case SPELL_Curse:
		{
			for ( int idx = STAT_STR; idx < STAT_BASE_QTY; ++idx )
				ITOA(iStatEffect, NumBuff[idx], 10);
			for ( int idx = 3; idx < 7; ++idx )
				ITOA(10, NumBuff[idx], 10);

			addBuff(BI_CURSE, 1075835, 1075836, iTimerEffect, pNumBuff, 7);
			break;
		}
		case SPELL_Mass_Curse:
		{
			for ( int idx = STAT_STR; idx < STAT_BASE_QTY; ++idx )
				ITOA(iStatEffect, NumBuff[idx], 10);

			addBuff(BI_MASSCURSE, 1075839, 1075840, iTimerEffect, pNumBuff, 3);
			break;
		}
		case SPELL_Strength:
			ITOA(iStatEffect, NumBuff[0], 10);
			addBuff( BI_STRENGTH, 0x106A85, 0x106A86, iTimerEffect, pNumBuff, 1 );
			break;
		case SPELL_Agility:
			ITOA(iStatEffect, NumBuff[0], 10);
			addBuff( BI_AGILITY, 0x106A85, 0x106A86, iTimerEffect, pNumBuff, 1 );
			break;
		case SPELL_Cunning:
			ITOA(iStatEffect, NumBuff[0], 10);
			addBuff( BI_CUNNING, 0x106A85, 0x106A86, iTimerEffect, pNumBuff, 1 );
			break;
		case SPELL_Bless:
		{
			for ( int idx = STAT_STR; idx < STAT_BASE_QTY; ++idx )
				ITOA(iStatEffect, NumBuff[idx], 10);

			addBuff( BI_BLESS, 1075847, 1075848, iTimerEffect, pNumBuff, STAT_BASE_QTY );
			break;
		}
		case SPELL_Reactive_Armor:
		{
			if ( IsSetCombatFlags(COMBAT_ELEMENTAL_ENGINE) )
			{
				ITOA(iStatEffect, NumBuff[0], 10);
				for ( int idx = 1; idx < 5; ++idx )
					ITOA(5, NumBuff[idx], 10);

				addBuff( BI_REACTIVEARMOR, 1075812, 1075813, iTimerEffect, pNumBuff, 5 );
			}
			else
			{
				addBuff( BI_REACTIVEARMOR, 1075812, 1070722, iTimerEffect );
			}
			break;
		}
		case SPELL_Protection:
		case SPELL_Arch_Prot:
		{
			BUFF_ICONS BuffIcon = BI_PROTECTION;
			DWORD BuffCliloc = 1075814;
			if ( pSpell->m_itSpell.m_spell == SPELL_Arch_Prot )
			{
				BuffIcon = BI_ARCHPROTECTION;
				BuffCliloc = 1075816;
			}

			if ( IsSetCombatFlags(COMBAT_ELEMENTAL_ENGINE) )
			{
				ITOA(static_cast<int>(-pSpell->m_itSpell.m_PolyStr), NumBuff[0], 10);
				ITOA(static_cast<int>(-pSpell->m_itSpell.m_PolyDex/10), NumBuff[1], 10);
				addBuff(BuffIcon, BuffCliloc, 1075815, iTimerEffect, pNumBuff, 2);
			}
			else
			{
				addBuff( BuffIcon, BuffCliloc, 1070722, iTimerEffect );
			}
			break;
		}
		case SPELL_Poison:
			addBuff( BI_POISON, 1017383, 1070722, iTimerEffect );
			break;
		case SPELL_Incognito:
			addBuff( BI_INCOGNITO, 1075819, 1075820, iTimerEffect );
			break;
		case SPELL_Paralyze:
			addBuff( BI_PARALYZE, 1075827, 1075828, iTimerEffect );
			break;
		case SPELL_Magic_Reflect:
		{
			if ( IsSetCombatFlags(COMBAT_ELEMENTAL_ENGINE) )
			{
				ITOA(-iStatEffect, NumBuff[0], 10);
				for ( int idx = 1; idx < 5; ++idx )
					ITOA(10, NumBuff[idx], 10);

				addBuff( BI_MAGICREFLECTION, 1075817, 1075818, iTimerEffect, pNumBuff, 5 );
			}
			else
			{
				addBuff( BI_MAGICREFLECTION, 1075817, 1070722, iTimerEffect );
			}
			break;
		}
		case SPELL_Invis:
			addBuff( BI_INVISIBILITY, 1075825, 1075826, iTimerEffect );
			break;
		default:
			break;
		}

	}
}

void CClient::addBuff( const BUFF_ICONS IconId, const DWORD ClilocOne, const DWORD ClilocTwo, const short Time, LPCTSTR* pArgs, size_t iArgCount)
{
	ADDTOCALLSTACK("CClient::addBuff");
	if ( !IsSetOF(OF_Buffs) )
		return;
	if ( PacketBuff::CanSendTo(GetNetState()) == false )
		return;

	new PacketBuff(this, IconId, ClilocOne, ClilocTwo, Time, pArgs, iArgCount);
}

void CClient::removeBuff(const BUFF_ICONS IconId)
{
	ADDTOCALLSTACK("CClient::removeBuff");
	if ( !IsSetOF(OF_Buffs) )
		return;
	if ( PacketBuff::CanSendTo(GetNetState()) == false )
		return;

	new PacketBuff(this, IconId);
}


bool CClient::addDeleteErr(BYTE code, unsigned int iSlot)
{
	ADDTOCALLSTACK("CClient::addDeleteErr");
	// code
	if (code == PacketDeleteError::Success)
		return true;
	CChar * pChar = m_tmSetupCharList[iSlot].CharFind();
	g_Log.EventWarn("%lx:Bad Char Delete Attempted %d (acct='%s', char='%s', ip='%s')\n", GetSocketID(), code, GetAccount()->GetName(), ((pChar != NULL) ? pChar->GetName() : ""), GetPeerStr());
	new PacketDeleteError(this, static_cast<PacketDeleteError::Reason>(code));
	return( false );
}

void CClient::addTime( bool bCurrent )
{
	ADDTOCALLSTACK("CClient::addTime");
	// Send time. (real or game time ??? why ?)

	if ( bCurrent )
	{
		long long lCurrentTime = (CServTime::GetCurrentTime()).GetTimeRaw();
		new PacketGameTime(this, 
								( lCurrentTime / ( 60*60*TICK_PER_SEC )) % 24,
								( lCurrentTime / ( 60*TICK_PER_SEC )) % 60,
								( lCurrentTime / ( TICK_PER_SEC )) % 60);
	}
	else
	{
		new PacketGameTime(this);
	}
}

void CClient::addObjectRemoveCantSee( CGrayUID uid, LPCTSTR pszName )
{
	ADDTOCALLSTACK("CClient::addObjectRemoveCantSee");
	// Seems this object got out of sync some how.
	if ( pszName == NULL ) pszName = "it";
	SysMessagef( "You can't see %s", pszName );
	addObjectRemove( uid );
}

void CClient::closeContainer( const CObjBase * pObj )
{
	ADDTOCALLSTACK("CClient::closeContainer");
	new PacketCloseContainer(this, pObj);
}

void CClient::closeUIWindow( const CChar* character, DWORD command )
{
	ADDTOCALLSTACK("CClient::closeUIWindow");
	new PacketCloseUIWindow(this, character, command);
}

void CClient::addObjectRemove( CGrayUID uid )
{
	ADDTOCALLSTACK("CClient::addObjectRemove");
	// Tell the client to remove the item or char
	new PacketRemoveObject(this, uid);
}

void CClient::addObjectRemove( const CObjBase * pObj )
{
	ADDTOCALLSTACK("CClient::addObjectRemove");
	addObjectRemove( pObj->GetUID());
}

void CClient::addRemoveAll( bool fItems, bool fChars )
{
	ADDTOCALLSTACK("CClient::addRemoveAll");
	if ( fItems )
	{
		// Remove any multi objects first ! or client will hang
		CWorldSearch AreaItems(GetChar()->GetTopPoint(), UO_MAP_VIEW_RADAR);
		AreaItems.SetSearchSquare(true);
		for (;;)
		{
			CItem * pItem = AreaItems.GetItem();
			if ( pItem == NULL )
				break;
			addObjectRemove(pItem);
		}
	}
	if ( fChars )
	{
		CChar * pCharSrc = GetChar();
		CWorldSearch AreaChars(GetChar()->GetTopPoint(), UO_MAP_VIEW_SIZE);
		AreaChars.SetAllShow(IsPriv(PRIV_ALLSHOW));
		AreaChars.SetSearchSquare(true);
		for (;;)
		{
			CChar * pChar = AreaChars.GetChar();
			if ( pChar == NULL )
				break;
			if ( pChar == pCharSrc )
				continue;
			addObjectRemove(pChar);
		}
	}
}

void CClient::addItem_OnGround( CItem * pItem ) // Send items (on ground)
{
	ADDTOCALLSTACK("CClient::addItem_OnGround");
	ASSERT(pItem);
	
	if ( PacketItemWorldNew::CanSendTo(GetNetState()) )
		new PacketItemWorldNew(this, pItem);
	else
		new PacketItemWorld(this, pItem);

	// send KR drop confirmation
	if ( PacketDropAccepted::CanSendTo(GetNetState()) )
		new PacketDropAccepted(this);

	// send item sound
	if (pItem->IsType(IT_SOUND))
	{
		addSound(static_cast<SOUND_TYPE>(pItem->m_itSound.m_Sound), pItem, pItem->m_itSound.m_Repeat );
	}

	// send corpse clothing
	if (IsPriv(PRIV_DEBUG) == false && (pItem->GetDispID() == ITEMID_CORPSE && CCharBase::IsPlayableID(pItem->GetCorpseType())) )	// cloths on corpse
	{
		CItemCorpse* pCorpse = dynamic_cast <CItemCorpse*> (pItem);
		if (pCorpse != NULL)
		{
			// send all the items on the corpse.
			addContents( pCorpse, false, true, false );
			// equip the proper items on the corpse.
			addContents( pCorpse, true, true, false );
		}
	}

	// send item tooltip
	addAOSTooltip(pItem);

	if ( (pItem->IsType(IT_MULTI_CUSTOM)) && (m_pChar->GetTopPoint().GetDistSight(pItem->GetTopPoint()) <= UO_MAP_VIEW_SIZE) )
	{
		// send house design version
		CItemMultiCustom * pItemMulti = dynamic_cast <CItemMultiCustom*> (pItem);
		if (pItemMulti != NULL)
			pItemMulti->SendVersionTo(this);
	}
}

void CClient::addItem_Equipped( const CItem * pItem )
{
	ADDTOCALLSTACK("CClient::addItem_Equipped");
	ASSERT(pItem);
	// Equip a single item on a CChar.
	CChar * pChar = dynamic_cast <CChar*> (pItem->GetParent());
	ASSERT( pChar != NULL );

	if ( ! m_pChar->CanSeeItem( pItem ) && m_pChar != pChar )
		return;

	new PacketItemEquipped(this, pItem);

	addAOSTooltip( pItem );
}

void CClient::addItem_InContainer( const CItem * pItem )
{
	ADDTOCALLSTACK("CClient::addItem_InContainer");
	ASSERT(pItem);
	CItemContainer * pCont = dynamic_cast <CItemContainer*> (pItem->GetParent());
	if ( pCont == NULL )
		return;

	new PacketItemContainer(this, pItem);
	
	if ( PacketDropAccepted::CanSendTo(GetNetState()) )
		new PacketDropAccepted(this);

	addAOSTooltip( pItem );
}

void CClient::addItem( CItem * pItem )
{
	ADDTOCALLSTACK("CClient::addItem");
	if ( pItem == NULL )
		return;
	if ( pItem->IsTopLevel())
	{
		addItem_OnGround( pItem );
	}
	else if ( pItem->IsItemEquipped())
	{
		addItem_Equipped( pItem );
	}
	else if ( pItem->IsItemInContainer())
	{
		addItem_InContainer( pItem );
	}
}

void CClient::addContents( const CItemContainer * pContainer, bool fCorpseEquip, bool fCorpseFilter, bool fShop, bool bExtra) // Send Backpack (with items)
{
	ADDTOCALLSTACK("CClient::addContents");
	// NOTE: We needed to send the header for this FIRST !!!
	// 1 = equip a corpse
	// 0 = contents.

	if (fCorpseEquip == true)
		new PacketCorpseEquipment(this, pContainer);
	else
		new PacketItemContents(this, pContainer, fShop, fCorpseFilter, bExtra);

	return;
}



void CClient::addOpenGump( const CObjBase * pContainer, GUMP_TYPE gump, bool IsVendorGump )
{
	ADDTOCALLSTACK("CClient::addOpenGump");
	// NOTE: if pContainer has not already been sent to the client
	//  this will crash client.
	new PacketContainerOpen(this, pContainer, gump, IsVendorGump);
}

bool CClient::addContainerSetup( const CItemContainer * pContainer ) // Send Backpack (with items)
{
	ADDTOCALLSTACK("CClient::addContainerSetup");
	ASSERT(pContainer);
	ASSERT( pContainer->IsItem());

	// open the container with the proper GUMP.
	CItemBase * pItemDef = pContainer->Item_GetDef();
	GUMP_TYPE gump = pItemDef->IsTypeContainer();
	if (!pItemDef)
		return false;

	if ( gump <= GUMP_RESERVED )
		return false;

	OpenPacketTransaction transaction(this, PacketSend::PRI_NORMAL);

	addOpenGump(pContainer, gump);
	addContents(pContainer, false, false, false);

	LogOpenedContainer(pContainer);
	return true;
}

void CClient::LogOpenedContainer(const CItemContainer* pContainer) // record a container in opened container list
{
	ADDTOCALLSTACK("CClient::LogOpenedContainer");
	if (pContainer == NULL)
		return;

	CObjBaseTemplate * pTopMostContainer = pContainer->GetTopLevelObj();
	CObjBase * pTopContainer = pContainer->GetContainer();

	DWORD dwTopMostContainerUID = pTopMostContainer->GetUID().GetPrivateUID();
	DWORD dwTopContainerUID = 0;
	
	if ( pTopContainer != NULL )
		dwTopContainerUID = pTopContainer->GetUID().GetPrivateUID();
	else
		dwTopContainerUID = dwTopMostContainerUID;

	m_openedContainers[pContainer->GetUID().GetPrivateUID()] = std::make_pair(
																std::make_pair( dwTopContainerUID, dwTopMostContainerUID ),
																pTopMostContainer->GetTopPoint()
															    );
}

void CClient::addSeason(SEASON_TYPE season)
{
	ADDTOCALLSTACK("CClient::addSeason");
	if ( m_pChar->IsStatFlag(STATF_DEAD) )		// everything looks like this when dead.
		season = SEASON_Desolate;
	if ( season == m_Env.m_Season )	// the season i saw last.
		return;

	m_Env.m_Season = season;

	new PacketSeason(this, season, true);

	// client resets light level on season change, so resend light here too
	m_Env.m_Light = UCHAR_MAX;
	addLight();
}

void CClient::addWeather( WEATHER_TYPE weather ) // Send new weather to player
{
	ADDTOCALLSTACK("CClient::addWeather");
	ASSERT( m_pChar );

	if ( g_Cfg.m_fNoWeather )
		return;

	if ( m_pChar->IsStatFlag( STATF_InDoors ))
	{
		// If there is a roof over our head at the moment then stop rain.
		weather = WEATHER_DRY;
	}
	else if ( weather == WEATHER_DEFAULT )
	{
		weather = m_pChar->GetTopSector()->GetWeather();
	}

	if ( weather == m_Env.m_Weather )
		return;

	m_Env.m_Weather = weather;
	new PacketWeather(this, weather, 0x40, 0x10);
}

void CClient::addLight()
{
	ADDTOCALLSTACK("CClient::addLight");
	// NOTE: This could just be a flash of light.
	// Global light level.
	ASSERT(m_pChar);
	BYTE iLight = UCHAR_MAX;

	if ( m_pChar->m_LocalLight )
		iLight = m_pChar->m_LocalLight;

	if ( iLight == UCHAR_MAX )
		iLight = m_pChar->GetLightLevel();
		
	// Scale light level for non-t2a.
	if ( iLight < LIGHT_BRIGHT )
		iLight = LIGHT_BRIGHT;
	else if ( iLight > LIGHT_DARK )
		iLight = LIGHT_DARK;

	if ( iLight == m_Env.m_Light )
		return;

	m_Env.m_Light = iLight;
	new PacketGlobalLight(this, iLight);
}

void CClient::addArrowQuest( int x, int y, int id )
{
	ADDTOCALLSTACK("CClient::addArrowQuest");

	new PacketArrowQuest(this, x, y, id);
}

void CClient::addMusic( MIDI_TYPE id )
{
	ADDTOCALLSTACK("CClient::addMusic");
	// Music is ussually appropriate for the region.
	new PacketPlayMusic(this, id);
}

bool CClient::addKick( CTextConsole * pSrc, bool fBlock )
{
	ADDTOCALLSTACK("CClient::addKick");
	// Kick me out.
	ASSERT( pSrc );
	if ( GetAccount() == NULL )
	{
		GetNetState()->markReadClosed();
		return( true );
	}

	if ( ! GetAccount()->Kick( pSrc, fBlock ))
		return( false );

	LPCTSTR pszAction = fBlock ? "KICK" : "DISCONNECT";
	SysMessagef( "You have been %sed by '%s'", static_cast<LPCTSTR>(pszAction), static_cast<LPCTSTR>(pSrc->GetName()));

	if ( IsConnectTypePacket() )
	{
		new PacketKick(this);
	}

	GetNetState()->markReadClosed();
	return( true );
}

void CClient::addSound( SOUND_TYPE id, const CObjBaseTemplate * pBase, int iOnce )
{
	ADDTOCALLSTACK("CClient::addSound");
	if ( !g_Cfg.m_fGenericSounds )
		return;

	CPointMap pt;
	if ( pBase )
	{
		pBase = pBase->GetTopLevelObj();
		pt = pBase->GetTopPoint();
	}
	else
		pt = m_pChar->GetTopPoint();

	if (( id > 0 ) && !iOnce && !pBase )
		return;

	new PacketPlaySound(this, id, iOnce, 0, pt);
}

void CClient::addBarkUNICODE( const NCHAR * pwText, const CObjBaseTemplate * pSrc, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, CLanguageID lang )
{
	ADDTOCALLSTACK("CClient::addBarkUNICODE");
	if ( pwText == NULL )
		return;

	if ( ! IsConnectTypePacket())
	{
		// Need to convert back from unicode !
		//SysMessage(pwText);
		return;
	}

	if ( mode == TALKMODE_BROADCAST )
	{
		mode = TALKMODE_SYSTEM;
		pSrc = NULL;
	}

	new PacketMessageUNICODE(this, pwText, pSrc, wHue, mode, font, lang);
}

void CClient::addBarkLocalized( int iClilocId, const CObjBaseTemplate * pSrc, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, LPCTSTR pArgs )
{
	ADDTOCALLSTACK("CClient::addBarkLocalized");
	if ( iClilocId <= 0 )
		return;

	if ( !IsConnectTypePacket() )
		return;

	if ( mode == TALKMODE_BROADCAST )
	{
		mode = TALKMODE_SYSTEM;
		pSrc = NULL;
	}

	new PacketMessageLocalised(this, iClilocId, pSrc, wHue, mode, font, pArgs);
}

void CClient::addBarkLocalizedEx( int iClilocId, const CObjBaseTemplate * pSrc, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, AFFIX_TYPE affix, LPCTSTR pAffix, LPCTSTR pArgs )
{
	ADDTOCALLSTACK("CClient::addBarkLocalizedEx");
	if ( iClilocId <= 0 )
		return;

	if ( !IsConnectTypePacket() )
		return;

	if ( mode == TALKMODE_BROADCAST )
	{
		mode = TALKMODE_SYSTEM;
		pSrc = NULL;
	}

	new PacketMessageLocalisedEx(this, iClilocId, pSrc, wHue, mode, font, affix, pAffix, pArgs);
}

void CClient::addBarkParse( LPCTSTR pszText, const CObjBaseTemplate * pSrc, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, bool bUnicode, LPCTSTR name)
{
	ADDTOCALLSTACK("CClient::addBarkParse");
	if ( !pszText )
		return;

	HUE_TYPE defaultHue = HUE_TEXT_DEF;
	FONT_TYPE defaultFont = FONT_NORMAL;
	int defaultUnicode = 0;

	switch ( mode )
	{
		case TALKMODE_SYSTEM:
		{
			defaultHue = static_cast<HUE_TYPE>(g_Exp.m_VarDefs.GetKeyNum("SMSG_DEF_COLOR"));
			defaultFont = static_cast<FONT_TYPE>(g_Exp.m_VarDefs.GetKeyNum("SMSG_DEF_FONT"));
			defaultUnicode = (g_Exp.m_VarDefs.GetKeyNum("SMSG_DEF_UNICODE",true) != 0);
			break;
		}
		case TALKMODE_EMOTE:
		{
			defaultHue = static_cast<HUE_TYPE>(g_Exp.m_VarDefs.GetKeyNum("EMOTE_DEF_COLOR"));
			defaultFont = static_cast<FONT_TYPE>(g_Exp.m_VarDefs.GetKeyNum("EMOTE_DEF_FONT"));
			defaultUnicode = (g_Exp.m_VarDefs.GetKeyNum("EMOTE_DEF_UNICODE",true) != 0);
			break;
		}
		case TALKMODE_SAY:
		{
			defaultHue = static_cast<HUE_TYPE>(g_Exp.m_VarDefs.GetKeyNum("SAY_DEF_COLOR"));
			defaultFont = static_cast<FONT_TYPE>(g_Exp.m_VarDefs.GetKeyNum("SAY_DEF_FONT"));
			defaultUnicode = (g_Exp.m_VarDefs.GetKeyNum("SAY_DEF_UNICODE",true) != 0);
			break;
		}
		case TALKMODE_OBJ:
		{
			defaultHue = static_cast<HUE_TYPE>(g_Exp.m_VarDefs.GetKeyNum("MSG_DEF_COLOR"));
			defaultFont = static_cast<FONT_TYPE>(g_Exp.m_VarDefs.GetKeyNum("MSG_DEF_FONT"));
			defaultUnicode = (g_Exp.m_VarDefs.GetKeyNum("MSG_DEF_UNICODE",true) != 0);
			break;
		}
		case TALKMODE_ITEM:
		{
			if ( !pSrc->IsChar() )		// Don't override color on char names to prevent conflict with notoriety color
				defaultHue = static_cast<HUE_TYPE>(g_Exp.m_VarDefs.GetKeyNum("IMSG_DEF_COLOR"));
			defaultFont = static_cast<FONT_TYPE>(g_Exp.m_VarDefs.GetKeyNum("IMSG_DEF_FONT"));
			defaultUnicode = (g_Exp.m_VarDefs.GetKeyNum("IMSG_DEF_UNICODE",true) != 0);
			break;
		}
	}

	WORD Args[] = { wHue, static_cast<WORD>(font), (bUnicode ? (WORD)1 : (WORD)0) };

	if ( *pszText == '@' )
	{
		pszText++;
		if ( *pszText == '@' ) // @@ = just a @ symbol
			goto bark_default;

		const char *s	= pszText;
		pszText		= strchr( s, ' ' );

		if ( !pszText )
			return;

		for ( int i = 0; ( s < pszText ) && ( i < 3 ); )
		{
			if ( *s == ',' ) // default value, skip it
			{
				i++;
				s++;
				continue;
			}
			Args[i] = static_cast<WORD>(Exp_GetVal(s));
			i++;

			if ( *s == ',' )
				s++;
			else
				break;	// no more args here!
		}
		pszText++;
		if ( Args[1] > FONT_QTY )
			Args[1] = static_cast<WORD>(FONT_NORMAL);
	}

	if ( Args[0] == HUE_TEXT_DEF )
		Args[0] = static_cast<WORD>(defaultHue);
	if ( Args[1] == FONT_NORMAL )
		Args[1] = static_cast<WORD>(defaultFont);
	if ( Args[2] == 0 )
		Args[2] = static_cast<WORD>(defaultUnicode);

	if ( m_BarkBuffer.IsEmpty())
	{
		m_BarkBuffer.Format( "%s%s", static_cast<LPCTSTR>(name), static_cast<LPCTSTR>(pszText));
	}

	switch ( Args[2] )
	{
		case 3:	// Extended localized message (with affixed ASCII text)
		{
            TCHAR * ppArgs[256];
			size_t iQty = Str_ParseCmds(const_cast<TCHAR *>(m_BarkBuffer.GetPtr()), ppArgs, COUNTOF(ppArgs), "," );
			int iClilocId = Exp_GetVal( ppArgs[0] );
			int iAffixType = Exp_GetVal( ppArgs[1] );
			CGString CArgs;
			for ( size_t i = 3; i < iQty; i++ )
			{
				if ( CArgs.GetLength() )
					CArgs += "\t";
				CArgs += ( !strcmp(ppArgs[i], "NULL") ? " " : ppArgs[i] );
			}

			addBarkLocalizedEx( iClilocId, pSrc, static_cast<HUE_TYPE>(Args[0]), mode, static_cast<FONT_TYPE>(Args[1]), static_cast<AFFIX_TYPE>(iAffixType), ppArgs[2], CArgs.GetPtr());
			break;
		}

		case 2:	// Localized
		{
            TCHAR * ppArgs[256];
			size_t iQty = Str_ParseCmds(const_cast<TCHAR *>(m_BarkBuffer.GetPtr()), ppArgs, COUNTOF(ppArgs), "," );
			int iClilocId = Exp_GetVal( ppArgs[0] );
			CGString CArgs;
			for ( size_t i = 1; i < iQty; i++ )
			{
				if ( CArgs.GetLength() )
					CArgs += "\t";
				CArgs += ( !strcmp(ppArgs[i], "NULL") ? " " : ppArgs[i] );
			}

			addBarkLocalized( iClilocId, pSrc, static_cast<HUE_TYPE>(Args[0]), mode, static_cast<FONT_TYPE>(Args[1]), CArgs.GetPtr());
			break;
		}

		case 1:	// Unicode
		{
			NCHAR szBuffer[ MAX_TALK_BUFFER ];
			CvtSystemToNUNICODE( szBuffer, COUNTOF(szBuffer), m_BarkBuffer.GetPtr(), -1 );
			addBarkUNICODE( szBuffer, pSrc, static_cast<HUE_TYPE>(Args[0]), mode, static_cast<FONT_TYPE>(Args[1]), 0 );
			break;
		}

		case 0:	// Ascii
		default:
		{
bark_default:
			if ( m_BarkBuffer.IsEmpty())
			{
				m_BarkBuffer.Format("%s%s", static_cast<LPCTSTR>(name), static_cast<LPCTSTR>(pszText));
			}

			addBark( m_BarkBuffer.GetPtr(), pSrc, static_cast<HUE_TYPE>(Args[0]), mode, static_cast<FONT_TYPE>(Args[1]));
			break;
		}
	}

	// Empty the buffer.
	m_BarkBuffer.Empty();
}



void CClient::addBark( LPCTSTR pszText, const CObjBaseTemplate * pSrc, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font )
{
	ADDTOCALLSTACK("CClient::addBark");
	if ( pszText == NULL )
		return;

	if ( ! IsConnectTypePacket())
	{
		SysMessage(pszText);
		return;
	}

	if ( mode == TALKMODE_BROADCAST )
	{
		mode = TALKMODE_SYSTEM;
		pSrc = NULL;
	}

	new PacketMessageASCII(this, pszText, pSrc, wHue, mode, font);
}

void CClient::addObjMessage( LPCTSTR pMsg, const CObjBaseTemplate * pSrc, HUE_TYPE wHue, TALKMODE_TYPE mode ) // The message when an item is clicked
{
	ADDTOCALLSTACK("CClient::addObjMessage");
	if ( !pMsg )
		return;

	if ( IsSetOF(OF_Flood_Protection) && ( GetPrivLevel() <= PLEVEL_Player )  )
	{
		if ( !strcmpi(pMsg, m_zLastObjMessage) )
			return;

		if ( strlen(pMsg) < SCRIPT_MAX_LINE_LEN )
			strcpy(m_zLastObjMessage, pMsg);
	}

	addBarkParse(pMsg, pSrc, wHue, mode);
}

void CClient::addEffect(EFFECT_TYPE motion, ITEMID_TYPE id, const CObjBaseTemplate * pDst, const CObjBaseTemplate * pSrc, BYTE bSpeedSeconds, BYTE bLoop, bool fExplode, DWORD color, DWORD render, WORD effectid, DWORD explodeid, WORD explodesound, DWORD effectuid, BYTE type)
{
	ADDTOCALLSTACK("CClient::addEffect");
	// bSpeedSeconds = seconds = 0=very fast, 7=slow.
	// wHue =

	ASSERT(m_pChar);
	ASSERT(pDst);

	if (pSrc == NULL && motion == EFFECT_BOLT) // source required for bolt effect
		return;

	PacketSend* cmd(NULL);
	if (effectid || explodeid)
		cmd = new PacketEffect(this, motion, id, pDst, pSrc, bSpeedSeconds, bLoop, fExplode, color, render, effectid, explodeid, explodesound, effectuid, type);
	else if (color || render)
		cmd = new PacketEffect(this, motion, id, pDst, pSrc, bSpeedSeconds, bLoop, fExplode, color, render);
	else
		cmd = new PacketEffect(this, motion, id, pDst, pSrc, bSpeedSeconds, bLoop, fExplode);
}


void CClient::GetAdjustedItemID( const CChar * pChar, const CItem * pItem, ITEMID_TYPE & id, HUE_TYPE & wHue ) const
{
	ADDTOCALLSTACK("CClient::GetAdjustedItemID");
	// An equipped item.
	ASSERT( pChar );

	id = pItem->GetDispID();
	wHue = pItem->GetHue();
	CItemBase * pItemDef = pItem->Item_GetDef();

	if ( pItem->IsType(IT_EQ_HORSE) )
	{
		// check the reslevel of the ridden horse
		CREID_TYPE idHorse = pItem->m_itFigurine.m_ID;
		CCharBase * pCharDef = CCharBase::FindCharBase(idHorse);
		if ( pCharDef && ( GetResDisp() < pCharDef->GetResLevel() ) )
		{
			idHorse = static_cast<CREID_TYPE>(pCharDef->GetResDispDnId());
			wHue = pCharDef->GetResDispDnHue();

			// adjust the item to display the mount item associated with
			// the resdispdnid of the mount's chardef
			if ( idHorse != pItem->m_itFigurine.m_ID )
			{
				TCHAR * sMountDefname = Str_GetTemp();
				sprintf(sMountDefname, "mount_0x%x", idHorse);
				ITEMID_TYPE idMountItem = static_cast<ITEMID_TYPE>(g_Exp.m_VarDefs.GetKeyNum(sMountDefname));
				if ( idMountItem > ITEMID_NOTHING )
				{
					id = idMountItem;
					pItemDef = CItemBase::FindItemBase(id);
				}
			}
		}
	}

	if ( m_pChar->IsStatFlag( STATF_Hallucinating ))
		wHue = static_cast<HUE_TYPE>(Calc_GetRandVal( HUE_DYE_HIGH ));
	else if ( pChar->IsStatFlag(STATF_Stone))
		wHue = HUE_STONE;
	else if ( pChar->IsStatFlag(STATF_Insubstantial))
		wHue = g_Cfg.m_iColorInvis;
	else
	{
		if ( pItemDef && ( GetResDisp() < pItemDef->GetResLevel() ) )
			if ( pItemDef->GetResDispDnHue() != HUE_DEFAULT )
				wHue = pItemDef->GetResDispDnHue();

		if (( wHue & HUE_MASK_HI ) > HUE_QTY )
			wHue &= HUE_MASK_LO | HUE_UNDERWEAR | HUE_TRANSLUCENT;
		else
			wHue &= HUE_MASK_HI | HUE_UNDERWEAR | HUE_TRANSLUCENT;

	}

	if ( pItemDef && ( GetResDisp() < pItemDef->GetResLevel() ) )
		id = static_cast<ITEMID_TYPE>(pItemDef->GetResDispDnId());
}

void CClient::GetAdjustedCharID( const CChar * pChar, CREID_TYPE &id, HUE_TYPE &wHue ) const
{
	ADDTOCALLSTACK("CClient::GetAdjustedCharID");
	// Some clients can't see all creature artwork and colors.
	// try to do the translation here,.

	ASSERT( GetAccount() );
	ASSERT( pChar );

	if ( IsPriv(PRIV_DEBUG) )
	{
		id = CREID_MAN;
		wHue = 0;
		return;
	}

	id = pChar->GetDispID();
	CCharBase * pCharDef = pChar->Char_GetDef();

	if ( m_pChar->IsStatFlag(STATF_Hallucinating) )
	{
		if ( pChar != m_pChar )
		{
			// Get a random creature from the artwork.
			pCharDef = NULL;
			while ( pCharDef == NULL )
			{
				id = static_cast<CREID_TYPE>(Calc_GetRandVal(CREID_EQUIP_GM_ROBE));
				if ( id != CREID_SEA_Creature )		// skip this chardef, it can crash many clients
					pCharDef = CCharBase::FindCharBase(id);
			}
		}

		wHue = static_cast<HUE_TYPE>(Calc_GetRandVal(HUE_DYE_HIGH));
	}
	else
	{
		if ( pChar->IsStatFlag(STATF_Stone) )	// turned to stone.
			wHue = HUE_STONE;
		else if ( pChar->IsStatFlag(STATF_Insubstantial) )	// turned to stone.
			wHue = g_Cfg.m_iColorInvis;
		else if ( pChar->IsStatFlag(STATF_Hidden) )	// turned to stone.
			wHue = g_Cfg.m_iColorHidden;
		else if ( pChar->IsStatFlag(STATF_Invisible) )	// turned to stone.
			wHue = g_Cfg.m_iColorInvisSpell;
		else
		{
			wHue = pChar->GetHue();
			// Allow transparency and underwear colors
			if ( (wHue & HUE_MASK_HI) > HUE_QTY )
				wHue &= HUE_MASK_LO | HUE_UNDERWEAR | HUE_TRANSLUCENT;
			else
				wHue &= HUE_MASK_HI | HUE_UNDERWEAR | HUE_TRANSLUCENT;
		}
	}

	if ( pCharDef && (GetResDisp() < pCharDef->GetResLevel()) )
	{
		id = static_cast<CREID_TYPE>(pCharDef->GetResDispDnId());
		if ( pCharDef->GetResDispDnHue() != HUE_DEFAULT )
			wHue = pCharDef->GetResDispDnHue();
	}
}

void CClient::addCharMove( const CChar * pChar )
{
	ADDTOCALLSTACK("CClient::addCharMove");

	addCharMove(pChar, pChar->GetDirFlag());
}

void CClient::addCharMove( const CChar * pChar, BYTE bCharDir )
{
	ADDTOCALLSTACK("CClient::addCharMove");
	// This char has just moved on screen.
	// or changed in a subtle way like "hidden"
	// NOTE: If i have been turned this will NOT update myself.

	new PacketCharacterMove(this, pChar, bCharDir);
}

void CClient::addChar( const CChar * pChar )
{
	ADDTOCALLSTACK("CClient::addChar");
	// Full update about a char.
	EXC_TRY("addChar");
	new PacketCharacter( this, pChar );

	EXC_SET("Wake sector");
	pChar->GetTopPoint().GetSector()->SetSectorWakeStatus();	// if it can be seen then wake it.

	EXC_SET("Health bar color");
	addHealthBarUpdate( pChar );

	if ( pChar->m_pNPC && pChar->m_pNPC->m_bonded && pChar->IsStatFlag(STATF_DEAD) )
	{
		EXC_SET("Bonded status");
		addBondedStatus(pChar, true);
	}

	EXC_SET("AOSToolTip adding (end)");
	addAOSTooltip( pChar );

	EXC_CATCH;

	EXC_DEBUG_START;
	g_Log.EventDebug("m_dirFace (0%x)\n", pChar->m_dirFace);
	EXC_DEBUG_END;
}

void CClient::addItemName( const CItem * pItem )
{
	ADDTOCALLSTACK("CClient::addItemName");
	// NOTE: CanSee() has already been called.
	ASSERT(pItem);

	bool fIdentified = ( IsPriv(PRIV_GM) || pItem->IsAttr( ATTR_IDENTIFIED ));
	LPCTSTR pszNameFull = pItem->GetNameFull( fIdentified );

	TCHAR szName[ MAX_ITEM_NAME_SIZE + 256 ];
	size_t len = strcpylen( szName, pszNameFull, COUNTOF(szName) );

	const CContainer* pCont = dynamic_cast<const CContainer*>(pItem);
	if ( pCont != NULL )
	{
		// ??? Corpses show hair as an item !!
		len += sprintf( szName+len, g_Cfg.GetDefaultMsg(DEFMSG_CONT_ITEMS), pCont->GetCount(), pCont->GetTotalWeight() / WEIGHT_UNITS);
	}

	// obviously damaged ?
	else if ( pItem->IsTypeArmorWeapon())
	{
		int iPercent = pItem->Armor_GetRepairPercent();
		if ( iPercent < 50 &&
			( m_pChar->Skill_GetAdjusted( SKILL_ARMSLORE ) / 10 > iPercent ))
		{
			len += sprintf( szName+len, " (%s)", pItem->Armor_GetRepairDesc());
		}
	}

	// Show the priced value
	CItemContainer * pMyCont = dynamic_cast <CItemContainer *>( pItem->GetParent());
	if ( pMyCont != NULL && pMyCont->IsType(IT_EQ_VENDOR_BOX))
	{
		const CItemVendable * pVendItem = dynamic_cast <const CItemVendable *> (pItem);
		if ( pVendItem )
		{
			len += sprintf( szName+len, " (%d gp)", pVendItem->GetBasePrice());
		}
	}

	HUE_TYPE wHue = HUE_TEXT_DEF;
	const CItemCorpse * pCorpseItem = dynamic_cast <const CItemCorpse *>(pItem);
	if ( pCorpseItem )
	{
		CChar * pCharCorpse = pCorpseItem->m_uidLink.CharFind();
		if ( pCharCorpse )
		{
			wHue = pCharCorpse->Noto_GetHue( m_pChar, true );
		}
	}

	if ( IsPriv( PRIV_GM ))
	{
		if ( pItem->IsAttr(ATTR_INVIS ))
		{
			len += strcpylen( szName+len, " (invis)" );
		}
		// Show the restock count
		if ( pMyCont != NULL && pMyCont->IsType(IT_EQ_VENDOR_BOX) )
		{
			len += sprintf( szName+len, " (%d restock)", pItem->GetContainedLayer());
		}
		switch ( pItem->GetType() )
		{
			case IT_SPAWN_CHAR:
			case IT_SPAWN_ITEM:
				{
					CItemSpawn *pSpawn = static_cast<CItemSpawn*>(const_cast<CItem*>(pItem));
					len += pSpawn->GetName( szName + len );
				}
				break;

			case IT_TREE:
			case IT_GRASS:
			case IT_ROCK:
			case IT_WATER:
				{
				const CResourceDef * pResDef = g_Cfg.ResourceGetDef( pItem->m_itResource.m_rid_res );
				if ( pResDef)
				{
					len += sprintf( szName+len, " (%s)", pResDef->GetName());
				}
				}
				break;

			default:
				break;
		}
	}
	if ( IsPriv(PRIV_DEBUG) )
		len += sprintf(szName+len, " [0%lx]", (DWORD) pItem->GetUID());

	if (( IsTrigUsed(TRIGGER_AFTERCLICK) ) || ( IsTrigUsed(TRIGGER_ITEMAFTERCLICK) ))
	{
		CScriptTriggerArgs Args( this );
		Args.m_VarsLocal.SetStrNew("ClickMsgText", &szName[0]);
		Args.m_VarsLocal.SetNumNew("ClickMsgHue", static_cast<int>(wHue));

		TRIGRET_TYPE ret = dynamic_cast<CObjBase*>(const_cast<CItem*>(pItem))->OnTrigger( "@AfterClick", m_pChar, &Args );	// CTRIG_AfterClick, ITRIG_AfterClick

		if ( ret == TRIGRET_RET_TRUE )
			return;

		LPCTSTR pNewStr = Args.m_VarsLocal.GetKeyStr("ClickMsgText");

		if ( pNewStr != NULL )
			strcpylen(szName, pNewStr, COUNTOF(szName));

		wHue = static_cast<HUE_TYPE>(Args.m_VarsLocal.GetKeyNum("ClickMsgHue", true));
	}

	addObjMessage( szName, pItem, wHue, TALKMODE_ITEM );
}

void CClient::addCharName( const CChar * pChar ) // Singleclick text for a character
{
	ADDTOCALLSTACK("CClient::addCharName");
	// Karma wHue codes ?
	ASSERT( pChar );

	HUE_TYPE wHue	= pChar->Noto_GetHue( m_pChar, true );

	TCHAR *pszTemp = Str_GetTemp();
	LPCTSTR prefix = pChar->GetKeyStr( "NAME.PREFIX" );
	if ( ! *prefix )
		prefix = pChar->Noto_GetFameTitle();

	strcpy( pszTemp, prefix );
	strcat( pszTemp, pChar->GetName() );
	strcat( pszTemp, pChar->GetKeyStr( "NAME.SUFFIX" ) );

	if ( !pChar->IsStatFlag(STATF_Incognito) || ( GetPrivLevel() > pChar->GetPrivLevel() ))
	{
		// Guild abbrev.
		LPCTSTR pAbbrev = pChar->Guild_AbbrevBracket(MEMORY_TOWN);
		if ( pAbbrev )
		{
			strcat( pszTemp, pAbbrev );
		}
		pAbbrev = pChar->Guild_AbbrevBracket(MEMORY_GUILD);
		if ( pAbbrev )
		{
			strcat( pszTemp, pAbbrev );
		}
	}
	else
		strcpy( pszTemp, pChar->GetName() );

	if ( pChar->m_pNPC && g_Cfg.m_fVendorTradeTitle )
	{
		if ( pChar->GetNPCBrain() == NPCBRAIN_HUMAN )
		{
			LPCTSTR title = pChar->GetTradeTitle();
			if ( *title )
			{
				strcat( pszTemp, " " );
				strcat( pszTemp, title );
			}
		}
	}

	bool fAllShow = IsPriv(PRIV_DEBUG|PRIV_ALLSHOW);

	if ( g_Cfg.m_fCharTags || fAllShow )
	{
		if ( pChar->m_pNPC )
		{
			if ( pChar->IsPlayableCharacter())
				strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_NPC) );
			if ( pChar->IsStatFlag( STATF_Conjured ))
				strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_SUMMONED) );
			else if ( pChar->IsStatFlag( STATF_Pet ))
				strcat( pszTemp, (pChar->m_pNPC->m_bonded) ? g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_BONDED) : g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_TAME) );
		}
		if ( pChar->IsStatFlag( STATF_INVUL ) && ! pChar->IsStatFlag( STATF_Incognito ) && ! pChar->IsPriv( PRIV_PRIV_NOSHOW ))
			strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_INVUL) );
		if ( pChar->IsStatFlag( STATF_Stone ))
			strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_STONE) );
		if ( pChar->IsStatFlag( STATF_Freeze ))
			strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_FROZEN) );
		if ( pChar->IsStatFlag( STATF_Insubstantial | STATF_Invisible | STATF_Hidden ))
			strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_HIDDEN) );
		if ( pChar->IsStatFlag( STATF_Sleeping ))
			strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_SLEEPING) );
		if ( pChar->IsStatFlag( STATF_Hallucinating ))
			strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_HALLU) );

		if ( fAllShow )
		{
			if ( pChar->IsStatFlag(STATF_Spawned) )
				strcat(pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_SPAWN));
			if ( IsPriv( PRIV_DEBUG ))
				sprintf(pszTemp+strlen(pszTemp), " [0%lx]", (DWORD) pChar->GetUID());
		}
	}
	if ( ! fAllShow && pChar->Skill_GetActive() == NPCACT_Napping )
		strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_AFK) );
	if ( pChar->GetPrivLevel() <= PLEVEL_Guest )
		strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_GUEST) );
	if ( pChar->IsPriv( PRIV_JAILED ))
		strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_JAILED) );
	if ( pChar->IsDisconnected())
		strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_LOGOUT) );
	if (( fAllShow || pChar == m_pChar ) && pChar->IsStatFlag( STATF_Criminal ))
		strcat( pszTemp, g_Cfg.GetDefaultMsg(DEFMSG_CHARINFO_CRIMINAL) );
	if ( fAllShow || ( IsPriv(PRIV_GM) && ( g_Cfg.m_wDebugFlags & DEBUGF_NPC_EMOTE )))
	{
		strcat( pszTemp, " [" );
		strcat( pszTemp, pChar->Skill_GetName());
		strcat( pszTemp, "]" );
	}

	if ( IsTrigUsed(TRIGGER_AFTERCLICK) )
	{
		CScriptTriggerArgs Args( this );
		Args.m_VarsLocal.SetStrNew("ClickMsgText", pszTemp);
		Args.m_VarsLocal.SetNumNew("ClickMsgHue", static_cast<int>(wHue));

		TRIGRET_TYPE ret = dynamic_cast<CObjBase*>(const_cast<CChar*>(pChar))->OnTrigger( "@AfterClick", m_pChar, &Args );	// CTRIG_AfterClick, ITRIG_AfterClick

		if ( ret == TRIGRET_RET_TRUE )
			return;

		LPCTSTR pNewStr = Args.m_VarsLocal.GetKeyStr("ClickMsgText");

		if ( pNewStr != NULL )
			strcpy(pszTemp, pNewStr);

		wHue = static_cast<HUE_TYPE>(Args.m_VarsLocal.GetKeyNum("ClickMsgHue", true));
	}

	addObjMessage( pszTemp, pChar, wHue, TALKMODE_ITEM );
}

void CClient::addPlayerStart( CChar * pChar )
{
	ADDTOCALLSTACK("CClient::addPlayerStart");

	if ( m_pChar != pChar )	// death option just usese this as a reload.
	{
		// This must be a CONTROL command ?
		CharDisconnect();
		if ( pChar->IsClient())	// not sure why this would happen but take care of it anyhow.
			pChar->GetClient()->CharDisconnect();
		m_pChar = pChar;
		m_pChar->ClientAttach( this );
	}
	ASSERT(m_pChar->m_pPlayer);

	CPointMap pt = m_pChar->GetTopPoint();
	CSector *pSector = pt.GetSector();

	CItem * pItemChange = m_pChar->LayerFind(LAYER_FLAG_ClientLinger);
	if ( pItemChange != NULL )
		pItemChange->Delete();

	ClearTargMode();	// clear death menu mode. etc. ready to walk about. cancel any previous modes
	m_Env.SetInvalid();
/*
	CExtData ExtData;
	ExtData.Party_Enable.m_state = 1;
	addExtData( EXTDATA_Party_Enable, &ExtData, sizeof(ExtData.Party_Enable));
*/

	new PacketPlayerStart(this);
	addMapDiff();
	//addChangeServer();		// we still need this?
	m_pChar->MoveToChar(pt);	// make sure we are in active list
	m_pChar->Update();
	addPlayerWarMode();
	addLoginComplete();
	addTime(true);
	if ( pSector != NULL )
		addSeason(pSector->GetSeason());
	if (pChar->m_pParty)
		pChar->m_pParty->SendAddList(NULL);

	addKRToolbar(pChar->m_pPlayer->getKrToolbarStatus());
	resendBuffs();
}

void CClient::addPlayerWarMode()
{
	ADDTOCALLSTACK("CClient::addPlayerWarMode");

	new PacketWarMode(this, m_pChar);
}

void CClient::addToolTip( const CObjBase * pObj, LPCTSTR pszText )
{
	ADDTOCALLSTACK("CClient::addToolTip");
	if ( pObj == NULL )
		return;
	if ( pObj->IsChar())
		return; // no tips on chars.

	new PacketTooltip(this, pObj, pszText);
}

bool CClient::addBookOpen( CItem * pBook )
{
	ADDTOCALLSTACK("CClient::addBookOpen");
	// word wrap is done when typed in the client. (it has a var size font)
	if (pBook == NULL)
		return false;

	size_t iPagesNow = 0;
	bool bNewPacket = PacketDisplayBookNew::CanSendTo(GetNetState());

	if (pBook->IsBookSystem() == false)
	{
		// User written book.
		CItemMessage* pMsgItem = dynamic_cast<CItemMessage*>(pBook);
		if (pMsgItem == NULL)
			return false;

		if (pMsgItem->IsBookWritable())
			iPagesNow = pMsgItem->GetPageCount(); // for some reason we must send them now
	}

	if (bNewPacket)
		new PacketDisplayBookNew(this, pBook);
	else
		new PacketDisplayBook(this, pBook);

	// We could just send all the pages now if we want.
	if (iPagesNow > 0)
		addBookPage(pBook, 1, iPagesNow);

	return( true );
}

void CClient::addBookPage( const CItem * pBook, size_t iPage, size_t iCount )
{
	ADDTOCALLSTACK("CClient::addBookPage");
	// ARGS:
	//  iPage = 1 based page.
	if ( iPage <= 0 )
		return;
	if ( iCount < 1 )
		iCount = 1;

	new PacketBookPageContent(this, pBook, iPage, iCount );
}

int CClient::Setup_FillCharList(Packet* pPacket, const CChar * pCharFirst)
{
	ADDTOCALLSTACK("CClient::Setup_FillCharList");
	// list available chars for your account that are idle.
	CAccount * pAccount = GetAccount();
	ASSERT( pAccount );

	size_t count = 0;

	if ( pCharFirst && pAccount->IsMyAccountChar( pCharFirst ))
	{
		m_tmSetupCharList[0] = pCharFirst->GetUID();

		pPacket->writeStringFixedASCII(pCharFirst->GetName(), MAX_NAME_SIZE);
		pPacket->writeStringFixedASCII("", MAX_NAME_SIZE);

		count++;
	}

	
	size_t iMax = minimum(maximum(pAccount->m_Chars.GetCharCount(), pAccount->GetMaxChars()), MAX_CHARS_PER_ACCT);

	size_t iQty = pAccount->m_Chars.GetCharCount();
	if (iQty > iMax)
		iQty = iMax;

	for (size_t i = 0; i < iQty; i++)
	{
		CGrayUID uid(pAccount->m_Chars.GetChar(i));
		CChar* pChar = uid.CharFind();
		if ( pChar == NULL )
			continue;
		if ( pCharFirst == pChar )
			continue;
		if ( count >= iMax )
			break;

		m_tmSetupCharList[count] = uid;

		pPacket->writeStringFixedASCII(pChar->GetName(), MAX_NAME_SIZE);
		pPacket->writeStringFixedASCII("", MAX_NAME_SIZE);

		count++;
	}

	// always show max count for some stupid reason. (client bug)
	// pad out the rest of the chars.
	size_t iClientMin = 5;
	if (GetNetState()->isClientVersion(MINCLIVER_PADCHARLIST) || !GetNetState()->getCryptVersion())
		iClientMin = maximum(iQty, 5);

	for ( ; count < iClientMin; count++)
	{
		pPacket->writeStringFixedASCII("", MAX_NAME_SIZE);
		pPacket->writeStringFixedASCII("", MAX_NAME_SIZE);
	}

	return count;
}

void CClient::SetTargMode( CLIMODE_TYPE targmode, LPCTSTR pPrompt, int iTimeout )
{
	ADDTOCALLSTACK("CClient::SetTargMode");
	// ??? Get rid of menu stuff if previous targ mode.
	// Can i close a menu ?
	// Cancel a cursor input.

	bool bSuppressCancelMessage = false;

	switch ( GetTargMode() )
	{
		case CLIMODE_TARG_OBJ_FUNC:
		{
			if ( IsTrigUsed(TRIGGER_TARGON_CANCEL) )
			{
				CScriptTriggerArgs Args;
				Args.m_s1 =  m_Targ_Text;
				if ( GetChar()->OnTrigger( CTRIG_Targon_Cancel, m_pChar, &Args ) == TRIGRET_RET_TRUE )
					bSuppressCancelMessage = true;
			}
		} break;
		case CLIMODE_TARG_USE_ITEM:
		{
			CItem * pItemUse = m_Targ_UID.ItemFind();
			if ( pItemUse != NULL && (( IsTrigUsed(TRIGGER_TARGON_CANCEL) ) || ( IsTrigUsed(TRIGGER_ITEMTARGON_CANCEL) ) ))
			{
				if ( pItemUse->OnTrigger( ITRIG_TARGON_CANCEL, m_pChar ) == TRIGRET_RET_TRUE )
					bSuppressCancelMessage = true;
			}
		} break;

		case CLIMODE_TARG_SKILL_MAGERY:
		{
			const CSpellDef* pSpellDef = g_Cfg.GetSpellDef(m_tmSkillMagery.m_Spell);
			if (m_pChar != NULL && pSpellDef != NULL)
			{
				CScriptTriggerArgs Args(m_tmSkillMagery.m_Spell, 0, m_Targ_PrvUID.ObjFind());

				if ( IsTrigUsed(TRIGGER_SPELLTARGETCANCEL) )
				{
					if ( m_pChar->OnTrigger( CTRIG_SpellTargetCancel, this, &Args ) == TRIGRET_RET_TRUE )
					{
						bSuppressCancelMessage = true;
						break;
					}
				}

				if ( IsTrigUsed(TRIGGER_TARGETCANCEL) )
				{
					if ( m_pChar->Spell_OnTrigger( m_tmSkillMagery.m_Spell, SPTRIG_TARGETCANCEL, m_pChar, &Args ) == TRIGRET_RET_TRUE )
						bSuppressCancelMessage = true;
				}
			}
		} break;

		case CLIMODE_TARG_SKILL:
		case CLIMODE_TARG_SKILL_HERD_DEST:
		case CLIMODE_TARG_SKILL_PROVOKE:
		case CLIMODE_TARG_SKILL_POISON:
		{
			SKILL_TYPE action = SKILL_NONE;
			switch (GetTargMode())
			{
				case CLIMODE_TARG_SKILL:
					action = m_tmSkillTarg.m_Skill;
					break;
				case CLIMODE_TARG_SKILL_HERD_DEST:
					action = SKILL_HERDING;
					break;
				case CLIMODE_TARG_SKILL_PROVOKE:
					action = SKILL_PROVOCATION;
					break;
				case CLIMODE_TARG_SKILL_POISON:
					action = SKILL_POISONING;
					break;
				default:
					break;
			}

			if (action != SKILL_NONE && m_pChar != NULL)
			{
				if ( IsTrigUsed(TRIGGER_SKILLTARGETCANCEL) )
				{
					if ( m_pChar->Skill_OnCharTrigger(action, CTRIG_SkillTargetCancel) == TRIGRET_RET_TRUE )
					{
						bSuppressCancelMessage = true;
						break;
					}
				}
				if ( IsTrigUsed(TRIGGER_TARGETCANCEL) )
				{
					if ( m_pChar->Skill_OnTrigger(action, SKTRIG_TARGETCANCEL) == TRIGRET_RET_TRUE )
						bSuppressCancelMessage = true;
				}
			}
		} break;

		default:
			break;
	}

	// determine timeout time
	if (iTimeout > 0)
		m_Targ_Timeout = CServTime::GetCurrentTime() + iTimeout;
	else
		m_Targ_Timeout.Init();

	if ( GetTargMode() == targmode )
		return;

	if ( GetTargMode() != CLIMODE_NORMAL && targmode != CLIMODE_NORMAL )
	{
		//If there's any item in LAYER_DRAGGING we remove it from view and then bounce it
		CItem * pItem = m_pChar->LayerFind( LAYER_DRAGGING );
		if (pItem != NULL)
		{
			pItem->RemoveFromView();		//Removing from view to avoid seeing it in the cursor
			m_pChar->ItemBounce(pItem);
			// Just clear the old target mode
			if (bSuppressCancelMessage == false)
			{
				addSysMessage(g_Cfg.GetDefaultMsg(DEFMSG_TARGET_CANCEL_2));
			}
		}
	}

	m_Targ_Mode = targmode;
	if ( targmode == CLIMODE_NORMAL && bSuppressCancelMessage == false )
		addSysMessage( g_Cfg.GetDefaultMsg(DEFMSG_TARGET_CANCEL_1) );
	else if ( pPrompt && *pPrompt ) // Check that the message is not blank.
		addSysMessage( pPrompt );
}

void CClient::addPromptConsole( CLIMODE_TYPE mode, LPCTSTR pPrompt, CGrayUID context1, CGrayUID context2, bool bUnicode )
{
	ADDTOCALLSTACK("CClient::addPromptConsole");

	m_Prompt_Uid = context1;
	m_Prompt_Mode = mode;

	if ( pPrompt && *pPrompt ) // Check that the message is not blank.
		addSysMessage( pPrompt );

	new PacketAddPrompt(this, context1, context2, bUnicode);
}

void CClient::addTarget( CLIMODE_TYPE targmode, LPCTSTR pPrompt, bool fAllowGround, bool fCheckCrime, int iTimeout ) // Send targetting cursor to client
{
	ADDTOCALLSTACK("CClient::addTarget");
	// Expect XCMD_Target back.
	// ??? will this be selective for us ? objects only or chars only ? not on the ground (statics) ?

	SetTargMode( targmode, pPrompt, iTimeout );

	new PacketAddTarget(this,
						fAllowGround? PacketAddTarget::Ground : PacketAddTarget::Object,
						targmode,
						fCheckCrime? PacketAddTarget::Harmful : PacketAddTarget::None);
}

void CClient::addTargetDeed( const CItem * pDeed )
{
	ADDTOCALLSTACK("CClient::addTargetDeed");
	// Place an item from a deed. preview all the stuff

	ASSERT( m_Targ_UID == pDeed->GetUID());
	ITEMID_TYPE iddef = static_cast<ITEMID_TYPE>(RES_GET_INDEX(pDeed->m_itDeed.m_Type));
	m_tmUseItem.m_pParent = pDeed->GetParent();	// Cheat Verify.
	addTargetItems( CLIMODE_TARG_USE_ITEM, iddef );
}

bool CClient::addTargetChars( CLIMODE_TYPE mode, CREID_TYPE baseID, bool fNotoCheck, int iTimeout )
{
	ADDTOCALLSTACK("CClient::addTargetChars");
	CCharBase * pBase = CCharBase::FindCharBase( baseID );
	if ( pBase == NULL )
		return( false );

	TCHAR * pszTemp = Str_GetTemp();
	sprintf(pszTemp, "%s '%s'?", g_Cfg.GetDefaultMsg(DEFMSG_WHERE_TO_SUMMON), static_cast<LPCTSTR>(pBase->GetTradeName()));

	addTarget(mode, pszTemp, true, fNotoCheck, iTimeout);
	return true;
}

bool CClient::addTargetItems( CLIMODE_TYPE targmode, ITEMID_TYPE id, bool fGround )
{
	ADDTOCALLSTACK("CClient::addTargetItems");
	// Add a list of items to place at target.
	// preview all the stuff

	ASSERT(m_pChar);

	LPCTSTR pszName;
	CItemBase * pItemDef;
	if ( id < ITEMID_TEMPLATE )
	{
		pItemDef = CItemBase::FindItemBase( id );
		if ( pItemDef == NULL )
			return false;
		pszName = pItemDef->GetName();

		if ( pItemDef->IsType(IT_STONE_GUILD) )
		{
			// Check if they are already in a guild first
			CItemStone * pStone = m_pChar->Guild_Find(MEMORY_GUILD);
			if (pStone)
			{
				addSysMessage( g_Cfg.GetDefaultMsg(DEFMSG_GUILD_ALREADY_MEMBER));
				return false;
			}
		}
	}
	else
	{
		pItemDef = NULL;
		pszName = "template";
	}

	TCHAR *pszTemp = Str_GetTemp();
	sprintf(pszTemp, "%s %s?", g_Cfg.GetDefaultMsg(DEFMSG_WHERE_TO_PLACE), pszName);

	if ( CItemBase::IsID_Multi( id ))	// a multi we get from Multi.mul
	{
		SetTargMode(targmode, pszTemp);

		new PacketAddTarget(this, fGround? PacketAddTarget::Ground : PacketAddTarget::Object, targmode, PacketAddTarget::None, id);
		return true;
	}

	// preview not supported by this ver?
	addTarget(targmode, pszTemp, true);
	return true;
}

void CClient::addTargetCancel()
{
	ADDTOCALLSTACK("CClient::addTargetCancel");

	// handle the cancellation now, in an ideal world the client would normally respond to
	// the cancel target packet as if the user had pressed ESC, but we shouldn't rely on
	// this happening (older clients for example don't support the cancel command and will
	// bring up a new target cursor)
	SetTargMode();

	// tell the client to cancel their cursor
	new PacketAddTarget(this, PacketAddTarget::Object, 0, PacketAddTarget::Cancel);
}

void CClient::addDyeOption( const CObjBase * pObj )
{
	ADDTOCALLSTACK("CClient::addDyeOption");
	// Put up the wHue chart for the client.
	// This results in a Event_Item_Dye message. CLIMODE_DYE

	new PacketShowDyeWindow(this, pObj);

	SetTargMode( CLIMODE_DYE );
}

void CClient::addSkillWindow(SKILL_TYPE skill, bool bFromInfo) // Opens the skills list
{
	ADDTOCALLSTACK("CClient::addSkillWindow");
	// Whos skills do we want to look at ?
	CChar* pChar = m_Prop_UID.CharFind();
	if (pChar == NULL)
		pChar = m_pChar;

	bool bAllSkills = (skill >= SKILL_MAX);
	if (bAllSkills == false && g_Cfg.m_SkillIndexDefs.IsValidIndex(skill) == false)
		return;

	if ( IsTrigUsed(TRIGGER_USERSKILLS) )
	{
		CScriptTriggerArgs Args(bAllSkills? -1 : skill, bFromInfo);
		if (m_pChar->OnTrigger(CTRIG_UserSkills, pChar, &Args) == TRIGRET_RET_TRUE)
			return;
	}

	if (bAllSkills == false && skill >= SKILL_SCRIPTED)
		return;

	new PacketSkills(this, pChar, skill);
}

void CClient::addAOSPlayerSeeNoCrypt()
{
	ADDTOCALLSTACK("CClient::addAOSPlayerSeeNoCrypt");
	// Adjust to my new location, what do I now see here?
	bool fAllShow = IsPriv(PRIV_ALLSHOW);
	bool fOsiSight = IsSetOF(OF_OSIMultiSight);
	CRegionBase * pCurrentCharRegion = m_pChar->GetTopPoint().GetRegion(REGION_TYPE_MULTI);

	//	Items on the ground
	CWorldSearch AreaItems(m_pChar->GetTopPoint(), UO_MAP_VIEW_SIZE);
	AreaItems.SetAllShow(fAllShow);
	AreaItems.SetSearchSquare(true);
	DWORD	dSeeItems = 0;

	for (;;)
	{
		CItem *pItem = AreaItems.GetItem();
		if ( !pItem )
			break;
		if ( !CanSee(pItem) )
			continue;
		if (fOsiSight)
		{
			if (( !pItem->GetTopLevelObj()->GetTopPoint().GetRegion(REGION_TYPE_MULTI) || ( pItem->GetKeyNum("ALWAYSSEND", true, true) ) || ( pItem->IsTypeMulti() ) || (( pItem->m_uidLink.IsValidUID() ) && ( pItem->m_uidLink.IsItem() ) && ( pItem->m_uidLink.ItemFind()->IsTypeMulti() ))
				|| ( pItem->GetTopLevelObj()->GetTopPoint().GetRegion(REGION_TYPE_MULTI) == pCurrentCharRegion )))
			{
				if ( dSeeItems < (g_Cfg.m_iMaxItemComplexity*30) )
				{
					++dSeeItems;
					addAOSTooltip(pItem);
				}
				else
					break;
			}
		}
		else
		{
			if ( dSeeItems < (g_Cfg.m_iMaxItemComplexity*30) )
			{
				++dSeeItems;
				addAOSTooltip(pItem);
			}
			else
				break;
		}
	}

	//	Characters around
	CWorldSearch AreaChars(m_pChar->GetTopPoint(), UO_MAP_VIEW_SIZE);
	AreaChars.SetAllShow(fAllShow);
	AreaChars.SetSearchSquare(true);
	DWORD	dSeeChars(0);
	for (;;)
	{
		CChar	*pChar = AreaChars.GetChar();
		if ( !pChar )
			break;
		if ( !CanSee(pChar) )
			continue;
		if ( dSeeChars < g_Cfg.m_iMaxCharComplexity*5 )
		{
			++dSeeChars;
			addAOSTooltip(pChar);
		}
		else
			break;
	}
}

void CClient::addPlayerSee( const CPointMap & ptold )
{
	ADDTOCALLSTACK("CClient::addPlayerSee");
	// Adjust to my new location, what do I now see here?
	bool fAllShow = IsPriv(PRIV_ALLSHOW);
	bool fOsiSight = IsSetOF(OF_OSIMultiSight);
	BYTE tViewDist = static_cast<unsigned char>(m_pChar->GetSight());
	CRegionBase * pCurrentCharRegion = m_pChar->GetTopPoint().GetRegion(REGION_TYPE_MULTI);

	//	Items on the ground
	CWorldSearch AreaItems(m_pChar->GetTopPoint(), UO_MAP_VIEW_RADAR);
	AreaItems.SetAllShow(fAllShow);
	AreaItems.SetSearchSquare(true);
	DWORD	dSeeItems = 0;

	if (GetNetState()->isClientVersion(MINCLIVER_HS) || GetNetState()->isClientSA())
	{
		for (;;)
		{
			CItem *pItem = AreaItems.GetItem();
			if (!pItem)
				break;
			if (!CanSee(pItem))
				continue;

			if (fOsiSight)
			{
				if ((ptold.GetDistSight(pItem->GetTopPoint()) > UO_MAP_VIEW_RADAR) && (pItem->IsTypeMulti()))  //Item is a Multi in Radar view
				{
					if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
					{
						CItemMulti * pMulti = dynamic_cast<CItemMulti*>(pItem);
						CObjBase * ppObjs[MAX_MULTI_CONTENT];
						DWORD	dMultiItems = 0;
						dMultiItems = pMulti->Multi_ListObjs(ppObjs);
						new PacketContainer(this, ppObjs, dMultiItems);
						addItem_OnGround(pItem);
					}
					else
						break;
				}
				if ((((m_pChar->GetTopPoint().GetDistSight(pItem->GetTopPoint()) <= tViewDist) && (ptold.GetDistSight(pItem->GetTopPoint()) > tViewDist)) //Item just came in to view
					&& ((pItem->GetKeyNum("ALWAYSSEND", true, true)) //Item has the alwayssend tag set to true
					|| (!pItem->GetTopLevelObj()->GetTopPoint().GetRegion(REGION_TYPE_HOUSE)) //Item is not in a house multi (Ships are ok)
					|| ((pItem->m_uidLink.IsValidUID()) && (pItem->m_uidLink.IsItem()) && (pItem->m_uidLink.ItemFind()->IsTypeMulti())) //Item is linked to a multi
					|| (pItem->IsTypeMulti()))) //Item is a multi
					|| (((ptold.GetRegion(REGION_TYPE_HOUSE) != pCurrentCharRegion) || (ptold.GetDistSight(pItem->GetTopPoint()) > tViewDist))
					&& (!pItem->IsTypeMulti()) && (pItem->GetTopLevelObj()->GetTopPoint().GetRegion(REGION_TYPE_HOUSE) == pCurrentCharRegion))) //Item is in same multi as me
				{
					if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
					{
						++dSeeItems;
						addItem_OnGround(pItem);
					}
					else
						break;
				}
			}
			else
			{
				if ((ptold.GetDistSight(pItem->GetTopPoint()) > UO_MAP_VIEW_RADAR) && (pItem->IsTypeMulti()))  //Item is a Multi in Radar view
				{
					if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
					{
						CItemMulti * pMulti = dynamic_cast<CItemMulti*>(pItem);
						CObjBase * ppObjs[MAX_MULTI_CONTENT];
						DWORD	dMultiItems = 0;
						dMultiItems = pMulti->Multi_ListObjs(ppObjs);
						new PacketContainer(this, ppObjs, dMultiItems);
						addItem_OnGround(pItem);
					}
					else
						break;
				}
				if ((m_pChar->GetTopPoint().GetDistSight(pItem->GetTopPoint()) <= tViewDist) && (ptold.GetDistSight(pItem->GetTopPoint()) > tViewDist)) //Item just came in to view
				{
					if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
					{
						++dSeeItems;
						addItem_OnGround(pItem);
					}
					else
						break;
				}
			}
		}
	}
	else
	{
		for (;;)
		{
			CItem *pItem = AreaItems.GetItem();
			if (!pItem)
				break;
			if (!CanSee(pItem))
				continue;

			if (fOsiSight)
			{
				if ((((m_pChar->GetTopPoint().GetDistSight(pItem->GetTopPoint()) <= tViewDist) && (ptold.GetDistSight(pItem->GetTopPoint()) > tViewDist)) //Item just came in to view
					&& ((pItem->GetKeyNum("ALWAYSSEND", true, true)) //Item has the alwayssend tag set to true
					|| (!pItem->GetTopLevelObj()->GetTopPoint().GetRegion(REGION_TYPE_HOUSE)) //Item is not in a house multi (Ships are ok)
					|| ((pItem->m_uidLink.IsValidUID()) && (pItem->m_uidLink.IsItem()) && (pItem->m_uidLink.ItemFind()->IsTypeMulti())) //Item is linked to a multi
					|| (pItem->IsTypeMulti()))) //Item is a multi
					|| (((ptold.GetRegion(REGION_TYPE_HOUSE) != pCurrentCharRegion) || (ptold.GetDistSight(pItem->GetTopPoint()) > tViewDist))
					&& (!pItem->IsTypeMulti()) && (pItem->GetTopLevelObj()->GetTopPoint().GetRegion(REGION_TYPE_HOUSE) == pCurrentCharRegion)) //Item is in same multi as me
					|| ((ptold.GetDistSight(pItem->GetTopPoint()) > UO_MAP_VIEW_RADAR) && (pItem->IsTypeMulti())))  //Item is a Multi in Radar view
				{
					if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
					{
						++dSeeItems;
						addItem_OnGround(pItem);
					}
					else
						break;
				}
			}
			else
			{
				if (((m_pChar->GetTopPoint().GetDistSight(pItem->GetTopPoint()) <= tViewDist) && (ptold.GetDistSight(pItem->GetTopPoint()) > tViewDist)) || ((ptold.GetDistSight(pItem->GetTopPoint()) > UO_MAP_VIEW_RADAR) && (pItem->IsTypeMulti())))
				{
					if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
					{
						++dSeeItems;
						addItem_OnGround(pItem);
					}
					else
						break;
				}
			}
		}
	}

	CWorldSearch AreaChars(m_pChar->GetTopPoint(), tViewDist);
	AreaChars.SetAllShow(fAllShow);
	AreaChars.SetSearchSquare(true);
	DWORD	dSeeChars = 0;

	for (;;)
	{
		CChar	*pChar = AreaChars.GetChar();
		if (!pChar)
			break;
		if ((m_pChar == pChar) || !CanSee(pChar))
			continue;

		if (ptold.GetDistSight(pChar->GetTopPoint()) > tViewDist)
		{
			if (dSeeChars < g_Cfg.m_iMaxCharComplexity * 5)
			{
				++dSeeChars;
				addChar(pChar);
			}
			else
				break;
		}
	}
}

void CClient::addPlayerSeeShip( const CPointMap & ptold )
{
	ADDTOCALLSTACK("CClient::addPlayerSee");
	// Adjust to my new location, what do I now see here?
	bool fAllShow = IsPriv(PRIV_ALLSHOW);
	bool fOsiSight = IsSetOF(OF_OSIMultiSight);
	BYTE tViewDist = static_cast<unsigned char>(m_pChar->GetSight());

	//	Items on the ground
	CWorldSearch AreaItems(m_pChar->GetTopPoint(), UO_MAP_VIEW_RADAR);
	AreaItems.SetAllShow(fAllShow);
	AreaItems.SetSearchSquare(true);
	DWORD	dSeeItems = 0;

	for (;;)
	{
		CItem *pItem = AreaItems.GetItem();
		if (!pItem)
			break;

		if (!CanSee(pItem))
			continue;

		if (fOsiSight)
		{
			if ((ptold.GetDistSight(pItem->GetTopPoint()) > UO_MAP_VIEW_RADAR) && (pItem->IsTypeMulti()))  //Item is a Multi in Radar view
			{
				if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
				{
					CItemMulti * pMulti = dynamic_cast<CItemMulti*>(pItem);
					CObjBase * ppObjs[MAX_MULTI_CONTENT];
					DWORD	dMultiItems = 0;
					dMultiItems = pMulti->Multi_ListObjs(ppObjs);
					new PacketContainer(this, ppObjs, dMultiItems);
					addItem_OnGround(pItem);
				}
				else
					break;
			}
			if (((m_pChar->GetTopPoint().GetDistSight(pItem->GetTopPoint()) <= tViewDist) && (ptold.GetDistSight(pItem->GetTopPoint()) > tViewDist)) //Item just came in to view
				&& ((pItem->GetKeyNum("ALWAYSSEND", true, true)) //Item has the alwayssend tag set to true
				|| (!pItem->GetTopLevelObj()->GetTopPoint().GetRegion(REGION_TYPE_HOUSE)) //Item is not in a house multi (Ships are ok)
				|| ((pItem->m_uidLink.IsValidUID()) && (pItem->m_uidLink.IsItem()) && (pItem->m_uidLink.ItemFind()->IsTypeMulti())) //Item is linked to a multi
				|| (pItem->IsTypeMulti()))) //Item is a multi
			{
				if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
				{
					++dSeeItems;
					addItem_OnGround(pItem);
				}
				else
					break;
			}
		}
		else
		{
			if ((ptold.GetDistSight(pItem->GetTopPoint()) > UO_MAP_VIEW_RADAR) && (pItem->IsTypeMulti()))
			{
				if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
				{
					CItemMulti * pMulti = dynamic_cast<CItemMulti*>(pItem);
					CObjBase * ppObjs[MAX_MULTI_CONTENT];
					DWORD	dMultiItems = 0;
					dMultiItems = pMulti->Multi_ListObjs(ppObjs);
					new PacketContainer(this, ppObjs, dMultiItems);
					addItem_OnGround(pItem);
				}
				else
					break;
			}
			if ((m_pChar->GetTopPoint().GetDistSight(pItem->GetTopPoint()) <= tViewDist) && (ptold.GetDistSight(pItem->GetTopPoint()) > tViewDist))
			{
				if (dSeeItems < g_Cfg.m_iMaxItemComplexity * 30)
				{
					++dSeeItems;
					addItem_OnGround(pItem);
				}
				else
					break;
			}
		}
	}

	CWorldSearch AreaChars(m_pChar->GetTopPoint(), tViewDist);
	AreaChars.SetAllShow(fAllShow);
	AreaChars.SetSearchSquare(true);
	DWORD	dSeeChars = 0;

	for (;;)
	{
		CChar	*pChar = AreaChars.GetChar();
		if (!pChar)
			break;
		if (m_pChar == pChar)
			continue;

		if (ptold.GetDistSight(pChar->GetTopPoint()) > tViewDist)
		{
			if (dSeeChars < g_Cfg.m_iMaxCharComplexity * 5)
			{
				++dSeeChars;
				addChar(pChar);
			}
			else
				break;
		}
	}
}

void CClient::addPlayerView( const CPointMap & pt, bool bFull )
{
	ADDTOCALLSTACK("CClient::addPlayerView");
	// I moved = Change my point of view. Teleport etc..
	// NotItems to not send or even do the checks, no client impact but server is not firing unnecessary loops and calls.

	new PacketPlayerPosition(this);

	if ( pt == m_pChar->GetTopPoint() )
		return;		// not a real move i guess. might just have been a change in face dir.

	m_Env.SetInvalid();	// Must resend environ stuff.

	if ( bFull )
		addPlayerSee(pt);
}

void CClient::addReSync()
{
	ADDTOCALLSTACK("CClient::addReSync");
	if ( m_pChar == NULL )
		return;
	// Reloads the client with all it needs.
	addMap();
	addChar(m_pChar);
	addPlayerView(NULL);
	addLight();		// Current light level where I am.
	addWeather();	// if any ...
	addSpeedMode(m_pChar->m_pPlayer->m_speedMode);
	addCharStatWindow(m_pChar->GetUID());
}

void CClient::addMap()
{
	ADDTOCALLSTACK("CClient::addMap");
	if ( m_pChar == NULL )
		return;

	CPointMap pt = m_pChar->GetTopPoint();
	new PacketMapChange(this, g_MapList.m_mapid[pt.m_map]);
}

void CClient::addMapDiff()
{
	ADDTOCALLSTACK("CClient::addMapDiff");
	// Enables map diff usage on the client. If the client is told to
	// enable diffs, and then logs back in without restarting, it will
	// continue to use the diffs even if not told to enable them - so
	// this packet should always be sent even if empty.

	new PacketEnableMapDiffs(this);
}

void CClient::addChangeServer()
{
	ADDTOCALLSTACK("CClient::addChangeServer");
	CPointMap pt = m_pChar->GetTopPoint();

	new PacketZoneChange(this, pt);
}

void CClient::UpdateStats()
{
	ADDTOCALLSTACK("CClient::UpdateStats");
	if ( !m_fUpdateStats || !m_pChar )
		return;

	if ( m_fUpdateStats & SF_UPDATE_STATUS )
	{
		addCharStatWindow( m_pChar->GetUID());
		m_fUpdateStats = 0;
	}
	else
	{
		if ( m_fUpdateStats & SF_UPDATE_HITS )
		{
			addHitsUpdate( m_pChar->GetUID() );
			m_fUpdateStats &= ~SF_UPDATE_HITS;
		}
		if ( m_fUpdateStats & SF_UPDATE_MANA )
		{
			addManaUpdate( m_pChar->GetUID() );
			m_fUpdateStats &= ~SF_UPDATE_MANA;
		}

		if ( m_fUpdateStats & SF_UPDATE_STAM )
		{
			addStamUpdate( m_pChar->GetUID() );
			m_fUpdateStats &= ~SF_UPDATE_STAM;
		}
	}
}

void CClient::addCharStatWindow( CGrayUID uid, bool fRequested ) // Opens the status window
{
	ADDTOCALLSTACK("CClient::addCharStatWindow");
	CChar * pChar = uid.CharFind();
	if ( !pChar )
		return;

	if ( IsTrigUsed(TRIGGER_USERSTATS) )
	{
		CScriptTriggerArgs	Args(0, 0, uid.ObjFind());
		Args.m_iN3 = fRequested;
		if ( m_pChar->OnTrigger(CTRIG_UserStats, pChar, &Args) == TRIGRET_RET_TRUE )
			return;
	}

	new PacketCharacterStatus(this, pChar);
	if ( pChar == m_pChar )
		m_fUpdateStats = 0;

	if ( (pChar == m_pChar) && (pChar->m_pPlayer != NULL) && (PacketStatLocks::CanSendTo(GetNetState())) )
		new PacketStatLocks(this, pChar);
}

void CClient::addHitsUpdate( CGrayUID uid )
{
	ADDTOCALLSTACK("CClient::addHitsUpdate");
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return;

	PacketHealthUpdate cmd(pChar, pChar == m_pChar);
	cmd.send(this);
}

void CClient::addManaUpdate( CGrayUID uid )
{
	ADDTOCALLSTACK("CClient::addManaUpdate");
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return;

	PacketManaUpdate cmd(pChar);
	cmd.send(this);

	if ( pChar->m_pParty )
		pChar->m_pParty->AddStatsUpdate( pChar, &cmd );
}

void CClient::addStamUpdate( CGrayUID uid )
{
	ADDTOCALLSTACK("CClient::addStamUpdate");
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return;

	PacketStaminaUpdate cmd(pChar);
	cmd.send(this);

	if ( pChar->m_pParty )
		pChar->m_pParty->AddStatsUpdate( pChar, &cmd );
}

void CClient::addHealthBarUpdate( const CChar * pChar )
{
	ADDTOCALLSTACK("CClient::addHealthBarUpdate");
	if ( pChar == NULL )
		return;

	if ( PacketHealthBarUpdate::CanSendTo(GetNetState()) )
		new PacketHealthBarUpdate(this, pChar);
}

void CClient::addBondedStatus( const CChar * pChar, bool bIsDead )
{
	ADDTOCALLSTACK("CClient::addBondedStatus");
	if ( pChar == NULL )
		return;

	new PacketBondedStatus(this, pChar, bIsDead);
}

void CClient::addSpellbookOpen( CItem * pBook, WORD offset )
{
	ADDTOCALLSTACK("CClient::addSpellbookOpen");

	if ( !m_pChar )
		return;

	if ( IsTrigUsed(TRIGGER_SPELLBOOK) )
	{
		CScriptTriggerArgs	Args( 0, 0, pBook );
		if ( m_pChar->OnTrigger( CTRIG_SpellBook, m_pChar, &Args ) == TRIGRET_RET_TRUE )
			return;
	}

	// NOTE: if the spellbook item is not present on the client it will crash.
	// count what spells I have.

	if ( pBook->GetDispID() == ITEMID_SPELLBOOK2 )
	{
		// weird client bug.
		pBook->SetDispID( ITEMID_SPELLBOOK );
		pBook->Update();
		return;
	}

	int count = pBook->GetSpellcountInBook();
	if ( count == -1 )
		return;

	OpenPacketTransaction transaction(this, PacketSend::PRI_NORMAL);
	addOpenGump( pBook, GUMP_OPEN_SPELLBOOK );

	//
	// New AOS spellbook packet required by client 4.0.0 and above.
	// Old packet is still required if both FEATURE_AOS_TOOLTIP and FEATURE_AOS_UPDATE aren't sent.
	//
	if ( PacketSpellbookContent::CanSendTo(GetNetState()) && GetNetState()->isClientVersion(MINCLIVER_SPELLBOOK) && IsAosFlagEnabled(FEATURE_AOS_UPDATE_B) )
	{
		// Handle new AOS spellbook stuff (old packets no longer work)
		new PacketSpellbookContent(this, pBook, offset);
		return;
	}

	if (count <= 0)
		return;

	new PacketItemContents(this, pBook);
}


void CClient::addCustomSpellbookOpen( CItem * pBook, DWORD gumpID )
{
	ADDTOCALLSTACK("CClient::addCustomSpellbookOpen");
	const CItemContainer * pContainer = dynamic_cast <CItemContainer *> (pBook);
	CItem	* pItem;
	if ( !pContainer )
		return;

	int count=0;
	for ( pItem=pContainer->GetContentHead(); pItem!=NULL; pItem=pItem->GetNext() )
	{
		if ( !pItem->IsType( IT_SCROLL ) )
			continue;
		count++;
	}

	OpenPacketTransaction transaction(this, PacketSend::PRI_NORMAL);
	addOpenGump( pBook, static_cast<GUMP_TYPE>(gumpID));
	if (count <= 0)
		return;

	new PacketItemContents(this, pContainer);
}

void CClient::addScrollScript( CResourceLock &s, SCROLL_TYPE type, DWORD context, LPCTSTR pszHeader )
{
	ADDTOCALLSTACK("CClient::addScrollScript");

	new PacketOpenScroll(this, s, type, context, pszHeader);
}

void CClient::addScrollResource( LPCTSTR pszSec, SCROLL_TYPE type, DWORD scrollID )
{
	ADDTOCALLSTACK("CClient::addScrollResource");
	//
	// type = 0 = TIPS
	// type = 2 = UPDATES

	CResourceLock s;
	if ( ! g_Cfg.ResourceLock( s, RES_SCROLL, pszSec ))
		return;
	addScrollScript( s, type, scrollID );
}

void CClient::addVendorClose( const CChar * pVendor )
{
	ADDTOCALLSTACK("CClient::addVendorClose");
	// Clear the vendor display.

	new PacketCloseVendor(this, pVendor);
}

int CClient::addShopItems(CChar * pVendor, LAYER_TYPE layer, bool bReal)
{
	ADDTOCALLSTACK("CClient::addShopItems");
	// Player buying from vendor.
	// Show the Buy menu for the contents of this container
	// RETURN: the number of items in container.
	//   < 0 = error.
	CItemContainer * pContainer = pVendor->GetBank( layer );
	if ( pContainer == NULL )
		return( -1 );

	addItem(pContainer);
	int count = 0;
	if ( bReal )
	{
		addContents(pContainer, false, false, true, false);
		for (const CItem* item = pContainer->GetContentTail(); item != NULL && count < MAX_ITEMS_CONT; item = item->GetPrev())
		{
			addAOSTooltip(item, false, true);
			count++;
		}
		if (GetNetState()->isClientSA())
		{
			addContents(pContainer, false, false, false, true);
			PacketVendorBuyList* cmd = new PacketVendorBuyList();
			count = cmd->fillContainer(pContainer, pVendor->NPC_GetVendorMarkup(m_pChar), count);
			cmd->push(this);
		}
	}

	if (!GetNetState()->isClientSA())
	{
		int iConvertFactor = pVendor->NPC_GetVendorMarkup(m_pChar );
		PacketVendorBuyList* cmd = new PacketVendorBuyList();
		count = cmd->fillContainer(pContainer, iConvertFactor, bReal? MAX_ITEMS_CONT : 0);
		cmd->push(this);
	}

	// Send a warning if the vendor somehow has more stock than the allowed limit
	if ( pContainer->GetCount() > MAX_ITEMS_CONT )
		g_Log.Event( LOGL_WARN, "Vendor 0%lx '%s' has exceeded their stock limit! (%d/%d items)\n", static_cast<DWORD>(pVendor->GetUID()), static_cast<LPCTSTR>(pVendor->GetName()), pContainer->GetCount(), MAX_ITEMS_CONT);

	return count;
}

bool CClient::addShopMenuBuy( CChar * pVendor )
{
	ADDTOCALLSTACK("CClient::addShopMenuBuy");
	// Try to buy stuff that the vendor has.
	if ( !pVendor || !pVendor->NPC_IsVendor() )
		return false;

	OpenPacketTransaction transaction(this, PacketSend::PRI_HIGH);

	//	non-player vendors could be restocked on-the-fly
	if ( !pVendor->IsStatFlag(STATF_Pet) )
	{
		pVendor->NPC_Vendor_Restock(false, true);
	}

	addChar(pVendor);

	int iRes = addShopItems(pVendor, LAYER_VENDOR_STOCK);
	if ( iRes < 0 )
		return false;

	//	classic clients will crash without extra packets,
	//	let's provide some empty packets specialy for them
	addShopItems(pVendor, LAYER_VENDOR_EXTRA, false);

	addOpenGump( pVendor, GUMP_VENDOR_RECT, true );
	addCharStatWindow( m_pChar->GetUID());	// Make sure the gold total has been updated.
	
	return( true );
}

bool CClient::addShopMenuSell( CChar * pVendor )
{
	ADDTOCALLSTACK("CClient::addShopMenuSell");
	// Player selling to vendor.
	// What things do you have in your inventory that the vendor would want ?
	// Should end with a returned Event_VendorSell()

	if ( !pVendor || !pVendor->NPC_IsVendor() )
		return false;

	OpenPacketTransaction transaction(this, PacketSend::PRI_LOW);

	//	non-player vendors could be restocked on-the-fly
	if ( !pVendor->IsStatFlag(STATF_Pet) )
	{
		pVendor->NPC_Vendor_Restock(false, true);
	}

	int iConvertFactor		= - pVendor->NPC_GetVendorMarkup( m_pChar );

	CItemContainer * pContainer1 = pVendor->GetBank( LAYER_VENDOR_BUYS );
	addItem( pContainer1 );
	CItemContainer * pContainer2 = pVendor->GetBank( LAYER_VENDOR_STOCK );
	addItem( pContainer2 );

	//	classic clients will crash without extra packets,
	//	let's provide some empty packets specialy for them
	CItemContainer * pContainer3 = pVendor->GetBank( LAYER_VENDOR_EXTRA );
	addItem( pContainer3 );

	if ( pVendor->IsStatFlag( STATF_Pet ))	// Player vendor.
	{
		pContainer2 = NULL; // no stock
	}

	PacketVendorSellList cmd(pVendor);
	size_t count = cmd.searchContainer(this, m_pChar->GetPackSafe(), pContainer1, pContainer2, iConvertFactor);
	if (count <= 0)
		return false;
	
	cmd.send(this);
	return true;
}

void CClient::addBankOpen( CChar * pChar, LAYER_TYPE layer )
{
	ADDTOCALLSTACK("CClient::addBankOpen");
	// open it up for this pChar.
	ASSERT( pChar );

	CItemContainer * pBankBox = pChar->GetBank(layer);
	ASSERT(pBankBox);
	addItem( pBankBox );	// may crash client if we dont do this.

	if ( pChar != GetChar())
	{
		// xbank verb on others needs this ?
		// addChar( pChar );
	}

	pBankBox->OnOpenEvent( m_pChar, pChar );
	addContainerSetup( pBankBox );
}

void CClient::addDrawMap( CItemMap * pMap )
{
	ADDTOCALLSTACK("CClient::addDrawMap");
	// Make player drawn maps possible. (m_map_type=0) ???

	if ( pMap == NULL )
	{
blank_map:
		addSysMessage( g_Cfg.GetDefaultMsg(DEFMSG_MAP_IS_BLANK) );
		return;
	}
	if ( pMap->IsType(IT_MAP_BLANK))
		goto blank_map;

	CRectMap rect;
	rect.SetRect( pMap->m_itMap.m_left,
		pMap->m_itMap.m_top,
		pMap->m_itMap.m_right,
		pMap->m_itMap.m_bottom,
		g_MapList.m_mapid[pMap->m_itMap.m_map]);

	if ( ! rect.IsValid())
		goto blank_map;
	if ( rect.IsRectEmpty())
		goto blank_map;

	if ( PacketDisplayMapNew::CanSendTo(GetNetState()))
		new PacketDisplayMapNew(this, pMap, rect);
	else
		new PacketDisplayMap(this, pMap, rect);

	addMapMode( pMap, MAP_UNSENT, false );

	// Now show all the pins
	PacketMapPlot plot(pMap, MAP_ADD, false);
	for ( size_t i = 0; i < pMap->m_Pins.GetCount(); i++ )
	{
		plot.setPin(pMap->m_Pins[i].m_x, pMap->m_Pins[i].m_y);
		plot.send(this);
	}
}

void CClient::addMapMode( CItemMap * pMap, MAPCMD_TYPE iType, bool fEdit )
{
	ADDTOCALLSTACK("CClient::addMapMode");
	// NOTE: MAPMODE_* depends on who is looking. Multi clients could interfere with each other ?
	if ( !pMap )
		return;

	pMap->m_fPlotMode = fEdit;

	new PacketMapPlot(this, pMap, iType, fEdit);
}

void CClient::addBulletinBoard( const CItemContainer * pBoard )
{
	ADDTOCALLSTACK("CClient::addBulletinBoard");
	// Open up the bulletin board and all it's messages
	// PacketBulletinBoardReq::onReceive
	if (pBoard == NULL)
		return;

	// Give the bboard name.
	new PacketBulletinBoard(this, pBoard);

	// Send Content messages for all the items on the bboard.
	// Not sure what x,y are here, date/time maybe ?
	addContents( pBoard, false, false, false );

	// The client will now ask for the headers it wants.
}

bool CClient::addBBoardMessage( const CItemContainer * pBoard, BBOARDF_TYPE flag, CGrayUID uidMsg )
{
	ADDTOCALLSTACK("CClient::addBBoardMessage");
	ASSERT(pBoard);

	CItemMessage* pMsgItem = dynamic_cast<CItemMessage *>(uidMsg.ItemFind());
	if (pBoard->IsItemInside( pMsgItem ) == false)
		return( false );

	// check author is properly linked
	if (pMsgItem->m_sAuthor.IsEmpty() == false && pMsgItem->m_uidLink.CharFind() == NULL)
	{
		pMsgItem->Delete();
		return( false );
	}

	// Send back the message header and/or body.
	new PacketBulletinBoard(this, flag, pBoard, pMsgItem);
	return( true );
}

void CClient::addLoginComplete()
{
	ADDTOCALLSTACK("CClient::addLoginComplete");
	new PacketLoginComplete(this);
}

void CClient::addChatSystemMessage( CHATMSG_TYPE iType, LPCTSTR pszName1, LPCTSTR pszName2, CLanguageID lang )
{
	ADDTOCALLSTACK("CClient::addChatSystemMessage");

	new PacketChatMessage(this, iType, pszName1, pszName2, lang);
}

void CClient::addGumpTextDisp( const CObjBase * pObj, GUMP_TYPE gump, LPCTSTR pszName, LPCTSTR pszText )
{
	ADDTOCALLSTACK("CClient::addGumpTextDisp");
	// ??? how do we control where exactly the text goes ??

	new PacketSignGump(this, pObj, gump, pszName, pszText);
}

void CClient::addItemMenu( CLIMODE_TYPE mode, const CMenuItem * item, size_t count, CObjBase * pObj )
{
	ADDTOCALLSTACK("CClient::addItemMenu");
	// We must set GetTargMode() to show what mode we are in for menu select.
	// Will result in PacketMenuChoice::onReceive()
	// cmd.ItemMenu.

	if (count <= 0)
		return;

	if (pObj == NULL)
		pObj = m_pChar;

	new PacketDisplayMenu(this, mode, item, count, pObj);

	m_tmMenu.m_UID = pObj->GetUID();
	SetTargMode( mode );
}


bool CClient::addWalkCode( EXTDATA_TYPE iType, size_t iCodes )
{
	ADDTOCALLSTACK("CClient::addWalkCode");
	// Fill up the walk code buffer.
	// RETURN: true = new codes where sent.

	if ( ! m_Crypt.IsInit() )	// This is not even a game client ! IsConnectTypePacket()
		return false;

	if ( GetNetState()->isClientLessVersion(MINCLIVER_CHECKWALKCODE) )
		return false;

	if ( ! ( g_Cfg.m_wDebugFlags & DEBUGF_WALKCODES ))
		return( false );

	if ( iType == EXTDATA_WalkCode_Add )
	{
		if ( m_Walk_InvalidEchos != UINT_MAX )
			return false;					// they are stuck til they give a valid echo!
		// On a timer tick call this.
		if ( m_Walk_CodeQty >= COUNTOF(m_Walk_LIFO))	// They are appearently not moving fast.
			return false;
	}
	else
	{
		// Fill the buffer at start.
		ASSERT( m_Walk_CodeQty == UINT_MAX );
		m_Walk_CodeQty = 0;
	}

	ASSERT( iCodes <= COUNTOF(m_Walk_LIFO));

	// make a new code and send it out
	size_t i = 0;
	for ( ; i < iCodes && m_Walk_CodeQty < COUNTOF(m_Walk_LIFO); m_Walk_CodeQty++, i++ )
		m_Walk_LIFO[m_Walk_CodeQty] = 0x88ca0000 + Calc_GetRandVal(0xffff);

	new PacketFastWalk(this, m_Walk_LIFO, m_Walk_CodeQty, i);
	return( true );
}

void CClient::addCharPaperdoll( CChar * pChar )
{
	ADDTOCALLSTACK("CClient::addCharPaperdoll");
	if ( !pChar )
		return;

	new PacketPaperdoll(this, pChar);
}

void CClient::addAOSTooltip( const CObjBase * pObj, bool bRequested, bool bShop )
{
	ADDTOCALLSTACK("CClient::addAOSTooltip");
	if ( !pObj )
		return;

	if ( PacketPropertyList::CanSendTo(GetNetState()) == false )
		return;

	bool bNameOnly = false;
	if (!IsResClient(RDS_AOS) || !IsAosFlagEnabled(FEATURE_AOS_UPDATE_B))
	{
		if ( !bShop )
			return;

		// shop items use tooltips whether they're disabled or not,
		// so we can just send a basic tooltip with the item name
		bNameOnly = true;
	}

	//DEBUG_MSG(("(( m_pChar->GetTopPoint().GetDistSight(pObj->GetTopPoint()) (%x) > UO_MAP_VIEW_SIZE (%x) ) && ( !bShop ) (%x) )", m_pChar->GetTopPoint().GetDistSight(pObj->GetTopPoint()), UO_MAP_VIEW_SIZE, ( !bShop )));
	if (( m_pChar->GetTopPoint().GetDistSight(pObj->GetTopPoint()) > UO_MAP_VIEW_SIZE ) && ( m_pChar->GetTopPoint().GetDistSight(pObj->GetTopPoint()) <= UO_MAP_VIEW_RADAR ) && ( !bShop ) ) //we do not need to send tooltips for items not in LOS (multis/ships)
		return;

	// We check here if we are sending a tooltip for a static/non-movable items
	// (client doesn't expect us to) but only in the world
	if ( pObj->IsItem() )
	{
		const CItem * pItem = dynamic_cast<const CItem *>( pObj );

		if ( !pItem->GetContainer() && pItem->IsAttr(/*ATTR_MOVE_NEVER|*/ATTR_STATIC) )
		{
			if ( ( ! this->GetChar()->IsPriv( PRIV_GM ) ) && ( ! this->GetChar()->IsPriv( PRIV_ALLMOVE ) ) )
				return;
		}
	}

	PacketPropertyList* propertyList = pObj->GetPropertyList();

	if (propertyList == NULL || propertyList->hasExpired(g_Cfg.m_iTooltipCache))
	{
		CItem * pItem = ( pObj->IsItem() ? const_cast<CItem *>(dynamic_cast<const CItem *>(pObj)) : NULL );
		CChar * pChar = ( pObj->IsChar() ? const_cast<CChar *>(dynamic_cast<const CChar *>(pObj)) : NULL );

		if (pItem != NULL)
			pItem->FreePropertyList();
		else if (pChar != NULL)
			pChar->FreePropertyList();

		CClientTooltip* t = NULL;
		this->m_TooltipData.Clean(true);

		//DEBUG_MSG(("Preparing tooltip for 0%lx (%s)\n", (DWORD)pObj->GetUID(), pObj->GetName()));

		if (bNameOnly) // if we only want to display the name (FEATURE_AOS_UPDATE_B disabled)
		{
			unsigned long ClilocName = static_cast<unsigned long>(pObj->GetDefNum("NAMELOC", false, true));

			if (ClilocName)
				m_TooltipData.InsertAt(0, new CClientTooltip(ClilocName));
			else
			{
				m_TooltipData.InsertAt(0, t = new CClientTooltip(1050045));
				t->FormatArgs(" \t%s\t ", pObj->GetName());
			}
		}
		else // we have FEATURE_AOS_UPDATE_B enabled
		{
			TRIGRET_TYPE iRet = TRIGRET_RET_FALSE;

			if (( IsTrigUsed(TRIGGER_CLIENTTOOLTIP) ) || (( IsTrigUsed(TRIGGER_ITEMCLIENTTOOLTIP) ) && ( pItem )) || (( IsTrigUsed(TRIGGER_CHARCLIENTTOOLTIP) ) && ( pChar )))
			{
				CScriptTriggerArgs args(const_cast<CObjBase *>(pObj));
				args.m_iN1 = bRequested;
				iRet = const_cast<CObjBase *>(pObj)->OnTrigger("@ClientTooltip", this->GetChar(), &args); //ITRIG_CLIENTTOOLTIP , CTRIG_ClientTooltip
			}

			if ( iRet != TRIGRET_RET_TRUE )
			{
				unsigned long ClilocName = static_cast<unsigned long>(pObj->GetDefNum("NAMELOC", false, true));

				if ( pItem )
				{
					if (ClilocName)
						m_TooltipData.InsertAt(0, new CClientTooltip(ClilocName));
					else
					{
						m_TooltipData.InsertAt(0, t = new CClientTooltip(1050045));
						if ( ( pItem->GetAmount() != 1 ) && ( pItem->GetType() != IT_CORPSE ) )
							t->FormatArgs("%d \t%s\t ", pItem->GetAmount(), pObj->GetName()); // ~1_PREFIX~~2_NAME~~3_SUFFIX~
						else
							t->FormatArgs(" \t%s\t ", pObj->GetName()); // ~1_PREFIX~~2_NAME~~3_SUFFIX~
					}
				}
				else if ( pChar )
				{
					LPCTSTR lpPrefix = pChar->GetKeyStr("NAME.PREFIX");
					// HUE_TYPE wHue = m_pChar->Noto_GetHue( pChar, true );

					if ( ! *lpPrefix )
						lpPrefix = pChar->Noto_GetFameTitle();

					if ( ! *lpPrefix )
						lpPrefix = " ";

					TCHAR * lpSuffix = Str_GetTemp();
					strcpy(lpSuffix, pChar->GetKeyStr("NAME.SUFFIX"));

					const CStoneMember * pGuildMember = pChar->Guild_FindMember(MEMORY_GUILD);
					if ( pGuildMember != NULL &&
						(pChar->IsStatFlag(STATF_Incognito) == false || GetPrivLevel() > pChar->GetPrivLevel()) )
					{
						const CItemStone * pParentStone = pGuildMember->GetParentStone();
						ASSERT(pParentStone != NULL);

						if ( pGuildMember->IsAbbrevOn() && pParentStone->GetAbbrev()[0] )
						{
							strcat(lpSuffix, " [");
							strcat(lpSuffix, pParentStone->GetAbbrev());
							strcat(lpSuffix, "]");
						}
					}

					if ( *lpSuffix == '\0' )
						strcpy( lpSuffix, " " );

					// The name
					if (ClilocName)
					{
						m_TooltipData.InsertAt(0, t = new CClientTooltip(1116690));
						t->FormatArgs("%s\t#%lu\t%s", lpPrefix, ClilocName, lpSuffix);
					}
					else
					{
						m_TooltipData.InsertAt(0, t = new CClientTooltip(1050045));
						t->FormatArgs("%s\t%s\t%s", lpPrefix, pObj->GetName(), lpSuffix); // ~1_PREFIX~~2_NAME~~3_SUFFIX~
					}

					// Need to find a way to get the ushort inside hues.mul for index wHue to get this working.
					// t->FormatArgs("<basefont color=\"#%02x%02x%02x\">%s\t%s\t%s</basefont>",
					//	(BYTE)((((int)wHue) & 0x7C00) >> 7), (BYTE)((((int)wHue) & 0x3E0) >> 2),
					//	(BYTE)((((int)wHue) & 0x1F) << 3),lpPrefix, pObj->GetName(), lpSuffix); // ~1_PREFIX~~2_NAME~~3_SUFFIX~

					if ( !pChar->IsStatFlag(STATF_Incognito) || ( GetPrivLevel() > pChar->GetPrivLevel() ))
					{
						if ( pGuildMember && pGuildMember->IsAbbrevOn() )
						{
							if ( pGuildMember->GetTitle()[0] )
							{
								this->m_TooltipData.Add(t = new CClientTooltip(1060776));
								t->FormatArgs( "%s\t%s", pGuildMember->GetTitle(), pGuildMember->GetParentStone()->GetName()); // ~1_val~, ~2_val~
							}
							else
							{
								this->m_TooltipData.Add(new CClientTooltip(1070722, pGuildMember->GetParentStone()->GetName())); // ~1_NOTHING~
							}
						}
					}
				}


				// Some default tooltip info if RETURN 0 or no script
				if ( pChar )
				{
					// Character specific stuff
					if ( ( pChar->IsPriv( PRIV_GM ) ) && ( ! pChar->IsPriv( PRIV_PRIV_NOSHOW ) ) )
						this->m_TooltipData.Add( new CClientTooltip( 1018085 ) ); // Game Master
				}

				if ( pItem )
				{
					if ( pItem->IsAttr( ATTR_LOCKEDDOWN ) )
						this->m_TooltipData.Add( new CClientTooltip( 501643 ) ); // Locked Down
					if ( pItem->IsAttr( ATTR_SECURE ) )
						this->m_TooltipData.Add( new CClientTooltip( 501644 ) ); // Locked Down & Secured
					if ( pItem->IsAttr( ATTR_BLESSED ) )
						this->m_TooltipData.Add( new CClientTooltip( 1038021 ) ); // Blessed
					if ( pItem->IsAttr( ATTR_CURSED ) )
						this->m_TooltipData.Add( new CClientTooltip( 1049643 ) ); // Cursed
					if ( pItem->IsAttr( ATTR_INSURED ) )
						this->m_TooltipData.Add( new CClientTooltip( 1061682 ) ); // <b>Insured</b>
					if ( pItem->IsAttr( ATTR_QUESTITEM ) )
						this->m_TooltipData.Add( new CClientTooltip( 1072351 ) ); // Quest Item
					if ( pItem->IsAttr( ATTR_MAGIC ) )
						this->m_TooltipData.Add( new CClientTooltip( 3010064 ) ); // Magic
					if ( pItem->IsAttr( ATTR_NEWBIE ) )
						this->m_TooltipData.Add( new CClientTooltip( 1070722, g_Cfg.GetDefaultMsg(DEFMSG_TOOLTIP_TAG_NEWBIE) ) ); // ~1_NOTHING~

					if ( g_Cfg.m_iFeatureML & FEATURE_ML_UPDATE )
					{
						if ( pItem->IsMovable() )
						{
							INT64 Weight = pItem->GetWeight() / WEIGHT_UNITS;
							this->m_TooltipData.Add( t = new CClientTooltip( Weight == 1 ? 1072788 : 1072789 ) ); // Weight: ~1_WEIGHT~ stone / Weight: ~1_WEIGHT~ stones
							t->FormatArgs( "%lld", Weight );
						}
					}

					CGrayUID uid( static_cast<unsigned long>(pItem->GetDefNum("CRAFTEDBY", true)) );
					CChar * pCraftsman = uid.CharFind();
					if ( pCraftsman )
					{
						this->m_TooltipData.Add( t = new CClientTooltip( 1050043 ) ); // crafted by ~1_NAME~
						t->FormatArgs( "%s", pCraftsman->GetName() );
					}

					if ( pItem->IsAttr( ATTR_EXCEPTIONAL ))
						this->m_TooltipData.Add( new CClientTooltip( 1060636 ) ); // exceptional

					INT64 ArtifactRarity = pItem->GetDefNum("RARITY", true, true);
					if ( ArtifactRarity > 0 )
					{
						this->m_TooltipData.Add( t = new CClientTooltip( 1061078 ) ); // artifact rarity ~1_val~
						t->FormatArgs( "%lld", ArtifactRarity );
					}

					INT64 UsesRemaining = pItem->GetDefNum("USESCUR", true, true);
					if ( UsesRemaining > 0 )
					{
						this->m_TooltipData.Add( t = new CClientTooltip( 1060584 ) ); // uses remaining: ~1_val~
						t->FormatArgs( "%lld", UsesRemaining );
					}

					if ( pItem->IsTypeArmorWeapon())
					{
						if ( pItem->GetDefNum("BALANCED", true, true))
							this->m_TooltipData.Add( new CClientTooltip( 1072792 ) ); // balanced

						INT64 DamageIncrease = pItem->GetDefNum("INCREASEDAM", true, true);
						if ( DamageIncrease != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060401 ) ); // damage increase ~1_val~%
							t->FormatArgs( "%lld", DamageIncrease );
						}

						INT64 DefenceChanceIncrease = pItem->GetDefNum("INCREASEDEFCHANCE", true, true);
						if ( DefenceChanceIncrease != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060408 ) ); // defense chance increase ~1_val~%
							t->FormatArgs( "%lld", DefenceChanceIncrease );
						}

						INT64 DexterityBonus = pItem->GetDefNum("BONUSDEX", true, true);
						if ( DexterityBonus != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060409 ) ); // dexterity bonus ~1_val~
							t->FormatArgs( "%lld", DexterityBonus );
						}

						INT64 EnhancePotions = pItem->GetDefNum("ENHANCEPOTIONS", true, true);
						if ( EnhancePotions != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060411 ) ); // enhance potions ~1_val~%
							t->FormatArgs( "%lld", EnhancePotions );
						}

						INT64 FasterCastRecovery = pItem->GetDefNum("FASTERCASTRECOVERY", true, true);
						if ( FasterCastRecovery != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060412 ) ); // faster cast recovery ~1_val~
							t->FormatArgs( "%lld", FasterCastRecovery );
						}

						INT64 FasterCasting = pItem->GetDefNum("FASTERCASTING", true, true);
						if ( FasterCasting != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060413 ) ); // faster casting ~1_val~
							t->FormatArgs( "%lld", FasterCasting );
						}

						INT64 HitChanceIncrease = pItem->GetDefNum("INCREASEHITCHANCE", true, true);
						if ( HitChanceIncrease != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060415 ) ); // hit chance increase ~1_val~%
							t->FormatArgs( "%lld", HitChanceIncrease );
						}

						INT64 HitPointIncrease = pItem->GetDefNum("BONUSHITS", true, true);
						if ( HitPointIncrease != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060431 ) ); // hit point increase ~1_val~%
							t->FormatArgs( "%lld", HitPointIncrease );
						}

						INT64 IntelligenceBonus = pItem->GetDefNum("BONUSINT", true, true);
						if ( IntelligenceBonus != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060432 ) ); // intelligence bonus ~1_val~
							t->FormatArgs( "%lld", IntelligenceBonus );
						}

						INT64 LowerManaCost = pItem->GetDefNum("LOWERMANACOST", true, true);
						if ( LowerManaCost != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060433 ) ); // lower mana cost ~1_val~%
							t->FormatArgs( "%lld", LowerManaCost );
						}

						INT64 LowerReagentCost = pItem->GetDefNum("LOWERREAGENTCOST", true, true);
						if ( LowerReagentCost != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060434 ) ); // lower reagent cost ~1_val~%
							t->FormatArgs( "%lld", LowerReagentCost );
						}

						INT64 LowerRequirements = pItem->GetDefNum("LOWERREQ", true, true);
						if ( LowerRequirements != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060435 ) ); // lower requirements ~1_val~%
							t->FormatArgs( "%lld", LowerRequirements );
						}

						INT64 Luck = pItem->GetDefNum("LUCK", true, true);
						if ( Luck != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060436 ) ); // luck ~1_val~
							t->FormatArgs( "%lld", Luck );
						}

						if ( pItem->GetDefNum("MAGEARMOR", true, true))
							this->m_TooltipData.Add( new CClientTooltip( 1060437 ) ); // mage armor

						INT64 MageWeapon = pItem->GetDefNum("MAGEWEAPON", true, true);
						if ( MageWeapon != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060438 ) ); // mage weapon -~1_val~ skill
							t->FormatArgs( "%lld", MageWeapon );
						}

						INT64 ManaIncrease = pItem->GetDefNum("BONUSMANA", true, true);
						if ( ManaIncrease != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060439 ) ); // mana increase ~1_val~
							t->FormatArgs( "%lld", ManaIncrease );
						}

						INT64 ManaRegeneration = pItem->GetDefNum("REGENMANA", true, true);
						if ( ManaRegeneration != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060440 ) ); // mana regeneration ~1_val~
							t->FormatArgs( "%lld", ManaRegeneration );
						}

						if ( pItem->GetDefNum("NIGHTSIGHT", true, true))
							this->m_TooltipData.Add( new CClientTooltip( 1060441 ) ); // night sight

						INT64 ReflectPhysicalDamage = pItem->GetDefNum("REFLECTPHYSICALDAM", true, true);
						if ( ReflectPhysicalDamage != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060442 ) ); // reflect physical damage ~1_val~%
							t->FormatArgs( "%lld", ReflectPhysicalDamage );
						}

						INT64 StaminaRegeneration = pItem->GetDefNum("REGENSTAM", true, true);
						if ( StaminaRegeneration != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060443 ) ); // stamina regeneration ~1_val~
							t->FormatArgs( "%lld", StaminaRegeneration );
						}

						INT64 HitPointRegeneration = pItem->GetDefNum("REGENHITS", true, true);
						if ( HitPointRegeneration != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060444 ) ); // hit point regeneration ~1_val~
							t->FormatArgs( "%lld", HitPointRegeneration );
						}

						INT64 SelfRepair = pItem->GetDefNum("SELFREPAIR", true, true);
						if ( SelfRepair != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060450 ) ); // self repair ~1_val~
							t->FormatArgs( "%lld", SelfRepair );
						}

						if ( pItem->GetDefNum("SPELLCHANNELING", true, true))
							this->m_TooltipData.Add( new CClientTooltip( 1060482 ) ); // spell channeling

						INT64 SpellDamageIncrease = pItem->GetDefNum("INCREASESPELLDAM", true, true);
						if ( SpellDamageIncrease != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060483 ) ); // spell damage increase ~1_val~%
							t->FormatArgs( "%lld", SpellDamageIncrease );
						}

						INT64 StaminaIncrease = pItem->GetDefNum("BONUSSTAM", true, true);
						if ( StaminaIncrease != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060484 ) ); // stamina increase ~1_val~
							t->FormatArgs( "%lld", StaminaIncrease );
						}

						INT64 StrengthBonus = pItem->GetDefNum("BONUSSTR", true, true);
						if ( StrengthBonus != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060485 ) ); // strength bonus ~1_val~
							t->FormatArgs( "%lld", StrengthBonus );
						}

						INT64 SwingSpeedIncrease = pItem->GetDefNum("INCREASESWINGSPEED", true, true);
						if ( SwingSpeedIncrease != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1060486 ) ); // swing speed increase ~1_val~%
							t->FormatArgs( "%lld", SwingSpeedIncrease );
						}

						INT64 IncreasedKarmaLoss = pItem->GetDefNum("INCREASEKARMALOSS", true, true);
						if ( IncreasedKarmaLoss != 0 )
						{
							this->m_TooltipData.Add( t = new CClientTooltip( 1075210 ) ); // increased karma loss ~1val~%
							t->FormatArgs( "%lld", IncreasedKarmaLoss );
						}
					}

					// Some type specific default stuff
					switch ( pItem->GetType() )
					{
						case IT_CONTAINER_LOCKED:
							this->m_TooltipData.Add( new CClientTooltip( 3005142 ) ); // Locked
						case IT_CONTAINER:
						case IT_TRASH_CAN:
							if ( pItem->IsContainer() )
							{
								const CContainer * pContainer = dynamic_cast <const CContainer *> ( pItem );
								this->m_TooltipData.Add( t = new CClientTooltip( 1050044 ) );
								t->FormatArgs( "%" FMTSIZE_T "\t%d", pContainer->GetCount(), pContainer->GetTotalWeight() / WEIGHT_UNITS ); // ~1_COUNT~ items, ~2_WEIGHT~ stones
							}
							break;

						case IT_ARMOR_LEATHER:
						case IT_ARMOR:
						case IT_CLOTHING:
						case IT_SHIELD:
							{
								if (IsSetCombatFlags(COMBAT_ELEMENTAL_ENGINE))
								{
									INT64 PhysicalResist = pItem->GetDefNum("RESPHYSICAL", true, true);
									if (PhysicalResist != 0)
									{
										this->m_TooltipData.Add(t = new CClientTooltip(1060448)); // physical resist ~1_val~%
										t->FormatArgs("%lld", PhysicalResist);
									}

									INT64 FireResist = pItem->GetDefNum("RESFIRE", true, true);
									if (FireResist != 0)
									{
										this->m_TooltipData.Add(t = new CClientTooltip(1060447)); // fire resist ~1_val~%
										t->FormatArgs("%lld", FireResist);
									}

									INT64 ColdResist = pItem->GetDefNum("RESCOLD", true, true);
									if (ColdResist != 0)
									{
										this->m_TooltipData.Add(t = new CClientTooltip(1060445)); // cold resist ~1_val~%
										t->FormatArgs("%lld", ColdResist);
									}

									INT64 PoisonResist = pItem->GetDefNum("RESPOISON", true, true);
									if (PoisonResist != 0)
									{
										this->m_TooltipData.Add(t = new CClientTooltip(1060449)); // poison resist ~1_val~%
										t->FormatArgs("%lld", PoisonResist);
									}

									INT64 EnergyResist = pItem->GetDefNum("RESENERGY", true, true);
									if (EnergyResist != 0)
									{
										this->m_TooltipData.Add(t = new CClientTooltip(1060446)); // energy resist ~1_val~%
										t->FormatArgs("%lld", EnergyResist);
									}
								}

								int ArmorRating = pItem->Armor_GetDefense();
								if ( ArmorRating != 0 )
								{
									// Obsolete AR was replaced by physical/fire/cold/poison/energy resist since AOS
									// and doesn't even have proper tooltips. It's just there for backward compatibility
									this->m_TooltipData.Add( t = new CClientTooltip( 1060658 ) ); // ~1_val~: ~2_val~
									t->FormatArgs( "%s\t%d", g_Cfg.GetDefaultMsg(DEFMSG_TOOLTIP_TAG_ARMOR), ArmorRating );
								}

								INT64 StrengthRequirement = pItem->Item_GetDef()->m_ttEquippable.m_StrReq - pItem->GetDefNum("LOWERREQ", true, true);
								if ( StrengthRequirement > 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1061170 ) ); // strength requirement ~1_val~
									t->FormatArgs( "%lld", StrengthRequirement );
								}

								this->m_TooltipData.Add( t = new CClientTooltip( 1060639 ) ); // durability ~1_val~ / ~2_val~
								t->FormatArgs( "%u\t%u", pItem->m_itArmor.m_Hits_Cur, pItem->m_itArmor.m_Hits_Max );
							}
							break;

						case IT_WEAPON_MACE_SMITH:
						case IT_WEAPON_MACE_SHARP:
						case IT_WEAPON_MACE_STAFF:
						case IT_WEAPON_MACE_CROOK:
						case IT_WEAPON_MACE_PICK:
						case IT_WEAPON_SWORD:
						case IT_WEAPON_FENCE:
						case IT_WEAPON_BOW:
						case IT_WEAPON_AXE:
						case IT_WEAPON_XBOW:
						case IT_WEAPON_THROWING:
							{
								if ( pItem->m_itWeapon.m_poison_skill )
									this->m_TooltipData.Add( new CClientTooltip( 1017383 ) ); // poisoned

								if ( pItem->GetDefNum("USEBESTWEAPONSKILL", true, true))
									this->m_TooltipData.Add( new CClientTooltip( 1060400 ) ); // use best weapon skill

								INT64 HitColdArea = pItem->GetDefNum("HITAREACOLD", true, true);
								if ( HitColdArea != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060416 ) ); // hit cold area ~1_val~%
									t->FormatArgs( "%lld", HitColdArea );
								}

								INT64 HitDispel = pItem->GetDefNum("HITDISPEL", true, true);
								if ( HitDispel != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060417 ) ); // hit dispel ~1_val~%
									t->FormatArgs( "%lld", HitDispel );
								}

								INT64 HitEnergyArea = pItem->GetDefNum("HITAREAENERGY", true, true);
								if ( HitEnergyArea != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060418 ) ); // hit energy area ~1_val~%
									t->FormatArgs( "%lld", HitEnergyArea );
								}

								INT64 HitFireArea = pItem->GetDefNum("HITAREAFIRE", true, true);
								if ( HitFireArea != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060419 ) ); // hit fire area ~1_val~%
									t->FormatArgs( "%lld", HitFireArea );
								}

								INT64 HitFireball = pItem->GetDefNum("HITFIREBALL", true, true);
								if ( HitFireball != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060420 ) ); // hit fireball ~1_val~%
									t->FormatArgs( "%lld", HitFireball );
								}

								INT64 HitHarm = pItem->GetDefNum("HITHARM", true, true);
								if ( HitHarm != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060421 ) ); // hit harm ~1_val~%
									t->FormatArgs( "%lld", HitHarm );
								}

								INT64 HitLifeLeech = pItem->GetDefNum("HITLEECHLIFE", true, true);
								if ( HitLifeLeech != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060422 ) ); // hit life leech ~1_val~%
									t->FormatArgs( "%lld", HitLifeLeech );
								}

								INT64 HitLightning = pItem->GetDefNum("HITLIGHTNING", true, true);
								if ( HitLightning != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060422 ) ); // hit lightning ~1_val~%
									t->FormatArgs( "%lld", HitLightning );
								}

								INT64 HitLowerAttack = pItem->GetDefNum("HITLOWERATK", true, true);
								if ( HitLowerAttack != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060424 ) ); // hit lower attack ~1_val~%
									t->FormatArgs( "%lld", HitLowerAttack );
								}

								INT64 HitLowerDefense = pItem->GetDefNum("HITLOWERDEF", true, true);
								if ( HitLowerDefense != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060425 ) ); // hit lower defense ~1_val~%
									t->FormatArgs( "%lld", HitLowerDefense );
								}

								INT64 HitMagicArrow = pItem->GetDefNum("HITMAGICARROW", true, true);
								if ( HitMagicArrow != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060426 ) ); // hit magic arrow ~1_val~%
									t->FormatArgs( "%lld", HitMagicArrow );
								}

								INT64 HitManaLeech = pItem->GetDefNum("HITLEECHMANA", true, true);
								if ( HitManaLeech != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060427 ) ); // hit mana leech ~1_val~%
									t->FormatArgs( "%lld", HitManaLeech );
								}

								INT64 HitPhysicalArea = pItem->GetDefNum("HITAREAPHYSICAL", true, true);
								if ( HitPhysicalArea != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060428 ) ); // hit physical area ~1_val~%
									t->FormatArgs( "%lld", HitPhysicalArea );
								}

								INT64 HitPoisonArea = pItem->GetDefNum("HITAREAPOISON", true, true);
								if ( HitPoisonArea != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060429 ) ); // hit poison area ~1_val~%
									t->FormatArgs( "%lld", HitPoisonArea );
								}

								INT64 HitStaminaLeech = pItem->GetDefNum("HITLEECHSTAM", true, true);
								if ( HitStaminaLeech != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060430 ) ); // hit stamina leech ~1_val~%
									t->FormatArgs( "%lld", HitStaminaLeech );
								}

								INT64 PhysicalDamage = pItem->GetDefNum("DAMPHYSICAL", true, true);
								if ( PhysicalDamage != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060403 ) ); // physical damage ~1_val~%
									t->FormatArgs( "%lld", PhysicalDamage );
								}

								INT64 FireDamage = pItem->GetDefNum("DAMFIRE", true, true);
								if ( FireDamage != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060405 ) ); // fire damage ~1_val~%
									t->FormatArgs( "%lld", FireDamage );
								}

								INT64 ColdDamage = pItem->GetDefNum("DAMCOLD", true, true);
								if ( ColdDamage != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060404 ) ); // cold damage ~1_val~%
									t->FormatArgs( "%lld", ColdDamage );
								}

								INT64 PoisonDamage = pItem->GetDefNum("DAMPOISON", true, true);
								if ( PoisonDamage != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060406 ) ); // poison damage ~1_val~%
									t->FormatArgs( "%lld", PoisonDamage );
								}

								INT64 EnergyDamage = pItem->GetDefNum("DAMENERGY", true, true);
								if ( EnergyDamage != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1060407 ) ); // energy damage ~1_val~%
									t->FormatArgs( "%lld", EnergyDamage );
								}

								INT64 ChaosDamage = pItem->GetDefNum("DAMCHAOS", true, true);
								if ( ChaosDamage != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1072846 ) ); // chaos damage ~1_val~%
									t->FormatArgs( "%lld", ChaosDamage );
								}

								INT64 DirectDamage = pItem->GetDefNum("DAMDIRECT", true, true);
								if ( DirectDamage != 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1079978 ) ); // direct damage: ~1_PERCENT~%
									t->FormatArgs( "%lld", DirectDamage );
								}

								this->m_TooltipData.Add( t = new CClientTooltip( 1061168 ) ); // weapon damage ~1_val~ - ~2_val~
								t->FormatArgs( "%d\t%d", pItem->m_attackBase + pItem->m_ModAr, ( pItem->Weapon_GetAttack(true) ) );

								this->m_TooltipData.Add( t = new CClientTooltip( 1061167 ) ); // weapon speed ~1_val~
								t->FormatArgs( "%d", pItem->GetSpeed() );

								int Range = pItem->RangeL();
								if ( Range > 1 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1061169 ) ); // range ~1_val~
									t->FormatArgs( "%d", Range );
								}

								INT64 StrengthRequirement = pItem->Item_GetDef()->m_ttEquippable.m_StrReq - pItem->GetDefNum("LOWERREQ", true, true);
								if ( StrengthRequirement > 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1061170 ) ); // strength requirement ~1_val~
									t->FormatArgs( "%lld", StrengthRequirement );
								}

								if ( pItem->Item_GetDef()->GetEquipLayer() == LAYER_HAND2 )
									this->m_TooltipData.Add( new CClientTooltip( 1061171 ) ); // two-handed weapon
								else
									this->m_TooltipData.Add( new CClientTooltip( 1061824 ) ); // one-handed weapon

								if ( !pItem->GetDefNum("USEBESTWEAPONSKILL", true, true))
								{
									switch ( pItem->Item_GetDef()->m_iSkill )
									{
										case SKILL_SWORDSMANSHIP:	this->m_TooltipData.Add( new CClientTooltip( 1061172 ) );	break; // skill required: swordsmanship
										case SKILL_MACEFIGHTING:	this->m_TooltipData.Add( new CClientTooltip( 1061173 ) );	break; // skill required: mace fighting
										case SKILL_FENCING:			this->m_TooltipData.Add( new CClientTooltip( 1061174 ) );	break; // skill required: fencing
										case SKILL_ARCHERY:			this->m_TooltipData.Add( new CClientTooltip( 1061175 ) );	break; // skill required: archery
										default:					break;
									}
								}

								this->m_TooltipData.Add( t = new CClientTooltip( 1060639 ) ); // durability ~1_val~ / ~2_val~
								t->FormatArgs( "%u\t%u", pItem->m_itWeapon.m_Hits_Cur, pItem->m_itWeapon.m_Hits_Max );
							}
							break;

						case IT_WAND:
							this->m_TooltipData.Add( t = new CClientTooltip( 1054132 ) ); // [charges: ~1_charges~]
							t->FormatArgs( "%d", pItem->m_itWeapon.m_spellcharges );
							break;

						case IT_TELEPAD:
						case IT_MOONGATE:
							if ( this->IsPriv( PRIV_GM ) )
							{
								this->m_TooltipData.Add( t = new CClientTooltip( 1060658 ) ); // ~1_val~: ~2_val~
								t->FormatArgs( "%s\t%s", g_Cfg.GetDefaultMsg(DEFMSG_TOOLTIP_TAG_DESTINATION), pItem->m_itTelepad.m_pntMark.WriteUsed() );
							}
							break;

						case IT_RUNE:
							{
								const CPointMap pt = pItem->m_itTelepad.m_pntMark;
								if ( !pt.IsValidPoint() )
									break;

								LPCTSTR regionName = g_Cfg.GetDefaultMsg(DEFMSG_RUNE_LOCATION_UNK);
								if ( pt.GetRegion(REGION_TYPE_AREA) )
									regionName = pt.GetRegion(REGION_TYPE_AREA)->GetName();
								bool regionMulti = (pt.GetRegion(REGION_TYPE_MULTI) != NULL);

								if ( pt.m_map == 0 )
									this->m_TooltipData.Add( t = new CClientTooltip( regionMulti? 1062452 : 1060805 ) ); // ~1_val~ (Felucca)[(House)]
								else if ( pt.m_map == 1 )
									this->m_TooltipData.Add( t = new CClientTooltip( regionMulti? 1062453 : 1060806 ) ); // ~1_val~ (Trammel)[(House)]
								else if ( pt.m_map == 3 )
									this->m_TooltipData.Add( t = new CClientTooltip( regionMulti? 1062454 : 1060804 ) ); // ~1_val~ (Malas)[(House)]
								else if ( pt.m_map == 4 )
									this->m_TooltipData.Add( t = new CClientTooltip( regionMulti? 1063260 : 1063259 ) ); // ~1_val~ (Tokuno Islands)[(House)]
								else if ( pt.m_map == 5 )
									this->m_TooltipData.Add( t = new CClientTooltip( regionMulti? 1113206 : 1113205 ) ); // ~1_val~ (Ter Mur)[(House)]
								else
									// There's no proper clilocs for Ilshenar (map2) and custom facets (map > 5), so let's use a generic cliloc
									this->m_TooltipData.Add( t = new CClientTooltip( 1042971 ) ); // ~1_NOTHING~

								t->FormatArgs( "%s %s", g_Cfg.GetDefaultMsg(DEFMSG_RUNE_TO), regionName );
							}
							break;

						case IT_SPELLBOOK:
						case IT_SPELLBOOK_NECRO:
						case IT_SPELLBOOK_PALA:
						case IT_SPELLBOOK_BUSHIDO:
						case IT_SPELLBOOK_NINJITSU:
						case IT_SPELLBOOK_ARCANIST:
						case IT_SPELLBOOK_MYSTIC:
						case IT_SPELLBOOK_BARD:
							{
								int count = pItem->GetSpellcountInBook();
								if ( count > 0 )
								{
									this->m_TooltipData.Add( t = new CClientTooltip( 1042886 ) ); // ~1_NUMBERS_OF_SPELLS~ Spells
									t->FormatArgs( "%d", count );
								}
							} break;

						case IT_SPAWN_CHAR:
							{
								CResourceDef * pSpawnCharDef = g_Cfg.ResourceGetDef( pItem->m_itSpawnChar.m_CharID );
								LPCTSTR pszName = NULL;
								if ( pSpawnCharDef )
								{
									CCharBase *pCharBase = dynamic_cast<CCharBase*>( pSpawnCharDef );
									if ( pCharBase )
										pszName = pCharBase->GetTradeName();
									else
										pszName = pSpawnCharDef->GetName();

									while (*pszName == '#')
										pszName++;
								}

								this->m_TooltipData.Add( t = new CClientTooltip( 1060658 ) ); // ~1_val~: ~2_val~
								t->FormatArgs( "Character\t%s", pszName ? pszName : "none" );
								this->m_TooltipData.Add( t = new CClientTooltip( 1061169 ) ); // range ~1_val~
								t->FormatArgs( "%d", pItem->m_itSpawnChar.m_DistMax );
								this->m_TooltipData.Add( t = new CClientTooltip( 1074247 ) );
								t->FormatArgs( "%lu\t%u", pItem->m_itSpawnChar.m_current, pItem->GetAmount() );
								this->m_TooltipData.Add( t = new CClientTooltip( 1060659 ) ); // ~1_val~: ~2_val~
								t->FormatArgs( "Min/max time\t%u min / %u min", pItem->m_itSpawnChar.m_TimeLoMin, pItem->m_itSpawnChar.m_TimeHiMin );
								this->m_TooltipData.Add( t = new CClientTooltip( 1060660 ) ); // ~1_val~: ~2_val~
								t->FormatArgs( "Time until next spawn\t%lld sec", pItem->GetTimerAdjusted() );
							} break;

						case IT_SPAWN_ITEM:
							{
								CResourceDef * pSpawnItemDef = g_Cfg.ResourceGetDef( pItem->m_itSpawnItem.m_ItemID );
								this->m_TooltipData.Add( t = new CClientTooltip( 1060658 ) ); // ~1_val~: ~2_val~
								t->FormatArgs( "Item\t%s", pSpawnItemDef ? pSpawnItemDef->GetName() : "none" );
								this->m_TooltipData.Add( t = new CClientTooltip( 1060656 ) ); // amount to make: ~1_val~
								t->FormatArgs( "%lu", pItem->m_itSpawnItem.m_pile );
								this->m_TooltipData.Add( t = new CClientTooltip( 1074247 ) );
								t->FormatArgs( "??\t%u", pItem->GetAmount() );
								this->m_TooltipData.Add( t = new CClientTooltip( 1060659 ) ); // ~1_val~: ~2_val~
								t->FormatArgs( "Min/max time\t%u min / %u min", pItem->m_itSpawnItem.m_TimeLoMin, pItem->m_itSpawnItem.m_TimeHiMin );
								this->m_TooltipData.Add( t = new CClientTooltip( 1060660 ) ); // ~1_val~: ~2_val~
								t->FormatArgs( "Time until next spawn\t%lld sec", pItem->GetTimerAdjusted() );
							} break;

						case IT_COMM_CRYSTAL:
							this->m_TooltipData.Add( t = new CClientTooltip( 1060658 ) ); // ~1_val~: ~2_val~
							t->FormatArgs( "Linked\t%s", ( ( (DWORD) pItem->m_uidLink == 0x4FFFFFFF ) ? "No" : "Yes" ) );
							break;

						case IT_STONE_GUILD:
							{
								this->m_TooltipData.Clean(true);
								this->m_TooltipData.Add( t = new CClientTooltip( 1041429 ) );
								const CItemStone * thisStone = dynamic_cast<const CItemStone *>(pItem);
								if ( thisStone )
								{
									if ( thisStone->GetAbbrev()[0] )
									{
										this->m_TooltipData.Add( t = new CClientTooltip( 1060802 ) ); // Guild name: ~1_val~
										t->FormatArgs( "%s [%s]", thisStone->GetName(), thisStone->GetAbbrev() );
									}
									else
									{
										this->m_TooltipData.Add( t = new CClientTooltip( 1060802 ) ); // Guild name: ~1_val~
										t->FormatArgs( "%s", thisStone->GetName() );
									}
								}
							} break;

						default:
							break;
					}
				}
			}
		}
	
#define DOHASH( value ) hash ^= ((value) & 0x3FFFFFF); \
						hash ^= ((value) >> 26) & 0x3F;

		// build a hash value from the tooltip entries
		DWORD hash = 0;
		DWORD argumentHash = 0;
		for (size_t i = 0; i < m_TooltipData.GetCount(); i++)
		{
			CClientTooltip* tipEntry = m_TooltipData.GetAt(i);
			argumentHash = HashString(tipEntry->m_args, strlen(tipEntry->m_args));

			DOHASH(tipEntry->m_clilocid);
			DOHASH(argumentHash);
		}
		hash |= UID_F_ITEM;

#undef DOHASH

		// clients actually expect to use an incremental revision number and not a
		// hash to check if a tooltip needs updating - the client will not request
		// updated tooltip data if the hash happens to be less than the previous one
		//
		// we still want to generate a hash though, so we don't have to increment
		// the revision number if the tooltip hasn't actually been changed
		DWORD revision = 0;
		if (pItem != NULL)
			revision = pItem->UpdatePropertyRevision(hash);
		else if (pChar != NULL)
			revision = pChar->UpdatePropertyRevision(hash);

		propertyList = new PacketPropertyList(pObj, revision, &m_TooltipData);

		// cache the property list for next time, unless property list is
		// incomplete (name only) or caching is disabled
		if (bNameOnly == false && g_Cfg.m_iTooltipCache > 0)
		{
			if (pItem != NULL)
				pItem->SetPropertyList(propertyList);
			else if (pChar != NULL)
				pChar->SetPropertyList(propertyList);
		}
	}

	if (propertyList->isEmpty() == false)
	{
		if (bShop && GetNetState()->isClientSA())
		{
			new PacketPropertyListVersion(this, pObj, propertyList->getVersion());
		}
		else
		{
			switch (g_Cfg.m_iTooltipMode)
			{
				case TOOLTIPMODE_SENDVERSION:
					if (bRequested == false && bShop == false)
					{
						// send property list version (client will send a request for the full tooltip if needed)
						if ( PacketPropertyListVersion::CanSendTo(GetNetState()) == false )
							new PacketPropertyListVersionOld(this, pObj, propertyList->getVersion());
						else
							new PacketPropertyListVersion(this, pObj, propertyList->getVersion());

						break;
					}

					// fall through to send full list

				case TOOLTIPMODE_SENDFULL:
				default:
					// send full property list
					new PacketPropertyList(this, propertyList);
					break;
			}
		}
	}
	
	// delete the original packet, as long as it doesn't belong
	// to the object (i.e. wasn't cached)
	if (propertyList != pObj->GetPropertyList())
		delete propertyList;
}

void CClient::addShowDamage( int damage, DWORD uid_damage )
{
	ADDTOCALLSTACK("CClient::addShowDamage");
	if ( damage < 0 )
		damage = 0;

	if ( PacketCombatDamage::CanSendTo(GetNetState()) )
		new PacketCombatDamage(this, damage, CGrayUID(uid_damage));
	else if ( PacketCombatDamageOld::CanSendTo(GetNetState()) )
		new PacketCombatDamageOld(this, damage, CGrayUID(uid_damage));
}

void CClient::addSpeedMode( int speedMode )
{
	ADDTOCALLSTACK("CClient::addSpeedMode");

	new PacketSpeedMode(this, static_cast<unsigned char>(speedMode));
}

void CClient::addVisualRange( BYTE visualRange )
{
	ADDTOCALLSTACK("CClient::addVisualRange");

	//DEBUG_ERR(("addVisualRange called with argument %d\n", visualRange));

	new PacketVisualRange(this, visualRange);
}

void CClient::addIdleWarning( BYTE message )
{
	ADDTOCALLSTACK("CClient::addIdleWarning");

	new PacketWarningMessage(this, static_cast<PacketWarningMessage::Message>(message));
}

void CClient::addKRToolbar( bool bEnable )
{
	ADDTOCALLSTACK("CClient::addKRToolbar");
	if ( PacketToggleHotbar::CanSendTo(GetNetState()) == false || !IsResClient(RDS_KR) || ( GetConnectType() != CONNECT_GAME ))
		return;

	new PacketToggleHotbar(this, bEnable);
}


// --------------------------------------------------------------------
void CClient::SendPacket( TCHAR * pszKey )
{
	ADDTOCALLSTACK("CClient::SendPacket");
	PacketSend* packet = new PacketSend(0, 0, PacketSend::PRI_NORMAL);
	packet->seek();

	while ( *pszKey )
	{
		if ( packet->getLength() > SCRIPT_MAX_LINE_LEN - 4 )
		{	// we won't get here because this lenght is enforced in all scripts
			DEBUG_ERR(("SENDPACKET too big.\n"));

			delete packet;
			return;
		}

		GETNONWHITESPACE( pszKey );

		if ( toupper(*pszKey) == 'D' )
		{
			++pszKey;
			DWORD iVal = Exp_GetVal(pszKey);

			packet->writeInt32(iVal);
		}
		else if ( toupper(*pszKey) == 'W' )
		{
			++pszKey;
			WORD iVal = static_cast<WORD>(Exp_GetVal(pszKey));

			packet->writeInt16(iVal);
		}
		else
		{
			if ( toupper(*pszKey) == 'B' )
				pszKey++;
			BYTE iVal = static_cast<unsigned char>(Exp_GetVal(pszKey));

			packet->writeByte(iVal);
		}
	}

	packet->trim();
	packet->push(this);
}

// ---------------------------------------------------------------------
// Login type stuff.

BYTE CClient::Setup_Start( CChar * pChar ) // Send character startup stuff to player
{
	ADDTOCALLSTACK("CClient::Setup_Start");
	// Play this char.
	ASSERT( GetAccount() );
	ASSERT( pChar );

	CharDisconnect();	// I'm already logged in as someone else ?
	m_pAccount->m_uidLastChar = pChar->GetUID();

	g_Log.Event( LOGM_CLIENTS_LOG, "%lx:Setup_Start acct='%s', char='%s', IP='%s'\n", GetSocketID(), GetAccount()->GetName(), pChar->GetName(), GetPeerStr() );

	if ( GetPrivLevel() > PLEVEL_Player )		// GMs should login with invul and without allshow flag set
	{
		ClearPrivFlags(PRIV_ALLSHOW);
		pChar->StatFlag_Set(STATF_INVUL);
	}

	addPlayerStart( pChar );
	ASSERT(m_pChar);

	bool fNoMessages = false;
	bool fQuickLogIn = !pChar->IsDisconnected();
	if ( IsTrigUsed(TRIGGER_LOGIN) )
	{
		CScriptTriggerArgs Args( fNoMessages, fQuickLogIn, static_cast<INT64>(0) );
		if ( pChar->OnTrigger( CTRIG_LogIn, pChar, &Args ) == TRIGRET_RET_TRUE )
		{
			m_pChar->ClientDetach();
			pChar->SetDisconnected();
			return PacketLoginError::Blocked;
		}
		fNoMessages	= (Args.m_iN1 != 0);
		fQuickLogIn	= (Args.m_iN2 != 0);
	}

	TCHAR *z = Str_GetTemp();
	if ( !fQuickLogIn )
	{
		if ( !fNoMessages )
		{
			addBark(g_szServerDescription, NULL, HUE_YELLOW, TALKMODE_SYSTEM, FONT_NORMAL);

			sprintf(z, (g_Serv.StatGet(SERV_STAT_CLIENTS)==2) ?
				g_Cfg.GetDefaultMsg( DEFMSG_LOGIN_PLAYER ) : g_Cfg.GetDefaultMsg( DEFMSG_LOGIN_PLAYERS ),
				g_Serv.StatGet(SERV_STAT_CLIENTS)-1 );
			addSysMessage(z);

			sprintf(z, g_Cfg.GetDefaultMsg( DEFMSG_LOGIN_LASTLOGGED ), GetAccount()->m_TagDefs.GetKeyStr("LastLogged"));
			addSysMessage(z);
		}
		if ( m_pChar->m_pArea && m_pChar->m_pArea->IsGuarded() && !m_pChar->m_pArea->IsFlag(REGION_FLAG_ANNOUNCE) )
		{
			const CVarDefContStr * pVarStr = dynamic_cast <CVarDefContStr *>( m_pChar->m_pArea->m_TagDefs.GetKey("GUARDOWNER"));
			SysMessagef(g_Cfg.GetDefaultMsg(DEFMSG_MSG_REGION_GUARDSP), ( pVarStr ) ? pVarStr->GetValStr() : g_Cfg.GetDefaultMsg(DEFMSG_MSG_REGION_GUARDSPT));
			if ( m_pChar->m_pArea->m_TagDefs.GetKeyNum("RED", true) )
				SysMessagef(g_Cfg.GetDefaultMsg(DEFMSG_MSG_REGION_REDDEF), g_Cfg.GetDefaultMsg(DEFMSG_MSG_REGION_REDENTER));
		}
	}

	if ( IsPriv(PRIV_GM_PAGE) && g_World.m_GMPages.GetCount() > 0 )
	{
		sprintf(z, g_Cfg.GetDefaultMsg(DEFMSG_MSG_GMPAGES), static_cast<int>(g_World.m_GMPages.GetCount()), g_Cfg.m_cCommandPrefix);
		addSysMessage(z);
	}
	if ( IsPriv(PRIV_JAILED) )
		m_pChar->Jail(&g_Serv, true, static_cast<int>(GetAccount()->m_TagDefs.GetKeyNum("JailCell", true)));
	if ( g_Serv.m_timeShutdown.IsTimeValid() )
		addBark( g_Cfg.GetDefaultMsg( DEFMSG_MSG_SERV_SHUTDOWN_SOON ), NULL, HUE_TEXT_DEF, TALKMODE_SYSTEM, FONT_BOLD);

	GetAccount()->m_TagDefs.DeleteKey("LastLogged");
	Announce(true);		// announce you to the world

	// Don't login on the water, bring us to nearest shore (unless I can swim)
	if ( !IsPriv(PRIV_GM) && !m_pChar->Char_GetDef()->Can(CAN_C_SWIM) && m_pChar->IsSwimming() )
	{
		int iDist = 1;
		int i;
		for ( i = 0; i < 20; i++ )
		{
			int iDistNew = iDist + 20;
			for ( int iDir = DIR_NE; iDir <= DIR_NW; iDir += 2 )	// try diagonal in all directions
			{
				if ( m_pChar->MoveToValidSpot(static_cast<DIR_TYPE>(iDir), iDistNew, iDist) )
				{
					i = 100;
					break;
				}
			}
			iDist = iDistNew;
		}
		addSysMessage( g_Cfg.GetDefaultMsg( i < 100 ? DEFMSG_MSG_REGION_WATER_1 : DEFMSG_MSG_REGION_WATER_2) );
	}

	DEBUG_MSG(( "%lx:Setup_Start done\n", GetSocketID()));
	return PacketLoginError::Success;
}

BYTE CClient::Setup_Play( unsigned int iSlot ) // After hitting "Play Character" button
{
	ADDTOCALLSTACK("CClient::Setup_Play");
	// Mode == CLIMODE_SETUP_CHARLIST

	DEBUG_MSG(( "%lx:Setup_Play slot %u\n", GetSocketID(), iSlot ));

	if ( ! GetAccount())
		return( PacketLoginError::Invalid );
	if ( iSlot >= COUNTOF(m_tmSetupCharList))
		return( PacketLoginError::BadCharacter );

	CChar * pChar = m_tmSetupCharList[ iSlot ].CharFind();
	if ( ! GetAccount()->IsMyAccountChar( pChar ))
		return( PacketLoginError::BadCharacter );

	CChar * pCharLast = GetAccount()->m_uidLastChar.CharFind();
	if ( pCharLast && GetAccount()->IsMyAccountChar( pCharLast ) && GetAccount()->GetPrivLevel() <= PLEVEL_GM &&
		! pCharLast->IsDisconnected() && (pChar->GetUID() != pCharLast->GetUID()))
	{
		addIdleWarning(PacketWarningMessage::CharacterInWorld);
		return(PacketLoginError::CharIdle);
	}

	// LastLogged update
	CGTime datetime = CGTime::GetCurrentTime();
	GetAccount()->m_TagDefs.SetStr("LastLogged", false, GetAccount()->m_dateLastConnect.Format(NULL));
	GetAccount()->m_dateLastConnect = datetime;

	return Setup_Start( pChar );
}

BYTE CClient::Setup_Delete( unsigned int iSlot ) // Deletion of character
{
	ADDTOCALLSTACK("CClient::Setup_Delete");
	ASSERT( GetAccount() );
	DEBUG_MSG(( "%lx:Setup_Delete slot=%u\n", GetSocketID(), iSlot ));
	if ( iSlot >= COUNTOF(m_tmSetupCharList))
		return PacketDeleteError::NotExist;

	CChar * pChar = m_tmSetupCharList[iSlot].CharFind();
	if ( ! GetAccount()->IsMyAccountChar( pChar ))
		return PacketDeleteError::BadPass;

	if ( ! pChar->IsDisconnected())
	{
		return PacketDeleteError::InUse;
	}

	// Make sure the char is at least x days old.
	if ( g_Cfg.m_iMinCharDeleteTime &&
		(- g_World.GetTimeDiff( pChar->m_timeCreate )) < g_Cfg.m_iMinCharDeleteTime )
	{
		if ( GetPrivLevel() < PLEVEL_Counsel )
		{
			return PacketDeleteError::NotOldEnough;
		}
	}

	//	Do the scripts allow to delete the char?
	enum TRIGRET_TYPE tr;
	CScriptTriggerArgs Args;
	Args.m_pO1 = this;
	pChar->r_Call("f_onchar_delete", pChar, &Args, NULL, &tr);
	if ( tr == TRIGRET_RET_TRUE )
	{
		return PacketDeleteError::InvalidRequest;
	}

	pChar->Delete();

	// refill the list.
	new PacketCharacterListUpdate(this, GetAccount()->m_uidLastChar.CharFind());

	return PacketDeleteError::Success;
}

BYTE CClient::Setup_ListReq( const char * pszAccName, const char * pszPassword, bool fTest )
{
	ADDTOCALLSTACK("CClient::Setup_ListReq");
	// XCMD_CharListReq
	// Gameserver login and request character listing

	if ( GetConnectType() != CONNECT_GAME )	// Not a game connection ?
	{
		return(PacketLoginError::Other);
	}

	switch ( GetTargMode())
	{
		case CLIMODE_SETUP_RELAY:
			ClearTargMode();
			break;

		default:
			break;
	}

	CGString sMsg;
	BYTE lErr = LogIn( pszAccName, pszPassword, sMsg );

	if ( lErr != PacketLoginError::Success )
	{
		if ( fTest && lErr != PacketLoginError::Other )
		{
			if ( ! sMsg.IsEmpty())
			{
				SysMessage( sMsg );
			}
		}
		return( lErr );
	}

	CAccountRef pAcc = GetAccount();
	ASSERT( pAcc );

	CChar * pCharLast = pAcc->m_uidLastChar.CharFind();

/*	if ( pCharLast &&
		GetAccount()->IsMyAccountChar( pCharLast ) &&
		GetAccount()->GetPrivLevel() <= PLEVEL_GM &&
		! pCharLast->IsDisconnected())
	{
		// If the last char is lingering then log back into this char instantly.
		// m_iClientLingerTime
		if ( Setup_Start(pCharLast) )
			return PacketLoginError::Success;
		return PacketLoginError::Blocked; //Setup_Start() returns false only when login blocked by Return 1 in @Login
	} */

	new PacketEnableFeatures(this, g_Cfg.GetPacketFlag(false, static_cast<RESDISPLAY_VERSION>(pAcc->GetResDisp()), static_cast<unsigned char>(maximum(pAcc->GetMaxChars(), pAcc->m_Chars.GetCharCount()))));
	new PacketCharacterList(this, pCharLast);

	m_Targ_Mode = CLIMODE_SETUP_CHARLIST;
	return PacketLoginError::Success;
}

BYTE CClient::LogIn( CAccountRef pAccount, CGString & sMsg )
{
	ADDTOCALLSTACK("CClient::LogIn");
	if ( pAccount == NULL )
		return( PacketLoginError::Invalid );

	if ( pAccount->IsPriv( PRIV_BLOCKED ))
	{
		g_Log.Event(LOGM_CLIENTS_LOG, "%lx: Account '%s' is blocked.\n", GetSocketID(), static_cast<LPCTSTR>(pAccount->GetName()));
		sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_MSG_ACC_BLOCKED ), static_cast<LPCTSTR>(g_Serv.m_sEMail));
		return( PacketLoginError::Blocked );
	}

	// Look for this account already in use.
	CClient * pClientPrev = pAccount->FindClient( this );
	if ( pClientPrev != NULL )
	{
		// Only if it's from a diff ip ?
		ASSERT( pClientPrev != this );

		bool bInUse = false;

		//	different ip - no reconnect
		if ( ! GetPeer().IsSameIP( pClientPrev->GetPeer() )) bInUse = true;
		else
		{
			//	from same ip - allow reconnect if the old char is lingering out
			CChar *pCharOld = pClientPrev->GetChar();
			if ( pCharOld )
			{
				CItem	*pItem = pCharOld->LayerFind(LAYER_FLAG_ClientLinger);
				if ( !pItem ) bInUse = true;
			}

			if ( !bInUse )
			{
				if ( IsConnectTypePacket() && pClientPrev->IsConnectTypePacket())
				{
					pClientPrev->CharDisconnect();
					pClientPrev->GetNetState()->markReadClosed();
				}
				else if ( GetConnectType() == pClientPrev->GetConnectType() ) bInUse = true;
			}
		}

		if ( bInUse )
		{
			g_Log.Event(LOGM_CLIENTS_LOG, "%lx: Account '%s' already in use.\n", GetSocketID(), static_cast<LPCTSTR>(pAccount->GetName()));
			sMsg = "Account already in use.";
			return PacketLoginError::InUse;
		}
	}

	if ( g_Cfg.m_iClientsMax <= 0 )
	{
		// Allow no one but locals on.
		CSocketAddress SockName = GetPeer();
		if ( ! GetPeer().IsLocalAddr() && SockName.GetAddrIP() != GetPeer().GetAddrIP() )
		{
			g_Log.Event(LOGM_CLIENTS_LOG, "%lx: Account '%s', maximum clients reached (only local connections allowed).\n", GetSocketID(), static_cast<LPCTSTR>(pAccount->GetName()));
			sMsg = g_Cfg.GetDefaultMsg( DEFMSG_MSG_SERV_LD );
			return( PacketLoginError::MaxClients );
		}
	}
	if ( g_Cfg.m_iClientsMax <= 1 )
	{
		// Allow no one but Administrator on.
		if ( pAccount->GetPrivLevel() < PLEVEL_Admin )
		{
			g_Log.Event(LOGM_CLIENTS_LOG, "%lx: Account '%s', maximum clients reached (only administrators allowed).\n", GetSocketID(), static_cast<LPCTSTR>(pAccount->GetName()));
			sMsg = g_Cfg.GetDefaultMsg( DEFMSG_MSG_SERV_AO );
			return( PacketLoginError::MaxClients );
		}
	}
	if ( pAccount->GetPrivLevel() < PLEVEL_GM &&
		g_Serv.StatGet(SERV_STAT_CLIENTS) > g_Cfg.m_iClientsMax  )
	{
		// Give them a polite goodbye.
		g_Log.Event(LOGM_CLIENTS_LOG, "%lx: Account '%s', maximum clients reached.\n", GetSocketID(), static_cast<LPCTSTR>(pAccount->GetName()));
		sMsg = g_Cfg.GetDefaultMsg( DEFMSG_MSG_SERV_FULL );
		return( PacketLoginError::MaxClients );
	}
	//	Do the scripts allow to login this account?
	pAccount->m_Last_IP = GetPeer();
	CScriptTriggerArgs Args;
	Args.Init(pAccount->GetName());
	Args.m_iN1 = GetConnectType();
	Args.m_pO1 = this;
	enum TRIGRET_TYPE tr;
	g_Serv.r_Call("f_onaccount_login", &g_Serv, &Args, NULL, &tr);
	if ( tr == TRIGRET_RET_TRUE )
	{
		sMsg = g_Cfg.GetDefaultMsg( DEFMSG_MSG_ACC_DENIED );
		return (PacketLoginError::Blocked);
	}

	m_pAccount = pAccount;
	pAccount->OnLogin( this );

	return( PacketLoginError::Success );
}

BYTE CClient::LogIn( LPCTSTR pszAccName, LPCTSTR pszPassword, CGString & sMsg )
{
	ADDTOCALLSTACK("CClient::LogIn");
	// Try to validate this account.
	// Do not send output messages as this might be a console or web page or game client.
	// NOTE: addLoginErr() will get called after this.

	if ( GetAccount()) // already logged in.
		return( PacketLoginError::Success );

	TCHAR szTmp[ MAX_NAME_SIZE ];
	size_t iLen1 = strlen( pszAccName );
	size_t iLen2 = strlen( pszPassword );
	size_t iLen3 = Str_GetBare( szTmp, pszAccName, MAX_NAME_SIZE );
	if ( iLen1 == 0 || iLen1 != iLen3 || iLen1 > MAX_NAME_SIZE )	// a corrupt message.
	{
		TCHAR szVersion[ 256 ];
		sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_MSG_ACC_WCLI ), static_cast<LPCTSTR>(m_Crypt.WriteClientVer( szVersion )));
		return( PacketLoginError::BadAccount );
	}

	iLen3 = Str_GetBare( szTmp, pszPassword, MAX_NAME_SIZE );
	if ( iLen2 != iLen3 )	// a corrupt message.
	{
		TCHAR szVersion[ 256 ];
		sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_MSG_ACC_WCLI ), static_cast<LPCTSTR>(m_Crypt.WriteClientVer( szVersion )));
		return( PacketLoginError::BadPassword );
	}


	TCHAR szName[ MAX_ACCOUNT_NAME_SIZE ];
	if ( !CAccount::NameStrip(szName, pszAccName) || Str_Check(pszAccName) )
		return( PacketLoginError::BadAccount );
	else if ( Str_Check(pszPassword) )
		return( PacketLoginError::BadPassword );

	bool fGuestAccount = ! strnicmp( pszAccName, "GUEST", 5 );
	if ( fGuestAccount )
	{
		// trying to log in as some sort of guest.
		// Find or create a new guest account.
		TCHAR *pszTemp = Str_GetTemp();
		for ( int i = 0; ; i++ )
		{
			if ( i>=g_Cfg.m_iGuestsMax )
			{
				sMsg = g_Cfg.GetDefaultMsg( DEFMSG_MSG_ACC_GUSED );
				return( PacketLoginError::MaxGuests );
			}

			sprintf(pszTemp, "GUEST%d", i);
			CAccountRef pAccount = g_Accounts.Account_FindCreate(pszTemp, true );
			ASSERT( pAccount );

			if ( pAccount->FindClient() == NULL )
			{
				pszAccName = pAccount->GetName();
				break;
			}
		}
	}
	else
	{
		if ( pszPassword[0] == '\0' )
		{
			sMsg = g_Cfg.GetDefaultMsg( DEFMSG_MSG_ACC_NEEDPASS );
			return( PacketLoginError::BadPassword );
		}
	}

	bool fAutoCreate = ( g_Serv.m_eAccApp == ACCAPP_Free || g_Serv.m_eAccApp == ACCAPP_GuestAuto || g_Serv.m_eAccApp == ACCAPP_GuestTrial );
	CAccountRef pAccount = g_Accounts.Account_FindCreate(pszAccName, fAutoCreate);
	if ( ! pAccount )
	{
		g_Log.Event(LOGM_CLIENTS_LOG, "%lx: Account '%s' does not exist\n", GetSocketID(), pszAccName);
		sMsg.Format(g_Cfg.GetDefaultMsg(DEFMSG_MSG_ACC_UNK), pszAccName);
		return PacketLoginError::Invalid;
	}

	if ( g_Cfg.m_iMaxAccountLoginTries && !pAccount->CheckPasswordTries(GetPeer()))
	{
		g_Log.Event(LOGM_CLIENTS_LOG, "%lx: '%s' exceeded password tries in time lapse\n", GetSocketID(), static_cast<LPCTSTR>(pAccount->GetName()));
		sMsg = g_Cfg.GetDefaultMsg(DEFMSG_MSG_ACC_BADPASS);
		return PacketLoginError::MaxPassTries;
	}

	if ( ! fGuestAccount && ! pAccount->IsPriv(PRIV_BLOCKED) )
	{
		if ( ! pAccount->CheckPassword(pszPassword))
		{
			g_Log.Event(LOGM_CLIENTS_LOG, "%lx: '%s' bad password\n", GetSocketID(), static_cast<LPCTSTR>(pAccount->GetName()));
			sMsg = g_Cfg.GetDefaultMsg(DEFMSG_MSG_ACC_BADPASS);
			return PacketLoginError::BadPass;
		}
	}

	return LogIn(pAccount, sMsg);
}


