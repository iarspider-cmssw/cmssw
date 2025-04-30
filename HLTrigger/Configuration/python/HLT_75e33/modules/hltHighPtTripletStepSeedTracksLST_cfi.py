import FWCore.ParameterSet.Config as cms

hltHighPtTripletStepSeedTracksLST = cms.EDProducer(
    "TrackFromSeedProducer",
    src = cms.InputTag("hltHighPtTripletStepSeeds"),
    beamSpot = cms.InputTag("hltOnlineBeamSpot"),
    TTRHBuilder = cms.string("hltESPTTRHBuilderWithoutRefit")
)
-- dummy change --
-- dummy change --
-- dummy change --
