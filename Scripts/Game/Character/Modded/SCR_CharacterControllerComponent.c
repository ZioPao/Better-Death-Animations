//#define DEBUG_PAO

modded class SCR_CharacterControllerComponent : CharacterControllerComponent{
	
	//protected ref PAO_SCTest m_testImpulse;
	const string TAG_HITZONE_HEAD = "Head";
	const string TAG_HITZONE_LCALF = "LCalf";
	const string TAG_HITZONE_RCALF = "RCalf";
	const string TAG_HITZONE_LTHIGH = "LThigh";
	const string TAG_HITZONE_RTHIGH = "RThigh";
	const string TAG_HITZONE_NECK = "Neck";
	const string TAG_HITZONE_CHEST = "Chest";	
	const string TAG_HITZONE_LARM = "LArm";	
	const string TAG_HITZONE_RARM = "RArm";
			
			
	PhysicsRagdoll currentRagdoll;
	GenericEntity waypointTestPao;
	CharacterControllerComponent m_characterControllerComponent;
	SCR_CharacterDamageManagerComponent m_characterDamageManagerComponent;
	float counter = 1;
	float deltaTime;
	
	override void OnInit(IEntity owner)
	{
		ChimeraCharacter character = GetCharacter();
		if (!character)
			return;
		
		if (!m_WeaponManager)
			m_WeaponManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));

		if (!m_MeleeComponent)
			m_MeleeComponent = SCR_MeleeComponent.Cast(character.FindComponent(SCR_MeleeComponent));
		if (!m_CameraHandler)
			m_CameraHandler = SCR_CharacterCameraHandlerComponent.Cast(character.FindComponent(SCR_CharacterCameraHandlerComponent));
		if (!m_characterControllerComponent)
			m_characterControllerComponent = CharacterControllerComponent.Cast(character.FindComponent(CharacterControllerComponent));
		if (!m_characterDamageManagerComponent)
			m_characterDamageManagerComponent = SCR_CharacterDamageManagerComponent.Cast(character.FindComponent(SCR_CharacterDamageManagerComponent));

		#ifdef ENABLE_DIAG
		if (!m_AnimComponent)
			m_AnimComponent = CharacterAnimationComponent.Cast(character.FindComponent(CharacterAnimationComponent));
		#endif	
	}
	

	override void OnDeath(IEntity instigator)
	{

		if (m_OnPlayerDeath != null)
			m_OnPlayerDeath.Invoke();

		if (m_OnPlayerDeathWithParam)
			m_OnPlayerDeathWithParam.Invoke(this, instigator);

		//Get the delta time for everything after this 
		deltaTime = CalculateDeltaTime(true);

		
		// Get the player stuff. We'll do it here 'cause we can't rely on OnInit since it could have changed. 
		SCR_PlayerController m_playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		CharacterControllerComponent m_playerCharacterControllerComponent = CharacterControllerComponent.Cast(m_playerController.GetControlledEntity().FindComponent(CharacterControllerComponent));

		
		if (m_playerCharacterControllerComponent != m_characterControllerComponent){
			// Get original ragdoll and destroy it
			PhysicsRagdoll.GetRagdoll(GetCharacter()).Destroy(1);
			
			// Recreate in
			currentRagdoll = RegenPhysicsRagdoll();


			//Get currentCharVelocity to interpolate with hitPosition later
			//vector currentCharVelocity = m_playerCharacterControllerComponent.GetMovementVelocity();

			
			// Get Last Hit
			array<vector> lastHitArray = m_characterDamageManagerComponent.GetLastHitCoordinates();
			vector lastHitDirection = {lastHitArray[1][0], lastHitArray[1][1], lastHitArray[1][2]};		// for some reason I can't assign a vec to a vec so whatever
			vector hitVector = {lastHitDirection[0], lastHitDirection[1]/5, lastHitDirection[2]};		// y stays the same since we want a little more oomph
			vector hitPosition = {lastHitArray[0][0], lastHitArray[0][1], lastHitArray[0][2]};
			
			//if it's a headshot, then no rolling around 
			HitZone hitZone = m_characterDamageManagerComponent.GetLastHitZone();
			int hitZoneColliderID = m_characterDamageManagerComponent.GetLastColliderID();
			string hitZoneName = hitZone.GetName();
			
			vector hitZoneColliders[4];
			hitZone.TryGetColliderDescription(GetCharacter(), hitZoneColliderID, hitZoneColliders, null, null);
			CharacterIdentityComponent identity = CharacterIdentityComponent.Cast(GetCharacter().FindComponent(CharacterIdentityComponent));
			identity.SetCovered(hitZoneName, false);
			
			//we need to figure out what vector we actually need.... now 
			//Print(GetCharacter().GetTransform(hitZoneColliders));
			//hitZone.GetTransform();
			
			
			Print(hitZoneColliders[1]);
			Print(GetCharacter().VectorToParent(hitZoneColliders[1]));
			Print("_____________________");
			
			//we should interpolate between the hitVector and the vector of the velocity of the player  
			
			vector finalHit;
			

			
			// INTERPOLATION IS A MESSY STEP SO FOR NOW LET'S DISABLE IT OOPS 
			//Print(currentCharVelocity);
			/*
			if (currentCharVelocity.Length() > 0.2){
				finalHit = vector.Lerp(currentCharVelocity, hitVector/10, 0.5);
			
			}
			else{
			
				finalHit = hitVector/10;

			
			}
			*/
			
			
			// "Neck"
			// LThigh  
			// RThigh
			// RForearm
			// LCalf 
			// Health 
			// Chest
			// Head
			// Arm
			// Hips
			
			
			if (hitZoneName == TAG_HITZONE_HEAD || hitZoneName == TAG_HITZONE_NECK || hitZoneName == TAG_HITZONE_LCALF || hitZoneName == TAG_HITZONE_RCALF || hitZoneName == TAG_HITZONE_LTHIGH || hitZoneName == TAG_HITZONE_RTHIGH)
			{


				Print(hitZoneName);
				Print("Got hit somewhere else");
				finalHit = hitVector/15;


				
			}else{
			
				Print(hitZoneName);
				Print("Generic Hit");
				finalHit = hitVector/10;
			
			}
			
			m_characterControllerComponent.Ragdoll();
			
			
			
			
			
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++){

			
				currentRagdoll.GetBoneRigidBody(i).ApplyForce("0 -2.81 0");		//dunno
			}
			
			
			
			
			
			currentRagdoll.GetBoneRigidBody(0).ApplyImpulseAt(hitPosition, finalHit);		//impact or velocity?

			// Special case for headshots, basically "instakill"
			if (hitZoneName == TAG_HITZONE_HEAD){
				GetGame().GetCallqueue().CallLater(WaitSecondaryScriptFastRagdollDeath, 800, false);

			}
			else{
				GetGame().GetCallqueue().CallLater(WaitSecondaryScriptPushRagdollAround, 500, false);		//wont repeat, wait 1 sec and then start the blbob

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
		else
		{
			SCR_CharacterCommandHandlerComponent tempHandler = FindCommandHandler(GetCharacter());
			//We need the CharacterInputContext for the player
			CharacterInputContext m_characterInputContext = m_playerCharacterControllerComponent.GetInputContext();
			float dyingDirection = m_characterInputContext.GetDie();
			
			if (dyingDirection != 0.0)
				tempHandler.StartCommand_Death(dyingDirection);

		}
				
			

		if (m_playerController && m_CameraHandler && m_CameraHandler.IsInThirdPerson())
			m_playerController.m_bRetain3PV = true;
		
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
	
	
	
	
	void WaitSecondaryScriptPushRagdollAround(){
	
		GetGame().GetCallqueue().CallLater(PushRagdollAround, 10, true); // in milliseconds
	}
	
	
	void WaitSecondaryScriptFastRagdollDeath(){
	
		GetGame().GetCallqueue().CallLater(FastRagdollDeath, 50, true); // in milliseconds
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	 
	void PushRagdollAround()
	{
		int currentBones = currentRagdoll.GetNumBones();
		
		if(currentBones > 0){
			
			deltaTime = CalculateDeltaTime(false);

			
			
			//Print(currentRagdoll.GetBoneRigidBody(0).GetSimulationState());
			
			//vector hitVector = {0.0 ,-0.8 , 0.0};
			//currentRagdoll.GetBoneRigidBody(0).ApplyImpulse(hitVector);
			


			//vector feetPos = {-0.11, 0.26, 0.29};

			
			//IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(Resource.Load("{37578B1666981FCE}Prefabs/Characters/Core/Character_Base.et"));
			//vector worldCoord = SCR_BaseContainerTools.GetWorldCoords(entitySource, feetPos);
			
			float valToScaleXZ = 0.15/counter;
			
		
			
			for(int i = 0; i < currentBones; i++){
				float x = Math.RandomFloatInclusive(-valToScaleXZ, valToScaleXZ);
				//float y = Math.RandomFloatInclusive(-valToScaleY, 0.01);
				float y = Math.Lerp(-0.000001, -0.00015, deltaTime);

				float z = Math.RandomFloatInclusive(-valToScaleXZ, valToScaleXZ);
				//Print(y);
				vector hitVector = {x, y , z};		//z makes them spin 

			
				currentRagdoll.GetBoneRigidBody(i).ApplyImpulse(hitVector);
			}
			//currentRagdoll.GetBoneRigidBody(0).ApplyImpulse(hitVector);
			
			
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
			counter = 1;		//not sure if it's needed but whatev
			return;
		}
	}
	
	

	//Used in case of headshots
	void FastRagdollDeath()
	{
				
		//todo make a time limit 
		Print("FRD");
		deltaTime = CalculateDeltaTime(false);

		if(currentRagdoll.GetNumBones() > 0)
		{
			vector currentVelocity;
			float tempX
			float tempY;
			float tempZ;
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++)
			{

				tempX = 0;
				tempY = Math.Lerp(-0.001, -0.02, deltaTime);
				
				if (Math.AbsFloat(tempY) > 0.02){
					tempY = - 0.02;
					
				}
				
				Print(tempY);
				tempZ = 0;
					
				currentRagdoll.GetBoneRigidBody(i).ApplyImpulse(Vector(tempX, tempY, tempZ));
			}
		}
		else
		{
			GetGame().GetCallqueue().Remove(FastRagdollDeath);
			return;
		}
	}
	
		

	
	
	protected PhysicsRagdoll RegenPhysicsRagdoll()
	{
			// Get original ragdoll and destroy it
			PhysicsRagdoll.GetRagdoll(GetCharacter()).Destroy(1);
			
			// Recreate it
			PhysicsRagdoll.CreateRagdoll(GetCharacter(), "{5DD1A0EBAD2116CB}Prefabs/Characters/Core/character_modded.ragdoll", 1, EPhysicsLayerDefs.Ragdoll);
			//PhysicsRagdoll.CreateRagdoll(GetCharacter(), "{BDAFE20F95BD19F0}Prefabs/Characters/Core/character_modded_encircle.ragdoll", 1, EPhysicsLayerDefs.Ragdoll);
			PhysicsRagdoll ragdoll = PhysicsRagdoll.GetRagdoll(GetCharacter());
			
		
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
	
	
	float CalculateDeltaTime(bool firstUpdate)
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
}

