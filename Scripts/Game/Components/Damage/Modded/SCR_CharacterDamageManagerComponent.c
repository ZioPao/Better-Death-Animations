modded class SCR_CharacterDamageManagerComponent : ScriptedDamageManagerComponent{

	// we need to have some references to some aspects 
	
	private vector lastHit[3];
	private string hitZoneName;
	
	
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
		lastHit = hitTransform;
		hitZoneName = pHitZone.GetName();
		
		UpdateBloodyFace();
	}

	
	array<vector> GetLastHit(){
		
		// i know it's shit, i dunno how to return an array right now so yeah.
		array<vector> temp = {};
		
		temp.Insert(lastHit[0]);
		temp.Insert(lastHit[1]);
		temp.Insert(lastHit[2]);
		return temp;
	
	}
	
	string GetHitZoneName(){
		return hitZoneName;
	}
		
}