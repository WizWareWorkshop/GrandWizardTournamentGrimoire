#include "SpellNodeDefinition.h"

USpellNodeDefinition::USpellNodeDefinition()
{
    DisplayName = FText::FromString(TEXT("New Spell Node Definition"));
    Description = FText::FromString(TEXT("Defines the static pins and metadata for a spell node."));
}

void USpellNodeDefinition::PostLoad()
{
    Super::PostLoad();
    NormalizePins();
}

#if WITH_EDITOR
void USpellNodeDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    NormalizePins();
}
#endif

void USpellNodeDefinition::NormalizePins()
{
    for (FSpellPinData& Pin : InputPins)
    {
        Pin.Direction = ESpellPinDirection::Input;

        if (!Pin.PinId.IsValid())
        {
            Pin.PinId = FGuid::NewGuid();
        }
    }

    for (FSpellPinData& Pin : OutputPins)
    {
        Pin.Direction = ESpellPinDirection::Output;

        if (!Pin.PinId.IsValid())
        {
            Pin.PinId = FGuid::NewGuid();
        }
    }

    if (DefinitionId.IsNone())
    {
        DefinitionId = FName(*GetName());
    }
}