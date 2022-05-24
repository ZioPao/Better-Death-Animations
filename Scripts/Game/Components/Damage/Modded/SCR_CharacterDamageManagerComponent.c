modded class SCR_CharacterDamageManagerComponent : ScriptedDamageManagerComponent{

	// we need to have some references to some aspects 
	
	private vector last_hit[3];
	
	
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
		last_hit = hitTransform;
		
		UpdateBloodyFace();
	}

	
	array<vector> GetLastHit(){
		
		// i know it's shit, i dunno how to return an array right now so yeah.
		array<vector> temp = {};
		
		temp.Insert(last_hit[0]);
		temp.Insert(last_hit[1]);
		temp.Insert(last_hit[2]);
		return temp;
	
	}
		
}