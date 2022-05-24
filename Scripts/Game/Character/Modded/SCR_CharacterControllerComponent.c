//#define DEBUG_PAO

modded class SCR_CharacterControllerComponent : CharacterControllerComponent{
	
	//protected ref PAO_SCTest m_testImpulse;

	PhysicsRagdoll currentRagdoll;
	
	
	
	
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
			
			// Recreate in
			currentRagdoll = RegenPhysicsRagdoll();


			
			// Get Last Hit
			array<vector> lastHitArray = damageComponent.GetLastHit();
			vector lastHitDirection = {lastHitArray[1][0], lastHitArray[1][1], lastHitArray[1][2]};		// for some reason I can't assign a vec to a vec so whatever
			vector hitVector = {lastHitDirection[0], lastHitDirection[1], lastHitDirection[2]};		// y stays the same since we want a little more oomph
			
			
			////if it's a headshot, then no rolling around 
			string hitZoneName = damageComponent.GetHitZoneName();
			
			
			currentRagdoll.GetBoneRigidBody(0).SetVelocity(hitVector*2);		//impact or velocity?
			test_CC.Ragdoll();
			
			
			
			if (hitZoneName == "Head"){
				//no rolling, only decrease speed and stop it.
				GetGame().GetCallqueue().CallLater(FastRagdollDeath, 500, true); // in milliseconds

			}
			else{
				GetGame().GetCallqueue().CallLater(PushRagdollAround, 200, true); // in milliseconds

			}
			
			#ifdef DEBUG_PAO
			Print("Velocity del body");
			Print(currentRagdoll.GetBoneRigidBody(0).GetVelocity());
			Print("last hit array");
			
			Print(lastHitArray[0]);
			Print(lastHitArray[1]);
			Print(lastHitArray[2]);
			Print("+____________________________");			
			#endif

			
			
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
		
	float counter = 1;
	 
	void PushRagdollAround(){
		
		
		//todo make a time limit 

		if(currentRagdoll.GetNumBones() > 0){
			
			currentRagdoll.GetBoneRigidBody(0).SetMass(25);		// Bigger boy in realtime?

			//vector hitVector = {0.0 ,-0.8 , 0.0};
			//currentRagdoll.GetBoneRigidBody(0).ApplyImpulse(hitVector);
			
			float x = Math.RandomFloatInclusive(-1/counter, 1/counter);
			float y = Math.RandomFloatInclusive(-1/counter, 1/counter);
			float z = Math.RandomFloatInclusive(-1/counter, 1/counter);
			

			//vector feetPos = {-0.11, 0.26, 0.29};
			vector hitVector = {x, y , z};		//z makes them spin 

			
			//IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(Resource.Load("{37578B1666981FCE}Prefabs/Characters/Core/Character_Base.et"));
			//vector worldCoord = SCR_BaseContainerTools.GetWorldCoords(entitySource, feetPos);
			currentRagdoll.GetBoneRigidBody(0).SetVelocity(hitVector);
			
			
			#ifdef DEBUG_PAODEBUG_PAO
			Print("Applied force: " + hitVector[0] + ", " + hitVector[1] + ", " + hitVector[2]);
			Print("Counter at " + counter);
			#endif
			counter = counter + 0.05;
		}
		else{
			

			//currentRagdoll = RegenPhysicsRagdoll();
			//Print("Regenareted ragdoll");

			GetGame().GetCallqueue().Remove(PushRagdollAround);
			counter = 0;		//not sure if it's needed but whatev
			return;
		}
		
	
	}
	
	//Used in case of headshots
	void FastRagdollDeath(){
				
		//todo make a time limit 

		if(currentRagdoll.GetNumBones() > 0){
			
			currentRagdoll.GetBoneRigidBody(0).SetMass(25);		// Bigger boy in realtime?
			
			vector currentVelocity = currentRagdoll.GetBoneRigidBody(0).GetVelocity();
			
			Print(Math.AbsFloat(currentVelocity[0]));
			if (Math.AbsFloat(currentVelocity[0]) < 0.025){

				currentRagdoll.Destroy(false);
				GetGame().GetCallqueue().Remove(FastRagdollDeath);
				return;
				}
			else{
				currentRagdoll.GetBoneRigidBody(0).SetVelocity(currentVelocity/4);

			}
			

		}
		else{

			GetGame().GetCallqueue().Remove(FastRagdollDeath);
			return;
		}
	}
	
	
	protected PhysicsRagdoll RegenPhysicsRagdoll(){
			// Get original ragdoll and destroy it
			PhysicsRagdoll.GetRagdoll(GetCharacter()).Destroy(1);
			
			// Recreate it
			PhysicsRagdoll.CreateRagdoll(GetCharacter(), "{5DD1A0EBAD2116CB}Prefabs/Characters/Core/character_modded.ragdoll", 1, EPhysicsLayerDefs.Ragdoll);
			PhysicsRagdoll ragdoll = PhysicsRagdoll.GetRagdoll(GetCharacter());
			
			ragdoll.GetBoneRigidBody(0).EnableGravity(true);
			ragdoll.GetBoneRigidBody(0).SetMass(5);		// SET THIS HIGHER!!!!
			ragdoll.GetBoneRigidBody(0).SetDamping(0.000000001 ,0.000000001);
			//test_phys.SetSleepingTreshold(0.000000001, 0.000000001);		//default 1 
			ragdoll.GetBoneRigidBody(0).SetSleepingTreshold(1,1);		//default 1, doesn't seem to work?
			ragdoll.Enable();
		
			return ragdoll;
	}
}

