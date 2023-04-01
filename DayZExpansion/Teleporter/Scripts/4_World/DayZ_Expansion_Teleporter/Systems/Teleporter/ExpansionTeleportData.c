/**
 * ExpansionTeleportData.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionTeleportDataBase
{
	int ConfigVersion;
};

class ExpansionTeleportData: ExpansionTeleportDataBase
{
	[NonSerialized()];
	static const int VERSION = 0;

	protected int m_ID;
	protected string m_DisplayName;
	protected vector m_ObjectPosition;
	protected vector m_ObjectOrientation;
	protected ref array<ref ExpansionTeleportPosition> m_TeleportPositions;

#ifdef EXPANSIONMODAI
	protected string m_FactionName;
#endif
#ifdef EXPANSIONMODHARDLINE
	protected int m_Reputation;
#endif
#ifdef EXPANSIONMODQUESTS
	protected int m_QuestID = -1;
#endif

	void ExpansionTeleportData()
	{
		ConfigVersion = VERSION;
		m_TeleportPositions = new array<ref ExpansionTeleportPosition>;
	}

	void CopyFromBaseClass(ExpansionTeleportDataBase base)
	{
		//! Nothing to do here yet
	}

	static ExpansionTeleportData Load(string fileName)
	{
		CF_Log.Info("[ExpansionTeleportData] Load existing teleporter file:" + fileName);
		ExpansionTeleportDataBase teleporterDataBase;
		ExpansionJsonFileParser<ExpansionTeleportDataBase>.Load(fileName, teleporterDataBase);

		bool save;
		ExpansionTeleportData teleporterData = new ExpansionTeleportData();
		if (teleporterDataBase.ConfigVersion < VERSION)
		{
			save = true;
			teleporterData.CopyFromBaseClass(teleporterDataBase); //! Copy over old data that has not changed.
			teleporterData.ConfigVersion = VERSION;

			if (save)
				Save(teleporterData);
		}
		else
		{
			if (!ExpansionJsonFileParser<ExpansionTeleportData>.Load(fileName, teleporterData))
				return NULL;
		}

		return teleporterData;
	}

	void Save()
	{
		ExpansionJsonFileParser<ExpansionTeleportData>.Save(ExpansionTeleporterModule.s_TeleporterDataFolderPath + "Teleporter_" + m_ID + ".json", this);
	}

	static void Save(ExpansionTeleportData teleporterData)
	{
		ExpansionJsonFileParser<ExpansionTeleportData>.Save(ExpansionTeleporterModule.s_TeleporterDataFolderPath + "Teleporter_" + teleporterData.GetID() + ".json", teleporterData);
	}

	void AddTeleportPosition(ExpansionTeleportPosition pos)
	{
		m_TeleportPositions.Insert(pos);
	}

	void SetID(int id)
	{
		m_ID = id;
	}

	int GetID()
	{
		return m_ID;
	}
	
	void SetDisplayName(string name)
	{
		m_DisplayName = name;
	}
	
	string GetDisplayName()
	{
		return m_DisplayName;
	}

	void SetObjectPosition(vector objPos)
	{
		m_ObjectPosition = objPos;
	}

	void SetObjectOrientation(vector objOri)
	{
		m_ObjectOrientation = objOri;
	}

	vector GetObjectPosition()
	{
		return m_ObjectPosition;
	}

	vector GetObjectOrientation()
	{
		return m_ObjectOrientation;
	}

#ifdef EXPANSIONMODAI
	void SetFactionName(string factionName)
	{
		m_FactionName = factionName;
	}

	string GetFactionName()
	{
		return m_FactionName;
	}
#endif

#ifdef EXPANSIONMODHARDLINE
	void SetReputation(int reputation)
	{
		m_Reputation = reputation;
	}

	int GetReputation()
	{
		return m_Reputation;
	}
#endif

#ifdef EXPANSIONMODQUESTS
	void SetQuestID(int questID)
	{
		m_QuestID = questID;
	}

	int GetQuestID()
	{
		return m_QuestID;
	}
#endif

	array<ref ExpansionTeleportPosition> GetTeleportPositions()
	{
		return m_TeleportPositions;
	}

	void SpawnTeleporter()
	{
		Object obj = GetGame().CreateObjectEx("Expansion_Teleporter_Big", m_ObjectPosition, ECE_NONE);
		Expansion_Teleporter_Big teleportObj = Expansion_Teleporter_Big.Cast(obj);
		if (!teleportObj)
			GetGame().ObjectDelete(obj);

		teleportObj.SetPosition(m_ObjectPosition);
		teleportObj.SetOrientation(m_ObjectOrientation);
		teleportObj.SetTeleporterID(m_ID);
	}

	void OnSend(ParamsWriteContext ctx)
	{
		auto trace = EXTrace.Start(EXTrace.TELEPORTER, this);

		ctx.Write(m_ID);
		ctx.Write(m_DisplayName);
		ctx.Write(m_ObjectPosition);
	#ifdef EXPANSIONMODAI
		ctx.Write(m_FactionName);
	#endif
	#ifdef EXPANSIONMODHARDLINE
		ctx.Write(m_Reputation);
	#endif
	#ifdef EXPANSIONMODQUESTS
		ctx.Write(m_QuestID);
	#endif

		int positionsCount = m_TeleportPositions.Count();
		ctx.Write(positionsCount);

		for (int i = 0; i < positionsCount; i++)
		{
			ExpansionTeleportPosition teleportPos = m_TeleportPositions[i];
			teleportPos.OnSend(ctx);
		}
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(m_ID))
			return false;
		
		if (!ctx.Read(m_DisplayName))
			return false;
		
		if (!ctx.Read(m_ObjectPosition))
			return false;
		
	#ifdef EXPANSIONMODAI
		if (!ctx.Read(m_FactionName))
			return false;
	#endif
		
	#ifdef EXPANSIONMODHARDLINE
		if (!ctx.Read(m_Reputation))
			return false;
	#endif
		
	#ifdef EXPANSIONMODQUESTS
		if (!ctx.Read(m_QuestID))
			return false;
	#endif

		int positionsCount;
		if (!ctx.Read(positionsCount))
			return false;

		for (int i = 0; i < positionsCount; i++)
		{
			ExpansionTeleportPosition teleportPos = new ExpansionTeleportPosition();
			if (!teleportPos.OnRecieve(ctx))
				return false;

			m_TeleportPositions.Insert(teleportPos);
		}

		return true;
	}
};