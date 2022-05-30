//#define DEBUG_PAO



//todo list 
/* 
- Interpolation between movement speed and impact speed, maybe clean it up a little
- Better check for terrain, it doesn't work well when a char is on a prefab (like stairs or inside a building)
- More cleaning 

- Consider prone units 
- Add a check for "falling" units. Could cause those goddamn legs in the ground
- performacne stuff you cock
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
	
	
	
	[Attribute(defvalue: "0.00000001", uiwidget: UIWidgets.Slider, params: "0 1.0 0.00001", precision: 24, desc: "test", category: "Ragdoll")]
	private float defaultMainDamping;
	private float defaultMainDampingSub = 1 - defaultMainDamping;

	
	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, params: "0 1.0 0.00001", precision: 24, desc: "Test", category: "Ragdoll")]
	private float defaultSecondaryDamping;
	private float defaultSecondaryDampingSub = 1 - defaultSecondaryDamping;
	
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 1.0 0.00001", precision: 24, desc: "Test", category: "Ragdoll")]
	private float modifiedSecondaryDamping;
	
	[Attribute(defvalue: "0.75", uiwidget: UIWidgets.Slider, params: "0 1.0 0.00001", precision: 24, desc: "Test", category: "Ragdoll")]
	private float modifiedSecondaryDampingWhileMoving;
	
	[Attribute(defvalue: "0.297619", uiwidget: UIWidgets.Slider, params: "0 1.0 000001", precision: 24, desc: "TEST", category: "Ragdoll")]
	private float modifiedMassFastDeath;
	

	
	PhysicsRagdoll currentRagdoll;
	CharacterControllerComponent m_characterControllerComponent;
	SCR_CharacterDamageManagerComponent m_characterDamageManagerComponent;
	
	float deltaTime;
	ref BDR_Timer timer;
	ref array<float> originalMasses;

	
	//todo find a better way
	ref array<CharacterBones> lowerBodyBones = new array<CharacterBones>;
	ref array<CharacterBones> lowerBodyBonesAndSpine = new array<CharacterBones>;


	
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
			timer = new BDR_Timer();
		
		//lowerBodyBones.Insert(CharacterBones.SPINE);			//not sure about this
		lowerBodyBones.Insert(CharacterBones.LCALF);
		lowerBodyBones.Insert(CharacterBones.RCALF);
		lowerBodyBones.Insert(CharacterBones.RFOOT);
		lowerBodyBones.Insert(CharacterBones.LFOOT);
		
		lowerBodyBonesAndSpine.InsertAll(lowerBodyBones);
		lowerBodyBonesAndSpine.Insert(CharacterBones.SPINE);
	
	}
	

	override void OnDeath(IEntity instigator)
	{

		if (m_OnPlayerDeath != null)
			m_OnPlayerDeath.Invoke();

		if (m_OnPlayerDeathWithParam)
			m_OnPlayerDeathWithParam.Invoke(this, instigator);

		Rpc(RpcAsk_MainMethodBroadcast);
		Rpc(RpcAsk_MainMethodAuthority);
		//RpcAsk_MainMethod();
		
		//will it work?
		

		if (SCR_PlayerController.Cast(GetGame().GetPlayerController()) && m_CameraHandler && m_CameraHandler.IsInThirdPerson())
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).m_bRetain3PV = true;
		
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

			
			
			//Set all the damping stuff
			float dampingDuration = 0.2;
			float dampingToSet = modifiedSecondaryDamping - BDR_Functions_Generic.Lerp(0.0, defaultSecondaryDampingSub, dampingDuration, deltaTime);
			ManageDamping(lowerBodyBones, dampingToSet);

			

			float massDuration = 0.2;
			if (deltaTime < massDuration)
			{
 				// todo maybe add al ittle bit of weight, but it'll affect movement after, keep it in mind
				float modifier = 5.0;
				foreach (CharacterBones x : lowerBodyBonesAndSpine)	
				{
					float currentMass = Math.Lerp(10.0, originalMasses[x] + modifier, deltaTime/massDuration);
					currentRagdoll.GetBoneRigidBody(x).SetMass(currentMass);
				}


			} 
			

			//Not really sure we should use that strange timeStep crap that I made before, maybe replace it with lerp?
			BDR_Functions_Generic.IncreaseAndThenDecrease(middleValue, endValue, timeStep, currentValToScale, hasReachedMiddleValue);
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++)
			{
				vector hitVector; //= {x, -y , z};		//z makes them spin 
							
				//Random for every loop.
				float x = Math.RandomFloatInclusive(-currentValToScale, currentValToScale);
				float y = BDR_Functions_Generic.Lerp(0.55, 2.5, 1, deltaTime);
				float z = Math.RandomFloatInclusive(-currentValToScale, currentValToScale);
				
				

				switch(i)
				{
					case CharacterBones.LARM:
					case CharacterBones.LFOREARM:
					case CharacterBones.RARM:
					case CharacterBones.RFOREARM:
					case CharacterBones.HEAD:
					{
						
						//All this stuff to make their arms move for a bit and then stop, going towards the ground
						float testDividerNeg = 500 - (deltaTime * 50);
						
						if (testDividerNeg < 0)
							testDividerNeg = 1;
						
						float testDividerPos = 1000 + (deltaTime * 150);
						float valToScaleY = BDR_Functions_Generic.Lerp(1, 3, 15, deltaTime);
						
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
						hitVector = {x, -y, z};				// Check again 
						break;
					}
					case CharacterBones.LTHIGH:
					case CharacterBones.RTHIGH: 
					{
						hitVector = {0, -y/2, 0};			// Just in case
						break;
					}
					default:
					{
						hitVector = {0, -y, 0};		// Applies only gravity
						break;

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
			
			
			ManageDamping(lowerBodyBones, defaultMainDamping);
			
			/* To make bodies go down faster we're gonna set an higher mass for every bone. We'll use the spine mass as a reference */
			foreach(CharacterBones x : lowerBodyBonesAndSpine)
				currentRagdoll.GetBoneRigidBody(x).SetMass(modifiedMassFastDeath);
			

			float x;
			float y;
			float z;
			
			float fastTempMovement = 0.05;
			
			for(int i = 0; i < currentRagdoll.GetNumBones(); i++)
			{
				
				
				if (counterLoopFastRagdollDeath < 10)
				{
					x = Math.RandomFloatInclusive(-fastTempMovement, fastTempMovement);
					z = Math.RandomFloatInclusive(-fastTempMovement, fastTempMovement);
				}
				else
				{
					x = 0;
					z = 0;
				}

				y = BDR_Functions_Generic.Lerp(0.0001, 0.025, 0.5, deltaTime);
				if (i == CharacterBones.LFOREARM || i == CharacterBones.RFOREARM)
					y -= 0.018;
				
	
					
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
	


	bool isCharacterInAcceptablePosition = true;		//default true
	bool TestPosition(notnull IEntity ent)
	{
		
		
		if (!isCharacterInAcceptablePosition)
			return true;
		
		isCharacterInAcceptablePosition = (ent.ClassName() != "SCR_DestructibleBuildingEntity") && (ent.ClassName() != "GenericEntity");
		
		
		return true;
		

	}
	
	
	
	
	
	void ManageDamping(array<CharacterBones> bonesArray, float newDamping)
	{
		foreach(CharacterBones x: bonesArray)
			currentRagdoll.GetBoneRigidBody(x).SetDamping(newDamping, newDamping);
		
	}
	
	
	
	
	
	// NETWORK STUFF 

	
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	void RpcAsk_MainMethodBroadcast()
	{
		ManageRagdoll();
	}
	
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	void RpcAsk_MainMethodAuthority()
	{
		ManageRagdoll();
	}
	
	void ManageRagdoll()
	{
			
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

			
						
			
			/* We need to consider moving characters, so let's just sum those two vectors and then manage it later.
			Consider that damping must not be as aggressive as when the character is not moving.*/
			vector movementVelocity = m_characterControllerComponent.GetMovementVelocity();
			hitVector += movementVelocity;		

			
			
			
			/*
			Print(m_characterControllerComponent.GetMovementVelocity());
			Print(hitVector);
			Print("After considering char velocity");
			Print(hitVector);
			Print("______________________");
			*/
			
			if (hitZone)
			{
				int hitZoneColliderID = m_characterDamageManagerComponent.GetLastColliderID();
				hitZoneName = hitZone.GetName();

				//Print(hitZoneName);
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
					case TAG_HITZONE_LARM:
					{
						hitToApply = hitVector/12;
						break;
					}
					
					default:
					{
						hitToApply = hitVector/3;
						break;		
					}
				
				}	
			}
			else
			{
				// If a character dies by falling orby getting hit by a car, there will be no hitzone. 
				hitToApply = hitVector/20;		
				hitZoneName = "";
			}
			
			


			
			/* Preventing feet to clip in the ground. Still a pretty jank solution but whatever*/
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

			
			
			/* Manages the ragdoll stuff, gravity, forces, etc. */
			currentRagdoll = BDR_Functions_Generic.RegenPhysicsRagdoll(GetCharacter());
			m_characterControllerComponent.Ragdoll();
			
			
			
			//Apply gravity
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
							currentRagdoll.GetBoneRigidBody(i).SetDamping(modifiedSecondaryDamping ,modifiedSecondaryDamping);
						else
							currentRagdoll.GetBoneRigidBody(i).SetDamping(modifiedSecondaryDampingWhileMoving, modifiedSecondaryDampingWhileMoving);
						
						currentRagdoll.GetBoneRigidBody(i).SetMass(10);

												
						break;


					}
					default:
					{
						gravityToApply = -9.81;		
						break;
					}
				
				}
			
				currentRagdoll.GetBoneRigidBody(i).ApplyForce(Vector(0, gravityToApply, 0));		
			}
						

			//Applies impulses
			currentRagdoll.GetBoneRigidBody(0).ApplyImpulseAt(hitPosition, hitToApply);		
			int waitTime = Math.RandomIntInclusive(20, 50);
			
			
			
			if (hitZoneName == TAG_HITZONE_HEAD)
				GetGame().GetCallqueue().CallLater(WaitSecondaryScriptFastRagdollDeath, waitTime, false);		// Special case for headshots, basically "instakill"
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
			SCR_CharacterCommandHandlerComponent characterCommandHandlerComponent = BDR_Functions_Generic.FindCommandHandler(GetCharacter());
			//We need the CharacterInputContext for the player
			CharacterInputContext m_characterInputContext = m_playerCharacterControllerComponent.GetInputContext();
			float dyingDirection = m_characterInputContext.GetDie();
			
			if (dyingDirection != 0.0)
				characterCommandHandlerComponent.StartCommand_Death(dyingDirection);

		}
		
		
	}
	

	
	
	
}
