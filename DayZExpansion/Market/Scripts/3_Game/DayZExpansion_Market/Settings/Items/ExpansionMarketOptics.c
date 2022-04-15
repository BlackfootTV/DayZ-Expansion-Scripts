/**
 * ExpansionMarketOptics.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionMarketOptics: ExpansionMarketCategory
{
	override void Defaults()
	{
		super.Defaults();

		CategoryID = 44;
		DisplayName = "#STR_EXPANSION_MARKET_CATEGORY_OPTICS";
		m_FileName = "Optics";
		
		AddItem("PistolOptic", 					100,		200,	1,		100);
		AddItem("ReflexOptic", 					150,		300,	1,		100,	{"Battery9V"});
		AddItem("M4_CarryHandleOptic", 			150,		300,	1,		100);
		AddItem("BUISOptic", 					150,		300,	1,		100);
		AddItem("FNP45_MRDSOptic", 				250,		500,	1,		100,	{"Battery9V"});
		AddItem("ACOGOptic", 					300,		600,	1,		100);
		AddItem("ACOGOptic_6x", 				1200,		2400,	1,		100);
		AddItem("M68Optic", 					500,		1000,	1,		100,	{"Battery9V"});
		AddItem("M4_T3NRDSOptic", 				500,		1000,	1,		100,	{"Battery9V"});
		AddItem("KobraOptic", 					600,		1200,	1,		100,	{"Battery9V"});		
		AddItem("KashtanOptic", 				700,		1400,	1,		100);
		AddItem("PUScopeOptic", 				900,		1800,	1,		100);
		AddItem("HuntingOptic", 				1000,		2000,	1,		100);
		AddItem("PSO1Optic", 					1100,		2200,	1,		100,	{"Battery9V"});
		AddItem("PSO11Optic", 					1400,		2800,	1,		100,	{"Battery9V"});
		AddItem("KazuarOptic", 					2200,		4400,	1,		100,	{"Battery9V"});
		AddItem("StarlightOptic", 				2400,		4800,	1,		100,	{"Battery9V"});
		
	#ifdef EXPANSIONMODWEAPONS	
		AddItem("Expansion_M1A_RailAtt", 		150,		300,	1,		100);
		AddItem("Expansion_MP5_RailAtt", 		150,		300,	1,		100);
		AddItem("ExpansionReflexMRSOptic", 		150,		300,	1,		100,	{"Battery9V"});
		AddItem("ExpansionDeltapointOptic", 	150,		300,	1,		100);
		AddItem("ExpansionEXPS3HoloOptic", 		200,		400,	1,		100,	{"Battery9V"});
		AddItem("ExpansionHAMROptic", 			400,		800,	1,		100,	{"Battery9V","ExpansionDeltapointOptic"});
		AddItem("Expansion_PMII25Optic", 		1100,		2200,	1,		100);
		AddItem("ExpansionKar98ScopeOptic", 	1100,		2200,	1,		100);
		
		//! For other players, laser points to the side (owner view ok)
		//AddItem("Expansion_ANPEQ15_Red", 		200,		400,	1,		100);
		//AddItem("Expansion_ANPEQ15_Green", 	200,		400,	1,		100);
	#endif
	}	
};