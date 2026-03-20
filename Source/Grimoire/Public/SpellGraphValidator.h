#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "GrimoireTypes.h"
#include "SpellGraphValidator.generated.h"

class USpellDataAsset;

UCLASS(BlueprintType)
class GRIMOIRE_API USpellGraphValidator : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Spell Validation")
    FSpellGraphValidationResult ValidateSpellAsset(const USpellDataAsset* SpellAsset) const;

    static FSpellGraphValidationResult RunValidation(const USpellDataAsset* SpellAsset);

private:
    static void AddIssue(
        FSpellGraphValidationResult& Result,
        ESpellGraphIssueSeverity Severity,
        const FString& Message,
        const FString& SuggestedFix = FString(),
        const FGuid& RelatedNodeId = FGuid(),
        const FGuid& RelatedPinId = FGuid(),
        const FGuid& RelatedEdgeId = FGuid()
    );

    static void ValidateVersions(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result);
    static void ValidateNodes(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result);
    static void ValidateEdges(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result);
    static void ValidateConnectivity(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result);
    static void ValidateExecutionCycles(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result);
};