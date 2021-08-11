class ExpansionVehicleSoundShader
{
	private ref Expression m_Volume;
	private ref Expression m_Frequency;

	void ExpansionVehicleSoundShader(string name)
	{
		string path;
		string expression;

		expression = "1";
		path = "CfgSoundShaders " + name + " ex_volume";
		if (GetGame().ConfigIsExisting(path))
		{
			expression = GetGame().ConfigGetTextOut(path);
		}
		
		#ifdef CF_EXPRESSION
		m_Volume = CF_ExpressionVM.Compile(expression, ExpansionVehicleSoundManager.s_SoundShaderParameters, CF_SQFExpression);
		#endif

		expression = "1";
		path = "CfgSoundShaders " + name + " ex_frequency";
		if (GetGame().ConfigIsExisting(path))
		{
			expression = GetGame().ConfigGetTextOut(path);
		}
		
		#ifdef CF_EXPRESSION
		m_Frequency = CF_ExpressionVM.Compile(expression, ExpansionVehicleSoundManager.s_SoundShaderParameters, CF_SQFExpression);
		#endif
	}

	void Calculate(array<float> variables, inout float volume, inout float frequency)
	{
		#ifdef CF_EXPRESSION
		frequency = m_Frequency.Evaluate(variables);
		
		if (frequency < 0.5 || frequency > 1.75)
		{
			volume = 0.0;
		} else
		{
			volume = m_Volume.Evaluate( variables );
		}
		#endif
	}
};

class ExpansionVehicleSoundManager
{
	static ref array<string> s_SoundShaderParameters = {
		"rpm",
		"engineOn",
		"campos",
		"doors",
		"speed",
		"thrust",
	};

	static ref map<string, ref ExpansionVehicleSoundShader> s_SoundShaders = new map<string, ref ExpansionVehicleSoundShader>();
	
	static void Get(string name, out ExpansionVehicleSoundShader shader)
	{
		if (s_SoundShaders.Find(name, shader)) return;
		
		shader = new ExpansionVehicleSoundShader(name);
		s_SoundShaders[name] = shader;
	}
};

class ExpansionVehicleSound
{
	private ExpansionVehicleBase m_Vehicle;

	private bool m_HasVolume = true;
	private bool m_HasFrequency = true;

	private ref ExpansionVehicleSoundShader m_Shader;
	
	private float m_TargetVolume;
	private float m_TargetFrequency;
	
	private float m_CurrentVolume;
	private float m_CurrentFrequency;

	private string m_SoundSetName;

	private ref SoundParams m_SoundParams;
	private ref SoundObjectBuilder m_SoundObjectBuilder;
	private ref SoundObject m_SoundObject;
	private ref AbstractWave m_SoundWaveObject;

	void ExpansionVehicleSound(ExpansionVehicleBase vehicle, string soundSetName)
	{
		m_Vehicle = vehicle;
		m_SoundSetName = soundSetName;

		array<string> soundShaders = new array<string>;
		GetGame().ConfigGetTextArray("CfgSoundSets " + soundSetName + " soundShaders", soundShaders);

		ExpansionVehicleSoundManager.Get(soundShaders[0], m_Shader);
	}

	bool Update(float pDt, array<float> variables)
	{
		if ( !m_SoundWaveObject )
		{
			m_SoundParams = new SoundParams( m_SoundSetName );
			if ( !m_SoundParams.IsValid() )
			{
				//Print( "[Error][Sound]: ExpansionVehicleSound::OnUpdate() -> m_SoundObject is null -> m_SoundSetName: " + m_SoundSetName );

				return false;
			}
			
			m_SoundObjectBuilder = new SoundObjectBuilder( m_SoundParams );
			m_SoundObjectBuilder.UpdateEnvSoundControllers( m_Vehicle.GetPosition() );
			
			m_SoundObject = m_SoundObjectBuilder.BuildSoundObject();
			
			if ( m_SoundObject )
			{
				m_SoundObject.SetPosition( m_Vehicle.GetPosition() );
				m_SoundObject.SetKind( WaveKind.WAVEATTALWAYS );
			} else
			{
				//Print( "[Error][Sound]: ExpansionVehicleSound::OnUpdate() -> m_SoundObject is null -> m_SoundSetName: " + m_SoundSetName );

				return false;
			}

			m_SoundWaveObject = GetGame().GetSoundScene().Play3D( m_SoundObject, m_SoundObjectBuilder );
			m_SoundWaveObject.Play();
			m_SoundWaveObject.Loop( true );
		}

		m_SoundWaveObject.SetPosition( m_Vehicle.GetPosition() );
		
		m_Shader.Calculate(variables, m_TargetVolume, m_TargetFrequency);

		m_SoundWaveObject.SetVolume( m_CurrentVolume );
		m_SoundWaveObject.SetFrequency( m_CurrentFrequency );
				
		#ifdef EXPANSION_DEBUG_UI_VEHICLE
		CF_Debugger_Block dbg_Vehicle = CF.Debugger.Get("Vehicle", m_Vehicle);

		dbg_Vehicle.Set("Sound Set", m_SoundSetName );
		dbg_Vehicle.Set("Target", "(volume=" + m_TargetVolume + ", frequency=" + m_TargetFrequency + ")" );
		dbg_Vehicle.Set("Current", "(volume=" + m_CurrentVolume + ", frequency=" + m_CurrentFrequency + ")" );
		#endif
		
		m_CurrentVolume += Math.Clamp( m_TargetVolume - m_CurrentVolume, -1.0 * pDt, 1.0 * pDt );
		m_CurrentFrequency += Math.Clamp( m_TargetFrequency - m_CurrentFrequency, -1.0 * pDt, 1.0 * pDt );
		
		return true;
	}
};