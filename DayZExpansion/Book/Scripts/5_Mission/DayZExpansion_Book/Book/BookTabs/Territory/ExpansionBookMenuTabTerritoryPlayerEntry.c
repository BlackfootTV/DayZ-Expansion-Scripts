/**
 * ExpansionBookMenuTabTerritoryPlayerEntry.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

#ifdef EXPANSIONMODBASEBUILDING
class ExpansionBookMenuTabTerritoryPlayerEntry: ExpansionScriptView
{
	ref ExpansionBookMenuTabTerritory m_TerritoryTab;
	ref SyncPlayer m_Player;
	ref ExpansionBookMenuTabTerritoryPlayerEntryController m_EntryController;
	
	TextWidget player_entry_label;
	ButtonWidget player_entry_button;
	ImageWidget player_entry_icon;
	
	void ExpansionBookMenuTabTerritoryPlayerEntry(ExpansionBookMenuTabTerritory tab, SyncPlayer player)
	{
		m_TerritoryTab = tab;
		m_Player = player;
				
		if (!m_EntryController)
			m_EntryController = ExpansionBookMenuTabTerritoryPlayerEntryController.Cast(GetController());
		
		SetEntry(m_Player);
	}
	
	void SetEntry(SyncPlayer player)
	{
		m_EntryController.PlayerName = player.m_PlayerName;
		m_EntryController.NotifyPropertyChanged("PlayerName");
		
		//m_EntryController.Background = GetRandomElementBackground();
		m_EntryController.Background = "set:book_assets image:menu_element_1";
		m_EntryController.NotifyPropertyChanged("Background");
	}
	
	/*string GetRandomElementBackground()
	{
		TStringArray backgrounds = new TStringArray;
		
		backgrounds.Insert("set:book_assets image:menu_element_1");
		backgrounds.Insert("set:book_assets image:menu_element_2");
		backgrounds.Insert("set:book_assets image:menu_element_3");
		backgrounds.Insert("set:book_assets image:menu_element_4");
		backgrounds.Insert("set:book_assets image:menu_element_5");
		backgrounds.Insert("set:book_assets image:menu_element_6");
		
		return backgrounds.GetRandomElement();
	}*/
	
	void OnElementButtonClick()
	{
		if (!m_TerritoryTab)
			return;
		
		if (!m_Player)
			return;
		
		m_TerritoryTab.InvitePlayer(m_Player);
	}
	
	override string GetLayoutFile() 
	{
		return "DayZExpansion/Book/GUI/layouts/tabs/territory/expansion_book_tab_territory_player_entry.layout";
	}
	
	override typename GetControllerType() 
	{
		return ExpansionBookMenuTabTerritoryPlayerEntryController;
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w == player_entry_button)
		{
			player_entry_label.SetColor(ARGB(255, 220, 220, 220));
			player_entry_icon.SetColor(ARGB(255, 220, 220, 220));
		}
		
		return super.OnMouseEnter(w, x, y);
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w == player_entry_button)
		{
			player_entry_label.SetColor(ARGB(255, 0, 0, 0));
			player_entry_icon.SetColor(ARGB(255, 0, 0, 0));
		}
		
		return super.OnMouseLeave(w, enterW, x, y);
	}
};

class ExpansionBookMenuTabTerritoryPlayerEntryController: ExpansionViewController
{
	string PlayerName;
	string Background;
};
#endif
