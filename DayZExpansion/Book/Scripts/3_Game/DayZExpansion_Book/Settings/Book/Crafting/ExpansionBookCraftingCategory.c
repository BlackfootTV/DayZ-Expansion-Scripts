/**
 * ExpansionBookCraftingCategory.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionBookCraftingCategory
{
	string CategoryName;
	autoptr TStringArray Results = new TStringArray;

	void ExpansionBookCraftingCategory(string name, TStringArray results = NULL)
	{
		CategoryName = name;
		if (results)
			Results = results;
	}

	void AddResult(string result)
	{
		result.ToLower();
		Results.Insert(result);
	}
};
