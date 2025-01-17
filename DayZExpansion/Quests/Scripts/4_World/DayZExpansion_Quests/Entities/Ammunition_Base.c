/**
 * Ammunition_Base.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

modded class Ammunition_Base
{
	override bool CanAddCartridges(int count)
	{
		if (!super.CanAddCartridges(count))
			return false;

		if (Expansion_IsQuestItem())
			return false;

		return true;
	}

	override bool IsCompatiableAmmo( ItemBase ammo )
	{
		if (!super.IsCompatiableAmmo(ammo))
			return false;

		if (Expansion_IsQuestItem() || ammo.Expansion_IsQuestItem())
		{
			if (Expansion_IsDeliveryItem() || ammo.Expansion_IsDeliveryItem())
				return false;
			
			if (Expansion_GetQuestID() != ammo.Expansion_GetQuestID())
				return false;
		}

		return true;
	}
	
	override bool CanBeSplit()
	{
		if (!super.CanBeSplit())
			return false;
		
		if (Expansion_IsQuestItem())
		{
			Man itemOwner = GetHierarchyRootPlayer();
			if (itemOwner && itemOwner.GetIdentity())
			{
				StringLocaliser text = new StringLocaliser("The item %1 is a quest item and can't be split!", GetDisplayName());
				ExpansionNotification("Can't split item", text, "Error", COLOR_EXPANSION_NOTIFICATION_ORANGE, 7).Create(itemOwner.GetIdentity());
			}

			return false;
		}

		return true;
	}
};
