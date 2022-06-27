/**
 * ItemBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

static const int RPC_EXP_HARDLINE_DATA = -25301;

modded class ItemBase
{
	ref ExpansionHardlineItemData m_HardlineItemData;
	int m_Rarity = ExpansionHardlineItemRarity.NONE;

	override void DeferredInit()
    {
		super.DeferredInit();
		
		if (!GetExpansionSettings().GetHardline().UseItemRarity)
			return;
		
		string itemName = GetType();
		itemName.ToLower();
		ExpansionHardlineItemData itemData = GetExpansionSettings().GetHardline().GetHardlineItemDataByType(itemName);
		if (itemData)
		{
			m_HardlineItemData = itemData;
			m_Rarity = itemData.GetRarity();
		
		#ifdef EXPANSIONMODHARDLINEDEBUG
			Print("-----------------------------------------------------------------------------------------");
			Print(ToString() + "::DeferredInit - Hardline item class name: " + ClassName());
			Print(ToString() + "::DeferredInit - Hardline item type name: " + GetType());
			Print(ToString() + "::DeferredInit - Hardline item data: " + m_HardlineItemData);
			Print(ToString() + "::DeferredInit - Hardline item rarity: " + m_Rarity);
			Print("-----------------------------------------------------------------------------------------");
		#endif
		}
    }

    ExpansionHardlineItemData GetHardlineItemData()
	{
		return m_HardlineItemData;
	}

	int GetRarity()
	{
		return m_Rarity;
	}

	void SetRarity(int rarity)
	{
		m_Rarity = rarity;
	}
};