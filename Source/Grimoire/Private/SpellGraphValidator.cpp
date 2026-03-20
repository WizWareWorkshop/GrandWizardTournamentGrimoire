#include "SpellGraphValidator.h"

#include "SpellDataAsset.h"
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

    const FSpellPinData* FindPinByNameAndDirection(
        const FSpellNodeInstance& Node,
        const FName& PinName,
        ESpellPinDirection Direction
    )
    {
        for (const FSpellPinData& Pin : Node.Pins)
        {
            if (Pin.PinName == PinName && Pin.Direction == Direction)
            {
                return &Pin;
            }
        }

        return nullptr;
    }

    bool ArePinsCompatible(const FSpellPinData& FromPin, const FSpellPinData& ToPin)
    {
        if (FromPin.Direction != ESpellPinDirection::Output || ToPin.Direction != ESpellPinDirection::Input)
        {
            return false;
        }

        if (FromPin.ValueType == ToPin.ValueType)
        {
            return true;
        }

        if (FromPin.AllowedConnectionTypes.Contains(ToPin.ValueType))
        {
            return true;
        }

        if (ToPin.AllowedConnectionTypes.Contains(FromPin.ValueType))
        {
            return true;
        }

        return false;
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
}

FSpellGraphValidationResult USpellGraphValidator::ValidateSpellAsset(const USpellDataAsset* SpellAsset) const
{
    return RunValidation(SpellAsset);
}

FSpellGraphValidationResult USpellGraphValidator::RunValidation(const USpellDataAsset* SpellAsset)
{
    FSpellGraphValidationResult Result;
    Result.Reset();

    if (!SpellAsset)
    {
        AddIssue(
            Result,
            ESpellGraphIssueSeverity::Error,
            TEXT("No spell asset was provided for validation."),
            TEXT("Pass a valid USpellDataAsset instance to the validator.")
        );
        return Result;
    }

    ValidateVersions(SpellAsset, Result);
    ValidateNodes(SpellAsset, Result);
    ValidateEdges(SpellAsset, Result);
    ValidateConnectivity(SpellAsset, Result);
    ValidateExecutionCycles(SpellAsset, Result);

    Result.RecalculateCounts();

    if (!Result.HasErrors())
    {
        const FString InfoMessage = Result.WarningCount > 0
            ? FString::Printf(TEXT("Spell '%s' is valid, but there are warnings to review."), *SpellAsset->GetName())
            : FString::Printf(TEXT("Spell '%s' passed validation."), *SpellAsset->GetName());

        AddIssue(
            Result,
            ESpellGraphIssueSeverity::Info,
            InfoMessage,
            TEXT("This graph is ready for the compiler layer once that layer is added.")
        );
    }

    Result.RecalculateCounts();
    return Result;
}

void USpellGraphValidator::AddIssue(
    FSpellGraphValidationResult& Result,
    ESpellGraphIssueSeverity Severity,
    const FString& Message,
    const FString& SuggestedFix,
    const FGuid& RelatedNodeId,
    const FGuid& RelatedPinId,
    const FGuid& RelatedEdgeId
)
{
    Result.AddIssue(Severity, Message, SuggestedFix, RelatedNodeId, RelatedPinId, RelatedEdgeId);
}

void USpellGraphValidator::ValidateVersions(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result)
{
    if (SpellAsset->AssetVersion <= 0)
    {
        AddIssue(
            Result,
            ESpellGraphIssueSeverity::Error,
            TEXT("AssetVersion must be greater than zero."),
            TEXT("Set AssetVersion to 1 or a later migration version.")
        );
    }

    if (SpellAsset->Graph.GraphVersion <= 0)
    {
        AddIssue(
            Result,
            ESpellGraphIssueSeverity::Error,
            TEXT("GraphVersion must be greater than zero."),
            TEXT("Set Graph.GraphVersion to 1 or a later migration version.")
        );
    }

    if (SpellAsset->SpellId.IsNone())
    {
        AddIssue(
            Result,
            ESpellGraphIssueSeverity::Warning,
            TEXT("SpellId is empty."),
            TEXT("Assign a stable SpellId or let EnsureStableIds generate one.")
        );
    }
}

