#include "SpellCompiler.h"

#include "Algo/Sort.h"
#include "Misc/SecureHash.h"
#include "SpellDataAsset.h"
#include "SpellGraphValidator.h"
#include "SpellNodeDefinition.h"

namespace
{
    const FSpellPinData* FindPinById(const FSpellNodeInstance& Node, const FGuid& PinId)
    {
        for (const FSpellPinData& Pin : Node.Pins)
        {
            if (Pin.PinId == PinId)
            {
                return &Pin;
            }
        }

        return nullptr;
    }

    bool IsExecutionPin(const FSpellPinData& Pin)
    {
        return Pin.ValueType == ESpellValueType::Execution;
    }

    FName GetDefinitionKey(const USpellNodeDefinition* Definition)
    {
        if (!Definition)
        {
            return NAME_None;
        }

        return Definition->DefinitionId.IsNone()
            ? FName(*Definition->GetName())
            : Definition->DefinitionId;
    }

    FString GetNodeCategoryString(ESpellNodeCategory Category)
    {
        if (const UEnum* Enum = StaticEnum<ESpellNodeCategory>())
        {
            return Enum->GetNameStringByValue(static_cast<int64>(Category));
        }

        return TEXT("Unknown");
    }

    void SortGuidArray(TArray<FGuid>& Values)
    {
        Algo::Sort(Values, [](const FGuid& A, const FGuid& B)
            {
                return A.ToString(EGuidFormats::DigitsWithHyphensLower) < B.ToString(EGuidFormats::DigitsWithHyphensLower);
            });
    }

    void SortNameArray(TArray<FName>& Values)
    {
        Algo::Sort(Values, [](const FName& A, const FName& B)
            {
                return A.ToString() < B.ToString();
            });
    }

    FString JoinGuids(const TArray<FGuid>& Values)
    {
        FString Out;
        for (int32 Index = 0; Index < Values.Num(); ++Index)
        {
            if (Index > 0)
            {
                Out += TEXT(",");
            }

            Out += Values[Index].ToString(EGuidFormats::DigitsWithHyphensLower);
        }

        return Out;
    }

    FString JoinNames(const TArray<FName>& Values)
    {
        FString Out;
        for (int32 Index = 0; Index < Values.Num(); ++Index)
        {
            if (Index > 0)
            {
                Out += TEXT(",");
            }

            Out += Values[Index].ToString();
        }

        return Out;
    }

    FString JoinInts(const TArray<int32>& Values)
    {
        FString Out;
        for (int32 Index = 0; Index < Values.Num(); ++Index)
        {
            if (Index > 0)
            {
                Out += TEXT(",");
            }

            Out += LexToString(Values[Index]);
        }

        return Out;
    }

    FString SerializeFloatMap(const TMap<FName, float>& Values)
    {
        TArray<FName> Keys;
        Values.GetKeys(Keys);
        SortNameArray(Keys);

        FString Out;
        for (int32 Index = 0; Index < Keys.Num(); ++Index)
        {
            if (Index > 0)
            {
                Out += TEXT(",");
            }

            const FName Key = Keys[Index];
            const float Value = Values.FindRef(Key);
            Out += Key.ToString();
            Out += TEXT("=");
            Out += FString::Printf(TEXT("%.6f"), Value);
        }

        return Out;
    }

    void AddUniqueTags(TArray<FName>& Destination, const TArray<FName>& Source)
    {
        for (const FName& Tag : Source)
        {
            if (!Tag.IsNone())
            {
                Destination.AddUnique(Tag);
            }
        }

        SortNameArray(Destination);
    }

