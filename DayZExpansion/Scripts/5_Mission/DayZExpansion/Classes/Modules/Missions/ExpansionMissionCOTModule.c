/**
 * ExpansionMissionCOTModule.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionMissionCOTModule: JMRenderableModuleBase
{
	private ExpansionMissionSettings settings;

	void ExpansionMissionCOTModule()
	{
		GetPermissionsManager().RegisterPermission( "Expansion.Mission" );

		GetPermissionsManager().RegisterPermission( "Expansion.Mission.View" );

		ExpansionSettings.SI_Mission.Insert( OnSettingsUpdated );
	}

	void ~ExpansionMissionCOTModule()
	{
		ExpansionSettings.SI_Mission.Remove( OnSettingsUpdated );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		Load();
	}
	
	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();

		settings = GetExpansionSettings().GetMission();
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Expansion.Mission.View" );
	}

	override string GetLayoutRoot()
	{
		return "DayZExpansion/GUI/layouts/COT/mission_form.layout";
	}

	override string GetTitle()
	{
		return "[EX] Mission Management";
	}
	
	override string GetIconName()
	{
		return "M";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override int GetRPCMin()
	{
		return ExpansionMissionCOTModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return ExpansionMissionCOTModuleRPC.COUNT;
	}

	#ifdef CF_BUGFIX_REF
	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	#else
	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	#endif
	{
		switch ( rpc_type )
		{
		case ExpansionMissionCOTModuleRPC.Load:
			RPC_Load( ctx, sender, target );
			break;
		}
	}

	void Load()
	{
		if ( GetGame().IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, ExpansionMissionCOTModuleRPC.Load, true, NULL );
		} else
		{
			OnSettingsUpdated();
		}
	}

	private void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( GetPermissionsManager().HasPermission( "Expansion.Mission.View", senderRPC ) )
		{
			GetExpansionSettings().GetMission().Send( senderRPC );
		}
	}
}