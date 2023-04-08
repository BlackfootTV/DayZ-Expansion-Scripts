/**
 * ExpansionAnomalyData.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

enum ExpansionAnomalyPersistance
{
	NONE = 0,
	LIFETIME = 1
};

enum ExpansionAnomalyLootSpawnType
{
	STATIC = 0,
	DYNAMIC = 1
};

class ExpansionAnomalyStatic
{
	ref array<string> AnomalyTypes = new array<string>;
	vector CenterPosition;
	ref array <ref ExpansionLoot> Loot;
	int LootItemsMin = 1;
	int LootItemsMax = 1;
	ExpansionAnomalyLootSpawnType LootSpawnType = ExpansionAnomalyLootSpawnType.STATIC;
	
	void ExpansionAnomalyStatic(array<string> anomalyTypes, vector center)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		AnomalyTypes = anomalyTypes;
		CenterPosition = center;
	}
	
	void SetLoot(array <ref ExpansionLoot> loot, int min, int max, ExpansionAnomalyLootSpawnType lootType)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		Loot = new array<ref ExpansionLoot>;
		Loot = loot;
		
		LootItemsMin = min;
		LootItemsMax = max;
		LootSpawnType = lootType;
	}
};

class ExpansionAnomalyDynamic: ExpansionAnomalyStatic
{
	float SquareSize;
	int Amount;
	ExpansionAnomalyPersistance Persistance;

	[NonSerialized()];
	private ref array<vector> m_Positions;
	[NonSerialized()];
    private int m_CurrentPositionIndex;
	
	void ExpansionAnomalyDynamic(array<string> anomalyTypes, vector center, float squareSize = 500, int amount = 1, ExpansionAnomalyPersistance persistance = ExpansionAnomalyPersistance.NONE)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		AnomalyTypes = anomalyTypes;
		CenterPosition = center;
		SquareSize = squareSize;
		Amount = amount;
		Persistance = persistance;
	}
	
	void GeneratePositions(float distance)
    {
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
        m_Positions = new array<vector>;
        m_Positions = GeneratePositions(CenterPosition, SquareSize, Amount, distance);
        m_CurrentPositionIndex = 0;
    }
	
	//! @note: In this modified version of the GeneratePositions method, a larger square area is generated using a for loop with a fixed number of iterations equal to amount * 2.
	//! Valid positions within the larger area are then selected randomly until the desired number of positions is reached or there are no more valid positions left.
	//! This reduces the number of collision checks and distance calculations required and should improve server performance.
	static array<vector> GeneratePositions(vector center, float squareSize, int amount, int distanceToPos = 0)
	{
	    auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE);

	    array<vector> positions = new array<vector>;
		vector position;

	    //! Generate positions within a larger square area
	    float largeSquareSize = squareSize * 2;
	    array<vector> largePositions = new array<vector>;
	    for (int i = 0; i < amount * 2; i++)
	    {
	        position = center + Vector(Math.RandomFloat(-largeSquareSize / 2, largeSquareSize / 2), 0, Math.RandomFloat(-largeSquareSize / 2, largeSquareSize / 2));
	        position[1] = GetGame().SurfaceY(position[0], position[2]);

	        if (!GetGame().SurfaceIsSea(position[0], position[2]) && !GetGame().SurfaceIsPond(position[0], position[2]))
	            largePositions.Insert(position);
	    }

	    //! Select valid positions from the larger area
	    while (positions.Count() < amount && largePositions.Count() > 0)
	    {
	        int index = Math.RandomInt(0, largePositions.Count() - 1);
	        position = largePositions[index];
	        largePositions.Remove(index);

	        array<Object> excludes;
	        if (!GetGame().IsBoxColliding(position, Vector(4, 1, 4), Vector(5, 0, 5), excludes))
	        {
	            bool validPos = true;
	            foreach (vector prevPos: positions)
	            {
					if (position == vector.Zero)
					{
	                    validPos = false;
	                    break;
	                }
					
					if (distanceToPos > 0)
					{
		                if (vector.Distance(position, prevPos) < distanceToPos)
		                {
		                    validPos = false;
		                    break;
		                }
					}
	            }

	            if (validPos)
	                positions.Insert(position);
	        }
	    }

	    return positions;
	}

    bool HasNextPosition()
    {
        return m_CurrentPositionIndex < m_Positions.Count();
    }

    vector GetNextPosition()
    {
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
        if (!HasNextPosition())
            return vector.Zero;

        vector position = m_Positions[m_CurrentPositionIndex];
        m_CurrentPositionIndex++;
        return position;
    }
	
	int PositionsCount()
	{
		return m_Positions.Count();
	}
	
	int GetCurrentPositionIndex()
	{
		return m_CurrentPositionIndex;
	}
};