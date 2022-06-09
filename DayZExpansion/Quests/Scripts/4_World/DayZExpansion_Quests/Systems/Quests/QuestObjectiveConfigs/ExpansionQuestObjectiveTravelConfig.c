/**
 * ExpansionQuestObjectiveTravelConfig.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionQuestObjectiveTravelConfig: ExpansionQuestObjectiveConfigBase
{
	protected vector Position = vector.Zero;
	protected float MaxDistance = 0;
	protected string MarkerName = string.Empty;

	void SetPosition(vector pos)
	{
		Position = pos;
	}

	override vector GetPosition()
	{
		return Position;
	}

	void SetMaxDistance(float max)
	{
		MaxDistance = max;
	}

	override float GetMaxDistance()
	{
		return MaxDistance;
	}

	void SetMarkerName(string name)
	{
		MarkerName = name;
	}

	override string GetMarkerName()
	{
		return MarkerName;
	}

	override void Save(string fileName)
	{
		JsonFileLoader<ExpansionQuestObjectiveTravelConfig>.JsonSaveFile(EXPANSION_QUESTS_OBJECTIVES_TRAVEL_FOLDER + fileName + ".JSON", this);
	}

	override void OnSend(ParamsWriteContext ctx)
	{
		super.OnSend(ctx);

		ctx.Write(Position);
		ctx.Write(MaxDistance);
		ctx.Write(MarkerName);
	}

	override bool OnRecieve(ParamsReadContext ctx)
	{
		if (!super.OnRecieve(ctx))
			return false;

		if (!ctx.Read(Position))
			return false;

		if (!ctx.Read(MaxDistance))
			return false;

		if (!ctx.Read(MarkerName))
			return false;

		return true;
	}
};