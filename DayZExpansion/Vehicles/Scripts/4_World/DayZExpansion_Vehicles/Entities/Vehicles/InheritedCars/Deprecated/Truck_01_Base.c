/**
 * Truck_01_Base.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
 */
modded class Truck_01_Base
{
	override string ExpansionGetWheelType(int slot_id)
	{
		string slot_name = InventorySlots.GetSlotName(slot_id);

		switch (slot_name)
		{
			case "Truck_01_Wheel_1_2":
			case "Truck_01_Wheel_2_2":
			case "Truck_01_Wheel_1_3":
			case "Truck_01_Wheel_2_3":
				return "Truck_01_WheelDouble";
		}

		return "Truck_01_Wheel";
	}

	override float GetTransportCameraDistance()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.VEHICLES, this, "GetTransportCameraDistance");
#endif

		return 6.0;
	}

	override float GetCameraHeight()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.VEHICLES, this, "GetCameraHeight");
#endif

		return 3.2;
	}
};
