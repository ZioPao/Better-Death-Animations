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
	

	
	PhysicsRagdoll currentRagdoll;
	CharacterControllerComponent m_characterControllerComponent;
	SCR_CharacterDamageManagerComponent m_characterDamageManagerComponent;
	
	float deltaTime;
	ref BDR_Timer timer;
	ref array<float> originalMasses;

	
	//todo find a better way
	ref array<CharacterBones> lowerBodyBones = new array<CharacterBones>;
	ref array<CharacterBones> lowerBodyBonesAndSpine = new array<CharacterBones>;

	
	float defaultMainDamping;
	float defaultSecondaryDamping;
	float modifiedSecondaryDamping;
	float modifiedSecondaryDampingWhileMoving;
	float modifiedMassFastDeath;
	
	bool activateHitImpact;
	
	

	
	ref MCF_JsonManager mcfJson;
	ref map<string, string> bdrSettings

	//ref array<ref Shape> debugShapesBDR;
	
	override void OnInit(IEntity owner)
	{
		
		super.OnInit(owner);
		
		
		
		//debugShapesBDR = new array<ref Shape>;
		
		
		ChimeraCharacter character = GetCharacter();
		if (!character)
			return;
		
		if (!m_characterControllerComponent)
			m_characterControllerComponent = CharacterControllerComponent.Cast(character.FindComponent(CharacterControllerComponent));
		if (!m_characterDamageManagerComponent)
			m_characterDamageManagerComponent = SCR_CharacterDamageManagerComponent.Cast(character.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!timer)
			timer = new BDR_Timer();
		
		lowerBodyBones.Insert(CharacterBones.LCALF);
		lowerBodyBones.Insert(CharacterBones.RCALF);
		lowerBodyBones.Insert(CharacterBones.RFOOT);
		lowerBodyBones.Insert(CharacterBones.LFOOT);
		
		lowerBodyBonesAndSpine.InsertAll(lowerBodyBones);
		lowerBodyBonesAndSpine.Insert(CharacterBones.SPINE);
		

	}
	

	override void OnDeath(IEntity instigator)
	{
		//Print("BDR: OnDeath");

		/* Settings initialization stuff */
		MCF_SettingsManager mcfSettingsManager = MCF_SettingsManager.GetInstance();
		const string bdrFileNameJson = "BDR_Settings.json";
		const string BDR_MOD_ID = "596CE5149F3F702A";				//it's probably possible to get this in a better way but ok
		OrderedVariablesMap variablesMap = new OrderedVariablesMap();
			
			
		variablesMap.Set("defaultMainDamping", new VariableInfo("Default Base Damping", "0.00000001", EFilterType.TYPE_FLOAT));
		variablesMap.Set("defaultSecondaryDamping", new VariableInfo("Default Secondary Damping", "0.1", EFilterType.TYPE_FLOAT));
		variablesMap.Set("modifiedSecondaryDamping", new VariableInfo("Modified Secondary Damping", "1.0", EFilterType.TYPE_FLOAT));
		variablesMap.Set("modifiedSecondaryDampingWhileMoving", new VariableInfo("Modified Secondary Damping While Moving", "0.75", EFilterType.TYPE_FLOAT));
		variablesMap.Set("modifiedMassFastDeath", new VariableInfo("Modified Mass with headshots", "0.297619", EFilterType.TYPE_FLOAT));
		variablesMap.Set("activateHitImpact", new VariableInfo("Activate Physical Hit on Impact", "1", EFilterType.TYPE_BOOL));		
		
		
		
		if (!mcfSettingsManager.GetJsonManager(BDR_MOD_ID))
		{
			#ifdef DEBUG_MCF
			Print("BDR: Preparing MCF");
			#endif
			bdrSettings = mcfSettingsManager.Setup(BDR_MOD_ID, bdrFileNameJson, variablesMap);
		}
		else if (!bdrSettings)
		{
			#ifdef DEBUG_MCF
			Print("BDR: Loading settings");
			#endif
			bdrSettings = mcfSettingsManager.GetModSettings(BDR_MOD_ID);
			mcfSettingsManager.GetJsonManager(BDR_MOD_ID).SetUserHelpers(variablesMap);		//slow and inefficient but it works for now
		}

		
		defaultMainDamping = bdrSettings.Get("defaultMainDamping").ToFloat();
		defaultSecondaryDamping = bdrSettings.Get("defaultSecondaryDamping").ToFloat();
		defaultSecondaryDamping = bdrSettings.Get("modifiedSecondaryDamping").ToFloat();
		modifiedSecondaryDamping = bdrSettings.Get("modifiedSecondaryDampingWhileMoving").ToFloat();
		modifiedMassFastDeath = bdrSettings.Get("modifiedMassFastDeath").ToFloat();
		activateHitImpact = bdrSettings.Get("activateHitImpact").ToInt();
		#ifdef DEBUG_MCF
		Print("BDR: Loaded Settings");
		#endif
		
		Rpc(RpcAsk_MainMethod);
		super.OnDeath(instigator);

		
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
			float dampingToSet = modifiedSecondaryDamping - BDR_Functions_Generic.Lerp(0.0, 1 - defaultSecondaryDamping, dampingDuration, deltaTime);
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
	vector endRiskyPosition;
	bool TestPosition(notnull IEntity ent)
	{
		

		
		isCharacterInAcceptablePosition = (ent.ClassName() != "SCR_DestructibleBuildingEntity") && (ent.ClassName() != "GenericEntity");
		
		
		
		
		if (!isCharacterInAcceptablePosition)
		{
			
			endRiskyPosition = ent.GetOrigin();		//wont work.
			return false;

		}
		
		return true;
		

	}
	
	
	
	
	
	void ManageDamping(array<CharacterBones> bonesArray, float newDamping)
	{
		foreach(CharacterBones x: bonesArray)
			currentRagdoll.GetBoneRigidBody(x).SetDamping(newDamping, newDamping);
		
	}
	
	
	
	
	
	// NETWORK STUFF 

	
	
	
	//
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_MainMethod()
	{
		array<int> playerIds = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		
		
		if (playerIds.Count() > 1)
			Rpc(RpcAsk_BroadcastMethod);
		else
			ManageRagdoll();
		//ManageRagdoll();


	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_BroadcastMethod()
	{
		ManageRagdoll();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcAsk_OwnerMethod()
	{
		Print("Owner method. Will run only on owner");
		SCR_PlayerController t1 = SCR_PlayerController.Cast(GetGame().GetPlayerController());

		CharacterControllerComponent t2 = CharacterControllerComponent.Cast(t1.GetControlledEntity().FindComponent(CharacterControllerComponent));
		ManageRagdoll();

	}

	
	void ManageRagdoll()
	{
			
		// Get the player stuff. We'll do it here 'cause we can't rely on OnInit since it could have changed. 
		SCR_PlayerController m_playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		
		if (!m_playerController)
			return;			//maybe it'll help?
		
		// how the fuck do we check if it's auth or not 
		 
		CharacterControllerComponent m_playerCharacterControllerComponent = CharacterControllerComponent.Cast(m_playerController.GetControlledEntity().FindComponent(CharacterControllerComponent));

		
		// Players won't receive it for various reasons. At least for now.
		if (m_playerCharacterControllerComponent != m_characterControllerComponent)
		{

			// Get Last Hit
			
					
			/* We need to consider moving characters, so let's just sum those two vectors and then manage it later.
			Consider that damping must not be as aggressive as when the character is not moving.*/
			vector movementVelocity = m_characterControllerComponent.GetMovementVelocity();
			
			
			array<vector> lastHitArray = m_characterDamageManagerComponent.GetLastHitCoordinates();
			vector lastHitDirection = {lastHitArray[1][0], lastHitArray[1][1], lastHitArray[1][2]};		// for some reason I can't assign a vec to a vec so whatever
			vector hitVector = {lastHitDirection[0], lastHitDirection[1]/5, lastHitDirection[2]};		// y stays the same since we want a little more oomph
			vector hitPosition = {lastHitArray[0][0], lastHitArray[0][1], lastHitArray[0][2]};
			HitZone hitZone = m_characterDamageManagerComponent.GetLastHitZone();
		
			vector hitToApply;
			string hitZoneName;

			if (activateHitImpact)
			{


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
			
			
			}
			else
			{
				hitToApply = {0,0,0};
				hitZoneName = "";
			}
			
			
			
			
	
			


			
			/* Preventing feet to clip in the ground. Still a pretty jank solution but whatever*/
			
			// this will work ONLY for terrains, not objects. and for a good reason
			GetGame().GetWorld().QueryEntitiesBySphere(GetCharacter().GetOrigin(), 0.1, TestPosition, null, EQueryEntitiesFlags.STATIC);
			if (!isCharacterInAcceptablePosition)
			{
				Print("Character in a risky position");
				vector characterOrigin = GetCharacter().GetOrigin();
				vector positionRiskyObject = endRiskyPosition;			//not necessary.
				
				
				float differenceY = Math.AbsFloat(characterOrigin[1] - positionRiskyObject[1]);
				float safetyY = 0.05;		
				if( differenceY >= 10)		//this is totally wrong but hey it works better than before :) 
				{
					Print("Fixing char position to compensate");
					vector matrixTransform[4];
					GetCharacter().GetTransform(matrixTransform);
					matrixTransform[3] = Vector(matrixTransform[3][0], matrixTransform[3][1] + safetyY, matrixTransform[3][2]);
					GetCharacter().SetTransform(matrixTransform);
	
				}
				
				
				//float surfWorldY = GetGame().GetWorld().GetSurfaceY(characterOrigin[0], characterOrigin[2]);
	
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
							currentRagdoll.GetBoneRigidBody(i).SetDamping(modifiedSecondaryDamping, modifiedSecondaryDamping);
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
			//BaseContainer baseContainer = GetComponentSource(GetOwner());
			
			//float currentDeathTimer = 20;
			//while (currentDeathTimer > 15)
			//{
			//	baseContainer.Set("DeathTimer", 2.0);
			//	baseContainer.Set("DeathTimer", 2.0);
			//	baseContainer.Set("DeathTimer", 2.0);
			//	baseContainer.Get("DeathTimer", currentDeathTimer);
			//	Print(currentDeathTimer);

		//	}

			
			m_playerCharacterControllerComponent.ForceDeath();
			SCR_CharacterCommandHandlerComponent characterCommandHandlerComponent = BDR_Functions_Generic.FindCommandHandler(GetCharacter());
			CharacterInputContext m_characterInputContext = m_playerCharacterControllerComponent.GetInputContext();
			float dyingDirection = m_characterInputContext.GetDie();
				
			if (dyingDirection != 0.0)
				characterCommandHandlerComponent.StartCommand_Death(dyingDirection);
			


		}
		
		
	}
	
	
	

	
	
	
}
