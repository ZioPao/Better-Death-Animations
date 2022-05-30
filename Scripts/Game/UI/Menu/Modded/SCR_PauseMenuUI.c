
//------------------------------------------------------------------------------------------------
modded class PauseMenuUI: ChimeraMenuBase
{
	
	protected SCR_ButtonTextComponent m_SettingsBetterDeathReactions;


//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_SaveLoadComponent = SCR_SaveLoadComponent.GetInstance();

		m_wRoot = GetRootWidget();
		m_wFade = m_wRoot.FindAnyWidget("BackgroundFade");
		m_wSystemTime = m_wRoot.FindAnyWidget("SystemTime");
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		SCR_ButtonTextComponent comp;

		// Continue
		comp = SCR_ButtonTextComponent.GetButtonText("Continue", m_wRoot);
		if (comp)
		{
			GetGame().GetWorkspace().SetFocusedWidget(comp.GetRootWidget());
			comp.m_OnClicked.Insert(Close);
		}

		// Restart
		comp = SCR_ButtonTextComponent.GetButtonText("Restart", m_wRoot);
		if (comp)
		{
			bool enabledRestart = !Replication.IsRunning();
			comp.GetRootWidget().SetVisible(enabledRestart);
			comp.m_OnClicked.Insert(OnRestart);
		}

		// Respawn
		comp = SCR_ButtonTextComponent.GetButtonText("Respawn", m_wRoot);
		if (comp)
		{
			bool canRespawn;
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				RespawnSystemComponent respawn = RespawnSystemComponent.Cast(gameMode.FindComponent(RespawnSystemComponent));
				canRespawn = (respawn != null);
			}

			comp.GetRootWidget().SetVisible(canRespawn);
			comp.m_OnClicked.Insert(OnRespawn);
		}

		// Exit
		comp = SCR_ButtonTextComponent.GetButtonText("Exit", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnExit);
			if (IsSavingOnExit())
				comp.SetText(EXIT_SAVE);
			else
				comp.SetText(EXIT_NO_SAVE);
		}
		
		// Save
		comp = SCR_ButtonTextComponent.GetButtonText("Save", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnSave);
			comp.GetRootWidget().SetVisible(CanSave());
			comp.SetEnabled(IsSaveAvailable());
		}

		// Load
		comp = SCR_ButtonTextComponent.GetButtonText("Load", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnLoad);
			comp.GetRootWidget().SetVisible(CanLoad());
			comp.SetEnabled(IsLoadAvailable());
		}
		
		// Camera
		comp = SCR_ButtonTextComponent.GetButtonText("Camera", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnCamera);
			comp.GetRootWidget().SetEnabled(editorManager && !editorManager.IsOpened());
			comp.GetRootWidget().SetVisible(Game.IsDev());
		}

		// Settings
		comp = SCR_ButtonTextComponent.GetButtonText("Settings", m_wRoot);
		if (comp)
			comp.m_OnClicked.Insert(OnSettings);

		// Field Manual
		comp = SCR_ButtonTextComponent.GetButtonText("FieldManual", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnFieldManual);
		}

		// Players
		comp = SCR_ButtonTextComponent.GetButtonText("Players", m_wRoot);
		if (comp)
			comp.m_OnClicked.Insert(OnPlayers);

		// Version
		m_wVersion = TextWidget.Cast(m_wRoot.FindAnyWidget("Version"));
		if (m_wVersion)
			m_wVersion.SetText(GetGame().GetBuildVersion());

		// Unlimited editor (Game Master)
		m_EditorUnlimitedOpenButton = SCR_ButtonTextComponent.GetButtonText("EditorUnlimitedOpen",m_wRoot);
		if (m_EditorUnlimitedOpenButton)		
			m_EditorUnlimitedOpenButton.m_OnClicked.Insert(OnEditorUnlimited);

		m_EditorUnlimitedCloseButton = SCR_ButtonTextComponent.GetButtonText("EditorUnlimitedClose",m_wRoot);
		if (m_EditorUnlimitedCloseButton)		
			m_EditorUnlimitedCloseButton.m_OnClicked.Insert(OnEditorUnlimited);
		
		//--- Photo mode
		m_EditorPhotoOpenButton = SCR_ButtonTextComponent.GetButtonText("EditorPhotoOpen",m_wRoot);
		if (m_EditorPhotoOpenButton)
			m_EditorPhotoOpenButton.m_OnClicked.Insert(OnEditorPhoto);
		m_EditorPhotoCloseButton = SCR_ButtonTextComponent.GetButtonText("EditorPhotoClose",m_wRoot);
		if (m_EditorPhotoCloseButton)
			m_EditorPhotoCloseButton.m_OnClicked.Insert(OnEditorPhoto);
		
		
		
		// BDR set
		comp = SCR_ButtonTextComponent.GetButtonText("BDRSettings", m_wRoot);
		if (comp)
			comp.m_OnClicked.Insert(OnBDRSettings);
		
		SetEditorUnlimitedButton(editorManager);
		SetEditorPhotoButton(editorManager);
		
		if (editorManager)
		{
			editorManager.GetOnModeAdd().Insert(OnEditorModeChanged);
			editorManager.GetOnModeRemove().Insert(OnEditorModeChanged);
		}

		comp = SCR_ButtonTextComponent.GetButtonText("Feedback", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnFeedback);
		}

		m_InputManager = GetGame().GetInputManager();
		
		SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
		if (hud)
			hud.SetVisible(false);
		
		m_OnPauseMenuOpened.Invoke();
	}

	
	private void OnBDRSettings()
	{
		
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.BDRSettings);



	}

};



