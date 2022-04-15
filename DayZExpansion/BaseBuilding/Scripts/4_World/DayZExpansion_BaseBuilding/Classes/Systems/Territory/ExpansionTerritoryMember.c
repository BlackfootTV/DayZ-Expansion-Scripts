/**
 * ExpansionTerritoryMember.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionTerritoryMember
{
	protected string m_ID;
	protected string m_Name;
	protected ExpansionTerritoryRank m_Rank;

	#ifdef EXPANSION_MODSTORAGE
	void OnStoreSave(CF_ModStorage ctx)
	{
		ctx.Write(m_ID);
		ctx.Write(m_Name);
		ctx.Write(m_Rank);
	}

	bool OnStoreLoad( CF_ModStorage ctx )
	{
		if (!ctx.Read(m_ID))
			return false;
		if (!ctx.Read(m_Name))
			return false;
		if (!ctx.Read(m_Rank))
			return false;

		return true;
	}
	#endif
	
	string GetID()
	{
		return m_ID;	
	}
	
	string GetName()
	{
		return m_Name;
	}
	
	string GetRankName()
	{
		switch (m_Rank)
		{
			case ExpansionTerritoryRank.ADMIN:
			{
				return "#STR_EXPANSION_PARTY_RANK_ADMIN";
			}
			break;
			
			case ExpansionTerritoryRank.MODERATOR:
			{
				return "#STR_EXPANSION_PARTY_RANK_MOD";
			}
			break;
			
			case ExpansionTerritoryRank.MEMBER:
			{
				return "#STR_EXPANSION_PARTY_RANK_MEMBER";
			}
			break;
		}
		
		return "#STR_EXPANSION_PARTY_RANK_MEMBER";
	}
	
	ExpansionTerritoryRank GetRank()
	{
		return m_Rank;
	}
	
	void SetRank( ExpansionTerritoryRank rank )
	{
		m_Rank = rank;
	}
	
	void ExpansionTerritoryMember(string ID = "", string name = "", bool owner = false)
	{
		m_ID = ID;
		m_Name = name;
		
		if ( owner ) 
		{
			m_Rank = ExpansionTerritoryRank.ADMIN;
		}
		else
		{
			m_Rank = ExpansionTerritoryRank.MEMBER;
		}
	}
};