void USpellGraphValidator::ValidateNodes(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result)
{
    TSet<FGuid> SeenNodeIds;
    TMap<FName, int32> DefinitionUseCounts;
    TMap<FName, const USpellNodeDefinition*> DefinitionByKey;

    for (const FSpellNodeInstance& Node : SpellAsset->Graph.Nodes)
    {
        if (!Node.NodeId.IsValid())
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                TEXT("A node has an invalid NodeId."),
                TEXT("Call EnsureStableIds on the spell asset.")
            );
            continue;
        }

        if (SeenNodeIds.Contains(Node.NodeId))
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Duplicate NodeId found: %s"), *Node.NodeId.ToString()),
                TEXT("Every graph node must have a unique id."),
                Node.NodeId
            );
            continue;
        }

        SeenNodeIds.Add(Node.NodeId);

        if (!Node.NodeDefinition)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Node %s has no NodeDefinition assigned."), *Node.NodeId.ToString()),
                TEXT("Assign a valid USpellNodeDefinition asset."),
                Node.NodeId
            );
            continue;
        }

        const FName DefinitionKey = GetDefinitionKey(Node.NodeDefinition);
        DefinitionUseCounts.FindOrAdd(DefinitionKey)++;
        DefinitionByKey.FindOrAdd(DefinitionKey) = Node.NodeDefinition;

        if (Node.NodeInstanceName.IsNone())
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Warning,
                FString::Printf(TEXT("Node %s has no NodeInstanceName."), *Node.NodeId.ToString()),
                TEXT("Give the node a stable readable instance name."),
                Node.NodeId
            );
        }

        TSet<FGuid> SeenPinIds;
        TSet<FString> SeenPinSignatures;

        for (const FSpellPinData& Pin : Node.Pins)
        {
            if (!Pin.PinId.IsValid())
            {
                AddIssue(
                    Result,
                    ESpellGraphIssueSeverity::Error,
                    FString::Printf(TEXT("Node %s contains a pin with an invalid PinId."), *Node.NodeId.ToString()),
                    TEXT("Rebuild pins from the node definition and regenerate missing pin ids."),
                    Node.NodeId
                );
                continue;
            }

            if (Pin.PinName.IsNone())
            {
                AddIssue(
                    Result,
                    ESpellGraphIssueSeverity::Error,
                    FString::Printf(TEXT("Node %s contains a pin with an empty PinName."), *Node.NodeId.ToString()),
                    TEXT("Every pin must have a stable name in the node definition."),
                    Node.NodeId,
                    Pin.PinId
                );
            }

            if (SeenPinIds.Contains(Pin.PinId))
            {
                AddIssue(
                    Result,
                    ESpellGraphIssueSeverity::Error,
                    FString::Printf(TEXT("Node %s contains duplicate PinIds."), *Node.NodeId.ToString()),
                    TEXT("Every pin instance must have a unique id."),
                    Node.NodeId,
                    Pin.PinId
                );
            }
            else
            {
                SeenPinIds.Add(Pin.PinId);
            }

            const FString Signature = FString::Printf(
                TEXT("%s|%d"),
                *Pin.PinName.ToString(),
                static_cast<int32>(Pin.Direction)
            );

            if (SeenPinSignatures.Contains(Signature))
            {
                AddIssue(
                    Result,
                    ESpellGraphIssueSeverity::Error,
                    FString::Printf(
                        TEXT("Node %s contains duplicate pins with the same name and direction (%s)."),
                        *Node.NodeId.ToString(),
                        *Pin.PinName.ToString()
                    ),
                    TEXT("Pin names must be unique per direction inside one node."),
                    Node.NodeId,
                    Pin.PinId
                );
            }
            else
            {
                SeenPinSignatures.Add(Signature);
            }
        }

        auto ValidateTemplatePins = [&](const TArray<FSpellPinData>& TemplatePins, ESpellPinDirection ExpectedDirection)
            {
                for (const FSpellPinData& TemplatePin : TemplatePins)
                {
                    const FSpellPinData* MatchingPin = FindPinByNameAndDirection(Node, TemplatePin.PinName, ExpectedDirection);
                    if (!MatchingPin)
                    {
                        AddIssue(
                            Result,
                            ESpellGraphIssueSeverity::Error,
                            FString::Printf(
                                TEXT("Node %s is missing required pin '%s' from its definition."),
                                *Node.NodeId.ToString(),
                                *TemplatePin.PinName.ToString()
                            ),
                            TEXT("Rebuild node pins from the definition."),
                            Node.NodeId
                        );
                    }
                }
            };

        ValidateTemplatePins(Node.NodeDefinition->InputPins, ESpellPinDirection::Input);
        ValidateTemplatePins(Node.NodeDefinition->OutputPins, ESpellPinDirection::Output);

        for (const FSpellPinData& ActualPin : Node.Pins)
        {
            const bool bInInputs = FindPinByNameAndDirection(Node, ActualPin.PinName, ESpellPinDirection::Input) != nullptr
                && ActualPin.Direction == ESpellPinDirection::Input
                && FindPinByNameAndDirection(Node, ActualPin.PinName, ESpellPinDirection::Input)->PinId == ActualPin.PinId;

            const bool bInOutputs = FindPinByNameAndDirection(Node, ActualPin.PinName, ESpellPinDirection::Output) != nullptr
                && ActualPin.Direction == ESpellPinDirection::Output
                && FindPinByNameAndDirection(Node, ActualPin.PinName, ESpellPinDirection::Output)->PinId == ActualPin.PinId;

            bool bExistsInDefinition = false;
            for (const FSpellPinData& TemplatePin : Node.NodeDefinition->InputPins)
            {
                if (TemplatePin.PinName == ActualPin.PinName && ActualPin.Direction == ESpellPinDirection::Input)
                {
                    bExistsInDefinition = true;
                    break;
                }
            }

            if (!bExistsInDefinition)
            {
                for (const FSpellPinData& TemplatePin : Node.NodeDefinition->OutputPins)
                {
                    if (TemplatePin.PinName == ActualPin.PinName && ActualPin.Direction == ESpellPinDirection::Output)
                    {
                        bExistsInDefinition = true;
                        break;
                    }
                }
            }

            if (!bExistsInDefinition || (!bInInputs && !bInOutputs))
            {
                AddIssue(
                    Result,
                    ESpellGraphIssueSeverity::Warning,
                    FString::Printf(
                        TEXT("Node %s contains pin '%s' that does not cleanly match its definition."),
                        *Node.NodeId.ToString(),
                        *ActualPin.PinName.ToString()
                    ),
                    TEXT("Rebuild the node pins from the node definition."),
                    Node.NodeId,
                    ActualPin.PinId
                );
            }
        }

        if (Node.NodeDefinition->Category == ESpellNodeCategory::Trigger)
        {
            bool bHasExecutionOutput = false;
            bool bHasExecutionInput = false;

            for (const FSpellPinData& Pin : Node.Pins)
            {
                if (!IsExecutionPin(Pin))
                {
                    continue;
                }

                if (Pin.Direction == ESpellPinDirection::Output)
                {
                    bHasExecutionOutput = true;
                }
                else if (Pin.Direction == ESpellPinDirection::Input)
                {
                    bHasExecutionInput = true;
                }
            }

            if (!bHasExecutionOutput)
            {
                AddIssue(
                    Result,
                    ESpellGraphIssueSeverity::Error,
                    FString::Printf(TEXT("Trigger node %s has no execution output pin."), *Node.NodeId.ToString()),
                    TEXT("Trigger nodes need at least one execution output to start spell flow."),
                    Node.NodeId
                );
            }

            if (bHasExecutionInput)
            {
                AddIssue(
                    Result,
                    ESpellGraphIssueSeverity::Warning,
                    FString::Printf(TEXT("Trigger node %s has an execution input pin."), *Node.NodeId.ToString()),
                    TEXT("Trigger nodes should usually be graph entry points, not downstream execution targets."),
                    Node.NodeId
                );
            }
        }
    }

    for (const TPair<FName, int32>& Pair : DefinitionUseCounts)
    {
        const USpellNodeDefinition* const* DefinitionPtr = DefinitionByKey.Find(Pair.Key);
        if (!DefinitionPtr || !(*DefinitionPtr))
        {
            continue;
        }

        if (!(*DefinitionPtr)->bAllowMultipleInstances && Pair.Value > 1)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(
                    TEXT("Node definition '%s' does not allow multiple instances, but %d instances were found."),
                    *Pair.Key.ToString(),
                    Pair.Value
                ),
                TEXT("Remove the extra node instances or allow multiple instances in the definition.")
            );
        }
    }
}

