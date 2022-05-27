//#define DEBUG_PAO



//todo list 
/* 
- Interpolation between movement speed and impact speed  
- Better check for terrain, it doesn't work well when a char is on a prefab (like stairs or inside a building)
- Lower gravity y 
- More cleaning 

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
	
	const float startDivider = 2000;
	float counter = 1;

	PhysicsRagdoll currentRagdoll;
	CharacterControllerComponent m_characterControllerComponent;
	SCR_CharacterDamageManagerComponent m_characterDamageManagerComponent;
	
	float deltaTime;
	ref BDA_Timer timer;
	float divider;
	
	
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
		if (m_playerCharacterControllerComponent != m_characterControllerComponent){

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
	
		GetGame().GetCallqueue().CallLater(PushRagdollAround, 10, true); // in milliseconds
	}
	
	
	void WaitSecondaryScriptFastRagdollDeath(){
	
		GetGame().GetCallqueue().CallLater(FastRagdollDeath, 50, true); // in milliseconds
	}
	
	
	
	/* Real function to make the ragdolls move around */
	void PushRagdollAround()
	{

		
		// we're gonna use a parabola to simulate all the phases, stupor, shock, and then death. 
		divider = startDivider;
		
		
		if(currentRagdoll.GetNumBones() > 0)
		{
			deltaTime = timer.UpdateDeltaTime();
			float valToScale = Math.Lerp(0,2, deltaTime)/divider;
			float valToScaleSecond = Math.Lerp(-100, 100, deltaTime);
			float valToScaleXZ = 0.12/counter;			//15 is too heavy? Also fuck this counter why did I even use it
			
			
			
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++)
			{
				
				divider += deltaTime;
				
				
				
				float x = Math.RandomFloatInclusive(-valToScaleXZ, valToScaleXZ);
				float y = -valToScale;
				float z = Math.RandomFloatInclusive(-valToScaleXZ, valToScaleXZ);

				//float y = -Math.Lerp(0,1,deltaTime)/divider;
				
				divider += deltaTime;	//Increment it with time
				//float y = Math.Lerp(-0.001, -0.015, deltaTime);			//todo this really needs to get fixed
				
				
				//Print(valToScaleSecond);
				//vector parabola = BDA_Functions_Generic.GenerateParabola(-5.0, 5.0, 10, deltaTime);
				//float y = parabola[1];		//not sure about this.

				
				vector hitVector = {x, y , z};		//z makes them spin 
				currentRagdoll.GetBoneRigidBody(i).ApplyImpulse(hitVector);
				counter += 0.0045;
			}
		}
		else
		{
			GetGame().GetCallqueue().Remove(PushRagdollAround);
			counter = 1;
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

