class BDA_Functions_Generic
{
	
	
	/* GENERIC FUNCTIONS */
	static SCR_CharacterCommandHandlerComponent FindCommandHandler(ChimeraCharacter character)
	{
		if (!character)
			return null;
		
		CharacterAnimationComponent animationComponent = character.GetCharacterController().GetAnimationComponent();
		if (!animationComponent)
			return null;
		
		return SCR_CharacterCommandHandlerComponent.Cast(animationComponent.FindComponent(SCR_CharacterCommandHandlerComponent));
		
		
	}
	
	//__________________________________________________________________ 
	
	/* Specific functions dedicated to ragdolls*/
	static PhysicsRagdoll RegenPhysicsRagdoll(ChimeraCharacter character)
	{
		// Get original ragdoll and destroy it
		PhysicsRagdoll.GetRagdoll(character).Destroy(1);
			
		// Recreate it
			
		int tempPhysicsLayerDefsProjectile = 3;		//this is a bug currently, EPhysicsLayerDefs.Projectile doesn't correspond to a real enum, so the layer mask won't work.
		PhysicsRagdoll.CreateRagdoll(character, "{5DD1A0EBAD2116CB}Prefabs/Characters/Core/character_modded.ragdoll", 1, 3 | EPhysicsLayerDefs.Weapon |EPhysicsLayerDefs.Ragdoll |  EPhysicsLayerDefs.Character | EPhysicsLayerDefs.Vehicle);
		//PhysicsRagdoll.CreateRagdoll(character, "{E80E7FF9A6480F9C}Prefabs/Characters/Core/character_modded_encircle_2.ragdoll", 1, 3 | EPhysicsLayerDefs.Weapon |EPhysicsLayerDefs.Ragdoll |  EPhysicsLayerDefs.Character | EPhysicsLayerDefs.Vehicle);
		//PhysicsRagdoll.CreateRagdoll(GetCharacter(), "{BDAFE20F95BD19F0}Prefabs/Characters/Core/character_modded_encircle.ragdoll", 1, EPhysicsLayerDefs.Ragdoll);
		
		PhysicsRagdoll ragdoll = PhysicsRagdoll.GetRagdoll(character);
			

		ragdoll.GetBoneRigidBody(0).SetResponseIndex(0);
		ragdoll.GetBoneRigidBody(0).SetMass(2);		//2
		ragdoll.GetBoneRigidBody(0).EnableGravity(true);
		ragdoll.GetBoneRigidBody(0).SetDamping(0.000000001 ,0.000000001);		//doesn't directly affect mass, so we don't have a real way to get this value programatically. bis pls fix
		ragdoll.GetBoneRigidBody(0).SetSleepingTreshold(1,1);		//default 1, doesn't seem to work?
			


		ragdoll.Enable();
		
			return ragdoll;
	}
	
	
	
	static float Lerp(float min, float max, float duration, float deltaTime)
	{
		//return bool and use out maybe? 
		if (deltaTime < duration)
			return Math.Lerp(min, max, deltaTime/duration);
		else 
			return max;

	}
	
	static void IncreaseAndThenDecrease(float middle, float end, float step, out float currentValue, out bool hasReachedMiddle)
	{
		if (currentValue > middle || hasReachedMiddle)
			{
				hasReachedMiddle = true;
			
				if (currentValue < end)
					currentValue = end;		//don't change it. 
				else
					currentValue -= step;
			}
			else
				currentValue += step;
			
	}
	
	
	
	
}







class BDA_Timer
{
		
	private float startTime;
	private float currentTime;
	
	
	void Start()
	{
		startTime = GetGame().GetWorld().GetWorldTime();
		
	}
	
	float UpdateDeltaTime()
	{
		currentTime = GetGame().GetWorld().GetWorldTime();
		
		float deltaTime = (currentTime - startTime) * 0.001;
		return deltaTime;
		

	}
	
	float GetStartTime()
	{
		return startTime;
	}
	

}


enum CharacterBones{
	HIPS,		//0
	SPINE,		//1
	HEAD,		//2
	LARM,		//3
	LFOREARM,	//4
	RARM,		//5
	RFOREARM,	//6
	LTHIGH,		//7
	RTHIGH,		//8
	LCALF,		//9
	RCALF,		//10
	RFOOT,		//11
	LFOOT		//12
}




