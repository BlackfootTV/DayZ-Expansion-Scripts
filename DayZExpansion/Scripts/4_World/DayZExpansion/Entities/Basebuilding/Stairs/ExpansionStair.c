/**
 * ExpansionStair.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionStairBase extends ExpansionBaseBuilding
{
	private bool m_HasStair;

	void ExpansionStairBase()
	{
		RegisterNetSyncVariableBool( "m_HasStair" );

		m_CurrentBuild = "wood";
	}

	override string GetConstructionKitType()
	{
		return "ExpansionStairKit";
	}

	override bool NameOverride(out string output)
	{
		if (IsLastStage())
			output = "#STR_EXPANSION_BB_" + m_CurrentBuild + "_STAIR_FINISHED";
		else
			output = "#STR_EXPANSION_BB_" + m_CurrentBuild + "_STAIR_BASE";
		return true;
	}

	override bool IsLastStage()
	{
		return m_HasStair;
	}

	override bool IsLastStageBuilt()
	{
		return IsPartBuilt( m_CurrentBuild + "_tread" ) );
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		UpdateVisuals();
	}
	
	override void AfterStoreLoad()
	{	
		super.AfterStoreLoad();

		if ( m_ExpansionSaveVersion < 18 )
			m_HasStair = IsLastStageBuilt();
		
		UpdateVisuals();	
	}

	override void OnStoreSave( ParamsWriteContext ctx )
	{
		#ifdef CF_MODULE_MODSTORAGE
		if ( GetGame().SaveVersion() >= EXPANSION_VERSION_GAME_MODSTORAGE_TARGET )
		{
			super.OnStoreSave( ctx );
			return;
		}
		#endif

		super.OnStoreSave( ctx );
		
		ctx.Write( m_HasStair );
	}

	override bool OnStoreLoad( ParamsReadContext ctx, int version )
	{
		if ( Expansion_Assert_False( super.OnStoreLoad( ctx, version ), "[" + this + "] Failed reading OnStoreLoad super" ) )
			return false;

		#ifdef CF_MODULE_MODSTORAGE
		if ( version > EXPANSION_VERSION_GAME_MODSTORAGE_TARGET || m_ExpansionSaveVersion > EXPANSION_VERSION_SAVE_MODSTORAGE_TARGET )
			return true;
		#endif

		if ( m_ExpansionSaveVersion < 18 )
			return true;

		if ( Expansion_Assert_False( ctx.Read( m_HasStair ), "[" + this + "] Failed reading m_HasStair" ) )
			return false;
		
		return true;
	}

	#ifdef CF_MODULE_MODSTORAGE
	override void CF_OnStoreSave( CF_ModStorage storage, string modName )
	{
		super.CF_OnStoreSave( storage, modName );

		if ( modName != "DZ_Expansion" )
			return;

		storage.Write( m_HasStair );
	}
	
	override bool CF_OnStoreLoad( CF_ModStorage storage, string modName )
	{
		if ( !super.CF_OnStoreLoad( storage, modName ) )
			return false;

		if ( modName != "DZ_Expansion" )
			return true;

		if ( storage.GetVersion() < 18 )
			return true;

		if ( Expansion_Assert_False( storage.Read( m_HasStair ), "[" + this + "] Failed reading m_HasStair" ) )
			return false;

		return true;
	}
	#endif

	override bool ExpansionGetCollisionBox( out vector minMax[2] )
	{
		minMax[0] = "-0.75 0 -1.5";
		minMax[1] = "0.75 3.25 1.5";
		return true;
	}

	override void OnPartBuiltServer( notnull Man player, string part_name, int action_id )
	{
		m_HasStair = false;

		ExpansionUpdateBaseBuildingStateFromPartBuilt( part_name );

		super.OnPartBuiltServer(player, part_name, action_id );
		UpdateVisuals();
	}

	override void ExpansionUpdateBaseBuildingStateFromPartBuilt( string part_name )
	{
		if ( part_name == m_CurrentBuild + "_tread" )
		{
			m_HasStair = true;
		}
	}

	override void OnPartDismantledServer( notnull Man player, string part_name, int action_id )
	{
		m_HasStair = false;

		super.OnPartDismantledServer( player, part_name, action_id );
		UpdateVisuals();
	}

	override void OnPartDestroyedServer( Man player, string part_name, int action_id, bool destroyed_by_connected_part = false )
	{
		m_HasStair = false;

		super.OnPartDestroyedServer( player, part_name, action_id, destroyed_by_connected_part );
		UpdateVisuals();
	}

	override bool CanPutInCargo (EntityAI parent)
	{
		return false;
	}

	override bool IsFacingPlayer( PlayerBase player, string selection )
	{
		return false;
	}

	override bool IsFacingCamera( string selection )
	{
		return false;
	}
} 