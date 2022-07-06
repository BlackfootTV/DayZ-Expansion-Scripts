/**
 * ExpansionAIPatrolBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionAISpawnBase
{
	string Faction;                     // Raiders, Mercenaries, West, East, Guards, Civilian, Passive
	string LoadoutFile;                 // a json file containing the loadout of this team - if empty, will use the default loadout of the faction
	int NumberOfAI;                     // How many bots, -x will make it random between 0 and x
	string Behaviour;                   // See eAIWaypointBehavior
	string Speed;                       // See eAIMovementSpeed
	string UnderThreatSpeed;            // 
	bool CanBeLooted;                   // if enabled, the bots can be looted by the players
	bool UnlimitedReload;               // should bots be able to reload indefinitely (still needs spare mag in inventory!)

	void ExpansionAISpawnBase(int bod = 1, string spd = "JOG", string threatspd = "SPRINT", string beh = "ALTERNATE", string fac = "WEST", string loa = "", bool canbelooted = true, bool unlimitedreload = false)
	{
		NumberOfAI = bod;
		Speed = spd;
		UnderThreatSpeed = threatspd;
		Behaviour = beh;
		Faction = fac;
		LoadoutFile = loa;
		CanBeLooted = canbelooted;
		UnlimitedReload = unlimitedreload;
	}

	float GetSpeed()
	{
		return typename.StringToEnum(eAIMovementSpeed, Speed);
	}

	float GetThreatSpeed()
	{
		return typename.StringToEnum(eAIMovementSpeed, UnderThreatSpeed);
	}

	int GetBehaviour()
	{
		return typename.StringToEnum(eAIWaypointBehavior, Behaviour);
	}

	void UpdateSettings()
	{
		//! v1/v2 to v3
		switch (Faction)
		{
			case "INSURGENT":
				Faction = "Raiders";
				break;
			case "WEST":
			case "EAST":
			case "RAIDERS":
			case "CIVILIAN":
			case "GUARDS":
			case "PASSIVE":
				string lowerpart = Faction.Substring(1, Faction.Length() - 1);
				lowerpart.ToLower();
				Faction = Faction[0] + lowerpart;
				break;
		}

		//! v1/v2/v3 to v4
		switch (Behaviour)
		{
			case "HOLD":
				Behaviour = "HALT";
				break;
			case "PATROL":
			case "REVERSE":
				Behaviour = "ALTERNATE";
				break;
			case "HOLD OR PATROL":
				Behaviour = "HALT_OR_ALTERNATE";
				break;
		}

		if (Speed == "RANDOM")
			Speed = "RANDOM_NONSTATIC";

		if (UnderThreatSpeed == "RANDOM")
			UnderThreatSpeed = "RANDOM_NONSTATIC";
	}
};

class ExpansionAIDynamicSpawnBase: ExpansionAISpawnBase
{
	float MinDistRadius;	            // If the player is closer than MinDistRadius from the spawn point, the patrol won't spawn, if set to -2, will use the general setting instead
	float MaxDistRadius;	            // Same but if the player is further away than MaxDistRadius, the bots won't spawn, if set to -2, will use the general setting instead
	float DespawnRadius;
	float MinSpreadRadius;
	float MaxSpreadRadius;
	float Chance;                       // chance for this patrol to spawn

	void ExpansionAIDynamicSpawnBase(int bod = 1, string spd = "JOG", string threatspd = "SPRINT", string beh = "ALTERNATE", string fac = "WEST", string loa = "", bool canbelooted = true, bool unlimitedreload = false, float chance = 1.0, float mindistradius = -2, float maxdistradius = -2)
	{
		Chance = chance;
		MinDistRadius = mindistradius;
		MaxDistRadius = maxdistradius;
		DespawnRadius = maxdistradius * 1.1;
	}
};