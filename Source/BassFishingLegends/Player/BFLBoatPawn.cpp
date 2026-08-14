#include "Player/BFLBoatPawn.h"

#include "Animation/AnimationAsset.h"
#include "Camera/CameraComponent.h"
#include "CableComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Fishing/BFLFishingComponent.h"
#include "Game/BFLAssignedMesh.h"
#include "Game/BFLGameSettings.h"
#include "Game/BFLPlayerController.h"
#include "Game/BFLStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

ABFLBoatPawn::ABFLBoatPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->InitBoxExtent(FVector(90.f, 40.f, 22.f));
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collision->SetCollisionObjectType(ECC_Pawn);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetSimulatePhysics(false);
	RootComponent = Collision;

	HullMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hull"));
	HullMesh->SetupAttachment(RootComponent);
	HullMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HullMesh->SetRelativeScale3D(FVector(1.9f, 0.85f, 0.28f));
	HullMesh->SetRelativeLocation(FVector(0.f, 0.f, -8.f));

	CabinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cabin"));
	CabinMesh->SetupAttachment(RootComponent);
	CabinMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CabinMesh->SetRelativeLocation(FVector(-25.f, 0.f, 18.f));
	CabinMesh->SetRelativeScale3D(FVector(0.55f, 0.5f, 0.32f));

	RodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rod"));
	RodMesh->SetupAttachment(RootComponent);
	RodMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RodMesh->SetRelativeLocation(FVector(50.f, 18.f, 20.f));
	RodMesh->SetRelativeRotation(FRotator(35.f, 12.f, 0.f));
	RodMesh->SetRelativeScale3D(FVector(0.045f, 0.045f, 1.55f));

	RodGripMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RodGrip"));
	RodGripMesh->SetupAttachment(RodMesh);
	RodGripMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RodGripMesh->SetRelativeLocation(FVector(0.f, 0.f, -42.f));
	RodGripMesh->SetRelativeScale3D(FVector(1.8f, 1.8f, 0.22f));

	AnglerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Angler"));
	AnglerMesh->SetupAttachment(RootComponent);
	AnglerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AnglerMesh->SetRelativeLocation(FVector(12.f, 6.f, -18.f));
	AnglerMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	RodTip = CreateDefaultSubobject<USceneComponent>(TEXT("RodTip"));
	RodTip->SetupAttachment(RodMesh);
	RodTip->SetRelativeLocation(FVector(0.f, 0.f, 58.f));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 520.f;
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	FishingLine = CreateDefaultSubobject<UCableComponent>(TEXT("FishingLine"));
	FishingLine->SetupAttachment(RodTip);
	FishingLine->CableLength = 80.f;
	FishingLine->NumSegments = 14;
	FishingLine->CableWidth = 1.4f;
	FishingLine->SolverIterations = 8;
	FishingLine->SetVisibility(false);
	FishingLine->bAttachEnd = true;

	Fishing = CreateDefaultSubobject<UBFLFishingComponent>(TEXT("Fishing"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (const UBFLGameSettings* Settings = UBFLGameSettings::Get())
	{
		MaxSpeed = Settings->BoatMaxSpeed;
	}
}

void ABFLBoatPawn::BeginPlay()
{
	Super::BeginPlay();

	ApplyPlaceholderMeshes();

	if (Fishing)
	{
		Fishing->SetRodTip(RodTip);
		Fishing->SetLineCable(FishingLine);
	}

	EnsureRuntimeInput();
	AddMappingToController();
}

void ABFLBoatPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AddMappingToController();
}

void ABFLBoatPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float WaterZ = UBFLStatics::GetWaterHeight(this);
	const FVector Center = UBFLStatics::GetLakeCenter(this);
	const float Radius = UBFLStatics::GetLakeRadius(this) - 180.f;

	Throttle = FMath::FInterpTo(Throttle, FMath::Clamp(MoveInput.Y, -1.f, 1.f), DeltaSeconds, Acceleration);

	const float Steer = FMath::Clamp(MoveInput.X, -1.f, 1.f);
	const float SteerScale = 0.35f + 0.65f * FMath::Abs(Throttle);
	AddActorWorldRotation(FRotator(0.f, Steer * TurnSpeed * SteerScale * DeltaSeconds, 0.f));

	const FVector Delta = GetActorForwardVector() * Throttle * MaxSpeed * DeltaSeconds;
	FVector Next = GetActorLocation() + Delta;
	Next = UBFLStatics::ClampToLake(Next, Center, Radius, WaterZ, 22.f);

	BobPhase += DeltaSeconds * 1.6f;
	Next.Z = WaterZ + 22.f + FMath::Sin(BobPhase) * 2.4f;
	SetActorLocation(Next);

	// Subtle hull roll with steering / bob.
	if (HullMesh)
	{
		const float Roll = Steer * -6.f + FMath::Sin(BobPhase * 0.7f) * 1.5f;
		HullMesh->SetRelativeRotation(FRotator(0.f, 0.f, Roll));
	}

	CameraYaw = FMath::UnwindDegrees(CameraYaw + LookInput.X * LookYawSpeed * DeltaSeconds);
	CameraPitch = FMath::Clamp(CameraPitch + LookInput.Y * LookPitchSpeed * DeltaSeconds, -50.f, 12.f);
	if (SpringArm)
	{
		SpringArm->SetRelativeRotation(FRotator(CameraPitch, CameraYaw, 0.f));
	}

	DrawCastPreview();

	// Mouse look is a per-tick delta. Clear it so the last mouse move does not keep spinning.
	LookInput = FVector2D(0.f, 0.f);
}

void ABFLBoatPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	EnsureRuntimeInput();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABFLBoatPawn::Move);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABFLBoatPawn::Move);
	}
	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABFLBoatPawn::Look);
	}
	if (CastAction)
	{
		EIC->BindAction(CastAction, ETriggerEvent::Started, this, &ABFLBoatPawn::CastStarted);
		EIC->BindAction(CastAction, ETriggerEvent::Completed, this, &ABFLBoatPawn::CastCompleted);
		EIC->BindAction(CastAction, ETriggerEvent::Canceled, this, &ABFLBoatPawn::CastCompleted);
	}
	if (CancelAction)
	{
		EIC->BindAction(CancelAction, ETriggerEvent::Started, this, &ABFLBoatPawn::CancelPressed);
	}
}

void ABFLBoatPawn::Move(const FInputActionValue& Value)
{
	MoveInput = Value.Get<FVector2D>();
}

void ABFLBoatPawn::Look(const FInputActionValue& Value)
{
	LookInput = Value.Get<FVector2D>();
}

void ABFLBoatPawn::CastStarted(const FInputActionValue& Value)
{
	if (Fishing)
	{
		Fishing->StartCastCharge();
	}
}

void ABFLBoatPawn::CastCompleted(const FInputActionValue& Value)
{
	if (Fishing)
	{
		Fishing->ReleaseCast();
	}
}

void ABFLBoatPawn::CancelPressed(const FInputActionValue& Value)
{
	if (Fishing)
	{
		Fishing->Cancel();
	}
}

void ABFLBoatPawn::EnsureRuntimeInput()
{
	if (!MoveAction)
	{
		MoveAction = NewObject<UInputAction>(this, TEXT("IA_BFL_Move"));
		MoveAction->ValueType = EInputActionValueType::Axis2D;
	}
	if (!LookAction)
	{
		LookAction = NewObject<UInputAction>(this, TEXT("IA_BFL_Look"));
		LookAction->ValueType = EInputActionValueType::Axis2D;
	}
	if (!CastAction)
	{
		CastAction = NewObject<UInputAction>(this, TEXT("IA_BFL_Cast"));
		CastAction->ValueType = EInputActionValueType::Boolean;
	}
	if (!CancelAction)
	{
		CancelAction = NewObject<UInputAction>(this, TEXT("IA_BFL_Cancel"));
		CancelAction->ValueType = EInputActionValueType::Boolean;
	}

	if (!MappingContext)
	{
		MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_BFL_Default"));
		BuildDefaultMappings(MappingContext);
	}
}

