
class PAO_ScriptedCommandTest : ScriptedCommand
{

	//! constructor
	void PAO_ScriptedCommandTest(BaseAnimPhysComponent pAnimPhysComponent)
	{
		m_AnimationComponent 	= CharacterAnimationComponent.Cast(pAnimPhysComponent);		

	}
	
	
	
	
	void PushCharacter(){
		
		PrePhys_SetTranslation("100 0 0");
	}

	//! 
	override void OnActivate()
	{

		Print("Started ScriptedCommandTest");
		PrePhysUpdate(10);
		m_AnimationComponent.PhysicsEnableGravity(true);
		//PushCharacter();
		
		


		
	}

	override void OnDeactivate()
	{
		
		Print("Finished ScriptedCommandTest");
		//m_AnimationComponent.PhysicsEnableGravity(true);

	}

	// called to set values to animation graph processing 
	override void PreAnimUpdate(float pDt)
	{
	   

	}

	override void 	PrePhysUpdate(float pDt)
	{
		Print("SCR_CharacterCommandSwim::PrePhysUpdate");
		
		vector trans = vector.Zero;
		PrePhys_GetTranslation(trans);		
	}

	//! called when all animation / pre phys update is handled
	override bool PostPhysUpdate(float pDt)
	{
		Print("SCR_CharacterCommandSwim::PostPhysUpdate: " );

		return true;	// handled with SetFlagFinished();
	}


	CharacterAnimationComponent			m_AnimationComponent;

};