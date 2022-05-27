modded class SCR_CharacterDamageManagerComponent : ScriptedDamageManagerComponent{

	// we need to have some references to some aspects 
	
	private vector lastHitCoordinates[3];
	private HitZone lastHitZone;
	private int lastColliderID;
	
	//-----------------------------------------------------------------------------------------------------------
	protected override void OnDamage(
				EDamageType type,
				float damage,
				HitZone pHitZone,
				IEntity instigator, 
				inout vector hitTransform[3], 
				float speed,
				int colliderID, 
				int nodeID)
	{
		super.OnDamage(type, damage, pHitZone, instigator, hitTransform, speed, colliderID, nodeID);
		lastHitCoordinates = hitTransform;
		lastHitZone = pHitZone;
		
		UpdateBloodyFace();
	}

	
	array<vector> GetLastHitCoordinates(){
		
		// i know it's shit, i dunno how to return an array right now so yeah.
		array<vector> temp = {};
		
		temp.Insert(lastHitCoordinates[0]);
		temp.Insert(lastHitCoordinates[1]);
		temp.Insert(lastHitCoordinates[2]);
		return temp;
	
	}
	
	HitZone GetLastHitZone(){
		return lastHitZone;
	
	}
	
	int GetLastColliderID(){
		return lastColliderID;
	}

}