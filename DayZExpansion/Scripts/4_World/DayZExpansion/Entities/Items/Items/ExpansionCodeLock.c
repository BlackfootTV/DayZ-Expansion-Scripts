/**
 * ExpansionCodeLock.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionCodeLock
 * @brief		
 **/
class ExpansionCodeLock extends ItemBase
{
	static protected const string RED_LIGHT_GLOW	= "dz\\gear\\camping\\data\\battery_charger_light_r.rvmat";
	static protected const string GREEN_LIGHT_GLOW	= "dz\\gear\\camping\\data\\battery_charger_light_g.rvmat";
	static protected const string DEFAULT_MATERIAL 	= "DayZExpansion\\Objects\\Basebuilding\\Items\\Codelock\\data\\codelock.rvmat";

	protected static ref TStringArray s_Expansion_InventorySlots = new TStringArray;

	protected EffectSound m_Sound;
	
	protected bool m_WasSynced;
	protected bool m_WasLocked;

	void ExpansionCodeLock()
	{
		m_KnownUIDs = new TStringArray;

		RegisterNetSyncVariableBool( "m_Locked" );
		RegisterNetSyncVariableInt( "m_CodeLength" );
	}

	override void DeferredInit()
	{
		super.DeferredInit();

		UpdateVisuals();

		ItemBase parent = ItemBase.Cast( GetHierarchyParent() );
		if ( parent && parent.ExpansionGetCodeLock() == this )
		{
			if ( GetGame().IsServer() && !HasCode() && parent.HasCode() )
			{
				EXPrint(ToString() + "::DeferredInit - migrating code from " + parent.ToString() + " " + parent.GetPosition());

				//! Migrate code
				SetCode(parent.GetCode(), NULL, false, false);

				//! Migrate locked state
				if (parent.ExpansionIsLocked())
					ExpansionLock();

				parent.SetCode("", NULL, true, false);
			}
			else if ( ExpansionIsLocked() )
			{
				SetSlotLock( parent, true );
				SetTakeable( false );
			}
		}
		else if ( GetGame().IsServer() && HasCode() )
		{
			EXPrint(ToString() + "::DeferredInit - removing code from unattached " + ToString() + " " + GetPosition());
			SetCode("");
		}
	}

	override void OnStoreSave( ParamsWriteContext ctx )
	{
		#ifdef CF_MODSTORAGE
		if ( GetGame().SaveVersion() >= EXPANSION_VERSION_GAME_MODSTORAGE_TARGET )
		{
			super.OnStoreSave( ctx );
			return;
		}
		#endif

		super.OnStoreSave( ctx );

		ctx.Write( m_Locked );
		ctx.Write( m_Code );
		ctx.Write( m_KnownUIDs );
	}

	override bool OnStoreLoad( ParamsReadContext ctx, int version )
	{
		if ( Expansion_Assert_False( super.OnStoreLoad( ctx, version ), "[" + this + "] Failed reading OnStoreLoad super" ) )
			return false;

		#ifdef CF_MODSTORAGE
		if ( version > EXPANSION_VERSION_GAME_MODSTORAGE_TARGET || m_ExpansionSaveVersion > EXPANSION_VERSION_SAVE_MODSTORAGE_TARGET )
			return true;
		#endif

		if ( m_ExpansionSaveVersion >= 38 )
		{
			if ( Expansion_Assert_False( ctx.Read( m_Locked ), "[" + this + "] Failed reading m_Locked" ) )
				return false;
			if ( Expansion_Assert_False( ctx.Read( m_Code ), "[" + this + "] Failed reading m_Code" ) )
				return false;

			m_CodeLength = m_Code.Length();
		}

		if ( m_ExpansionSaveVersion >= 20 )
		{
			if ( Expansion_Assert_False( ctx.Read( m_KnownUIDs ), "[" + this + "] Failed reading m_KnownUIDs" ) )
				return false;
		}

		return true;
	}

	#ifdef CF_MODSTORAGE
	override void CF_OnStoreSave(CF_ModStorageMap storage)
	{
		super.CF_OnStoreSave(storage);

		auto ctx = storage[DZ_Expansion];
		if (!ctx) return;

		ctx.Write( m_Locked );
		ctx.Write( m_Code );

		ctx.Write( m_KnownUIDs.Count() );
		for ( int i = 0; i < m_KnownUIDs.Count(); i++ )
		{
			ctx.Write( m_KnownUIDs[i] );
		}
	}
	
