/**
 * ExpansionQuestObjectiveTreasureHuntEvent.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionQuestObjectiveTreasureHuntEvent: ExpansionQuestObjectiveEventBase
{
	private ref array<Object> LootItems = new array<Object>;
	private UndergroundStash Stash;
	private SeaChest Chest;
	private vector StashPos;

	override void OnStart()
	{
	#ifdef EXPANSIONMODQUESTSDEBUG
		Print("ExpansionQuestObjectiveTreasureHuntEvent::OnStart - Start");
	#endif

		ExpansionQuestObjectiveTreasureHunt treasureHunt = GetObjectiveConfig().GetTreasureHunt();
		if (!treasureHunt)
			return;

		CreateTreasure(treasureHunt);

		super.OnStart();

	#ifdef EXPANSIONMODQUESTSDEBUG
		Print("ExpansionQuestObjectiveTreasureHuntEvent::OnStart - End");
	#endif
	}

	/*override void OnComplete()
	{
		super.OnComplete();
	}

	override void OnTurnIn()
	{
		super.OnTurnIn();
	}

	override void OnCleanup()
	{
		super.OnCleanup();
	}*/

	override void OnCancel()
	{
		foreach (Object obj: LootItems)
		{
			GetGame().ObjectDelete(obj);
		}

		GetGame().ObjectDelete(Chest);
		GetGame().ObjectDelete(Stash);

		super.OnCancel();
	}

	void CreateTreasure(ExpansionQuestObjectiveTreasureHunt treasureHunt)
	{
	#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
		Print("ExpansionQuestObjectiveTreasureHuntEvent::CreateTreasure - Start");
	#endif

		vector pos = treasureHunt.GetPositions().GetRandomElement();
		StashPos = pos;
		treasureHunt.SetSelectedPosition(StashPos);
		
		//! Create the underground stash and hide it
		if (!Class.CastTo(Stash, GetGame().CreateObjectEx("UndergroundStash", pos, ECE_PLACE_ON_SURFACE)))
			return;

		if (Stash)
		{
			Stash.PlaceOnGround();
		}
	#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
		Print("ExpansionQuestObjectiveTreasureHuntEvent::CreateTreasure - Spawned stash on pos: " + pos);
	#endif

		EntityAI stashEntity;
		if (!Class.CastTo(stashEntity, Stash))
			return;

		//! Spawn the chest in the underground stash
		PlayerBase questPlayer = PlayerBase.GetPlayerByUID(GetQuest().GetPlayerUID());

		Object chestObj = Spawn("SeaChest", 1, questPlayer, stashEntity, pos, Vector(0, 0, 0));
		if (!Class.CastTo(Chest, chestObj))
			return;

	#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
		Print("ExpansionQuestObjectiveTreasureHuntEvent::CreateTreasure - Spawned chest in stash!");
	#endif

		//! Spawn the loot in the chest
		EntityAI chestEntity;
		if (!Class.CastTo(chestEntity, Chest))
			return;

		for (int i = 0; i < treasureHunt.GetItems().Count(); i++)
		{
			string name = treasureHunt.GetItems().GetKey(i);
			int amount = treasureHunt.GetItems().GetElement(i);
			
		#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
			Print("ExpansionQuestObjectiveTreasureHuntEvent::CreateTreasure - Add item to chest: " + name + " x" + amount);
		#endif
			Object item = Spawn(name, amount, questPlayer, chestEntity, pos, Vector(0, 0, 0));

			LootItems.Insert(item);
		}

	#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
		Print("ExpansionQuestObjectiveTreasureHuntEvent::CreateTreasure - End");
	#endif
	}

	Object Spawn(string name, int amount, PlayerBase player, inout EntityAI parent, vector position, vector orientation)
	{
		Object obj = ExpansionItemSpawnHelper.SpawnOnParent(name, player, parent, amount);

		return obj;
	}

	override void OnContinue()
	{
		if (!GetQuest().GetPlayer())
		{
			GetQuest().SetPlayer();
			if (!GetQuest().GetPlayer())
				return;
		}

		ExpansionQuestObjectiveTreasureHunt treasureHunt = GetObjectiveConfig().GetTreasureHunt();
		if (!treasureHunt)
			return;

		CreateTreasure(treasureHunt);

		super.OnContinue();
	}

	override void OnUpdate(float timeslice)
	{
		if (!GetObjectiveConfig() || !GetQuest() || !IsInitialized())
			return;

		ExpansionQuestObjectiveTreasureHunt treasureHunt = GetObjectiveConfig().GetTreasureHunt();
		if (!treasureHunt)
			return;

		vector position = StashPos;
		float maxDistance = 5.0;
		float currentDistance;

		//! Set the position of the group member that has the shortest distance to the target location
		//! as our current position if the quest is a group quest.
		array<vector> groupMemberPos = new array<vector>;

		if (!GetQuest().IsGroupQuest())
		{
			PlayerBase questPlayer = PlayerBase.GetPlayerByUID(GetQuest().GetPlayerUID());
			vector playerPos = questPlayer.GetPosition();
			currentDistance = vector.Distance(playerPos, position);
		}
	#ifdef EXPANSIONMODGROUPS
		else
		{
			ExpansionPartyData group = GetQuest().GetGroup();
			if (!group)
				return;

			for (int i = 0; i < group.GetPlayers().Count(); i++)
			{
				ExpansionPartyPlayerData playerGroupData = group.GetPlayers()[i];
				if (!playerGroupData)
					continue;

				PlayerBase groupPlayer = PlayerBase.GetPlayerByUID(playerGroupData.GetID());
				if (!groupPlayer)
					continue;

				groupMemberPos.Insert(groupPlayer.GetPosition());
			}

			float smallestDistance;
			int posIndex;
			bool firstSet = false;
			for (int p = 0; p < groupMemberPos.Count(); p++)
			{
				vector pos = groupMemberPos[p];
				float dist = vector.Distance(pos, position);
				if (!firstSet)
				{
					smallestDistance = dist;
					posIndex = p;
					firstSet = true;
				}
				else if (firstSet && dist < smallestDistance)
				{
					smallestDistance = dist;
					posIndex = p;
				}
			}

			currentDistance = vector.Distance(groupMemberPos[posIndex], position);
		}
	#endif

		position[1] = GetGame().SurfaceY(position[0], position[2]);

		if (position != vector.Zero && currentDistance <= maxDistance)
		{
			SetCompleted(true);
		}
		else if (position != vector.Zero && currentDistance > maxDistance)
		{
			SetCompleted(false);
		}
	}

	vector GetPosition()
	{
		return StashPos;
	}
	
	override int GetObjectiveType()
	{
		return ExpansionQuestObjectiveType.TREASUREHUNT;
	}
};