void ABFLBoatPawn::AddMappingToController()
{
	if (!MappingContext)
	{
		return;
	}

	if (ABFLPlayerController* PC = Cast<ABFLPlayerController>(GetController()))
	{
		PC->AddMappingContext(MappingContext, 0);
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
}

void ABFLBoatPawn::BuildDefaultMappings(UInputMappingContext* IMC)
{
	if (!IMC)
	{
		return;
	}

	auto SwizzleY = [IMC](FEnhancedActionKeyMapping& Mapping)
	{
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(IMC);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		Mapping.Modifiers.Add(Swizzle);
	};
	auto Negate = [IMC](FEnhancedActionKeyMapping& Mapping)
	{
		Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(IMC));
	};

	// Keyboard move (WASD) as a 2D axis.
	{
		FEnhancedActionKeyMapping& W = IMC->MapKey(MoveAction, EKeys::W);
		SwizzleY(W);
	}
	{
		FEnhancedActionKeyMapping& S = IMC->MapKey(MoveAction, EKeys::S);
		SwizzleY(S);
		Negate(S);
	}
	IMC->MapKey(MoveAction, EKeys::D);
	{
		FEnhancedActionKeyMapping& A = IMC->MapKey(MoveAction, EKeys::A);
		Negate(A);
	}

	// Gamepad left stick.
	IMC->MapKey(MoveAction, EKeys::Gamepad_LeftX);
	{
		FEnhancedActionKeyMapping& GY = IMC->MapKey(MoveAction, EKeys::Gamepad_LeftY);
		SwizzleY(GY);
	}

	// Mouse look + right stick.
	IMC->MapKey(LookAction, EKeys::Mouse2D);
	IMC->MapKey(LookAction, EKeys::Gamepad_RightX);
	{
		FEnhancedActionKeyMapping& RY = IMC->MapKey(LookAction, EKeys::Gamepad_RightY);
		SwizzleY(RY);
		Negate(RY);
	}

	IMC->MapKey(CastAction, EKeys::LeftMouseButton);
	IMC->MapKey(CastAction, EKeys::Gamepad_RightTrigger);

	IMC->MapKey(CancelAction, EKeys::RightMouseButton);
	IMC->MapKey(CancelAction, EKeys::Gamepad_LeftTrigger);
	IMC->MapKey(CancelAction, EKeys::F);
}

void ABFLBoatPawn::LoadAuthoredMeshes()
{
	const UBFLGameSettings* Settings = UBFLGameSettings::Get();
	if (!Settings)
	{
		return;
	}

	if (HullMesh)
	{
		if (UStaticMesh* Hull = Settings->BoatHullMesh.LoadSynchronous())
		{
			UStaticMesh* Current = HullMesh->GetStaticMesh();
			HullMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Hull));
		}
	}
	if (RodMesh)
	{
		if (UStaticMesh* Rod = Settings->RodMesh.LoadSynchronous())
		{
			UStaticMesh* Current = RodMesh->GetStaticMesh();
			RodMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Rod));
		}
	}
	if (AnglerMesh)
	{
		if (USkeletalMesh* Angler = Settings->AnglerMesh.LoadSynchronous())
		{
			AnglerMesh->SetSkeletalMesh(BFLAssignedMesh::Keep(AnglerMesh->GetSkeletalMeshAsset(), Angler));
		}
		if (UAnimationAsset* Pose = Settings->AnglerPose.LoadSynchronous())
		{
			if (AnglerMesh->GetSkeletalMeshAsset())
			{
				AnglerMesh->PlayAnimation(Pose, true);
			}
		}
	}
}

