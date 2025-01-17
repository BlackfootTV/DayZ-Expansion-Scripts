/**
 * MissionServer.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		MissionServer
 * @brief		
 **/
modded class MissionServer
{
	bool EXPANSION_CLASSNAME_DUMP = false;

	// ------------------------------------------------------------
	// MissionServer Destructor
	// ------------------------------------------------------------
	void ~MissionServer()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.GLOBAL, this, "~MissionServer");
#endif
			
		DestroyNotificationSystem();
	}

	override void HandleBody(PlayerBase player)
	{
		//! Avoid exploit where players that are given (temporary) godmode for any reason (e.g. through a 3rd party mod,
		//! since Expansion no longer uses player godmode for anything),
		//! can disconnect while uncon or restrained and reconnect to dupe their character and all their gear
		//! (the other unconscious body will still be on the ground due to the game not being able to kill it)
		if (player.IsUnconscious() || player.IsRestrained())
			player.SetAllowDamage(true);

		super.HandleBody(player);
	}

	// ------------------------------------------------------------
	// OnMissionStart
	// ------------------------------------------------------------
	override void OnMissionStart()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.GLOBAL, this, "OnMissionStart");
#endif
		
		super.OnMissionStart();
		
		GetDayZExpansion().OnStart();	

		if (EXPANSION_CLASSNAME_DUMP)
		{
			DumpClassNameJSON();
			DumpClassNameJSON("Expansion");
		}
	}
	
	// ------------------------------------------------------------
	// OnMissionLoaded
	// ------------------------------------------------------------
	override void OnMissionLoaded()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.GLOBAL, this, "OnMissionLoaded");
#endif
		
		ExpansionGlobalID.s_IsMissionLoaded = true;

		super.OnMissionLoaded();
		
		GetDayZExpansion().OnLoaded();

		CF_ModuleCoreManager.OnSettingsChanged(this, CF_EventArgs.Empty);

		string exitAfter;
		if (GetCLIParam("exitAfter", exitAfter))
		{
			EXPrint(this, "Command line parameter exitAfter found - exiting after " + exitAfter + " seconds");
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().RequestExit, exitAfter.ToInt() * 1000, false, 0);
		}
	}
	
	override void OnClientReadyEvent( PlayerIdentity identity, PlayerBase player )
	{
		super.OnClientReadyEvent( identity, player );

//! DON'T remove! Update this for the next version of DayZ when necessary.
#ifdef DAYZ_1_18
		string preferredVersion = "1.19";
		string version;
		GetDayZGame().GetVersion(version);
		string warningText = "This server is running an unsupported version of the game (" + version + ").";
		if (GetGame().ConfigIsExisting("CfgMods DZ_Expansion_Animations_Preload"))
			warningText += " Some player animations MAY not work correctly.";
		warningText += " Preferably, please use DayZ " + preferredVersion + " (Experimental or stable when available).";
		ExpansionNotification("WARNING", warningText, "Error", COLOR_EXPANSION_NOTIFICATION_ORANGE, 20).Create(identity);
#endif
	}

	// ------------------------------------------------------------
	// OnClientReconnectEvent
	// ------------------------------------------------------------
	override void OnClientReconnectEvent( PlayerIdentity identity, PlayerBase player )
	{
		super.OnClientReconnectEvent( identity, player );

		g_exGlobalSettings.Send( identity );
	}
	
	// ------------------------------------------------------------
	// InvokeOnConnect
	// ------------------------------------------------------------
	override void InvokeOnConnect( PlayerBase player, PlayerIdentity identity )
	{
#ifdef EXPANSION_SETTINGS_DELAY
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Expansion_SendSettings, 20000, false, player);
#else
		GetExpansionSettings().Send( identity );
#endif

		PlayerBase.Expansion_AddPlayer( player, identity );
		
		//! Do after, because some modules use PlayerIdentity setup inside AddPlayer of PlayerBase class
		super.InvokeOnConnect( player, identity );
	}
	
	void Expansion_SendSettings(PlayerBase player)
	{
		GetExpansionSettings().Send(player.GetIdentity());
	}

	// ------------------------------------------------------------
	// PlayerDisconnected
	// ------------------------------------------------------------
	override void PlayerDisconnected( PlayerBase player, PlayerIdentity identity, string uid )
	{
		PlayerBase.RemovePlayer( uid );
		
		super.PlayerDisconnected( player, identity, uid );

		PlayerBase.Expansion_RemovePlayerPlainID(uid);
	}
	
	// ------------------------------------------------------------
	// DumpClassNameJSON
	// ------------------------------------------------------------
	void DumpClassNameJSON(string mustContain = "")
	{
		array<string> classNames();
		Param1<array<string>> dump = new Param1<array<string>>(classNames);
		
		for (int i = 0; i < GetGame().ConfigGetChildrenCount("CfgVehicles"); i++)
	    {
	        string name;
	        GetGame().ConfigGetChildName("CfgVehicles", i, name);
	 
	        if (name == string.Empty)
	            continue;
	        
			ExpansionString exName = new ExpansionString(name);
			if (exName.EndsWith("_Base"))
			{
				continue;
			}
			
			if (mustContain && !name.Contains(mustContain))
				continue;
		        
		    classNames.Insert(name);	
	    }
		
		JsonFileLoader<Param1<array<string>>>.JsonSaveFile("$profile:\\" + mustContain + "ClassNames.json", dump);
	}
};
