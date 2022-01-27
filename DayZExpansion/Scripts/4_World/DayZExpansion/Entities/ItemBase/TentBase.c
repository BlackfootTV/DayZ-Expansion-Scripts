/**
 * TentBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
 * Special thanks to DaemonForge for his scripts.
 *
*/

modded class TentBase
{
	protected bool m_IsOpened = true;
	protected bool m_IsOpened1 = true;
	protected bool m_IsOpened2 = true;
	protected bool m_IsOpened3 = true;
	protected bool m_IsOpened4 = true;
	protected bool m_IsOpened5 = true;
	protected bool m_IsOpened6 = true;

	void TentBase()
	{
		RegisterNetSyncVariableBool( "m_IsOpened" );
		RegisterNetSyncVariableBool( "m_IsOpened1" );
		RegisterNetSyncVariableBool( "m_IsOpened2" );
		RegisterNetSyncVariableBool( "m_IsOpened3" );
		RegisterNetSyncVariableBool( "m_IsOpened4" );
		RegisterNetSyncVariableBool( "m_IsOpened5" );
		RegisterNetSyncVariableBool( "m_IsOpened6" );
	}
	
	override void SetActions()
	{
		super.SetActions();

		AddAction( ExpansionActionEnterCodeLock );
		AddAction( ExpansionActionChangeCodeLock );
	}

	override void ToggleAnimation( string selection )
	{
		super.ToggleAnimation( selection );

		ToggleTentOpening( selection );
	}
	
	protected void ToggleTentOpening( string state )
	{
		bool wasOpened = m_IsOpened;

		//! toggleing an closed door so it would now be open
		if (state == "entrancec")
		{
			m_IsOpened = true;
		}
		//! toggleing an open door so it would now be closed
		else if (state == "entranceo")
		{
			m_IsOpened = false;
		}
		else //! Might Be Party Tent
		{
			//Party Tent Logic
			if ( state.Contains("door") )
			{
				if (state == "door1o"){      m_IsOpened1 = false; }
				else if (state == "door1c"){ m_IsOpened1 = true;  }
				else if (state == "door2o"){ m_IsOpened2 = false; }
				else if (state == "door2c"){ m_IsOpened2 = true;  }
				else if (state == "door3o"){ m_IsOpened3 = false; }
				else if (state == "door3c"){ m_IsOpened3 = true;  }
				else if (state == "door4o"){ m_IsOpened4 = false; }
				else if (state == "door4c"){ m_IsOpened4 = true;  }
				else if (state == "door5o"){ m_IsOpened5 = false; }
				else if (state == "door5c"){ m_IsOpened5 = true;  }
				else if (state == "door6o"){ m_IsOpened6 = false; }
				else if (state == "door6c"){ m_IsOpened6 = true;  }
				else { return; } //Wasn't Party Tent :) No need to continue any more :)
			} else { return; } //Wasn't Party Tent :) Probly a large tent window, no need to continue any more :)
			
			//! if any doors are open its now a closed door
			if ( m_IsOpened1 || m_IsOpened2 || m_IsOpened3 || m_IsOpened4 || m_IsOpened5 || m_IsOpened6 )
			{
				m_IsOpened = true;
			} else {
				m_IsOpened = false;
			}
		}

		
		if ( HasCode() && m_IsOpened != wasOpened )
		{
			if ( m_IsOpened )
				Unlock();
			else
				ExpansionLock();
		} else
		{
			SetSynchDirty();
		}
	}

	//! Only call this after settings have been loaded
	bool ExpansionCanAttachCodeLock()
	{
		int attachMode = GetExpansionSettings().GetBaseBuilding().CodelockAttachMode;
		return attachMode == ExpansionCodelockAttachMode.ExpansionAndTents || attachMode == ExpansionCodelockAttachMode.ExpansionAndFenceAndTents;
	}
	
	override bool CanReceiveItemIntoCargo(EntityAI item )
	{
        if (IsLocked() && GetExpansionSettings().GetBaseBuilding() )
		{
			if ( ExpansionCanAttachCodeLock() )
			{
           	 	return false;
			}
		}

        return super.CanReceiveItemIntoCargo(item );
    }


    override bool CanReleaseCargo(EntityAI cargo)
	{
        if ( IsLocked() && GetExpansionSettings().GetBaseBuilding() )
		{
			if ( ExpansionCanAttachCodeLock() )
			{
           	 	return false;
			}
		}

        return super.CanReleaseCargo(cargo);
    }

    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
	{
		if ( GetExpansionSettings().GetBaseBuilding() )
		{
			if ( attachment.IsInherited( ExpansionCodeLock ) )
			{
				if ( !ExpansionCanAttachCodeLock() )
					return false;

				//! Safety to prevent attaching Expansion Code Lock if another lock is already present in different slot (e.g. silver Code Lock from RoomService's mod)
				if ( FindAttachmentBySlotName( "Att_CombinationLock" ) )
					return false;

				if ( IsEntranceRuined() )
					return false;
			}

			//! Safety to prevent attaching other locks (e.g. silver Code Lock from RoomService's mod) if Expansion Code Lock is already present
			if ( attachment.IsKindOf( "CombinationLock" ) && ExpansionGetCodeLock() )
				return false;
		}

        return super.CanReceiveAttachment(attachment, slotId);
    }
	
	bool IsEntranceRuined()
	{
		TStringArray selections = new TStringArray;
		GetSelectionList( selections );

		foreach ( string selection : selections )
		{
			if ( selection.IndexOf( "door" ) == 0 || selection.IndexOf( "entrance" ) == 0 )
			{
				string zone;
				DamageSystem.GetDamageZoneFromComponentName( this, selection, zone );
				if ( GetHealthLevel( zone ) == GameConstants.STATE_RUINED )
					return true;
			}
		}

		return false;
	}
	
	override bool CanReleaseAttachment( EntityAI attachment )
	{
        if ( IsLocked() && GetExpansionSettings().GetBaseBuilding() )
		{
			if ( ExpansionCanAttachCodeLock() )
			{
           	 	return false;
			}
		}

        return super.CanReleaseAttachment(attachment);
	}
	
	override bool IsOpened()
	{
		return m_IsOpened;
	}

	override bool ExpansionIsOpenable()
	{
		return true;
	}
	
	override bool ExpansionCanOpen( PlayerBase player, string selection )
	{
		return !m_IsOpened && ( !IsLocked() || IsKnownUser( player ) );
	}

	override ExpansionCodeLock ExpansionGetCodeLock()
	{
		return ExpansionCodeLock.Cast(FindAttachmentBySlotName( "Att_ExpansionCodeLock" ));
	}

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		#ifdef CF_MODSTORAGE
		if ( GetGame().SaveVersion() >= EXPANSION_VERSION_GAME_MODSTORAGE_TARGET )
		{
			super.OnStoreSave( ctx );
			return;
		}
		#endif

		super.OnStoreSave( ctx );
		
		ctx.Write( m_IsOpened );
		ctx.Write( m_IsOpened1 );
		ctx.Write( m_IsOpened2 );
		ctx.Write( m_IsOpened3 );
		ctx.Write( m_IsOpened4 );
		ctx.Write( m_IsOpened5 );
		ctx.Write( m_IsOpened6 );
	}


	override bool OnStoreLoad( ParamsReadContext ctx, int version )
	{
		if ( Expansion_Assert_False( super.OnStoreLoad( ctx, version ), "[" + this + "] Failed reading OnStoreLoad super" ) )
			return false;

		#ifdef CF_MODSTORAGE
		if ( version > EXPANSION_VERSION_GAME_MODSTORAGE_TARGET || m_ExpansionSaveVersion > EXPANSION_VERSION_SAVE_MODSTORAGE_TARGET )
			return true;
		#endif

		if ( m_ExpansionSaveVersion < 19 )
			return true;

		bool loadingsuccessfull = true;

		if ( m_ExpansionSaveVersion < 38 )
		{
			if ( Expansion_Assert_False( ctx.Read( m_Locked ) , "[" + this + "] Failed reading m_Locked" ))
				loadingsuccessfull = false;
			
			if ( Expansion_Assert_False( ctx.Read( m_Code ), "[" + this + "] Failed reading m_Code" ) )
				loadingsuccessfull = false;

			m_CodeLength = m_Code.Length();

			bool hasCode;
			if ( Expansion_Assert_False( ctx.Read( hasCode ), "[" + this + "] Failed reading hasCode" ) )
				loadingsuccessfull = false;
		}
		
		if ( Expansion_Assert_False( ctx.Read( m_IsOpened ), "[" + this + "] Failed reading m_IsOpened" ) )
		{
			m_IsOpened = true;
			loadingsuccessfull = false;
		}
		
		if ( Expansion_Assert_False( ctx.Read( m_IsOpened1 ), "[" + this + "] Failed reading m_IsOpened1" ) )
		{
			m_IsOpened1 = true;
			loadingsuccessfull = false;
		}
		
		if ( Expansion_Assert_False( ctx.Read( m_IsOpened2 ), "[" + this + "] Failed reading mm_IsOpened2_Locked" ) )
		{
			m_IsOpened2 = true;
			loadingsuccessfull = false;
		}
		
		if ( Expansion_Assert_False( ctx.Read( m_IsOpened3 ), "[" + this + "] Failed reading m_IsOpened3" ) )
		{
			m_IsOpened3 = true;
			loadingsuccessfull = false;
		}
		
		if ( Expansion_Assert_False( ctx.Read( m_IsOpened4 ), "[" + this + "] Failed reading m_IsOpened4" ) )
		{
			m_IsOpened4 = true;
			loadingsuccessfull = false;
		}
			
		if ( Expansion_Assert_False( ctx.Read( m_IsOpened5 ), "[" + this + "] Failed reading m_IsOpened5" ) )
		{
			m_IsOpened5 = true;
			loadingsuccessfull = false;
		}
		
		if ( Expansion_Assert_False( ctx.Read( m_IsOpened6 ), "[" + this + "] Failed reading m_IsOpened6" ) )
		{
			m_IsOpened6 = true;
			loadingsuccessfull = false;
		}
		
		//! If Code Locks on the tents it will remove them Just calling later so simplify and ensure that the code lock has been created
		if ( !ExpansionCanAttachCodeLock() )
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.ExpansionCodeLockRemove, 1000, false );
		}
		
		return loadingsuccessfull;
	}

	#ifdef CF_MODSTORAGE
	override void CF_OnStoreSave(CF_ModStorageMap storage)
	{
		super.CF_OnStoreSave(storage);

		auto ctx = storage[DZ_Expansion];
		if (!ctx) return;

		ctx.Write(m_IsOpened);
		ctx.Write(m_IsOpened1);
		ctx.Write(m_IsOpened2);
		ctx.Write(m_IsOpened3);
		ctx.Write(m_IsOpened4);
		ctx.Write(m_IsOpened5);
		ctx.Write(m_IsOpened6);
	}
	
	override bool CF_OnStoreLoad(CF_ModStorageMap storage)
	{
		if (!super.CF_OnStoreLoad(storage))
			return false;

		auto ctx = storage[DZ_Expansion];
		if (!ctx) return true;

		if (ctx.GetVersion() < 38)
		{
			if (!ctx.Read(m_Locked))
				return false;
				
			if (!ctx.Read(m_Code))
				return false;

			m_CodeLength = m_Code.Length();

			bool hasCode;
			if (!ctx.Read(hasCode))
				return false;
		}

		if (!ctx.Read(m_IsOpened))
			return false;

		if (!ctx.Read(m_IsOpened1))
			return false;
			
		if (!ctx.Read(m_IsOpened2))
			return false;

		if (!ctx.Read(m_IsOpened3))
			return false;

		if (!ctx.Read(m_IsOpened4))
			return false;

		if (!ctx.Read(m_IsOpened5))
			return false;

		if (!ctx.Read(m_IsOpened6))
			return false;

		if (!ExpansionCanAttachCodeLock())
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ExpansionCodeLockRemove, 1000, false);
		
		return true;
	}
	#endif
	
	void ExpansionCodeLockRemove()
	{
		if ( !ExpansionCanAttachCodeLock() )
		{
			ExpansionCodeLock codelock = ExpansionGetCodeLock();
			if (codelock)
				codelock.Delete();
		}
	}

	override bool CanDisplayAttachmentSlot( string slot_name )
	{
        if ( slot_name == "Att_ExpansionCodeLock" )
		{
			return ExpansionCanAttachCodeLock();
		}

		return super.CanDisplayAttachmentSlot( slot_name );
	}

	override void EEHitBy( TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef )
	{
		super.EEHitBy( damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef );

		ExpansionCodeLock codelock = ExpansionGetCodeLock();
		if ( codelock && IsEntranceRuined() )
			codelock.UnlockServer( null, this );
	}

	override void EEKilled( Object killer )
	{
		super.EEKilled( killer );

		ExpansionCodeLock codelock = ExpansionGetCodeLock();
		if ( codelock )
			codelock.UnlockServer( null, this );
	}
};