    void BuildExecutionAdjacency(
        const USpellDataAsset* SpellAsset,
        TMap<FGuid, TArray<FGuid>>& OutAdjacency
    )
    {
        OutAdjacency.Reset();

        for (const FSpellEdgeData& Edge : SpellAsset->Graph.Edges)
        {
            const FSpellNodeInstance* FromNode = SpellAsset->FindNode(Edge.FromNodeId);
            const FSpellNodeInstance* ToNode = SpellAsset->FindNode(Edge.ToNodeId);

            if (!FromNode || !ToNode)
            {
                continue;
            }

            const FSpellPinData* FromPin = FindPinById(*FromNode, Edge.FromPinId);
            const FSpellPinData* ToPin = FindPinById(*ToNode, Edge.ToPinId);

            if (!FromPin || !ToPin)
            {
                continue;
            }

            if (FromPin->Direction == ESpellPinDirection::Output &&
                ToPin->Direction == ESpellPinDirection::Input &&
                IsExecutionPin(*FromPin) &&
                IsExecutionPin(*ToPin))
            {
                TArray<FGuid>& Targets = OutAdjacency.FindOrAdd(Edge.FromNodeId);
                Targets.AddUnique(Edge.ToNodeId);
                SortGuidArray(Targets);
            }
        }
    }

    ESpellCompilerIssueSeverity MapValidationSeverity(ESpellGraphIssueSeverity Severity)
    {
        switch (Severity)
        {
        case ESpellGraphIssueSeverity::Error:
            return ESpellCompilerIssueSeverity::Error;

        case ESpellGraphIssueSeverity::Warning:
            return ESpellCompilerIssueSeverity::Warning;

        case ESpellGraphIssueSeverity::Info:
        default:
            return ESpellCompilerIssueSeverity::Info;
        }
    }
}

FCompiledSpellDefinition UGWTSpellCompiler::CompileSpellAsset(const USpellDataAsset* SpellAsset) const
{
    return RunCompile(SpellAsset);
}

