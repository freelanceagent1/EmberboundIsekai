#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampBuildingActor.generated.h"

class UBuildingData;
class UBoxComponent;
class UCampBuildingPanelWidget;
class UMaterialInterface;
class UPrimitiveComponent;
class USphereComponent;
class UTextRenderComponent;
class UWidgetComponent;

USTRUCT()
struct FCampBuildingLayoutSpec
{
	GENERATED_BODY()

	UPROPERTY()
	FName BuildingId;

	UPROPERTY()
	FVector Location = FVector::ZeroVector;
};

UCLASS()
class EMBERBOUNDISEKAI_API ACampBuildingActor : public AActor
{
	GENERATED_BODY()

public:
	ACampBuildingActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;
	static FText DisplayNameForBuildingId(FName InBuildingId);
	static TArray<FCampBuildingLayoutSpec> CampLayoutSpecs();

	UPROPERTY(EditAnywhere, Category = "Building")
	FName BuildingId;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BillboardVisual;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FlameVisual;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> NameplateText;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> NameplateWidget;

	UPROPERTY()
	TObjectPtr<UBuildingData> BuildingData;

	virtual bool IsInteractionLocked(FText& OutReason) const;
	virtual void ConfigureVisual();
	virtual void OpenPanel();
	UBuildingData* LoadBuildingData() const;
	void ConfigureNameplate();
	void ConfigureBillboardMaterial(UStaticMeshComponent* Component, const FString& TexturePath, UMaterialInterface* OverrideMaterial = nullptr);
	FText GetDisplayName() const;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

UCLASS()
class EMBERBOUNDISEKAI_API ACampfireCoreActor : public ACampBuildingActor
{
	GENERATED_BODY()

public:
	ACampfireCoreActor();

protected:
	virtual void ConfigureVisual() override;
};

UCLASS()
class EMBERBOUNDISEKAI_API AGrimForgeActor : public ACampBuildingActor
{
	GENERATED_BODY()

public:
	AGrimForgeActor();
};

UCLASS()
class EMBERBOUNDISEKAI_API AMonsterPenActor : public ACampBuildingActor
{
	GENERATED_BODY()

public:
	AMonsterPenActor();

protected:
	virtual bool IsInteractionLocked(FText& OutReason) const override;
};

UCLASS()
class EMBERBOUNDISEKAI_API AWatchtowerActor : public ACampBuildingActor
{
	GENERATED_BODY()

public:
	AWatchtowerActor();
};

UCLASS()
class EMBERBOUNDISEKAI_API AExpeditionPortalActor : public ACampBuildingActor
{
	GENERATED_BODY()

public:
	AExpeditionPortalActor();

protected:
	virtual void OpenPanel();
};
