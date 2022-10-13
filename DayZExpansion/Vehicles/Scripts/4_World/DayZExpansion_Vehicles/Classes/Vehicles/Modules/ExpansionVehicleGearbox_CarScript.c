/*
 * ExpansionVehicleGearbox_CarScript.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionVehicleGearbox_CarScript : ExpansionVehicleGearboxDefault
{
	CarScript m_Car;

	void ExpansionVehicleGearbox_CarScript(EntityAI vehicle, string rootPath)
	{
		//m_TEMP_DeferredInit = false;
		//m_SettingsChanged = false;
		//m_Control = false;
		//m_PreSimulate = false;
		//m_Simulate = false;
		//m_Animate = false;
		//m_Network = false;

		Class.CastTo(m_Car, m_Vehicle);
	}

	override void PreSimulate(ExpansionPhysicsState pState)
	{
#ifdef DAYZ_1_18
		m_Gear = m_Car.GetController().GetGear() + 2;
#else
		//! 1.19
		m_Gear = m_Car.GetGear() + 2;
#endif
		m_Ratio = m_Ratios[m_Gear];

		super.PreSimulate(pState);
	}
};
