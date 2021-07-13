/**
 * ExpansionLoadingScreenBackground.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionLoadingScreenBackground
{
	string MapName;
	ref array<string> Path;
	
	// ------------------------------------------------------------
	// ExpansionLoadingScreenBackground Constructor
	// ------------------------------------------------------------
	void ExpansionLoadingScreenBackground( string map_name, ref array<string> texture_path ) 
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionLoadingScreenBackground::Constructor - Start");
		#endif
		
		MapName = map_name;
		Path = texture_path;
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionLoadingScreenBackground::Constructor - End");
		#endif
	}
};