void ABFLBoatPawn::AttachRodToAngler()
{
	if (!RodMesh || !AnglerMesh || !AnglerMesh->GetSkeletalMeshAsset())
	{
		return;
	}

	const FName Socket = AnglerHandSocket.IsNone() ? FName(TEXT("hand_r")) : AnglerHandSocket;
	const bool bHasAttach = AnglerMesh->DoesSocketExist(Socket) || AnglerMesh->GetBoneIndex(Socket) != INDEX_NONE;
	RodMesh->AttachToComponent(
		AnglerMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		bHasAttach ? Socket : NAME_None);
	RodMesh->SetRelativeLocation(RodInHandLocation);
	RodMesh->SetRelativeRotation(RodInHandRotation);
}

void ABFLBoatPawn::ApplyPlaceholderMeshes()
{
	LoadAuthoredMeshes();

	UStaticMesh* const HullBefore = HullMesh ? HullMesh->GetStaticMesh() : nullptr;
	UStaticMesh* const RodBefore = RodMesh ? RodMesh->GetStaticMesh() : nullptr;

	if (UStaticMesh* Cube = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		if (HullMesh)
		{
			UStaticMesh* Current = HullMesh->GetStaticMesh();
			HullMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Cube));
		}
	}
	if (UStaticMesh* Cylinder = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		if (RodMesh)
		{
			UStaticMesh* Current = RodMesh->GetStaticMesh();
			RodMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Cylinder));
		}
		if (RodGripMesh && !RodBefore)
		{
			UStaticMesh* Current = RodGripMesh->GetStaticMesh();
			RodGripMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Cylinder));
		}
	}

	if (HullBefore && HullMesh)
	{
		HullMesh->SetRelativeScale3D(FVector(1.f));
		HullMesh->SetRelativeLocation(FVector::ZeroVector);
	}
	else if (HullMesh)
	{
		if (UMaterialInstanceDynamic* HullMat = UBFLStatics::MakeTintedMeshMaterial(this, FLinearColor(0.12f, 0.22f, 0.38f)))
		{
			HullMesh->SetMaterial(0, HullMat);
		}
	}

	if (RodBefore && RodMesh)
	{
		RodMesh->SetRelativeScale3D(FVector(1.f));
	}
	if (!RodBefore && RodMesh)
	{
		if (UMaterialInstanceDynamic* RodMat = UBFLStatics::MakeTintedMeshMaterial(this, FLinearColor(0.15f, 0.15f, 0.16f)))
		{
			RodMesh->SetMaterial(0, RodMat);
		}
		if (RodGripMesh)
		{
			if (UMaterialInstanceDynamic* GripMat = UBFLStatics::MakeTintedMeshMaterial(this, FLinearColor(0.28f, 0.18f, 0.10f)))
			{
				RodGripMesh->SetMaterial(0, GripMat);
			}
		}
	}
	else if (RodGripMesh)
	{
		RodGripMesh->SetVisibility(false);
		RodGripMesh->SetHiddenInGame(true);
		RodGripMesh->SetStaticMesh(nullptr);
	}

	if (CabinMesh)
	{
		CabinMesh->SetVisibility(false);
		CabinMesh->SetHiddenInGame(true);
		CabinMesh->SetStaticMesh(nullptr);
	}

	if (HullMesh)
	{
		HullMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (RodMesh)
	{
		RodMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (RodGripMesh)
	{
		RodGripMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (AnglerMesh)
	{
		AnglerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	AttachRodToAngler();
}

void ABFLBoatPawn::DrawCastPreview() const
{
	if (!Fishing || Fishing->GetState() != EFishingState::ChargingCast || !GetWorld())
	{
		return;
	}

	FVector Landing;
	if (Fishing->PredictLanding(Landing))
	{
		DrawDebugSphere(GetWorld(), Landing + FVector(0.f, 0.f, 8.f), 22.f, 12, FColor(255, 220, 80), false, 0.f, 0, 2.f);
		DrawDebugCircle(GetWorld(), Landing, 40.f, 24, FColor(80, 180, 255), false, 0.f, 0, 2.f, FVector::ForwardVector, FVector::RightVector, false);
	}
}
