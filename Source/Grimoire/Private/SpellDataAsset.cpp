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

    const FSpellPinData* FindPinByNameAndDirection(const FSpellNodeInstance& Node, const FName& PinName, ESpellPinDirection Direction)
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
}

USpellDataAsset::USpellDataAsset()
{
    DisplayName = FText::FromString(TEXT("New Spell"));
    Description = FText::FromString(TEXT("Spell graph asset."));
}

void USpellDataAsset::EnsureStableIds()
{
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

void USpellDataAsset::ValidateGraph()
{
    FString ErrorMessage;
    bLastValidationSucceeded = IsGraphStructurallyValid(ErrorMessage);

    if (bLastValidationSucceeded)
    {
        LastValidationMessage = FString::Printf(
            TEXT("Graph is structurally valid. Nodes: %d, Edges: %d"),
            Graph.Nodes.Num(),
            Graph.Edges.Num()
        );
    }
    else
    {
        LastValidationMessage = ErrorMessage;
    }
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

bool USpellDataAsset::IsGraphStructurallyValid(FString& OutError) const
{
    TSet<FGuid> SeenNodeIds;
    TSet<FGuid> SeenEdgeIds;

    if (Graph.GraphVersion <= 0)
    {
        OutError = TEXT("GraphVersion must be greater than zero.");
        return false;
    }

    for (const FSpellNodeInstance& Node : Graph.Nodes)
    {
        if (!Node.NodeId.IsValid())
        {
            OutError = TEXT("A node has an invalid NodeId.");
            return false;
        }

        if (SeenNodeIds.Contains(Node.NodeId))
        {
            OutError = FString::Printf(TEXT("Duplicate NodeId found: %s"), *Node.NodeId.ToString());
            return false;
        }

        SeenNodeIds.Add(Node.NodeId);

        if (!Node.NodeDefinition)
        {
            OutError = FString::Printf(TEXT("Node %s has no NodeDefinition assigned."), *Node.NodeId.ToString());
            return false;
        }

        TSet<FGuid> SeenPinIds;
        for (const FSpellPinData& Pin : Node.Pins)
        {
            if (!Pin.PinId.IsValid())
            {
                OutError = FString::Printf(TEXT("Node %s contains a pin with an invalid PinId."), *Node.NodeId.ToString());
                return false;
            }

            if (SeenPinIds.Contains(Pin.PinId))
            {
                OutError = FString::Printf(TEXT("Node %s contains duplicate PinIds."), *Node.NodeId.ToString());
                return false;
            }

            SeenPinIds.Add(Pin.PinId);
        }
    }

    for (const FSpellEdgeData& Edge : Graph.Edges)
    {
        if (!Edge.EdgeId.IsValid())
        {
            OutError = TEXT("An edge has an invalid EdgeId.");
            return false;
        }

        if (SeenEdgeIds.Contains(Edge.EdgeId))
        {
            OutError = FString::Printf(TEXT("Duplicate EdgeId found: %s"), *Edge.EdgeId.ToString());
            return false;
        }

        SeenEdgeIds.Add(Edge.EdgeId);

        const FSpellNodeInstance* FromNode = FindNode(Edge.FromNodeId);
        const FSpellNodeInstance* ToNode = FindNode(Edge.ToNodeId);

        if (!FromNode)
        {
            OutError = FString::Printf(TEXT("Edge %s references missing source node %s."), *Edge.EdgeId.ToString(), *Edge.FromNodeId.ToString());
            return false;
        }

        if (!ToNode)
        {
            OutError = FString::Printf(TEXT("Edge %s references missing target node %s."), *Edge.EdgeId.ToString(), *Edge.ToNodeId.ToString());
            return false;
        }

        const FSpellPinData* FromPin = FindPinById(*FromNode, Edge.FromPinId);
        const FSpellPinData* ToPin = FindPinById(*ToNode, Edge.ToPinId);

        if (!FromPin)
        {
            OutError = FString::Printf(TEXT("Edge %s references missing source pin %s."), *Edge.EdgeId.ToString(), *Edge.FromPinId.ToString());
            return false;
        }

        if (!ToPin)
        {
            OutError = FString::Printf(TEXT("Edge %s references missing target pin %s."), *Edge.EdgeId.ToString(), *Edge.ToPinId.ToString());
            return false;
        }

        if (!ArePinsCompatible(*FromPin, *ToPin))
        {
            OutError = FString::Printf(
                TEXT("Edge %s connects incompatible pins (%s -> %s)."),
                *Edge.EdgeId.ToString(),
                *FromPin->PinName.ToString(),
                *ToPin->PinName.ToString()
            );
            return false;
        }
    }

    OutError = TEXT("");
    return true;
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

void USpellDataAsset::PostLoad()
{
    Super::PostLoad();
    RebuildNodePinsFromDefinitions();
    EnsureStableIds();
    ValidateGraph();
}

#if WITH_EDITOR
void USpellDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    RebuildNodePinsFromDefinitions();
    EnsureStableIds();
    ValidateGraph();
}
#endif