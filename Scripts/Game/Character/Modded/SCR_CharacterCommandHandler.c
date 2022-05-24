modded class SCR_CharacterCommandHandlerComponent : CharacterCommandHandlerComponent{
	
	protected ref PAO_ScriptedCommandTest m_ScriptedCommandTest;
		
	
	override protected void OnInit(IEntity owner){
	
		super.OnInit(owner);
		m_CharacterControllerComp = CharacterControllerComponent.Cast(m_OwnerEntity.FindComponent(CharacterControllerComponent));
		
		m_ScriptedCommandTest = new PAO_ScriptedCommandTest(m_CharacterAnimComp);
		
		
	}
	

	
	
	override bool HandleDeath(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		if (pCurrentCommandID == ECharacterCommandIDs.DEATH){
			Print("Someone died");
			float die = pInputCtx.GetDie();
			if ( die != 0.0 )
			{
				
				
				Print("someone died and we're going in");
				//StartCommand_Fall(die);		//this doesn't seem needed, no idea where it's starting from then... Maybe default?
				
				SetSimulationDisabled(false);


				
				m_CharacterAnimComp.SetCurrentCommand(m_ScriptedCommandTest); // it doesn't wanna work goddamn it
	
				return true;
			}

		}
		
		return false;
		
	}
	
	
}
		
	

	

