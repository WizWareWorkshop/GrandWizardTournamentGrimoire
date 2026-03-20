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

UENUM(BlueprintType)
enum class ESpellGraphIssueSeverity : uint8
{
    Info    UMETA(DisplayName = "Info"),
    Warning UMETA(DisplayName = "Warning"),
    Error   UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class ESpellCompilerIssueSeverity : uint8
{
    Info    UMETA(DisplayName = "Info"),
    Warning UMETA(DisplayName = "Warning"),
    Error   UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class ECompiledSpellPayloadType : uint8
{
    None        UMETA(DisplayName = "None"),
    Projectile  UMETA(DisplayName = "Projectile"),
    Burst       UMETA(DisplayName = "Burst"),
    Zone        UMETA(DisplayName = "Zone"),
    Arc         UMETA(DisplayName = "Arc"),
    Chain       UMETA(DisplayName = "Chain")
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
    TArray<FSpellPinData> Pins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    TMap<FName, float> NumericOverrides;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    TArray<FName> RuntimeTags;

    // Legacy migration only. New code should use Graph.EditorData.NodeLayouts.
    UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use Graph.EditorData.NodeLayouts instead."))
    FVector2D GraphPosition = FVector2D::ZeroVector;
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
struct GRIMOIRE_API FSpellNodeLayoutData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FGuid NodeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FVector2D Position = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellGraphEditorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    TArray<FSpellNodeLayoutData> NodeLayouts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    FVector2D ViewOffset = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell Graph")
    float ZoomAmount = 1.0f;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellGraphIssue
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    ESpellGraphIssueSeverity Severity = ESpellGraphIssueSeverity::Info;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    FString Message;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    FString SuggestedFix;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    FGuid RelatedNodeId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    FGuid RelatedPinId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    FGuid RelatedEdgeId;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellGraphValidationResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    bool bIsValid = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    int32 ErrorCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    int32 WarningCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    int32 InfoCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Validation")
    TArray<FSpellGraphIssue> Issues;

    void Reset();

    void AddIssue(
        ESpellGraphIssueSeverity Severity,
        const FString& Message,
        const FString& SuggestedFix = FString(),
        const FGuid& RelatedNodeId = FGuid(),
        const FGuid& RelatedPinId = FGuid(),
        const FGuid& RelatedEdgeId = FGuid()
    );

    bool HasErrors() const
    {
        return ErrorCount > 0;
    }

    FString BuildSummary() const;
    void RecalculateCounts();
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FSpellCompilerIssue
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    ESpellCompilerIssueSeverity Severity = ESpellCompilerIssueSeverity::Info;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    FString Message;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    FString SuggestedFix;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    FGuid RelatedNodeId;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FCompiledSpellEntryPoint
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FGuid TriggerNodeId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FName TriggerTag = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FGuid> NextStepNodeIds;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FCompiledSpellConditionBlock
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FGuid NodeId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FName DefinitionId = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FName CompileBehaviorTag = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FName> RuntimeTags;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FCompiledSpellVariableOp
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FGuid NodeId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FName DefinitionId = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FName CompileBehaviorTag = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TMap<FName, float> NumericOperands;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FCompiledSpellPayloadSpec
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    int32 PayloadIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FGuid SourceNodeId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FName DefinitionId = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FName CompileBehaviorTag = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    ECompiledSpellPayloadType PayloadType = ECompiledSpellPayloadType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    ESpellElement Element = ESpellElement::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    float Magnitude = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    float Radius = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    float Range = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    float Speed = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    int32 Count = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FName> RuntimeTags;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FCompiledSpellFlowStep
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FGuid NodeId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    ESpellNodeCategory Category = ESpellNodeCategory::Magic;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FName DefinitionId = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    FName CompileBehaviorTag = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FGuid> NextStepNodeIds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<int32> ConditionBlockIndices;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<int32> VariableOpIndices;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<int32> PayloadIndices;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FName> RuntimeTags;
};

USTRUCT(BlueprintType)
struct GRIMOIRE_API FCompiledSpellDefinition
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    bool bCompileSucceeded = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    int32 ErrorCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    int32 WarningCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    int32 InfoCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    int32 SourceAssetVersion = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    int32 SourceGraphVersion = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    FName SourceSpellId = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    FString SourceAssetName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    FString CompiledHash;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    FString CompileSummary;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell Compiler")
    TArray<FSpellCompilerIssue> Issues;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FName> RuntimeTags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FCompiledSpellEntryPoint> EntryPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FCompiledSpellFlowStep> FlowSteps;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FCompiledSpellConditionBlock> ConditionBlocks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FCompiledSpellVariableOp> VariableOps;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compiled Spell")
    TArray<FCompiledSpellPayloadSpec> PayloadSpecs;

    void Reset();

    void AddIssue(
        ESpellCompilerIssueSeverity Severity,
        const FString& Message,
        const FString& SuggestedFix = FString(),
        const FGuid& RelatedNodeId = FGuid()
    );

    void RecalculateCounts();

    bool HasErrors() const
    {
        return ErrorCount > 0;
    }

    FString BuildSummary() const;
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