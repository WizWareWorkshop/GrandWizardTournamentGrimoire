#include "GrimoireTypes.h"

void FSpellGraphValidationResult::Reset()
{
    bIsValid = false;
    ErrorCount = 0;
    WarningCount = 0;
    InfoCount = 0;
    Issues.Reset();
}

void FSpellGraphValidationResult::AddIssue(
    ESpellGraphIssueSeverity Severity,
    const FString& Message,
    const FString& SuggestedFix,
    const FGuid& RelatedNodeId,
    const FGuid& RelatedPinId,
    const FGuid& RelatedEdgeId
)
{
    FSpellGraphIssue NewIssue;
    NewIssue.Severity = Severity;
    NewIssue.Message = Message;
    NewIssue.SuggestedFix = SuggestedFix;
    NewIssue.RelatedNodeId = RelatedNodeId;
    NewIssue.RelatedPinId = RelatedPinId;
    NewIssue.RelatedEdgeId = RelatedEdgeId;

    Issues.Add(MoveTemp(NewIssue));
    RecalculateCounts();
}

FString FSpellGraphValidationResult::BuildSummary() const
{
    return FString::Printf(
        TEXT("Validation %s. Errors: %d, Warnings: %d, Infos: %d, Issues: %d"),
        bIsValid ? TEXT("succeeded") : TEXT("failed"),
        ErrorCount,
        WarningCount,
        InfoCount,
        Issues.Num()
    );
}

void FSpellGraphValidationResult::RecalculateCounts()
{
    ErrorCount = 0;
    WarningCount = 0;
    InfoCount = 0;

    for (const FSpellGraphIssue& Issue : Issues)
    {
        switch (Issue.Severity)
        {
        case ESpellGraphIssueSeverity::Error:
            ++ErrorCount;
            break;

        case ESpellGraphIssueSeverity::Warning:
            ++WarningCount;
            break;

        case ESpellGraphIssueSeverity::Info:
        default:
            ++InfoCount;
            break;
        }
    }

    bIsValid = (ErrorCount == 0);
}

void FCompiledSpellDefinition::Reset()
{
    bCompileSucceeded = false;
    ErrorCount = 0;
    WarningCount = 0;
    InfoCount = 0;
    SourceAssetVersion = 0;
    SourceGraphVersion = 0;
    SourceSpellId = NAME_None;
    SourceAssetName.Empty();
    CompiledHash.Empty();
    CompileSummary.Empty();
    Issues.Reset();
    RuntimeTags.Reset();
    EntryPoints.Reset();
    FlowSteps.Reset();
    ConditionBlocks.Reset();
    VariableOps.Reset();
    PayloadSpecs.Reset();
}

void FCompiledSpellDefinition::AddIssue(
    ESpellCompilerIssueSeverity Severity,
    const FString& Message,
    const FString& SuggestedFix,
    const FGuid& RelatedNodeId
)
{
    FSpellCompilerIssue NewIssue;
    NewIssue.Severity = Severity;
    NewIssue.Message = Message;
    NewIssue.SuggestedFix = SuggestedFix;
    NewIssue.RelatedNodeId = RelatedNodeId;

    Issues.Add(MoveTemp(NewIssue));
    RecalculateCounts();
}

void FCompiledSpellDefinition::RecalculateCounts()
{
    ErrorCount = 0;
    WarningCount = 0;
    InfoCount = 0;

    for (const FSpellCompilerIssue& Issue : Issues)
    {
        switch (Issue.Severity)
        {
        case ESpellCompilerIssueSeverity::Error:
            ++ErrorCount;
            break;

        case ESpellCompilerIssueSeverity::Warning:
            ++WarningCount;
            break;

        case ESpellCompilerIssueSeverity::Info:
        default:
            ++InfoCount;
            break;
        }
    }

    bCompileSucceeded = (ErrorCount == 0 && EntryPoints.Num() > 0);
}

FString FCompiledSpellDefinition::BuildSummary() const
{
    return FString::Printf(
        TEXT("Compile %s. Errors: %d, Warnings: %d, Infos: %d, EntryPoints: %d, FlowSteps: %d, Conditions: %d, Variables: %d, Payloads: %d"),
        bCompileSucceeded ? TEXT("succeeded") : TEXT("failed"),
        ErrorCount,
        WarningCount,
        InfoCount,
        EntryPoints.Num(),
        FlowSteps.Num(),
        ConditionBlocks.Num(),
        VariableOps.Num(),
        PayloadSpecs.Num()
    );
}