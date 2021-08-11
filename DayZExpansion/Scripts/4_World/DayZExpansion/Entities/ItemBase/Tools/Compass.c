/**
 * Compass.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

modded class Compass
{	
	private bool m_Added = false;
	
	// ------------------------------------------------------------
	// Compass EEInventoryIn
	// ------------------------------------------------------------	
	override void EEInventoryIn(Man newParentMan, EntityAI diz, EntityAI newParent)
	{
		//#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("Compass::EEInventoryIn - Start");
		//#endif
		
		super.EEInventoryIn( newParentMan, diz, newParent );
		
		if ( IsMissionClient() && newParentMan && newParentMan.IsInherited( SurvivorBase ) )
		{
			PlayerBase newParentBase;
			if (Class.CastTo(newParentBase,newParentMan) && !m_Added)
			{
				m_Added = true;
				
				newParentBase.SetHasItemCompass( true );
			}
		}
		
		//#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("Compass::EEInventoryIn - End");
		//#endif
	}
	
	// ------------------------------------------------------------
	// Compass EEInventoryOut
	// ------------------------------------------------------------	
	override void EEInventoryOut(Man oldParentMan, EntityAI diz, EntityAI newParent)
	{
		//#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("Compass::EEInventoryOut - Start");
		//#endif
		
		super.EEInventoryOut( oldParentMan, diz, newParent );
		
		if ( IsMissionClient() && oldParentMan && oldParentMan.IsInherited( SurvivorBase ) )
		{
			PlayerBase oldParentBase;
			if (Class.CastTo(oldParentBase, oldParentMan) && m_Added)
			{
				m_Added = false;
				
				oldParentBase.SetHasItemCompass( false );
			}
		}
		
		//#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("Compass::EEInventoryOut - End");
		//#endif
	}
	
	// ------------------------------------------------------------
	// Compass OnInventoryExit
	// ------------------------------------------------------------
	//! Inventory manipulation
	override void OnInventoryExit(Man player)
	{
		//#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("Compass::OnInventoryExit - Start");
		//#endif
			
		super.OnInventoryExit(player);
				
		if ( GetGame() && (!GetGame().IsServer() || !GetGame().IsMultiplayer()) )
		{
			if (GetHierarchyParent())
			{
				ItemBase item;
				if (Class.CastTo(item, GetHierarchyParent()))
				{
					PlayerBase parentPlayer;
					if (!Class.CastTo(parentPlayer, item.GetHierarchyRootPlayer()) && m_Added)
					{
						m_Added = false;
						
						PlayerBase.Cast(player).SetHasItemCompass( false );
					}
					else if (Class.CastTo(parentPlayer, item.GetHierarchyRootPlayer()) && m_Added)
					{
						if (parentPlayer != player)
						{
							PlayerBase playerBase;
							if (!Class.CastTo(playerBase, player) && m_Added)
							{
								m_Added = false;
								playerBase.SetHasItemCompass( false );
							}
						}
					}
				}
			}
		}
	
		//#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("Compass::OnInventoryExit - End");
		//#endif
	}
	
	// ------------------------------------------------------------
	// Compass OnInventoryExit
	// ------------------------------------------------------------
	override void OnInventoryEnter(Man player)
	{
		//#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("Compass::OnInventoryEnter - Start");
		//#endif
			
		super.OnInventoryEnter(player);
		
		if ( GetGame() && (!GetGame().IsServer() || !GetGame().IsMultiplayer()) )
		{
			if (GetHierarchyParent())
			{
				ItemBase item;
				if (Class.CastTo(item, GetHierarchyParent()))
				{
					PlayerBase parentPlayer;
					if (Class.CastTo(parentPlayer, item.GetHierarchyRootPlayer()) && !m_Added)
					{
						m_Added = true;
						
						parentPlayer.SetHasItemCompass( true );
					}
				}
			}
		}
		
		//#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("Compass::OnInventoryEnter - End");
		//#endif
	}
}