void USpellGraphValidator::ValidateEdges(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result)
{
    TSet<FGuid> SeenEdgeIds;
    TSet<FString> SeenEdgePairs;

    for (const FSpellEdgeData& Edge : SpellAsset->Graph.Edges)
    {
        if (!Edge.EdgeId.IsValid())
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                TEXT("An edge has an invalid EdgeId."),
                TEXT("Regenerate missing edge ids."),
                FGuid(),
                FGuid(),
                Edge.EdgeId
            );
            continue;
        }

        if (SeenEdgeIds.Contains(Edge.EdgeId))
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Duplicate EdgeId found: %s"), *Edge.EdgeId.ToString()),
                TEXT("Every graph edge must have a unique id."),
                FGuid(),
                FGuid(),
                Edge.EdgeId
            );
            continue;
        }

        SeenEdgeIds.Add(Edge.EdgeId);

        const FSpellNodeInstance* FromNode = SpellAsset->FindNode(Edge.FromNodeId);
        const FSpellNodeInstance* ToNode = SpellAsset->FindNode(Edge.ToNodeId);

        if (!FromNode)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Edge %s references missing source node %s."), *Edge.EdgeId.ToString(), *Edge.FromNodeId.ToString()),
                TEXT("Remove the edge or restore the missing node."),
                Edge.FromNodeId,
                FGuid(),
                Edge.EdgeId
            );
            continue;
        }

        if (!ToNode)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Edge %s references missing target node %s."), *Edge.EdgeId.ToString(), *Edge.ToNodeId.ToString()),
                TEXT("Remove the edge or restore the missing node."),
                Edge.ToNodeId,
                FGuid(),
                Edge.EdgeId
            );
            continue;
        }

        const FSpellPinData* FromPin = FindPinById(*FromNode, Edge.FromPinId);
        const FSpellPinData* ToPin = FindPinById(*ToNode, Edge.ToPinId);

        if (!FromPin)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Edge %s references missing source pin %s."), *Edge.EdgeId.ToString(), *Edge.FromPinId.ToString()),
                TEXT("Rebuild node pins or remove the broken edge."),
                Edge.FromNodeId,
                Edge.FromPinId,
                Edge.EdgeId
            );
            continue;
        }

        if (!ToPin)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Edge %s references missing target pin %s."), *Edge.EdgeId.ToString(), *Edge.ToPinId.ToString()),
                TEXT("Rebuild node pins or remove the broken edge."),
                Edge.ToNodeId,
                Edge.ToPinId,
                Edge.EdgeId
            );
            continue;
        }

        if (FromPin->Direction != ESpellPinDirection::Output)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Edge %s starts from non-output pin '%s'."), *Edge.EdgeId.ToString(), *FromPin->PinName.ToString()),
                TEXT("Edges must originate from output pins."),
                Edge.FromNodeId,
                Edge.FromPinId,
                Edge.EdgeId
            );
        }

        if (ToPin->Direction != ESpellPinDirection::Input)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Edge %s ends at non-input pin '%s'."), *Edge.EdgeId.ToString(), *ToPin->PinName.ToString()),
                TEXT("Edges must terminate at input pins."),
                Edge.ToNodeId,
                Edge.ToPinId,
                Edge.EdgeId
            );
        }

        if (!ArePinsCompatible(*FromPin, *ToPin))
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(
                    TEXT("Edge %s connects incompatible pins (%s -> %s)."),
                    *Edge.EdgeId.ToString(),
                    *FromPin->PinName.ToString(),
                    *ToPin->PinName.ToString()
                ),
                TEXT("Connect only pins with matching or explicitly allowed value types."),
                Edge.FromNodeId,
                Edge.FromPinId,
                Edge.EdgeId
            );
        }

        const FString EdgePairKey = Edge.FromPinId.ToString() + TEXT("->") + Edge.ToPinId.ToString();
        if (SeenEdgePairs.Contains(EdgePairKey))
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Duplicate edge connection found for %s."), *EdgePairKey),
                TEXT("Remove the duplicate connection."),
                Edge.FromNodeId,
                Edge.FromPinId,
                Edge.EdgeId
            );
        }
        else
        {
            SeenEdgePairs.Add(EdgePairKey);
        }

        if (Edge.FromPinId == Edge.ToPinId)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Edge %s connects a pin to itself."), *Edge.EdgeId.ToString()),
                TEXT("Remove self-referential edges."),
                Edge.FromNodeId,
                Edge.FromPinId,
                Edge.EdgeId
            );
        }
    }
}

