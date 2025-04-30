import FWCore.ParameterSet.Config as cms

hltAK8PFPuppiJetCorrectorL2 = cms.EDProducer("LXXXCorrectorProducer",
    algorithm = cms.string('AK8PFPuppi'),
    level = cms.string('L2Relative')
)
-- dummy change --
-- dummy change --
