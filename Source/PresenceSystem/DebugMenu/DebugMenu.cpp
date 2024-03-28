#include "DebugMenu.h"

//#include "Pantheon/Common/PantheonCommon.h"

#include "Blueprint/WidgetTree.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

#include "Engine/Canvas.h"
#include "Engine/GameInstance.h"

#include "GameFramework/PlayerInput.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char const* const DEBUG_MENU_CANVAS =				"DEBUG_MENU_CANVAS";
char const* const DEBUG_MENU_GRAB =					"DEBUG_MENU_GRAB";
char const* const DEBUG_MENU_TITLE =				"DEBUG_MENU_TITLE";
char const* const DEBUG_MENU_CONTAINER =			"DEBUG_MENU_CONTAINER";

char const* const DEBUG_MENU_NAME_SLIDER_SLIDER =	"DEBUG_MENU_SLIDER_SLIDER";
char const* const DEBUG_MENU_NAME_SLIDER_TITLE =	"DEBUG_MENU_SLIDER_TITLE";
char const* const DEBUG_MENU_NAME_SLIDER_VALUE =	"DEBUG_MENU_SLIDER_VALUE";

char const* const DEBUG_MENU_BUTTON_BUTTON =		"DEBUG_MENU_BUTTON_BUTTON";
char const* const DEBUG_MENU_BUTTON_TITLE =			"DEBUG_MENU_BUTTON_TITLE";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int32 LastZOrderAndInputPriority = 1;

