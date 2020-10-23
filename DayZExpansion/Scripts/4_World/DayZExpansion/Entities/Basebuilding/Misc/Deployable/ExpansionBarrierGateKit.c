/**
 * ExpansionBarrierGateKit.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2020 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionBarrierGateKit
 * @brief		This class handle base class of expansion basebuilding	
 **/
class ExpansionBarrierGateKit: ExpansionKitLarge
{
	// ------------------------------------------------------------
	// EEInit
	// ------------------------------------------------------------
	override void EEInit()
	{
		//set visual on init
		UpdateVisuals();
		UpdatePhysics();
	}
	
	// ------------------------------------------------------------
	// OnPlacementComplete
	// ------------------------------------------------------------
	#ifdef DAYZ_1_10
	override void OnPlacementComplete( Man player, vector position = "0 0 0", vector orientation = "0 0 0" )
	#else
	override void OnPlacementComplete( Man player )
	#endif
	{
		if ( IsMissionHost() )
		{
			//! Create gate
			#ifndef DAYZ_1_10
			PlayerBase player_base = PlayerBase.Cast( player );
			vector position = player_base.GetLocalProjectionPosition();
			vector orientation = player_base.GetLocalProjectionOrientation();
			#endif
			
			ExpansionBarrierGate gate = ExpansionBarrierGate.Cast( GetGame().CreateObject( "ExpansionBarrierGate", GetPosition() ) );
			gate.SetPosition( position );
			gate.SetOrientation( orientation );
			
			//! Make the kit invisible, so it can be destroyed from deploy UA when action ends
			HideAllSelections();

			GetGame().ObjectDelete(this);
		}	
		
		SetIsDeploySound( true );
	}

	override string GetDeploySoundset()
	{
		return "placeLargeTent_SoundSet";
	}
	
	override string GetLoopDeploySoundset()
	{
		return "largetent_deploy_SoundSet";
	}
}