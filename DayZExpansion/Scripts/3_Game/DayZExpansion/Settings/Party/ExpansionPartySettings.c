/**
 * ExpansionPartySettings.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/
class ExpansionPartySettingsBase: ExpansionSettingBase
{
	bool EnableParties;												// enable party module, allow players to create parties
	int MaxMembersInParty; 										// If <= 0, unlimited party size
	bool UseWholeMapForInviteList; 								// Use it if you want whole map available in invite list, instead only nearby players
	bool ShowPartyMember3DMarkers;							// If enabled, allow to see 3D marker above teammates location
	bool ShowDistanceUnderPartyMembersMarkers;		// Show the distance of the party member marker
	bool ShowNameOnPartyMembersMarkers;				// Show the name of the party member marker
	bool EnableQuickMarker;										// Enable/Diable quick marker option
	bool ShowDistanceUnderQuickMarkers;					//  Show the distance of the quick marker
	bool ShowNameOnQuickMarkers;							// Show the distance of the quick marker
	bool CanCreatePartyMarkers;									// Allow player to create party markers
}


/**@class		ExpansionPartySettings
 * @brief		Party settings class
 **/
class ExpansionPartySettings: ExpansionPartySettingsBase
{
	static const int VERSION = 2;
	
	bool ShowPartyMemberHUD;									// Show the party hud interface that displays eacht party members name and health
	
	[NonSerialized()]
	private bool m_IsLoaded;

	// ------------------------------------------------------------
	// Expansion ExpansionPartySettings
	// ------------------------------------------------------------
	void ExpansionPartySettings()
	{
	}
	
	override bool OnRecieve( ParamsReadContext ctx )
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionPartySettings::OnRecieve - Start");
		#endif
		
		ExpansionPartySettings setting;
		if ( !ctx.Read( setting ) )
		{
			Error("ExpansionPartySettings::OnRecieve setting");
			return false;
		}

		CopyInternal( setting );

		m_IsLoaded = true;

		ExpansionSettings.SI_Party.Invoke();
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionPartySettings::OnRecieve - End");
		#endif

		return true;
	}
	
	override void OnSend( ParamsWriteContext ctx )
	{
		ExpansionPartySettings thisSetting = this;

		ctx.Write( thisSetting );
	}

	override int Send( PlayerIdentity identity )
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionPartySettings::Send - Start");
		#endif
		
		if ( !IsMissionHost() )
		{
			return 0;
		}
		
		ScriptRPC rpc = new ScriptRPC;
		OnSend( rpc );
		rpc.Send( null, ExpansionSettingsRPC.Party, true, identity );
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionPartySettings::Send - End and return");
		#endif
		return 0;
	}

	// ------------------------------------------------------------
	// Expansion Send
	// ------------------------------------------------------------
	override bool Copy( ExpansionSettingBase setting )
	{
		ExpansionPartySettings s;
		if ( !Class.CastTo( s, setting ) )
			return false;

		CopyInternal( s );
		return true;
	}
	
	// ------------------------------------------------------------
	private void CopyInternal(  ExpansionPartySettings s )
	{
		ShowPartyMemberHUD = s.ShowPartyMemberHUD;
		
		ExpansionPartySettingsBase sb = s;
		CopyInternal( sb );
	}
	
	// ------------------------------------------------------------
	private void CopyInternal(  ExpansionPartySettingsBase s )
	{
		EnableParties = s.EnableParties;
		MaxMembersInParty = s.MaxMembersInParty;
		UseWholeMapForInviteList = s.UseWholeMapForInviteList;
		ShowPartyMember3DMarkers = s.ShowPartyMember3DMarkers;
		ShowDistanceUnderPartyMembersMarkers = s.ShowDistanceUnderPartyMembersMarkers;
		ShowNameOnPartyMembersMarkers = s.ShowNameOnPartyMembersMarkers;
		EnableQuickMarker = s.EnableQuickMarker;
		ShowDistanceUnderQuickMarkers = s.ShowDistanceUnderQuickMarkers;
		ShowNameOnQuickMarkers = s.ShowNameOnQuickMarkers;
		CanCreatePartyMarkers = s.CanCreatePartyMarkers;
	}
	
	// ------------------------------------------------------------
	override bool IsLoaded()
	{
		return m_IsLoaded;
	}

	// ------------------------------------------------------------
	override void Unload()
	{
		m_IsLoaded = false;
	}

	// ------------------------------------------------------------
	override bool OnLoad()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionPartySettings::Load - Start");
		#endif

		m_IsLoaded = true;

		bool save;

		bool partySettingsExist = FileExist(EXPANSION_PARTY_SETTINGS);

		if (partySettingsExist)
		{
			ExpansionPartySettings settingsDefault = new ExpansionPartySettings;
			settingsDefault.Defaults();

			ExpansionPartySettingsBase settingsBase;

			JsonFileLoader<ExpansionPartySettingsBase>.JsonLoadFile(EXPANSION_PARTY_SETTINGS, settingsBase);

			if (settingsBase.m_Version < VERSION)
			{
				if (settingsBase.m_Version < 2)
				{
					EXPrint("[ExpansionPartySettings] Load - Converting v1 \"" + EXPANSION_PARTY_SETTINGS + "\" to v" + VERSION);
					
					//! New with v2
					CopyInternal(settingsDefault);
				}
				//! Copy over old settings that haven't changed
				CopyInternal(settingsBase);

				m_Version = VERSION;
				save = true;
			}
			else
			{
				JsonFileLoader<ExpansionPartySettings>.JsonLoadFile(EXPANSION_PARTY_SETTINGS, this);
			}
		}
		else
		{
			Defaults();
			save = true;
		}
		
		if (save)
			Save();
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionPartySettings::Load - End - Loaded: " + partySettingsExist);
		#endif
		
		return partySettingsExist;
	}

	// ------------------------------------------------------------
	// Expansion Save
	// ------------------------------------------------------------
	override bool OnSave()
	{
		Print("[ExpansionPartySettings] Saving settings");

		JsonFileLoader<ExpansionPartySettings>.JsonSaveFile( EXPANSION_PARTY_SETTINGS, this );

		return true;
	}

	// ------------------------------------------------------------
	// Expansion Defaults
	// ------------------------------------------------------------
	override void Defaults()
	{
		Print("[ExpansionPartySettings] Loading default settings");
	
		m_Version = VERSION;
		
		EnableParties = true;
		MaxMembersInParty = 10;
		UseWholeMapForInviteList = false;
		ShowPartyMember3DMarkers = true;
		ShowDistanceUnderPartyMembersMarkers = true;
		ShowNameOnPartyMembersMarkers = true;
		EnableQuickMarker = true;
		ShowDistanceUnderQuickMarkers = true;
		ShowNameOnQuickMarkers = true;
		CanCreatePartyMarkers = true;
		ShowPartyMemberHUD = true;
	}
	
	// ------------------------------------------------------------
	override string SettingName()
	{
		return "Party Settings";
	}
};