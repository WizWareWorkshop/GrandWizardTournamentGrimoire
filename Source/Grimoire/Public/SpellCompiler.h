#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GrimoireTypes.h"
#include "SpellCompiler.generated.h"

class USpellDataAsset;
class USpellNodeDefinition;

UCLASS(BlueprintType)
class GRIMOIRE_API UGWTSpellCompiler : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Spell Compiler")
    FCompiledSpellDefinition CompileSpellAsset(const USpellDataAsset* SpellAsset) const;

    static FCompiledSpellDefinition RunCompile(const USpellDataAsset* SpellAsset);

private:
    static void AddIssue(
        FCompiledSpellDefinition& Result,
        ESpellCompilerIssueSeverity Severity,
        const FString& Message,
        const FString& SuggestedFix = FString(),
        const FGuid& RelatedNodeId = FGuid()
    );

    static ECompiledSpellPayloadType ResolvePayloadType(const USpellNodeDefinition* NodeDefinition);
    static ESpellElement ResolveElement(const USpellNodeDefinition* NodeDefinition);
    static float GetNumericOverride(const FSpellNodeInstance& Node, const FName& Key, float DefaultValue);
    static int32 GetCountOverride(const FSpellNodeInstance& Node, const FName& Key, int32 DefaultValue);
    static FString BuildDeterministicSignature(const FCompiledSpellDefinition& Result);
};