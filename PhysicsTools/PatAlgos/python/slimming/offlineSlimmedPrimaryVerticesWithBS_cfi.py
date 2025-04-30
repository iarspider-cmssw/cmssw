import FWCore.ParameterSet.Config as cms

offlineSlimmedPrimaryVerticesWithBS = cms.EDProducer("PATVertexSlimmer",
    src = cms.InputTag("offlinePrimaryVerticesWithBS"),
    score = cms.InputTag("primaryVertexWithBSAssociation","original"),
)
-- dummy change --
-- dummy change --
-- dummy change --
