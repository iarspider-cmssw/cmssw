import FWCore.ParameterSet.Config as cms

hltL1SeedsForPuppiJetFilter = cms.EDFilter("PathStatusFilter",
    logicalExpression = cms.string('pSinglePuppiJet230')
)
-- dummy change --
-- dummy change --
