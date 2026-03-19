#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GrimoireTypes.h"
#include "SpellNodeDefinition.generated.h"

UCLASS(BlueprintType)
class GRIMOIRE_API USpellNodeDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    USpellNodeDefinition();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    FName DefinitionId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition", meta = (MultiLine = "true"))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    ESpellNodeCategory Category = ESpellNodeCategory::Magic;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    TArray<ESpellElement> SupportedElements;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    TArray<FSpellPinData> InputPins;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    TArray<FSpellPinData> OutputPins;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    bool bAllowMultipleInstances = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    bool bAllowAsEntryPoint = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    bool bRequiresOutgoingConnection = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Definition")
    FName CompileBehaviorTag = NAME_None;

    virtual void PostLoad() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    void NormalizePins();
};