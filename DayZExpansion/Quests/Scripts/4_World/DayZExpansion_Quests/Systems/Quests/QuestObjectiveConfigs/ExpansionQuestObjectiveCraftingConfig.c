/**
 * ExpansionQuestObjectiveCraftingConfig.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionQuestObjectiveCraftingConfigBase: ExpansionQuestObjectiveConfig
{
	autoptr array<string> ItemNames = new array<string>;
};

class ExpansionQuestObjectiveCraftingConfig: ExpansionQuestObjectiveCraftingConfigBase
{
	int ExecutionAmount = 1;

	void AddItemName(string name)
	{
		ItemNames.Insert(name);
	}

	override array<string> GetItemNames()
	{
		return ItemNames;
	}

	void SetExecutionAmount(int amount)
	{
		ExecutionAmount = amount;
	}

	override int GetExecutionAmount()
	{
		return ExecutionAmount;
	}

	static ExpansionQuestObjectiveCraftingConfig Load(string fileName)
	{
		bool save;
		Print("[ExpansionQuestObjectiveCraftingConfig] Load existing configuration file:" + EXPANSION_QUESTS_OBJECTIVES_CRAFTING_FOLDER + fileName);

		ExpansionQuestObjectiveCraftingConfig config;
		ExpansionQuestObjectiveCraftingConfigBase configBase;

		if (!ExpansionJsonFileParser<ExpansionQuestObjectiveCraftingConfigBase>.Load(EXPANSION_QUESTS_OBJECTIVES_CRAFTING_FOLDER + fileName, configBase))
			return NULL;

		if (configBase.ConfigVersion < CONFIGVERSION)
		{
			Print("[ExpansionQuestObjectiveCraftingConfig] Convert existing configuration file:" + EXPANSION_QUESTS_OBJECTIVES_CRAFTING_FOLDER + fileName + " to version " + CONFIGVERSION);
			config = new ExpansionQuestObjectiveCraftingConfig();
			
			//! Copy over old configuration that haven't changed
			config.CopyConfig(configBase);

			if (configBase.ConfigVersion < 12)
			{
				config.ExecutionAmount = 1;
			}

			config.ConfigVersion = CONFIGVERSION;
			save = true;
		}
		else
		{
			if (!ExpansionJsonFileParser<ExpansionQuestObjectiveCraftingConfig>.Load(EXPANSION_QUESTS_OBJECTIVES_CRAFTING_FOLDER + fileName, config))
				return NULL;
		}

		if (save)
		{
			config.Save(fileName);
		}

		return config;
	}
	
	override void Save(string fileName)
	{
		Print(ToString() + "::Save - FileName: " + EXPANSION_QUESTS_OBJECTIVES_CRAFTING_FOLDER + fileName);
		if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
			fileName += ".json";
		
		ExpansionJsonFileParser<ExpansionQuestObjectiveCraftingConfig>.Save(EXPANSION_QUESTS_OBJECTIVES_CRAFTING_FOLDER + fileName, this);
	}

	void CopyConfig(ExpansionQuestObjectiveCraftingConfigBase configBase)
	{
		ID = configBase.ID;
		ObjectiveType = configBase.ObjectiveType;
		ObjectiveText = configBase.ObjectiveText;
		TimeLimit = configBase.TimeLimit;

		ItemNames = configBase.ItemNames;
	}

	override void OnSend(ParamsWriteContext ctx)
	{
		super.OnSend(ctx);
	}

	override bool OnRecieve(ParamsReadContext ctx)
	{
		if (!super.OnRecieve(ctx))
			return false;

		return true;
	}

	override bool Validate()
	{
		if (!super.Validate())
			return false;

		if (!ItemNames || ItemNames && ItemNames.Count() == 0)
			return false;

		return true;
	}
};