	override bool CF_OnStoreLoad(CF_ModStorageMap storage)
	{
		if (!super.CF_OnStoreLoad(storage))
			return false;

		auto ctx = storage[DZ_Expansion];
		if (!ctx) return true;

		if ( ctx.GetVersion() >= 38 )
		{
			if ( Expansion_Assert_False( ctx.Read( m_Locked ), "[" + this + "] Failed reading m_Locked" ) )
				return false;
			if ( Expansion_Assert_False( ctx.Read( m_Code ), "[" + this + "] Failed reading m_Code" ) )
				return false;

			m_CodeLength = m_Code.Length();
		}

		int count;
		if (!ctx.Read(count))
			return false;

		for (int i = 0; i < count; i++)
		{
			string knownUID;
			if (!ctx.Read(knownUID))
				return false;

			m_KnownUIDs.Insert( knownUID );
		}

		return true;
	}
	#endif

	override void SetActions()
	{
		super.SetActions();
		
		AddAction( ExpansionActionAttachCodeLock );
		//AddAction( ExpansionActionEnterCodeLock );
		//AddAction( ExpansionActionChangeCodeLock );
	}
	
	override bool CanDetachAttachment(EntityAI parent)
	{
		if ( !super.CanDetachAttachment( parent ) )
		{
			return false;
		}
		
		ItemBase target = ItemBase.Cast( parent );

		if ( target && target.ExpansionIsLocked() )
		{
			return false;
		}
		
		return true;
	}

	override void OnWasAttached( EntityAI parent, int slot_id )
	{
		super.OnWasAttached( parent, slot_id );

		UpdateVisuals();
	}

	override void OnWasDetached( EntityAI parent, int slot_id )
	{
		super.OnWasDetached( parent, slot_id );

		UpdateVisuals();

		if (GetGame().IsServer())
			SetCode("");
	}

	void UnlockServer( EntityAI player, EntityAI parent )
	{
		if ( parent && GetInventory().IsAttachment() )
		{
			ExpansionUnlock();
			ItemBase item;
			if (parent.GetInventory() && parent.GetInventory().IsInventoryLocked() && Class.CastTo(item, parent) && !item.IsOpen())
				item.Open();
			ExpansionDropServer( PlayerBase.Cast( player ) );
		}
	}

	override protected string GetDestroySound()
	{
		return "combinationlock_open_SoundSet";
	}

	override void ExpansionOnDestroyed( Object killer )
	{
		if ( IsDamageDestroyed() )
			return;

		SetHealth( 0 );
		UnlockServer( EntityAI.Cast( killer ), GetHierarchyParent() );
	}
		
	static TStringArray Expansion_GetInventorySlots()
	{
		if (!s_Expansion_InventorySlots.Count())
			GetGame().ConfigGetTextArray("CfgVehicles ExpansionCodeLock inventorySlot", s_Expansion_InventorySlots);

		return s_Expansion_InventorySlots;
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if ( m_WasSynced && m_WasLocked != m_Locked )
		{
			if ( m_Locked )
				SoundCodeLockLocked();
			else
				SoundCodeLockUnlocked();
		}

		m_WasLocked = m_Locked;
		m_WasSynced = true;

		UpdateVisuals();
	}

	// --- VISUALS
	void UpdateVisuals()
	{
		//Client/Server
		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdateVisuals_Deferred, 0, false );
	}
	
	protected void UpdateVisuals_Deferred()
	{
		ItemBase parent = ItemBase.Cast( GetHierarchyParent() );

		if ( parent && parent.IsInherited( Fence ) )
		{
			HideSelection( "camo" );
			HideSelection( "Codelock" );
			ShowSelection( "attach_fence" );
		} else
		{
			ShowSelection( "camo" );
			ShowSelection( "Codelock" );
			HideSelection( "attach_fence" );
		}

		if ( !IsMissionClient() )
			return;

		if ( parent && ExpansionIsLocked() )
		{
			SetObjectMaterial( 0, DEFAULT_MATERIAL );
			SetObjectMaterial( 1, GREEN_LIGHT_GLOW );
			SetObjectMaterial( 2, DEFAULT_MATERIAL );
		} else
		{
			SetObjectMaterial( 0, RED_LIGHT_GLOW );
			SetObjectMaterial( 1, DEFAULT_MATERIAL );
			SetObjectMaterial( 2, DEFAULT_MATERIAL );
		}
	}
	
	protected void SoundCodeLockLocked()
	{
		if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			m_Sound = SEffectManager.PlaySound("Expansion_CodeLock_Lock1_SoundSet", GetPosition());
			m_Sound.SetSoundAutodestroy( true );
		}
	}
	
	protected void SoundCodeLockUnlocked()
	{
		if ( !GetGame().IsMultiplayer() || GetGame().IsClient() ) // client side
		{
			m_Sound = SEffectManager.PlaySound("Expansion_CodeLock_Unlock1_SoundSet", GetPosition());
			m_Sound.SetSoundAutodestroy( true );
		}
	}
}
