/**
 * ExpansionDataCollectionModule.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionDataCollectionModule
 * @brief		This class handles the data collection system
 **/
//! TODO: Get rid of RPCs and use SyncPlayerList on client
[CF_RegisterModule(ExpansionDataCollectionModule)]
class ExpansionDataCollectionModule: CF_ModuleWorld
{
	private autoptr map <string, ref ExpansionPlayerDataCollection> m_PlayerData;
	static ref ScriptInvoker m_ModuleSI = new ScriptInvoker();
	
	void ExpansionDataCollectionModule()
	{		
		m_PlayerData = new map <string, ref ExpansionPlayerDataCollection>;
	}
	
	override void OnInit()
	{
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		super.OnInit();

		EnableClientDisconnect();
		EnableInvokeConnect();
		EnableRPC();
	}

	override int GetRPCMin()
	{
		return ExpansionDataCollectionRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return ExpansionDataCollectionRPC.COUNT;
	}
	
	override void OnRPC(Class sender, CF_EventArgs args)
	{
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		super.OnRPC(sender, args);

		auto rpc = CF_EventRPCArgs.Cast(args);

		switch ( rpc.ID )
		{
		case ExpansionDataCollectionRPC.RequestPlayerData:
			RPC_RequestPlayerData(rpc.Context, rpc.Sender);
			break;
		case ExpansionDataCollectionRPC.SendPlayerData:
			RPC_SendPlayerData(rpc.Context);
			break;
		}
	}
		
	override void OnInvokeConnect(Class sender, CF_EventArgs args)
	{
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		super.OnInvokeConnect(sender, args);
		
		auto cArgs = CF_EventPlayerArgs.Cast(args);

		if (!cArgs.Player)
			return;
		
		OnPlayerConnect(cArgs.Player);
	}
	
	private void OnPlayerConnect(PlayerBase player)
	{		
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		if (!IsMissionHost())
			return;
		
		string playerUID = player.GetIdentity().GetId();
		ExpansionPlayerDataCollection playerData;
		if (!m_PlayerData.Find(playerUID, playerData))
		{
			playerData = new ExpansionPlayerDataCollection();
			m_PlayerData.Insert(playerUID, playerData);
		}

		playerData.SetFromPlayerBase(player);
	}
	
	override void OnClientDisconnect(Class sender, CF_EventArgs args)
	{
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		super.OnClientDisconnect(sender, args);

		auto cArgs = CF_EventPlayerDisconnectedArgs.Cast(args);

		OnPlayerDisconnect(cArgs.UID);
	}
	
	private void OnPlayerDisconnect(string playerUID)
	{		
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		if (!IsMissionHost())
			return;
		
		if (m_PlayerData.Contains(playerUID))
		{
			m_PlayerData.Remove(playerUID);
		}
	}
	
	//! Client
	void RequestPlayerData()
	{		
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		if (!IsMissionClient())
			return;
		
		ScriptRPC rpc = new ScriptRPC();
 		rpc.Send(NULL, ExpansionDataCollectionRPC.RequestPlayerData, false);
	}
	
	//! Server
	void RPC_RequestPlayerData(ParamsReadContext ctx, PlayerIdentity sender)
	{		
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		if (!IsMissionHost())
			return;
		
		ScriptRPC rpc = new ScriptRPC();		
		int playersClount = m_PlayerData.Count();
		rpc.Write(playersClount);
	
		foreach (auto playerData: m_PlayerData)
		{
			if (!playerData)
				continue;
			
			playerData.OnSend(rpc);
		}
		
 		rpc.Send(NULL, ExpansionDataCollectionRPC.SendPlayerData, false, sender);
	}
	
	//! Client
	void RPC_SendPlayerData(ParamsReadContext ctx)
	{
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		if (!IsMissionClient())
			return;
		
		int playersCount;
		if (!ctx.Read(playersCount))
			return;

		m_PlayerData.Clear();
		
		for (int i = 0; i < playersCount; i++)
		{
			if (!OnReceivePlayerData(ctx))
				continue;
		}
		
		if (!m_ModuleSI)
			return;
		
		m_ModuleSI.Invoke();
	}
	
	bool OnReceivePlayerData(ParamsReadContext ctx)
	{		
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		string playerUID;
		if (Expansion_Assert_False(ctx.Read(playerUID), "Failed to read player UID"))
			return false;

		ExpansionPlayerDataCollection playerData;
		if (!m_PlayerData.Find(playerUID, playerData))
		{
			playerData = new ExpansionPlayerDataCollection();
			playerData.PlayerUID = playerUID;
			m_PlayerData.Insert(playerUID, playerData);
		}

		if (Expansion_Assert_False(playerData.OnRecieve(ctx), "Failed to read player data"))
			return false;
		
		return true;
	}
	
	map <string, ref ExpansionPlayerDataCollection> GetAllPlayers()
	{
		return m_PlayerData;
	}
};

class ExpansionDataCollection
{
	bool OnRecieve(ParamsReadContext ctx);
	void OnSend(ParamsWriteContext ctx);
};

class ExpansionPlayerDataCollection: ExpansionDataCollection
{
	string PlayerUID;
	string Name;
	
	void SetFromPlayerBase(PlayerBase player)
	{
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		PlayerUID = player.GetIdentity().GetId();
		Name = player.GetIdentity().GetName();
	}
	
	override void OnSend(ParamsWriteContext ctx)
	{
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		ctx.Write(PlayerUID);
		ctx.Write(Name);
	}
	
	override bool OnRecieve(ParamsReadContext ctx)
	{
		auto trace = EXTrace.Start(ExpansionTracing.DATACOLLECTION);

		if (!ctx.Read(Name))
			return false;
		
		return true;
	}
};