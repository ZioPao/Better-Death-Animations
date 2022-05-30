class BDR_SettingsMenu : DialogUI
{

	SCR_EditBoxComponent dampingValue;
	SCR_EditBoxComponent test;
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		Widget dampingWidget = GetRootWidget().FindAnyWidget("DampingWidget");
		dampingValue = SCR_EditBoxComponent.Cast(dampingWidget.FindHandler(SCR_EditBoxComponent));

	}
	
	
	override void OnConfirm()
	{
		Print("Save stuff somewhere");
		super.OnConfirm();
	}

	override void OnCancel()
	{
		Print("Don't save");
		super.OnCancel();
	}
}