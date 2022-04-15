/*
 * ExpansionBulldozerScript.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionBulldozerScript
 * @brief		This class handle bulldozer simulation
 **/
class ExpansionBulldozerScript extends CarScript
{
	protected int m_TreesCutted;
	protected vector m_RayStart;

	void ExpansionBulldozerScript()
	{
		m_dmgContactCoef = 0.075;	

		//! Default values	
		m_TreesCutted = 0;
		
		//! Events layer mask
		SetEventMask(EntityEvent.SIMULATE);
	}

	void SyncVariables()
	{
		TreeHard tree;
		BushHard bush;

		m_RayStart = ModelToWorld(GetMemoryPointPos("fence_start"));	

		array<Object> objects = new array<Object>;
		array<CargoBase> proxy = new array<CargoBase>;

		if ( IsMissionHost( ) ) 
		{
			ItemBase radiator;
			Class.CastTo( radiator, FindAttachmentBySlotName("TruckRadiator"));

			if ( radiator )
			{
				if (m_TreesCutted > Math.RandomInt(50,100))
				{
					radiator.DecreaseHealth("", "", 100);	
				}
			}
		}

		GetGame().GetObjectsAtPosition(m_RayStart, 1, objects, proxy);
		if ( objects && GetSpeedometer() > 5 ) 
		{
			for ( int i = 0; i < objects.Count(); i++ ) 
			{
				Object obj = objects.Get(i);
				if ( obj.IsTree() || obj.IsBush() )
				{
					EntityAI cutting_tool;
					if ( IsMissionHost() ) 
					{
						cutting_tool = EntityAI.Cast( GetGame().CreateObject("WoodAxe", vector.Zero, false, true) );
					}

					if ( obj.IsTree() )
					{
						tree = TreeHard.Cast( obj );		
						if ( tree && !tree.IsCut() )
						{
							if ( IsMissionHost() ) 
							{
								tree.DecreaseHealth("", "", 100, true);			
								tree.OnTreeCutDown( cutting_tool );
								if ( tree.IsTree() )
								{
									ItemBase wooden_logs = ItemBase.Cast(GetGame().CreateObject("WoodenLog", GetPosition(), false));
								}
								dBodyDestroy(tree);
								GetGame().ObjectDelete(cutting_tool);	
								m_TreesCutted++;	
							}
							if (GetGame().IsClient() || !GetGame().IsMultiplayer())
								SoundHardTreeFallingPlay();
						}	
					} else if (obj.IsBush())
					{
						bush = BushHard.Cast(obj);		
						if ( bush && !bush.IsCut() )
						{
							if ( IsMissionHost()) 
							{
								bush.DecreaseHealth("", "", 100, true);			
								bush.OnTreeCutDown( cutting_tool );
								ItemBase wooden_sticks = ItemBase.Cast(GetGame().CreateObject("LongWoodenStick", GetPosition(), false));
								dBodyDestroy(bush);
								GetGame().ObjectDelete(cutting_tool);	
								m_TreesCutted++;	
							}

							if ( GetGame().IsClient() || !GetGame().IsMultiplayer() )
								SoundHardBushFallingPlay();
						}
					}
				}
			}
		}	
	}

	override void EOnSimulate(IEntity owner, float dt)
	{
		//SyncVariables( );
	}
	
	override void SoundHardTreeFallingPlay()
	{
		EffectSound sound =	SEffectManager.PlaySound( "hardTreeFall_SoundSet", GetPosition() );
		sound.SetSoundAutodestroy( true );
	}
		
	override void SoundSoftTreeFallingPlay()
	{
		EffectSound sound =	SEffectManager.PlaySound( "softTreeFall_SoundSet", GetPosition() );
		sound.SetSoundAutodestroy( true );
	}
		
	override void SoundHardBushFallingPlay()
	{
		EffectSound sound =	SEffectManager.PlaySound( "hardBushFall_SoundSet", GetPosition() );
		sound.SetSoundAutodestroy( true );
	}
		
	override void SoundSoftBushFallingPlay()
	{
		EffectSound sound =	SEffectManager.PlaySound( "softBushFall_SoundSet", GetPosition() );
		sound.SetSoundAutodestroy( true );
	}

	override int GetAnimInstance()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.VEHICLES, this, "GetAnimInstance");
#endif

		return ExpansionVehicleAnimInstances.EXPANSION_MH6;
	}

	override int GetSeatAnimationType(int posIdx)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.VEHICLES, this, "GetSeatAnimationType").Add(posIdx);
#endif

		switch( posIdx )
		{
		case 0:
			return DayZPlayerConstants.VEHICLESEAT_DRIVER;
		default:
			return 0;
		}

		return 0;
	}
	
	override bool CrewCanGetThrough(int posIdx)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.VEHICLES, this, "CrewCanGetThrough").Add(posIdx);
#endif

		return true;
	}

	override bool CanReachDoorsFromSeat(string pDoorsSelection, int pCurrentSeat)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_2(ExpansionTracing.VEHICLES, this, "CanReachDoorsFromSeat").Add(pDoorsSelection).Add(pCurrentSeat);
#endif

		return true;		
	}
	
	override bool IsVitalCarBattery()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.VEHICLES, this, "IsVitalHelicopterBattery");
#endif

		return false;
	}

	override bool IsVitalSparkPlug()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.VEHICLES, this, "IsVitalSparkPlug");
#endif

		return true;
	}
	
	override bool IsVitalRadiator()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.VEHICLES, this, "IsVitalRadiator");
#endif

		return true;
	}
	
	override bool IsVitalGlowPlug()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.VEHICLES, this, "IsVitalGlowPlug");
#endif

		return false;
	}

	override bool IsVitalEngineBelt()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.VEHICLES, this, "IsVitalEngineBelt");
#endif

		return false;
	}

	override bool IsVitalTruckBattery()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.VEHICLES, this, "IsVitalTruckBattery");
#endif

		return true;
	}
};
