#define _ARMA_

class CfgPatches
{
	class DayZExpansion_NamalskAdventure_Objects_Lore
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data","DZ_Structures_Residential","ns2_build_a3","DayZExpansion_Objects_Gear_Electricity"};
	};
};
class CfgVehicles
{
	class HouseNoDestruct;
	class Inventory_Base;
	class ExpansionGenerator_Base;
	class Expansion_Satellite_Generator: ExpansionGenerator_Base
	{
		scope = 2;
		displayName = "$STR_EXPANSION_MOBILE_GENERATOR";
		descriptionShort = "Not needed.";
		model = "\DayZExpansion\Objects\Gear\Electricity\NewGenerator.p3d";
		inputRange = 24;
		fuelTankCapacity = 7000;
		handheld = "false";
		weight = 50000;
		physLayer = "item_large";
		carveNavmesh = 1;
		storageCategory = 10;
		class EnergyManager
		{
			hasIcon = 1;
			autoSwitchOff = 1;
			energyStorageMax = 10000;
			energyUsagePerSecond = 0.28;
			reduceMaxEnergyByDamageCoef = 0.5;
			energyAtSpawn = 5000;
			powerSocketsCount = 4;
			compatiblePlugTypes[] = {2,"PLUG_PAS_TERMINAL",6};
		};
		class AnimationSources
		{
			class socket_1_plugged
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 1;
			};
			class socket_2_plugged
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 1;
			};
			class socket_3_plugged
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 1;
			};
			class socket_4_plugged
			{
				source = "user";
				animPeriod = 0.01;
				initPhase = 1;
			};
			class dial_fuel
			{
				source = "user";
				animPeriod = 1;
				initPhase = 0;
			};
		};
	};
	class Expansion_Satellite_Control: Inventory_Base
	{
		scope = 2;
		displayName = "Satellite Control";
		descriptionShort = "Not needed.";
		model = "\nst\ns2\build\a3\proxy\a3_flaxbt_panel.p3d";
		handheld = "false";
		weight = 50000;
		physLayer = "item_large";
		carveNavmesh = 1;
		storageCategory = 10;
		attachments[] = {"Att_ExpansionKeyCard"};
		class GUIInventoryAttachmentsProps
		{
			class Attachments_Access
			{
				name = "Key Card";
				description = "Not needed.";
				attachmentSlots[] = {"Att_ExpansionKeyCard"};
				icon = "set:dayz_inventory image:cat_fp_tents";
				view_index = 1;
			};
		};
		class DamageSystem
		{
			class GlobalArmor
			{
				class Projectile
				{
					class Health
					{
						damage = 0;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0;
					};
				};
				class Melee
				{
					class Health
					{
						damage = 0;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0;
					};
				};
				class FragGrenade
				{
					class Health
					{
						damage = 0;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0;
					};
				};
			};
		};
	};
	class Expansion_CommunityGoals_Board: HouseNoDestruct
	{
		scope = 2;
		model = "DZ\structures\Residential\Misc\Misc_NoticeBoard2.p3d";
		handheld = "false";
		weight = 50000;
		physLayer = "item_large";
		carveNavmesh = 1;
		storageCategory = 10;
	};
	class Expansion_Fusion_Core: Inventory_Base
	{
		scope = 2;
		model = "DayZExpansion\NamalskAdventure\Dta\Objects\Lore\Expansion_Fusion_Core.p3d";
		bounding = "BSphere";
		forceFarBubble = "true";
		itemBehaviour = 2;
		handheld = "false";
		allowOwnedCargoManipulation = 1;
		attachments[] = {"Att_ExpansionAnomalyCore"};
		weight = 1000000;
		inventoryCondition = "true";
		storageCategory = 1;
		openable = 1;
		vehicleClass = "Inventory";
		physLayer = "item_large";
		hiddenSelections[] = {"body","core","core_shell"};
		hiddenSelectionsTextures[] = {"DayZExpansion\NamalskAdventure\Dta\Objects\Lore\data\fusion_core_co.paa","#(argb,8,8,3)color(0,1,1,1.0,CO)","DayZExpansion\NamalskAdventure\Dta\Objects\Lore\data\fusion_core_co.paa"};
		hiddenSelectionsMaterials[] = {"DayZExpansion\NamalskAdventure\Dta\Objects\Lore\data\Expansion_Fusion_Core.rvmat","","DayZExpansion\NamalskAdventure\Dta\Objects\Lore\data\Expansion_Fusion_Core.rvmat"};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100000;
					healthLevels[] = {};
				};
			};
			class GlobalArmor
			{
				class Projectile
				{
					class Health
					{
						damage = 0;
					};
				};
				class FragGrenade
				{
					class Health
					{
						damage = 0;
					};
				};
				class Meele
				{
					class Health
					{
						damage = 0;
					};
				};
			};
		};
		class GUIInventoryAttachmentsProps
		{
			class Attachments
			{
				name = "$STR_attachment_accessories";
				description = "";
				attachmentSlots[] = {"Att_ExpansionAnomalyCore"};
				icon = "set:expansion_inventory image:anomaly";
			};
		};
	};
	class NA_Dokuments_AthenaOneBunker1: Inventory_Base
	{
		model = "\nst\ns_dayz\gear\lore\paper_files.p3d";
		scope = 2;
		title = "PLACEHOLDER";
		author = "NAC";
		file = "DayZExpansion\NamalskAdventure\Dta\Objects\Lore\data\athena1_bunker1.html";
		displayName = "PLACEHOLDER";
		descriptionShort = "PLACEHOLDER";
		hiddenSelections[] = {"camoGround"};
		hiddenSelectionsTextures[] = {"nst\ns_dayz\gear\lore\data\paper_files2_co.paa"};
		rotationFlags = 1;
		weight = 100;
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100;
					healthLabels[] = {1.0,0.7,0.5,0.3,0.0};
					healthLevels[] = {{1.0,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.7,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.5,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.3,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.0,{"nst\ns_dayz\gear\lore\data\lore_paper_destruct.rvmat"}}};
				};
			};
		};
	};
	class NA_Dokuments_AthenaOneBunker2: Inventory_Base
	{
		model = "\nst\ns_dayz\gear\lore\paper_files.p3d";
		scope = 2;
		title = "PLACEHOLDER";
		author = "NAC";
		file = "DayZExpansion\NamalskAdventure\Dta\Objects\Lore\data\athena1_bunker2.html";
		displayName = "PLACEHOLDER";
		descriptionShort = "PLACEHOLDER";
		hiddenSelections[] = {"camoGround"};
		hiddenSelectionsTextures[] = {"nst\ns_dayz\gear\lore\data\paper_files2_co.paa"};
		rotationFlags = 1;
		weight = 100;
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100;
					healthLabels[] = {1.0,0.7,0.5,0.3,0.0};
					healthLevels[] = {{1.0,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.7,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.5,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.3,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.0,{"nst\ns_dayz\gear\lore\data\lore_paper_destruct.rvmat"}}};
				};
			};
		};
	};
	class NA_Dokuments_Antenna1: Inventory_Base
	{
		model = "\nst\ns_dayz\gear\lore\paper_files.p3d";
		scope = 2;
		title = "Outpost Expedition";
		author = "NAC";
		file = "DayZExpansion\NamalskAdventure\Dta\Objects\Lore\data\secret_antenna.html";
		displayName = "Outpost Expedition";
		descriptionShort = "PLACEHOLDER";
		hiddenSelections[] = {"camoGround"};
		hiddenSelectionsTextures[] = {"nst\ns_dayz\gear\lore\data\paper_files2_co.paa"};
		rotationFlags = 1;
		weight = 100;
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100;
					healthLabels[] = {1.0,0.7,0.5,0.3,0.0};
					healthLevels[] = {{1.0,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.7,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.5,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.3,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.0,{"nst\ns_dayz\gear\lore\data\lore_paper_destruct.rvmat"}}};
				};
			};
		};
	};
	class NA_Dokuments_Antenna2: Inventory_Base
	{
		model = "\nst\ns_dayz\gear\lore\paper_files.p3d";
		scope = 2;
		title = "Outpost Attack";
		author = "NAC";
		file = "DayZExpansion\NamalskAdventure\Dta\Objects\Lore\data\antenna_attack.html";
		displayName = "Outpost Attack";
		descriptionShort = "PLACEHOLDER";
		hiddenSelections[] = {"camoGround"};
		hiddenSelectionsTextures[] = {"nst\ns_dayz\gear\lore\data\paper_files2_co.paa"};
		rotationFlags = 1;
		weight = 100;
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100;
					healthLabels[] = {1.0,0.7,0.5,0.3,0.0};
					healthLevels[] = {{1.0,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.7,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.5,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.3,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.0,{"nst\ns_dayz\gear\lore\data\lore_paper_destruct.rvmat"}}};
				};
			};
		};
	};
	class NA_Dokuments_Antenna3: Inventory_Base
	{
		model = "\nst\ns_dayz\gear\lore\paper_files.p3d";
		scope = 2;
		title = "Outpost Research";
		author = "NAC";
		file = "DayZExpansion\NamalskAdventure\Dta\Objects\Lore\data\antenna_research.html";
		displayName = "Outpost Research";
		descriptionShort = "PLACEHOLDER";
		hiddenSelections[] = {"camoGround"};
		hiddenSelectionsTextures[] = {"nst\ns_dayz\gear\lore\data\paper_files2_co.paa"};
		rotationFlags = 1;
		weight = 100;
		attachments[] = {"Att_ExpansionKeyCard"};
		class GUIInventoryAttachmentsProps
		{
			class Attachments_Access
			{
				name = "Key Card";
				description = "Not needed.";
				attachmentSlots[] = {"Att_ExpansionKeyCard"};
				icon = "set:dayz_inventory image:cat_fp_tents";
				view_index = 1;
			};
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100;
					healthLabels[] = {1.0,0.7,0.5,0.3,0.0};
					healthLevels[] = {{1.0,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.7,{"nst\ns_dayz\gear\lore\data\lore_paper.rvmat"}},{0.5,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.3,{"nst\ns_dayz\gear\lore\data\lore_paper_damage.rvmat"}},{0.0,{"nst\ns_dayz\gear\lore\data\lore_paper_destruct.rvmat"}}};
				};
			};
		};
	};
	class Expansion_3DPrinter: Inventory_Base
	{
		scope = 2;
		displayName = "Advanced 3D Printer";
		descriptionShort = "Not needed.";
		handheld = "false";
		weight = 50000;
		physLayer = "item_large";
		carveNavmesh = 1;
		storageCategory = 10;
		itemsCargoSize[] = {10,10};
		model = "\nst\ns3\structures\oilrig\sea_oilrig_printer.p3d";
		class DamageSystem
		{
			class GlobalArmor
			{
				class Projectile
				{
					class Health
					{
						damage = 0;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0;
					};
				};
				class Melee
				{
					class Health
					{
						damage = 0;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0;
					};
				};
				class FragGrenade
				{
					class Health
					{
						damage = 0;
					};
					class Blood
					{
						damage = 0;
					};
					class Shock
					{
						damage = 0;
					};
				};
			};
		};
		class AnimationSources
		{
			class PrinterHead
			{
				source = "user";
				animPeriod = 1;
				initPhase = 0;
			};
		};
	};
};
