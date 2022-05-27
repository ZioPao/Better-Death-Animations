//#define DEBUG_PAO



//todo list 
/* 
- Interpolation between movement speed and impact speed  
- Better check for terrain, it doesn't work well when a char is on a prefab (like stairs or inside a building)
- Limiter for valToScale, we need to set 3 values.
- Lower gravity y 
- More cleaning 
- Stiffness to legs BEFORE they fall, then dampen it 
- No hitzones when vehicles crash characters
*/


modded class SCR_CharacterControllerComponent : CharacterControllerComponent{
	
	const string TAG_HITZONE_HEAD = "Head";
	const string TAG_HITZONE_LCALF = "LCalf";
	const string TAG_HITZONE_RCALF = "RCalf";
	const string TAG_HITZONE_LTHIGH = "LThigh";
	const string TAG_HITZONE_RTHIGH = "RThigh";
	const string TAG_HITZONE_NECK = "Neck";
	const string TAG_HITZONE_CHEST = "Chest";	
	const string TAG_HITZONE_LARM = "LArm";	
	const string TAG_HITZONE_RARM = "RArm";
	const string TAG_HITZONE_HIPS = "Hips";
	
	const float DIVIDER = 50;

	PhysicsRagdoll currentRagdoll;
	CharacterControllerComponent m_characterControllerComponent;
	SCR_CharacterDamageManagerComponent m_characterDamageManagerComponent;
	
	float deltaTime;
	ref BDA_Timer timer;
	
	
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
		if (!timer)
			timer = new BDA_Timer();
	
	}
	

	override void OnDeath(IEntity instigator)
	{

		if (m_OnPlayerDeath != null)
			m_OnPlayerDeath.Invoke();

		if (m_OnPlayerDeathWithParam)
			m_OnPlayerDeathWithParam.Invoke(this, instigator);



		
		// Get the player stuff. We'll do it here 'cause we can't rely on OnInit since it could have changed. 
		SCR_PlayerController m_playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		CharacterControllerComponent m_playerCharacterControllerComponent = CharacterControllerComponent.Cast(m_playerController.GetControlledEntity().FindComponent(CharacterControllerComponent));

		
		// Players won't receive it for various reasons. At least for now.
		if (m_playerCharacterControllerComponent != m_characterControllerComponent)
		{

			// Get Last Hit
			array<vector> lastHitArray = m_characterDamageManagerComponent.GetLastHitCoordinates();
			vector lastHitDirection = {lastHitArray[1][0], lastHitArray[1][1], lastHitArray[1][2]};		// for some reason I can't assign a vec to a vec so whatever
			vector hitVector = {lastHitDirection[0], lastHitDirection[1]/5, lastHitDirection[2]};		// y stays the same since we want a little more oomph
			vector hitPosition = {lastHitArray[0][0], lastHitArray[0][1], lastHitArray[0][2]};
			
			//if it's a headshot, then no rolling around 
			
			//fucking hell it stopped working for some reasons, vehicles no longer work.
			HitZone hitZone = m_characterDamageManagerComponent.GetLastHitZone();

				int hitZoneColliderID = m_characterDamageManagerComponent.GetLastColliderID();
				string hitZoneName = hitZone.GetName();
				
				vector hitZoneColliders[4];
				hitZone.TryGetColliderDescription(GetCharacter(), hitZoneColliderID, hitZoneColliders, null, null);
		
				vector hitToApply;			
				//Print(hitZoneName);
				switch(hitZoneName)
				{
	
					case TAG_HITZONE_LCALF:
					case TAG_HITZONE_RCALF:
					case TAG_HITZONE_LTHIGH:
					case TAG_HITZONE_RTHIGH:
					case TAG_HITZONE_HIPS:
					
					{
						hitToApply = hitVector/20;
						break;
					}
					case TAG_HITZONE_HEAD:
					case TAG_HITZONE_NECK:
					{
						hitToApply = hitVector/15;
						break;
					}
					default:
					{
						hitToApply = hitVector/7;
	
					}
				
				}			



			
			/* Preventing feet to clip in the ground */
			vector characterOrigin = GetCharacter().GetOrigin();
			float surfWorldY = GetGame().GetWorld().GetSurfaceY(characterOrigin[0], characterOrigin[2]);
			float gravityToApply;
			float differenceY = Math.AbsFloat(characterOrigin[1] - surfWorldY);
			float safetyY = 0.05;		
			if( differenceY >= 0.005)
			{
				vector matrixTransform[4];
				GetCharacter().GetTransform(matrixTransform);
				matrixTransform[3] = Vector(matrixTransform[3][0], matrixTransform[3][1] + differenceY + safetyY, matrixTransform[3][2]);
				GetCharacter().SetTransform(matrixTransform);

			}

			// Regen ragdoll
			currentRagdoll = BDA_Functions_Generic.RegenPhysicsRagdoll(GetCharacter());
			
			//Finally starts the ragdoll
			m_characterControllerComponent.Ragdoll();
			
			
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++)
			{	
				//we need to know which rigidbody we're applying the force. we can't apply force to the feet, they will glitch out.
				if (i == 9 || i == 10 || i == 11 || i == 12)
					gravityToApply = -0.3;
				else
					gravityToApply = -9.81;		
				
				
				currentRagdoll.GetBoneRigidBody(i).ApplyForce(Vector(0, gravityToApply, 0));		
			}
			

			//Applies an impulse to let the things start.
			currentRagdoll.GetBoneRigidBody(0).ApplyImpulseAt(hitPosition, hitToApply);		

			// Special case for headshots, basically "instakill"
			
			int waitTime = Math.RandomIntInclusive(300, 800);
			
			//Get the delta time for everything after this 
			timer.Start();
			if (hitZoneName == TAG_HITZONE_HEAD)
				GetGame().GetCallqueue().CallLater(WaitSecondaryScriptFastRagdollDeath, waitTime, false);
			else
				GetGame().GetCallqueue().CallLater(WaitSecondaryScriptPushRagdollAround, 500, false);		



			
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
			SCR_CharacterCommandHandlerComponent characterCommandHandlerComponent = BDA_Functions_Generic.FindCommandHandler(GetCharacter());
			//We need the CharacterInputContext for the player
			CharacterInputContext m_characterInputContext = m_playerCharacterControllerComponent.GetInputContext();
			float dyingDirection = m_characterInputContext.GetDie();
			
			if (dyingDirection != 0.0)
				characterCommandHandlerComponent.StartCommand_Death(dyingDirection);

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
		

	
	/* Wrappers to wait before starting the real functions*/
	
	void WaitSecondaryScriptPushRagdollAround(){
		
		//Right when we start, we're gonna start from this value to scale on
		float startValue = 0.0005;
		// Middle Point 
		float middleValue = 0.03;  
		//When it's gonna stop to change 
		float endValue = 0.0;		
		
		//how much we're gonna increment, make it a little random. This is gonna be a seed basically 
		float step = 0.0002;
		
		GetGame().GetCallqueue().CallLater(PushRagdollAround, 10, true, startValue, middleValue, endValue, step); // in milliseconds
	}
	
	
	void WaitSecondaryScriptFastRagdollDeath(){
	
		GetGame().GetCallqueue().CallLater(FastRagdollDeath, 10, true); // in milliseconds
	}
	
	
	
	bool hasReachedMiddleValue = false;		//should be "local" afaik but i'm not sure.
	float currentValToScale = 0.0;
	
	/* Real function to make the ragdolls move around */
	void PushRagdollAround(float startValue, float middleValue, float endValue, float step)
	{
		
		
		deltaTime = timer.UpdateDeltaTime();
		
		float timeStep = Math.AbsFloat(step/deltaTime);
		
		if (timeStep < step)
			timeStep = step;	//restore it
		

		if (currentRagdoll.GetNumBones() > 0)
		{
			
			if (currentValToScale > middleValue || hasReachedMiddleValue)
			{
				hasReachedMiddleValue=true;
				//decrease until endValue
				

				if (currentValToScale < endValue)
				{
					//Print("Keeping end value");
					currentValToScale = endValue;		//don't change it. 
					//Print(currentValToScale);
	
				}
				else
				{
					//Print("Decreasing");
					currentValToScale -= timeStep;
					//Print(currentValToScale);

				}
			}
			else
			{
				//Print("Increasing");
				//Increase till middle value 
				currentValToScale += timeStep;
				//Print(currentValToScale);
			}
			
			Print("___________________________________________");
			
			
			float x = Math.RandomFloatInclusive(-currentValToScale, currentValToScale);
			
			float y = Math.Lerp(0.1, 0.2, deltaTime)/DIVIDER;
			
			if (y >= 0.2)
				y = 0.2;
			
			
			
			float z = Math.RandomFloatInclusive(-currentValToScale, currentValToScale);
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++)
			{
				vector hitVector = {x, -y , z};		//z makes them spin 
				currentRagdoll.GetBoneRigidBody(i).ApplyImpulse(hitVector);
			}

	

			
		}
		else
		{
			GetGame().GetCallqueue().Remove(PushRagdollAround);
			return;
			
			
		}
		
	}
	
	/* Used when charcter get headshotted*/
	void FastRagdollDeath()
	{
		
		deltaTime = timer.UpdateDeltaTime();
		
		
		
		
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
				
				if (Math.AbsFloat(tempY) > 0.02)
					tempY = - 0.02;
				
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
	
	
	
	void HeadDismemberment(){
				
			
			// We can use this for head dismemberment... not sure about other stuff
			//CharacterIdentityComponent identity = CharacterIdentityComponent.Cast(GetCharacter().FindComponent(CharacterIdentityComponent));
			//identity.SetCovered(hitZoneName, false);
	}
}

