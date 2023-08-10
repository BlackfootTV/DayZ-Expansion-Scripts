/**
 * ExpansionMarketBelts.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionMarketBelts: ExpansionMarketCategory
{
	override void Defaults()
	{
		super.Defaults();

		CategoryID = 8;
		DisplayName = "#STR_EXPANSION_MARKET_CATEGORY_BELTS";
		m_FileName = "Belts";

		AddItem("CivilianBelt",		100,	200,	1,		100);
		AddItem("MilitaryBelt", 	100,	200,	1,		100);
	#ifndef DAYZ_1_21
		AddItem("HipPack_Black", 	100,	200,	1,		100, null, {"HipPack_Green", "HipPack_Medical", "HipPack_Party"});
	#endif
	}
};