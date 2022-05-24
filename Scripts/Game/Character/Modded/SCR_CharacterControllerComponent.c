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
		
		
		
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		CharacterControllerComponent cc = CharacterControllerComponent.Cast(pc.GetControlledEntity().FindComponent(CharacterControllerComponent));
		CharacterAnimationComponent cc_ac = CharacterAnimationComponent.Cast(cc.FindComponent(CharacterAnimationComponent));		
		CharacterControllerComponent test_CC = CharacterControllerComponent.Cast(GetCharacter().FindComponent(CharacterControllerComponent));
		CharacterInputContext cic = test_CC.GetInputContext();
		
		//Probably slow as fuck, I don't care right now
		
		if (pc.GetControlledEntity().FindComponent(CharacterControllerComponent) != test_CC){
			//having this seems to force the commandid to shut off before we can actually use it
			
			test_CC.Ragdoll();
			SCR_CharacterCommandHandlerComponent cchc = FindCommandHandler(instigator);
			cchc.HandleDeath(cic, 0, 26, true);
			
			
			
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
