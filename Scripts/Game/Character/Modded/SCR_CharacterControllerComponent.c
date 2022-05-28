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
	const string TAG_HITZONE_ABDOMEN = "Abdomen";
	
	const float DIVIDER = 100;
	
	const float DEFAULT_MAIN_DAMPING = 0.00000001;		// ONLY FOR 0
 	float DEFAULT_MAIN_DAMPING_SUB = 1 - DEFAULT_MAIN_DAMPING;
	
	const float DEFAULT_SECONDARY_DAMPING = 0.1;				//0.25 ?
	float DEFAULT_SECONDARY_DAMPING_SUB = 1 - DEFAULT_SECONDARY_DAMPING;
	
	const float MODIFIED_SECONDARY_DAMPING = 1.0;
	const float MODIFIED_SECONDARY_DAMPING_WHILE_MOVING = 0.75;

	
	// fucking hell  
	
	const float DAMPING_ARM = 15.0;
	const float DAMPING_FOREARM = 10.0;
	
	
	
	PhysicsRagdoll currentRagdoll;
	CharacterControllerComponent m_characterControllerComponent;
	SCR_CharacterDamageManagerComponent m_characterDamageManagerComponent;
	
	float deltaTime;
	ref BDA_Timer timer;
	ref array<float> originalMasses;

	
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
			float lastHitSpeed = m_characterDamageManagerComponent.GetLastHitSpeed();
			
			HitZone hitZone = m_characterDamageManagerComponent.GetLastHitZone();
			vector hitToApply;		
			string hitZoneName;

			
						
			
			// Get current velocity of char 
			
			// DONT CHANGE THE DAMPING WHEN THERE IS MOVEMENT YOU COCK
			
			vector movementVelocity = m_characterControllerComponent.GetMovementVelocity();
			
			
			
			
			// 
			
			
			Print(m_characterControllerComponent.GetMovementVelocity());
			Print(hitVector);
			Print("After considering char velocity");
			hitVector += movementVelocity;		//no idea if it's gonna work
			Print(hitVector);

			Print("______________________");

			
			if (hitZone)
			{
				int hitZoneColliderID = m_characterDamageManagerComponent.GetLastColliderID();
				hitZoneName = hitZone.GetName();

				Print(hitZoneName);
				switch(hitZoneName)
				{
	
					case TAG_HITZONE_LCALF:
					case TAG_HITZONE_RCALF:
					case TAG_HITZONE_LTHIGH:
					case TAG_HITZONE_RTHIGH:
					{
						hitToApply = hitVector/5;
						break;
					}
					case TAG_HITZONE_HIPS:
					case TAG_HITZONE_ABDOMEN:
					{
						hitToApply = hitVector/3;
						break;
					}

					case TAG_HITZONE_HEAD:
					case TAG_HITZONE_NECK:
					{

						hitToApply = hitVector/15;
						break;
					}
					case TAG_HITZONE_CHEST:
					{
						hitToApply = hitVector/4;
						break;
					}
					case TAG_HITZONE_RARM:
					case TAG_HITZONE_RFOREARM:
					case TAG_HITZONE_LARM:
					case TAG_HITZONE_LFOREARM:
					{
						hitToApply = hitVector/12;
						break;
					}
					
					default:
					{
						hitToApply = hitVector/3;
	
					}
				
				}	
			}
			else
			{
				hitToApply = hitVector/10;		//no idea 
				hitZoneName = "";
			}
			
			


			
			/* Preventing feet to clip in the ground */
			
			//todo this is crap 
			
			
			// from the char right down to check if it's a prefab\building or something like that. if it is... then whatever, don't do shit.
			
			// todo maybe a ray is better? It is but it doesn't work thanks bohemia
			GetGame().GetWorld().QueryEntitiesBySphere(GetCharacter().GetOrigin(), 0.1, TestPosition, null, EQueryEntitiesFlags.STATIC);
			if (isCharacterInAcceptablePosition)
			{
				vector characterOrigin = GetCharacter().GetOrigin();
				float surfWorldY = GetGame().GetWorld().GetSurfaceY(characterOrigin[0], characterOrigin[2]);
				float differenceY = Math.AbsFloat(characterOrigin[1] - surfWorldY);
				float safetyY = 0.05;		
				if( differenceY >= 0.0005)
				{
					vector matrixTransform[4];
					GetCharacter().GetTransform(matrixTransform);
					matrixTransform[3] = Vector(matrixTransform[3][0], matrixTransform[3][1] + differenceY + safetyY, matrixTransform[3][2]);
					GetCharacter().SetTransform(matrixTransform);
	
				}
			}
			//else
			//{
				//Print("In building or something");

			//}
			
			

			

			


			// Regen ragdoll
			currentRagdoll = BDA_Functions_Generic.RegenPhysicsRagdoll(GetCharacter());
			
			//Finally starts the ragdoll
			m_characterControllerComponent.Ragdoll();
			int test = 1000;
			
			
			
			
			float gravityToApply = 0;
			originalMasses = new array<float>;
			
			

			
			
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++)
			{	

				originalMasses.Insert(currentRagdoll.GetBoneRigidBody(i).GetMass());

				
				switch(i)
				{
					case CharacterBones.LCALF:
					case CharacterBones.RCALF:
					case CharacterBones.RFOOT:
					case CharacterBones.LFOOT:
					{
						
						//todo clean this shit
									
						if (movementVelocity.Length() < 0.05)
						
							currentRagdoll.GetBoneRigidBody(i).SetDamping(MODIFIED_SECONDARY_DAMPING ,MODIFIED_SECONDARY_DAMPING);
		
						
						else
							currentRagdoll.GetBoneRigidBody(i).SetDamping(MODIFIED_SECONDARY_DAMPING_WHILE_MOVING ,	MODIFIED_SECONDARY_DAMPING_WHILE_MOVING);
						
						currentRagdoll.GetBoneRigidBody(i).SetMass(10);

												
						break;


					}
					default:
					{
						gravityToApply = -9.81;		
					}
				
				}
				

				currentRagdoll.GetBoneRigidBody(i).ApplyForce(Vector(0, gravityToApply, 0));		
			}
						

			//Applies an impulse to let the things start.
			currentRagdoll.GetBoneRigidBody(0).ApplyImpulseAt(hitPosition, hitToApply);		

			// Special case for headshots, basically "instakill"
			
			int waitTime = Math.RandomIntInclusive(20, 50);
			

			if (hitZoneName == TAG_HITZONE_HEAD)
				GetGame().GetCallqueue().CallLater(WaitSecondaryScriptFastRagdollDeath, waitTime, false);
			else
				GetGame().GetCallqueue().CallLater(WaitSecondaryScriptPushRagdollAround, waitTime, false);		



			
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
		float startValue = 0.02;
		// Middle Point 
		float middleValue = 2;
		//When it's gonna stop to change 
		float endValue = 0.0;		
		
		//how much we're gonna increment, make it a little random. This is gonna be a seed basically 
		float step = 0.01;
		
		restoredMasses = new array<float>;			
		
		//Get the delta time for everything after this 
		timer.Start();

		GetGame().GetCallqueue().CallLater(PushRagdollAround, 50, true, startValue, middleValue, endValue, step); // in milliseconds
	}
	
	
	void WaitSecondaryScriptFastRagdollDeath(){
	
		GetGame().GetCallqueue().CallLater(FastRagdollDeath, 10, true); // in milliseconds
	}
	
	
	float dampingStep = 0.08;

	bool hasReachedMiddleValue = false;		//should be "local" afaik but i'm not sure.
	float currentValToScale = 0.0;
	ref array<float> restoredMasses;
	
	/* Real function to make the ragdolls move around */
	void PushRagdollAround(float startValue, float middleValue, float endValue, float step)
	{
		

		deltaTime = timer.UpdateDeltaTime();

		if (!currentRagdoll)
			return;			//we must wait I guess

		if (currentRagdoll.GetNumBones() > 0)
		{
			float timeStep;
			if (deltaTime > 0)
			{
				timeStep = Math.AbsFloat(step/deltaTime);
				if (timeStep < step)
					timeStep = step;	//restore it
			}
			else
				timeStep = step;

			float dampintToSet;
			float dampingDuration = 0.2;
			
			if (deltaTime < dampingDuration)
			{
				
				// damping to start on should be
				
				//todo no easy way to get original dampening ffs
				dampintToSet = MODIFIED_SECONDARY_DAMPING - Math.Lerp(0.0, DEFAULT_SECONDARY_DAMPING_SUB, deltaTime) ;			// just for test

				//currentRagdoll.GetBoneRigidBody(1).SetDamping(dampintToSet, dampintToSet);
				currentRagdoll.GetBoneRigidBody(CharacterBones.LCALF).SetDamping(dampintToSet, dampintToSet);
				currentRagdoll.GetBoneRigidBody(CharacterBones.RCALF).SetDamping(dampintToSet, dampintToSet);
				currentRagdoll.GetBoneRigidBody(CharacterBones.RFOOT).SetDamping(dampintToSet, dampintToSet);
				currentRagdoll.GetBoneRigidBody(CharacterBones.LFOOT).SetDamping(dampintToSet, dampintToSet);
			
			}
			

			float massDuration = 0.2;
			if (deltaTime < massDuration)
			{
 				// todo maybe add al ittle bit of weight, but it'll affect movement after, keep it in mind
				float modifier = 5.0;
				currentRagdoll.GetBoneRigidBody(CharacterBones.SPINE).SetMass(Math.Lerp(10.0, originalMasses[CharacterBones.SPINE] + modifier, deltaTime/massDuration));
				currentRagdoll.GetBoneRigidBody(CharacterBones.LCALF).SetMass(Math.Lerp(10.0, originalMasses[CharacterBones.LCALF] + modifier, deltaTime/massDuration));
				currentRagdoll.GetBoneRigidBody(CharacterBones.RCALF).SetMass(Math.Lerp(10.0, originalMasses[CharacterBones.RCALF] + modifier, deltaTime/massDuration));
				currentRagdoll.GetBoneRigidBody(CharacterBones.RFOOT).SetMass(Math.Lerp(10.0, originalMasses[CharacterBones.RFOOT] + modifier, deltaTime/massDuration));
				currentRagdoll.GetBoneRigidBody(CharacterBones.LFOOT).SetMass(Math.Lerp(10.0, originalMasses[CharacterBones.LFOOT] + modifier, deltaTime/massDuration));

			} 
			

		
			
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
			
			//Print("___________________________________________");
			
			
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++)
			{
				vector hitVector; //= {x, -y , z};		//z makes them spin 
							
				//Random for every loop.
				float x = Math.RandomFloatInclusive(-currentValToScale, currentValToScale);
				float y = BDA_Functions_Generic.Lerp(0.55, 2.5, 1, deltaTime);
				float z = Math.RandomFloatInclusive(-currentValToScale, currentValToScale);
				
				

				switch(i)
				{
					case CharacterBones.LARM:
					case CharacterBones.LFOREARM:
					case CharacterBones.RARM:
					case CharacterBones.RFOREARM:
					case CharacterBones.HEAD:
					{
						//hitVector = {x/2, -y + 0.08 , z/2};		//arms are a special case, let's just help them a bit poor things
						
						
						
						
						
						float testDividerNeg = 500 - (deltaTime * 50);
						
						if (testDividerNeg < 0)
							testDividerNeg = 1;
						
						float testDividerPos = 1000 + (deltaTime * 150);

						
						float valToScaleY = BDA_Functions_Generic.Lerp(1, 3, 15, deltaTime);
						
						y =  Math.RandomFloatInclusive(-valToScaleY/testDividerNeg , valToScaleY/testDividerPos);


						#ifdef DEBUG_PAO
						Print(testDividerNeg);
						Print(testDividerPos);
						Print(valToScaleY);
						
						Print("___________");
						#endif
						hitVector = {x/200, y/200, z/200};
						//hitVector = {0, 0, 0};	S
						
						
						break;
					}
					case CharacterBones.LCALF:
					case CharacterBones.LFOOT:
					case CharacterBones.RCALF:
					case CharacterBones.RFOOT:
					{
						// SOmething else since they need some more force
						//hitVector = {x*100, -y , z*100};		//arms are a special case, let's just help them a bit poor things
						
						// With these settings, 2 should be the middle point.

						hitVector = {x, -y, z};
						break;
					}
					case CharacterBones.LTHIGH:
					case CharacterBones.RTHIGH: 
					{
						hitVector = {0, -y/2, 0};	
						break;
					}
					default:
					{
						//No changes? there is something wrong with x and z, scale them manually for now
						//hitVector = {x, -y , z};		
						//hitVector = {0, -y , 0};
						hitVector = {0, -y, 0};

					}
				}
				
				currentRagdoll.GetBoneRigidBody(i).ApplyImpulse(hitVector);

				

			}		
		}
		else
		{
			GetGame().GetCallqueue().Remove(PushRagdollAround);
			return;
			
			
		}
		
	}
	
	
	
	
	
	
	int counterLoopFastRagdollDeath = 0;
	/* Used when charcter get headshotted*/
	void FastRagdollDeath()
	{
		
		deltaTime = timer.UpdateDeltaTime();
		

		
		if(currentRagdoll.GetNumBones() > 0)
		{
			currentRagdoll.GetBoneRigidBody(CharacterBones.SPINE).SetDamping(DEFAULT_MAIN_DAMPING, DEFAULT_MAIN_DAMPING);
			currentRagdoll.GetBoneRigidBody(CharacterBones.LCALF).SetDamping(DEFAULT_MAIN_DAMPING, DEFAULT_MAIN_DAMPING);
			currentRagdoll.GetBoneRigidBody(CharacterBones.RCALF).SetDamping(DEFAULT_MAIN_DAMPING, DEFAULT_MAIN_DAMPING);
			currentRagdoll.GetBoneRigidBody(CharacterBones.RFOOT).SetDamping(DEFAULT_MAIN_DAMPING, DEFAULT_MAIN_DAMPING);
			currentRagdoll.GetBoneRigidBody(CharacterBones.LFOOT).SetDamping(DEFAULT_MAIN_DAMPING, DEFAULT_MAIN_DAMPING);
			
			
			//pls dont be broken
			
			
			// i fucking hate myself.
			
			// ok so this stinks, it was a mistake but it worked. spine has an higher mass, it works better than a lower 
			// mass like the original.... So I should really refactor this crap.
			currentRagdoll.GetBoneRigidBody(CharacterBones.SPINE).SetMass(originalMasses[CharacterBones.SPINE]);
			currentRagdoll.GetBoneRigidBody(CharacterBones.LCALF).SetMass(originalMasses[CharacterBones.SPINE]);
			currentRagdoll.GetBoneRigidBody(CharacterBones.RCALF).SetMass(originalMasses[CharacterBones.SPINE]);
			currentRagdoll.GetBoneRigidBody(CharacterBones.RFOOT).SetMass(originalMasses[CharacterBones.SPINE]);
			currentRagdoll.GetBoneRigidBody(CharacterBones.LFOOT).SetMass(originalMasses[CharacterBones.SPINE]);
	
			float x;
			float y;
			float z;
			
			
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++)
			{
				
				
				if (counterLoopFastRagdollDeath < 10)
				{
					x = Math.RandomFloatInclusive(-0.1, 0.1);
					z = Math.RandomFloatInclusive(-0.1, 0.1);
				}
				else
				{
					x = 0;
					z = 0;
				}
				
				
				
				y = BDA_Functions_Generic.Lerp(0.0001, 0.02, 1.0, deltaTime);

				
				if (i == CharacterBones.LFOREARM || i == CharacterBones.RFOREARM)
					y -= 0.005;
				
	
					
				currentRagdoll.GetBoneRigidBody(i).ApplyImpulse(Vector(x, -y, z));
			}
		}
		else
		{
			GetGame().GetCallqueue().Remove(FastRagdollDeath);
			return;
		}
		
		
		counterLoopFastRagdollDeath++;
	}
	
	
	
	void HeadDismemberment()
	{
				
			
			// We can use this for head dismemberment... not sure about other stuff
			//CharacterIdentityComponent identity = CharacterIdentityComponent.Cast(GetCharacter().FindComponent(CharacterIdentityComponent));
			//identity.SetCovered(hitZoneName, false);
	}
	
	
	

	bool isCharacterInAcceptablePosition = true;		//default true
	
	
	bool TestPosition(notnull IEntity ent)
	{
		
		
		if (!isCharacterInAcceptablePosition)
			return true;
		
		isCharacterInAcceptablePosition = (ent.ClassName() != "SCR_DestructibleBuildingEntity") && (ent.ClassName() != "GenericEntity");
		
		//	if (!isCharacterInAcceptablePosition)
		//		Print(ent.ClassName());
		
		return true;
		
	//not sure if there is a better way but right now i dont care

		
		//m_aDbgCollisionShapes = new array<ref Shape>;
		//vector charOriginTemp = GetCharacter().GetOrigin();
		//float yWorldTemp = GetGame().GetWorld().GetSurfaceY(characterOrigin[0], characterOrigin[2]);
		//Shape shapeTest = Shape.CreateSphere(ARGBF(1,1,1,1), ShapeFlags.NOOUTLINE, charOriginTemp, 0.2);
		//m_aDbgCollisionShapes.Insert(shapeTest);

		
		//vector checkVector = {charOriginTemp[0], yWorldTemp, charOriginTemp[2]};
		
		//Debug_DrawLineSimple(GetCharacter().GetOrigin(), checkVector, m_aDbgCollisionShapes);		
				
		//Print(ent.GetName());
	}
	
}


/*

//mixed left hand
		bone "leftArm", true, 0.1
		{
			sixdofjoint "0 0 20", -30, 10, -40, 20, -10, 0, 4, 225
	

			bone "LeftForeArm", true, 0.07
			{
				sixdofjoint "20 0 0", -30, 10, -40, 20, -10, 0, 50, 15

				bone "LeftHandMiddle1", false
			}
		}

		//mixed right hand
		bone "RightArm", true, 0.1
		{
			// x is the value we most need. manages forward motion for the right arm
			sixdofjoint "0 0 10", 30, -10, 0, 0, 0,0, 50 , 225

			bone "RightForeArm", true, 0.07
			{
				sixdofjoint "0 0 20", -30, 10, -40, 20, 0, 0, 50, 15

				bone "RightHandMiddle1", false
			}
		}

*/
