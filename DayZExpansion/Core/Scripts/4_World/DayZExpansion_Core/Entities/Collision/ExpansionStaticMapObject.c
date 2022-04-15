class ExpansionStaticMapObject : Static
{
	float m_ImpulseRequired;

	void ExpansionStaticMapObject()
	{
		m_ImpulseRequired = 100.0;
	}

	void Create(EntityAI other, vector position, Contact data)
	{
		if (data.Impulse < m_ImpulseRequired)
			return;

		ItemBase.ExpansionPhaseObject(other);

		if (GetGame().IsClient())
			return;

		Object object = GetGame().CreateObjectEx(ExpansionGetPhysicsType(), GetPosition(), ECE_IN_INVENTORY);

		ExpansionPhysicsStructure structure;
		if (Class.CastTo(structure, object))
		{
			structure.Create(this, other, position, data);
		}
	}

	string ExpansionGetPhysicsType()
	{
		return "ExpansionPhysics_" + GetType();
	}
};
