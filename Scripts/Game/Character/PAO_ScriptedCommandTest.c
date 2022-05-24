
class PAO_ScriptedCommandTest : ScriptedCommand
{

	//! constructor
	void PAO_ScriptedCommandTest(BaseAnimPhysComponent pAnimPhysComponent)
	{
		m_AnimationComponent 	= CharacterAnimationComponent.Cast(pAnimPhysComponent);		

	}

	//!
	void StartSwimming()
	{
		//PreAnim_CallCommand(m_Table.m_CmdStartSwimming, 1, 1);
	}

	void EndSwimming()
	{
		//PreAnim_CallCommand(m_Table.m_CmdStartSwimming, 0, 0);
	}
	
	//	
	void UpdateWaterDepth()
	{
		//vector 	pp = m_pPlayer.GetPosition();
		//vector  wl = HumanCommandSwim.WaterLevelCheck(m_pPlayer, pp);

		//m_fWaterLevelDepth = wl[0];		// whats water depth at player's position
		//m_fCharacterDepth = wl[1];		// whats character depth  at player's position
	}


	//! 
	override void OnActivate()
	{
		//StartSwimming();
		//m_AnimationComponent.PhysicsEnableGravity(false);
		Print("Hello there!!!!!!!!!!");

		
	}

	override void OnDeactivate()
	{
	}

	// called to set values to animation graph processing 
	override void PreAnimUpdate(float pDt)
	{
	   

	}

	override void 	PrePhysUpdate(float pDt)
	{
		Print("SCR_CharacterCommandSwim::PrePhysUpdate");
		//PrePhys_SetTranslation(pDt);
	}

	//! called when all animation / pre phys update is handled
	override bool PostPhysUpdate(float pDt)
	{
		Print("SCR_CharacterCommandSwim::PostPhysUpdate: " );

		return true;	// handled with SetFlagFinished();
	}


	CharacterAnimationComponent			m_AnimationComponent;

};