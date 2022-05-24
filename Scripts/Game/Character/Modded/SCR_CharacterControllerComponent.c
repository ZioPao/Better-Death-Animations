modded class SCR_CharacterControllerComponent : CharacterControllerComponent{
	
	//protected ref PAO_SCTest m_testImpulse;

	
	
	
	
	
	protected SCR_CharacterCommandHandlerComponent FindCommandHandler(IEntity pUser)
	{
		ChimeraCharacter character = GetCharacter();
		if (!character)
			return null;
		
		CharacterAnimationComponent animationComponent = character.GetCharacterController().GetAnimationComponent();
		if (!animationComponent)
			return null;
		
		return SCR_CharacterCommandHandlerComponent.Cast(animationComponent.FindComponent(SCR_CharacterCommandHandlerComponent));
	}
	
	override void OnDeath(IEntity instigator)
	{

		if (m_OnPlayerDeath != null)
			m_OnPlayerDeath.Invoke();

		if (m_OnPlayerDeathWithParam)
			m_OnPlayerDeathWithParam.Invoke(this, instigator);
		
		//GetCharacter().GetPhysics().ApplyImpulse("0 1000 0");
		//GetCharacter().GetPhysics().ApplyImpulse("0 1000 0");
		//GetCharacter().GetPhysics().ApplyImpulse("0 1000 0");
		
		// todo init them before
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		CharacterControllerComponent cc = CharacterControllerComponent.Cast(pc.GetControlledEntity().FindComponent(CharacterControllerComponent));
		CharacterAnimationComponent cc_ac = CharacterAnimationComponent.Cast(cc.FindComponent(CharacterAnimationComponent));		
		CharacterControllerComponent test_CC = CharacterControllerComponent.Cast(GetCharacter().FindComponent(CharacterControllerComponent));
		SCR_HybridPhysicsComponent test_hpc = SCR_HybridPhysicsComponent.Cast(GetCharacter().FindComponent(SCR_HybridPhysicsComponent));
		SCR_CharacterDamageManagerComponent damageComponent = SCR_CharacterDamageManagerComponent.Cast(GetCharacter().FindComponent(SCR_CharacterDamageManagerComponent));
		
		
		
		
		
		CharacterInputContext cic = test_CC.GetInputContext();
		
		//Probably slow as fuck, I don't care right now
		
		if (pc.GetControlledEntity().FindComponent(CharacterControllerComponent) != test_CC){
			//having this seems to force the commandid to shut off before we can actually use it

			//
			
			
			// Get original ragdoll and destroy it
			PhysicsRagdoll.GetRagdoll(GetCharacter()).Destroy(1);
			
			// Recreate it
			PhysicsRagdoll.CreateRagdoll(GetCharacter(), "{5DD1A0EBAD2116CB}Prefabs/Characters/Core/character_modded.ragdoll",1, EPhysicsLayerDefs.Ragdoll);
			PhysicsRagdoll current_ragdoll = PhysicsRagdoll.GetRagdoll(GetCharacter());
			Physics test_phys = current_ragdoll.GetBoneRigidBody(0);
			
			test_phys.EnableGravity(true);
			test_phys.SetMass(1);		//just to be sure, 5 feels strange
			test_phys.SetDamping(0.001,0.0001);
			//test_phys.SetSleepingTreshold(0.000000001, 0.000000001);		//default 1 
			test_phys.SetSleepingTreshold(1,1);		//default 1, doesn't seem to work?

			
			
			// Get Last Hit
			array<vector> lastHitArray = damageComponent.GetLastHit();
			
			
			
			Print(test_phys.GetVelocity());
			
			float x = Math.RandomFloatInclusive(-1.5, 0.0);
			float y = Math.RandomFloatInclusive(-1.5, 0.0);
			float z = Math.RandomFloatInclusive(-1.5, 0.0);
			
			
			vector hitVector = {x,y,z};
			//vector hitVector = {Math.RandomFloatInclusive(-1.0, 0.0), -1.0, Math.RandomFloatIncluse(-1.0, 0.0)};
			
			
			
			
			
			test_phys.SetVelocity(hitVector);
			Print(test_phys.GetVelocity());
			Print("_____________________________");
			
			//test_phys.ApplyImpulse("0 -10 0");
			/// we need to calculate the direction of the hit before doing anything else
			//test_phys.ApplyForceAt(lastHitArray[0], lastHitArray[1]);		//really dumb test
			current_ragdoll.Enable();
			
			
			
			
			test_CC.Ragdoll();
			

			
			
		}

		if (pc && m_CameraHandler && m_CameraHandler.IsInThirdPerson())
			pc.m_bRetain3PV = true;
		
		// Insert the character and see if it held a weapon, if so, try adding that as well
		
		
		GarbageManager garbageManager = GetGame().GetGarbageManager();
		if (garbageManager)
		{
			garbageManager.Insert(GetCharacter());
			
			if (!m_WeaponManager)
				return;
			
			BaseWeaponComponent weaponOrSlot = m_WeaponManager.GetCurrentWeapon();
			if (!weaponOrSlot)
				return;
			
			IEntity weaponEntity;
			WeaponSlotComponent slot = WeaponSlotComponent.Cast(weaponOrSlot);
			if (slot)
				weaponEntity = slot.GetWeaponEntity();
			else
				weaponEntity = weaponOrSlot.GetOwner();
			
			if (!weaponEntity)
				return;
			
			garbageManager.Insert(weaponEntity);
		}
		
	}
		
		
	
}
