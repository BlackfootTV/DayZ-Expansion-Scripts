/**
 * MissionGameplay.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		MissionGameplay
 * @brief		
 **/
modded class MissionGameplay
{
	Chat 										m_Chat;
	//! Expansion chat channel counter
	ExpansionChatChannels 						m_ChatChannel;
	//! Expansion chat fade timer
	ref ExpansionWidgetFadeTimer 				exp_m_ChannelFadeTimer;
	//! Expansion chat fade timeout timer
	ref Timer 									exp_m_ChannelTimeoutTimer;
	//! Expansion chat fade timer
	ref ExpansionWidgetFadeTimer 				exp_m_ChannelNameFadeTimer;
	//! Expansion chat fade timeout timer
	ref Timer 									exp_m_ChannelNameTimeoutTimer;
	//! Expansion HUD Root Widget
	Widget 										m_ExpansionHudRootWidget;
	//! Expansion chat channel widget
	Widget 										m_WidgetChatChannel;
	//! Expansion chat channel name widget
	TextWidget 									m_ChatChannelName;
	//! Expansion Hud
	ref ExpansionIngameHud 						m_ExpansionHud;
	//!	Earplug check
	protected bool 								m_WasEarplugToggled;
	//! Client/Player Data
	protected bool 								m_DataSent;
	
	Widget										m_ChatRootWidget;
	
	protected bool								m_WasGPSToggled;
	
	Widget										m_ChatPanel;
	Widget										m_VoiceLevelSeperator;
	ref WidgetFadeTimer							m_SeperatorFadeTimer;

	//! Modules
	ref ExpansionAutorunModule 					m_AutoRunModule;
	ExpansionMarkerModule 						m_MarkerModule;
	private bool								m_MarkerToggleState = true;
	private bool								m_ServerMarkerToggleState = true;
	private bool 								m_HideChatToggle = true;

	protected ref MapMenu 						m_MapMenu;
	protected ref ExpansionMapMenu 				m_ExpansionMapMenu;
	
	// ------------------------------------------------------------
	// Constructor
	// ------------------------------------------------------------
	void MissionGameplay()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::MissionGameplay - Start");
		#endif
		
		m_ExpansionHudRootWidget = null;		
		m_ChatRootWidget = null;
		
		exp_m_ChannelFadeTimer = new ExpansionWidgetFadeTimer();
		exp_m_ChannelTimeoutTimer = new Timer(CALL_CATEGORY_GUI);
		exp_m_ChannelNameFadeTimer = new ExpansionWidgetFadeTimer();
		exp_m_ChannelNameTimeoutTimer = new Timer(CALL_CATEGORY_GUI);

		Class.CastTo( m_AutoRunModule, GetModuleManager().GetModule( ExpansionAutorunModule ) );
		Class.CastTo( m_MarkerModule, GetModuleManager().GetModule( ExpansionMarkerModule ) );

		m_DataSent = false;
			
		m_SeperatorFadeTimer = new WidgetFadeTimer;
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::MissionGameplay - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Expansion SwitchChatChannelToGlobal
	// ------------------------------------------------------------
	private void SwitchChatChannelToGlobal()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToGlobal - Start");
		#endif
		
		if ( GetExpansionSettings().GetGeneral().EnableGlobalChat )
		{
			m_ChatChannel = ExpansionChatChannels.CCGlobal;

			m_ChatChannelName.SetText("Global Chat");
			m_ChatChannelName.SetColor(GetExpansionSettings().GetGeneral().ChatColors.Get("GlobalChatColor"));
		} else
		{
			SwitchChatChannelToTeam();
		}

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToGlobal - End");
		#endif
	}

	// ------------------------------------------------------------
	// Expansion SwitchChatChannelToTeam
	// ------------------------------------------------------------
	private void SwitchChatChannelToTeam()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToGlobal - Start");
		#endif

		ExpansionPartyModule partyModule = ExpansionPartyModule.Cast(GetModuleManager().GetModule(ExpansionPartyModule));
		
		if (partyModule.HasParty() && GetExpansionSettings().GetGeneral().EnablePartyChat)
		{
			m_ChatChannel = ExpansionChatChannels.CCTeam;

			m_ChatChannelName.SetText("Team Chat");
			m_ChatChannelName.SetColor(GetExpansionSettings().GetGeneral().ChatColors.Get("PartyChatColor"));
		} else
		{
			SwitchChatChannelToTransport();
		}

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToGlobal - End");
		#endif
	}

	// ------------------------------------------------------------
	// Expansion SwitchChatChannelToTransport
	// ------------------------------------------------------------
	private void SwitchChatChannelToTransport()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToTransport - Start");
		#endif

		PlayerBase player = PlayerBase.Cast( g_Game.GetPlayer() );
		Object parent = Object.Cast( player.GetParent() );

		if (parent && parent.IsTransport() && GetExpansionSettings().GetGeneral().EnableTransportChat)
		{
			m_ChatChannel = ExpansionChatChannels.CCTransport;

			m_ChatChannelName.SetText("Transport Chat");
			m_ChatChannelName.SetColor(GetExpansionSettings().GetGeneral().ChatColors.Get("TransportChatColor"));
		} else
		{
			SwitchChatChannelToAdmin();
		}

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToTransport - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Expansion SwitchChatChannelToAdmin
	// ------------------------------------------------------------
	private void SwitchChatChannelToAdmin()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToAdmin - Start");
		#endif

		if (GetPermissionsManager().HasPermission("Admin.Chat"))
		{
			m_ChatChannel = ExpansionChatChannels.CCAdmin;

			m_ChatChannelName.SetText("Admin Chat");
			m_ChatChannelName.SetColor(GetExpansionSettings().GetGeneral().ChatColors.Get("AdminChatColor"));
		} 
		else
		{
			SwitchChatChannelToDirect();
		}

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToAdmin - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Expansion SwitchChatChannelToDirect
	// ------------------------------------------------------------
	private void SwitchChatChannelToDirect()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToDirect - Start");
		#endif

		m_ChatChannel = ExpansionChatChannels.CCDirect;

		m_ChatChannelName.SetText("Direct Chat");
		m_ChatChannelName.SetColor(GetExpansionSettings().GetGeneral().ChatColors.Get("DirectChatColor"));

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChatChannelToDirect - End");
		#endif
	}

	// ------------------------------------------------------------
	// Expansion SwitchChannel
	// ------------------------------------------------------------
	void SwitchChannel()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChannel - Start");
		#endif

		switch ( m_ChatChannel )
		{
			case ExpansionChatChannels.CCDirect:
				SwitchChatChannelToGlobal();
				break;
			case ExpansionChatChannels.CCGlobal:
				SwitchChatChannelToTeam();
				break;
			case ExpansionChatChannels.CCTeam:
				SwitchChatChannelToTransport();
				break;
			case ExpansionChatChannels.CCTransport:
				SwitchChatChannelToAdmin();
				break;
			case ExpansionChatChannels.CCAdmin:
				SwitchChatChannelToDirect();
				break;
			default:
				SwitchChatChannelToDirect();
				break;
		}

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::SwitchChannel - End");
		#endif
	}

	// ------------------------------------------------------------
	// Expansion GetChatChannel
	// ------------------------------------------------------------
	int GetChatChannel()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint( "MissionGameplay::GetChatChannel" );
		#endif

		return m_ChatChannel;
	}

	// ------------------------------------------------------------
	// OnInit
	// ------------------------------------------------------------
	override void ShowChat()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::ShowChat - Start");
		#endif

		if (g_Game.GetProfileOption(EDayZProfilesOptions.PLAYER_MESSAGES))
			return;

		//! If we are no longer in a vehicle and last used channel was transport, switch to direct
		if (m_ChatChannel == ExpansionChatChannels.CCTransport)
		{
			PlayerBase player = PlayerBase.Cast( g_Game.GetPlayer() );
			Object parent = Object.Cast( player.GetParent() );

			if (!parent || !parent.IsTransport() || !GetExpansionSettings().GetGeneral().EnableTransportChat)
				SwitchChatChannelToDirect();
		} 
		else if ( m_ChatChannel == ExpansionChatChannels.CCDirect )
		{
			//! Initialize direct chat channel name to correct color
			m_ChatChannelName.SetColor(GetExpansionSettings().GetGeneral().ChatColors.Get("DirectChatColor"));
		}
				
		exp_m_ChannelNameTimeoutTimer.Stop();
		exp_m_ChannelTimeoutTimer.Stop();
		m_WidgetChatChannel.SetAlpha( 0.6 );
		m_ChatChannelName.SetAlpha( 1 );
		m_WidgetChatChannel.Show( true );
		m_ChatChannelName.Show( true );
		
		int level = GetGame().GetVoiceLevel();
		UpdateVoiceLevelWidgets( level );

		m_UIManager.EnterScriptedMenu( MENU_CHAT_INPUT, NULL );
	
		PlayerControlDisable( INPUT_EXCLUDE_ALL );
		
		PlayerControlDisable( INPUT_EXCLUDE_CHAT_EXPANSION );
				
		GetUApi().GetInputByName("UAPersonView").Supress();	
		GetUApi().GetInputByName( "UAPersonView" ).ForceDisable( true );
		GetGame().GetUIManager().ShowUICursor( true );
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::ShowChat - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Override HideChat
	// ------------------------------------------------------------
	override void HideChat()
	{
		super.HideChat();
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::HideChat - Start");
		#endif

		GetUApi().GetInputByName( "UAPersonView" ).ForceDisable( false );
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::HideChat - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Override PlayerControlDisable
	// ------------------------------------------------------------
	override void PlayerControlDisable(int mode)
	{
		switch (mode)
		{
			case INPUT_EXCLUDE_CHAT_EXPANSION:
			{
				GetUApi().ActivateExclude("chatexpansion");
				GetUApi().UpdateControls();
				break;
			}
		}
		
		super.PlayerControlDisable(mode);
	}

	// ------------------------------------------------------------
	// OnInit
	// ------------------------------------------------------------
	override void OnInit()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnInit - Start");
		#endif

		if ( m_Initialized )
		{
			#ifdef EXPANSIONEXPRINT
			EXPrint("MissionGameplay::OnInit - End");
			#endif

			return;
		}
		
		super.OnInit();
		
		//! Expansion Hud
		InitExpansionHud();
		
		//! Expansion Chat
		InitChat();
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnInit - End");
		#endif
	}
	
	override void OnMissionFinish()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnMissionFinish - Start");
		#endif

		super.OnMissionFinish();

		if ( m_MapMenu )
			m_MapMenu.CloseMapMenu();  //! Safely destroys map menu
		
		if ( m_ExpansionMapMenu )
			m_ExpansionMapMenu.CloseMapMenu(true);	//! Safely destroys expansion map menu

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnMissionFinish - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// InitExpansionHud
	// ------------------------------------------------------------
	void InitExpansionHud()
	{
		if ( !m_ExpansionHudRootWidget )
		{
			m_ExpansionHudRootWidget = GetGame().GetWorkspace().CreateWidgets("DayZExpansion/GUI/layouts/hud/expansion_hud.layout");
			m_ExpansionHudRootWidget.Show(false);
			
			m_ExpansionHud = new ExpansionIngameHud;
			m_ExpansionHud.Init(m_ExpansionHudRootWidget);
			
			InitVoiceLevelIndicator();
		}
	}
	
	// ------------------------------------------------------------
	// InitChat
	// ------------------------------------------------------------
	void InitChat()
	{
		if ( !m_ChatRootWidget && m_HudRootWidget )
		{
			m_ChatPanel = Widget.Cast( m_HudRootWidget.FindAnyWidget("ChatFrameWidget") );
			m_ChatPanel.SetFlags( WidgetFlags.IGNOREPOINTER, true );
			
			m_ChatRootWidget = GetGame().GetWorkspace().CreateWidgets( "DayZExpansion/GUI/layouts/chat/expansion_chat_panel.layout", m_ChatPanel );
			m_Chat.Init( m_ChatRootWidget );
	
			if ( m_Chat )
			{
				m_WidgetChatChannel = Widget.Cast( m_ExpansionHudRootWidget.FindAnyWidget("ChatChannelPanel") );
				m_ChatChannelName = TextWidget.Cast( m_ExpansionHudRootWidget.FindAnyWidget("ChatChannelName") );
	
				m_WidgetChatChannel.SetAlpha(0);
				m_ChatChannelName.SetAlpha(0);
	
				m_WidgetChatChannel.Show(false);
				m_ChatChannelName.Show(false);
	
				SwitchChatChannelToDirect();
			}
			
			m_ChatRootWidget.Show( m_HideChatToggle );
		}
	}
	
	// ------------------------------------------------------------
	// HideChatToggle
	// ------------------------------------------------------------
	void HideChatToggle()
	{
		m_HideChatToggle = !m_HideChatToggle;

		m_ChatRootWidget.Show( m_HideChatToggle );
	}

	// ------------------------------------------------------------
	// GetChatToggleState
	// ------------------------------------------------------------
	bool GetChatToggleState()
	{
		return m_HideChatToggle;
	}
	
	// ------------------------------------------------------------
	// InitChat
	// ------------------------------------------------------------
	void InitVoiceLevelIndicator()
	{
		//! Unlink vanilla voice level indicator
		m_HudRootWidget.FindAnyWidget("mic").Unlink();
		m_HudRootWidget.FindAnyWidget("VoiceLevelsPanel").Unlink();
		
		m_VoiceLevelsWidgets.Clear();
		m_VoiceLevelTimers.Clear();
		
		//! Von enabled icon
		m_MicrophoneIcon = ImageWidget.Cast( m_ExpansionHudRootWidget.FindAnyWidget("mic") );
		m_MicrophoneIcon.Show(false);
		
		//! Seperator
		m_VoiceLevelSeperator = Widget.Cast( m_ExpansionHudRootWidget.FindAnyWidget("BadgesSpacer") );
		m_VoiceLevelSeperator.Show(false);
		
		//! Von voice level
		m_VoiceLevels = m_ExpansionHudRootWidget.FindAnyWidget("VoiceLevelsPanel");
		m_VoiceLevelsWidgets = new map<int, ImageWidget>; // [key] voice level
		m_VoiceLevelTimers = new map<int,ref WidgetFadeTimer>; // [key] voice level
	
		if( m_VoiceLevels )
		{
			m_VoiceLevelsWidgets.Set(VoiceLevelWhisper, ImageWidget.Cast( m_VoiceLevels.FindAnyWidget("Whisper") ));
			m_VoiceLevelsWidgets.Set(VoiceLevelTalk, ImageWidget.Cast( m_VoiceLevels.FindAnyWidget("Talk") ));
			m_VoiceLevelsWidgets.Set(VoiceLevelShout, ImageWidget.Cast( m_VoiceLevels.FindAnyWidget("Shout") ));
			
			m_VoiceLevelTimers.Set(VoiceLevelWhisper, new WidgetFadeTimer);
			m_VoiceLevelTimers.Set(VoiceLevelTalk, new WidgetFadeTimer);
			m_VoiceLevelTimers.Set(VoiceLevelShout, new WidgetFadeTimer);
		}
		
		HideVoiceLevelWidgets();
	}

	// ------------------------------------------------------------
	// OnResizeScreen
	// ------------------------------------------------------------
	void OnResizeScreen()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnResizeScreen - Start");
		#endif

		m_Chat.Init( m_ChatRootWidget );

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnResizeScreen - End");
		#endif
	}

	// ------------------------------------------------------------
	// OnUpdate
	// ------------------------------------------------------------
	override void OnUpdate( float timeslice )
	{	
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnUpdate - Start");
		#endif

		super.OnUpdate( timeslice );

		if ( !m_bLoaded )
		{
			#ifdef EXPANSIONEXPRINT
			EXPrint("MissionGameplay::OnUpdate - End");
			#endif

			return;
		}

		//! Checking for keyboard focus
		bool inputIsFocused = false;
		
		//! Reference to focused windget
		Widget focusedWidget = GetFocus();

		if ( focusedWidget )
		{
			if ( focusedWidget.ClassName().Contains( "EditBoxWidget" ) )
			{
				inputIsFocused = true;
			} 
			else if ( focusedWidget.ClassName().Contains( "MultilineEditBoxWidget" ) )
			{
				inputIsFocused = true;
			}
		}

		Man man 				= GetGame().GetPlayer(); 	//! Refernce to man
		Input input 			= GetGame().GetInput(); 	//! Reference to input
		UIScriptedMenu topMenu 	= m_UIManager.GetMenu(); 	//! Expansion reference to menu
		PlayerBase playerPB 	= PlayerBase.Cast( man );	//! Expansion reference to player
		ExpansionScriptViewMenu viewMenu 	= ExpansionScriptViewMenu.Cast(GetDayZExpansion().GetExpansionUIManager().GetMenu());
		ExpansionPlayerListMenu playerListMenu = ExpansionPlayerListMenu.Cast(GetDayZExpansion().GetExpansionUIManager().GetMenu());
		
		if ( playerPB && playerPB.GetHumanInventory() ) 
		{
			//! Expansion reference to item in hands
			EntityAI itemInHands = playerPB.GetHumanInventory().GetEntityInHands();

			//! Expansion reference to hologram
			Hologram hologram;	

			if ( playerPB.GetPlayerState() == EPlayerStates.ALIVE && !playerPB.IsUnconscious() )
			{
				//TODO: Make ExpansionInputs class and handle stuff there to keep this clean

				//! Chat
				if ( input.LocalPress( "UAChat", false ) && !inputIsFocused && !topMenu )
				{
					ShowChat();
				}
				
				//! Chat
				if ( input.LocalPress( "UAExpansionHideChatToggle", false ) && !inputIsFocused && !topMenu )
				{
					HideChatToggle();
					m_Chat.HideChatToggle();				

					if (m_HideChatToggle)
					{	
						ExpansionNotification("STR_EXPANSION_CHATTOGGLE_TITLE", "STR_EXPANSION_CHATTOGGLE_ON", EXPANSION_NOTIFICATION_ICON_T_Walkie_Talkie, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 5).Info(playerPB.GetIdentity());
					} else {
						ExpansionNotification("STR_EXPANSION_CHATTOGGLE_TITLE", "STR_EXPANSION_CHATTOGGLE_OFF", EXPANSION_NOTIFICATION_ICON_T_Walkie_Talkie, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 5).Info(playerPB.GetIdentity());
					}
				}
				
				//! Switch Chat Channel
				if ( input.LocalPress( "UAExpansionChatSwitchChannel", false ) && !inputIsFocused )
				{
					SwitchChannel();

					exp_m_ChannelNameFadeTimer.FadeIn(m_ChatChannelName, EXP_FADE_IN_DURATION);
					exp_m_ChannelNameTimeoutTimer.Run(EXP_FADE_TIMEOUT, exp_m_ChannelNameFadeTimer, "FadeOut", new Param2<Widget, float>(m_ChatChannelName, EXP_FADE_OUT_DURATION));

					exp_m_ChannelFadeTimer.FadeIn(m_WidgetChatChannel, EXP_FADE_IN_DURATION);
					exp_m_ChannelTimeoutTimer.Run(EXP_FADE_TIMEOUT, exp_m_ChannelFadeTimer, "FadeOut", new Param2<Widget, float>(m_WidgetChatChannel, EXP_FADE_OUT_DURATION));
				}

				if ( !topMenu && !inputIsFocused )
				{
					//! Autorun
					if ( input.LocalPress( "UAExpansionAutoRunToggle", false ) )
					{
						if ( !man.GetParent() && GetExpansionSettings() && GetExpansionSettings().GetGeneral().EnableAutoRun )
						{
							m_AutoRunModule.AutoRun();
						}
					}
					
					//! Map Menu
					if ( input.LocalPress( "UAExpansionMapToggle", false ) && !viewMenu )
					{
						ToggleMapMenu(playerPB);
					}
					
					//! GPS	
					if ( input.LocalPress( "UAExpansionGPSToggle", false ) )
					{
						#ifdef EXPANSIONEXLOGPRINT
						EXLogPrint("MissionGameplay::OnUpdate - UAExpansionGPSToggle pressed and setting for item is: " + GetExpansionSettings().GetMap().NeedGPSItemForKeyBinding.ToString() );
						#endif

						if ( GetExpansionSettings() && GetExpansionSettings().GetMap().EnableHUDGPS )
						{
							if ( GetExpansionSettings().GetMap().NeedGPSItemForKeyBinding )
							{
								#ifdef EXPANSIONEXLOGPRINT
								EXLogPrint("MissionGameplay::OnUpdate - UAExpansionGPSToggle pressed and player has gps: " + PlayerBase.Cast( GetGame().GetPlayer() ).HasItemGPS().ToString() );
								#endif
								
								if ( PlayerBase.Cast( GetGame().GetPlayer() ).HasItemGPS() )
									ToggleHUDGPSMode();
							}
							else
							{
								ToggleHUDGPSMode();
							}
						}
					}
					
					if ( input.LocalPress( "UAExpansionGPSMapScaleDown", false ) )
					{
						if ( GetExpansionSettings() && GetExpansionSettings().GetMap().EnableHUDGPS && m_ExpansionHud.IsInitialized() && m_ExpansionHud.GetGPSMapState() )
						{							
							DecreaseGPSMapScale();
						}
					}
					
					if ( input.LocalPress( "UAExpansionGPSMapScaleUp", false ) )
					{
						if ( GetExpansionSettings() && GetExpansionSettings().GetMap().EnableHUDGPS && m_ExpansionHud.IsInitialized() && m_ExpansionHud.GetGPSMapState() )
						{
							IncreaseGPSMapScale();
						}
					}

					//! Expansion Compass Hud
					if ( input.LocalPress( "UAExpansionCompassToggle", false ) )
					{
						if ( GetExpansionSettings().GetMap().EnableHUDCompass )
						{
							m_ExpansionHud.SetCompassToggleState();
						}
					}
					
					//! Expansion Hud
					if ( input.LocalHold( "UAUIQuickbarToggle", false ) )
					{
						if ( !m_Hud.GetHudState() )
						{
							m_ExpansionHud.ShowHud( false );
						} else
						{
							m_ExpansionHud.ShowHud( true );
						}
					}
					
					//! Gestures
					if ( input.LocalPress( "UAUIGesturesOpen",false ) )
					{
						//! Open gestures menu
						if ( !playerPB.IsRaised() && !playerPB.GetCommand_Vehicle() )
						{
							if ( !GetUIManager().IsMenuOpen( MENU_GESTURES ) )
							{
								m_ExpansionHud.ShowHud( false );
							}
						}
					}
					
					//! Toggle Earplugs
					if ( input.LocalPress( "UAExpansionEarplugsToggle", false )  && !viewMenu )
					{
						m_ExpansionHud.ToggleEarplugs();
					}
					
					//! Toggle Player list menu
					if ( input.LocalPress("UAExpansionPlayerListToggle", false) )
					{
						if ((playerListMenu || !topMenu) && !inputIsFocused)
						{
							OnPlayerListTogglePressed();
						}
					}

					if (m_MarkerModule)
					{
						if (input.LocalPress("UAExpansion3DMarkerToggle", false)  && !viewMenu) 
						{
							
							m_MarkerToggleState = !m_MarkerToggleState;
							m_ServerMarkerToggleState = m_MarkerToggleState;
							
							if (m_MarkerToggleState) {
								ExpansionNotification("STR_EXPANSION_MARKERTOGGLE_TITLE", "STR_EXPANSION_MARKERTOGGLEALL_OFF", EXPANSION_NOTIFICATION_ICON_MARKER, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 5).Info(playerPB.GetIdentity());
								m_MarkerModule.SetVisibility(ExpansionMapMarkerType.SERVER, EXPANSION_MARKER_VIS_WORLD);
								m_MarkerModule.SetVisibility(ExpansionMapMarkerType.PARTY, EXPANSION_MARKER_VIS_WORLD);
								m_MarkerModule.SetVisibility(ExpansionMapMarkerType.PLAYER, EXPANSION_MARKER_VIS_WORLD);
								m_MarkerModule.SetVisibility(ExpansionMapMarkerType.PERSONAL, EXPANSION_MARKER_VIS_WORLD);
								
							} else {
								ExpansionNotification("STR_EXPANSION_MARKERTOGGLE_TITLE", "STR_EXPANSION_MARKERTOGGLEALL_ON", EXPANSION_NOTIFICATION_ICON_MARKER, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 5).Info(playerPB.GetIdentity());
								m_MarkerModule.RemoveVisibility(ExpansionMapMarkerType.SERVER, EXPANSION_MARKER_VIS_WORLD);
								m_MarkerModule.RemoveVisibility(ExpansionMapMarkerType.PARTY, EXPANSION_MARKER_VIS_WORLD);
								m_MarkerModule.RemoveVisibility(ExpansionMapMarkerType.PLAYER, EXPANSION_MARKER_VIS_WORLD);
								m_MarkerModule.RemoveVisibility(ExpansionMapMarkerType.PERSONAL, EXPANSION_MARKER_VIS_WORLD);
							}
						}
						
						if (input.LocalPress("UAExpansionServerMarkersToggle", false)  && !viewMenu) 
						{
							m_ServerMarkerToggleState = !m_ServerMarkerToggleState;
							
							if (m_ServerMarkerToggleState) {
								ExpansionNotification("STR_EXPANSION_MARKERTOGGLE_TITLE", "STR_EXPANSION_MARKERTOGGLESERVER_OFF", EXPANSION_NOTIFICATION_ICON_MARKER, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 5).Info(playerPB.GetIdentity());
								m_MarkerModule.SetVisibility(ExpansionMapMarkerType.SERVER, EXPANSION_MARKER_VIS_WORLD);
								
							} else {
								ExpansionNotification("STR_EXPANSION_MARKERTOGGLE_TITLE", "STR_EXPANSION_MARKERTOGGLESERVER_ON", EXPANSION_NOTIFICATION_ICON_MARKER, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 5).Info(playerPB.GetIdentity());
								m_MarkerModule.RemoveVisibility(ExpansionMapMarkerType.SERVER, EXPANSION_MARKER_VIS_WORLD);
							}
						}
					}
				}

				//! Basebuilding Snaping
				if ( playerPB && playerPB.IsPlacingLocal() && !inputIsFocused )
				{
					hologram = playerPB.GetHologramLocal();

					if ( hologram )
					{
						if ( input.LocalPress( "UAExpansionSnappingToggle" ) )
						{
							hologram.SetUsingSnap( !hologram.IsUsingSnap() );
							
							if ( hologram.IsUsingSnap() )
							{
								ExpansionNotification("STR_EXPANSION_SNAPPING_TITLE", "STR_EXPANSION_SNAPPING_ENABLED", EXPANSION_NOTIFICATION_ICON_INFO, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 5).Info(playerPB.GetIdentity());
							} else {
								ExpansionNotification("STR_EXPANSION_SNAPPING_TITLE", "STR_EXPANSION_SNAPPING_DISABLED", EXPANSION_NOTIFICATION_ICON_INFO, COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 5).Info(playerPB.GetIdentity());
							}
						}

						if ( input.LocalValue( "UAExpansionSnappingDirectionNext" ) != 0 )
						{
							hologram.NextDirection();
						}

						if ( input.LocalValue( "UAExpansionSnappingDirectionPrevious" ) != 0 )
						{
							hologram.PreviousDirection();
						}
					}
				}

				if ( m_AutoRunModule )
				{
					//! Autowalk
					if ( GetExpansionSettings() && GetExpansionSettings().GetGeneral().EnableAutoRun )
					{
						m_AutoRunModule.UpdateAutoWalk();
					}
					
					//! Stop autorun when different inputs are pressed
					if ( !m_AutoRunModule.IsDisabled() )
					{
						if ( INPUT_FORWARD() || INPUT_BACK() || INPUT_LEFT() || INPUT_RIGHT() || INPUT_STANCE() )
						{
							m_AutoRunModule.AutoRun();
						}
					}
				}
						
				//! Data
				if ( !m_DataSent ) 
				{
					ExpansionPlayerData();
					m_DataSent = true;
				}
			}
			
			//! Nightvision check
			if ( GetExpansionSettings() && GetExpansionSettings().GetGeneral().EnableHUDNightvisionOverlay )
			{
				PlayerCheckNV( playerPB );
			}
		}
		
		//! Expansion hud updater
		if ( m_Hud &&  m_ExpansionHud.IsInitialized() )
			m_ExpansionHud.Update( timeslice );
		
		//! Chat updater
		m_Chat.Update( timeslice );

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnUpdate - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// ToggleMapMenu
	// ------------------------------------------------------------
	void ToggleMapMenu(PlayerBase playerPB)
	{
		if (!GetExpansionSettings().GetMap().EnableMap)
		{
			bool show_map;
			if (m_MapMenu && m_MapMenu.IsVisible())
			{
				m_MapMenu.CloseMapMenu();
			} 
			else if (!GetGame().GetUIManager().GetMenu() && GetExpansionSettings().GetMap() && GetExpansionSettings().GetMap().CanOpenMapWithKeyBinding)
			{
				if (GetExpansionSettings().GetMap().NeedMapItemForKeyBinding)
				{
					if (playerPB.HasItemMap() || playerPB.HasItemGPS())
						show_map = true;
				} 
				else
				{
					show_map = true;
				}
	
				if (show_map)
				{
					if (m_MapMenu)
					{
						GetGame().GetUIManager().ShowScriptedMenu(m_MapMenu, NULL);
					} 
					else
					{
						m_MapMenu = MapMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(MENU_MAP, NULL));
					}
				}
			}
		}
		else
		{
			if (m_ExpansionMapMenu && m_ExpansionMapMenu.IsVisible())
			{
				m_ExpansionMapMenu.CloseMapMenu();
			} 
			else if (!GetGame().GetUIManager().GetMenu() && GetExpansionSettings().GetMap() && GetExpansionSettings().GetMap().CanOpenMapWithKeyBinding)
			{
				if (GetExpansionSettings().GetMap().NeedMapItemForKeyBinding)
				{
					if (playerPB.HasItemMap() || playerPB.HasItemGPS())
						show_map = true;
				} 
				else
				{
					show_map = true;
				}
	
				if (show_map)
				{
					if (m_ExpansionMapMenu)
					{
						GetGame().GetUIManager().ShowScriptedMenu(m_ExpansionMapMenu, NULL);
					} 
					else
					{
						m_ExpansionMapMenu = ExpansionMapMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(MENU_EXPANSION_MAP, NULL));
					}
				}
			}
		}
	}
	
	// ------------------------------------------------------------
	// Expansion PlayerCheckNV
	// ------------------------------------------------------------
	void PlayerCheckNV(PlayerBase player)
	{		
		if ( !GetGame() )
			return;
		
		if ( player && player.GetCurrentCamera() )
		{
			private DayZPlayerCameraBase camera = DayZPlayerCameraBase.Cast( GetGame().GetPlayer().GetCurrentCamera() );
			if ( camera )
			{
				if ( camera && camera.IsCameraNV() ) 
				{
					if ( !m_ExpansionHud.GetNVState() )
				 		m_ExpansionHud.ShowNV( true );
				}
				else
				{
					if ( m_ExpansionHud.GetNVState() )
				 		m_ExpansionHud.ShowNV( false );
				}
			}
		}
		
		EntityAI entity;
		NVGoggles googles;
		ItemBase headgear;
		ItemBase eyewear;
		ItemBase handitem;
		
		if (player && player.FindAttachmentBySlotName("Headgear") != null)
			headgear = ItemBase.Cast(player.FindAttachmentBySlotName("Headgear"));
		
		if (player && player.FindAttachmentBySlotName("Eyewear") != null)
			eyewear = ItemBase.Cast(player.FindAttachmentBySlotName("Eyewear"));
		
		if (player && player.GetHumanInventory().GetEntityInHands() != null)
			handitem = ItemBase.Cast( player.GetHumanInventory().GetEntityInHands() );
		
		// Nvg - Headgear check
		if ( headgear )
		{
			entity = headgear.FindAttachmentBySlotName("NVG");
			if (entity)
			{
				Class.CastTo(googles, entity);
				GetNVBatteryState( googles );
			}
		}
		// Nvg - Eyewear check
		if ( eyewear )
		{
			entity = eyewear.FindAttachmentBySlotName("NVG");
			if (entity)
			{
				Class.CastTo(googles, entity);
				GetNVBatteryState( googles );
			}
		}
		// Nvg - In hands check
		if ( handitem )
		{
			entity = handitem;
			if (entity)
			{
				Class.CastTo(googles, entity);
				GetNVBatteryState( googles );
			}
		}
	}
	
	// ------------------------------------------------------------
	// Expansion PlayerCheckNV
	// ------------------------------------------------------------
	void GetNVBatteryState(NVGoggles googles)
	{
		if ( GetGame().IsClient() )
		{
			int energy_percent = 0;
			if ( googles && googles.GetCompEM().CanWork() )
			{
				energy_percent = googles.GetBatteryEnergy();					
				m_ExpansionHud.SetNVBatteryState( energy_percent );
			}
		}
	}
	
	// ------------------------------------------------------------
	// Expansion GetExpansionHud
	// ------------------------------------------------------------
	ExpansionIngameHud GetExpansionHud()
	{ 
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::GetExpansionHud");
		#endif

		return m_ExpansionHud;
	}
	
	// ------------------------------------------------------------
	// Expansion ExpansionPlayerData
	// ------------------------------------------------------------
	void ExpansionPlayerData()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::ExpansionPlayerData - Start");
		#endif
		
		Man man = GetGame().GetPlayer();
		PlayerBase player = PlayerBase.Cast(man);
		
		//! Offline Check?!
		if ( !GetPermissionsManager().GetClientPlayer() )
		{
			m_DataSent = true;
			return;
		}

		string guid = "OFFLINE";

		if ( player.GetIdentity() )
		{
			guid = player.GetIdentityUID();
		}
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::ExpansionPlayerData - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Expansion OnKeyPress
	//! Includes key-press commands for some inputs in menus
	// ------------------------------------------------------------
	override void OnKeyPress( int key )
	{
		super.OnKeyPress( key );
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnKeyPress - Start");
		#endif

		m_Hud.KeyPress( key );
		
		ExpansionLockUIBase menu;
		if ( Class.CastTo( menu, GetGame().GetUIManager().FindMenu( MENU_EXPANSION_NUMPAD_MENU ) ) )
			menu.OnKeyPress( key );
		
		menu = NULL;
		if ( Class.CastTo( menu, GetGame().GetUIManager().FindMenu( MENU_EXPANSION_CODELOCK_MENU ) ) )
			menu.OnKeyPress( key );
		
		switch (key)
		{
			case KeyCode.KC_PRIOR:
			{
				if (m_ExpansionHud.GetEarplugsState())
				{
					GetExpansionClientSettings().EarplugLevel = Math.Clamp( GetExpansionClientSettings().EarplugLevel + 0.01, 0.0, 1.0 );
					GetExpansionClientSettings().Save();
					
					m_ExpansionHud.UpdateEarplugs();
				}
					
				break;
			}
			
			case KeyCode.KC_NEXT:
			{
				if (m_ExpansionHud.GetEarplugsState())
				{
					GetExpansionClientSettings().EarplugLevel = Math.Clamp( GetExpansionClientSettings().EarplugLevel - 0.01, 0.0, 1.0 );
					GetExpansionClientSettings().Save();
					
					m_ExpansionHud.UpdateEarplugs();
				}
				
				break;
			}
		}

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::OnKeyPress - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Expansion INPUT_FORWARD
	// ------------------------------------------------------------
	bool INPUT_FORWARD()
	{
   		return GetGame().GetInput().LocalPress( "UAMoveForward", false );
	}
	
	// ------------------------------------------------------------
	// Expansion INPUT_BACK
	// ------------------------------------------------------------
	bool INPUT_BACK()
	{
   		return GetGame().GetInput().LocalPress( "UAMoveBack", false );
	}
	
	// ------------------------------------------------------------
	// Expansion INPUT_S
	// ------------------------------------------------------------
	bool INPUT_LEFT()
	{
   		return GetGame().GetInput().LocalPress( "UAMoveLeft", false );
	}
	
	// ------------------------------------------------------------
	// Expansion INPUT_RIGHT
	// ------------------------------------------------------------
	bool INPUT_RIGHT()
	{		
   		return GetGame().GetInput().LocalPress( "UAMoveRight", false );
	}
		
	// ------------------------------------------------------------
	// Expansion INPUT_GETOVER
	// ------------------------------------------------------------
	bool INPUT_GETOVER()
	{
   		return GetGame().GetInput().LocalPress( "UAGetOver", false );
	}
	
	// ------------------------------------------------------------
	// Expansion INPUT_STANCE
	// ------------------------------------------------------------
	bool INPUT_STANCE()
	{
   		return GetGame().GetInput().LocalPress( "UAStance", false );
	}
		
	// ------------------------------------------------------------
	// Expansion DecreaseGPSMapScale
	// ------------------------------------------------------------
	void DecreaseGPSMapScale()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::DecreaseGPSMapScale - Start");
		#endif
		
		if ( m_ExpansionHud )
		{
			float current_scale;
			float new_scale;
			
			current_scale = m_ExpansionHud.GetCurrentGPSMapScale();
			new_scale = ( current_scale - 0.1 );
			
			if (new_scale <= 0.1)
				return;
			
			m_ExpansionHud.SetGPSMapScale( new_scale );
		}

		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::DecreaseGPSMapScale - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Expansion IncreaseGPSMapScale
	// ------------------------------------------------------------
	void IncreaseGPSMapScale()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::IncreaseGPSMapScale - Start");
		#endif

		if ( m_ExpansionHud )
		{		
			float current_scale;
			float new_scale;
			
			current_scale = m_ExpansionHud.GetCurrentGPSMapScale();
			new_scale = ( current_scale + 0.1 );
			
			if ( new_scale >= 0.8 )
				return;
			
			m_ExpansionHud.SetGPSMapScale( new_scale );
		}
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("MissionGameplay::IncreaseGPSMapScale - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Expansion ToggleHUDGPSMode
	//! Changes GPS Mode
	// ------------------------------------------------------------
	void ToggleHUDGPSMode()
	{
		#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("MissionGameplay::ToggleHUDGPSMode - Start");
		#endif

		if ( !m_ExpansionHud.GetGPSMapState() && m_ExpansionHud.GetGPSMapStatsState() )
		{
			m_ExpansionHud.ShowGPSMap( true );
			m_ExpansionHud.ShowGPSMapStats( false );
		}
		else if ( m_ExpansionHud.GetGPSMapState() && !m_ExpansionHud.GetGPSMapStatsState() )
		{	
			m_ExpansionHud.ShowGPSMap( false );
			m_ExpansionHud.ShowGPSMapStats( true );
		}

		#ifdef EXPANSIONEXLOGPRINT
		EXLogPrint("MissionGameplay::ToggleHUDGPSMode - End");
		#endif
	}
	
	// ------------------------------------------------------------
	// Expansion GetChat
	// ------------------------------------------------------------
	Chat GetChat()
	{
		return m_Chat;
	}
	
	// ------------------------------------------------------------
	// Override UpdateVoiceLevelWidgets
	// ------------------------------------------------------------
	override void UpdateVoiceLevelWidgets(int level)
	{
		for( int n = 0; n < m_VoiceLevelsWidgets.Count(); n++ )
		{
			int voiceKey = m_VoiceLevelsWidgets.GetKey(n);
			ImageWidget voiceWidget = m_VoiceLevelsWidgets.Get(n);
			
			// stop fade timer since it will be refreshed
			WidgetFadeTimer timer = m_VoiceLevelTimers.Get(n);		
			timer.Stop();
		
			// show widgets according to the level
			if( voiceKey <= level )
			{
				voiceWidget.SetAlpha(1.0); // reset from possible previous fade out 
				voiceWidget.Show(true);
				
				if( !m_VoNActive && !GetUIManager().FindMenu(MENU_CHAT_INPUT) ) 	
					timer.FadeOut(voiceWidget, 3.0);	
			}
			else
				voiceWidget.Show(false);
		}
		
		// fade out microphone icon when switching levels without von on
		if( !m_VoNActive )
		{
		  	if( !GetUIManager().FindMenu(MENU_CHAT_INPUT) )
			{
				m_MicrophoneIcon.SetAlpha(1.0); 
				m_MicrophoneIcon.Show(true);
				
				m_MicFadeTimer.FadeOut(m_MicrophoneIcon, 3.0);
				m_SeperatorFadeTimer.FadeOut(m_VoiceLevelSeperator, 3.0);
			}
		}
		else
		{
			// stop mic icon fade timer when von is activated
			m_MicFadeTimer.Stop();
			m_SeperatorFadeTimer.Stop();
		}
	}
	
	// ------------------------------------------------------------
	// Override Pause
	// ------------------------------------------------------------
	override void Pause()
	{
		if ( GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu() )
			return;
		
		super.Pause();
	}
	
	// ------------------------------------------------------------
	// Override CloseAllMenus
	// ------------------------------------------------------------
	override void CloseAllMenus()
	{
		super.CloseAllMenus();
		
		if ( GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu() )
			GetDayZGame().GetExpansionGame().GetExpansionUIManager().CloseAll();
	}
	
	// ------------------------------------------------------------
	// Override OnPlayerListTogglePressed
	// ------------------------------------------------------------	
	void OnPlayerListTogglePressed()
	{
		if (GetExpansionSettings().GetPlayerList().EnablePlayerList)
		{
			ExpansionUIManager uiManager = GetDayZGame().GetExpansionGame().GetExpansionUIManager();	//! Reference to expansion ui manager
			ScriptView menu	= uiManager.GetMenu();											//! Reference to current opened script view menu
			
			ExpansionPlayerListMenu playerListMenu = ExpansionPlayerListMenu.Cast(menu);
			if (!playerListMenu)
			{
				uiManager.CreateSVMenu(EXPANSION_MENU_PLAYERLIST);
			} 
			else if (playerListMenu && playerListMenu.IsVisible())
			{
				uiManager.CloseMenu();
			}
		}
	}
}