FCompiledSpellDefinition UGWTSpellCompiler::RunCompile(const USpellDataAsset* SpellAsset)
{
    FCompiledSpellDefinition Result;
    Result.Reset();

    if (!SpellAsset)
    {
        AddIssue(
            Result,
            ESpellCompilerIssueSeverity::Error,
            TEXT("No spell asset was provided to the compiler."),
            TEXT("Pass a valid USpellDataAsset instance to compile.")
        );
        return Result;
    }

    Result.SourceAssetVersion = SpellAsset->AssetVersion;
    Result.SourceGraphVersion = SpellAsset->Graph.GraphVersion;
    Result.SourceSpellId = SpellAsset->SpellId;
    Result.SourceAssetName = SpellAsset->GetName();

    const FSpellGraphValidationResult ValidationResult = USpellGraphValidator::RunValidation(SpellAsset);
    for (const FSpellGraphIssue& Issue : ValidationResult.Issues)
    {
        AddIssue(
            Result,
            MapValidationSeverity(Issue.Severity),
            FString::Printf(TEXT("Validation: %s"), *Issue.Message),
            Issue.SuggestedFix,
            Issue.RelatedNodeId
        );
    }

    if (ValidationResult.HasErrors())
    {
        Result.RecalculateCounts();
        Result.CompileSummary = Result.BuildSummary();
        return Result;
    }

    TMap<FGuid, TArray<FGuid>> ExecutionAdjacency;
    BuildExecutionAdjacency(SpellAsset, ExecutionAdjacency);

    TArray<const FSpellNodeInstance*> SortedNodes;
    SortedNodes.Reserve(SpellAsset->Graph.Nodes.Num());

    for (const FSpellNodeInstance& Node : SpellAsset->Graph.Nodes)
    {
        SortedNodes.Add(&Node);
    }

    Algo::Sort(SortedNodes, [](const FSpellNodeInstance* A, const FSpellNodeInstance* B)
        {
            const FString AId = A ? A->NodeId.ToString(EGuidFormats::DigitsWithHyphensLower) : FString();
            const FString BId = B ? B->NodeId.ToString(EGuidFormats::DigitsWithHyphensLower) : FString();
            return AId < BId;
        });

    for (const FSpellNodeInstance* NodePtr : SortedNodes)
    {
        if (!NodePtr || !NodePtr->NodeDefinition)
        {
            continue;
        }

        const FSpellNodeInstance& Node = *NodePtr;
        const USpellNodeDefinition* Definition = Node.NodeDefinition;

        if (Definition->Category != ESpellNodeCategory::Trigger)
        {
            continue;
        }

        FCompiledSpellEntryPoint EntryPoint;
        EntryPoint.TriggerNodeId = Node.NodeId;
        EntryPoint.TriggerTag = !Definition->CompileBehaviorTag.IsNone()
            ? Definition->CompileBehaviorTag
            : GetDefinitionKey(Definition);
        EntryPoint.NextStepNodeIds = ExecutionAdjacency.FindRef(Node.NodeId);
        SortGuidArray(EntryPoint.NextStepNodeIds);

        Result.EntryPoints.Add(MoveTemp(EntryPoint));
        AddUniqueTags(Result.RuntimeTags, Node.RuntimeTags);

        if (Result.EntryPoints.Last().NextStepNodeIds.Num() == 0)
        {
            AddIssue(
                Result,
                ESpellCompilerIssueSeverity::Warning,
                FString::Printf(
                    TEXT("Trigger node %s compiles as an entry point but has no downstream execution targets."),
                    *Node.NodeId.ToString()
                ),
                TEXT("Connect the trigger's execution output to the rest of the spell flow."),
                Node.NodeId
            );
        }
    }

    for (const FSpellNodeInstance* NodePtr : SortedNodes)
    {
        if (!NodePtr || !NodePtr->NodeDefinition)
        {
            continue;
        }

        const FSpellNodeInstance& Node = *NodePtr;
        const USpellNodeDefinition* Definition = Node.NodeDefinition;

        if (Definition->Category == ESpellNodeCategory::Trigger)
        {
            continue;
        }

        FCompiledSpellFlowStep Step;
        Step.NodeId = Node.NodeId;
        Step.Category = Definition->Category;
        Step.DefinitionId = GetDefinitionKey(Definition);
        Step.CompileBehaviorTag = Definition->CompileBehaviorTag;
        Step.NextStepNodeIds = ExecutionAdjacency.FindRef(Node.NodeId);
        Step.RuntimeTags = Node.RuntimeTags;

        SortGuidArray(Step.NextStepNodeIds);
        SortNameArray(Step.RuntimeTags);

        switch (Definition->Category)
        {
        case ESpellNodeCategory::Condition:
        {
            FCompiledSpellConditionBlock Block;
            Block.NodeId = Node.NodeId;
            Block.DefinitionId = GetDefinitionKey(Definition);
            Block.CompileBehaviorTag = Definition->CompileBehaviorTag;
            Block.RuntimeTags = Node.RuntimeTags;
            SortNameArray(Block.RuntimeTags);

            const int32 BlockIndex = Result.ConditionBlocks.Add(MoveTemp(Block));
            Step.ConditionBlockIndices.Add(BlockIndex);
            break;
        }

        case ESpellNodeCategory::Variable:
        {
            FCompiledSpellVariableOp VariableOp;
            VariableOp.NodeId = Node.NodeId;
            VariableOp.DefinitionId = GetDefinitionKey(Definition);
            VariableOp.CompileBehaviorTag = Definition->CompileBehaviorTag;
            VariableOp.NumericOperands = Node.NumericOverrides;

            const int32 VariableIndex = Result.VariableOps.Add(MoveTemp(VariableOp));
            Step.VariableOpIndices.Add(VariableIndex);
            break;
        }

        case ESpellNodeCategory::Magic:
        case ESpellNodeCategory::Effect:
        {
            const ECompiledSpellPayloadType PayloadType = ResolvePayloadType(Definition);
            if (PayloadType != ECompiledSpellPayloadType::None)
            {
                FCompiledSpellPayloadSpec Payload;
                Payload.PayloadIndex = Result.PayloadSpecs.Num();
                Payload.SourceNodeId = Node.NodeId;
                Payload.DefinitionId = GetDefinitionKey(Definition);
                Payload.CompileBehaviorTag = Definition->CompileBehaviorTag;
                Payload.PayloadType = PayloadType;
                Payload.Element = ResolveElement(Definition);
                Payload.Magnitude = GetNumericOverride(Node, TEXT("Magnitude"), 0.0f);
                Payload.Radius = GetNumericOverride(Node, TEXT("Radius"), 0.0f);
                Payload.Range = GetNumericOverride(Node, TEXT("Range"), 0.0f);
                Payload.Speed = GetNumericOverride(Node, TEXT("Speed"), 0.0f);
                Payload.Count = GetCountOverride(Node, TEXT("Count"), 1);
                Payload.RuntimeTags = Node.RuntimeTags;
                SortNameArray(Payload.RuntimeTags);

                const int32 PayloadIndex = Result.PayloadSpecs.Add(MoveTemp(Payload));
                Step.PayloadIndices.Add(PayloadIndex);
            }
            else
            {
                AddIssue(
                    Result,
                    ESpellCompilerIssueSeverity::Warning,
                    FString::Printf(
                        TEXT("Node %s is in category %s but does not resolve to a known payload type."),
                        *Node.NodeId.ToString(),
                        *GetNodeCategoryString(Definition->Category)
                    ),
                    TEXT("Set CompileBehaviorTag to Payload.Projectile, Payload.Burst, Payload.Zone, Payload.Arc, or Payload.Chain."),
                    Node.NodeId
                );
            }

            break;
        }

        case ESpellNodeCategory::Flow:
        default:
            break;
        }

        Result.FlowSteps.Add(MoveTemp(Step));
        AddUniqueTags(Result.RuntimeTags, Node.RuntimeTags);
    }

    if (Result.EntryPoints.Num() == 0)
    {
        AddIssue(
            Result,
            ESpellCompilerIssueSeverity::Error,
            TEXT("Compilation produced no entry points."),
            TEXT("Add at least one valid trigger node and reconnect the graph.")
        );
    }

    Result.CompiledHash = FMD5::HashAnsiString(*BuildDeterministicSignature(Result));

    if (!Result.HasErrors() && Result.EntryPoints.Num() > 0)
    {
        AddIssue(
            Result,
            ESpellCompilerIssueSeverity::Info,
            FString::Printf(
                TEXT("Compiled spell '%s' into deterministic runtime data."),
                *Result.SourceAssetName
            ),
            TEXT("This output is ready for the future runtime executor layer.")
        );
    }

    Result.RecalculateCounts();
    Result.CompileSummary = Result.BuildSummary();
    return Result;
}

