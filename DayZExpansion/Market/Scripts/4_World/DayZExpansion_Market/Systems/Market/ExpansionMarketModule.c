/**
 * ExpansionMarketModule.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

enum ExpansionMarketResult
{
	Success = 0,
	FailedReserveTime,
	FailedNoCount,
	FailedUnknown,
	FailedStockChange,
	FailedOutOfStock,
	FailedAttachmentOutOfStock,
	FailedNotEnoughMoney,

	//! NOTE: Make sure that vehicle enums are together with no others in between!
	FailedNoVehicleSpawnPositions,
	FailedNotEnoughVehicleSpawnPositionsNear,
	FailedVehicleSpawnOccupied,

	FailedTooFarAway,
	FailedCannotSell,
	FailedCannotBuy,
	FailedNotInPlayerPossession
}

class ExpansionMarketPlayerInventory
{
	PlayerBase m_Player;
	ref array<EntityAI> m_Inventory;

	void ExpansionMarketPlayerInventory(PlayerBase player)
	{
		m_Player = player;
		m_Inventory = new array<EntityAI>;
		Enumerate();
	}

	void ~ExpansionMarketPlayerInventory()
	{
		EXPrint("~ExpansionMarketPlayerInventory");
	}

	void Enumerate()
	{
		array<EntityAI> items = new array<EntityAI>;
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		AddPlayerItems(items);

		array<EntityAI> driven = GetNearbyDrivenVehicles();
		AddPlayerItems(driven, m_Inventory);
	}

	private void AddPlayerItems(array<EntityAI> items, array<EntityAI> existing = NULL)
	{
		foreach (EntityAI item: items)
		{
			item = SubstituteOwnedVehicle(item);

			if (!item)
				continue;

			if (existing && existing.Find(item) > -1)
				continue;

			m_Inventory.Insert( item );
		}
	}
	
	EntityAI SubstituteOwnedVehicle(EntityAI item)
	{
		#ifdef EXPANSIONMODVEHICLE
		//! If this is a master key of a vehicle, substitute the vehicle itself if it's close
		ExpansionCarKey key;
		if (Class.CastTo(key, item) && key.IsMaster())
		{
			Object keyObject = key.GetKeyObject();

			if (keyObject && IsVehicleNearby(keyObject))
				item = EntityAI.Cast(keyObject);
		}
		#endif

		return item;
	}
	
	array<EntityAI> GetNearbyDrivenVehicles(string className = "", int amount = -1)
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("GetNearbyDrivenVehicles - Start - " + m_Player + " '" + className + "' " + amount);
		#endif

		array<EntityAI> driven = new array<EntityAI>;

		string type;

		set<CarScript> cars = CarScript.GetAll();
		foreach (CarScript car: cars)
		{
			type = car.GetType();
			type.ToLower();
			if ((!className || type == className) && car.ExpansionGetLastDriverUID() == m_Player.GetIdentityUID())
			{
				#ifdef EXPANSIONMODVEHICLE
				if (car.IsLocked())
					continue;
				#endif

				if (IsVehicleNearby(car))
					driven.Insert(car);

				if (driven.Count() == amount)
					return driven;
			}
		}

		#ifdef EXPANSIONMODVEHICLE
		set<ExpansionVehicleBase> vehicles = ExpansionVehicleBase.GetAll();
		foreach (ExpansionVehicleBase vehicle: vehicles)
		{
			type = vehicle.GetType();
			type.ToLower();
			if ((!className || type == className) && vehicle.ExpansionGetLastDriverUID() == m_Player.GetIdentityUID())
			{
				if (vehicle.IsLocked())
					continue;

				if (IsVehicleNearby(vehicle))
					driven.Insert(vehicle);

				if (driven.Count() == amount)
					return driven;
			}
		}
		#endif

		#ifdef EXPANSIONEXPRINT
		EXPrint("GetNearbyDrivenVehicles - End - driven: " + driven.Count());
		#endif

		return driven;
	}
	
	bool IsVehicleNearby(Object vehicle)
	{
		float maxDistance = GetExpansionSettings().GetMarket().GetMaxVehicleDistanceToTrader(vehicle.GetType());

		bool isNear = vector.Distance(m_Player.GetPosition(), vehicle.GetPosition()) <= maxDistance;

		return isNear;
	}
}

class ExpansionMarketModule: JMModuleBase
{
	static const float MAX_TRADER_INTERACTION_DISTANCE = 5;

	static ref ScriptInvoker SI_SetTraderInvoker = new ScriptInvoker();
	static ref ScriptInvoker SI_SelectedItemUpdatedInvoker = new ScriptInvoker();
	static ref ScriptInvoker SI_Callback = new ScriptInvoker();
	static ref ScriptInvoker SI_ATMMenuInvoker = new ScriptInvoker();
	static ref ScriptInvoker SI_ATMMenuCallback = new ScriptInvoker();
	static ref ScriptInvoker SI_ATMMenuTransferCallback = new ScriptInvoker();
	static ref ScriptInvoker SI_ATMMenuPartyCallback = new ScriptInvoker();

	//! Client
	protected ref ExpansionMarketPlayerInventory m_LocalEntityInventory;
	protected ref TIntArray m_TmpVariantIds;
	protected int m_TmpVariantIdIdx;
	protected ref map<int, ref ExpansionMarketCategory> m_TmpNetworkCats;
	protected ref array<ref ExpansionMarketNetworkBaseItem> m_TmpNetworkBaseItems;
	protected ExpansionMarketCurrency m_PlayerWorth;

	ref map<string, ExpansionMarketCurrency> m_MoneyTypes;
	ref array<string> m_MoneyDenominations;

	protected ref ExpansionMarketTraderZone m_ClientMarketZone;
	
	protected ref ExpansionTraderObjectBase m_OpenedClientTrader;
	
	ref array<ref ExpansionMarketATM_Data> m_ATMData;

	static ref map<string, ExpansionMarketItem> s_AmmoItems = new map<string, ExpansionMarketItem>;
	static ref map<string, string> s_AmmoBullets = new map<string, string>;
	
	// ------------------------------------------------------------
	// ExpansionMarketModule Constructor
	// ------------------------------------------------------------
	void ExpansionMarketModule()
	{
		MarketModulePrint("ExpansionMarketModule - Start");
		
		m_TmpVariantIds = new TIntArray;
		m_TmpNetworkCats = new map<int, ref ExpansionMarketCategory>;
		m_TmpNetworkBaseItems = new array<ref ExpansionMarketNetworkBaseItem>;

		m_MoneyTypes = new map<string, ExpansionMarketCurrency>;
		m_MoneyDenominations = new array<string>;

		m_ClientMarketZone = new ExpansionMarketClientTraderZone;
		
		m_ATMData = new array<ref ExpansionMarketATM_Data>;
		
		if (!FileExist(EXPANSION_ATM_FOLDER) && !IsMissionClient())
		{
			MakeDirectory(EXPANSION_ATM_FOLDER);
		}
		
		LoadATMData();
		
		if (IsMissionClient())
		{
			if (!FileExist(EXPANSION_MARKET_PRESETS_FOLDER))
			{
				MakeDirectory(EXPANSION_MARKET_PRESETS_FOLDER);
			}
			
			if (!FileExist(EXPANSION_MARKET_WEAPON_PRESETS_FOLDER))
			{
				MakeDirectory(EXPANSION_MARKET_WEAPON_PRESETS_FOLDER);
			}
			
			if (!FileExist(EXPANSION_MARKET_CLOTHING_PRESETS_FOLDER))
			{
				MakeDirectory(EXPANSION_MARKET_CLOTHING_PRESETS_FOLDER);
			}
		}
		
		MarketModulePrint("ExpansionMarketModule - End");
	}
	
	// ------------------------------------------------------------
	// ExpansionMarketModule Deconstructor
	// ------------------------------------------------------------
	void ~ExpansionMarketModule()
	{
		delete m_LocalEntityInventory;
		delete m_MoneyTypes;
		delete m_MoneyDenominations;
		delete m_ClientMarketZone;
		delete m_ATMData;
	}
	
	// ------------------------------------------------------------
	// ExpansionMarketModule IsEnabled
	// ------------------------------------------------------------	
	override bool IsEnabled()
	{
		// Do not keep spammy prints on released builds
		#ifdef EXPANSIONEXPRINT
		if (!GetExpansionSettings().GetMarket().MarketSystemEnabled)
			EXPrint(ToString() + "::IsEnabled " + GetExpansionSettings().GetMarket().MarketSystemEnabled);
		#endif

		return GetExpansionSettings().GetMarket().MarketSystemEnabled;
	}
	
	// ------------------------------------------------------------
	// Override OnMissionLoaded
	// ------------------------------------------------------------
	override void OnMissionLoaded()
	{
		MarketModulePrint("OnMissionLoaded - Start");
		
		super.OnMissionLoaded();
		
		LoadMoneyPrice();
		
		MarketModulePrint("OnMissionLoaded - End");
	}
	
	// ------------------------------------------------------------
	// Override OnMissionFinish
	// ------------------------------------------------------------
	override void OnMissionFinish()
	{	
		MarketModulePrint("OnMissionFinish - Start");
		
		super.OnMissionFinish();

		m_MoneyTypes.Clear();
		m_MoneyDenominations.Clear();
		
		if (IsMissionClient())
		{
			//! Clear cached categories and traders so that they are requested from server again after (e.g.) reconnect, to make sure they are in sync
			GetExpansionSettings().GetMarket().ClearMarketCaches();
		}

		MarketModulePrint("OnMissionFinish - End");
	}

	// ------------------------------------------------------------
	// Expansion GetClientZone
	// ------------------------------------------------------------
	ExpansionMarketTraderZone GetClientZone()
	{
		return m_ClientMarketZone;
	}
	
	// ------------------------------------------------------------
	// Expansion ExpansionMarketCurrency GetMoneyPrice
	// ------------------------------------------------------------	
	ExpansionMarketCurrency GetMoneyPrice(string type)
	{
		ExpansionMarketCurrency price;
		if (m_MoneyTypes && m_MoneyTypes.Contains(type))
		{
			price = m_MoneyTypes.Get(type);
			MarketModulePrint("GetMoneyPrice - Got price: " + string.ToString(price) + "| Type: " + type);
			return price;
		}
		
		MarketModulePrint("GetMoneyPrice - Failed to get price: " + string.ToString(price) + "| Type: " + type);
		return price;
	}

	// ------------------------------------------------------------
	// Expansion LoadMoneyPrice
	// ------------------------------------------------------------
	void LoadMoneyPrice()
	{
		MarketModulePrint("LoadMoneyPrice - Start");

		if (!GetExpansionSettings())
			return;

		ExpansionMarketSettings market = GetExpansionSettings().GetMarket();
		if (!market)
			return;

		int i;
		int j;
		int min_idx;
		
		map<int, ref ExpansionMarketCategory> categories = market.GetCategories();

		foreach (int categoryID, ExpansionMarketCategory category : categories)
		{
			if (!category.IsExchange())
				continue;

			//! Loop through all the items in this category to get the different price denominations
			//! It's OK to not use GetItems() here since we don't need variants
			foreach (ExpansionMarketItem marketItem: category.Items)
			{
				ExpansionMarketCurrency worth = marketItem.MinPriceThreshold;
				
				Print(marketItem);
				Print(worth);
							
				string name = marketItem.ClassName;
				
				name.ToLower();
				
				m_MoneyTypes.Insert(name, worth);
				m_MoneyDenominations.Insert(name);
			}
		}

		//! Sort lowest to highest value currency
		for (i = 0; i < m_MoneyDenominations.Count() - 1; i++) 
		{
			min_idx = i;
			for (j = i + 1; j < m_MoneyDenominations.Count(); j++) 
			{
				ExpansionMarketCurrency jMoney = GetMoneyPrice(m_MoneyDenominations[j]);
				ExpansionMarketCurrency minIndexMoney = GetMoneyPrice(m_MoneyDenominations[min_idx]);
				if (jMoney < minIndexMoney)
				{
					min_idx = j;
				}
			}

			m_MoneyDenominations.SwapItems(min_idx, i);
		}

		//! Invert so array is now ordered from highest to lowest value currency
		m_MoneyDenominations.Invert();

		MarketModulePrint("LoadMoneyPrice - End");
	}
	
	// ------------------------------------------------------------
	// Expansion SetTrader
	// Client only
	// ------------------------------------------------------------
	void SetTrader(ExpansionTraderObjectBase trader, bool complete)
	{
		MarketModulePrint("SetTrader - Start");
	
		if (complete)
			trader.GetTraderMarket().m_StockOnly = true;
		m_OpenedClientTrader = trader;
		SI_SetTraderInvoker.Invoke(trader, complete);
		
		MarketModulePrint("SetTrader - End");
	}
	
	// ------------------------------------------------------------
	// Expansion GetTrader
	// ------------------------------------------------------------
	ExpansionTraderObjectBase GetTrader()
	{
		MarketModulePrint("GetTrader - Start");
		
		if (IsMissionClient())
		{
			MarketModulePrint("GetTrader - End and return!");
			
			return m_OpenedClientTrader;
		}

		Error( "GetTrader - Invalid operation" );
		MarketModulePrint("GetTrader - [ERROR]: End and NULL!");
				
		return NULL;
	}
	
	// ------------------------------------------------------------
	// Expansion FindSellPrice
	// ------------------------------------------------------------
	//! Check if item can be sold. `ExpansionMarketResult result` indicates reason if cannot be sold.
	bool FindSellPrice(notnull PlayerBase player, array<EntityAI> items, int stock, int amountWanted, ExpansionMarketSell sell, out ExpansionMarketResult result = ExpansionMarketResult.Success)
	{
		MarketModulePrint("FindSellPrice - Start - stock " + stock + " wanted " + amountWanted);
		
		if (!player)
		{
			Error("FindSellPrice - [ERROR]: Player Base is NULL!");
			result = ExpansionMarketResult.FailedUnknown;
			return false;
		}
		
		if (amountWanted < 0)
		{
			Error("FindSellPrice - [ERROR]: Amount wanted is smaller then 0: " + amountWanted);
			result = ExpansionMarketResult.FailedUnknown;
			return false;
		}
		
		if (!sell)
		{
			Error("FindSellPrice - [ERROR]: ExpansionMarketSell is NULL!");
			result = ExpansionMarketResult.FailedUnknown;
			return false;
		}

		if (!sell.Item)
		{		
			Error("FindSellPrice - [ERROR]: ExpansionMarketItem is NULL!");
			result = ExpansionMarketResult.FailedUnknown;
			return false;
		}
		
		if (!sell.Trader)
		{
			Error("FindSellPrice - [WARNING]: ExpansionMarketSell.Trader is NULL! Stock cannot be taken into account");
		}

		sell.Price = 0;
		sell.TotalAmount = 0;
		
		float curAddedStock;

		int increaseStockBy;

		ExpansionMarketTraderZone zone;

		if (GetGame().IsClient())
			zone = GetClientZone();
		else
			zone = sell.Trader.GetTraderZone();
		
		float initialSellPriceModifier = 1;

		if (!GetItemCategory(sell.Item).IsExchange())
		{
			float sellPricePct = sell.Item.SellPricePercent;
			if (sellPricePct < 0)
				sellPricePct = zone.SellPricePercent;
			if (sellPricePct < 0)
				sellPricePct = GetExpansionSettings().GetMarket().SellPricePercent;
			initialSellPriceModifier = sellPricePct / 100;
		}

		MarketModulePrint("FindSellPrice - player inventory: " + items.Count() + " - looking for " + sell.Item.ClassName);
		
		foreach (EntityAI itemEntity: items) 
		{
			string itemClassName = itemEntity.GetType();
			itemClassName.ToLower();
			
			if (itemClassName == sell.Item.ClassName)
			{
				if (!sell.Trader.GetTraderMarket().CanSellItem(itemClassName))
				{
					EXPrint(ToString() + "::FindSellPrice - Cannot sell " + itemClassName + " to " + sell.Trader.GetTraderMarket().TraderName + " trader");
					result = ExpansionMarketResult.FailedCannotSell;
					return false;
				}

				int playerInventoryAmount = GetItemAmount(itemEntity);
				int amountTaken;  //! Amount taken from inventory
				int amountLeft;   //! Amount left in inventory after deducting taken
				
				//! If the item is stackable then we want to remove the wanted amount from this item pile.
				if (playerInventoryAmount >= 1)
				{
					//! If the item pile contains more or exacly the amount of units we wanted then we take that requested amount from that pile.
					if (playerInventoryAmount >= amountWanted)
					{
						amountTaken = amountWanted;
					}
					else if (playerInventoryAmount < amountWanted)
					{
						amountTaken = playerInventoryAmount;
					}
				}
				else if (playerInventoryAmount <= 0)
				{
					//! Can't sell this. Either the item is excluded bacause it has inventory/attachments,
					//! is a magazine that still contains ammo, or is depleted like a batttery or food.
					continue;
				}

				amountLeft = playerInventoryAmount - amountTaken;
				amountWanted -= amountTaken;
								
				MarketModulePrint("FindSellPrice - original amount in inventory: " + playerInventoryAmount);
				MarketModulePrint("FindSellPrice - amount taken: " + amountTaken);
				MarketModulePrint("FindSellPrice - amount left in inventory: " + amountLeft);
				MarketModulePrint("FindSellPrice - amount still wanted: " + amountWanted);
				
				increaseStockBy += amountTaken;

				//! Add all attachments first (and attachments of attachments)
				FindAttachmentsSellPrice(itemEntity, sell);

				float incrementStockModifier;
				float modifier = GetSellPriceModifier(itemEntity, incrementStockModifier, initialSellPriceModifier);

				sell.AddItem(amountLeft, amountTaken, incrementStockModifier, itemEntity);
				sell.TotalAmount += amountTaken;

				for (int j = 0; j < amountTaken; j++)
				{
					if (!sell.Item.IsStaticStock())
						curAddedStock += incrementStockModifier;

					sell.Price += sell.Item.CalculatePrice(stock + curAddedStock, modifier);
				}

				if (amountWanted == 0)
				{
					MarketModulePrint("FindSellPrice - End and return true");
					return true;
				}
			}
		}
		
		result = ExpansionMarketResult.FailedNotInPlayerPossession;

		MarketModulePrint("FindSellPrice - End and return false");
		return false;
	}

	void FindAttachmentsSellPrice(EntityAI itemEntity, ExpansionMarketSell sell, inout map<string, float> addedStock = NULL)
	{
		ExpansionMarketTraderZone zone;

		if (GetGame().IsClient())
			zone = GetClientZone();
		else
			zone = sell.Trader.GetTraderZone();

		float initialSellPriceModifier = 1;

		if (!GetItemCategory(sell.Item).IsExchange())
		{
			float sellPricePct = sell.Item.SellPricePercent;
			if (sellPricePct < 0)
				sellPricePct = zone.SellPricePercent;
			if (sellPricePct < 0)
				sellPricePct = GetExpansionSettings().GetMarket().SellPricePercent;
			initialSellPriceModifier = sellPricePct / 100;
		}

		if (!addedStock)
			addedStock = new map<string, float>;

		int i;

		if (itemEntity.IsInherited(MagazineStorage))
		{
			//! Magazines
			Magazine mag;
			Class.CastTo(mag, itemEntity);

			map<string, float> incrementStockModifiers = new map<string, float>;

			for (i = 0; i < mag.GetAmmoCount(); i++)
			{
				float damage;  //! NOTE: Damage is the damage of the cartridge itself (0..1), NOT the damage it inflicts!
				string cartTypeName;
				mag.GetCartridgeAtIndex(i, damage, cartTypeName);
				//EXPrint("Bullet: " + cartTypeName + ", " + "Damage: "+ damage.ToString());
				float incrementStockModifierAdd = initialSellPriceModifier * (1 - damage);
				float incrementStockModifierCur;
				if (incrementStockModifiers.Find(cartTypeName, incrementStockModifierCur))
					incrementStockModifiers.Set(cartTypeName, incrementStockModifierCur + incrementStockModifierAdd);
				else
					incrementStockModifiers.Insert(cartTypeName, incrementStockModifierAdd);
			}

			foreach (string bulletClassName, float incrementStockModifier: incrementStockModifiers)
			{
				ExpansionMarketItem ammoItem = NULL;
				if (!s_AmmoItems.Find(bulletClassName, ammoItem))
				{
					string ammoClassName = GetGame().ConfigGetTextOut("CfgAmmo " + bulletClassName + " spawnPileType");
					ammoClassName.ToLower();
					ammoItem = GetExpansionSettings().GetMarket().GetItem(ammoClassName, false);
					if (!ammoItem)
						EXPrint("FindAttachmentsSellPrice - market item " + ammoClassName + " (" + bulletClassName + ") does not exist");
					s_AmmoItems.Insert(bulletClassName, ammoItem);
				}
				if (ammoItem)
					FindAttachmentsSellPriceInternal(ammoItem, NULL, sell, addedStock, zone, incrementStockModifier);
			}

			return;
		}

		//! Everything else

		if (!itemEntity.GetInventory())
			return;

		for (i = 0; i < itemEntity.GetInventory().AttachmentCount(); i++)
		{
			EntityAI attachmentEntity = itemEntity.GetInventory().GetAttachmentFromIndex(i);

			if (!attachmentEntity)
				continue;

			string attachmentName = attachmentEntity.GetType();

			attachmentName.ToLower();

			ExpansionMarketItem attachment = GetExpansionSettings().GetMarket().GetItem(attachmentName, false);

			if (!attachment)
				continue;

			FindAttachmentsSellPriceInternal(attachment, attachmentEntity, sell, addedStock, zone, initialSellPriceModifier);
		}
	}

	protected void FindAttachmentsSellPriceInternal(ExpansionMarketItem attachment, EntityAI attachmentEntity, ExpansionMarketSell sell, inout map<string, float> addedStock, ExpansionMarketTraderZone zone, float initialSellPriceModifier)
	{
		if (!sell.Trader.GetTraderMarket().CanSellItem(attachment.ClassName))
		{
			EXPrint(ToString() + "::FindSellPrice - Cannot sell " + attachment.ClassName + " to " + sell.Trader.GetTraderMarket().TraderName + " trader");
			return;
		}

		int stock = 1;
		float curAddedStock;

		float incrementStockModifier;
		float modifier = initialSellPriceModifier;

		if (attachmentEntity)
			modifier = GetSellPriceModifier(attachmentEntity, incrementStockModifier, initialSellPriceModifier);
		else
			incrementStockModifier = initialSellPriceModifier;

		if (!attachment.IsStaticStock())
		{
			stock = zone.GetStock(attachment.ClassName);

			if (stock < 0)  //! Attachment does not exist in trader zone - set min stock so price calc can work correctly
				stock = attachment.MinStockThreshold;

			if (addedStock.Find(attachment.ClassName, curAddedStock))
				addedStock.Set(attachment.ClassName, curAddedStock + incrementStockModifier);
			else
				addedStock.Insert(attachment.ClassName, incrementStockModifier);

			curAddedStock += incrementStockModifier;
		}

		sell.AddItem(0, 1, incrementStockModifier, attachmentEntity, attachment.ClassName);

		sell.Price += attachment.CalculatePrice(stock + curAddedStock, modifier);

		if (attachmentEntity)
			FindAttachmentsSellPrice(attachmentEntity, sell, addedStock);
	}

	//! Get sell price modifier, taking into account item condition (including quantity and food stage for food)
	//! Ruined items will always return a sell price modifier of 0.0
	//! If sell price modifier reaches zero, `incrementStockModifier` will be zero as well, otherwise <modifier> for non-ruined non-food items,
	//! and from 0.0 to <modifier> for food items, depending on quantity and food stage (when applicable, non-raw food will have a value of 0.0).
	static float GetSellPriceModifier(EntityAI item, out float incrementStockModifier, float modifier = 0.75)
	{
		switch (item.GetHealthLevel())
		{
			case GameConstants.STATE_PRISTINE:
				break;

			case GameConstants.STATE_WORN:
				modifier *= 0.75;
				break;

			case GameConstants.STATE_DAMAGED:
				modifier *= 0.5;
				break;

			case GameConstants.STATE_BADLY_DAMAGED:
				modifier *= 0.25;
				break;

			case GameConstants.STATE_RUINED:
				modifier = 0;
				break;
		}

		//! Selling ruined items shall not increase stock
		incrementStockModifier = modifier > 0;  //! 0.0 or 1.0

		if (modifier && item.IsKindOf("Edible_Base"))
		{
			//! Food and liquid containers
			Edible_Base edible = Edible_Base.Cast(item);

			float minFactor;
			if (!edible.ConfigGetBool("varQuantityDestroyOnMin"))
				minFactor = 0.25;  //! Quarter price at zero quantity for e.g. liquid containers

			if (edible.HasQuantity() && !edible.IsFullQuantity())  //! Full modifier at full quantity
				modifier = ExpansionMath.LinearConversion(0.0, 1.0, edible.GetQuantity() / edible.GetQuantityMax(), modifier * minFactor, modifier, true);

			if (edible.HasFoodStage())
			{
				switch (edible.GetFoodStageType())
				{
					case FoodStageType.RAW:
						//! Let quantity and condition influence stock increment modifier
						incrementStockModifier = modifier;
						break;

					//! Selling non-raw food shall not increase stock

					case FoodStageType.BAKED:
						modifier *= 0.75;
						incrementStockModifier = 0;
						break;

					case FoodStageType.BOILED:
						modifier *= 0.5;
						incrementStockModifier = 0;
						break;

					case FoodStageType.DRIED:
						modifier *= 0.25;
						incrementStockModifier = 0;
						break;

					case FoodStageType.BURNED:
					case FoodStageType.ROTTEN:
						modifier = 0;
						incrementStockModifier = 0;
						break;
				}
			}
		}

		EXPrint("GetSellPriceModifier " + item.ToString() + " -> " + modifier + " incrementStock " + incrementStockModifier);

		return modifier;
	}
	
	// ------------------------------------------------------------
	// Expansion GetItemAmount
	// Returns the amount of the given item the player has, taking into account whether it's stackable or not.
	// If the item is not sellable (might be a permanent condition because it's ruined
	// or a temporary one because it's attached to something or has cargo) returns -1
	// If the item is not stackable returns 1.
	// ------------------------------------------------------------
	int GetItemAmount(EntityAI item)
	{
		int amount = -1;
		
		ItemBase itemBase = ItemBase.Cast(item);
		
		MarketModulePrint("GetItemAmount - Item type:" + item.GetType());
		
		if (!CanSellItem(item))
			return -1;
		
		if (item.IsKindOf("Container_Base"))
		{
			amount = 1;
		}
		else if (item.IsKindOf("ExpansionSpraycanBase"))
		{
			amount = 1;
		}
		else if (item.IsKindOf("Edible_Base"))
		{
			//! Food and liquid containers
			amount = 1;
		}
		else if (item.IsInherited(MagazineStorage))
		{
			amount = 1;
		}
		else if (item.IsKindOf("Ammunition_Base"))
		{
			if (item.IsAmmoPile())
			{
				MarketModulePrint("GetItemAmount - Ammo Pile");
				//! This looks like a wierd method but this how we get the actual ammo amount from an ammo pile
				Magazine magazine = Magazine.Cast(item);
				amount = magazine.GetAmmoCount();
			}
		}
		else if (item.HasEnergyManager())
		{
			MarketModulePrint("GetItemAmount - Battery");
			amount = 1;
		}
		else if (itemBase && itemBase.ConfigGetBool("canBeSplit"))
		{
			MarketModulePrint("GetItemAmount - Stackable");
			amount = item.GetQuantity();
		}
		else
		{
			MarketModulePrint("GetItemAmount - Else");
			amount = 1;
		}
		
		return amount;
	}
	
	// ------------------------------------------------------------
	// Expansion Bool FindPurchasePriceAndReserve
	// ------------------------------------------------------------
	private bool FindPurchasePriceAndReserve(ExpansionMarketItem item, int amountWanted, out ExpansionMarketReserve reserved, bool includeAttachments = true, out ExpansionMarketResult result = ExpansionMarketResult.Success)
	{
		MarketModulePrint("FindPurchasePriceAndReserve - Start");		
					
		if (!item)
		{		
			Error("FindPurchasePriceAndReserve - [ERROR]: ExpansionMarketItem is NULL!");
			return false;
		}
		
		ExpansionMarketTraderZone zone = reserved.Trader.GetTraderZone();
		
		if (!zone)
		{	
			Error("FindPurchasePriceAndReserve - [ERROR]: ExpansionMarketTraderZone is NULL!");
			return false;
		}

		ExpansionMarketTrader trader = reserved.Trader.GetTraderMarket();
		
		if (!trader)
		{	
			Error("FindPurchasePriceAndReserve - [ERROR]: ExpansionMarketTrader is NULL!");
			return false;
		}
		
		if (amountWanted < 0)
		{
			Error("FindPurchasePriceAndReserve - [ERROR]: Amount wanted is smaller then 0: " + amountWanted);
			return false;
		}
		
		MarketModulePrint("FindPurchasePriceAndReserve - Amount wanted: " + amountWanted);
		
		ExpansionMarketCurrency price;
		if (!FindPriceOfPurchase(item, zone, trader, amountWanted, price, includeAttachments, result, reserved))
		{
			MarketModulePrint("FindPurchasePriceAndReserve - ExpansionMarketItem " + item.ClassName + " is out of stock or item is set to not be buyable! End and return false!");
			reserved.ClearReserved(zone);	
			return false;
		}

		MarketModulePrint("FindPurchasePriceAndReserve - price: " + string.ToString(price));
		
		reserved.RootItem = item;
		reserved.TotalAmount = amountWanted;
					
		MarketModulePrint("FindPurchasePriceAndReserve - End and return true!");		

		return true;
	}
	
	// ------------------------------------------------------------
	// Expansion Bool FindPriceOfPurchase
	// ------------------------------------------------------------
	//! Returns true if item and attachments (if any) are in stock, false otherwise
	bool FindPriceOfPurchase(ExpansionMarketItem item, ExpansionMarketTraderZone zone, ExpansionMarketTrader trader, int amountWanted, inout ExpansionMarketCurrency price, bool includeAttachments = true, out ExpansionMarketResult result = ExpansionMarketResult.Success, out ExpansionMarketReserve reserved = NULL, inout map<string, int> removedStock = NULL, int level = 0)
	{
		MarketModulePrint("FindPriceOfPurchase - Start");

		int stock;

		if (item.IsStaticStock())
			stock = 1;
		else
			stock = zone.GetStock(item.ClassName);

		if (!removedStock)
			removedStock = new map<string, int>;
		
		bool ret = true;

		MarketModulePrint("FindPriceOfPurchase - Class name: " + item.ClassName);
		MarketModulePrint("FindPriceOfPurchase - Stock: " + stock);
		MarketModulePrint("FindPriceOfPurchase - Amount wanted: " + amountWanted);
		
		if (amountWanted > stock && !item.IsStaticStock())
		{
			result = ExpansionMarketResult.FailedOutOfStock;
			ret = false;
		}
		
		if (!trader.CanBuyItem(item.ClassName))
		{
			result = ExpansionMarketResult.FailedCannotBuy;
			ret = false;
		}

		int curRemovedStock;
		if (!removedStock.Find(item.ClassName, curRemovedStock))
			removedStock.Insert(item.ClassName, 0);

		float priceModifier = zone.BuyPricePercent / 100;

		ExpansionMarketCurrency currentItemBasePrice;  //! Base item price (no atts) at current stock level
		ExpansionMarketCurrency itemPrice;  //! Item price (chosen amount + atts)
		for (int i = 0; i < amountWanted; i++)
		{
			currentItemBasePrice = item.CalculatePrice(stock - curRemovedStock, priceModifier);
			price += currentItemBasePrice;
			itemPrice += currentItemBasePrice;

			if (!item.IsStaticStock())
			{
				removedStock.Set(item.ClassName, curRemovedStock + 1);

				curRemovedStock += 1;
			}

			if (includeAttachments && level < 3)
			{
				int magAmmoCount;
				map<string, bool> attachmentTypes = item.GetAttachmentTypes(magAmmoCount);
				map<string, int> magAmmoQuantities = item.GetMagAmmoQuantities(attachmentTypes, magAmmoCount);

				foreach (string attachmentName: item.SpawnAttachments)
				{
					ExpansionMarketItem attachment = GetExpansionSettings().GetMarket().GetItem(attachmentName, false);
					if (attachment)
					{
						int quantity = 1;

						//! If parent item is a mag and we are buying with ammo "attachment", set quantity to ammo quantity
						bool isMagAmmo = attachmentTypes.Get(attachmentName);
						if (isMagAmmo)
						{
							quantity = magAmmoQuantities.Get(attachmentName);
							if (!quantity)
								continue;
						}

						if (!FindPriceOfPurchase(attachment, zone, trader, quantity, price, !isMagAmmo, result, reserved, removedStock, level + 1))
						{
							if (result == ExpansionMarketResult.FailedOutOfStock)
								result = ExpansionMarketResult.FailedAttachmentOutOfStock;
							ret = false;
						}
					}
				}
			}
		}
				
		if (ret && reserved)
			reserved.AddReserved(zone, item.ClassName, amountWanted, itemPrice);

		if (ret)
			MarketModulePrint("FindPriceOfPurchase - End and return true!");
		else
			MarketModulePrint("FindPriceOfPurchase - End and return false! Zone stock is lower then requested amount or item is set to not be buyable!");
		
		return ret;
	}

	// ------------------------------------------------------------
	// Expansion Array<Object> Spawn
	// ------------------------------------------------------------
	array<Object> Spawn(ExpansionMarketReserve reserve, PlayerBase player, inout EntityAI parent, bool includeAttachments = true, int skinIndex = -1, inout bool attachmentNotAttached = false)
	{		
		MarketModulePrint("Spawn - Start");
				
		array< Object > objs = new array<Object>;

		ExpansionMarketItem item = reserve.RootItem;
		if (!item)
		{		
			MarketModulePrint("Spawn - End and return objects: " + objs.ToString() );		
			
			return objs;
		}

		int quantity = reserve.TotalAmount;
		while (quantity > 0)
		{
			int quantityBefore = quantity;

			vector position = "0 0 0";
			vector orientation = "0 0 0";
			if (item.IsVehicle())
			{
				if ( !reserve.Trader.HasVehicleSpawnPosition(item.ClassName, position, orientation) )
				{
					quantity--;
					continue;
				}
			}

			Object obj = Spawn(reserve.Trader.GetTraderMarket(), item, player, parent, position, orientation, quantity, includeAttachments, skinIndex, 0, attachmentNotAttached);

			objs.Insert(obj);

			if (!obj)
			{
				Error("Error: Couldn't spawn " + item.ClassName);
				break;
			}

			if (quantity == quantityBefore)
			{
				//! Should not be possible, just in case...
				Error("Error: Spawning " + item.ClassName + " did not affect remaining amount!");
				break;
			}
		}		
		
		MarketModulePrint("Spawn - End and return objects: " + objs.ToString());
		
		return objs;
	}
	
	// ------------------------------------------------------------
	// Expansion Object Spawn
	// ------------------------------------------------------------
	Object Spawn(ExpansionMarketTrader trader, ExpansionMarketItem item, PlayerBase player, inout EntityAI parent, vector position, vector orientation, out int quantity, bool includeAttachments = true, int skinIndex = -1, int level = 0, inout bool attachmentNotAttached = false)
	{		
		MarketModulePrint("Spawn - Start");

		Object obj;

		if (!item.IsVehicle())
			obj = ExpansionItemSpawnHelper.SpawnOnParent( item.ClassName, player, parent, quantity, NULL, skinIndex, false );
		else
			obj = ExpansionItemSpawnHelper.SpawnVehicle( item.ClassName, player, parent, position, orientation, quantity, NULL, skinIndex );
		
		//! Now deal with attachments and attachments on attachments
		if (obj && includeAttachments && level < 3)
		{
			EntityAI objEntity = EntityAI.Cast(obj);
			if (objEntity)
			{
				int magAmmoCount;
				map<string, bool> attachmentTypes = item.GetAttachmentTypes(magAmmoCount);

				foreach (string attachmentName: item.SpawnAttachments)
				{
					ExpansionMarketItem attachment = GetExpansionSettings().GetMarket().GetItem(attachmentName);
					if (attachment)
					{
						bool isMagAmmoTmp = attachmentTypes.Get(attachmentName);
						if (isMagAmmoTmp)
							continue;  //! Ammo "attachment" on mag

						//! Everything else
						int attachmentQuantity = 1;
						if (!Spawn(trader, attachment, player, objEntity, position, orientation, attachmentQuantity, true, skinIndex, level + 1))
							attachmentNotAttached = true;
					}
				}

				if (objEntity.IsInherited(ExpansionTemporaryOwnedContainer))
					parent = objEntity;

				MagazineStorage mag;
				if (Class.CastTo(mag, obj) && magAmmoCount > 0)
				{
					//! Fill up mag. If we have several ammo types, alternate them.
					int totalAmmo;
					while (totalAmmo < mag.GetAmmoMax())
					{
						foreach (string ammoName, bool isMagAmmo: attachmentTypes)
						{
							if (isMagAmmo)
							{
								string bulletName = "";
								if (!s_AmmoBullets.Find(ammoName, bulletName))
								{
									bulletName = GetGame().ConfigGetTextOut("CfgMagazines " + ammoName + " ammo");
									s_AmmoBullets.Insert(ammoName, bulletName);
								}

								if (bulletName)
								{
									mag.ServerStoreCartridge(0, bulletName);
								}

								totalAmmo++;
								if (totalAmmo == mag.GetAmmoMax())
									break;
							}
						}
					}

					mag.SetSynchDirty();
				}
			}
		}

		MarketModulePrint("Spawn - End and return " + obj);
		
		return obj;
	}

	// ------------------------------------------------------------
	// Expansion MarketMessageGM
	// ------------------------------------------------------------
	void MarketMessageGM(string message)
	{	
		MarketModulePrint("MarketMessageGM - Start");
			
		GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", message, ""));
			
		MarketModulePrint("MarketMessageGM - End");
		
	}
	
	// ------------------------------------------------------------
	// Expansion SpawnMoney
	// ------------------------------------------------------------
	array<ItemBase> SpawnMoney(PlayerBase player, inout EntityAI parent, ExpansionMarketCurrency amount, bool useExisingStacks = true, ExpansionMarketItem marketItem = NULL, ExpansionMarketTrader trader = NULL)
	{	
		MarketModulePrint("SpawnMoney - Start");	
		
		array<ItemBase> monies = new array<ItemBase>;

		array<ref array<ItemBase>> foundMoney;

		if (useExisingStacks)
		{
			//! Will increment existing stacks and only spawn new money when needed
			foundMoney = new array<ref array<ItemBase>>;

			foreach (string moneyName: m_MoneyDenominations)
			{
				foundMoney.Insert(new array<ItemBase>);
			}

			ItemBase existingMoney;

			array<EntityAI> items = new array<EntityAI>;
			player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

			foreach (EntityAI item: items)
			{
				if (Class.CastTo(existingMoney, item) && existingMoney.ExpansionIsMoney())
				{
					string existingType = existingMoney.GetType();
					existingType.ToLower();

					//! Ignore currencies this trader does not accept
					if (trader && trader.Currencies.Find(existingType) == -1)
						continue;

					int idx = m_MoneyDenominations.Find(existingType);
					MarketModulePrint("SpawnMoney: Found " + existingMoney.GetQuantity() + " " + existingType + " worth " + GetMoneyPrice(existingType) + " a piece on player");
					foundMoney[idx].Insert(existingMoney);
				}
			}
		}
		
		ExpansionMarketCurrency remainingAmount = amount;
		int lastCurrencyIdx = m_MoneyDenominations.Count() - 1;
		ExpansionMarketCurrency minAmount = GetMoneyPrice(m_MoneyDenominations[lastCurrencyIdx]);

		for (int currentDenomination = 0; currentDenomination < m_MoneyDenominations.Count(); currentDenomination++)
		{
			string type = m_MoneyDenominations[currentDenomination];

			//! Ignore currencies this trader does not accept
			if (trader && trader.Currencies.Find(type) == -1)
				continue;

			if (marketItem && GetItemCategory(marketItem).IsExchange() && currentDenomination < lastCurrencyIdx && type == marketItem.ClassName)
			{
				//! Apply exchange logic, exclude this currency
				continue;
			}

			ExpansionMarketCurrency denomPrice = GetMoneyPrice(type);
			ExpansionMarketCurrency divAmount = remainingAmount / denomPrice;

			int toSpawn = Math.Floor(divAmount);

			ExpansionMarketCurrency amountSpawned = denomPrice * toSpawn;

			if (divAmount < 1)
				continue;

			remainingAmount -= amountSpawned;

			MarketModulePrint("SpawnMoney - need to spawn " + toSpawn + " " + type);

			while (toSpawn > 0)
			{
				ItemBase money = NULL;
				int stack = 0;

				if (useExisingStacks)
				{
					MarketModulePrint("SpawnMoney - check for existing stack of " + type);
					array<ItemBase> existingMonies = foundMoney[currentDenomination];
					for (int i = 0; i < existingMonies.Count(); i++)
					{
						existingMoney = existingMonies[i];
						if (existingMoney.GetQuantity() < existingMoney.GetQuantityMax())
						{
							//! Player already has money of that type that's not full quantity, increase existing stack
							money = existingMoney;
							stack = existingMoney.GetQuantity();
							MarketModulePrint("SpawnMoney - player has " + stack + " " + type);
							existingMonies.Remove(i);
							break;
						}
					}
				}

				if (!money)
				{
					//! Create new money item in player inventory (or in temporary storage if inventory full)
					MarketModulePrint("SpawnMoney - spawning " + type);
					money = ItemBase.Cast(ExpansionItemSpawnHelper.SpawnInInventorySecure(type, player, parent));
				}

				if (money)
				{
					int max = money.GetQuantityMax();

					if (stack + toSpawn <= max)
					{
						money.SetQuantity(stack + toSpawn);

						toSpawn = 0;
					}
					else
					{
						money.SetQuantity(max);

						toSpawn -= max - stack;
					}

					monies.Insert(money);
				} 
				else
				{
					//! Force this loop to end
					toSpawn = 0;
				}
			}

			if (remainingAmount < minAmount)
				break;
		}	
		
		MarketModulePrint("SpawnMoney - money: " + monies.ToString());
		
		return monies;
	}
	
	// From Tyler so CallFunctionParams works
	array<ItemBase> SpawnMoneyEx(PlayerBase player, inout EntityAI parent, int amount, bool useExistingStacks = true, ExpansionMarketItem marketItem = NULL, ExpansionMarketTrader trader = NULL)
	{
		return SpawnMoney(player, parent, amount, useExistingStacks, marketItem, trader);
	}

	// ------------------------------------------------------------
	// Expansion Bool FindMoneyAndCountTypes
	// ------------------------------------------------------------
	//! Return true if total <amount> monies have been found, false otherwise
	//! If player is given, use money in player inventory (if enough), otherwise use amounts that would be needed
	//! Out array <monies> contains needed amounts for each money type to reach total <amount>
	//! Note: Out array <monies> is always ordered from highest to lowest value currency
	bool FindMoneyAndCountTypes(PlayerBase player, ExpansionMarketCurrency amount, out array<int> monies, bool reserve = false, ExpansionMarketItem marketItem = NULL, ExpansionMarketTrader trader = NULL)
	{	
		MarketModulePrint("FindMoneyAndCountTypes - player " + player + " - amount " + amount);

		if (amount < 0)
		{
			MarketModulePrint("FindMoneyAndCountTypes - amount is not a positive value - end and return false!");
			return false;
		}

		if (!monies)
			monies = new array<int>;
		
		if (amount == 0)
			return true;

		array<ref array<ItemBase>> foundMoney = new array<ref array<ItemBase>>;
		array<int> playerMonies = new array<int>;
		
		foreach (string moneyName: m_MoneyDenominations)
		{
			MarketModulePrint("FindMoneyAndCountTypes: " + moneyName);
			foundMoney.Insert(new array<ItemBase>);
			monies.Insert(0);
			playerMonies.Insert(0);
		}

		ItemBase money;
		int playerWorth;

		if (player)
		{
			array<EntityAI> items = new array<EntityAI>;
			player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

			foreach (EntityAI item: items)
			{
				if (Class.CastTo(money, item) && money.ExpansionIsMoney())
				{
					string type = money.GetType();
					type.ToLower();

					//! Ignore currencies this trader does not accept
					if (trader && trader.Currencies.Find(type) == -1)
						continue;

					int idx = m_MoneyDenominations.Find(type);
					MarketModulePrint("FindMoneyAndCountTypes: Found " + money.GetQuantity() + " " + type + " worth " + GetMoneyPrice(type) + " a piece on player ");
					foundMoney[idx].Insert(money);
					playerMonies[idx] = playerMonies[idx] + money.GetQuantity();
					playerWorth += GetMoneyPrice(type) * money.GetQuantity();
				}
			}
		}

		ExpansionMarketCurrency foundAmount = 0;
		int lastCurrencyIdx = m_MoneyDenominations.Count() - 1;
		ExpansionMarketCurrency minAmount = GetMoneyPrice(m_MoneyDenominations[lastCurrencyIdx]);
		ExpansionMarketCurrency remainingAmount = amount;

		string info = "Would reserve";
		if (reserve)
			info = "Reserved";

		for (int j = 0; j < foundMoney.Count(); j++)
		{
			//! Ignore currencies this trader does not accept
			if (!player && trader && trader.Currencies.Find(m_MoneyDenominations[j]) == -1)
				continue;

			if (marketItem && GetItemCategory(marketItem).IsExchange() && j < lastCurrencyIdx && m_MoneyDenominations[j] == marketItem.ClassName)
			{
				//! Apply exchange logic, exclude this currency
				continue;
			}

			ExpansionMarketCurrency denomPrice = GetMoneyPrice(m_MoneyDenominations[j]);
			float divAmount = remainingAmount / denomPrice;
			int toReserve = Math.Floor(divAmount);
			int reserved = 0;

			int countCurrentDenom = foundMoney[j].Count();
			int checkedCurrentDenom = 0;

			while (toReserve > 0)
			{
				if (checkedCurrentDenom >= countCurrentDenom && player)
					break;

				int number = 0;

				if (!player || playerWorth < amount)
				{
					number = toReserve;
				}
				else
				{
					money = foundMoney[j][checkedCurrentDenom];
					int stack = money.GetQuantity();

					if (stack >= toReserve)
					{
						number = toReserve;
					} 
					else
					{
						number = stack;
					}

					if (reserve)
						money.ExpansionReserveMoney(number);
				}

				monies[j] = monies[j] + number;

				MarketModulePrint("FindMoneyAndCountTypes: " + info + " " + number + " " + m_MoneyDenominations[j] + " worth " + (number * denomPrice));

				toReserve -= number;
				reserved += number;
				checkedCurrentDenom++;
			}

			ExpansionMarketCurrency amountReserve = reserved * denomPrice;
			foundAmount += amountReserve;
			remainingAmount -= amountReserve;

			if (foundAmount > amount - minAmount)
			{
				return playerWorth >= amount;
			}
		}

		//! Failed to reserve exact amounts needed from each currency, but player may have actually more than needed overall
		//! Find lowest value currency that would push us to or over the required amount
		for (int i = foundMoney.Count() - 1; i >= 0; i--)
		{
			//! Ignore currencies this trader does not accept
			if (!player && trader && trader.Currencies.Find(m_MoneyDenominations[i]) == -1)
				continue;

			if (marketItem && GetItemCategory(marketItem).IsExchange() && i < lastCurrencyIdx && m_MoneyDenominations[i] == marketItem.ClassName)
			{
				//! Apply exchange logic, exclude this currency
				continue;
			}

			denomPrice = GetMoneyPrice(m_MoneyDenominations[i]);
			if ((!player || monies[i] < playerMonies[i]) && denomPrice >= remainingAmount)
			{
				if (reserve)
				{
					foreach (ItemBase existingMoney : foundMoney[i])
					{
						if (existingMoney.ExpansionGetReservedMoneyAmount() < existingMoney.GetQuantity())
							existingMoney.ExpansionReserveMoney(existingMoney.ExpansionGetReservedMoneyAmount() + 1);
					}
				}
				
				monies[i] = monies[i] + 1;
				MarketModulePrint("FindMoneyAndCountTypes: " + info + " one additional " + m_MoneyDenominations[i] + " worth " + denomPrice);
				foundAmount += denomPrice;
				remainingAmount = amount - foundAmount;
				
				for (int k = i + 1; k < foundMoney.Count(); k++)
				{
					denomPrice = GetMoneyPrice(m_MoneyDenominations[k]);
					divAmount = remainingAmount / denomPrice;
					toReserve = Math.Ceil(divAmount);
					monies[k] = toReserve;
					remainingAmount -= toReserve * denomPrice;
				}
				
				return playerWorth >= amount;
			}
			else
			{
				if (reserve)
				{
					foreach (ItemBase existingReserved : foundMoney[i])
					{
						if (existingReserved.ExpansionIsMoneyReserved())
							existingReserved.ExpansionReserveMoney(0);
					}
				}
				foundAmount -= monies[i] * denomPrice;
			}
		}

		MarketModulePrint("FindMoneyAndCountTypes - not enough money found - end and return false!");
		return false;
	}

	// Tyler requested, <3
	bool FindMoneyAndCountTypesEx(PlayerBase player, int amount, out array<int> monies, bool reserve = false, ExpansionMarketItem marketItem = NULL, ExpansionMarketTrader trader = NULL)
	{
		return FindMoneyAndCountTypes(player, amount, monies, reserve, marketItem, trader);
	}
		
	// ------------------------------------------------------------
	// Expansion Float GetPlayerWorth
	// ------------------------------------------------------------
	ExpansionMarketCurrency GetPlayerWorth(PlayerBase player, out array<int> monies, ExpansionMarketTrader trader = NULL)
	{
		m_PlayerWorth = 0;

		if (!monies)
		{
			monies = new array<int>;
		}
		else
		{
			monies.Clear();
		}

		for (int i = 0; i < m_MoneyDenominations.Count(); i++)
		{
			monies.Insert(0);
		}

		if (!player)
		{
			return m_PlayerWorth;
		}

		array<EntityAI> items = new array<EntityAI>;
	   	player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		for (int j = 0; j < items.Count(); j++)
		{
			ItemBase money;
			if (Class.CastTo(money, items[j]) && money.ExpansionIsMoney())
			{
				string type = money.GetType();
				type.ToLower();

				//! Always include all money types the player has, even if trader would not accept
				int idx = m_MoneyDenominations.Find(type);
				monies[idx] = monies[idx] + money.GetQuantity();

				//! Do not include currencies this trader does not accept in player overall worth calc
				if (trader && trader.Currencies.Find(type) == -1)
					continue;

				m_PlayerWorth += GetMoneyPrice(type) * money.GetQuantity();
			}
		}

		return m_PlayerWorth;
	}
	
	ExpansionMarketCurrency GetPlayerWorth()
	{
		return m_PlayerWorth;
	}

	// ------------------------------------------------------------
	// Expansion GetMoneyBases
	// ------------------------------------------------------------
	array<ref array<ItemBase>> GetMoneyBases(PlayerBase player)
	{
		array<ref array<ItemBase>> foundMoney = new array<ref array<ItemBase>>;
		
		for (int i = 0; i < m_MoneyDenominations.Count(); i++)
		{
			array<ItemBase> money_array = new array<ItemBase>;
			foundMoney.Insert(money_array);
		}
		
		array<EntityAI> items = new array<EntityAI>;
	   	player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		for (int j = 0; j < items.Count(); j++)
		{
			ItemBase money;
			if (Class.CastTo(money, items[j]) && money.ExpansionIsMoney())
			{
				string type = money.GetType();
				type.ToLower();

				int idx = m_MoneyDenominations.Find(type);
				MarketModulePrint("GetMoneyBases - idx: " + idx);
				MarketModulePrint("GetMoneyBases - foundMoney[idx]: " + foundMoney[idx]);
				foundMoney[idx].Insert(money);
			}
		}
		
		return foundMoney;
	}
				
	// ------------------------------------------------------------
	// Expansion UnlockMoney
	// ------------------------------------------------------------
	void UnlockMoney(PlayerBase player)
	{
		if (!player)
		{
			return;
		}

		array<EntityAI> items = new array<EntityAI>;
	   	player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		for (int i = 0; i < items.Count(); i++)
		{
			ItemBase money;
			if (Class.CastTo(money, items[i]) && money.ExpansionIsMoneyReserved())
			{
				money.ExpansionReserveMoney(0);
			}
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RemoveMoney
	// ------------------------------------------------------------
	//! Remove reserved money amounts from player, return removed price
	int RemoveMoney(PlayerBase player)
	{
		Print("RemoveMoney - Start");
		if (!player)
		{
			return 0;
		}

		int removed;

		array<EntityAI> items = new array<EntityAI>;
	   	player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		foreach (EntityAI item : items)
		{
			ItemBase money;
			if (Class.CastTo(money, item) && money.ExpansionIsMoneyReserved())
			{
				int quantity = money.GetQuantity() - money.ExpansionGetReservedMoneyAmount();
				string type = money.GetType();
				type.ToLower();
				removed += money.ExpansionGetReservedMoneyAmount() * GetMoneyPrice(type);
				Print("RemoveMoney - Removed " + money.ExpansionGetReservedMoneyAmount() + " from " + money);
				if (!quantity)
				{
					GetGame().ObjectDelete(money);
				}
				else
				{
					money.ExpansionReserveMoney(0);
					money.SetQuantity(quantity);
				}
			}
		}
		Print("RemoveMoney - End");

		return removed;
	}

	// ------------------------------------------------------------
	// Override GetRPCMin
	// ------------------------------------------------------------
	override int GetRPCMin()
	{
		return ExpansionMarketModuleRPC.INVALID;
	}
	
	// ------------------------------------------------------------
	// Override GetRPCMax
	// ------------------------------------------------------------
	override int GetRPCMax()
	{
		return ExpansionMarketModuleRPC.COUNT;
	}
	
	// ------------------------------------------------------------
	// Override OnRPC
	// ------------------------------------------------------------
	#ifdef CF_BUGFIX_REF
	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	#else
	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	#endif
	{
		switch (rpc_type)
		{
			case ExpansionMarketModuleRPC.Callback:
			{
				RPC_Callback(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.MoneyDenominations:
			{
				RPC_MoneyDenominations(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.RequestPurchase:
			{
				RPC_RequestPurchase(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.ConfirmPurchase:
			{
				RPC_ConfirmPurchase(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.CancelPurchase:
			{
				RPC_CancelPurchase(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.RequestSell:
			{
				RPC_RequestSell(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.ConfirmSell:
			{
				RPC_ConfirmSell(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.CancelSell:
			{
				RPC_CancelSell(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.RequestTraderData:
			{
				RPC_RequestTraderData(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.LoadTraderData:
			{
				RPC_LoadTraderData(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.RequestTraderItems:
			{
				RPC_RequestTraderItems(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.LoadTraderItems:
			{
				RPC_LoadTraderItems(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.RequestPlayerATMData:
			{
				RPC_RequestPlayerATMData(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.SendPlayerATMData:
			{
				RPC_SendPlayerATMData(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.RequestDepositMoney:
			{
				RPC_RequestDepositMoney(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.ConfirmDepositMoney:
			{
				RPC_ConfirmDepositMoney(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.RequestWithdrawMoney:
			{
				RPC_RequestWithdrawMoney(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.ConfirmWithdrawMoney:
			{
				RPC_ConfirmWithdrawMoney(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.RequestTransferMoneyToPlayer:
			{
				RPC_RequestTransferMoneyToPlayer(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.ConfirmTransferMoneyToPlayer:
			{
				RPC_ConfirmTransferMoneyToPlayer(ctx, sender, target);
				break;
			}
			#ifdef EXPANSIONMOD
			case ExpansionMarketModuleRPC.RequestPartyTransferMoney:
			{
				RPC_RequestPartyTransferMoney(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.ConfirmPartyTransferMoney:
			{
				RPC_ConfirmPartyTransferMoney(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.RequestPartyWithdrawMoney:
			{
				RPC_RequestPartyWithdrawMoney(ctx, sender, target);
				break;
			}
			case ExpansionMarketModuleRPC.ConfirmPartyWithdrawMoney:
			{
				RPC_ConfirmPartyWithdrawMoney(ctx, sender, target);
				break;
			}
			#endif
		}
	}
	
	/*
	 * Called Server Only: 
	*/
	// ------------------------------------------------------------
	// Expansion RemoveReservedStock
	// ------------------------------------------------------------
	private void RemoveReservedStock(PlayerBase player, string itemClassName)
	{
		if (GetGame().IsServer() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RemoveReservedStock - Start");
			
			if (!player)
				return;
	
			ExpansionMarketReserve reserve = player.GetMarketReserve();
			if (reserve)
			{
				if (player.IsMarketItemReserved(itemClassName))
				{
					EXPrint("RemoveReservedStock: FailedReserveTime");
					
					UnlockMoney(player);
					
					reserve.ClearReserved(reserve.Trader.GetTraderZone());
		
					Callback(reserve.RootItem.ClassName, ExpansionMarketResult.FailedReserveTime, player.GetIdentity());
		
					player.ClearMarketReserve();
				}
			}
			
			MarketModulePrint("RemoveReservedStock - End");
		}
	}
	
	/*
	 * Called Server Only: 
	*/
	// ------------------------------------------------------------
	// Expansion Callback
	// ------------------------------------------------------------
	void Callback(string itemClassName, ExpansionMarketResult result, PlayerIdentity playerIdent, int option = -1, Object object = NULL)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(itemClassName);
		rpc.Write(result);
		rpc.Write(option);
		rpc.Write(object);
		rpc.Send(NULL, ExpansionMarketModuleRPC.Callback, true, playerIdent);
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_Callback
	// ------------------------------------------------------------
	private void RPC_Callback(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_Callback - Start");
		
		string itemClassName;
		if (!ctx.Read(itemClassName))
		{
			Error("RPC_Callback - Could not read param string itemClassName!");
			return;
		}

		int result;
		if (!ctx.Read(result))
		{
			Error("RPC_Callback - Could not read param int result!");
			return;
		}

		int option;
		if (!ctx.Read(option))
		{
			Error("RPC_Callback - Could not read param int option!");
			return;
		}

		Object object;
		if (!ctx.Read(object))
		{
			Error("RPC_Callback - Could not read param Object object!");
			return;
		}

		MarketModulePrint("RPC_Callback - result: " + result + " option: " + option + " object: " + object);
		
		SI_Callback.Invoke(result, option, object);
		
		MarketModulePrint("RPC_Callback - End");
	}

	// -----------------------------------------------------------
	// Expansion OnInvokeConnect
	// -----------------------------------------------------------
	override void OnInvokeConnect(PlayerBase player, PlayerIdentity identity)
	{
		MarketModulePrint("OnInvokeConnect - Start");
		
		SendMoneyDenominations(identity);
		
		if (!GetPlayerATMData(identity.GetId()))
		{
			CreateATMData(identity);
		}
		
		MarketModulePrint("OnInvokeConnect - End");
	}
	
	// -----------------------------------------------------------
	// Expansion SendMoneyDenominations
	// -----------------------------------------------------------
	private void SendMoneyDenominations(PlayerIdentity identity)
	{
		if (GetGame().IsServer() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("SendMoneyDenominations - Start");
			
			ScriptRPC rpc = new ScriptRPC();

			//! Order needs to match highest to lowest currency value
			rpc.Write(m_MoneyDenominations);
			array < ExpansionMarketCurrency > values = new array< ExpansionMarketCurrency >;
			foreach ( string moneyName: m_MoneyDenominations )
			{
				values.Insert(GetMoneyPrice(moneyName));
			}
			rpc.Write(values);

			rpc.Send(NULL, ExpansionMarketModuleRPC.MoneyDenominations, true, identity);
			
			MarketModulePrint("SendMoneyDenominations - End");
		}
	}
	
	// -----------------------------------------------------------
	// Expansion RPC_MoneyDenominations
	// -----------------------------------------------------------
	private void RPC_MoneyDenominations(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_MoneyDenominations - Start");
		
		array<string> keys = new array<string>;
		array<ExpansionMarketCurrency> values = new array<ExpansionMarketCurrency>;

		if (!ctx.Read(keys) || !ctx.Read(values))
			return;

		int count = keys.Count();
		if (count != values.Count())
			return;

		m_MoneyTypes.Clear();
		m_MoneyDenominations.Clear();

		for (int i = 0; i < count; i++)
		{
			m_MoneyDenominations.Insert(keys[i]);
			m_MoneyTypes.Insert(keys[i], values[i]);
		}
		
		MarketModulePrint("RPC_MoneyDenominations - End");
	}

	/* TODO: GET MORE INFO FROM JACOB ABOUT THIS!
	 * Called Client Only: The client would send this RPC to request a purchase 
	 * to be made and to lock in the price it is at. This lock will last
	 * 30 seconds and on any new clients will show the new price as if
	 * the stock of those items were released.
	 */
	// ------------------------------------------------------------
	// Expansion RequestPurchase
	// Client only
	// ------------------------------------------------------------
	void RequestPurchase(string itemClassName, int count, ExpansionMarketCurrency currentPrice, ExpansionTraderObjectBase trader, PlayerBase player = NULL, bool includeAttachments = true, int skinIndex = -1, TIntArray attachmentIDs = NULL)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RequestPurchase - Sart");
			
			if (!trader)
				return;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(itemClassName);
			rpc.Write(count);
			rpc.Write(currentPrice);
			rpc.Write(includeAttachments);
			rpc.Write(skinIndex);
			rpc.Write(attachmentIDs);
			rpc.Send(trader.GetTraderEntity(), ExpansionMarketModuleRPC.RequestPurchase, true, NULL);
			
			MarketModulePrint("RequestPurchase - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestPurchase
	// Server only
	// ------------------------------------------------------------
	private void RPC_RequestPurchase(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestPurchase - Start");
				
		string itemClassName;
		if (!ctx.Read(itemClassName))
			return;

		int count;
		if (!ctx.Read(count))
			return;
			
		ExpansionMarketCurrency currentPrice;
		if (!ctx.Read(currentPrice))
			return;

		bool includeAttachments;
		if (!ctx.Read(includeAttachments))
			return;
		
		bool skinIndex;
		if (!ctx.Read(skinIndex))
			return;

		TIntArray attachmentIDs;
		if (!ctx.Read(attachmentIDs))
			return;

		ExpansionTraderObjectBase trader = GetTraderFromObject(target);
		if (!trader)
			return;
			
		PlayerBase player = PlayerBase.GetPlayerByUID(senderRPC.GetId());
		if (!player)
			return;
			
		Exec_RequestPurchase(player, itemClassName, count, currentPrice, trader, includeAttachments, skinIndex, attachmentIDs);
		
		MarketModulePrint("RPC_RequestPurchase - End");
	}
	
	static ExpansionTraderObjectBase GetTraderFromObject(Object obj, bool errorOnNoTrader = true)
	{
		ExpansionTraderNPCBase traderNPC;
		ExpansionTraderStaticBase traderStatic;
		ExpansionTraderZombieBase traderZombie;
		#ifdef ENFUSION_AI_PROJECT
		ExpansionTraderAIBase traderAI;
		#endif

		ExpansionTraderObjectBase trader;
		if (Class.CastTo(traderNPC, obj))
			trader = traderNPC.GetTraderObject();
		else if (Class.CastTo(traderStatic, obj))
			trader = traderStatic.GetTraderObject();
		else if (Class.CastTo(traderZombie, obj))
			trader = traderZombie.GetTraderObject();
		#ifdef ENFUSION_AI_PROJECT
		else if (Class.CastTo(traderAI, obj))
			trader = traderAI.GetTraderObject();
		#endif

		if (!obj)
		{
			//! It shouldn't be possible for this to happen, but just to be safe...
			Error("ExpansionMarketModule::GetTraderFromObject - entity is NULL!");
		}
		else if (!trader && errorOnNoTrader)
		{
			Error("ExpansionMarketModule::GetTraderFromObject - trader object of " + obj.GetType() + " (" + obj + ") at " + obj.GetPosition() + " is NULL!");
		}

		return trader;
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestPurchase
	//	Server only
	// ------------------------------------------------------------
	private void Exec_RequestPurchase(notnull PlayerBase player, string itemClassName, int count, ExpansionMarketCurrency currentPrice, ExpansionTraderObjectBase trader, bool includeAttachments = true, int skinIndex = -1, TIntArray attachmentIDs = NULL)
	{
		MarketModulePrint("Exec_RequestPurchase - Sart");
		
		if (!player)
		{
			return;
		}
		
		if (!count)
		{
			MarketModulePrint("Exec_RequestPurchase - Callback_FailedNoCount");
			
			Callback(itemClassName, ExpansionMarketResult.FailedNoCount, player.GetIdentity());
			return;
		}
		
		if (!trader)
		{
			MarketModulePrint("Exec_RequestPurchase - Callback_FailedUnknown: 1");
			
			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}
		
		MarketModulePrint("Exec_RequestPurchase - count: " + count);
		MarketModulePrint("Exec_RequestPurchase - currentPrice: " + string.ToString(currentPrice));
		
		//! Get current market Trader Zone from given trader
		ExpansionMarketSettings market = GetExpansionSettings().GetMarket();
		if (!market)
		{	
			MarketModulePrint("Exec_RequestPurchase - Callback_FailedUnknown: 2");			

			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}

		ExpansionMarketTraderZone zone = trader.GetTraderZone();
		if (!zone)
		{			
			MarketModulePrint("Exec_RequestPurchase - Callback_FailedUnknown: 3");
			
			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}

		//! Afterwards calculate the price of the items at that stock		
		ExpansionMarketReserve reservedList = player.GetMarketReserve();		
		reservedList.Trader = trader;
		
		MarketModulePrint("Exec_RequestPurchase - reservedList: " + reservedList);
		MarketModulePrint("Exec_RequestPurchase - reservedList.Trader: " + reservedList.Trader);
		
		ExpansionMarketItem item = market.GetItem(itemClassName);
		if (!item /* || !reservedList.Valid*/)
		{		
			MarketModulePrint("Exec_RequestPurchase - Callback_FailedUnknown: 4 itemClassName " + itemClassName);
			MarketModulePrint("Exec_RequestPurchase - Callback_FailedUnknown: 4");		

			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}

		//! If custom attachments are chosen, create a derivative of the market item with them and cache it
		if (attachmentIDs && attachmentIDs.Count())
		{
			ExpansionMarketItem derivative = new ExpansionMarketItem(item.CategoryID, itemClassName, item.MinPriceThreshold, item.MaxPriceThreshold, item.MinStockThreshold, item.MaxStockThreshold, NULL, item.Variants, item.SellPricePercent, item.ItemID, attachmentIDs);
			derivative.SetAttachmentsFromIDs();
			item = derivative;
		}

		//! Result if the price the player has seen and agreed to in menu doesn't match anymore
		//! the current item price of the trader because stock has changed enough to affect it
		//! (another player was quicker to get his transaction through)
		ExpansionMarketResult result = ExpansionMarketResult.FailedStockChange;

		//! Compare that price to the one the player sent
		if (!FindPurchasePriceAndReserve(item, count, reservedList, includeAttachments, result) || reservedList.Price != currentPrice)
		{
			EXPrint("Exec_RequestPurchase - Player sent price: " + currentPrice);
			EXPrint("Exec_RequestPurchase - Current stock: " + zone.GetStock(itemClassName, true));
			reservedList.Debug();

			reservedList.ClearReserved(zone);
			player.ClearMarketReserve();

			Callback(itemClassName, result, player.GetIdentity());
			
			string cbtype = typename.EnumToString(ExpansionMarketResult, result);

			MarketModulePrint("Exec_RequestPurchase - Callback " + cbtype + " Item: " + reservedList.RootItem);
			MarketModulePrint("Exec_RequestPurchase - Callback " + cbtype + " Zone: " + zone.ToString());
			MarketModulePrint("Exec_RequestPurchase - Callback " + cbtype + " Count: " + count);
			MarketModulePrint("Exec_RequestPurchase - Callback " + cbtype + ": 1");
			MarketModulePrint("Exec_RequestPurchase - Callback " + cbtype + ": itemClassName " + itemClassName);
			MarketModulePrint("Exec_RequestPurchase - Callback " + cbtype + ": player " + player);
			MarketModulePrint("Exec_RequestPurchase - Callback " + cbtype + ": identity " + player.GetIdentity());

			return;
		}
		
		UnlockMoney(player);

		array<int> monies;
		if (!FindMoneyAndCountTypes(player, reservedList.Price, monies, true, reservedList.RootItem, trader.GetTraderMarket())) // currentPrice -> reservedList.Price
		{
			UnlockMoney( player );

			reservedList.ClearReserved(zone);
			player.ClearMarketReserve();

			Callback(itemClassName, ExpansionMarketResult.FailedNotEnoughMoney, player.GetIdentity());
					
			MarketModulePrint("Callback_FailedNotEnoughMoney: 1");				
			MarketModulePrint("Callback_FailedNotEnoughMoney: player: " + player);	
			MarketModulePrint("Callback_FailedNotEnoughMoney: current price: " + string.ToString(currentPrice));
			MarketModulePrint("Callback_FailedNotEnoughMoney: reserved list price: " + string.ToString(reservedList.Price));

			return;
		}

		reservedList.Valid = true;
		reservedList.Time = GetGame().GetTime();

		// !TODO: Finish method RemoveReservedStock in PlayerBase
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(RemoveReservedStock, 30000, false, player, itemClassName);
		
		Callback(itemClassName, ExpansionMarketResult.Success, player.GetIdentity());
		
		ExpansionLogMarket(string.Format("Player \"%1\" (id=%2) has send a requested to purchase %3 x%4 from the trader \"%5 (%6)\" in market zone \"%7\" (pos=%8).", player.GetIdentity().GetName(), player.GetIdentity().GetId(), itemClassName, count, trader.GetTraderMarket().TraderName, trader.GetDisplayName(), trader.GetTraderZone().m_DisplayName, trader.GetTraderZone().Position.ToString()));
		
		MarketModulePrint("Exec_RequestPurchase - End");
	}

	/*
	 * Called Client Only: The server will finalize the transaction with the 
	 * details that were stored in RequestPurchase. This also finalizes the stock values.
	 */
	// ------------------------------------------------------------
	// Expansion ConfirmPurchase
	// ------------------------------------------------------------
	void ConfirmPurchase(string itemClassName, PlayerBase player = NULL, bool includeAttachments = true, int skinIndex = -1)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("ConfirmPurchase - Start");
			
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(itemClassName);
			rpc.Write(includeAttachments);
			rpc.Write(skinIndex);
			rpc.Send(NULL, ExpansionMarketModuleRPC.ConfirmPurchase, true, NULL);
			
			MarketModulePrint("ConfirmPurchase - End");
		}
	}
		
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmPurchase
	// ------------------------------------------------------------
	private void RPC_ConfirmPurchase(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_ConfirmPurchase - Start");
		
		string itemClassName;
		if (!ctx.Read(itemClassName))
			return;

		bool includeAttachments;
		if (!ctx.Read(includeAttachments))
			return;
		
		int skinIndex;
		if (!ctx.Read(skinIndex))
			return;

		PlayerBase player = PlayerBase.GetPlayerByUID(senderRPC.GetId());
		if (!player)
			return;

		Exec_ConfirmPurchase(player, itemClassName, includeAttachments, skinIndex);
		
		MarketModulePrint("RPC_ConfirmPurchase - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmPurchase
	// ------------------------------------------------------------
	private void Exec_ConfirmPurchase(notnull PlayerBase player, string itemClassName, bool includeAttachments = true, int skinIndex = -1)
	{
		MarketModulePrint("Exec_ConfirmPurchase - Start");
		
		ExpansionMarketReserve reserve = player.GetMarketReserve();
		if (!reserve)
		{
			MarketModulePrint("Exec_ConfirmPurchase - Callback_FailedUnknown: Could not get reserved data from player!");
			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}
		
		if (!reserve.Trader || !reserve.Trader.GetTraderEntity())
		{
			MarketModulePrint("Exec_ConfirmPurchase - Callback_FailedUnknown: Could not get trader data from reserved data!");
			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}
		
		ExpansionMarketTraderZone zone = reserve.Trader.GetTraderZone();
		if (!zone)
		{
			MarketModulePrint("Exec_ConfirmPurchase - Callback_FailedUnknown: Could not get trader zone data from reserved trader data!");
			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}
		
		if (vector.Distance(player.GetPosition(), reserve.Trader.GetTraderEntity().GetPosition()) > MAX_TRADER_INTERACTION_DISTANCE)
		{
			MarketModulePrint("Exec_ConfirmPurchase - Callback_FailedTooFarAway: Player is too far from trader!");

			ClearReserved(player, true);

			Callback(itemClassName, ExpansionMarketResult.FailedTooFarAway, player.GetIdentity());
			return;
		}

		if (!player.IsMarketItemReserved(itemClassName))
		{
			EXPrint("Exec_ConfirmPurchase - Callback_FailedReserveTime: Could not get reserved item data from player!");

			ClearReserved(player, true);

			Callback(itemClassName, ExpansionMarketResult.FailedReserveTime, player.GetIdentity());
			return;
		}

		ExpansionMarketItem item = reserve.RootItem;
		if (!item)
		{
			MarketModulePrint("Exec_ConfirmPurchase - Callback_FailedUnknown: Could not get item data from market!");

			ClearReserved(player, true);

			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}

		vector spawnPos;
		vector spawnDir;
		ExpansionMarketVehicleSpawnType spawnType;
		ExpansionMarketResult result;
		Object blockingObject;
		if (item.IsVehicle() && !reserve.Trader.HasVehicleSpawnPosition(itemClassName, spawnPos, spawnDir, spawnType, result, blockingObject, reserve.TotalAmount))
		{
			MarketModulePrint("Exec_ConfirmPurchase - HasVehicleSpawnPosition - Type: " + typename.EnumToString(ExpansionMarketVehicleSpawnType, spawnType));
			MarketModulePrint("Exec_ConfirmPurchase - HasVehicleSpawnPosition - Callback: " + typename.EnumToString(ExpansionMarketResult, result));

			ClearReserved(player, true);

			Callback(itemClassName, result, player.GetIdentity(), spawnType, blockingObject);
			return;
		}

		int removed = RemoveMoney(player);

		MarketModulePrint("Exec_ConfirmPurchase - Total money removed: " + removed);
		MarketModulePrint("Exec_ConfirmPurchase - Change owed to player: " + (removed - reserve.Price));

		EntityAI parent = player;

		if (removed - reserve.Price > 0)
		{
			SpawnMoney(player, parent, removed - reserve.Price, true, NULL, reserve.Trader.GetTraderMarket());
		}
		
		array<Object> objs = new array<Object>;
		bool attachmentNotAttached;
		objs = Spawn(reserve, player, parent, includeAttachments, skinIndex, attachmentNotAttached);
		
		MarketModulePrint("objs : " + objs);
		MarketModulePrint("Exec_ConfirmPurchase " + reserve.RootItem.ClassName + " " + reserve.TotalAmount + " " + reserve.Reserved.Count());
		
		foreach (ExpansionMarketReserveItem currentReservedItem: reserve.Reserved)
		{
			zone.RemoveStock(currentReservedItem.ClassName, currentReservedItem.Amount, false);
		}
		
		ExpansionNotification("STR_EXPANSION_TRADER_PURCHASE_SUCCESS", new StringLocaliser("STR_EXPANSION_TRADER_PURCHASE_TEXT", ExpansionStatic.GetItemDisplayNameWithType(reserve.RootItem.ClassName), reserve.TotalAmount.ToString() + "x", reserve.Price.ToString()), EXPANSION_NOTIFICATION_ICON_INFO, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 3, ExpansionNotificationType.MARKET).Create(player.GetIdentity());
		
		CheckSpawn(player, parent, attachmentNotAttached);

		ExpansionLogMarket(string.Format("Player \"%1\" (id=%2) has bought %3 x%4 from the trader \"%5 (%6)\" in market zone \"%7\" (pos=%8) for a price of %9.", player.GetIdentity().GetName(), player.GetIdentity().GetId(), reserve.RootItem.ClassName, reserve.TotalAmount, reserve.Trader.GetTraderMarket().TraderName, reserve.Trader.GetDisplayName(), reserve.Trader.GetTraderZone().m_DisplayName, reserve.Trader.GetTraderZone().Position.ToString(), reserve.Price));	
		
		//! Need to clear reserved after a purchase
		ClearReserved(player);
		
		zone.Save();

		Callback(itemClassName, ExpansionMarketResult.Success, player.GetIdentity());
		MarketModulePrint("Exec_ConfirmPurchase - End");
	}

	void ClearReserved(PlayerBase player, bool unlockMoney = false)
	{
		if (unlockMoney)
			UnlockMoney(player);

		ExpansionMarketReserve reserve = player.GetMarketReserve();
		ExpansionMarketTraderZone zone = reserve.Trader.GetTraderZone();

		reserve.ClearReserved(zone);
		player.ClearMarketReserve();
	}

	protected void ClearTmpNetworkCaches()
	{
		m_TmpVariantIds.Clear();
		m_TmpVariantIdIdx = 0;
		m_TmpNetworkCats.Clear();
		m_TmpNetworkBaseItems.Clear();
	}

	/*
	 * Called Client Only: The server will clear the reserved stock that the player made
	 */
	// ------------------------------------------------------------
	// Expansion CancelPurchase
	// ------------------------------------------------------------
	void CancelPurchase(string itemClassName, PlayerBase player = NULL)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("CancelPurchase - Sart");
			
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(itemClassName);
			rpc.Send(NULL, ExpansionMarketModuleRPC.CancelPurchase, true, NULL);
			
			MarketModulePrint("CancelPurchase - End");
		}
	}
		
	// ------------------------------------------------------------
	// Expansion Exec_CancelPurchase
	// ------------------------------------------------------------
	private void RPC_CancelPurchase(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_CancelPurchase - Sart");
		
		string itemClassName;
		if (!ctx.Read(itemClassName))
			return;

		PlayerBase player = PlayerBase.GetPlayerByUID(senderRPC.GetId());
		if (!player)
			return;
	
		Exec_CancelPurchase(player, itemClassName);
		
		MarketModulePrint("RPC_CancelPurchase - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_CancelPurchase
	// ------------------------------------------------------------
	private void Exec_CancelPurchase(notnull PlayerBase player, string itemClassName)
	{
		MarketModulePrint("Exec_CancelPurchase - Sart");
		
		RemoveReservedStock(player, itemClassName);
		
		MarketModulePrint("Exec_CancelPurchase - End");
	}
	
	/*
	 * Called Client Only: Request sell
	 */
	// ------------------------------------------------------------
	// Expansion RequestSell
	// ------------------------------------------------------------
	void RequestSell(string itemClassName, int count, ExpansionMarketCurrency currentPrice, ExpansionTraderObjectBase trader, PlayerBase player = NULL)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RequestSell - Sart");
			
			if (!trader)
			{
				return;
			}
				
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(itemClassName);
			rpc.Write(count);
			rpc.Write(currentPrice);
			rpc.Send(trader.GetTraderEntity(), ExpansionMarketModuleRPC.RequestSell, true, NULL);
						
			MarketModulePrint("RequestSell - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestSell
	// ------------------------------------------------------------
	private void RPC_RequestSell(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestSell - Sart");
		
		string itemClassName;
		if (!ctx.Read(itemClassName))
			return;

		int count;
		if (!ctx.Read(count))
			return;
			
		ExpansionMarketCurrency currentPrice;
		if (!ctx.Read(currentPrice))
			return;

		ExpansionTraderObjectBase trader = GetTraderFromObject(target);
		if (!trader)
			return;

		PlayerBase player = PlayerBase.GetPlayerByUID(senderRPC.GetId());
		if (!player) 
			return;

		Exec_RequestSell(player, itemClassName, count, currentPrice, trader);
		
		MarketModulePrint("RPC_RequestSell - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestSell
	// ------------------------------------------------------------
	private void Exec_RequestSell(notnull PlayerBase player, string itemClassName, int count, ExpansionMarketCurrency currentPrice, ExpansionTraderObjectBase trader)
	{
		MarketModulePrint("Exec_RequestSell - Sart");
		
		if (!player)
		{
			return;
		}
		
		ExpansionMarketTraderZone zone = trader.GetTraderZone();
		ExpansionMarketSettings market = GetExpansionSettings().GetMarket();
		
		if (!market)
		{
			MarketModulePrint("Callback_FailedUnknown: 6");
			
			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}
		
		if (!zone)
		{
			MarketModulePrint("Callback_FailedUnknown: 7");

			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}

		// Afterwards calculate the price of the items at that stock		
		ExpansionMarketSell sellList = player.GetMarketSell();
		sellList.Trader = trader;

		sellList.Item = market.GetItem( itemClassName );
		if (!sellList.Item /*|| !sellList.Valid*/)
		{
			MarketModulePrint("Callback_FailedUnknown: 8");

			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}

		//! Check if we can sell this item to this specific trader
		if (!sellList.Trader.GetTraderMarket().CanSellItem(itemClassName))
		{
			Callback(itemClassName, ExpansionMarketResult.FailedCannotSell, player.GetIdentity());
			return;
		}
		
		ExpansionMarketPlayerInventory inventory = new ExpansionMarketPlayerInventory(player);

		//! Result if the price the player has seen and agreed to in menu doesn't match anymore
		//! the current item price of the trader because stock has changed enough to affect it
		//! (another player was quicker to get his transaction through)
		ExpansionMarketResult result = ExpansionMarketResult.FailedStockChange;

		//! Compare that price to the one the player sent
		if (!FindSellPrice(player, inventory.m_Inventory, zone.GetStock(itemClassName), count, sellList, result) || sellList.Price != currentPrice)
		{
			EXPrint("Exec_RequestSell - Player sent price: " + currentPrice);
			EXPrint("Exec_RequestSell - Current stock: " + zone.GetStock(itemClassName, true));
			sellList.Debug();

			player.ClearMarketSell();
			
			MarketModulePrint("Callback " + typename.EnumToString(ExpansionMarketResult, result));

			Callback(itemClassName, result, player.GetIdentity());
			
			return;
		}

		//sellList.Debug();
		
		sellList.Valid = true;
		sellList.Time = GetGame().GetTime();

		Callback(itemClassName, ExpansionMarketResult.Success, player.GetIdentity());

		//zone.Save();
		
		ExpansionLogMarket(string.Format("Player \"%1\" (id=%2) has send a requested to sell %3 x%4 at the trader \"%5 (%6)\" in market zone \"%7\" (pos=%8).", player.GetIdentity().GetName(), player.GetIdentity().GetId(), itemClassName, count, trader.GetTraderMarket().TraderName, trader.GetDisplayName(), trader.GetTraderZone().m_DisplayName, trader.GetTraderZone().Position.ToString()));

		MarketModulePrint("Exec_RequestSell - End");
	}
	
	/*
	 * Called Client Only: The server will finalize the transaction with the 
	 * details that were stored in RequestSell.
	 */
	// ------------------------------------------------------------
	// Expansion ConfirmSell
	// ------------------------------------------------------------
	void ConfirmSell(string itemClassName, PlayerBase player = NULL)
	{		
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("ConfirmSell - Sart");
			
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(itemClassName);
			rpc.Send(NULL, ExpansionMarketModuleRPC.ConfirmSell, true, NULL);
					
			MarketModulePrint("ConfirmSell - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmSell
	// ------------------------------------------------------------
	private void RPC_ConfirmSell(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_ConfirmSell - Sart");
		
		string itemClassName;
		if (!ctx.Read(itemClassName))
			return;

		PlayerBase player = PlayerBase.GetPlayerByUID(senderRPC.GetId());
		if (!player)
			return;

		Exec_ConfirmSell(player, itemClassName);
		
		MarketModulePrint("RPC_ConfirmSell - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmSell
	// ------------------------------------------------------------
	private void Exec_ConfirmSell(notnull PlayerBase player, string itemClassName)
	{
		MarketModulePrint("Exec_ConfirmSell - Start");
		
		if (!player)
		{
			return;
		}
		
		ExpansionMarketSell sell = player.GetMarketSell();
		if (!sell || !sell.Valid || !sell.Trader || !sell.Trader.GetTraderEntity())
		{
			player.ClearMarketSell();

			EXPrint("Exec_ConfirmSell - Callback_FailedReserveTime");
			
			Callback(itemClassName, ExpansionMarketResult.FailedReserveTime, player.GetIdentity());
			return;
		}

		ExpansionMarketTraderZone zone = sell.Trader.GetTraderZone();
		if (!zone)
		{
			player.ClearMarketSell();

			MarketModulePrint("Exec_ConfirmSell - Callback_FailedUnknown");
			
			Callback(itemClassName, ExpansionMarketResult.FailedUnknown, player.GetIdentity());
			return;
		}
		
		if (vector.Distance(player.GetPosition(), sell.Trader.GetTraderEntity().GetPosition()) > MAX_TRADER_INTERACTION_DISTANCE)
		{
			MarketModulePrint("Exec_ConfirmSell - Callback_FailedTooFarAway: Player is too far from trader!");

			player.ClearMarketSell();

			Callback(itemClassName, ExpansionMarketResult.FailedTooFarAway, player.GetIdentity());
			return;
		}

		//sell.Debug();
		
		for (int j = 0; j < sell.Sell.Count(); j++)
		{
			zone.AddStock(sell.Sell[j].ClassName, sell.Sell[j].AddStockAmount);
			if (sell.Sell[j].ItemRep && !sell.Sell[j].ItemRep.IsPendingDeletion())
				sell.Sell[j].DestroyItem();
		}

		EntityAI parent = player;
		
		SpawnMoney(player, parent, sell.Price, true, sell.Item, sell.Trader.GetTraderMarket());
		
		zone.Save();

		ExpansionLogMarket(string.Format("Player \"%1\" (id=%2) has sold %3 x%4 at the trader \"%5 (%6)\" in market zone \"%7\" (pos=%8) and got %9.", player.GetIdentity().GetName(), player.GetIdentity().GetId(), itemClassName, sell.TotalAmount, sell.Trader.GetTraderMarket().TraderName, sell.Trader.GetDisplayName(), sell.Trader.GetTraderZone().m_DisplayName, sell.Trader.GetTraderZone().Position.ToString(), sell.Price));	
		
		player.ClearMarketSell();
		
		Callback(itemClassName, ExpansionMarketResult.Success, player.GetIdentity());
		
		ExpansionNotification("STR_EXPANSION_TRADER_SELL_SUCCESS", new StringLocaliser("STR_EXPANSION_TRADER_SELL_SUCCESS_TEXT", ExpansionStatic.GetItemDisplayNameWithType(itemClassName), sell.TotalAmount.ToString() + "x", sell.Price.ToString()), EXPANSION_NOTIFICATION_ICON_INFO, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 3, ExpansionNotificationType.MARKET).Create(player.GetIdentity());	
		
		CheckSpawn(player, parent);
		
		MarketModulePrint("Exec_ConfirmSell - End");
	}

	// ------------------------------------------------------------
	// Expansion CancelSell
	// ------------------------------------------------------------
	void CancelSell(string itemClassName, PlayerBase player = NULL)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("CancelSell - Sart");
			
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(itemClassName);
			rpc.Send(NULL, ExpansionMarketModuleRPC.CancelSell, true, NULL);
			
			MarketModulePrint("CancelSell - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_CancelSell
	// ------------------------------------------------------------
	private void RPC_CancelSell(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_CancelSell - Sart");
		
		string itemClassName;
		if (!ctx.Read(itemClassName))
			return;

		PlayerBase player = PlayerBase.GetPlayerByUID(senderRPC.GetId());
		if (!player)
			return;

		Exec_CancelSell(player, itemClassName);
		
		MarketModulePrint("RPC_CancelSell - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_CancelSell
	// ------------------------------------------------------------
	private void Exec_CancelSell(notnull PlayerBase player, string itemClassName)
	{
		MarketModulePrint("Exec_CancelSell - Sart");
		
		player.ClearMarketSell();
		
		MarketModulePrint("Exec_CancelSell - End");
	}

	// ------------------------------------------------------------
	// Expansion RequestTraderData
	// ------------------------------------------------------------
	void RequestTraderData(ExpansionTraderObjectBase trader)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RequestTraderData - Sart");
			
			if (!trader)
			{
				Error("ExpansionMarkerModule::RequestTraderData - Trader is NULL!");
				return;
			}
			
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(trader.GetTraderEntity(), ExpansionMarketModuleRPC.RequestTraderData, true, NULL);
				
			MarketModulePrint("RequestTraderData - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestTraderData - server
	// ------------------------------------------------------------
	private void RPC_RequestTraderData(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestTraderData - Sart");
		
		ExpansionTraderObjectBase trader = GetTraderFromObject(target);
		if (!trader)
		{
			Error("ExpansionMarketModule::RPC_RequestTraderData - Trader object is NULL!");
			return;
		}

		if (!senderRPC)
		{
			Error("ExpansionMarketModule::RPC_RequestTraderData - Player identity is NULL!");
			return;
		}

		string uid = senderRPC.GetId();

		float startTime = GetGame().GetTickTime();

		LoadTraderData(trader, uid);

		EXPrintHitch(ToString() + "::RPC_RequestTraderData - LoadTraderData", startTime);

		MarketModulePrint("RPC_RequestTraderData - End");
	}

	// ------------------------------------------------------------
	// Expansion LoadTraderData - server
	// ------------------------------------------------------------
	//! Send trader data to client
	void LoadTraderData(ExpansionTraderObjectBase trader, string uid)
	{
		MarketModulePrint("LoadTraderData - Sart");

		PlayerBase player = PlayerBase.GetPlayerByUID(uid);
		if (!player)
		{
			Error("ExpansionMarketModule::SendTraderData - Player is NULL!");
			return;
		}

		PlayerIdentity ident = player.GetIdentity();
		if (!ident)
		{
			Error("ExpansionMarketModule::SendTraderData - Player identity is NULL!");
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(trader.GetTraderZone().BuyPricePercent);
		rpc.Write(trader.GetTraderZone().SellPricePercent);

		rpc.Send(trader.GetTraderEntity(), ExpansionMarketModuleRPC.LoadTraderData, true, ident);

		MarketModulePrint("LoadTraderData - End");
	}
	
	//! Send trader items to client in batches
	protected void LoadTraderItems(ExpansionTraderObjectBase trader, PlayerIdentity ident, int start = 0, bool stockOnly = false, TIntArray itemIDs = NULL)
	{
		EXPrint("LoadTraderItems - Start - start: " + start + " stockOnly: " + stockOnly);

		if (!trader)
		{
			Error("ExpansionMarketModule::LoadTraderItems - Trader object is NULL!");
			return;
		}

		if (!ident)
		{
			Error("ExpansionMarketModule::LoadTraderItems - Player identity is NULL!");
			return;
		}

		array<ref ExpansionMarketNetworkItem> networkItemsTmp = new array<ref ExpansionMarketNetworkItem>;

		float startTime = GetGame().GetTickTime();
		
		TIntArray itemIDsTmp;
		if (itemIDs && itemIDs.Count())
		{
			//! Make sure we do not have duplicate IDs so counts are correct
			itemIDsTmp = new TIntArray;
			foreach (int itemID: itemIDs)
			{
				if (itemIDsTmp.Find(itemID) == -1)
					itemIDsTmp.Insert(itemID);
			}
			EXPrint(ToString() + "::LoadTraderItems - IDs: " + itemIDsTmp);
		}

		int next = trader.GetNetworkSerialization(networkItemsTmp, start, stockOnly, itemIDsTmp);

		EXPrintHitch(ToString() + "::LoadTraderItems - GetNetworkSerialization ", startTime);

		if (next < 0)
		{
			Error("ExpansionMarketModule::LoadTraderItems - GetNetworkSerialization failed!");
			return;
		}

		array<ref ExpansionMarketNetworkBaseItem> networkBaseItems = new array<ref ExpansionMarketNetworkBaseItem>;
		array<ref ExpansionMarketNetworkItem> networkItems = new array<ref ExpansionMarketNetworkItem>;

		foreach (ExpansionMarketNetworkItem item : networkItemsTmp)
		{
			if (stockOnly || item.m_StockOnly)
				networkBaseItems.Insert(new ExpansionMarketNetworkBaseItem(item.ItemID, item.Stock));
			else
				networkItems.Insert(item);
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(start);
		rpc.Write(next);
		if (itemIDsTmp && itemIDsTmp.Count())
			rpc.Write(itemIDsTmp.Count());
		else
			rpc.Write(trader.GetTraderMarket().m_Items.Count());
		rpc.Write(stockOnly);
		rpc.Write(networkBaseItems);
		rpc.Write(networkItems);
		rpc.Send(trader.GetTraderEntity(), ExpansionMarketModuleRPC.LoadTraderItems, true, ident);

		EXPrint("LoadTraderItems - End - start: " + start + " end: " + next);
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_LoadTraderData - client
	// ------------------------------------------------------------
	private void RPC_LoadTraderData(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_LoadTraderData - Start");
		
		ExpansionTraderObjectBase trader = GetTraderFromObject(target);
		if (!trader)
		{
			Error("ExpansionMarketModule::RPC_LoadTraderData - Could not get ExpansionTraderObjectBase!");
			return;
		}
		
		if (!ctx.Read(m_ClientMarketZone.BuyPricePercent))
		{
			Error("ExpansionMarketModule::RPC_LoadTraderData - Could not read buy price percent!");
			return;
		}
		
		if (!ctx.Read(m_ClientMarketZone.SellPricePercent))
		{
			Error("ExpansionMarketModule::RPC_LoadTraderData - Could not read sell price percent!");
			return;
		}

		bool stockOnly = trader.GetTraderMarket().m_StockOnly;  //! If already netsynched, request stock only
		RequestTraderItems(trader, 0, stockOnly);

		MarketModulePrint("RPC_LoadTraderData - End");
	}

	// ------------------------------------------------------------
	// Expansion RequestTraderItems - client
	// ------------------------------------------------------------
	void RequestTraderItems(ExpansionTraderObjectBase trader, int start = 0, bool stockOnly = false, TIntArray itemIDs = NULL)
	{
		MarketModulePrint("RequestTraderItems - Start - " + start);
		
		if (!trader)
		{
			Error("ExpansionMarkerModule::RequestTraderItems - Trader is NULL!");
			return;
		}
		
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(start);
		rpc.Write(stockOnly);
		rpc.Write(itemIDs);
		rpc.Send(trader.GetTraderEntity(), ExpansionMarketModuleRPC.RequestTraderItems, true, NULL);
			
		MarketModulePrint("RequestTraderItems - End - " + start);
	}

	// ------------------------------------------------------------
	// Expansion RPC_RequestTraderItems - server
	// ------------------------------------------------------------
	private void RPC_RequestTraderItems(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestTraderItems - Start");

		ExpansionTraderObjectBase trader = GetTraderFromObject(target);
		if (!trader)
		{
			Error("ExpansionMarketModule::RPC_RequestTraderItems - Could not get ExpansionTraderObjectBase!");
			return;
		}

		int start;
		if (!ctx.Read(start))
		{
			Error("ExpansionMarketModule::RPC_RequestTraderItems - Could not read remaining items start!");
			return;
		}

		bool stockOnly;
		if (!ctx.Read(stockOnly))
		{
			Error("ExpansionMarketModule::RPC_RequestTraderItems - Could not read stockOnly!");
			return;
		}

		TIntArray itemIDs;
		if (!ctx.Read(itemIDs))
		{
			Error("ExpansionMarketModule::RPC_RequestTraderItems - Could not read itemIDs!");
			return;
		}

		LoadTraderItems(trader, senderRPC, start, stockOnly, itemIDs);

		MarketModulePrint("RPC_RequestTraderItems - End");
	}

	// ------------------------------------------------------------
	// Expansion RPC_LoadTraderItems - client
	// ------------------------------------------------------------
	private void RPC_LoadTraderItems(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_LoadTraderItems - Start");

		ExpansionTraderObjectBase trader = GetTraderFromObject(target);
		if (!trader)
		{
			Error("ExpansionMarketModule::RPC_LoadTraderItems - Could not get ExpansionTraderObjectBase!");
			return;
		}

		int start;
		if (!ctx.Read(start))
		{
			Error("ExpansionMarketModule::RPC_LoadTraderItems - Could not read items start index!");
			return;
		}

		int next;
		if (!ctx.Read(next))
		{
			Error("ExpansionMarketModule::RPC_LoadTraderItems - Could not read items next index!");
			return;
		}

		int count;
		if (!ctx.Read(count))
		{
			Error("ExpansionMarketModule::RPC_LoadTraderItems - Could not read items count!");
			return;
		}

		bool stockOnly;
		if (!ctx.Read(stockOnly))
		{
			Error("ExpansionMarketModule::RPC_LoadTraderItems - Could not read stockOnly!");
			return;
		}

		EXPrint("RPC_LoadTraderItems - received batch total: " + next + " remaining: " + (count - next));

		float startTime = GetGame().GetTickTime();
	
		array<ref ExpansionMarketNetworkBaseItem> networkBaseItems = new array<ref ExpansionMarketNetworkBaseItem>;
		if (!ctx.Read(networkBaseItems))
		{
			Error("ExpansionMarketModule::RPC_LoadTraderItems - Could not read networkBaseItems array!");
			return;
		}

		array<ref ExpansionMarketNetworkItem> networkItems = new array<ref ExpansionMarketNetworkItem>;
		if (!ctx.Read(networkItems))
		{
			Error("ExpansionMarketModule::RPC_LoadTraderItems - Could not read networkItems array!");
			return;
		}
	
		if (networkBaseItems.Count() + networkItems.Count() <= 0)
		{
			Error("ExpansionMarketModule::RPC_LoadTraderItems - networkBaseItems + networkItems count is 0!");
			return;
		}

		int i;
		ExpansionMarketItem item;

		if (start == 0)
		{
			//! 1st batch, make sure cache is clear
			ClearTmpNetworkCaches();
		}

		if (networkItems.Count())
		{
			//! Add full items + set stock
			EXPrint(ToString() + "::RPC_LoadTraderItems - Adding and setting stock for " + networkItems.Count() + " items");

			for (i = 0; i < networkItems.Count(); i++)
			{
				//EXPrint("RPC_LoadTraderItems - " + networkItems[i].ClassName + " (ID " + networkItems[i].ItemID + ") - stock: " + networkItems[i].Stock);
				item = GetExpansionSettings().GetMarket().UpdateMarketItem_Client(networkItems[i]);
				m_ClientMarketZone.SetStock(networkItems[i].ClassName, networkItems[i].Stock);
				int buySell = networkItems[i].Packed >> 24;
				trader.GetTraderMarket().AddItemInternal(item, buySell);
				if (!m_TmpNetworkCats.Contains(networkItems[i].CategoryID))
					m_TmpNetworkCats.Insert(networkItems[i].CategoryID, GetExpansionSettings().GetMarket().GetCategory(networkItems[i].CategoryID));
			}
		}

		if (m_TmpNetworkCats.Count())
		{
			foreach (ExpansionMarketNetworkBaseItem networkBaseItem: networkBaseItems)
			{
				m_TmpVariantIds.Insert(networkBaseItem.ItemID);
			}
		}

		//! Set stock only
		for (i = 0; i < networkBaseItems.Count(); i++)
		{
			m_TmpNetworkBaseItems.Insert(networkBaseItems[i]);
		}
		
		EXPrintHitch(ToString() + "::RPC_LoadTraderItems - update market items ", startTime);

		if (count - next == 0)
		{
			//! Last batch

			if (m_TmpVariantIds.Count())
			{
				foreach (ExpansionMarketTraderItem tItem: trader.GetTraderMarket().m_Items)
				{
					if (tItem.MarketItem.Variants.Count())
					{
						//EXPrint("RPC_LoadTraderItems - adding variants for " + tItem.MarketItem.ClassName + " (ID " + tItem.MarketItem.ItemID + ")");
						ExpansionMarketCategory itemCat = GetExpansionSettings().GetMarket().GetCategory(tItem.MarketItem.CategoryID);
						itemCat.AddVariants(tItem.MarketItem, m_TmpVariantIds, m_TmpVariantIdIdx);
					}
				}
			}

			if (m_TmpNetworkCats.Count())
			{
				foreach (ExpansionMarketCategory cat : m_TmpNetworkCats)
				{
					cat.SetAttachmentsFromIDs();
					cat.Finalize(false);
				}

				trader.GetTraderMarket().Finalize();
			}

			EXPrint(ToString() + "::RPC_LoadTraderItems - Setting stock for " + m_TmpNetworkBaseItems.Count() + " items");
			foreach (ExpansionMarketNetworkBaseItem tmpNetworkBaseItem: m_TmpNetworkBaseItems)
			{
				item = GetExpansionSettings().GetMarket().GetItem(tmpNetworkBaseItem.ItemID, false);
				if (!item)
				{
					EXPrint(ToString() + "::RPC_LoadTraderItems - WARNING - item ID " + tmpNetworkBaseItem.ItemID + " does not exist!");
					continue;
				}
				item.m_UpdateView = true;
				//EXPrint("RPC_LoadTraderItems - " + item.ClassName + " (ID " + item.ItemID + ") - stock: " + tmpNetworkBaseItem.Stock);
				m_ClientMarketZone.SetStock(item.ClassName, tmpNetworkBaseItem.Stock);
			}

			ClearTmpNetworkCaches();

			SetTrader(trader, true);
		}
		else
		{
			//! Client can draw received items so far
			SetTrader(trader, false);

			//! Request next batch
			RequestTraderItems(trader, next, stockOnly);
		}
		
		MarketModulePrint("RPC_LoadTraderItems - End");
	}

	bool IsMoney(string type)
	{
		return m_MoneyTypes.Contains(type);
	}

	bool IsMoney(EntityAI item)
	{
		string type = item.GetType();
		type.ToLower();
		return IsMoney(type);
	}

	// -----------------------------------------------------------
	// Expansion array< EntityAI > LocalGetEntityInventory
	// -----------------------------------------------------------
	array<EntityAI> LocalGetEntityInventory()
	{
		return m_LocalEntityInventory.m_Inventory;
	}

	// -----------------------------------------------------------
	// Expansion EnumeratePlayerInventory
	// -----------------------------------------------------------
	void EnumeratePlayerInventory(PlayerBase player)
	{
		m_LocalEntityInventory = new ExpansionMarketPlayerInventory(player);
	}
	
	// ------------------------------------------------------------
	// Expansion Int GetAmountInInventory
	// Gets the amount of market items the player has in his inventroy
	// ------------------------------------------------------------
	//! Returns positive number if at least one sellable item found, negative number if only unsellable items found
	int GetAmountInInventory(ExpansionMarketItem item, array< EntityAI > entities)
	{
		MarketModulePrint("GetAmountInInventory - Start");
		
		string itemName = item.ClassName;
		itemName.ToLower();
		
		int sellable;
		int unsellable;

		foreach (EntityAI entity: entities)
		{
			if (entity == NULL)
				continue;

			string entName = entity.GetType();
			entName.ToLower();
			
			if (entName != itemName)
				continue;

			int amount = GetItemAmount(entity);

			if (amount > 0)
				sellable += amount;
			else
				unsellable += amount;
		}

		if (sellable > 0)
		{
			MarketModulePrint("GetAmountInInventory - End and return sellable: " + sellable + " for item " + itemName);
			return sellable;
		}
		else
		{
			MarketModulePrint("GetAmountInInventory - End and return unsellable: " + unsellable + " for item " + itemName);
			return unsellable;
		}
	}
	
	// ------------------------------------------------------------
	// Expansion Array GetEnitysOfItemInventory
	// ------------------------------------------------------------
	array<EntityAI> GetEnitysOfItemInventory(ExpansionMarketItem item, array< EntityAI > entitys)
	{
		MarketModulePrint("GetEnitysOfItemInventory - Start");
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		int totalAmount = 0;

		for (int i = 0; i < entitys.Count(); i++)
		{
			EntityAI entity = entitys.Get(i);
			if (entity == NULL)
				continue;

			string entName = entity.GetType();
			entName.ToLower();
			
			string itemName = item.ClassName;
			itemName.ToLower();
			
			if (entName != itemName)
				continue;
			
			itemsArray.Insert(entity);
		}

		return itemsArray;
	}

	// ------------------------------------------------------------
	// Expansion Bool CanSellItem
	// ------------------------------------------------------------
	bool CanSellItem(EntityAI item, bool checkIfRuined = false)
	{
		if (checkIfRuined && item.IsRuined())
			return false;

		if (item.GetInventory())
		{
			//! Check if the item has a container and any items in it
			if (item.HasAnyCargo())
				return false;

			//! Check if any of the item's attachments has any cargo
			for (int i = 0; i < item.GetInventory().AttachmentCount(); i++)
			{
				EntityAI attachment = item.GetInventory().GetAttachmentFromIndex(i);
				if (attachment && attachment.HasAnyCargo())
					return false;
			}

			//! Check if item is attachment that can be released
			if (item.GetInventory().IsAttachment())
				return item.GetHierarchyParent().CanReleaseAttachment(item) && item.GetHierarchyParent().GetInventory().CanRemoveAttachment(item);
		}

		#ifdef EXPANSIONMODVEHICLE
		//! If this is a master key of a vehicle, don't allow to sell it
		ExpansionCarKey key;
		if (Class.CastTo(key, item) && key.IsMaster())
			return false;
		#endif

		return true;
	}

	// ------------------------------------------------------------
	// Expansion Bool CanOpenMenu
	// ------------------------------------------------------------
	bool CanOpenMenu()
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			if (GetGame().GetUIManager().GetMenu())
				return false;
			
			if (GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu())
				return false;
		}

		return true;
	}
	
	// ------------------------------------------------------------
	// Expansion OpenTraderMenu
	// ------------------------------------------------------------
	void OpenTraderMenu()
	{
		GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu(EXPANSION_MENU_MARKET);
	}
	
	// ------------------------------------------------------------
	// Expansion OpenTraderMenu
	// ------------------------------------------------------------
	void OpenATMMenu()
	{
		GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu(EXPANSION_MENU_ATM);
	}
	
	// ------------------------------------------------------------
	// Expansion CloseMenu
	// ------------------------------------------------------------
	void CloseMenu()
	{
		GetDayZGame().GetExpansionGame().GetExpansionUIManager().CloseMenu();
	}

	// ------------------------------------------------------------
	// Expansion MarketModulePrint
	// ------------------------------------------------------------
	void MarketModulePrint(string text)
	{
	#ifdef EXPANSIONMODMARKET_DEBUG
		EXPrint("ExpansionMarketModule::" + text);
	#endif
	}

	void CheckSpawn(PlayerBase player, EntityAI parent, bool attachmentNotAttached = false)
	{
		if (parent != player || attachmentNotAttached)
		{
			ExpansionNotification("STR_EXPANSION_MARKET_TITLE", "STR_EXPANSION_TEMPORARY_STORAGE_INFO", EXPANSION_NOTIFICATION_ICON_TRADER, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 6, ExpansionNotificationType.MARKET).Create(player.GetIdentity());
		}
	}
	
	// ------------------------------------------------------------
	// Expansion GetMoneyDenominations
	// ------------------------------------------------------------
	array<string> GetMoneyDenominations()
	{
		return m_MoneyDenominations;
	}

	string GetMoneyDenomination(int i)
	{
		return m_MoneyDenominations[i];
	}
	
	ExpansionMarketCategory GetItemCategory(ExpansionMarketItem item)
	{
		return GetExpansionSettings().GetMarket().GetCategory(item.CategoryID);
	}
	
	// ------------------------------------------------------------
	// Expansion GetATMData
	// ------------------------------------------------------------	
	array<ref ExpansionMarketATM_Data> GetATMData()
	{
		return m_ATMData;
	}
	
	// ------------------------------------------------------------
	// Expansion GetPlayerATMData
	// ------------------------------------------------------------		
	ExpansionMarketATM_Data GetPlayerATMData(string id)
	{
		array<ref ExpansionMarketATM_Data> data = GetATMData();
		foreach (ExpansionMarketATM_Data currentData : data)
		{
			if (currentData && currentData.PlayerID == id)
				return currentData;
		}
		
		return NULL;
	}
	
	// ------------------------------------------------------------
	// Expansion LoadATMData
	// ------------------------------------------------------------
	void LoadATMData()
	{
		array<string> files = ExpansionStatic.FindFilesInLocation(EXPANSION_ATM_FOLDER, ".json");
		
		foreach (string fileName : files)
		{
			//! Strip '.json' extension
			fileName = fileName.Substring(0, fileName.Length() - 5);

			ExpansionMarketATM_Data data = ExpansionMarketATM_Data.Load(fileName);
			m_ATMData.Insert(data);
		}
	}
	
	// ------------------------------------------------------------
	// Expansion CreateATMData
	// ------------------------------------------------------------
	void CreateATMData(PlayerIdentity ident)
	{
		ExpansionMarketATM_Data newData = new ExpansionMarketATM_Data;
		newData.m_FileName = ident.GetId();
		newData.PlayerID = ident.GetId();
			
		newData.MoneyDeposited = GetExpansionSettings().GetMarket().DefaultDepositMoney;
		
		newData.Save();
		
		m_ATMData.Insert(newData);
	}
	
	// ------------------------------------------------------------
	// Expansion RequestPlayerATMData
	// ------------------------------------------------------------
	void RequestPlayerATMData()
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RequestPlayerATMData - Start");
						
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(NULL, ExpansionMarketModuleRPC.RequestPlayerATMData, true, NULL);
				
			MarketModulePrint("RequestPlayerATMData - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestPlayerATMData
	// ------------------------------------------------------------
	private void RPC_RequestPlayerATMData(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestPlayerATMData - Start");
		
		if (!senderRPC)
		{
			Error("ExpansionMarketModule::RPC_RequestPlayerATMData - Could not get sender indentity!");
			return;
		}
		
		Exec_RequestPlayerATMData(senderRPC);
		
		MarketModulePrint("RPC_RequestPlayerATMData - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestTraderData
	// ------------------------------------------------------------
	private void Exec_RequestPlayerATMData(PlayerIdentity ident)
	{
		MarketModulePrint("Exec_RequestPlayerATMData - Start");
		
		if (!ident)
		{
			Error("ExpansionMarketModule::Exec_RequestPlayerATMData - Could not get sender indentity!");
			return;
		}
		
		SendPlayerATMData(ident);
		
		MarketModulePrint("Exec_RequestPlayerATMData - End");
	}
	
	// ------------------------------------------------------------
	// Expansion SendPlayerATMData
	// ------------------------------------------------------------
	void SendPlayerATMData(PlayerIdentity ident)
	{
		if (!ident)
		{
			Error("ExpansionMarketModule::SendPlayerATMData - Could not get sender indentity!");
			return;
		}
		
		ExpansionMarketATM_Data data = GetPlayerATMData(ident.GetId());
		if (!data)
		{
			Error("ExpansionMarketModule::SendPlayerATMData - Could not get ExpansionMarketATM_Data!");
			return;
		}
			
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(data);
		rpc.Send(NULL, ExpansionMarketModuleRPC.SendPlayerATMData, true, ident);
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_SendPlayerATMData
	// ------------------------------------------------------------
	private void RPC_SendPlayerATMData(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_SendPlayerATMData - Start");

		ExpansionMarketATM_Data data;
		if (!ctx.Read(data))
		{
			Error("ExpansionMarketModule::RPC_SendPlayerATMData - Could not get ExpansionMarketATM_Data!");
			return;
		}
		
		Exec_SendPlayerATMData(data);
		
		MarketModulePrint("RPC_SendPlayerATMData - End");
	}

	// ------------------------------------------------------------
	// Expansion Exec_SendPlayerATMData
	// ------------------------------------------------------------
	private void Exec_SendPlayerATMData(ExpansionMarketATM_Data data)
	{		
		MarketModulePrint("Exec_SendPlayerATMData - Start");
				
		SetPlayerATMData(data);
		
		MarketModulePrint("Exec_SendPlayerATMData - End");
	}
	
	// ------------------------------------------------------------
	// Expansion SetPlayerATMData
	// ------------------------------------------------------------
	void SetPlayerATMData(ExpansionMarketATM_Data data)
	{
		MarketModulePrint("SetPlayerATMData - Start");
	
		SI_ATMMenuInvoker.Invoke(data);
		
		MarketModulePrint("SetPlayerATMData - End");
	}
	
	// ------------------------------------------------------------
	// Expansion RequestDepositMoney
	// ------------------------------------------------------------	
	void RequestDepositMoney(int amount)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RequestDepositMoney - Start");
						
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(amount);
			rpc.Send(NULL, ExpansionMarketModuleRPC.RequestDepositMoney, true, NULL);
				
			MarketModulePrint("RequestDepositMoney - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestDepositMoney
	// ------------------------------------------------------------
	private void RPC_RequestDepositMoney(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestDepositMoney - Start");
		
		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_RequestDepositMoney - Could not get amount!");
			return;
		}
		
		Exec_RequestDepositMoney(amount, senderRPC);
		
		MarketModulePrint("RPC_RequestDepositMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestDepositMoney
	// ------------------------------------------------------------
	private void Exec_RequestDepositMoney(int amount, PlayerIdentity ident)
	{
		MarketModulePrint("Exec_RequestDepositMoney - Start");
		
		if (!ident)
		{
			Error("ExpansionMarketModule::Exec_RequestDepositMoney - Could not get player identity!");
			return;
		}

		PlayerBase player = PlayerBase.GetPlayerByUID(ident.GetId());
		if (!player)
		{
			Error("ExpansionMarketModule::Exec_RequestDepositMoney - Could not get player base enity!");
			return;
		}
				
		ExpansionMarketATM_Data data = GetPlayerATMData(ident.GetId());
		if (!data)
		{
			Error("ExpansionMarketModule::Exec_RequestDepositMoney - Could not find player atm data!");
			
			return;
		}
		
		array<int> monies = new array<int>;		
		if (!FindMoneyAndCountTypes(player, amount, monies, true))
		{
			Error("ExpansionMarketModule::Exec_RequestDepositMoney - Could not find player money!");			
			UnlockMoney(player);
			
			return;
		}

		EntityAI parent = player;
		
		int removed = RemoveMoney(player);
		if (removed - amount > 0)
		{
		    SpawnMoney(player, parent, removed - amount);

			CheckSpawn(player, parent);
		}
		
		data.AddMoney(amount);
		data.Save();
		
		ExpansionLogATM(string.Format("Player \"%1\" (id=%2) has deposited %3 on his ATM account.", ident.GetName(), ident.GetId(), amount));
		
		ConfirmDepositMoney(amount, ident, data);
		
		MarketModulePrint("Exec_RequestDepositMoney - End");
	}
		
	// ------------------------------------------------------------
	// Expansion ConfirmDepositMoney
	// ------------------------------------------------------------	
	void ConfirmDepositMoney(int amount, PlayerIdentity ident, ExpansionMarketATM_Data data)
	{
		MarketModulePrint("RequestDepositMoney - Start");
					
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(amount);
		rpc.Write(data);
		rpc.Send(NULL, ExpansionMarketModuleRPC.ConfirmDepositMoney, true, ident);
			
		MarketModulePrint("RequestDepositMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmDepositMoney
	// ------------------------------------------------------------
	private void RPC_ConfirmDepositMoney(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_ConfirmDepositMoney - Start");
		
		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_ConfirmDepositMoney - Could not get amount!");
			return;
		}
		
		ExpansionMarketATM_Data data;
		if (!ctx.Read(data))
		{
			Error("ExpansionMarketModule::RPC_ConfirmDepositMoney - Could not get player ATM data!");
			return;
		}
		
		Exec_ConfirmDepositMoney(amount, data);
		
		MarketModulePrint("RPC_ConfirmDepositMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmDepositMoney
	// ------------------------------------------------------------
	private void Exec_ConfirmDepositMoney(int amount, ExpansionMarketATM_Data data)
	{
		MarketModulePrint("Exec_ConfirmDepositMoney - Start");
		
		SI_ATMMenuCallback.Invoke(amount, data, 1);
		
		MarketModulePrint("Exec_ConfirmDepositMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion RequestWithdraw
	// ------------------------------------------------------------	
	void RequestWithdrawMoney(int amount)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RequestWithdraw - Start");
						
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(amount);
			rpc.Send(NULL, ExpansionMarketModuleRPC.RequestWithdrawMoney, true, NULL);
				
			MarketModulePrint("RequestWithdraw - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestWithdrawMoney
	// ------------------------------------------------------------
	private void RPC_RequestWithdrawMoney(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestWithdrawMoney - Start");
		
		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_RequestWithdrawMoney - Could not get amount!");
			return;
		}
				
		if (!senderRPC)
		{
			Error("ExpansionMarketModule::RPC_RequestWithdrawMoney - Could not get sender indentity!");
			return;
		}
		
		Exec_RequestWithdrawMoney(amount, senderRPC);
		
		MarketModulePrint("RPC_RequestWithdrawMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestWithdrawMoney
	// ------------------------------------------------------------
	private void Exec_RequestWithdrawMoney(int amount, PlayerIdentity ident)
	{
		MarketModulePrint("Exec_RequestWithdrawMoney - Start");
					
		if (!ident)
		{
			Error("ExpansionMarketModule::Exec_RequestWithdrawMoney - Could not get sender indentity!");
			return;
		}	
					
		ExpansionMarketATM_Data data = GetPlayerATMData(ident.GetId());
		if (!data)
		{
			Error("ExpansionMarketModule::Exec_RequestWithdrawMoney - Could not get player ATM data!");			
			return;
		}
		
		if (data.MoneyDeposited < amount)
		{
			Error("ExpansionMarketModule::Exec_RequestWithdrawMoney - Tryed to deposit more money then in inventory!");
			return;
		}
		
		PlayerBase player = PlayerBase.GetPlayerByUID(ident.GetId());
		if (!player)
		{
			Error("ExpansionMarketModule::Exec_RequestWithdrawMoney - Could not get player base entity!");
			return;
		}	
		
		EntityAI parent = player;
		if (!parent)
		{
			Error("ExpansionMarketModule::Exec_RequestWithdrawMoney - Could not get player entity!");
			return;
		}	
		
		SpawnMoney(player, parent, amount);

		CheckSpawn(player, parent);
		
		data.RemoveMoney(amount);
		data.Save();
		
		ExpansionLogATM(string.Format("Player \"%1\" (id=%2) has withdrawn %3 from his ATM account.", ident.GetName(), ident.GetId(), amount));
		
		ConfirmWithdrawMoney(amount, ident, data);
		
		MarketModulePrint("Exec_RequestWithdrawMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion ConfirmWithdrawMoney
	// ------------------------------------------------------------	
	void ConfirmWithdrawMoney(int amount, PlayerIdentity ident, ExpansionMarketATM_Data data)
	{
		MarketModulePrint("RequestDepositMoney - Start");
					
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(amount);
		rpc.Write(data);
		rpc.Send(NULL, ExpansionMarketModuleRPC.ConfirmWithdrawMoney, true, ident);
			
		MarketModulePrint("RequestDepositMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmWithdrawMoney
	// ------------------------------------------------------------
	private void RPC_ConfirmWithdrawMoney(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_ConfirmWithdrawMoney - Start");
		
		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_ConfirmWithdrawMoney - Could not get amount!");
			return;
		}
		
		ExpansionMarketATM_Data data;
		if (!ctx.Read(data))
		{
			Error("ExpansionMarketModule::RPC_ConfirmWithdrawMoney - Could not get player ATM data!");
			return;
		}
		
		Exec_ConfirmWithdrawMoney(amount, data);
		
		MarketModulePrint("Exec_ConfirmWithdrawMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmWithdrawMoney
	// ------------------------------------------------------------
	private void Exec_ConfirmWithdrawMoney(int amount, ExpansionMarketATM_Data data)
	{
		MarketModulePrint("Exec_ConfirmWithdrawMoney - Start");
		
		SI_ATMMenuCallback.Invoke(amount, data, 2);
		
		MarketModulePrint("Exec_ConfirmWithdrawMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion RequestTransferMoneyToPlayer
	// Called from the client
	// ------------------------------------------------------------	
	void RequestTransferMoneyToPlayer(int amount, string playerID)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RequestTransferMoneyToPlayer - Start");
			
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(amount);
			rpc.Write(playerID);
			rpc.Send(NULL, ExpansionMarketModuleRPC.RequestTransferMoneyToPlayer, true, NULL);
				
			MarketModulePrint("RequestTransferMoneyToPlayer - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestTransferMoneyToPlayer
	// Called on the server
	// ------------------------------------------------------------
	private void RPC_RequestTransferMoneyToPlayer(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestTransferMoneyToPlayer - Start");
		
		if (!senderRPC)
		{
			Error("ExpansionMarketModule::RPC_RequestTransferMoneyToPlayer - Could not get sender identity!");
			return;
		}
		
		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_RequestTransferMoneyToPlayer - Could not get amount!");
			return;
		}
		
		string playerID;
		if (!ctx.Read(playerID))
		{
			Error("ExpansionMarketModule::RPC_RequestTransferMoneyToPlayer - Could not get player id!");
			return;
		}
		
		Exec_RequestTransferMoneyToPlayer(amount, playerID, senderRPC);
		
		MarketModulePrint("RPC_RequestTransferMoneyToPlayer - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestTransferMoneyToPlayer
	// Called on the server
	// ------------------------------------------------------------
	private void Exec_RequestTransferMoneyToPlayer(int amount, string receiverID, PlayerIdentity ident)
	{
		MarketModulePrint("Exec_RequestTransferMoneyToPlayer - Start");
								
		if (!ident)
		{
			Error("ExpansionMarketModule::Exec_RequestTransferMoneyToPlayer - Could not get sender identity!");
			return;
		}
		
		ExpansionMarketATM_Data data_sender = GetPlayerATMData(ident.GetId());
		if (!data_sender)
		{
			Error("ExpansionMarketModule::Exec_RequestTransferMoneyToPlayer - Could not get senders player ATM data!");			
			return;
		}
		
		ExpansionMarketATM_Data data_receiver = GetPlayerATMData(receiverID);
		if (!data_receiver)
		{
			Error("ExpansionMarketModule::Exec_RequestTransferMoneyToPlayer - Could not get receivers player ATM data!");			
			return;
		}
		
		if (data_sender.MoneyDeposited < amount)
		{
			Error("ExpansionMarketModule::Exec_RequestTransferMoneyToPlayer - Tried to transfer more money than in inventory!");
			return;
		}
		
		if (data_receiver.MoneyDeposited + amount > GetExpansionSettings().GetMarket().MaxDepositMoney)
		{
			//! TODO: This case can only be checked server side, so client should be notified in case of failure
			Error("ExpansionMarketModule::Exec_RequestTransferMoneyToPlayer - Receiving player would go over max allowed deposit money value!");
			return;
		}
		
		//! TODO: Something in the commented out code section below potentially caused the identity bug
		//! (player gets same identity as another player, eventually the server crashes) which was fixed with
		//! one (or both) of the following commits:
		//! b90cbd9518b1fcf2b20e41d0bd038b043585ebb7 (Only get identity once)
		//! 466db038831ea4191208fb7e2169c7c619494ce8 (We only use this for logging, comment it for now until ID bug is found)
		//! GetPlayerByUID doesn't seem to have issues elsewhere (it's used extensively by the party module).
		//! Are RPCs in DayZ running in a separate thread? Maybe something is not thread-safe?
		//! Then party module would be affected as well though, which it seemingly isn't.
		//! As we only use this for logging, commented out for now.

		//PlayerBase receiverPlayer = PlayerBase.GetPlayerByUID(receiverID);
		
		//if (!receiverPlayer)
		//{
			//Error("ExpansionMarketModule::Exec_RequestTransferMoneyToPlayer - Could not get player base enity!");
			//return;
		//}
		
		//if (!receiverPlayer.GetIdentity())
		//{
			//Error("ExpansionMarketModule::Exec_RequestTransferMoneyToPlayer - Could not get player identity!");
			//return;
		//}
		
		//! Remove the money from the sender players deposit
		data_sender.RemoveMoney(amount);
		data_sender.Save();
		
		//! Add the money to the receiver players deposit
		data_receiver.AddMoney(amount);
		data_receiver.Save();
		
		//ExpansionLogATM(string.Format("Player \"%1\" (id=%2) has transfered %3 to the player \"%4\" (id=%5).", ident.GetName(), ident.GetId(), amount, receiverPlayer.GetIdentity().GetName(), receiverPlayer.GetIdentity().GetId()));
		
		ConfirmTransferMoneyToPlayer(ident, data_sender);
		
		PlayerBase receiverPlayer = PlayerBase.GetPlayerByUID(receiverID);
		if (!receiverPlayer)
		{
			Error("ExpansionMarketModule::Exec_RequestTransferMoneyToPlayer - Could not get reciver player base enity!");
			return;
		}
		
		ConfirmTransferMoneyToPlayer(receiverPlayer.GetIdentity(), data_receiver);
		
		string senderName = ident.GetName();
		string senderID = ident.GetId();
		string revicerName = receiverPlayer.GetIdentity().GetName();

		StringLocaliser senderText = new StringLocaliser("STR_EXPANSION_ATM_TRANSFER_SENDER", amount.ToString(), revicerName);
		StringLocaliser reciverText = new StringLocaliser("STR_EXPANSION_ATM_TRANSFER_RECEIVER", amount.ToString(), senderName);
		
		ExpansionNotification("STR_EXPANSION_MARKET_TITLE", senderText, EXPANSION_NOTIFICATION_ICON_TRADER, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 3, ExpansionNotificationType.MARKET).Create(ident);
		ExpansionNotification("STR_EXPANSION_MARKET_TITLE", reciverText, EXPANSION_NOTIFICATION_ICON_TRADER, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 3, ExpansionNotificationType.MARKET).Create(receiverPlayer.GetIdentity());
		
		//! This could potentaly cause an identity issue (need to test)
		ExpansionLogATM(string.Format("Player %1 (id=%2) has transfered %3 to the player %4 (id=%5).", senderName, senderID, amount.ToString(), revicerName, receiverID));
		
		MarketModulePrint("Exec_RequestWithdrawMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion ConfirmTransferMoneyToPlayer
	// ------------------------------------------------------------	
	void ConfirmTransferMoneyToPlayer(PlayerIdentity ident, ExpansionMarketATM_Data data)
	{
		MarketModulePrint("ConfirmTransferMoneyToPlayer - Start");
					
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(data);
		rpc.Send(NULL, ExpansionMarketModuleRPC.ConfirmTransferMoneyToPlayer, true, ident);

		MarketModulePrint("ConfirmTransferMoneyToPlayer - End");
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmTransferMoneyToPlayer
	// ------------------------------------------------------------
	private void RPC_ConfirmTransferMoneyToPlayer(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_ConfirmTransferMoneyToPlayer - Start");
	
		ExpansionMarketATM_Data data;
		if (!ctx.Read(data))
		{
			Error("ExpansionMarketModule::RPC_ConfirmTransferMoneyToPlayer - Could not get sender player ATM data!");
			return;
		}
		
		Exec_ConfirmTransferMoneyToPlayer(data);
		
		MarketModulePrint("RPC_ConfirmTransferMoneyToPlayer - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmTransferMoneyToPlayer
	// ------------------------------------------------------------
	private void Exec_ConfirmTransferMoneyToPlayer(ExpansionMarketATM_Data data)
	{
		MarketModulePrint("Exec_ConfirmTransferMoneyToPlayer - Start");
		
		SI_ATMMenuTransferCallback.Invoke(data);
		
		MarketModulePrint("Exec_ConfirmTransferMoneyToPlayer - End");
	}
	
	#ifdef EXPANSIONMOD
	// ------------------------------------------------------------
	// Expansion RequestPartyTransferMoney
	// ------------------------------------------------------------	
	void RequestPartyTransferMoney(int amount, int partyID)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RequestPartyTransferMoney - Start");
						
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(amount);
			rpc.Write(partyID);
			rpc.Send(NULL, ExpansionMarketModuleRPC.RequestPartyTransferMoney, true, NULL);
				
			MarketModulePrint("RequestPartyTransferMoney - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestPartyTransferMoney
	// ------------------------------------------------------------
	private void RPC_RequestPartyTransferMoney(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestPartyTransferMoney - Start");
		
		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_RequestPartyTransferMoney - Could not get amount!");
			return;
		}
		
		int partyID;
		if (!ctx.Read(partyID))
		{
			Error("ExpansionMarketModule::RPC_RequestPartyTransferMoney - Could not get party id!");
			return;
		}
		
		if (!senderRPC)
		{
			Error("ExpansionMarketModule::RPC_RequestPartyTransferMoney - Could not get sender indetity!");
			return;
		}
		
		Exec_RequestPartyTransferMoney(amount, partyID, senderRPC);
		
		MarketModulePrint("RPC_RequestPartyTransferMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestDepositMoney
	// ------------------------------------------------------------
	private void Exec_RequestPartyTransferMoney(int amount, int partyID, PlayerIdentity ident)
	{
		MarketModulePrint("Exec_RequestPartyTransferMoney - Start");
		
		if (!ident)
		{
			Error("ExpansionMarketModule::Exec_RequestPartyTransferMoney - Could not get sender indetity!");
			return;
		}
		
		StringLocaliser title;
		StringLocaliser text;
				
		ExpansionMarketATM_Data data = GetPlayerATMData(ident.GetId());
		if (!data)
		{
			Error("ExpansionMarketModule::Exec_RequestPartyTransferMoney - Could not find player atm data!");
			
			return;
		}
		
		if (data.MoneyDeposited < amount)
		{
			return;
		}
		
		ExpansionPartyModule module = ExpansionPartyModule.Cast(GetModuleManager().GetModule(ExpansionPartyModule));
		if (!module)
		{
			return;
		}
		
		ExpansionPartyData party = module.GetPartyByID(partyID);
		
		if (party.GetMoneyDeposited() + amount > GetExpansionSettings().GetMarket().MaxPartyDepositMoney)
		{
			return;
		}
		
		party.AddMoney(amount);
		party.Save();

		/*int removed = RemoveMoney(player);
		if (removed - amount > 0)
		{
		    SpawnMoney(player, removed - amount);
		}*/
		
		data.RemoveMoney(amount);
		data.Save();
		
		ExpansionLogATM(string.Format("Player \"%1\" (id=%2) has deposited %3 on the party \"%4\" (partyid=%5 | ownerid=%6) ATM account.", ident.GetName(), ident.GetId(), amount, party.GetPartyName(), partyID, party.GetOwnerUID()));
		
		ConfirmPartyTransferMoney(amount, party, data, ident);
		
		MarketModulePrint("Exec_RequestPartyTransferMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestDepositMoney
	// ------------------------------------------------------------
	private void ConfirmPartyTransferMoney(int amount, ExpansionPartyData party, ExpansionMarketATM_Data data, PlayerIdentity ident)
	{
		MarketModulePrint("ConfirmPartyTransferMoney - Start");
							
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(amount);
		party.OnSend(rpc, false);
		rpc.Write(data);
		rpc.Send(NULL, ExpansionMarketModuleRPC.ConfirmPartyTransferMoney, true, ident);
			
		MarketModulePrint("ConfirmPartyTransferMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmPartyTransferMoney
	// ------------------------------------------------------------
	private void RPC_ConfirmPartyTransferMoney(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_ConfirmPartyTransferMoney - Start");
		
		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_ConfirmPartyTransferMoney - Could not get amount!");
			return;
		}
		
		int partyID;
		if (!ctx.Read(partyID))
		{
			Error("ExpansionMarketModule::RPC_ConfirmPartyWithdrawMoney - Could not get party ID!");
			return;
		}

		ExpansionPartyData party = new ExpansionPartyData(partyID);
		if (!party.OnRecieve(ctx))
		{
			Error("ExpansionMarketModule::RPC_ConfirmPartyTransferMoney - Could not get party data!");
			return;
		}
		
		ExpansionMarketATM_Data data;
		if (!ctx.Read(data))
		{
			Error("ExpansionMarketModule::RPC_ConfirmPartyTransferMoney - Could not get sender player ATM data!");
			return;
		}
		
		Exec_ConfirmPartyTransferMoney(amount, party, data);
		
		MarketModulePrint("RPC_ConfirmPartyTransferMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmPartyTransferMoney
	// ------------------------------------------------------------
	private void Exec_ConfirmPartyTransferMoney(int amount, ExpansionPartyData party, ExpansionMarketATM_Data data)
	{
		MarketModulePrint("Exec_ConfirmPartyTransferMoney - Start");
		
		SI_ATMMenuPartyCallback.Invoke(amount, party, data, 1);
		
		MarketModulePrint("Exec_ConfirmPartyTransferMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion RequestPartyWithdrawMoney
	// ------------------------------------------------------------	
	void RequestPartyWithdrawMoney(int amount, int partyID)
	{
		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			MarketModulePrint("RequestPartyWithdrawMoney - Start");
						
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(amount);
			rpc.Write(partyID);
			rpc.Send(NULL, ExpansionMarketModuleRPC.RequestPartyWithdrawMoney, true, NULL);
				
			MarketModulePrint("RequestPartyWithdrawMoney - End");
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestPartyWithdrawMoney
	// ------------------------------------------------------------
	private void RPC_RequestPartyWithdrawMoney(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_RequestPartyWithdrawMoney - Start");
		
		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_RequestPartyWithdrawMoney - Could not get amount!");
			return;
		}
		
		int partyID;
		if (!ctx.Read(partyID))
		{
			Error("ExpansionMarketModule::RPC_RequestPartyWithdrawMoney - Could not get party id!");
			return;
		}
				
		Exec_RequestPartyWithdrawMoney(amount, partyID, senderRPC);
		
		MarketModulePrint("RPC_RequestPartyWithdrawMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestPartyWithdrawMoney
	// ------------------------------------------------------------
	private void Exec_RequestPartyWithdrawMoney(int amount, int partyID, PlayerIdentity ident)
	{
		MarketModulePrint("Exec_RequestPartyWithdrawMoney - Start");
					
		StringLocaliser title;
		StringLocaliser text;
						
		ExpansionMarketATM_Data data = GetPlayerATMData(ident.GetId());
		if (!data)
		{
			Error("ExpansionMarketModule::Exec_RequestPartyWithdrawMoney - Could not find player atm data!");
			
			return;
		}
		
		ExpansionPartyModule module = ExpansionPartyModule.Cast(GetModuleManager().GetModule(ExpansionPartyModule));
		if (!module)
		{
			Error("ExpansionMarketModule::Exec_RequestPartyWithdrawMoney - Could not get party party module!");
			return;
		}
		
		ExpansionPartyData party = module.GetPartyByID(partyID);
		if (!party)
		{
			Error("ExpansionMarketModule::Exec_RequestPartyWithdrawMoney - Could not get party data!");
			return;
		}
		
		ExpansionPartyPlayerData player = party.GetPlayer(ident.GetId());
		if (!player)
		{
			Error("ExpansionMarketModule::Exec_RequestPartyWithdrawMoney - Could not get party player data!");
			return;
		}
		
		if (!player.CanWithdrawMoney())
			return;
		
		if (party.GetMoneyDeposited() < amount)
		{
			return;
		}
		
		party.RemoveMoney(amount);
		party.Save();

		/*int removed = RemoveMoney(player);
		if (removed - amount > 0)
		{
		    SpawnMoney(player, removed - amount);
		}*/
		
		data.AddMoney(amount);
		data.Save();
		
		ExpansionLogATM(string.Format("Player \"%1\" (id=%2) has withdrawn %3 from the party \"%4\" (partyid=%5 | ownerid=%6) ATM account.", ident.GetName(), ident.GetId(), amount, party.GetPartyName(), partyID, party.GetOwnerUID()));
		
		ConfirmPartyWithdrawMoney(amount, party, data, ident);
		
		MarketModulePrint("Exec_RequestPartyWithdrawMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestWithdrawMoney
	// ------------------------------------------------------------
	private void ConfirmPartyWithdrawMoney(int amount, ExpansionPartyData party, ExpansionMarketATM_Data data, PlayerIdentity ident)
	{
		MarketModulePrint("ConfirmPartyWithdrawMoney - Start");
							
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(amount);
		party.OnSend(rpc, false);
		rpc.Write(data);
		rpc.Send(NULL, ExpansionMarketModuleRPC.ConfirmPartyWithdrawMoney, true, ident);
			
		MarketModulePrint("ConfirmPartyWithdrawMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmPartyWithdrawMoney
	// ------------------------------------------------------------
	private void RPC_ConfirmPartyWithdrawMoney(ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		MarketModulePrint("RPC_ConfirmPartyWithdrawMoney - Start");
		
		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionMarketModule::RPC_ConfirmPartyWithdrawMoney - Could not get amount!");
			return;
		}
		
		int partyID;
		if (!ctx.Read(partyID))
		{
			Error("ExpansionMarketModule::RPC_ConfirmPartyWithdrawMoney - Could not get party ID!");
			return;
		}

		ExpansionPartyData party = new ExpansionPartyData(partyID);
		if (!party.OnRecieve(ctx))
		{
			Error("ExpansionMarketModule::RPC_ConfirmPartyWithdrawMoney - Could not get party data!");
			return;
		}
		
		ExpansionMarketATM_Data data;
		if (!ctx.Read(data))
		{
			Error("ExpansionMarketModule::RPC_ConfirmPartyWithdrawMoney - Could not get sender player ATM data!");
			return;
		}
		
		Exec_ConfirmPartyWithdrawMoney(amount, party, data);
		
		MarketModulePrint("RPC_ConfirmPartyWithdrawMoney - End");
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmPartyWithdrawMoney
	// ------------------------------------------------------------
	private void Exec_ConfirmPartyWithdrawMoney(int amount, ExpansionPartyData party, ExpansionMarketATM_Data data)
	{
		MarketModulePrint("Exec_ConfirmPartyWithdrawMoney - Start");
		
		SI_ATMMenuPartyCallback.Invoke(amount, party, data, 2);
		
		MarketModulePrint("Exec_ConfirmPartyWithdrawMoney - End");
	}
	#endif
	
	// ------------------------------------------------------------
	// Expansion ExpansionLogMarket
	// ------------------------------------------------------------
	private void ExpansionLogMarket(string message)
	{
		if (GetExpansionSettings().GetLog().Market)
			GetExpansionSettings().GetLog().PrintLog("[Market] " + message);
	}
	
	// ------------------------------------------------------------
	// Expansion ExpansionLogATM
	// ------------------------------------------------------------
	private void ExpansionLogATM(string message)
	{
		if (GetExpansionSettings().GetLog().ATM)
			GetExpansionSettings().GetLog().PrintLog("[ATM Locker] " + message);
	}
}