/**
 * ExpansionDialogContent_TextScroller.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionDialogContent_Text: ExpansionDialogContentBase
{
	ref ExpansionDialogContent_TextController m_TextController;	
	private string m_Text;
	
	RichTextWidget dialog_text;
	
	void ExpansionDialogContent_Text(ExpansionDialogBase dialog)
	{
		m_Dialog = dialog;

		if (!m_TextController)
			m_TextController = ExpansionDialogContent_TextController.Cast(GetController());
		
		SetContent();
	}
	
	override string GetLayoutFile() 
	{
		return "DayZExpansion/Core/GUI/layouts/mvc/dialogs/expansion_dialog_text.layout";
	}
	
	override typename GetControllerType() 
	{
		return ExpansionDialogContent_TextController;
	}
	
	void SetText(string text)
	{
		m_Text = text;
	}
	
	void SetContent()
	{	
		m_TextController.Text = m_Text;
		m_TextController.NotifyPropertyChanged("Text");
	}
	
	override void OnShow()
	{
		super.OnShow();
		
		SetContent();
	}
	
	void SetTextColor(int color)
	{
		dialog_text.SetColor(color);
	}
};

class ExpansionDialogContent_TextController: ExpansionViewController
{
	string Text;
}

class ExpansionMenuDialogContent_Text: ExpansionMenuDialogContentBase
{
	ref ExpansionMenuDialogContent_TextController m_TextController;	
	private string m_Text = "";
	
	RichTextWidget dialog_text;
	
	void ExpansionMenuDialogContent_Text(ExpansionMenuDialogBase dialog)
	{
		m_Dialog = dialog;

		if (!m_TextController)
			m_TextController = ExpansionMenuDialogContent_TextController.Cast(GetController());
	}
	
	override string GetLayoutFile() 
	{
		return "DayZExpansion/Core/GUI/layouts/mvc/dialogs/expansion_menu_dialog_text.layout";
	}
	
	override typename GetControllerType() 
	{
		return ExpansionMenuDialogContent_TextController;
	}
	
	string GetText()
	{
		return m_Text;
	}
	
	void SetText(string text)
	{
		m_Text = text;
	}
	
	void SetContent()
	{	
		m_TextController.Text = GetText();
		m_TextController.NotifyPropertyChanged("Text");
	}
	
	override void OnShow()
	{
		super.OnShow();
		
		SetContent();
	}
	
	void SetTextColor(int color)
	{
		dialog_text.SetColor(color);
	}
};

class ExpansionMenuDialogContent_TextController: ExpansionViewController
{
	string Text;
}