void UGWTSpellCompiler::AddIssue(
    FCompiledSpellDefinition& Result,
    ESpellCompilerIssueSeverity Severity,
    const FString& Message,
    const FString& SuggestedFix,
    const FGuid& RelatedNodeId
)
{
    Result.AddIssue(Severity, Message, SuggestedFix, RelatedNodeId);
}

ECompiledSpellPayloadType UGWTSpellCompiler::ResolvePayloadType(const USpellNodeDefinition* NodeDefinition)
{
    if (!NodeDefinition)
    {
        return ECompiledSpellPayloadType::None;
    }

    const FString CompileTag = NodeDefinition->CompileBehaviorTag.ToString();

    if (CompileTag.Contains(TEXT("Payload.Projectile"), ESearchCase::IgnoreCase) ||
        CompileTag.Contains(TEXT("Projectile"), ESearchCase::IgnoreCase))
    {
        return ECompiledSpellPayloadType::Projectile;
    }

    if (CompileTag.Contains(TEXT("Payload.Burst"), ESearchCase::IgnoreCase) ||
        CompileTag.Contains(TEXT("Burst"), ESearchCase::IgnoreCase))
    {
        return ECompiledSpellPayloadType::Burst;
    }

    if (CompileTag.Contains(TEXT("Payload.Zone"), ESearchCase::IgnoreCase) ||
        CompileTag.Contains(TEXT("Zone"), ESearchCase::IgnoreCase))
    {
        return ECompiledSpellPayloadType::Zone;
    }

    if (CompileTag.Contains(TEXT("Payload.Arc"), ESearchCase::IgnoreCase) ||
        CompileTag.Contains(TEXT("Arc"), ESearchCase::IgnoreCase))
    {
        return ECompiledSpellPayloadType::Arc;
    }

    if (CompileTag.Contains(TEXT("Payload.Chain"), ESearchCase::IgnoreCase) ||
        CompileTag.Contains(TEXT("Chain"), ESearchCase::IgnoreCase))
    {
        return ECompiledSpellPayloadType::Chain;
    }

    return ECompiledSpellPayloadType::None;
}

