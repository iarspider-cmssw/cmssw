import FWCore.ParameterSet.Config as cms

from ..modules.hltInitialStepTrackSelectionHighPurity_cfi import hltInitialStepTrackSelectionHighPurity as _hltInitialStepTrackSelectionHighPurity
hltInitialStepTrackSelectionHighPuritypLSTCLST = _hltInitialStepTrackSelectionHighPurity.clone(
    originalMVAVals = "hltInitialStepTrackCutClassifierpLSTCLST:MVAValues",
    originalQualVals = "hltInitialStepTrackCutClassifierpLSTCLST:QualityMasks",
    originalSource = "hltInitialStepTrackspLSTCLST"
)
-- dummy change --
-- dummy change --
-- dummy change --
