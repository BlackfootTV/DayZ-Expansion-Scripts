/**
 * ExpansionAnomaliesModule.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

[CF_RegisterModule(ExpansionAnomaliesModule)]
class ExpansionAnomaliesModule: CF_ModuleWorld
{
	protected static ExpansionAnomaliesModule s_ModuleInstance;

	protected static const int ANOMALY_SPAWN_DISTANCE = 250;
	protected const float UPDATE_TICK_TIME = 0.100;
	protected const int UPDATE_ANOMALIES_PER_TICK = 10;  //! Refresh up to 10 anomalies and amomaliycores every UPDATE_TICK_TIME interval
	protected const int SPAWN_ANOMALIES_PER_TICK = 5;  //! Spawn up to 5 anomalies every UPDATE_TICK_TIME interval

	static ref array<vector> m_AnomalyPositions;
	protected bool m_DynamicSpawned;
	protected bool m_StaticSpawned;

	protected typename m_LastNamalskEvent;

	protected bool m_EVRStormActive;
	protected bool m_EVRStormBlowout;
	protected bool m_SpawnAnomalies;
	protected bool m_StabilizedAnomalies;

	protected float m_UpdateQueueTimer;

	protected int m_Expansion_ProcessedAnomaliesCount;
	protected int m_Expansion_ProcessedCoresCount;

	protected int m_Expansion_ProcessedAnomaliesBlowouts;
	protected int m_Expansion_ProcessedCoreBlowouts;

	protected int m_Expansion_ProcessedAnomaliesStabilisations;
	protected int m_Expansion_ProcessedCoreStabilisations;

	protected ref array<ref ExpansionAnomalyStatic> m_StaticAnomalySpawns;
	protected ref array<ref ExpansionAnomalyDynamic> m_DynamicAnomalySpawns;

	protected static ScriptCaller s_EVRStormStartAnomaliesSC;
	protected static ScriptCaller s_EVRStormBlowoutAnomaliesSC;
	protected static ScriptCaller s_StablizeAnomaliesSC;

	protected static ScriptCaller s_EVRStormStartCoresSC;
	protected static ScriptCaller s_EVRStormBlowoutCoresSC;
	
	protected int m_DynamicAnomaliesToSpawn;
	protected int m_SpawnedDynamicAnomaliesCount;
	protected int m_SpawnedDynamicAnomaliesInTick;
	
	protected int m_StaticAnomaliesToSpawn;
	protected int m_SpawnedStaticAnomaliesCount;
	protected int m_SpawnedStaticAnomaliesInTick;

	void ExpansionAnomaliesModule()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		s_ModuleInstance = this;

		m_AnomalyPositions = new array<vector>;
		m_StaticAnomalySpawns = new array<ref ExpansionAnomalyStatic>;
		m_DynamicAnomalySpawns = new array<ref ExpansionAnomalyDynamic>;
	}

	override void OnInit()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		super.OnInit();

		EnableMissionLoaded();
		EnableUpdate();
	}

	override void OnMissionLoaded(Class sender, CF_EventArgs args)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		super.OnMissionLoaded(sender, args);

		//! Server only
		if (GetGame().IsServer() && GetGame().IsMultiplayer())
		{
			if (GetExpansionSettings().GetNamalskAdventure().EnableAnomalies)
			{
				m_DynamicAnomalySpawns = GetExpansionSettings().GetNamalskAdventure().DynamicAnomalies;
				for (int i = 0; i < m_DynamicAnomalySpawns.Count(); i++)
				{
					ExpansionAnomalyDynamic anomaylDynamic = m_DynamicAnomalySpawns[i];
					anomaylDynamic.GeneratePositions(ANOMALY_SPAWN_DISTANCE);
					m_DynamicAnomaliesToSpawn += anomaylDynamic.PositionsCount();
				}

				m_StaticAnomalySpawns = GetExpansionSettings().GetNamalskAdventure().StaticAnomalies;
				m_StaticAnomaliesToSpawn = m_StaticAnomalySpawns.Count();
				
				SpawnAnomalies();
			}
		}
	}

	//! @note: Spawns all configured anomalies from the NamalskAdventureSettings class.
	protected void SpawnAnomalies(bool isDynamicEvent = false)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		if (GetExpansionSettings().GetNamalskAdventure().EnableDynamic)
		{
			if (!m_DynamicSpawned && !isDynamicEvent && !GetExpansionSettings().GetNamalskAdventure().SpawnDynamicWithEVRStorms)
			{
				if (m_SpawnedDynamicAnomaliesCount >= m_DynamicAnomaliesToSpawn) 
				{
					m_DynamicSpawned = true;
					return;
				}
				
				if (m_DynamicAnomalySpawns && m_DynamicAnomalySpawns.Count() > 0) 
				{
					for (int i = m_DynamicAnomalySpawns.Count() - 1; i >= 0; i--)
					{
						ExpansionAnomalyDynamic dynamicSpawn = m_DynamicAnomalySpawns[i];
						if (!dynamicSpawn)
							continue;
						
						for (int j = dynamicSpawn.PositionsCount() - 1; j >= 0; j--)
						{
							SpawnAnomalyDynamic(dynamicSpawn, i);
							m_SpawnedDynamicAnomaliesInTick++;
							
							if (m_SpawnedDynamicAnomaliesInTick >= SPAWN_ANOMALIES_PER_TICK) 
							{
								m_SpawnedDynamicAnomaliesInTick = 0;
								break;
							}
						}
					}
				}
			}
		}

		if (GetExpansionSettings().GetNamalskAdventure().EnableStatic)
		{
			if (!m_StaticSpawned && !isDynamicEvent && !GetExpansionSettings().GetNamalskAdventure().SpawnStaticWithEVRStorms)
			{
				if (m_SpawnedStaticAnomaliesCount >= m_StaticAnomaliesToSpawn) 
				{
					m_StaticSpawned = true;
					return;
				}
				
				if (m_StaticAnomalySpawns && m_StaticAnomalySpawns.Count() > 0) 
				{
					for (int k = m_StaticAnomalySpawns.Count() - 1; k >= 0; k--)
					{
						ExpansionAnomalyStatic staticAnoamlyData = m_StaticAnomalySpawns[k];
						if (!staticAnoamlyData)
							continue;
						
						SpawnAnomalyStatic(staticAnoamlyData, k, isDynamicEvent);
						
						m_SpawnedStaticAnomaliesInTick++;
					
						if (m_SpawnedStaticAnomaliesInTick >= SPAWN_ANOMALIES_PER_TICK) 
						{
							m_SpawnedStaticAnomaliesInTick = 0;
							break;
						}
					}
				}
			}
		}
	}

	//! @note: Handles spawns of static anomaly spawns
	protected void SpawnAnomalyStatic(ExpansionAnomalyStatic anomaly, int index, bool isDynamicEvent = false)
	{
		string typeToSpawn = anomaly.AnomalyTypes.GetRandomElement();
	    Object entity = GetGame().CreateObjectEx(typeToSpawn, anomaly.CenterPosition, ECE_PLACE_ON_SURFACE, ECE_NOLIFETIME);
	    Expansion_Anomaly_Base anomalyObj = Expansion_Anomaly_Base.Cast(entity);
		if (!anomalyObj)
        {
           GetGame().ObjectDelete(entity);
           return;
        }

		if (anomaly.Loot && anomaly.Loot.Count() > 0)
		{
			anomalyObj.SetLoot(anomaly.Loot, anomaly.LootItemsMin, anomaly.LootItemsMax, anomaly.LootSpawnType);
			if (!isDynamicEvent && anomaly.LootSpawnType == ExpansionAnomalyLootSpawnType.STATIC || isDynamicEvent && anomaly.LootSpawnType == ExpansionAnomalyLootSpawnType.DYNAMIC)
				anomalyObj.SpawnLoot();
		}
		
		//! @note: We increase the survived EVR storms count for the static spawned anomalies on spawn so the OnEVRStormFinalBlowout method
		//! takes these anomalies into account for the anomaly implosion event gamble on the next EVR storm event that hits the server.
		anomalyObj.IncreaseEVRStormsCount();
        m_AnomalyPositions.Insert(anomaly.CenterPosition);
		
		m_SpawnedStaticAnomaliesCount++;
		m_StaticAnomalySpawns.RemoveOrdered(index);
	}

	//! @note: Handles spawns of dynamic anomaly spawns
	protected void SpawnAnomalyDynamic(ExpansionAnomalyDynamic dynamicSpawn, int index, bool isDynamic = false)
	{
		string typeToSpawn = dynamicSpawn.AnomalyTypes.GetRandomElement();
		int flags = ECE_PLACE_ON_SURFACE;
		if (dynamicSpawn.Persistance < ExpansionAnomalyPersistance.LIFETIME)
		    flags = ECE_NOLIFETIME;

		vector position = dynamicSpawn.GetNextPosition();
		Object entity = GetGame().CreateObjectEx(typeToSpawn, position, flags);
		Expansion_Anomaly_Base anomalyObj = Expansion_Anomaly_Base.Cast(entity);
		if (!anomalyObj)
		{
		    GetGame().ObjectDelete(entity);
		    return;
		}

		if (dynamicSpawn.Loot && dynamicSpawn.Loot.Count() > 0)
		{
		    anomalyObj.SetLoot(dynamicSpawn.Loot, dynamicSpawn.LootItemsMin, dynamicSpawn.LootItemsMax, dynamicSpawn.LootSpawnType);
		    if (!isDynamic && dynamicSpawn.LootSpawnType == ExpansionAnomalyLootSpawnType.STATIC || isDynamic && dynamicSpawn.LootSpawnType == ExpansionAnomalyLootSpawnType.DYNAMIC)
		        anomalyObj.SpawnLoot();
		}

		//! @note: We increase the survived EVR storms count for the dynamic spawned anomalies on spawn so the OnEVRStormFinalBlowout method
		//! takes these anomalies into account for the anomaly implosion event gamble on the next EVR storm event that hits the server.
		anomalyObj.IncreaseEVRStormsCount();
		m_AnomalyPositions.Insert(position);
		
		m_SpawnedDynamicAnomaliesCount++;
		
		if (dynamicSpawn.GetNextPosition() == vector.Zero)
			m_DynamicAnomalySpawns.RemoveOrdered(index);
	}

	void OnNamalskEventStart(typename eventType)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

	#ifdef DIAG
		ExpansionNotification(new StringLocaliser("NAMALSK EVENT STARTED"), new StringLocaliser("%1 EVENT STARTED", eventType.ToString()), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_INFO, 7, ExpansionNotificationType.TOAST).Create();
	#endif

		if (eventType == EVRStorm || eventType == EVRStormDeadly)
		{
			m_Expansion_ProcessedAnomaliesCount = 0;
			m_Expansion_ProcessedCoresCount = 0;
			m_Expansion_ProcessedAnomaliesBlowouts = 0;
			m_Expansion_ProcessedCoreBlowouts = 0;
			m_Expansion_ProcessedAnomaliesStabilisations = 0;
			m_Expansion_ProcessedCoreStabilisations = 0;

			m_EVRStormActive = true;
		}
	}

	void OnNamalskEventCancel(typename eventType)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

	#ifdef DIAG
		ExpansionNotification(new StringLocaliser("NAMALSK EVENT CANCELED"), new StringLocaliser("%1 EVENT CANCELED", eventType.ToString()), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_INFO, 7, ExpansionNotificationType.TOAST).Create();
	#endif

		m_LastNamalskEvent = eventType;
		if (eventType == EVRStorm || eventType == EVRStormDeadly)
			m_EVRStormActive = false;
	}

	//! @note: Handles events that should start when ever a EVR storm event starts.
	void OnEVRStormInitPhaseServer()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
	}

	//! @note: Handles events that should start when ever a EVR storm mid phase starts.
	void OnEVRStormMidPhaseServer()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
	}

	//! @note: Handles events that should start when ever the EVR storm final blowout starts.
	//! Modified version to process and loop thrue spawned anomaly entities in chuncks.
	void OnEVRStormFinalBlowout()
	{
	    auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		m_EVRStormBlowout = true;
	}

	//! @note: Handles events that should start when ever a EVR storm event end phase starts.
	void OnEVRStormEndPhaseServer()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		m_SpawnAnomalies = true;

		NamEventManager event_manager;
	    g_Script.CallFunction(GetGame().GetMission(), "GetNamEventManager", event_manager, null);

		m_LastNamalskEvent = event_manager.GetLastEventType();
		m_EVRStormActive = false;
		m_EVRStormBlowout = false;
	}

	//! @note: Handles events that should start when ever a EVR storm event ended.
	//! Gets called on every active player on the server.
	void OnEVRStormFinished(SurvivorBase player)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		//! PLACEHOLDER
	}

	//! @note: Condition check if a EVR storm is currently active.
	bool IsEVRStormActive()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

	#ifdef NAMALSK_SURVIVAL
	    NamEventManager event_manager;
	    g_Script.CallFunction(GetGame().GetMission(), "GetNamEventManager", event_manager, null);

	    if (!event_manager)
	        return false;

		EVRStorm storm = EVRStorm.Cast(event_manager.GetEvent(EVRStorm));
		EVRStormDeadly stormDeadly = EVRStormDeadly.Cast(event_manager.GetEvent(EVRStormDeadly));
		if (storm && storm.GetEVRStormPhase() > ExpansionEVRStormPhase.NONE || stormDeadly && stormDeadly.GetEVRStormPhase() > ExpansionEVRStormPhase.NONE)
	   		return true;
	#endif

		return false;
	}

	bool HasActiveLEHSSuit(PlayerBase player)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		int slot_id_lehsSuit = InventorySlots.GetSlotIdFromString("LEHS");
		dzn_lehs lehsSuit = dzn_lehs.Cast(player.GetInventory().FindAttachment(slot_id_lehsSuit));
		if (!lehsSuit || lehsSuit && lehsSuit.IsRuined())
		{
			ModuleDebugPrint("::OnEnterAnomalyServer - Return FALSE");
			return false;
		}

		ModuleDebugPrint("::OnEnterAnomalyServer - Player has LEHS suit: " + lehsSuit.ToString());

		int slot_id_Headgear = InventorySlots.GetSlotIdFromString("Headgear");
		dzn_lehs_helmet lehsHelmet = dzn_lehs_helmet.Cast(player.GetInventory().FindAttachment(slot_id_Headgear));
		if (!lehsHelmet || lehsHelmet && lehsHelmet.IsRuined())
		{
			ModuleDebugPrint("::OnEnterAnomalyServer - Return FALSE");
			return false;
		}

		ModuleDebugPrint("::OnEnterAnomalyServer - Player has LEHS helmet: " + lehsHelmet.ToString());
		ModuleDebugPrint("::OnEnterAnomalyServer - LEHS status: Visor up: " + lehsHelmet.IsVisorUp() + " | Has power: " + lehsHelmet.HasPower() + " | Pressurized: " + lehsHelmet.IsPressurized());

		if (lehsHelmet.IsVisorUp() || !lehsHelmet.HasPower() || !lehsHelmet.IsPressurized() || !lehsHelmet.HasCircuitBoard())
		{
			ModuleDebugPrint("::OnEnterAnomalyServer - Return FALSE");
			return false;
		}


		ModuleDebugPrint("::OnEnterAnomalyServer - Return TRUE");
		return true;
	}

	void ProcessCargoDamage(EntityAI parent, int minDmg, int maxDmg)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		ModuleDebugPrint("::ProcessCargoDamage - Entity: " + parent.GetType() + " | Entity parent: " + parent.GetParent().ToString());

		array<EntityAI> items = new array<EntityAI>;
		parent.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		for (int i = 0; i < items.Count(); i++)
		{
			EntityAI item = items[i];

			ModuleDebugPrint("::ProcessCargoDamage - Item: " + item.GetType() + " | Item parent: " + item.GetParent().ToString());
			if (item.IsInherited(SurvivorBase) || item.IsInherited(ExpansionAnomalyCoreProtectiveCase))
				continue;

			//! @note: Don't damage items in anomaly/evr protector cases.
			if (item.GetParent() && ExpansionAnomalyCoreProtectiveCase.Cast(item.GetParent()))
				continue;

			item.AddHealth("", "", Math.RandomFloatInclusive(minDmg, maxDmg));
		}
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
	    for (int i = 0; i < amount; i++)
	    {
	        position = center + Vector(Math.RandomFloat(-largeSquareSize / 2, largeSquareSize / 2), 0, Math.RandomFloat(-largeSquareSize / 2, largeSquareSize / 2));
	        position[1] = GetGame().SurfaceY(position[0], position[2]);

			if (position == vector.Zero)
				continue;

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

				for (int j = 0; j < positions.Count(); j++)
	            {
					vector prevPos = positions[j];
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

	void OnEVRStormStartAnomaly(Expansion_Anomaly_Base anomaly)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE);

		anomaly.SetAnomalyUnstable();
	}

	void OnEVRStormFinalBlowoutAnomaly(Expansion_Anomaly_Base anomaly)
    {
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE);

		if (anomaly.SurviviedEVRStormsCount() > 0)
		{
		   //! @note: We gamble if the the anomaly will impode this when it has already survived one EVR storm.
		   int gamble = Math.RandomInt(0, 2); //! Gamble if this anomaly will implode or not
		   if (gamble == 0)
		   {
				int randomTime = Math.RandomInt(0, 3);
				anomaly.SetAnomalyExplosion(randomTime);
		   }
		   else
		   {
				anomaly.IncreaseEVRStormsCount();
				anomaly.IncreaseLifetime();
		   }
		}
		//! @note: Anomaly just got spawned thru the EVR storm event so dont harm it for now.
		else
		{
			anomaly.IncreaseEVRStormsCount();
			anomaly.IncreaseLifetime();
		}
    }

	void OnStabilizeAnomaly(Expansion_Anomaly_Base anomaly)
	{
	    auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE);

		if (anomaly.GetAnomalyState() >= ExpansionAnomalyState.UNSTABLE)
		    anomaly.SetAnomalyStable();
	}

	void OnEVRStormStartCore(Expansion_AnomalyCore_Base anomalyCore)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE);

		anomalyCore.SetAnomalyCoreUnstable();
	}

	void OnEVRStormFinalBlowoutCore(Expansion_AnomalyCore_Base anomalyCore)
    {
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE);

		if (!anomalyCore.CanUnsatabilize())
			return;

		int gamble2 = Math.RandomInt(0, 2); //! Gamble if this core will explode or not if on player
		int randomTime2 = Math.RandomInt(1, 3);
		if (gamble2 == 0)
		{
			anomalyCore.ActivateAnomalyCore(randomTime2); //! Let the core explode after 1-3 seconds.
		}
		else
		{
			anomalyCore.SetAnomalyCoreStable(); //! Set the core state back to stable
		}
    }

	override void OnUpdate(Class sender, CF_EventArgs args)
	{
		super.OnUpdate(sender, args);

		if (!GetGame().IsServer())
			return;

		auto update = CF_EventUpdateArgs.Cast(args);
		m_UpdateQueueTimer += update.DeltaTime;
		if (m_UpdateQueueTimer >= UPDATE_TICK_TIME)
		{
			//! @note: Update events when m_EVRStormActive flag is set to true.
			if (m_EVRStormActive && m_StabilizedAnomalies && !m_EVRStormBlowout)
			{
				OnEVRStormActive();
			}
			//! @note: Update events when m_EVRStormBlowout flag is set to true.
			else if (m_EVRStormActive && m_EVRStormBlowout)
			{
				OnEVRStormBlowout();
			}
			//! @note: Update events when m_EVRStormActive flag is set to false.
			else if (!m_EVRStormActive && !m_StabilizedAnomalies && m_SpawnAnomalies)
			{
				OnIdle();
			}

			m_UpdateQueueTimer = 0;
		}
	}

	protected void OnEVRStormActive()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		if (m_Expansion_ProcessedAnomaliesCount < Expansion_Anomaly_Base.s_Expansion_AllAnomalies.m_Count)
		{
			if (!s_EVRStormStartAnomaliesSC)
	    		s_EVRStormStartAnomaliesSC = ScriptCaller.Create(OnEVRStormStartAnomaly);

			m_Expansion_ProcessedAnomaliesCount += Expansion_Anomaly_Base.s_Expansion_AllAnomalies.Each(s_EVRStormStartAnomaliesSC, UPDATE_ANOMALIES_PER_TICK);
		}

		if (m_Expansion_ProcessedCoresCount < Expansion_AnomalyCore_Base.s_Expansion_AllAnomalyCores.m_Count)
		{
			if (!s_EVRStormStartCoresSC)
				s_EVRStormStartCoresSC = ScriptCaller.Create(OnEVRStormStartCore);

			m_Expansion_ProcessedCoresCount += Expansion_AnomalyCore_Base.s_Expansion_AllAnomalyCores.Each(s_EVRStormStartCoresSC, UPDATE_ANOMALIES_PER_TICK);
		}
		
		if (m_Expansion_ProcessedAnomaliesCount == Expansion_Anomaly_Base.s_Expansion_AllAnomalies.m_Count && m_Expansion_ProcessedCoresCount == Expansion_AnomalyCore_Base.s_Expansion_AllAnomalyCores.m_Count)
			m_StabilizedAnomalies = false;
	}

	protected void OnEVRStormBlowout()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		if (m_Expansion_ProcessedAnomaliesBlowouts < Expansion_Anomaly_Base.s_Expansion_AllAnomalies.m_Count)
		{
			if (!s_EVRStormBlowoutAnomaliesSC)
  				s_EVRStormBlowoutAnomaliesSC = ScriptCaller.Create(OnEVRStormFinalBlowoutAnomaly);

			m_Expansion_ProcessedAnomaliesBlowouts += Expansion_Anomaly_Base.s_Expansion_AllAnomalies.Each(s_EVRStormBlowoutAnomaliesSC, UPDATE_ANOMALIES_PER_TICK);
		}

		if (m_Expansion_ProcessedCoreBlowouts < Expansion_AnomalyCore_Base.s_Expansion_AllAnomalyCores.m_Count)
		{
			if (!s_EVRStormBlowoutCoresSC)
  				s_EVRStormBlowoutCoresSC = ScriptCaller.Create(OnEVRStormFinalBlowoutCore);

			m_Expansion_ProcessedCoreBlowouts += Expansion_AnomalyCore_Base.s_Expansion_AllAnomalyCores.Each(s_EVRStormBlowoutCoresSC, UPDATE_ANOMALIES_PER_TICK);
		}
	}

	protected void OnIdle()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		if (!m_StabilizedAnomalies)
		{
			if (m_Expansion_ProcessedAnomaliesStabilisations < Expansion_Anomaly_Base.s_Expansion_AllAnomalies.m_Count)
			{
				if (!s_StablizeAnomaliesSC)
	  				s_StablizeAnomaliesSC = ScriptCaller.Create(OnStabilizeAnomaly);
	
				m_Expansion_ProcessedAnomaliesStabilisations += Expansion_Anomaly_Base.s_Expansion_AllAnomalies.Each(s_StablizeAnomaliesSC, UPDATE_ANOMALIES_PER_TICK);
			}
			
			if (m_Expansion_ProcessedAnomaliesStabilisations == Expansion_Anomaly_Base.s_Expansion_AllAnomalies.m_Count)
				m_StabilizedAnomalies = true;
		}

		//! @note: Update events when m_SpawnAnomalies flag is set to true.
		if (m_SpawnAnomalies)
		{
			OnSpawnAnomalies();
		}
	}

	protected void OnSpawnAnomalies()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		if (!m_DynamicSpawned)
		{
			if (m_SpawnedDynamicAnomaliesCount >= m_DynamicAnomaliesToSpawn) 
			{
				m_DynamicSpawned = true;
				return;
			}
			
			if (m_DynamicAnomalySpawns && m_DynamicAnomalySpawns.Count() > 0) 
			{
				for (int i = m_DynamicAnomalySpawns.Count() - 1; i >= 0; i--)
				{
					ExpansionAnomalyDynamic dynamicSpawn = m_DynamicAnomalySpawns[i];
					if (!dynamicSpawn)
						continue;
					
					for (int j = dynamicSpawn.PositionsCount() - 1; j >= 0; j--)
					{
						SpawnAnomalyDynamic(dynamicSpawn, i, true);
						m_SpawnedDynamicAnomaliesInTick++;
						
						if (m_SpawnedDynamicAnomaliesInTick >= SPAWN_ANOMALIES_PER_TICK) 
						{
							m_SpawnedDynamicAnomaliesInTick = 0;
							break;
						}
					}
				}
			}
		}

		if (!m_StaticSpawned)
		{
			if (m_SpawnedStaticAnomaliesCount >= m_StaticAnomaliesToSpawn) 
			{
				m_StaticSpawned = true;
				return;
			}
			
			if (m_StaticAnomalySpawns && m_StaticAnomalySpawns.Count() > 0) 
			{
				for (int k = m_StaticAnomalySpawns.Count() - 1; k >= 0; k--)
				{
					ExpansionAnomalyStatic staticAnoamlyData = m_StaticAnomalySpawns[k];
					if (!staticAnoamlyData)
						continue;
					
					SpawnAnomalyStatic(staticAnoamlyData, k, true);
					
					m_SpawnedStaticAnomaliesInTick++;
				
					if (m_SpawnedStaticAnomaliesInTick >= SPAWN_ANOMALIES_PER_TICK) 
					{
						m_SpawnedStaticAnomaliesInTick = 0;
						break;
					}
				}
			}
		}
		
		m_SpawnAnomalies = false;
	}

	static ExpansionAnomaliesModule GetModuleInstance()
	{
		return s_ModuleInstance;
	}

	void ModuleDebugPrint(string text)
	{
	#ifdef EXPANSION_NAMALSK_ADVENTURE_DEBUG
		EXTrace.Print(EXTrace.NAMALSKADVENTURE, this, text);
	#endif
	}
};