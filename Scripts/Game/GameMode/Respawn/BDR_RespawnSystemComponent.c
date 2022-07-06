modded class SCR_RespawnSystemComponent : RespawnSystemComponent
{

	//------------------------------------------------------------------------------------------------
	// Called when a spawn is requested
	// Asks the gamemode with PickPlayerSpawnPoint query expecting to get a spawn point
	// at which the player should be spawned
	protected override GenericEntity RequestSpawn(int playerId)
	{
		// Catch illicit requests,
		// TODO@AS:
		// TODO@LK:
		// We should probably make it so RequestRespawn()
		// is not even called from client if !CanPlayerRespawn(playerId)
		// and only resort to this as a safety measure
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode && !gameMode.CanPlayerRespawn(playerId) && !m_bCustomRespawn)
		{
			Print("Requested spawn denied! GameMode returned false in CanPlayerRespawn() for playerId=" + playerId, LogLevel.WARNING);
			return null;
		}
		
		if (m_bCustomRespawn)
		{
			m_CustomSpawnedEntity = DoSpawn(m_sCustomRespawnPrefab, m_vCustomRespawnPos, m_vCustomRespawnRot);
			if (!m_CustomSpawnedEntity)
				return null;

			FactionAffiliationComponent affiliationComp = FactionAffiliationComponent.Cast(m_CustomSpawnedEntity.FindComponent(FactionAffiliationComponent));
			if (affiliationComp)
			{
				Faction faction = affiliationComp.GetAffiliatedFaction();
				if (faction)
					DoSetPlayerFaction(playerId, GetFactionIndex(faction));
			}

			return m_CustomSpawnedEntity;
		}

		SCR_BasePlayerLoadout loadout = GetPlayerLoadout(playerId);
		if (!loadout)
		{
			Print(LOG_HEAD+" No valid entity to spawn could be returned in RequestSpawn. Are there valid loadouts for the target player faction?", LogLevel.ERROR);
			return null;
		}

		SCR_SpawnPoint spawnPoint = GetPlayerSpawnPoint(playerId);
		if (!spawnPoint)
		{
			Print(LOG_HEAD+" No valid spawn point available in RequestSpawn. Player will not spawn!", LogLevel.ERROR);
			return null;
		}
		
		SCR_PlayerSpawnPoint playerSpawnPoint = SCR_PlayerSpawnPoint.Cast(spawnPoint);
		if (playerSpawnPoint)
		{
			if (!CanSpawnOnPlayerSpawnPoint(playerSpawnPoint))
			{
				string playerFactionKey;
				Faction faction = GetPlayerFaction(playerId);
				if (faction)
					playerFactionKey = faction.GetFactionKey();
				
				SCR_SpawnPoint nearestSpawnPoint = FindNearestAvailableSpawnPoint(playerSpawnPoint.GetOrigin(), playerFactionKey, playerId);
				if (nearestSpawnPoint)
					spawnPoint = nearestSpawnPoint;
			}
		}
		
		vector spawnPosition = vector.Zero;
		vector spawnRotation = vector.Zero;
		if (spawnPoint)
		{
			spawnPoint.GetPositionAndRotation(spawnPosition, spawnRotation);
			m_pGameMode.OnSpawnPointUsed(spawnPoint, playerId);
		}

		GenericEntity spawned = DoSpawn(loadout.GetLoadoutResource(), spawnPosition, spawnRotation);
		loadout.OnLoadoutSpawned(spawned, playerId);
		if (spawnPoint)
			spawnPoint.EOnPlayerSpawn(spawned);
		
		return spawned;
	}
	
	GenericEntity ForceSpawn(int playerId)
	{
// Catch illicit requests,
		// TODO@AS:
		// TODO@LK:
		// We should probably make it so RequestRespawn()
		// is not even called from client if !CanPlayerRespawn(playerId)
		// and only resort to this as a safety measure
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			Print("Requested spawn denied! GameMode returned false in CanPlayerRespawn() for playerId=" + playerId, LogLevel.WARNING);
			return null;
		}
		

		SCR_BasePlayerLoadout loadout = GetPlayerLoadout(playerId);
		if (!loadout)
		{
			Print(LOG_HEAD+" No valid entity to spawn could be returned in RequestSpawn. Are there valid loadouts for the target player faction?", LogLevel.ERROR);
			return null;
		}

		SCR_SpawnPoint spawnPoint = GetPlayerSpawnPoint(playerId);
		if (!spawnPoint)
		{
			Print(LOG_HEAD+" No valid spawn point available in RequestSpawn. Player will not spawn!", LogLevel.ERROR);
			return null;
		}
		
		SCR_PlayerSpawnPoint playerSpawnPoint = SCR_PlayerSpawnPoint.Cast(spawnPoint);
		if (playerSpawnPoint)
		{
			if (!CanSpawnOnPlayerSpawnPoint(playerSpawnPoint))
			{
				string playerFactionKey;
				Faction faction = GetPlayerFaction(playerId);
				if (faction)
					playerFactionKey = faction.GetFactionKey();
				
				SCR_SpawnPoint nearestSpawnPoint = FindNearestAvailableSpawnPoint(playerSpawnPoint.GetOrigin(), playerFactionKey, playerId);
				if (nearestSpawnPoint)
					spawnPoint = nearestSpawnPoint;
			}
		}
		
		vector spawnPosition = vector.Zero;
		vector spawnRotation = vector.Zero;
		if (spawnPoint)
		{
			spawnPoint.GetPositionAndRotation(spawnPosition, spawnRotation);
			m_pGameMode.OnSpawnPointUsed(spawnPoint, playerId);
		}

		GenericEntity spawned = DoSpawn(loadout.GetLoadoutResource(), spawnPosition, spawnRotation);
		loadout.OnLoadoutSpawned(spawned, playerId);
		if (spawnPoint)
			spawnPoint.EOnPlayerSpawn(spawned);
		
		return spawned;
	}
	
	
};