void USpellGraphValidator::ValidateConnectivity(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result)
{
    int32 TriggerCount = 0;

    TMap<FGuid, int32> IncomingNodeConnections;
    TMap<FGuid, int32> OutgoingNodeConnections;
    TMap<FGuid, int32> PinConnectionCounts;

    for (const FSpellEdgeData& Edge : SpellAsset->Graph.Edges)
    {
        OutgoingNodeConnections.FindOrAdd(Edge.FromNodeId)++;
        IncomingNodeConnections.FindOrAdd(Edge.ToNodeId)++;
        PinConnectionCounts.FindOrAdd(Edge.FromPinId)++;
        PinConnectionCounts.FindOrAdd(Edge.ToPinId)++;
    }

    for (const FSpellNodeInstance& Node : SpellAsset->Graph.Nodes)
    {
        if (!Node.NodeDefinition)
        {
            continue;
        }

        const int32 IncomingConnections = IncomingNodeConnections.FindRef(Node.NodeId);
        const int32 OutgoingConnections = OutgoingNodeConnections.FindRef(Node.NodeId);
        const int32 TotalConnections = IncomingConnections + OutgoingConnections;

        if (Node.NodeDefinition->Category == ESpellNodeCategory::Trigger)
        {
            ++TriggerCount;

            if (IncomingConnections > 0)
            {
                AddIssue(
                    Result,
                    ESpellGraphIssueSeverity::Warning,
                    FString::Printf(TEXT("Trigger node %s has incoming connections."), *Node.NodeId.ToString()),
                    TEXT("Trigger nodes should usually be entry points."),
                    Node.NodeId
                );
            }
        }

        if (Node.NodeDefinition->bRequiresOutgoingConnection && OutgoingConnections == 0)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Node %s requires an outgoing connection but has none."), *Node.NodeId.ToString()),
                TEXT("Connect one of the node's output pins to a downstream node."),
                Node.NodeId
            );
        }

        const bool bCriticalCategory = Node.NodeDefinition->Category != ESpellNodeCategory::Variable;
        if (bCriticalCategory && TotalConnections == 0)
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                FString::Printf(TEXT("Node %s is isolated."), *Node.NodeId.ToString()),
                TEXT("Connect the node into the graph or remove it."),
                Node.NodeId
            );
        }

        for (const FSpellPinData& Pin : Node.Pins)
        {
            const int32 ConnectionCount = PinConnectionCounts.FindRef(Pin.PinId);
            if (!Pin.bSupportsMultipleConnections && ConnectionCount > 1)
            {
                AddIssue(
                    Result,
                    ESpellGraphIssueSeverity::Error,
                    FString::Printf(
                        TEXT("Pin '%s' on node %s does not allow multiple connections, but %d were found."),
                        *Pin.PinName.ToString(),
                        *Node.NodeId.ToString(),
                        ConnectionCount
                    ),
                    TEXT("Remove the extra connections or mark the pin as multi-connect."),
                    Node.NodeId,
                    Pin.PinId
                );
            }
        }
    }

    if (TriggerCount == 0)
    {
        AddIssue(
            Result,
            ESpellGraphIssueSeverity::Error,
            TEXT("The graph contains no trigger node."),
            TEXT("Add at least one Trigger node to define a valid entry point.")
        );
    }
}

