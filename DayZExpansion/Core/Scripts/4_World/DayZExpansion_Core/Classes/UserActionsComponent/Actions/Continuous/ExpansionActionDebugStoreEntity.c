/**
 * ExpansionActionDebugStoreEntity.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionActionDebugStoreEntityCB: ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(5);
	}
};

class ExpansionActionDebugStoreEntity: ActionContinuousBase
{
	void ExpansionActionDebugStoreEntity()
	{
		m_CallbackClass = ExpansionActionDebugStoreEntityCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		m_Text = "[DEBUG] Store";
	}

	override void CreateConditionComponents()  
	{
		m_ConditionTarget = new CCTCursor;
		m_ConditionItem = new CCINone;
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override bool HasTarget()
	{
		return true;
	}

	override bool HasProgress()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		auto entity = EntityAI.Cast(target.GetParentOrObject());
		if (!entity)
			return false;

		CarScript vehicle;
		if (Class.CastTo(vehicle, entity) && vehicle.Expansion_GetVehicleCrew().Count())
			return false;

#ifdef EXPANSIONMODVEHICLE
		ExpansionVehicleBase exVehicle;
		if (Class.CastTo(exVehicle, entity) && exVehicle.Expansion_GetVehicleCrew().Count())
			return false;
#endif

		return true;
	}
	
	override void OnFinishProgressServer(ActionData action_data)
	{
		auto entity = EntityAI.Cast(action_data.m_Target.GetParentOrObject());
		if (!entity)
			return;

		ExpansionEntityStoragePlaceholder.Expansion_StoreEntityAndReplace(entity, "ExpansionDebugGoat", entity.GetPosition());
	}
}
