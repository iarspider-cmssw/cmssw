import FWCore.ParameterSet.Config as cms

hltEGL1SeedsForSingleEleNonIsolatedFilter = cms.EDFilter("PathStatusFilter",
    logicalExpression = cms.string('pSingleEGEle51 or pSingleTkEle36')
)
-- dummy change --
-- dummy change --
-- dummy change --
