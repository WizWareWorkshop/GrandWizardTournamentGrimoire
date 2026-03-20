#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GrimoireTypes.h"
#include "SpellDataAsset.generated.h"
class USpellNodeDefinition;

UCLASS(BlueprintType)
class GRIMOIRE_API USpellDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    USpellDataAsset();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell", meta = (ClampMin = "1"))
    int32 AssetVersion = 1;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    FSpellGraphValidationResult LastValidationResult;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation")
    bool bLastCompileSucceeded = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation")
    FString LastCompileMessage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compilation")
    FCompiledSpellDefinition LastCompiledDefinition;

    UFUNCTION(BlueprintCallable, Category = "Spell Graph")
    FGuid AddNodeFromDefinition(USpellNodeDefinition* NodeDefinition, const FVector2D& GraphPosition, FName DesiredInstanceName);

    UFUNCTION(BlueprintCallable, Category = "Spell Graph")
    bool RemoveNodeById(const FGuid& NodeId);

    UFUNCTION(BlueprintCallable, Category = "Spell Graph")
    bool RemoveEdgeById(const FGuid& EdgeId);

    UFUNCTION(BlueprintCallable, Category = "Spell Graph")
    bool ConnectPinsByName(const FGuid& FromNodeId, FName FromPinName, const FGuid& ToNodeId, FName ToPinName, FString& OutError);

    UFUNCTION(BlueprintCallable, Category = "Spell Graph")
    bool DisconnectPinsByName(const FGuid& FromNodeId, FName FromPinName, const FGuid& ToNodeId, FName ToPinName);

    UFUNCTION(BlueprintPure, Category = "Spell Graph")
    bool GetPinIdByName(const FGuid& NodeId, FName PinName, ESpellPinDirection Direction, FGuid& OutPinId) const;

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Spell Graph")
    void EnsureStableIds();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Spell Graph")
    void EnsureNodeLayouts();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Spell Graph")
    void RebuildNodePinsFromDefinitions();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Spell Graph")
    void ValidateGraph();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Spell Graph")
    void CompileSpell();

    UFUNCTION(BlueprintPure, Category = "Spell Graph")
    int32 GetNodeCount() const;

    UFUNCTION(BlueprintPure, Category = "Spell Graph")
    int32 GetEdgeCount() const;

    UFUNCTION(BlueprintPure, Category = "Spell Graph")
    bool ContainsNode(const FGuid& NodeId) const;

    UFUNCTION(BlueprintPure, Category = "Spell Graph")
    FVector2D GetNodePosition(const FGuid& NodeId) const;

    UFUNCTION(BlueprintCallable, Category = "Spell Graph")
    void SetNodePosition(const FGuid& NodeId, const FVector2D& NewPosition);

    const FSpellNodeInstance* FindNode(const FGuid& NodeId) const;
    FSpellNodeInstance* FindNodeMutable(const FGuid& NodeId);

    const FSpellNodeLayoutData* FindNodeLayout(const FGuid& NodeId) const;
    FSpellNodeLayoutData* FindNodeLayoutMutable(const FGuid& NodeId);

    virtual void PostLoad() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};