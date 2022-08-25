/**
 * ExpansionChatLine.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionChatMessage
{
	int Channel;
	string From;
	string Text;
}

class ExpansionChatLineBase: ExpansionScriptView
{
	private ref ExpansionChatLineController m_ChatLineController;
	private ref Chat m_Chat;
	
	private TextWidget SenderName;
	private TextWidget Message;
	private ref WidgetFadeTimer m_FadeInTimer;
	private Widget m_Parent;
	private string m_LayoutPath;
	
	void ExpansionChatLineBase(Widget parent, Chat chat)
	{
		auto trace = EXTrace.Start(ExpansionTracing.CHAT);
		
		m_ChatLineController = ExpansionChatLineController.Cast(GetController());
		m_Parent = parent;
		m_Chat = chat;
		
		m_Parent.AddChild(GetLayoutRoot());

		GetLayoutRoot().Show(false);
	}
	
	void ~ExpansionChatLineBase()
	{
		auto trace = EXTrace.Start(ExpansionTracing.CHAT);

		if (m_FadeInTimer)
			m_FadeInTimer.Stop();
	}
	
	void Set(ExpansionChatMessage message)	// Param 1 --> Channel, Param 2 --> sender name, Param 3 --> message, Param 4 ?? 
	{
#ifdef EXPANSIONTRACE
		auto trace = EXTrace.Start(ExpansionTracing.CHAT);
#endif

		MissionGameplay mission;
		if (!Class.CastTo(mission, GetGame().GetMission()))
			return;
		
		if (!message)
		{
			GetLayoutRoot().Show(false);
			m_ChatLineController.SenderName = "";
			m_ChatLineController.NotifyPropertyChanged("SenderName");
			m_ChatLineController.Message = "";
			m_ChatLineController.NotifyPropertyChanged("Message");
			return;
		}
		
		GetLayoutRoot().Show(true);
		
		switch (message.Channel)
		{
		case CCSystem:
			SenderSetColour(GetExpansionSettings().GetChat().ChatColors.Get("SystemChatColor"));
			SetTextColor(GetExpansionSettings().GetChat().ChatColors.Get("SystemChatColor"));
			m_ChatLineController.SenderName = " " + "Game" + ": ";
			break;
		case CCAdmin:
		case CCBattlEye:
			SenderSetColour(GetExpansionSettings().GetChat().ChatColors.Get("AdminChatColor"));	
			SetTextColor(GetExpansionSettings().GetChat().ChatColors.Get("AdminChatColor"));
			
			if ( message.From )
			{
				m_ChatLineController.SenderName = " " + message.From + ": ";
			} else
			{ 
				m_ChatLineController.SenderName = " " + "Admin" + ": ";
			}
			break;
		case CCTransmitter:
			SenderSetColour(GetExpansionSettings().GetChat().ChatColors.Get("TransmitterChatColor"));	
			SetTextColor(GetExpansionSettings().GetChat().ChatColors.Get("TransmitterChatColor"));
			m_ChatLineController.SenderName = " " + "PAS" + ": ";
			break;
		case ExpansionChatChannels.CCTransport:
			SenderSetColour(GetExpansionSettings().GetChat().ChatColors.Get("TransportChatColor"));	
			SetTextColor(GetExpansionSettings().GetChat().ChatColors.Get("TransportChatColor"));
			
			if ( message.From )
			{
				m_ChatLineController.SenderName = " " + message.From + ": ";
			} else
			{ 
				m_ChatLineController.SenderName = " ";
			}
			break;
		case ExpansionChatChannels.CCGlobal:
			SenderSetColour(GetExpansionSettings().GetChat().ChatColors.Get("GlobalChatColor"));	
			SetTextColor(GetExpansionSettings().GetChat().ChatColors.Get("GlobalChatColor"));
			
			if ( message.From )
			{
				m_ChatLineController.SenderName = " " + message.From + ": ";
			} else
			{ 
				m_ChatLineController.SenderName = " ";
			}
			break;
#ifdef EXPANSIONMODGROUPS
		case ExpansionChatChannels.CCTeam:
			SenderSetColour(GetExpansionSettings().GetChat().ChatColors.Get("PartyChatColor"));	
			SetTextColor(GetExpansionSettings().GetChat().ChatColors.Get("PartyChatColor"));
			
			if (message.From)
			{
				m_ChatLineController.SenderName = " " + message.From + ": ";
			} else
			{ 
				m_ChatLineController.SenderName = " ";
			}
			break;
#endif
		default:
			SenderSetColour(GetExpansionSettings().GetChat().ChatColors.Get("DirectChatColor"));	
			SetTextColor(GetExpansionSettings().GetChat().ChatColors.Get("DirectChatColor"));

			if (message.From)
			{
				m_ChatLineController.SenderName = " " + message.From + ": ";
			} else
			{ 
				m_ChatLineController.SenderName = " ";
			}
			break;
		}

		//! maxLineCharacters is the amount of charaters that can fit into one line before a line 
		//! break need to happend with the current chat font size.
		//! Note: Hardcoded values below assume 2560x1440 resolution, so need to adjust for other resolutions!
		int maxLineCharacters;
		ExpansionClientUIChatSize chatsize = GetExpansionClientSettings().HUDChatSize;
		switch (chatsize)
		{
			case ExpansionClientUIChatSize.VERYSMALL:
				maxLineCharacters = 56;
				break;
			case ExpansionClientUIChatSize.SMALL:
				maxLineCharacters = 50;
				break;
			case ExpansionClientUIChatSize.MEDIUM:
				maxLineCharacters = 42;
				break;
			case ExpansionClientUIChatSize.LARGE:
				maxLineCharacters = 30;
				break;
			case ExpansionClientUIChatSize.VERYLARGE:
				maxLineCharacters = 24;
				break;
		}
		
		int senderCharacters = m_ChatLineController.SenderName.Length();
		int maxWordCharacters = maxLineCharacters - senderCharacters;

		//! adjust for actual screen res
		int w, h;
		GetScreenSize(w, h);
		float adjust = maxWordCharacters * w / 2560.0;
		maxWordCharacters = adjust;

		//! split words and limit to max characters per word
		string messageText;
		if (maxWordCharacters > 0)
		{
			TStringArray words();
			message.Text.Split(" ", words);
			foreach (string word: words)
			{
				while (word.Length() > maxWordCharacters)
				{
					messageText += word.Substring(0, maxWordCharacters) + " ";
					word = word.Substring(maxWordCharacters, word.Length() - maxWordCharacters);
				}
				messageText += word + " ";
			}
		}
		else
		{
			messageText = message.Text;
		}
		
		m_ChatLineController.Message = messageText;
		m_ChatLineController.NotifyPropertiesChanged({"SenderName", "Message"});

		if (!IsVisible())
		{
			FadeInChatLine();
		}

		//! Adjust message size so it actually fits and doesn't get cut off
		float root_w, root_h;
		GetLayoutRoot().GetScreenSize(root_w, root_h);
		float sender_w, sender_h;
		SenderName.GetScreenSize(sender_w, sender_h);
		Message.SetSize(1.0 - sender_w / root_w, 1.0);
	}
	
	private void FadeInChatLine()
	{
		auto trace = EXTrace.Start(ExpansionTracing.CHAT);

		m_Chat.OnChatInputShow();
		
		if (m_FadeInTimer)
			m_FadeInTimer.Stop();	

		m_FadeInTimer = new WidgetFadeTimer;
		m_FadeInTimer.FadeIn(GetLayoutRoot(), 1.5);
	}
	
	void Clear()
	{
		auto trace = EXTrace.Start(ExpansionTracing.CHAT);

		if (m_FadeInTimer)
			m_FadeInTimer.Stop();
	}
	
	private void SetTextColor(int colour)
	{
		Message.SetColor(colour);
	}
	
	private void SenderSetColour(int colour)
	{
		SenderName.SetColor(colour);
	}
	
	override string GetLayoutFile() 
	{
		string path;
		ExpansionClientUIChatSize chatsize = GetExpansionClientSettings().HUDChatSize;
		switch (chatsize)
		{
			case ExpansionClientUIChatSize.VERYSMALL:
				path = "DayZExpansion/Chat/GUI/layouts/expansion_chat_entry_verysmall.layout";
				break;
			case ExpansionClientUIChatSize.SMALL:
				path = "DayZExpansion/Chat/GUI/layouts/expansion_chat_entry_small.layout";
				break;
			case ExpansionClientUIChatSize.MEDIUM:
				path = "DayZExpansion/Chat/GUI/layouts/expansion_chat_entry_medium.layout";
				break;
			case ExpansionClientUIChatSize.LARGE:
				path = "DayZExpansion/Chat/GUI/layouts/expansion_chat_entry_large.layout";
				break;
			case ExpansionClientUIChatSize.VERYLARGE:
				path = "DayZExpansion/Chat/GUI/layouts/expansion_chat_entry_verylarge.layout";
				break;
			default:
				path = "DayZExpansion/Chat/GUI/layouts/expansion_chat_entry_small.layout";
				break;
		}
		
		return path;
	}
	
	override typename GetControllerType() 
	{
		return ExpansionChatLineController;
	}
	
	Widget GetParentWidget()
	{
		return m_Parent;
	}
};

class ExpansionChatLineController: ExpansionViewController
{
	string SenderName;
	string Message;
};