bool IsServerCode(UWorld* World)
{
	ENetMode NetMode = World->GetNetMode();
	if (NetMode != NM_Client)
		return true;

	return false;
}
bool IsClientCode(UWorld* World)
{
	ENetMode NetMode = World->GetNetMode();
	if (NetMode == NM_Client || NetMode == NM_Standalone || NetMode == NM_ListenServer)
		return true;

	return false;
}
bool IsDedicatedServerCode(UWorld* World)
{
	ENetMode NetMode = World->GetNetMode();
	if (NetMode == NM_DedicatedServer)
		return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UGameInstanceSubsystem_DebugMenu
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UPantheonGenericDebugMenuSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (this == nullptr)
	{
		check(false);
		return false;
	}
	if(GetClass()->IsInBlueprint() == true)
	{
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::InitializeGenericDebugMenuSubsystem(APlayerController* OwningPlayerController)
{
	const ENetMode NetMode = GetWorld()->GetNetMode();

	// Create replicating actor on the server
	if (NetMode == ENetMode::NM_DedicatedServer)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = OwningPlayerController;
		//SpawnParameters.Name = FName(FString::Printf(TEXT("GameInstanceSubsystem_DebugMenu_ReplicatingActor_%s"), *OwningPlayerController->GetName()));
		ADebugMenu_ReplicatingActor* NewReplicatingActor = GetWorld()->SpawnActor<ADebugMenu_ReplicatingActor>(ADebugMenu_ReplicatingActor::StaticClass(), SpawnParameters);
		ReplicatingActorsOnServer.Add(FDebugMenuReplicatingActor_ArrayHolder(OwningPlayerController, NewReplicatingActor));

		for (int i = 0; i < StoredSlidersInfos.Num(); ++i)
		{
			FName const& Name = StoredSlidersInfos[i].SliderName;
			const float CurrentValue = StoredSlidersInfos[i].CurrentValue;
			NewReplicatingActor->ClientRPC_ChangeSliderValue(Name, CurrentValue);
		}
	}
	else
	{
		cmd_DisplayDebugMenu = IConsoleManager::Get().RegisterConsoleCommand(TEXT("pan.debug_menu.display"),
		TEXT(""), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UPantheonGenericDebugMenuSubsystem::Exec_DisplayDebugMenu),ECVF_Default);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::UninitializeGenericDebugMenuSubsystem(APlayerController* OwningPlayerController)
{
	for (int i = 0; i < ReplicatingActorsOnServer.Num(); ++i)
	{
		if (ReplicatingActorsOnServer[i].OwningPlayerController == OwningPlayerController)
		{
			ReplicatingActorsOnServer[i].ReplicatingActor->Destroy();
			ReplicatingActorsOnServer[i].ReplicatingActor = nullptr;
			ReplicatingActorsOnServer.RemoveAt(i);
			break;
		}
	}
	
	if (ReplicatingActorOnClient)
	{
		ReplicatingActorOnClient->Destroy();
		ReplicatingActorOnClient = nullptr;
	}
	if (cmd_DisplayDebugMenu)
	{
		IConsoleManager::Get().UnregisterConsoleObject(cmd_DisplayDebugMenu);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::CreateDebugMenu(FName const& DebugMenuName, FName const& PresetName, FVector2d InitialPosition, bool IsDisplayedByDefault)
{
	// TODO Julien Rogel (21/03/2024): Check if this menu already exist
	
	// Create UI widgets on the client
	const ENetMode NetMode = GetWorld()->GetNetMode();
	if (NetMode == ENetMode::NM_DedicatedServer)
	{
		return;
	}

	const TSubclassOf<UDebugMenuUserWidget>* DebugMenuClass = DebugMenuClassPresets.Find(PresetName);
	if (DebugMenuClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Faield to find DebugMenuClassPreset: %s"), *PresetName.ToString());
		return;
	}
	
	UDebugMenuUserWidget* DebugMenuUserWidget = CreateWidget<UDebugMenuUserWidget>(GetWorld()->GetFirstPlayerController(), *DebugMenuClass);
	if (DebugMenuUserWidget == nullptr)
	{
		return;
	}
	
	LastZOrderAndInputPriority++;
	
	DebugMenuUserWidget->AddToViewport(LastZOrderAndInputPriority);
	DebugMenuUserWidget->InitializeDebugMenuWidget(DebugMenuName);
	DebugMenuUserWidget->SetPositionInViewport(InitialPosition, true);

	DebugMenus.Add(FDebugMenuWidget_ArrayHolder(DebugMenuName, DebugMenuUserWidget));
	
	if (IsDisplayedByDefault == false)
	{
		Internal_SetDebugMenuVisibility(DebugMenuName, false);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::DestroyDebugMenu(FName const& DebugMenuName)
{
	for (int i = StoredSlidersInfos.Num() - 1; i >= 0 ; --i)
	{
		FPanDebugMenuSliderInfo* CurrentSliderInfo = &StoredSlidersInfos[i];
		if (CurrentSliderInfo->DebugMenuName == DebugMenuName)
		{
			if (IsValid(CurrentSliderInfo->SliderWidget))
			{
				CurrentSliderInfo->SliderWidget->RemoveFromParent();
			}
			CurrentSliderInfo->SliderWidget = nullptr;
			CurrentSliderInfo->OnDebugMenuSliderChangedValue.Unbind();
			StoredSlidersInfos.RemoveAt(i);
		}
	}
	
	for (int i = StoredButtonsInfos.Num() - 1; i >= 0 ; --i)
	{
		FPanDebugMenuButtonInfo* CurrentMenuButtonInfo = &StoredButtonsInfos[i];
		if (CurrentMenuButtonInfo->DebugMenuName == DebugMenuName)
		{
			if (IsValid(CurrentMenuButtonInfo->ButtonWidget))
			{
				CurrentMenuButtonInfo->ButtonWidget->RemoveFromParent();
			}
			CurrentMenuButtonInfo->ButtonWidget = nullptr;
			CurrentMenuButtonInfo->OnButtonPressed.Unbind();
			StoredButtonsInfos.RemoveAt(i);
		}
	}
	
	FDebugMenuWidget_ArrayHolder* MatchingDebugMenu = nullptr;
	for (int i = 0; i < DebugMenus.Num(); ++i)
	{
		MatchingDebugMenu = &DebugMenus[i];
		if (MatchingDebugMenu->Name == DebugMenuName)
		{
			MatchingDebugMenu->Widget->RemoveFromParent();
			MatchingDebugMenu->Widget = nullptr;
			MatchingDebugMenu->Name = "Invalid";
			DebugMenus.RemoveAt(i);

			return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::AddSliderToDebugMenu(FName const& DebugMenuName, FName const& PresetName,
	FPanDebugMenuSliderParameters const& DebugMenuSliderParameters, TFunction<void(float)> Lambda)
{
	if (CheckIfSliderAlreadyExist(DebugMenuSliderParameters.SliderName) == true)
		return;

	if (IsDedicatedServerCode(GetWorld()))
	{
		if (DebugMenuSliderParameters.ShouldReplicate == false)
			return;
		
		for (int i = 0; i < ReplicatingActorsOnServer.Num(); ++i)
		{
			FName const& Name = DebugMenuSliderParameters.SliderName;
			const float CurrentValue = DebugMenuSliderParameters.DefaultValue;
			ReplicatingActorsOnServer[i].ReplicatingActor->ClientRPC_ChangeSliderValue(Name, CurrentValue);
		}
	}
	
	const FPanDebugMenuSliderInfo NewSliderInfo = FPanDebugMenuSliderInfo(DebugMenuSliderParameters);
	StoredSlidersInfos.Add(NewSliderInfo);
	FPanDebugMenuSliderInfo* NewSliderInfoPtr = &StoredSlidersInfos[StoredSlidersInfos.Num() - 1];
	NewSliderInfoPtr->DebugMenuName = DebugMenuName;
	
	if (IsClientCode(GetWorld()))
	{
		UDebugMenuUserWidget* DebugMenuUserWidget = nullptr;
		for (int i = 0; i < DebugMenus.Num(); ++i)
		{
			if (DebugMenus[i].Name == DebugMenuName)
			{
				DebugMenuUserWidget = DebugMenus[i].Widget;
				break;
			}
		}
		if (DebugMenuUserWidget == nullptr)
			return;

		const TSubclassOf<UDebugMenu_SliderWidget>* SliderClass = SliderClassPresets.Find(PresetName);
		if (SliderClass == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to find SliderClassPreset: %s"), *PresetName.ToString());
			return;
		}
		
		UDebugMenu_SliderWidget* NewSliderWidget = DebugMenuUserWidget->AddSliderToDebugMenuWidget(DebugMenuSliderParameters, *SliderClass);
		NewSliderInfoPtr->SliderWidget = NewSliderWidget;
	}

	NewSliderInfoPtr->OnDebugMenuSliderChangedValue.BindLambda(Lambda);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::AddButtonToDebugMenu(FName const& DebugMenuName, FName const& PresetName,
	FPanDebugMenuButtonParameters const& DebugMenuButtonParameters, TFunction<void()> Lambda)
{
	if (CheckIfButtonAlreadyExist(DebugMenuButtonParameters.ButtonName) == true)
		return;

	if (IsDedicatedServerCode(GetWorld()))
	{
		if (DebugMenuButtonParameters.ShouldReplicate == false)
			return;
	}

	const FPanDebugMenuButtonInfo NewButtonInfo = FPanDebugMenuButtonInfo(DebugMenuButtonParameters);
	const int IndexNewButton = StoredButtonsInfos.Add(NewButtonInfo);
	FPanDebugMenuButtonInfo* NewButtonInfoPtr = &StoredButtonsInfos[IndexNewButton];
	NewButtonInfoPtr->DebugMenuName = FName(DebugMenuName);
	
	if (IsClientCode(GetWorld()))
	{
		FDebugMenuWidget_ArrayHolder* DebugMenuWidget_ArrayHolder = GetDebugMenuFromName(DebugMenuName);
		if (DebugMenuWidget_ArrayHolder == nullptr)
			return;
		
		UDebugMenuUserWidget* DebugMenuUserWidget = DebugMenuWidget_ArrayHolder->Widget;
		if (DebugMenuUserWidget == nullptr)
			return;
			
		const TSubclassOf<UDebugMenu_ButtonWidget>* ButtonClass = ButtonClassPresets.Find(PresetName);
		if (ButtonClass == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to find ButtonClassPreset: %s"), *PresetName.ToString());
			return;
		}
		
		UDebugMenu_ButtonWidget* NewButtonWidget = DebugMenuUserWidget->AddButtonToDebugMenuWidget(DebugMenuButtonParameters, *ButtonClass);
		NewButtonInfoPtr->ButtonWidget = NewButtonWidget;
	}

	NewButtonInfoPtr->OnButtonPressed.BindLambda(Lambda);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void UPantheonGenericDebugMenuSubsystem::Internal_RegisterReplicatingActorToDebugMenuOnClient(ADebugMenu_ReplicatingActor* InReplicatingActor)
{
	ensureAlways(ReplicatingActorOnClient == nullptr);
	
	ReplicatingActorOnClient = InReplicatingActor;
	
	for (int i = 0; i < StoredSlidersInfos.Num(); ++i)
	{
		FPanDebugMenuSliderInfo const& SliderInfo = StoredSlidersInfos[i];
		if (SliderInfo.IsNeedingReplication() == true)
		{
			ReplicatingActorOnClient->ClientRPC_ChangeSliderValue(SliderInfo.SliderName, SliderInfo.CurrentValue);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::Internal_NotifyNewSliderValueOnClient(FName const& SliderName, float NewValue)
{
	FPanDebugMenuSliderInfo* SliderInfo = GetSliderInfoFromName(SliderName);
	
	if (SliderInfo->IsReplicated == true)
	{
		SliderInfo->IsWaitingForReplication = true;
		if (ReplicatingActorOnClient)
		{
			ReplicatingActorOnClient->ServerRPC_RequestChangeSliderValue(SliderName, NewValue);	
		}
	}
	else
	{
		SliderInfo->SliderWidget->SetCurrentSliderValue(NewValue);
		SliderInfo->SliderWidget->SetCurrentValueText(NewValue);
		SliderInfo->OnDebugMenuSliderChangedValue.Execute(NewValue);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::Internal_NotifyNewSliderValueOnServer(FName const& SliderName, float NewValue)
{
	const FPanDebugMenuSliderInfo* SliderInfo = GetSliderInfoFromName(SliderName);
	SliderInfo->OnDebugMenuSliderChangedValue.Execute(NewValue);

	for (int i = 0; i < ReplicatingActorsOnServer.Num(); ++i)
	{
		ReplicatingActorsOnServer[i].ReplicatingActor->ClientRPC_ChangeSliderValue(SliderName, NewValue);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::Internal_UpdateClientSliderValueFromServer(FName const& SliderName, float NewValue)
{
	FPanDebugMenuSliderInfo* SliderInfo = GetSliderInfoFromName(SliderName);
	if (SliderInfo == nullptr)
		return;
	
	SliderInfo->IsWaitingForReplication = false;

	if (IsValid(SliderInfo->SliderWidget))
	{
		SliderInfo->SliderWidget->SetCurrentValueText(NewValue);
	}
	SliderInfo->OnDebugMenuSliderChangedValue.Execute(NewValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::Internal_NotifyButtonPressedOnClient(FName const& ButtonName)
{
	FPanDebugMenuButtonInfo* ButtonInfo = GetButtonInfoFromName(ButtonName);
	if (ButtonInfo == nullptr)
		return;
	
	if (ButtonInfo->IsReplicated == true)
	{
		ButtonInfo->IsWaitingForReplication = true;
		if (ReplicatingActorOnClient)
		{
			ReplicatingActorOnClient->ServerRPC_RequestPressButton(ButtonName);	
		}
	}
	else
	{
		ButtonInfo->OnButtonPressed.Execute();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::Internal_NotifyButtonPressedOnServer(FName const& ButtonName)
{
	FPanDebugMenuButtonInfo* ButtonInfo = GetButtonInfoFromName(ButtonName);
	if (ButtonInfo == nullptr)
		return;
	
	ButtonInfo->OnButtonPressed.Execute();

	for (int i = 0; i < ReplicatingActorsOnServer.Num(); ++i)
	{
		ReplicatingActorsOnServer[i].ReplicatingActor->ClientRPC_PressButton(ButtonName);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::Internal_PressButtonFromServer(FName const& ButtonName)
{
	FPanDebugMenuButtonInfo* ButtonInfo = GetButtonInfoFromName(ButtonName);
	if (ButtonInfo == nullptr)
		return;
	
	ButtonInfo->OnButtonPressed.Execute();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::Internal_SetDebugMenuVisibility(FName const& DebugMenuName, bool Visibility)
{
	for (int i = 0; i < DebugMenus.Num(); ++i)
	{
		if (DebugMenus[i].Name == DebugMenuName)
		{
			const ESlateVisibility SlateVisibility = Visibility ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
			DebugMenus[i].Widget->SetVisibility(SlateVisibility);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UPantheonGenericDebugMenuSubsystem::Exec_DisplayDebugMenu(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		return;
	}
	
	const FString DebugMenuName = *Args[0];
	const int IsActivated = FCString::Atoi(*Args[1]);
	Internal_SetDebugMenuVisibility(FName(DebugMenuName), IsActivated > 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FDebugMenuWidget_ArrayHolder* UPantheonGenericDebugMenuSubsystem::GetDebugMenuFromName(FName const& DebugMenuName)
{
	FDebugMenuWidget_ArrayHolder* MatchingDebugMenu = nullptr;
	for (int i = 0; i < DebugMenus.Num(); ++i)
	{
		MatchingDebugMenu = &DebugMenus[i];
		if (MatchingDebugMenu->Name == DebugMenuName)
		{
			break;
		}
	}
	return MatchingDebugMenu;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FPanDebugMenuSliderInfo* UPantheonGenericDebugMenuSubsystem::GetSliderInfoFromName(FName const& SliderName)
{
	FPanDebugMenuSliderInfo* MatchingSlider = nullptr;
	for (int i = 0; i < StoredSlidersInfos.Num(); ++i)
	{
		MatchingSlider = &StoredSlidersInfos[i];
		if (MatchingSlider->SliderName == SliderName)
		{
			return MatchingSlider;
		}
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UPantheonGenericDebugMenuSubsystem::CheckIfSliderAlreadyExist(FName const& SliderName)
{
	for (int i = 0; i < StoredSlidersInfos.Num(); ++i)
	{
		if (StoredSlidersInfos[i].SliderName == SliderName)
		{
			return true;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FPanDebugMenuButtonInfo* UPantheonGenericDebugMenuSubsystem::GetButtonInfoFromName(FName const& ButtonName)
{
	FPanDebugMenuButtonInfo* MatchingButtonInfo = nullptr;
	for (int i = 0; i < StoredButtonsInfos.Num(); ++i)
	{
		MatchingButtonInfo = &StoredButtonsInfos[i];
		if (MatchingButtonInfo->ButtonName == ButtonName)
		{
			return MatchingButtonInfo;
		}
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UPantheonGenericDebugMenuSubsystem::CheckIfButtonAlreadyExist(FName const& ButtonName)
{
	for (int i = 0; i < StoredButtonsInfos.Num(); ++i)
	{
		if (StoredButtonsInfos[i].ButtonName == ButtonName)
		{
			return true;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UDebugMenu_SliderWidget
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UDebugMenu_SliderWidget::~UDebugMenu_SliderWidget()
{
	int test = 0;
}

void UDebugMenu_SliderWidget::InitializeSlider(FPanDebugMenuSliderParameters const& InDebugMenuSliderParameters)
{
	SliderName = InDebugMenuSliderParameters.SliderName;
	ValueUnitOfMeasurement = InDebugMenuSliderParameters.ValueUnitOfMeasurement;

	UWidget* DebugMenuSliderWidget = WidgetTree->FindWidget(DEBUG_MENU_NAME_SLIDER_SLIDER);
	SliderWidget = Cast<USlider>(DebugMenuSliderWidget);
	if (SliderWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No DebugMenuSliderWidget found, please check that you created a widget with this name: %hs"), DEBUG_MENU_NAME_SLIDER_SLIDER);
	}
	
	UWidget* DebugMenuNameTextWidget = WidgetTree->FindWidget(DEBUG_MENU_NAME_SLIDER_TITLE);
	TitleTextWidget = Cast<UTextBlock>(DebugMenuNameTextWidget);
	if (TitleTextWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No DebugMenuNameTextWidget found, please check that you created a widget with this name: %hs"), DEBUG_MENU_NAME_SLIDER_TITLE);
	}
	
	UWidget* DebugMenuValueTextWidget = WidgetTree->FindWidget(DEBUG_MENU_NAME_SLIDER_VALUE);
	ValueTextWidget = Cast<UTextBlock>(DebugMenuValueTextWidget);
	if (ValueTextWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No DebugMenuValueTextWidget found, please check that you created a widget with this name: %hs"), DEBUG_MENU_NAME_SLIDER_VALUE);
	}
	
	if (SliderWidget)
	{
		SliderWidget->SetMinValue(InDebugMenuSliderParameters.MinimumValue);
		SliderWidget->SetMaxValue(InDebugMenuSliderParameters.MaximumValue);
		SetCurrentSliderValue(InDebugMenuSliderParameters.DefaultValue);
		SliderWidget->OnValueChanged.AddDynamic(this, &UDebugMenu_SliderWidget::OnSliderChangedEvent);
	}
	if (ValueTextWidget)
	{
		SetCurrentValueText(InDebugMenuSliderParameters.DefaultValue);
	}
	if (TitleTextWidget)
	{
		const FText TextValue = FText::FromString(InDebugMenuSliderParameters.SliderName.ToString());
		TitleTextWidget->SetText(TextValue);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UDebugMenu_SliderWidget::SetCurrentSliderValue(float CurrentValue)
{
	if (SliderWidget == nullptr)
	{
		return;
	}
	SliderWidget->SetValue(CurrentValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UDebugMenu_SliderWidget::SetCurrentValueText(float CurrentValue)
{
	if (ValueTextWidget == nullptr)
	{
		return;
	}
	const FString StringValue = FString::Printf(TEXT("%3.0f %s"), CurrentValue, *ValueUnitOfMeasurement.ToString());
	const FText TextValue = FText::FromString(StringValue);
	ValueTextWidget->SetText(TextValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UDebugMenu_SliderWidget::OnSliderChangedEvent(float NewValue)
{
	UPantheonGenericDebugMenuSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	Subsystem->Internal_NotifyNewSliderValueOnClient(SliderName, NewValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UDebugMenu_ButtonWidget
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UDebugMenu_ButtonWidget::InitializeButton(FPanDebugMenuButtonParameters const& InDebugMenuButtonParameters)
{
	ButtonName = InDebugMenuButtonParameters.ButtonName;
	
	ButtonWidget = WidgetTree->FindWidget<UButton>(DEBUG_MENU_BUTTON_BUTTON);
	if (ButtonWidget)
	{
		ButtonWidget->OnPressed.AddDynamic(this, &UDebugMenu_ButtonWidget::OnButtonPressedEvent);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No ButtonWidget found, please check that you created a button widget with this name: %hs"), DEBUG_MENU_BUTTON_BUTTON);
	}

	TitleTextBlockWidget = WidgetTree->FindWidget<UTextBlock>(DEBUG_MENU_BUTTON_TITLE);
	if (TitleTextBlockWidget)
	{
		const FText TextValue = FText::FromString(InDebugMenuButtonParameters.ButtonName.ToString());
		TitleTextBlockWidget->SetText(TextValue);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No TitleTextBlock found, please check that you created a button widget with this name: %hs"), DEBUG_MENU_BUTTON_TITLE);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UDebugMenu_ButtonWidget::OnButtonPressedEvent()
{
	UPantheonGenericDebugMenuSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>();
	Subsystem->Internal_NotifyButtonPressedOnClient(ButtonName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// UDebugMenuUserWidget
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UDebugMenuUserWidget::InitializeDebugMenuWidget(FName const& Name)
{
	UWidget* DebugMenuTitleWidget = WidgetTree->FindWidget(DEBUG_MENU_TITLE);
	UTextBlock* DebugMenuTitleTextBlock = Cast<UTextBlock>(DebugMenuTitleWidget);
	if (DebugMenuTitleTextBlock == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No DebugMenuTitleTextBlock found, please check that you created a text widget with this name: %hs"), DEBUG_MENU_TITLE);
	}
	else
	{
		const FText Text = FText::FromName(Name);
		DebugMenuTitleTextBlock->SetText(Text);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UDebugMenu_SliderWidget* UDebugMenuUserWidget::AddSliderToDebugMenuWidget(FPanDebugMenuSliderParameters const& DebugMenuSliderParameters, TSubclassOf<UDebugMenu_SliderWidget> SliderClass)
{
	if (DebugMenuHolderPanelWidget == nullptr)
	{
		return nullptr;
	}
	UDebugMenu_SliderWidget* DebugMenuSliderWidget = WidgetTree->ConstructWidget<UDebugMenu_SliderWidget>(SliderClass); 
	DebugMenuHolderPanelWidget->AddChild(DebugMenuSliderWidget);
	DebugMenuSliderWidget->InitializeSlider(DebugMenuSliderParameters);
	
	return DebugMenuSliderWidget;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UDebugMenu_ButtonWidget* UDebugMenuUserWidget::AddButtonToDebugMenuWidget(
	FPanDebugMenuButtonParameters const& DebugMenuButtonParameters, TSubclassOf<UDebugMenu_ButtonWidget> ButtonClass)
{
	if (DebugMenuHolderPanelWidget == nullptr)
	{
		return nullptr;
	}
	UDebugMenu_ButtonWidget* DebugMenuButtonWidget = WidgetTree->ConstructWidget<UDebugMenu_ButtonWidget>(ButtonClass); 
	DebugMenuHolderPanelWidget->AddChild(DebugMenuButtonWidget);
	DebugMenuButtonWidget->InitializeButton(DebugMenuButtonParameters);
	
	return DebugMenuButtonWidget;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UDebugMenuUserWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	UWidget* CanvasWidget = WidgetTree->FindWidget(DEBUG_MENU_CANVAS);
	DebugMenuCanvas = Cast<UPanelWidget>(CanvasWidget);
	if (DebugMenuCanvas == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No CanvasWidget found, please check that you created a canvas with this name: %hs"), DEBUG_MENU_CANVAS);
	}
	
	UWidget* GrabBorderWidget = WidgetTree->FindWidget(DEBUG_MENU_GRAB);
	DebugMenuBorder = Cast<UBorder>(GrabBorderWidget);
	if (DebugMenuBorder == nullptr)
	{ 
		UE_LOG(LogTemp, Error, TEXT("No GrabBorderWidget found, please check that you created a border with this name: %hs"), DEBUG_MENU_GRAB);
	}
	else
	{
		DebugMenuBorder->OnMouseButtonDownEvent.BindDynamic(this, &UDebugMenuUserWidget::OnMouseButtonDowned);
		DebugMenuBorder->OnMouseButtonUpEvent.BindDynamic(this, &UDebugMenuUserWidget::OnMouseButtonUpped);
	}
	
	UWidget* DebugMenuHolderWidget = WidgetTree->FindWidget(DEBUG_MENU_CONTAINER);
	DebugMenuHolderPanelWidget = Cast<UPanelWidget>(DebugMenuHolderWidget);
	if (DebugMenuHolderPanelWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No DebugMenuHolderWidget found, please check that you created a panel widget with this name: %hs"), DEBUG_MENU_CONTAINER);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UDebugMenuUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (IsMouseGrabbed)
	{
		if (GetWorld()->GetFirstPlayerController()->PlayerInput->IsPressed(FKey(TEXT("LeftMouseButton"))) == false)
		{
			IsMouseGrabbed = false;
		}
		
		float XMouseLocation, YMouseLocation;
		GetWorld()->GetFirstPlayerController()->GetMousePosition(XMouseLocation, YMouseLocation);
		
		const FVector2d MousePosition = FVector2d(XMouseLocation, YMouseLocation);
		SetPositionInViewport(MousePosition - MouseGrabOffset, true);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FEventReply UDebugMenuUserWidget::OnMouseButtonDowned(FGeometry InGeometry, FPointerEvent const& MouseEvent)
{
	FEventReply Reply;
	
	const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
	if (bIsLeftMouseButtonDown)
	{
		if (IsMouseGrabbed == false)
		{
			IsMouseGrabbed = true;
			
			UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(this->Slot);
			if (CanvasSlot)
			{
				// TODO Julien Rogel (18/03/2024): Doesnt work for now, could be nice to have
				//LastZOrderAndInputPriority++;
				//CanvasSlot->SetZOrder(LastZOrderAndInputPriority);
			}
			SetInputActionPriority(LastZOrderAndInputPriority);
			
			FVector2D MousePosition = MouseEvent.GetScreenSpacePosition();
			FVector2D LocalMousePosition = MousePosition - GetCachedGeometry().GetAbsolutePosition();
			MouseGrabOffset = LocalMousePosition;
		}
	}
	return Reply;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FEventReply UDebugMenuUserWidget::OnMouseButtonUpped(FGeometry InGeometry, FPointerEvent const& MouseEvent)
{
	FEventReply Reply;
	FKey Key = MouseEvent.GetEffectingButton();
	if (Key == FKey(TEXT("LeftMouseButton")))
	{
		IsMouseGrabbed = false;
	}
	Reply.NativeReply.Handled();
	return Reply;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ADebugMenu_ReplicatingActor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADebugMenu_ReplicatingActor::ADebugMenu_ReplicatingActor()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ADebugMenu_ReplicatingActor::BeginPlay()
{
	Super::BeginPlay();

	// Subscribe the replication actor to the subsystem, because client need to be able to send rpcs to the server
	const bool IsOwnedByLocalPlayerController = IsOwnedBy(GetWorld()->GetFirstPlayerController());
	if (IsOwnedByLocalPlayerController == true)
	{
		const ENetMode NetMode = GetWorld()->GetNetMode();
		if (NetMode == ENetMode::NM_Client || NetMode == ENetMode::NM_Standalone)
		{
			GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>()->Internal_RegisterReplicatingActorToDebugMenuOnClient(this);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ADebugMenu_ReplicatingActor::ServerRPC_RequestChangeSliderValue_Implementation(FName const& SliderName, float Value)
{
	GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>()->Internal_NotifyNewSliderValueOnServer(SliderName, Value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ADebugMenu_ReplicatingActor::ClientRPC_ChangeSliderValue_Implementation(FName const& SliderName, float Value)
{
	GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>()->Internal_UpdateClientSliderValueFromServer(SliderName, Value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ADebugMenu_ReplicatingActor::ServerRPC_RequestPressButton_Implementation(FName const& ButtonName)
{
	GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>()->Internal_NotifyButtonPressedOnServer(ButtonName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ADebugMenu_ReplicatingActor::ClientRPC_PressButton_Implementation(FName const& ButtonName)
{
	GetGameInstance()->GetSubsystem<UPantheonGenericDebugMenuSubsystem>()->Internal_PressButtonFromServer(ButtonName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

