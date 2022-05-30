class BDR_Struct : SCR_JsonApiStruct
{
	
	string msg = "You don't know the power of the Dark side!";
	
	void BDR_Struct()
	{
		StoreFloat("testValueToSave", 0.001);
	}

	void Reset()
	{
		msg = "garbage";
	}

	void OnLoad()
	{
		Print("SavePoint::OnLoad()");
		Print(msg);
	}
	
	
	
	
}
