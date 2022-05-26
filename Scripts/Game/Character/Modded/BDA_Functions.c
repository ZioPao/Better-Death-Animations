class BDA_Functions
{
	
	
	/* GENERIC FUNCTIONS */
		
	static float CalculateDeltaTime(bool firstUpdate)
	{
		// Calculate actual delta time
		BaseWorld world = GetGame().GetWorld();
		//if (!world)
		//	return;
		
		float time = world.GetWorldTime();
		if (firstUpdate)
			SplashScreenSequence.m_fWorldTime = time;
		
		float delta = (time - SplashScreenSequence.m_fWorldTime) / 1000;
		return delta;
		
	
	}
	
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
		
		
		
			//PhysicsRagdoll.CreateRagdoll(GetCharacter(), "{BDAFE20F95BD19F0}Prefabs/Characters/Core/character_modded_encircle.ragdoll", 1, EPhysicsLayerDefs.Ragdoll);
			PhysicsRagdoll ragdoll = PhysicsRagdoll.GetRagdoll(character);
			
		
			ragdoll.GetBoneRigidBody(0).SetResponseIndex(0);
			ragdoll.GetBoneRigidBody(0).SetMass(2);		//2
			
			ragdoll.GetBoneRigidBody(0).EnableGravity(true);
			//ragdoll.GetBoneRigidBody(0).SetMass(5);		// SET THIS HIGHER!!!!
			ragdoll.GetBoneRigidBody(0).SetDamping(0.000000001 ,0.000000001);
			//test_phys.SetSleepingTreshold(0.000000001, 0.000000001);		//default 1 
			ragdoll.GetBoneRigidBody(0).SetSleepingTreshold(1,1);		//default 1, doesn't seem to work?
			ragdoll.Enable();
		
			return ragdoll;
	}
	
	
	



}