ESpellElement UGWTSpellCompiler::ResolveElement(const USpellNodeDefinition* NodeDefinition)
{
    if (!NodeDefinition || NodeDefinition->SupportedElements.Num() == 0)
    {
        return ESpellElement::None;
    }

    return NodeDefinition->SupportedElements[0];
}

float UGWTSpellCompiler::GetNumericOverride(const FSpellNodeInstance& Node, const FName& Key, float DefaultValue)
{
    if (const float* Value = Node.NumericOverrides.Find(Key))
    {
        return *Value;
    }

    return DefaultValue;
}

int32 UGWTSpellCompiler::GetCountOverride(const FSpellNodeInstance& Node, const FName& Key, int32 DefaultValue)
{
    if (const float* Value = Node.NumericOverrides.Find(Key))
    {
        return FMath::Max(1, FMath::RoundToInt(*Value));
    }

    return DefaultValue;
}

FString UGWTSpellCompiler::BuildDeterministicSignature(const FCompiledSpellDefinition& Result)
{
    FString Signature;
    Signature += TEXT("AssetVersion=");
    Signature += LexToString(Result.SourceAssetVersion);
    Signature += TEXT("|GraphVersion=");
    Signature += LexToString(Result.SourceGraphVersion);
    Signature += TEXT("|SpellId=");
    Signature += Result.SourceSpellId.ToString();
    Signature += TEXT("|AssetName=");
    Signature += Result.SourceAssetName;

    {
        TArray<FName> SortedTags = Result.RuntimeTags;
        SortNameArray(SortedTags);
        Signature += TEXT("|RuntimeTags=");
        Signature += JoinNames(SortedTags);
    }

    Signature += TEXT("|EntryPoints[");
    for (int32 Index = 0; Index < Result.EntryPoints.Num(); ++Index)
    {
        const FCompiledSpellEntryPoint& Entry = Result.EntryPoints[Index];
        if (Index > 0)
        {
            Signature += TEXT(";");
        }

        TArray<FGuid> SortedNext = Entry.NextStepNodeIds;
        SortGuidArray(SortedNext);

        Signature += Entry.TriggerNodeId.ToString(EGuidFormats::DigitsWithHyphensLower);
        Signature += TEXT("|");
        Signature += Entry.TriggerTag.ToString();
        Signature += TEXT("|");
        Signature += JoinGuids(SortedNext);
    }
    Signature += TEXT("]");

    Signature += TEXT("|FlowSteps[");
    for (int32 Index = 0; Index < Result.FlowSteps.Num(); ++Index)
    {
        const FCompiledSpellFlowStep& Step = Result.FlowSteps[Index];
        if (Index > 0)
        {
            Signature += TEXT(";");
        }

        TArray<FGuid> SortedNext = Step.NextStepNodeIds;
        SortGuidArray(SortedNext);

        TArray<FName> SortedTags = Step.RuntimeTags;
        SortNameArray(SortedTags);

        Signature += Step.NodeId.ToString(EGuidFormats::DigitsWithHyphensLower);
        Signature += TEXT("|");
        Signature += LexToString(static_cast<int32>(Step.Category));
        Signature += TEXT("|");
        Signature += Step.DefinitionId.ToString();
        Signature += TEXT("|");
        Signature += Step.CompileBehaviorTag.ToString();
        Signature += TEXT("|");
        Signature += JoinGuids(SortedNext);
        Signature += TEXT("|");
        Signature += JoinInts(Step.ConditionBlockIndices);
        Signature += TEXT("|");
        Signature += JoinInts(Step.VariableOpIndices);
        Signature += TEXT("|");
        Signature += JoinInts(Step.PayloadIndices);
        Signature += TEXT("|");
        Signature += JoinNames(SortedTags);
    }
    Signature += TEXT("]");

    Signature += TEXT("|ConditionBlocks[");
    for (int32 Index = 0; Index < Result.ConditionBlocks.Num(); ++Index)
    {
        const FCompiledSpellConditionBlock& Block = Result.ConditionBlocks[Index];
        if (Index > 0)
        {
            Signature += TEXT(";");
        }

        TArray<FName> SortedTags = Block.RuntimeTags;
        SortNameArray(SortedTags);

        Signature += Block.NodeId.ToString(EGuidFormats::DigitsWithHyphensLower);
        Signature += TEXT("|");
        Signature += Block.DefinitionId.ToString();
        Signature += TEXT("|");
        Signature += Block.CompileBehaviorTag.ToString();
        Signature += TEXT("|");
        Signature += JoinNames(SortedTags);
    }
    Signature += TEXT("]");

    Signature += TEXT("|VariableOps[");
    for (int32 Index = 0; Index < Result.VariableOps.Num(); ++Index)
    {
        const FCompiledSpellVariableOp& VariableOp = Result.VariableOps[Index];
        if (Index > 0)
        {
            Signature += TEXT(";");
        }

        Signature += VariableOp.NodeId.ToString(EGuidFormats::DigitsWithHyphensLower);
        Signature += TEXT("|");
        Signature += VariableOp.DefinitionId.ToString();
        Signature += TEXT("|");
        Signature += VariableOp.CompileBehaviorTag.ToString();
        Signature += TEXT("|");
        Signature += SerializeFloatMap(VariableOp.NumericOperands);
    }
    Signature += TEXT("]");

    Signature += TEXT("|PayloadSpecs[");
    for (int32 Index = 0; Index < Result.PayloadSpecs.Num(); ++Index)
    {
        const FCompiledSpellPayloadSpec& Payload = Result.PayloadSpecs[Index];
        if (Index > 0)
        {
            Signature += TEXT(";");
        }

        TArray<FName> SortedTags = Payload.RuntimeTags;
        SortNameArray(SortedTags);

        Signature += LexToString(Payload.PayloadIndex);
        Signature += TEXT("|");
        Signature += Payload.SourceNodeId.ToString(EGuidFormats::DigitsWithHyphensLower);
        Signature += TEXT("|");
        Signature += Payload.DefinitionId.ToString();
        Signature += TEXT("|");
        Signature += Payload.CompileBehaviorTag.ToString();
        Signature += TEXT("|");
        Signature += LexToString(static_cast<int32>(Payload.PayloadType));
        Signature += TEXT("|");
        Signature += LexToString(static_cast<int32>(Payload.Element));
        Signature += TEXT("|");
        Signature += FString::Printf(TEXT("%.6f"), Payload.Magnitude);
        Signature += TEXT("|");
        Signature += FString::Printf(TEXT("%.6f"), Payload.Radius);
        Signature += TEXT("|");
        Signature += FString::Printf(TEXT("%.6f"), Payload.Range);
        Signature += TEXT("|");
        Signature += FString::Printf(TEXT("%.6f"), Payload.Speed);
        Signature += TEXT("|");
        Signature += LexToString(Payload.Count);
        Signature += TEXT("|");
        Signature += JoinNames(SortedTags);
    }
    Signature += TEXT("]");

    return Signature;
}