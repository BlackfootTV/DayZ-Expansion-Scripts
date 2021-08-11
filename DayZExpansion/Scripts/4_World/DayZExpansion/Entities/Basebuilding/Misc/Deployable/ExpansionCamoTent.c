/**
 * ExpansionCamoTent.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionCamoTent
 * @brief		
 **/
//! TODO: Is there a good reason this inherits from ExpansionBaseBuilding and not DeployableContainer_Base or similar?
class ExpansionCamoTent: ExpansionBaseBuilding 
{
	int m_CanBeDamaged = -1;

	// ------------------------------------------------------------
	// Constructor
	// ------------------------------------------------------------
	void ExpansionCamoTent()
	{
	}

	// ------------------------------------------------------------
	// Destructor
	// ------------------------------------------------------------
	void ~ExpansionCamoTent()
	{
	}

	// ------------------------------------------------------------
	// CanPutInCargo
	// ------------------------------------------------------------
	override bool CanPutInCargo (EntityAI parent)
	{
		return false;
	}

	// ------------------------------------------------------------
	// CanPutIntoHands
	// ------------------------------------------------------------
	override bool CanPutIntoHands(EntityAI parent)
	{
		return false;
	}
	
	override bool CanObstruct()
	{
		return true;
	}

	// ------------------------------------------------------------
	// IsInventoryVisible
	// ------------------------------------------------------------
	override bool IsInventoryVisible()
	{
		return true;
	}

	// ------------------------------------------------------------
	// GetConstructionKitType
	// ------------------------------------------------------------
	override string GetConstructionKitType()
	{
		return "ExpansionCamoTentKit";
	}

	// ------------------------------------------------------------
	// CanBeDamaged
	// ------------------------------------------------------------
	override bool CanBeDamaged()
	{
		if ( m_CanBeDamaged == -1 && GetGame() && GetGame().IsMultiplayer() && GetGame().IsServer() )
			m_CanBeDamaged = GetGame().ServerConfigGetInt( "disableContainerDamage" ) == 0;

		return m_CanBeDamaged;
	}

	override void SetPartsAfterStoreLoad()
	{
		
	}
}

class ExpansionATent: ExpansionCamoTent 
{
}