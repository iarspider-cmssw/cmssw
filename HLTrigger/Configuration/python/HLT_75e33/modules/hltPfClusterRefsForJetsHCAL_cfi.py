import FWCore.ParameterSet.Config as cms

hltPfClusterRefsForJetsHCAL = cms.EDProducer("PFClusterRefCandidateProducer",
    particleType = cms.string('pi+'),
    src = cms.InputTag("hltParticleFlowClusterHCAL")
)
-- dummy change --
-- dummy change --
-- dummy change --
