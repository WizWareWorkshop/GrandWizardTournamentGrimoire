#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GrimoireTypes.h"
#include "SpellDataAsset.generated.h"

UCLASS(BlueprintType)
class GRIMOIRE_API USpellDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    USpellDataAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
    FName SpellId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell", meta = (MultiLine = "true"))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
    FSpellGraphData Graph;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bLastValidationSucceeded = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    FString LastValidationMessage;

    UFUNCTION(BlueprintCallable, Category = "Spell Graph")
    void EnsureStableIds();

    UFUNCTION(BlueprintCallable, Category = "Spell Graph")
    void RebuildNodePinsFromDefinitions();

    UFUNCTION(BlueprintCallable, Category = "Spell Graph")
    void ValidateGraph();

    UFUNCTION(BlueprintPure, Category = "Spell Graph")
    int32 GetNodeCount() const;

    UFUNCTION(BlueprintPure, Category = "Spell Graph")
    int32 GetEdgeCount() const;

    UFUNCTION(BlueprintPure, Category = "Spell Graph")
    bool ContainsNode(const FGuid& NodeId) const;

    bool IsGraphStructurallyValid(FString& OutError) const;
    const FSpellNodeInstance* FindNode(const FGuid& NodeId) const;
    FSpellNodeInstance* FindNodeMutable(const FGuid& NodeId);

    virtual void PostLoad() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};