modded class DayZPlayerImplement
{
#ifdef DIAG
	static int DEBUG_EXPANSION_AI_CLIMB;
	static bool DEBUG_EXPANSION_AI_VEHICLE;
#endif
	
	private autoptr eAITargetInformation m_TargetInformation;

	private eAIGroup m_eAI_Group;
	private eAIGroup m_Expansion_FormerGroup;
	protected typename m_eAI_FactionType;
	private int m_eAI_GroupID;
	private int m_eAI_FactionTypeID;
	private int m_eAI_FactionTypeIDSynch;
	private int m_eAI_GroupMemberIndex;
	private int m_eAI_GroupMemberIndexSynch;

	private bool m_eAI_IsPassive;

	float m_eAI_LastAggressionTime;
	float m_eAI_LastAggressionTimeout;

	float m_eAI_LastHitTime;

	ref eAIFirearmEffectsData m_eAI_FirearmEffectsData;

#ifdef DIAG
#ifndef SERVER
	autoptr array<Shape> m_Expansion_DebugShapes = new array<Shape>();
#endif
#endif

	void DayZPlayerImplement()
	{
#ifdef DIAG
		auto trace = CF_Trace_0(EXTrace.AI, this);
#endif

		m_TargetInformation = CreateTargetInformation();
	}

	override void Expansion_Init()
	{
#ifdef DIAG
		auto trace = CF_Trace_0(EXTrace.AI, this);
#endif

		super.Expansion_Init();

		RegisterNetSyncVariableInt("m_eAI_GroupID", -1, int.MAX);
		RegisterNetSyncVariableInt("m_eAI_FactionTypeIDSynch");
		RegisterNetSyncVariableInt("m_eAI_GroupMemberIndexSynch", 0, 0xffff);

		m_eAI_GroupID = -1;
		m_eAI_FactionTypeID = -1;

		if (GetGame().IsServer() && m_eAI_FactionType)
		{
			//! @note w/o the cast to eAIFaction, the compiler warns about unsafe downcasting.
			//! Of course the compiler is wrong, because we're casting up, not down, so this cast here is just there to satisfy compiler shortcomings.
			//! Yes I wrote this comment for the sole reason that I'm annoyed by this.
			SetGroup(eAIGroup.CreateGroup(eAIFaction.Cast(m_eAI_FactionType.Spawn())));
		}
	}

	//! Vanilla, can this AI be targeted by Zs/Animals?
	override bool CanBeTargetedByAI(EntityAI ai)
	{
		if (!super.CanBeTargetedByAI(ai))
			return false;

		if (GetGroup())
			return !GetGroup().GetFaction().IsFriendly(ai);

		return true;
	}

	protected eAITargetInformation CreateTargetInformation()
	{
#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "CreateTargetInformation");
#endif

		return new eAIPlayerTargetInformation(this);
	}

	eAITargetInformation GetTargetInformation()
	{
#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "GetTargetInformation");
#endif

		return m_TargetInformation;
	}

	bool IsAI()
	{
#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "IsAI");
#endif

		return false;
	}

	void SetGroup(eAIGroup group, bool autoDeleteFormerGroupIfEmpty = true)
	{
#ifdef DIAG
		auto trace = CF_Trace_2(EXTrace.AI, this).Add(group).Add(autoDeleteFormerGroupIfEmpty);
#endif

		if (m_eAI_Group == group)
			return;

		if (m_eAI_Group)
		{
			m_eAI_Group.RemoveMember(this, autoDeleteFormerGroupIfEmpty);

			m_eAI_GroupID = -1;

			EXTrace.Print(EXTrace.AI, this, "Current AI group: " + m_eAI_Group);
			if (!autoDeleteFormerGroupIfEmpty)
				m_Expansion_FormerGroup = m_eAI_Group;
		}

		EXTrace.Print(EXTrace.AI, this, "Setting AI group: " + group);
		m_eAI_Group = group;

		if (m_eAI_Group)
		{
			m_eAI_GroupID = m_eAI_Group.GetID();
			eAI_SetFactionTypeID(m_eAI_Group.GetFaction().GetTypeID());

			SetGroupMemberIndex(m_eAI_Group.AddMember(this));
			EXTrace.Print(EXTrace.AI, this, "Group ID: " + m_eAI_GroupID);
		}
		else
		{
			eAI_SetFactionTypeID(-1);
		}

		if (GetGame().IsDedicatedServer())
			SetSynchDirty();
	}

	eAIGroup GetGroup()
	{
#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "GetGroup");
#endif

		return m_eAI_Group;
	}

	void Expansion_SetFormerGroup(eAIGroup group)
	{
		m_Expansion_FormerGroup = group;
	}

	eAIGroup Expansion_GetFormerGroup()
	{
		return m_Expansion_FormerGroup;
	}

	int GetGroupID()
	{
#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "GetGroupID");
#endif

		return m_eAI_GroupID;
	}

	void SetGroupMemberIndex(int index)
	{
#ifdef EAI_TRACE
		auto trace = CF_Trace_1(this, "SetGroupMemberIndex").Add(index);
#endif

		m_eAI_GroupMemberIndex = index;
		m_eAI_GroupMemberIndexSynch = index;

		if (GetGame().IsDedicatedServer())
			SetSynchDirty();
	}

	void eAI_SetFactionTypeID(int id)
	{
#ifdef DIAG
		auto trace = CF_Trace_1(EXTrace.AI, this).Add(id);
#endif
		
#ifdef EXPANSIONMODHARDLINE
		if (GetGame().IsServer() && GetIdentity())
		{
			auto hardlineSettings = GetExpansionSettings().GetHardline();
			if (hardlineSettings.UseReputation && hardlineSettings.UseFactionReputation)
				ExpansionHardlineModule.GetModuleInstance().OnFactionChange(this, m_eAI_FactionTypeID, id);
		}
#endif
		m_eAI_FactionTypeID = id;
		m_eAI_FactionTypeIDSynch = id;

		if (GetGame().IsDedicatedServer())
			SetSynchDirty();
	}

	int eAI_GetFactionTypeID()
	{
		return m_eAI_FactionTypeID;
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if (m_eAI_GroupID == -1 && m_eAI_Group)
		{
			//! @note this is only ever to be used for players, not AI! AI may NEVER not have a group!

			EXTrace.Print(EXTrace.AI, this, "left group ID " + m_eAI_Group.GetID());

			m_eAI_Group.RemoveMember(this);

			m_eAI_Group = null;

			m_eAI_FactionTypeID = -1;  //! Make sure faction is updated if necessary
		}
		else if (m_eAI_Group && m_eAI_Group.GetID() != m_eAI_GroupID)
		{
			EXTrace.Print(EXTrace.AI, this, "moved from group ID " + m_eAI_Group.GetID() + " -> " + m_eAI_GroupID);

			m_eAI_Group.RemoveMember(this);

			m_eAI_Group = eAIGroup.GetGroupByID(m_eAI_GroupID, true);

			m_eAI_Group.Client_SetMemberIndex(this, m_eAI_GroupMemberIndexSynch);

			m_eAI_FactionTypeID = -1;  //! Make sure faction is updated if necessary
		}
		else if (m_eAI_GroupID != -1 && !m_eAI_Group)
		{
			EXTrace.Print(EXTrace.AI, this, "joined group ID " + m_eAI_GroupID);

			m_eAI_Group = eAIGroup.GetGroupByID(m_eAI_GroupID, true);

			m_eAI_Group.Client_SetMemberIndex(this, m_eAI_GroupMemberIndexSynch);

			m_eAI_FactionTypeID = -1;  //! Make sure faction is updated if necessary
		}
		else if (m_eAI_Group && m_eAI_GroupMemberIndexSynch != m_eAI_GroupMemberIndex)
		{
			EXTrace.Print(EXTrace.AI, this, "moved within group, member index " + m_eAI_GroupMemberIndex + " -> " + m_eAI_GroupMemberIndexSynch);

			// @note: this has to be the last check as when moving/joining groups
			// the index is out of synch and will be handled in the above checks

			m_eAI_Group.Client_SetMemberIndex(this, m_eAI_GroupMemberIndexSynch);
		}

		if (m_eAI_Group && m_eAI_GroupMemberIndexSynch != m_eAI_GroupMemberIndex)
			m_eAI_GroupMemberIndex = m_eAI_GroupMemberIndexSynch;

		if (m_eAI_Group && m_eAI_FactionTypeID != m_eAI_FactionTypeIDSynch)
		{
			EXTrace.Print(EXTrace.AI, this, "changing faction ID from " + m_eAI_FactionTypeID + " -> " + m_eAI_FactionTypeIDSynch);

			m_eAI_FactionTypeID = m_eAI_FactionTypeIDSynch;
	
			typename factionType = eAIFaction.GetTypeByID(m_eAI_FactionTypeIDSynch);

			if (factionType && m_eAI_Group.GetFaction().Type() != factionType)
			{
				EXTrace.Print(EXTrace.AI, this, "changing faction from " + m_eAI_Group.GetFaction().Type() + " -> " + factionType);

				auto faction = eAIFaction.Cast(factionType.Spawn());
				if (faction)
					m_eAI_Group.SetFaction(faction);
			}
		}
	}

	void eAI_SetPassive(bool state = true)
	{
		m_eAI_IsPassive = state;
	}

	bool eAI_IsPassive()
	{
		if (m_eAI_IsPassive)
			return true;
		if (m_eAI_Group)
			return m_eAI_Group.GetFaction().IsPassive();
		return false;
	}

	bool eAI_IsSideSteppingObstacles()
	{
		return false;
	}

	override bool Expansion_CanBeDamaged(string ammo = string.Empty)
	{
		if (!super.Expansion_CanBeDamaged(ammo))
			return false;

		if (m_eAI_Group)
			return !m_eAI_Group.GetFaction().IsInvincible();
			
		return true;
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
	#ifdef DIAG
		EXTrace.PrintHit(EXTrace.AI, this, "EEOnDamageCalculated", damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	#endif

		DayZPlayerImplement player;
		if (Class.CastTo(player, source.GetHierarchyRootPlayer()) && player != this)
			player.m_eAI_LastAggressionTime = ExpansionStatic.GetTime(true);  //! Aggro guards in area (if any)

		if (damageType == DT_FIRE_ARM && player && player.IsAI() && player == this)
		{
			//! This shouldn't be possible because AI don't use suicide emote
			EXPrint(this, "WARNING: Game encountered an impossible state (AI damage source is firearm in AI's own hands)");
			return false;
		}

		return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	}

	//! Suppress "couldn't kill player" in server logs when AI gets killed
	Hive GetHive()
	{
		#ifdef DIAG
		auto trace = CF_Trace_0(EXTrace.AI, this);
		#endif

		if (IsAI())
			return null;

		return Expansion_GlobalGetHive();
	}

	override void EEKilled(Object killer)
	{
		m_TargetInformation.OnDeath();

		super.EEKilled(killer);
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
	#ifdef DIAG
		EXTrace.PrintHit(EXTrace.AI, this, "EEHitBy", damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	#endif

		eAIBase ai;
		if (damageType == DT_FIRE_ARM && Class.CastTo(ai, source.GetHierarchyRootPlayer()) && ai == this)
		{
			//! This shouldn't be possible because AI don't use suicide emote
			EXPrint(this, "WARNING: Game encountered an impossible state (AI damage source is firearm in AI's own hands)");
			return;
		}

		m_eAI_LastHitTime = GetGame().GetTickTime();

		m_TargetInformation.OnHit();

		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	}

	//! @note unfortunately vanilla docs are correct and this doesn't get called when AI hits something :-(
	/*
	override void EEHitByRemote(int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos)
	{
	#ifdef DIAG
		EXTrace.PrintHit(EXTrace.AI, this, "EEHitByRemote", null, damageType, source, component, dmgZone, ammo, modelPos, 0.0);
	#endif

		super.EEHitByRemote(damageType, source, component, dmgZone, ammo, modelPos);

		if (g_Game.IsClient() && damageType == DT_FIRE_ARM)
		{
			//! MP client only!
			eAIBase ai;
			if (Class.CastTo(ai, source.GetHierarchyRootPlayer()) && m_eAI_FirearmEffectsData)
				eAI_HandleFirearmEffects();
		}
	}
	*/

	override bool HandleDamageHit(int pCurrentCommandID)
	{
		if (g_Game.IsClient() && m_eAI_FirearmEffectsData)
		{
			//! MP client only!
			foreach (SyncHitInfo data: m_SyncedHitDataArray)
			{
				if (data.m_AnimType == 0)
				{
					//! Firearm or explosive, potentially custom
					eAI_HandleFirearmEffects();
					break;
				}
			}
		}

		return super.HandleDamageHit(pCurrentCommandID);
	}

	void eAI_SetFirearmEffectsOnHit(EntityAI source, float timeOut, int componentIndex, vector pos, vector surfNormal, vector inSpeed, string ammoType)
	{
		//! MP client only!
		m_eAI_FirearmEffectsData = new eAIFirearmEffectsData(source, timeOut);
		m_eAI_FirearmEffectsData.m_ComponentIndex 	= componentIndex;
		m_eAI_FirearmEffectsData.m_Position		= pos;
		m_eAI_FirearmEffectsData.m_SurfaceNormal	= surfNormal;
		m_eAI_FirearmEffectsData.m_InSpeed			= inSpeed;
		m_eAI_FirearmEffectsData.m_AmmoType		= ammoType;
	}

	void eAI_RemoveFirearmEffectsOnHit(EntityAI source)
	{
		if (m_eAI_FirearmEffectsData && m_eAI_FirearmEffectsData.m_Source == source)
			m_eAI_FirearmEffectsData = null;
	}

	//! MP client only!
	void eAI_HandleFirearmEffects()
	{
		eAIFirearmEffectsData data = m_eAI_FirearmEffectsData;

		bool valid;
		if (data.m_TimeOut >= g_Game.GetTickTime())
			valid = true;

	#ifdef DIAG
		EXTrace.Print(EXTrace.AI, this, "::eAI_HandleFirearmEffects " + data.m_Source + " valid " + valid);
	#endif

		if (valid)
		{
			GetDayZGame().eAI_HandleFirearmEffects(data.m_Source, this, data.m_ComponentIndex, "Hit_MeatBones", data.m_Position, data.m_SurfaceNormal, data.m_InSpeed, data.m_AmmoType);
		}

		m_eAI_FirearmEffectsData = null;
	}

	bool eAI_UpdateAgressionTimeout(float timeThreshold)
	{
		if (!m_eAI_LastAggressionTime)
			return false;

		float time = ExpansionStatic.GetTime(true);
		float timeout = timeThreshold - (time - m_eAI_LastAggressionTime);
		bool active;
		if (timeout > 0)
			active = true;

		if (active && time + timeout > m_eAI_LastAggressionTimeout)
		{
			m_eAI_LastAggressionTimeout = time + timeout;
			SetSynchDirty();
		}

		return active;
	}

	float eAI_GetLastAggressionCooldown()
	{
		float cooldown = m_eAI_LastAggressionTimeout - ExpansionStatic.GetTime(true);
		if (cooldown > 0)
			return cooldown;

		return 0;
	}

#ifdef DIAG
#ifndef SERVER
	void AddShape(Shape shape)
	{
		m_Expansion_DebugShapes.Insert(shape);
	}
#endif

	override void CommandHandler(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
#ifndef SERVER
		for (int i = m_Expansion_DebugShapes.Count() - 1; i >= 0; i--)
			m_Expansion_DebugShapes[i].Destroy();
		m_Expansion_DebugShapes.Clear();
#endif

		if (DEBUG_EXPANSION_AI_CLIMB != 0)
		{
			PlayerBase playerPB = PlayerBase.Cast(this);
	
			SHumanCommandClimbResult result();
			
			if (DEBUG_EXPANSION_AI_CLIMB & 0x01 != 0)
			{
				HumanCommandClimb.DoClimbTest(playerPB, result, 0);
				ExpansionClimb.DebugClimb(playerPB, result, 0xAAFFFF00, 0xAA00FFFF);
			}
			
			if (DEBUG_EXPANSION_AI_CLIMB & 0x10 != 0)
			{
				ExpansionClimb.DoClimbTest(playerPB, result);
				ExpansionClimb.DebugClimb(playerPB, result, 0xAAFF0000, 0xAA0000FF);
			}
		}

		super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);
	}
#endif
};
