/**
 * BaseBuildingBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

modded class BaseBuildingBase
{
	override vector GetKitSpawnPosition()
	{
		return super.GetKitSpawnPosition();
	}

	override bool IsOpen()
	{
		if (!super.IsOpen())
			return false;

		//! For 3rd party mod compat (we override and return ExpansionIsOpened() in ExpansionbaseBuilding::IsOpened to support our basebuilding transparently)
		if (ExpansionIsOpenable())
			return IsOpened();

		//! @note vanilla by default returns true, we need to keep this compatible
		return true;
	}
}
