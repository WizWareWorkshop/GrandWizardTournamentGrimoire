#pragma once

#include "CoreMinimal.h"
#include "GrimoireTypes.generated.h"

UENUM(BlueprintType)
enum class ESpellNodeCategory : uint8
{
    Trigger     UMETA(DisplayName = "Trigger"),
    Magic       UMETA(DisplayName = "Magic"),
    Effect      UMETA(DisplayName = "Effect"),
    Condition   UMETA(DisplayName = "Condition"),
    Variable    UMETA(DisplayName = "Variable"),
    Flow        UMETA(DisplayName = "Flow")
};

UENUM(BlueprintType)
enum class ESpellPinDirection : uint8
{
    Input   UMETA(DisplayName = "Input"),
    Output  UMETA(DisplayName = "Output")
};

UENUM(BlueprintType)
enum class ESpellValueType : uint8
{
    Execution   UMETA(DisplayName = "Execution"),
    Boolean     UMETA(DisplayName = "Boolean"),
    Integer     UMETA(DisplayName = "Integer"),
    Float       UMETA(DisplayName = "Float"),
    Name        UMETA(DisplayName = "Name"),
    String      UMETA(DisplayName = "String"),
    Vector      UMETA(DisplayName = "Vector"),
    Target      UMETA(DisplayName = "Target"),
    Element     UMETA(DisplayName = "Element"),
    Spell       UMETA(DisplayName = "Spell")
};

UENUM(BlueprintType)
enum class ESpellElement : uint8
{
    None        UMETA(DisplayName = "None"),
    Fire        UMETA(DisplayName = "Fire"),
    Water       UMETA(DisplayName = "Water"),
    Ice         UMETA(DisplayName = "Ice"),
    Electricity UMETA(DisplayName = "Electricity"),
    Earth       UMETA(DisplayName = "Earth"),
    Plant       UMETA(DisplayName = "Plant"),
    Metal       UMETA(DisplayName = "Metal"),
    Poison      UMETA(DisplayName = "Poison")
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellPinValue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    bool BoolValue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    int32 IntValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    float FloatValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    FName NameValue = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    FString StringValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    FVector VectorValue = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellPinData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    FGuid PinId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    FName PinName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    ESpellPinDirection Direction = ESpellPinDirection::Input;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    ESpellValueType ValueType = ESpellValueType::Execution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    FSpellPinValue DefaultValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    bool bSupportsMultipleConnections = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    bool bSupportsGemSocket = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Pin")
    TArray<ESpellValueType> AllowedConnectionTypes;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellNodeInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FGuid NodeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    TObjectPtr<class USpellNodeDefinition> NodeDefinition = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FName NodeInstanceName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FVector2D GraphPosition = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    TArray<FSpellPinData> Pins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    TMap<FName, float> NumericOverrides;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    TArray<FName> RuntimeTags;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellEdgeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FGuid EdgeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FGuid FromNodeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FGuid FromPinId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FGuid ToNodeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FGuid ToPinId;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellGraphEditorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FVector2D ViewOffset = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    float ZoomAmount = 1.0f;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellGraphData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    int32 GraphVersion = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    TArray<FSpellNodeInstance> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    TArray<FSpellEdgeData> Edges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FSpellGraphEditorData EditorData;
};