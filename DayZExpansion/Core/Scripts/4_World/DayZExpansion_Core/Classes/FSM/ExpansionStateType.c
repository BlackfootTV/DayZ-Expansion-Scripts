class ExpansionStateType
{
	private static ref map<string, autoptr ExpansionStateType> m_Types = new map<string, autoptr ExpansionStateType>();

	string m_ClassName;
	ref array<string> m_Variables;

	void ExpansionStateType()
	{
		#ifdef EAI_TRACE
		auto trace = CF_Trace_0(this, "ExpansionStateType");
		#endif
		
		m_Variables = new array<string>();
	}

	static bool Contains(string name)
	{
		#ifdef EAI_TRACE
		auto trace = CF_Trace_1("ExpansionStateType", "Contains").Add(name);
		#endif
		
		return m_Types.Contains(name);
	}

	static void Add(ExpansionStateType type)
	{
		#ifdef EAI_TRACE
		auto trace = CF_Trace_1("ExpansionStateType", "Add").Add(type);
		#endif
		
		m_Types.Insert(type.m_ClassName, type);
	}

	static ExpansionStateType Get(string type)
	{
		#ifdef EAI_TRACE
		auto trace = CF_Trace_1("ExpansionStateType", "Get").Add(type);
		#endif
		
		return m_Types[type];
	}

	static ExpansionStateType LoadXML(ExpansionFSMType fsmType, CF_XML_Tag xml_root_tag, FileHandle file)
	{
		#ifdef EAI_TRACE
		auto trace = CF_Trace_2("ExpansionStateType", "LoadXML").Add(fsmName).Add(xml_root_tag);
		#endif

		string fsmName = fsmType.m_Name;

		string name = xml_root_tag.GetAttribute("name").ValueAsString();
		
		string child_fsm;
		if (xml_root_tag.GetAttribute("fsm"))
		{
			child_fsm = xml_root_tag.GetAttribute("fsm").ValueAsString();
		}

		string class_name = "Expansion_" + fsmName + "_" + name + "_State_" + ExpansionFSMType.s_ReloadNumber;
		
		//if (child_fsm != "") class_name = "Expansion_FSM_" + child_fsm + "_State";

		if (ExpansionStateType.Contains(class_name))
		{
			return ExpansionStateType.Get(class_name);
		}

		ExpansionStateType new_type = new ExpansionStateType();
		new_type.m_ClassName = class_name;

		FPrintln(file, "class " + class_name + " extends " + fsmType.m_Type + "State {");

		string child_fsm_name = "Expansion_" + child_fsm + "_FSM_" + ExpansionFSMType.s_ReloadNumber;

		if (child_fsm != "")
		{
			FPrintln(file, child_fsm_name + " sub_fsm;");
		}

		FPrintln(file, "Expansion_" + fsmName + "_FSM_" + ExpansionFSMType.s_ReloadNumber + " fsm;");

		auto variables = xml_root_tag.GetTag("variables");
		if (variables.Count() > 0)
		{
			variables = variables[0].GetTag("variable");
			foreach (auto variable : variables)
			{
				string variable_name = variable.GetAttribute("name").ValueAsString();
				string variable_type = variable.GetAttribute("type").ValueAsString();
				string variable_default = "";

				if (variable.GetAttribute("default"))
				{
					variable_default = variable.GetAttribute("default").ValueAsString();
				}

				new_type.m_Variables.Insert(variable_name);

				string variable_line = "" + variable_type + " " + variable_name;
				if (variable_default != "")
				{
					variable_line = variable_line + " = " + variable_default;
				}

				variable_line = variable_line + ";";

				FPrintln(file, variable_line);
			}
		}

		FPrintln(file, "void " + class_name + "(ExpansionFSM _fsm) {");
		FPrintln(file, "Class.CastTo(fsm, _fsm);");
		FPrintln(file, "m_ClassName = \"" + class_name + "\";");
		if (child_fsm != "")
		{
			FPrintln(file, "m_Name = \"" + child_fsm + "\";");
			FPrintln(file, "m_SubFSM = new " + child_fsm_name + "(_fsm.GetOwner(), this);");
			FPrintln(file, "Class.CastTo(sub_fsm, m_SubFSM);");
		}
		else
		{
			FPrintln(file, "m_Name = \"" + name + "\";");
		}

		FPrintln(file, "}");

		auto event_entry = xml_root_tag.GetTag("event_entry");
		auto event_exit = xml_root_tag.GetTag("event_exit");
		auto event_update = xml_root_tag.GetTag("event_update");

		if (child_fsm != "")
		{
			FPrintln(file, "override void OnEntry(string Event, ExpansionState From) {");
#ifdef EAI_TRACE
			FPrintln(file, "auto trace = CF_Trace_2(this, \"OnEntry\").Add(Event).Add(From);");
#endif
			FPrintln(file, "if (Event != \""+"\") m_SubFSM.Start(Event);");
			FPrintln(file, "else m_SubFSM.StartDefault();");
			FPrintTag0(file, event_entry);
			FPrintln(file, "}");

			FPrintln(file, "override void OnExit(string Event, bool Aborted, ExpansionState To) {");
#ifdef EAI_TRACE
			FPrintln(file, "auto trace = CF_Trace_3(this, \"OnExit\").Add(Event).Add(Aborted).Add(To);");
#endif
			FPrintln(file, "if (Aborted) m_SubFSM.Abort(Event);");
			FPrintTag0(file, event_exit);
			FPrintln(file, "}");

			FPrintln(file, "override int OnUpdate(float DeltaTime, int SimulationPrecision) {");
#ifdef EAI_TRACE
			FPrintln(file, "auto trace = CF_Trace_2(this, \"OnUpdate\").Add(DeltaTime).Add(SimulationPrecision);");
#endif
			FPrintln(file, "if (m_SubFSM.Update(DeltaTime, SimulationPrecision) == EXIT) return EXIT;");
			if (!FPrintTag0(file, event_update))
			{
				FPrintln(file, "return CONTINUE;");
			}
			FPrintln(file, "}");
		}
		else
		{
			FPrintln(file, "override void OnEntry(string Event, ExpansionState From) {");
#ifdef EAI_TRACE
			FPrintln(file, "auto trace = CF_Trace_2(this, \"OnEntry\").Add(Event).Add(From);");
#endif
			FPrintTag0(file, event_entry);
			FPrintln(file, "}");

			FPrintln(file, "override void OnExit(string Event, bool Aborted, ExpansionState To) {");
#ifdef EAI_TRACE
			FPrintln(file, "auto trace = CF_Trace_3(this, \"OnExit\").Add(Event).Add(Aborted).Add(To);");
#endif
			FPrintTag0(file, event_exit);
			FPrintln(file, "}");

			FPrintln(file, "override int OnUpdate(float DeltaTime, int SimulationPrecision) {");
#ifdef EAI_TRACE
			FPrintln(file, "auto trace = CF_Trace_2(this, \"OnUpdate\").Add(DeltaTime).Add(SimulationPrecision);");
#endif
			if (!FPrintTag0(file, event_update))
			{
				FPrintln(file, "return CONTINUE;");
			}

			FPrintln(file, "}");
		}

		FPrintln(file, "}");
		
		ExpansionStateType.Add(new_type);
		
		return new_type;
	}

	static bool FPrintTag0(FileHandle file, array<CF_XML_Tag> tags)
	{
		if (tags.Count() > 0)
		{
			string content = tags[0].GetContent().GetContent();
			if (content.Trim())
			{
				FPrintln(file, content);
				return true;
			}
		}

		return false;
	}
};
