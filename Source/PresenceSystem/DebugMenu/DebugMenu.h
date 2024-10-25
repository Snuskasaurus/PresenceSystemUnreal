#pragma once

#include "Core.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DebugMenu.generated.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FGeometry;

class USlider;
class UTextBlock;
class UBorder;
class UButton;
class UEditableText;
class UCanvasPanelSlot;

class APlayerController;

class ADebugMenu_ReplicatingActor;
class UDebugMenuUserWidget;
class UDebugMenu_SliderWidget;
class UDebugMenu_ButtonWidget;
class UDebugMenu_TextInputWidget;
class UDebugMenu_CustomWidget;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DELEGATE_OneParam(FOnDebugMenuSliderChangedValueEvent, float);
DECLARE_DELEGATE_OneParam(FOnDebugMenuTextInputValidatedEvent, FString const&);
DECLARE_DELEGATE(FOnDebugMenuButtonPressedEvent)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO Julien Rogel (21/03/2024): Make the code less repetitive for different elements like the button and the sliders
// TODO Julien Rogel (21/03/2024): Polish the logs, add a new logtype for the generic debug menu

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FPanDebugMenuSliderParameters
{
	FPanDebugMenuSliderParameters() = default;
	FPanDebugMenuSliderParameters(
		FName const& Name,
		float Minimum,
		float Maximum,
		float Current,
		FName const& UnitOfMeasurement,
		bool InIsReplicated)
	{
		SliderName = Name;
		ValueUnitOfMeasurement = UnitOfMeasurement;
		MinimumValue = Minimum;
		MaximumValue = Maximum;
		DefaultValue = Current;
		ShouldReplicate = InIsReplicated;
	}
	
	FName SliderName = "Default";
	FName ValueUnitOfMeasurement = "units";
	float MinimumValue = 0.0f;
	float MaximumValue = 1.0f;
	float DefaultValue = 0.5f;
	bool ShouldReplicate = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FPanDebugMenuSliderInfo
{
	FPanDebugMenuSliderInfo() = default;
	FPanDebugMenuSliderInfo(FPanDebugMenuSliderParameters const& SliderParameters)
	{
		SliderName = SliderParameters.SliderName;
		CurrentValue = SliderParameters.DefaultValue;
		IsReplicated = SliderParameters.ShouldReplicate;
		IsWaitingForReplication = IsReplicated;
	}

	bool IsNeedingReplication() const { return IsReplicated && IsWaitingForReplication; }
		
	FName SliderName = "Default";			// TODO Julien Rogel (20/03/2024): Replace later by an index maybe ?
	FName DebugMenuName = "None";			// TODO Julien Rogel (20/03/2024): Replace later by an index maybe ?
	bool IsReplicated = false;
	bool IsWaitingForReplication = false;
	float CurrentValue = 0.5f;
	
	TObjectPtr<UDebugMenu_SliderWidget> SliderWidget;
	FOnDebugMenuSliderChangedValueEvent OnDebugMenuSliderChangedValue;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FPanDebugMenuButtonParameters
{
	FPanDebugMenuButtonParameters() = default;
	FPanDebugMenuButtonParameters(
		FName const& InButtonName,
		bool InIsReplicated)
	{
		ButtonName = InButtonName;
		ShouldReplicate = InIsReplicated;
	}
	
	FName ButtonName = "Default";
	bool ShouldReplicate = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FPanDebugMenuButtonInfo
{
	FPanDebugMenuButtonInfo() = default;
	FPanDebugMenuButtonInfo(FPanDebugMenuButtonParameters const& ButtonParameters)
	{
		ButtonName = ButtonParameters.ButtonName;
		IsReplicated = ButtonParameters.ShouldReplicate;
		IsWaitingForReplication = IsReplicated;
	}

	bool IsNeedingReplication() const { return IsReplicated && IsWaitingForReplication; }
		
	FName ButtonName = "Default";			// TODO Julien Rogel (20/03/2024): Replace later by an index maybe ?
	FName DebugMenuName = "None";			// TODO Julien Rogel (20/03/2024): Replace later by an index maybe ?
	bool IsReplicated = false;
	bool IsWaitingForReplication = false;
	
	TObjectPtr<UDebugMenu_ButtonWidget> ButtonWidget;
	FOnDebugMenuButtonPressedEvent OnButtonPressed;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FPanDebugMenuTextInputParameters
{
	FPanDebugMenuTextInputParameters() = default;
	FPanDebugMenuTextInputParameters(
		FName const& InWidgetName,
		bool InIsReplicated,
		FString InDefaultText)
	{
		WidgetName = InWidgetName;
		ShouldReplicate = InIsReplicated;
		DefaultText = InDefaultText;
	}
	
	FName WidgetName = "Default";
	bool ShouldReplicate = false;
	FString DefaultText;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FPanDebugMenuTextInputInfo
{
	FPanDebugMenuTextInputInfo() = default;
	FPanDebugMenuTextInputInfo(FPanDebugMenuTextInputParameters const& ButtonParameters)
	{
		WidgetName = ButtonParameters.WidgetName;
		IsReplicated = ButtonParameters.ShouldReplicate;
		IsWaitingForReplication = IsReplicated;
	}

	bool IsNeedingReplication() const { return IsReplicated && IsWaitingForReplication; }
		
	FName WidgetName = "Default";			// TODO Julien Rogel (20/03/2024): Replace later by an index maybe ?
	FName DebugMenuName = "None";			// TODO Julien Rogel (20/03/2024): Replace later by an index maybe ?
	bool IsReplicated = false;
	bool IsWaitingForReplication = false;
	
	TObjectPtr<UDebugMenu_TextInputWidget> WidgetPtr;
	FOnDebugMenuTextInputValidatedEvent OnTextInputValidated;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FPanDebugMenuCustomWidgetInfo
{
	FName WidgetName = "Default";
	FName DebugMenuName = "None";
	TObjectPtr<UDebugMenu_CustomWidget> WidgetPtr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FDebugMenuReplicatingActor_ArrayHolder
{
	FDebugMenuReplicatingActor_ArrayHolder() = default;
	FDebugMenuReplicatingActor_ArrayHolder(APlayerController* InOwningPlayerController, ADebugMenu_ReplicatingActor* InReplicatingActor)
		: OwningPlayerController(InOwningPlayerController), ReplicatingActor(InReplicatingActor) {}
	
	TObjectPtr<APlayerController> OwningPlayerController;
	ADebugMenu_ReplicatingActor* ReplicatingActor;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FDebugMenuWidget_ArrayHolder
{
	FDebugMenuWidget_ArrayHolder() = default;
	FDebugMenuWidget_ArrayHolder(FName InName, UDebugMenuUserWidget* InWidget)
		: Name(InName), Widget(InWidget) {}
	
	FName Name;
	TObjectPtr<UDebugMenuUserWidget> Widget;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Debug Menu Subsystem (On client and server)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Blueprintable)
class UPantheonGenericDebugMenuSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	void InitializeGenericDebugMenuSubsystem(APlayerController* OwningPlayerController);
	void UninitializeGenericDebugMenuSubsystem(APlayerController* OwningPlayerController);
	void CreateDebugMenu(FName const& DebugMenuName, FName const& PresetName, FVector2d InitialPosition, bool IsDisplayedByDefault);
	void DestroyDebugMenu(FName const& DebugMenuName);

	void AddSliderToDebugMenu(FName const& DebugMenuName, FName const& PresetName, FPanDebugMenuSliderParameters const& DebugMenuSliderParameters, TFunction<void(float)> Lambda);
	void AddButtonToDebugMenu(FName const& DebugMenuName, FName const& PresetName, FPanDebugMenuButtonParameters const& DebugMenuButtonParameters, TFunction<void()> Lambda);
	void AddTextInputToDebugMenu(FName const& DebugMenuName, FName const& PresetName, FPanDebugMenuTextInputParameters const& DebugMenuTextInputParameters, TFunction<void(FString const&)> Lambda);
	FPanDebugMenuCustomWidgetInfo* AddCustomWidgetToDebugMenu(FName const& DebugMenuName, FName const& WidgetName, TSubclassOf<UDebugMenu_CustomWidget> PresetClass);

	void RemoveCustomWidgetFromDebugMenu(FName const& DebugMenuName, FName const& WidgetName);
	
	void Internal_RegisterReplicatingActorToDebugMenuOnClient(ADebugMenu_ReplicatingActor* InReplicatingActor);
	void Internal_SetDebugMenuVisibility(FName const& DebugMenuName, bool Visibility);
	
	void Internal_NotifyNewSliderValueOnClient(FName const& WidgetName, float NewValue);
	void Internal_NotifyNewSliderValueOnServer(FName const& WidgetName, float NewValue);
	void Internal_UpdateClientSliderValueFromServer(FName const& WidgetName, float NewValue);

	void Internal_NotifyButtonPressedOnClient(FName const& WidgetName);
	void Internal_NotifyButtonPressedOnServer(FName const& WidgetName);
	void Internal_PressButtonFromServer(FName const& WidgetName);
	
	void Internal_NotifyTextChangedOnClient(FName const& WidgetName, FString const& NewString);
	void Internal_NotifyTextChangedOnServer(FName const& WidgetName, FString const& NewString);
	void Internal_TextChangedFromServer(FName const& WidgetName, FString const& NewString);
	
private:

	void Exec_DisplayDebugMenu(const TArray<FString>& Args);
	FDebugMenuWidget_ArrayHolder* GetDebugMenuFromName(FName const& DebugMenuName);
	
	FPanDebugMenuSliderInfo* GetSliderInfoFromName(FName const& SliderName);
	bool CheckIfSliderAlreadyExist(FName const& WidgetName);
	
	FPanDebugMenuButtonInfo* GetButtonInfoFromName(FName const& ButtonName);
	bool CheckIfButtonAlreadyExist(FName const& WidgetName);
	
	FPanDebugMenuTextInputInfo* GetTextInputInfoFromName(FName const& ButtonName);
	bool CheckIfTextInputAlreadyExist(FName const& WidgetName);
	
protected:

	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<UDebugMenuUserWidget>> DebugMenuClassPresets;
	
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<UDebugMenu_SliderWidget>> SliderClassPresets;
	
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<UDebugMenu_ButtonWidget>> ButtonClassPresets;
	
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<UDebugMenu_TextInputWidget>> TextInputClassPresets;


private:

	TArray<FDebugMenuReplicatingActor_ArrayHolder> ReplicatingActorsOnServer;
	ADebugMenu_ReplicatingActor* ReplicatingActorOnClient;
	IConsoleCommand* cmd_DisplayDebugMenu;
	TArray<FDebugMenuWidget_ArrayHolder> DebugMenus;
	
	TArray<FPanDebugMenuSliderInfo> StoredSlidersInfos;
	TArray<FPanDebugMenuButtonInfo> StoredButtonsInfos;
	TArray<FPanDebugMenuTextInputInfo> StoredTextInputInfos;
	TArray<FPanDebugMenuCustomWidgetInfo> StoredCustomWidgetInfos;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Slider UI (Only on client)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class UDebugMenu_SliderWidget : public UUserWidget
{
	GENERATED_BODY()
	
	friend UPantheonGenericDebugMenuSubsystem;
	
public:
	
	void InitializeSlider(FPanDebugMenuSliderParameters const& InDebugMenuSliderParameters);
	
private:
	
	void SetCurrentSliderValue(float CurrentValue);
	void SetCurrentValueText(float CurrentValue);

	UFUNCTION() void OnSliderChangedEvent(float NewValue);
	
private:
	
	TObjectPtr<USlider>		SliderWidget;
	TObjectPtr<UTextBlock>	ValueTextWidget;
	TObjectPtr<UTextBlock>	TitleTextWidget;

	FName SliderName;
	FName ValueUnitOfMeasurement = "invalid";
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Button UI (Only on client)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class UDebugMenu_ButtonWidget : public UUserWidget
{
	GENERATED_BODY()
	
	friend UPantheonGenericDebugMenuSubsystem;
	
public:
	
	void InitializeButton(FPanDebugMenuButtonParameters const& InDebugMenuButtonParameters);
	
private:

	UFUNCTION() void OnButtonPressedEvent();
	
private:
	
	TObjectPtr<UButton> ButtonWidget;
	TObjectPtr<UTextBlock> TitleTextBlockWidget;
	
	FName ButtonName;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Text Input UI (Only on client)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class UDebugMenu_TextInputWidget : public UUserWidget
{
	GENERATED_BODY()
	
	friend UPantheonGenericDebugMenuSubsystem;
	
public:
	
	void InitializeTextInput(FPanDebugMenuTextInputParameters const& InDebugMenuTextInputParameters);
	
private:

	UFUNCTION() void OnTextChangedEvent(FText const& NewText);
	UFUNCTION() void OnButtonPressedEvent();
	
private:
	
	TObjectPtr<UTextBlock> TitleTextBlockWidget;
	TObjectPtr<UButton> ButtonWidget;
	TObjectPtr<UEditableText> EditableTextBoxWidget;
	
	FName WidgetName;
	FString TextFromInput;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Custom UI (Only on client)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class UDebugMenu_CustomWidget : public UUserWidget
{
	GENERATED_BODY()
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Debug Menu UI (Only on client)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class UDebugMenuUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void InitializeDebugMenuWidget(FName const& Name);
	
	UDebugMenu_SliderWidget* AddSliderToDebugMenuWidget(FPanDebugMenuSliderParameters const& DebugMenuSliderParameters, TSubclassOf<UDebugMenu_SliderWidget> SliderClass);
	UDebugMenu_ButtonWidget* AddButtonToDebugMenuWidget(FPanDebugMenuButtonParameters const& DebugMenuButtonParameters, TSubclassOf<UDebugMenu_ButtonWidget> ButtonClass);
	UDebugMenu_TextInputWidget* AddTextInputToDebugMenuWidget(FPanDebugMenuTextInputParameters const& DebugMenuTextInputParameters, TSubclassOf<UDebugMenu_TextInputWidget> TextInputClass);
	UDebugMenu_CustomWidget* AddCustomWidgetToDebugMenuWidget(TSubclassOf<UDebugMenu_CustomWidget> CustomWidgetClass);
	
protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:

	UFUNCTION() FEventReply OnMouseButtonDowned(FGeometry InGeometry, FPointerEvent const& MouseEvent);
	UFUNCTION() FEventReply OnMouseButtonUpped(FGeometry InGeometry, FPointerEvent const& MouseEvent);

private:
	
	TObjectPtr<UPanelWidget> DebugMenuCanvas;
	TObjectPtr<UBorder> DebugMenuBorder;
	TObjectPtr<UPanelWidget> DebugMenuHolderPanelWidget;

	FVector2d TargetScreenLocation;
	FVector2d MouseGrabOffset = FVector2d::Zero();
	bool IsMouseGrabbed = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Debug Menu Replicating Actor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS()
class ADebugMenu_ReplicatingActor : public AActor
{
	GENERATED_BODY()
	
public:
	
	ADebugMenu_ReplicatingActor();

public:
	
	UFUNCTION(Server, Unreliable)
	void ServerRPC_RequestChangeSliderValue(FName const& SliderName, float Value);
	UFUNCTION(Client, Unreliable)
	void ClientRPC_ChangeSliderValue(FName const& SliderName, float Value);
	
	UFUNCTION(Server, Unreliable)
	void ServerRPC_RequestPressButton(FName const& ButtonName);
	UFUNCTION(Client, Unreliable)
	void ClientRPC_PressButton(FName const& ButtonName);
	
	UFUNCTION(Server, Unreliable)
	void ServerRPC_RequestTextChange(FName const& WidgetName, FString const& NewString);
	UFUNCTION(Client, Unreliable)
	void ClientRPC_TextChange(FName const& WidgetName, FString const& NewString);

protected:

	virtual void BeginPlay() override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
