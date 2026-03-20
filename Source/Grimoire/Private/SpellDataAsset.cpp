#include "SpellDataAsset.h"

#include "SpellCompiler.h"
#include "SpellGraphValidator.h"
#include "SpellNodeDefinition.h"

namespace
{
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

    int32 FindLayoutIndex(const TArray<FSpellNodeLayoutData>& NodeLayouts, const FGuid& NodeId)
    {
        for (int32 Index = 0; Index < NodeLayouts.Num(); ++Index)
        {
            if (NodeLayouts[Index].NodeId == NodeId)
            {
                return Index;
            }
        }

        return INDEX_NONE;
    }
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

USpellDataAsset::USpellDataAsset()
{
    DisplayName = FText::FromString(TEXT("New Spell"));
    Description = FText::FromString(TEXT("Spell graph asset."));
}

void USpellDataAsset::EnsureStableIds()
{
    if (AssetVersion <= 0)
    {
        AssetVersion = 1;
    }

    if (Graph.GraphVersion <= 0)
    {
        Graph.GraphVersion = 1;
    }

    for (FSpellNodeInstance& Node : Graph.Nodes)
    {
        if (!Node.NodeId.IsValid())
        {
            Node.NodeId = FGuid::NewGuid();
        }

        for (FSpellPinData& Pin : Node.Pins)
        {
            if (!Pin.PinId.IsValid())
            {
                Pin.PinId = FGuid::NewGuid();
            }
        }
    }

    for (FSpellEdgeData& Edge : Graph.Edges)
    {
        if (!Edge.EdgeId.IsValid())
        {
            Edge.EdgeId = FGuid::NewGuid();
        }
    }

    if (SpellId.IsNone())
    {
        SpellId = FName(*GetName());
    }
}

void USpellDataAsset::EnsureNodeLayouts()
{
    TSet<FGuid> ValidNodeIds;
    ValidNodeIds.Reserve(Graph.Nodes.Num());

    for (const FSpellNodeInstance& Node : Graph.Nodes)
    {
        if (Node.NodeId.IsValid())
        {
            ValidNodeIds.Add(Node.NodeId);
        }
    }

    TSet<FGuid> SeenLayoutIds;
    for (int32 Index = Graph.EditorData.NodeLayouts.Num() - 1; Index >= 0; --Index)
    {
        const FGuid LayoutNodeId = Graph.EditorData.NodeLayouts[Index].NodeId;
        const bool bRemoveLayout =
            !LayoutNodeId.IsValid() ||
            !ValidNodeIds.Contains(LayoutNodeId) ||
            SeenLayoutIds.Contains(LayoutNodeId);

        if (bRemoveLayout)
        {
            Graph.EditorData.NodeLayouts.RemoveAt(Index);
            continue;
        }

        SeenLayoutIds.Add(LayoutNodeId);
    }

    for (const FSpellNodeInstance& Node : Graph.Nodes)
    {
        if (!Node.NodeId.IsValid())
        {
            continue;
        }

        if (FindLayoutIndex(Graph.EditorData.NodeLayouts, Node.NodeId) != INDEX_NONE)
        {
            continue;
        }

        FSpellNodeLayoutData NewLayout;
        NewLayout.NodeId = Node.NodeId;
        NewLayout.Position = Node.GraphPosition;
        Graph.EditorData.NodeLayouts.Add(NewLayout);
    }
}

void USpellDataAsset::RebuildNodePinsFromDefinitions()
{
    for (FSpellNodeInstance& Node : Graph.Nodes)
    {
        if (!Node.NodeId.IsValid())
        {
            Node.NodeId = FGuid::NewGuid();
        }

        if (!Node.NodeDefinition)
        {
            Node.Pins.Reset();
            continue;
        }

        TArray<FSpellPinData> NewPins;
        NewPins.Reserve(Node.NodeDefinition->InputPins.Num() + Node.NodeDefinition->OutputPins.Num());

        auto CopyPinTemplate = [&Node, &NewPins](const FSpellPinData& TemplatePin)
            {
                FSpellPinData FinalPin = TemplatePin;

                if (const FSpellPinData* ExistingPin = FindPinByNameAndDirection(Node, TemplatePin.PinName, TemplatePin.Direction))
                {
                    FinalPin.PinId = ExistingPin->PinId;
                    FinalPin.DefaultValue = ExistingPin->DefaultValue;
                }

                if (!FinalPin.PinId.IsValid())
                {
                    FinalPin.PinId = FGuid::NewGuid();
                }

                NewPins.Add(FinalPin);
            };

        for (const FSpellPinData& TemplatePin : Node.NodeDefinition->InputPins)
        {
            CopyPinTemplate(TemplatePin);
        }

        for (const FSpellPinData& TemplatePin : Node.NodeDefinition->OutputPins)
        {
            CopyPinTemplate(TemplatePin);
        }

        Node.Pins = MoveTemp(NewPins);

        if (Node.NodeInstanceName.IsNone())
        {
            Node.NodeInstanceName = Node.NodeDefinition->DefinitionId.IsNone()
                ? FName(*Node.NodeDefinition->GetName())
                : Node.NodeDefinition->DefinitionId;
        }
    }
}

FGuid USpellDataAsset::AddNodeFromDefinition(USpellNodeDefinition* NodeDefinition, const FVector2D& GraphPosition, FName DesiredInstanceName)
{
    if (!NodeDefinition)
    {
        return FGuid();
    }

    FSpellNodeInstance NewNode;
    NewNode.NodeId = FGuid::NewGuid();
    NewNode.NodeDefinition = NodeDefinition;
    NewNode.NodeInstanceName = DesiredInstanceName.IsNone()
        ? (NodeDefinition->DefinitionId.IsNone() ? FName(*NodeDefinition->GetName()) : NodeDefinition->DefinitionId)
        : DesiredInstanceName;
    NewNode.GraphPosition = GraphPosition;

    Graph.Nodes.Add(MoveTemp(NewNode));

    RebuildNodePinsFromDefinitions();
    EnsureStableIds();
    ValidateGraph();

    return Graph.Nodes.Last().NodeId;
}

bool USpellDataAsset::RemoveNodeById(const FGuid& NodeId)
{
    if (!NodeId.IsValid())
    {
        return false;
    }

    const int32 RemovedNodeCount = Graph.Nodes.RemoveAll(
        [&NodeId](const FSpellNodeInstance& Node)
        {
            return Node.NodeId == NodeId;
        }
    );

    if (RemovedNodeCount <= 0)
    {
        return false;
    }

    Graph.Edges.RemoveAll(
        [&NodeId](const FSpellEdgeData& Edge)
        {
            return Edge.FromNodeId == NodeId || Edge.ToNodeId == NodeId;
        }
    );

    EnsureStableIds();
    ValidateGraph();
    return true;
}

bool USpellDataAsset::RemoveEdgeById(const FGuid& EdgeId)
{
    if (!EdgeId.IsValid())
    {
        return false;
    }

    const int32 RemovedEdgeCount = Graph.Edges.RemoveAll(
        [&EdgeId](const FSpellEdgeData& Edge)
        {
            return Edge.EdgeId == EdgeId;
        }
    );

    if (RemovedEdgeCount <= 0)
    {
        return false;
    }

    EnsureStableIds();
    ValidateGraph();
    return true;
}

bool USpellDataAsset::ConnectPinsByName(const FGuid& FromNodeId, FName FromPinName, const FGuid& ToNodeId, FName ToPinName, FString& OutError)
{
    OutError = TEXT("");

    if (!FromNodeId.IsValid() || !ToNodeId.IsValid())
    {
        OutError = TEXT("Both node ids must be valid.");
        return false;
    }

    RebuildNodePinsFromDefinitions();
    EnsureStableIds();

    const FSpellNodeInstance* FromNode = FindNode(FromNodeId);
    if (!FromNode)
    {
        OutError = FString::Printf(TEXT("Source node %s was not found."), *FromNodeId.ToString());
        return false;
    }

    const FSpellNodeInstance* ToNode = FindNode(ToNodeId);
    if (!ToNode)
    {
        OutError = FString::Printf(TEXT("Target node %s was not found."), *ToNodeId.ToString());
        return false;
    }

    const FSpellPinData* FromPin = FindPinByNameAndDirection(*FromNode, FromPinName, ESpellPinDirection::Output);
    if (!FromPin)
    {
        OutError = FString::Printf(
            TEXT("Source pin '%s' was not found as an output pin on node %s."),
            *FromPinName.ToString(),
            *FromNodeId.ToString()
        );
        return false;
    }

    const FSpellPinData* ToPin = FindPinByNameAndDirection(*ToNode, ToPinName, ESpellPinDirection::Input);
    if (!ToPin)
    {
        OutError = FString::Printf(
            TEXT("Target pin '%s' was not found as an input pin on node %s."),
            *ToPinName.ToString(),
            *ToNodeId.ToString()
        );
        return false;
    }

    if (!ArePinsCompatible(*FromPin, *ToPin))
    {
        OutError = FString::Printf(
            TEXT("Pins are incompatible (%s -> %s)."),
            *FromPin->PinName.ToString(),
            *ToPin->PinName.ToString()
        );
        return false;
    }

    for (const FSpellEdgeData& Edge : Graph.Edges)
    {
        if (Edge.FromNodeId == FromNodeId &&
            Edge.FromPinId == FromPin->PinId &&
            Edge.ToNodeId == ToNodeId &&
            Edge.ToPinId == ToPin->PinId)
        {
            ValidateGraph();
            return true;
        }

        if (!FromPin->bSupportsMultipleConnections && Edge.FromPinId == FromPin->PinId)
        {
            OutError = FString::Printf(
                TEXT("Source pin '%s' does not allow multiple outgoing connections."),
                *FromPin->PinName.ToString()
            );
            return false;
        }

        if (!ToPin->bSupportsMultipleConnections && Edge.ToPinId == ToPin->PinId)
        {
            OutError = FString::Printf(
                TEXT("Target pin '%s' does not allow multiple incoming connections."),
                *ToPin->PinName.ToString()
            );
            return false;
        }
    }

    FSpellEdgeData NewEdge;
    NewEdge.EdgeId = FGuid::NewGuid();
    NewEdge.FromNodeId = FromNodeId;
    NewEdge.FromPinId = FromPin->PinId;
    NewEdge.ToNodeId = ToNodeId;
    NewEdge.ToPinId = ToPin->PinId;

    Graph.Edges.Add(MoveTemp(NewEdge));

    EnsureStableIds();
    ValidateGraph();

    if (!bLastValidationSucceeded)
    {
        Graph.Edges.RemoveAt(Graph.Edges.Num() - 1);
        ValidateGraph();

        OutError = LastValidationMessage.IsEmpty()
            ? TEXT("Connecting the pins would make the graph invalid.")
            : LastValidationMessage;

        return false;
    }

    return true;
}

bool USpellDataAsset::DisconnectPinsByName(const FGuid& FromNodeId, FName FromPinName, const FGuid& ToNodeId, FName ToPinName)
{
    RebuildNodePinsFromDefinitions();
    EnsureStableIds();

    const FSpellNodeInstance* FromNode = FindNode(FromNodeId);
    const FSpellNodeInstance* ToNode = FindNode(ToNodeId);

    if (!FromNode || !ToNode)
    {
        return false;
    }

    const FSpellPinData* FromPin = FindPinByNameAndDirection(*FromNode, FromPinName, ESpellPinDirection::Output);
    const FSpellPinData* ToPin = FindPinByNameAndDirection(*ToNode, ToPinName, ESpellPinDirection::Input);

    if (!FromPin || !ToPin)
    {
        return false;
    }

    const int32 RemovedEdgeCount = Graph.Edges.RemoveAll(
        [FromNodeId, ToNodeId, FromPinId = FromPin->PinId, ToPinId = ToPin->PinId](const FSpellEdgeData& Edge)
        {
            return Edge.FromNodeId == FromNodeId &&
                Edge.FromPinId == FromPinId &&
                Edge.ToNodeId == ToNodeId &&
                Edge.ToPinId == ToPinId;
        }
    );

    if (RemovedEdgeCount <= 0)
    {
        return false;
    }

    EnsureStableIds();
    ValidateGraph();
    return true;
}

bool USpellDataAsset::GetPinIdByName(const FGuid& NodeId, FName PinName, ESpellPinDirection Direction, FGuid& OutPinId) const
{
    OutPinId.Invalidate();

    const FSpellNodeInstance* Node = FindNode(NodeId);
    if (!Node)
    {
        return false;
    }

    const FSpellPinData* Pin = FindPinByNameAndDirection(*Node, PinName, Direction);
    if (!Pin)
    {
        return false;
    }

    OutPinId = Pin->PinId;
    return OutPinId.IsValid();
}

void USpellDataAsset::ValidateGraph()
{
    LastValidationResult = USpellGraphValidator::RunValidation(this);
    bLastValidationSucceeded = !LastValidationResult.HasErrors();

    const FString Summary = LastValidationResult.BuildSummary();
    if (LastValidationResult.Issues.Num() > 0)
    {
        LastValidationMessage = Summary + TEXT(" First issue: ") + LastValidationResult.Issues[0].Message;
    }
    else
    {
        LastValidationMessage = Summary;
    }
}

void USpellDataAsset::CompileSpell()
{
    LastCompiledDefinition = UGWTSpellCompiler::RunCompile(this);
    bLastCompileSucceeded = LastCompiledDefinition.bCompileSucceeded;
    LastCompileMessage = LastCompiledDefinition.BuildSummary();
}

int32 USpellDataAsset::GetNodeCount() const
{
    return Graph.Nodes.Num();
}

int32 USpellDataAsset::GetEdgeCount() const
{
    return Graph.Edges.Num();
}

bool USpellDataAsset::ContainsNode(const FGuid& NodeId) const
{
    return FindNode(NodeId) != nullptr;
}

FVector2D USpellDataAsset::GetNodePosition(const FGuid& NodeId) const
{
    if (const FSpellNodeLayoutData* Layout = FindNodeLayout(NodeId))
    {
        return Layout->Position;
    }

    return FVector2D::ZeroVector;
}

void USpellDataAsset::SetNodePosition(const FGuid& NodeId, const FVector2D& NewPosition)
{
    if (FSpellNodeLayoutData* Layout = FindNodeLayoutMutable(NodeId))
    {
        Layout->Position = NewPosition;
        return;
    }

    if (!ContainsNode(NodeId))
    {
        return;
    }

    FSpellNodeLayoutData NewLayout;
    NewLayout.NodeId = NodeId;
    NewLayout.Position = NewPosition;
    Graph.EditorData.NodeLayouts.Add(NewLayout);
}

const FSpellNodeInstance* USpellDataAsset::FindNode(const FGuid& NodeId) const
{
    for (const FSpellNodeInstance& Node : Graph.Nodes)
    {
        if (Node.NodeId == NodeId)
        {
            return &Node;
        }
    }

    return nullptr;
}

FSpellNodeInstance* USpellDataAsset::FindNodeMutable(const FGuid& NodeId)
{
    for (FSpellNodeInstance& Node : Graph.Nodes)
    {
        if (Node.NodeId == NodeId)
        {
            return &Node;
        }
    }

    return nullptr;
}

const FSpellNodeLayoutData* USpellDataAsset::FindNodeLayout(const FGuid& NodeId) const
{
    const int32 LayoutIndex = FindLayoutIndex(Graph.EditorData.NodeLayouts, NodeId);
    if (LayoutIndex == INDEX_NONE)
    {
        return nullptr;
    }

    return &Graph.EditorData.NodeLayouts[LayoutIndex];
}

FSpellNodeLayoutData* USpellDataAsset::FindNodeLayoutMutable(const FGuid& NodeId)
{
    const int32 LayoutIndex = FindLayoutIndex(Graph.EditorData.NodeLayouts, NodeId);
    if (LayoutIndex == INDEX_NONE)
    {
        return nullptr;
    }

    return &Graph.EditorData.NodeLayouts[LayoutIndex];
}

void USpellDataAsset::PostLoad()
{
    Super::PostLoad();

    EnsureStableIds();
    RebuildNodePinsFromDefinitions();
    EnsureStableIds();
    EnsureNodeLayouts();
    ValidateGraph();
    CompileSpell();
}

#if WITH_EDITOR
void USpellDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    EnsureStableIds();
    RebuildNodePinsFromDefinitions();
    EnsureStableIds();
    EnsureNodeLayouts();
    ValidateGraph();
    CompileSpell();
}
#endif