void USpellGraphValidator::ValidateExecutionCycles(const USpellDataAsset* SpellAsset, FSpellGraphValidationResult& Result)
{
    TMap<FGuid, TArray<FGuid>> ExecutionAdjacency;

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

        if (IsExecutionPin(*FromPin) && IsExecutionPin(*ToPin))
        {
            ExecutionAdjacency.FindOrAdd(Edge.FromNodeId).AddUnique(Edge.ToNodeId);
        }
    }

    TSet<FGuid> Visiting;
    TSet<FGuid> Visited;

    TFunction<bool(const FGuid&)> VisitNode = [&](const FGuid& NodeId) -> bool
        {
            if (Visiting.Contains(NodeId))
            {
                return true;
            }

            if (Visited.Contains(NodeId))
            {
                return false;
            }

            Visiting.Add(NodeId);

            if (const TArray<FGuid>* NextNodes = ExecutionAdjacency.Find(NodeId))
            {
                for (const FGuid& NextNodeId : *NextNodes)
                {
                    if (VisitNode(NextNodeId))
                    {
                        return true;
                    }
                }
            }

            Visiting.Remove(NodeId);
            Visited.Add(NodeId);
            return false;
        };

    for (const TPair<FGuid, TArray<FGuid>>& Pair : ExecutionAdjacency)
    {
        if (VisitNode(Pair.Key))
        {
            AddIssue(
                Result,
                ESpellGraphIssueSeverity::Error,
                TEXT("An execution cycle was detected in the spell graph."),
                TEXT("Break the cycle or introduce an explicit future loop model instead of a raw execution back-edge."),
                Pair.Key
            );
            return;
        }
    }
}