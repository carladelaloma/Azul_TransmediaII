#include "Actors/AzulInteractuableBase.h"
#include "Characters/AzulCharacterBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/LatentActionManager.h"
//#include "AzulComponentes/AzulStoryTextComponent.h"
#include "Dialogos/AzulDialogue.h"
#include "Dialogos/AzulWidgetDialogueBase.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AAzulInteractuableBase::AAzulInteractuableBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Crear el componente raíz (puede ser cualquier USceneComponent)
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);
	

	// Sphere collision (trigger)
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->InitSphereRadius(100.0f); // valor por defecto editable en BP si se expone
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetGenerateOverlapEvents(true);

	// Static mesh (visual)
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	// Dejar la colisión del mesh desactivada: la esfera manejará los overlaps
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComp->SetGenerateOverlapEvents(false);

	HiloEndPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("HiloEndPoint"));
	HiloEndPoint->SetupAttachment(RootComponent);
	HiloEndPoint->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	//HiloEndPoint->SetRelativeRotation(FRotator::ZeroRotator);

	// Opcional pero MUY recomendable
	HiloEndPoint->ArrowSize = 1.5f;
	HiloEndPoint->bHiddenInGame = true;
	HiloEndPoint->SetVisibility(false);
}

// Called when the game starts or when spawned
void AAzulInteractuableBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (CollisionSphere)
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAzulInteractuableBase::OnBeginOverlap);
		CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AAzulInteractuableBase::OnEndOverlap);
	}
}

void AAzulInteractuableBase::Interactua_Implementation()
{
	OnExtraInteractBP();
}

// Called every frame
void AAzulInteractuableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


bool AAzulInteractuableBase::IsValidInteractionComponent(
	const UPrimitiveComponent* Component) const
{
	if (!Component)
		return false;

	if (InteractionComponents.Num() == 0)
	{
		return Component == MeshComp;
	}

	return InteractionComponents.Contains(Component);
}


void AAzulInteractuableBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[INTERACT] Overlap BEGIN with %s"), *GetName());
	// Verificar que el actor que entra sea del tipo AzulCharacterBase o derivado
	OverlappingCharacter = Cast<AAzulCharacterBase>(OtherActor);
	if (!OverlappingCharacter)
	{
		return; // Si no es un personaje AzulCharacterBase, no hacer nada
	}

	OverlappingCharacter->AddInteractable(
		TScriptInterface<IAzulInteractuableInterface>(this)
	);

}

void AAzulInteractuableBase::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[INTERACT] Overlap END with %s"), *GetName());

	// Verificar que el actor que sale sea del tipo AzulCharacterBase o derivado
	OverlappingCharacter = Cast<AAzulCharacterBase>(OtherActor);
	if (!OverlappingCharacter)
	{
		return; // Si no es un personaje AzulCharacterBase, no hacer nada
	}

	OverlappingCharacter->RemoveInteractable(
		TScriptInterface<IAzulInteractuableInterface>(this)
	);

}

