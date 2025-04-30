import FWCore.ParameterSet.Config as cms

hltEgammaGsfElectronsUnseeded = cms.EDProducer("EgammaHLTPixelMatchElectronProducers",
    BSProducer = cms.InputTag("hltOnlineBeamSpot"),
    GsfTrackProducer = cms.InputTag("hltEgammaGsfTracksUnseeded"),
    TrackProducer = cms.InputTag(""),
    UseGsfTracks = cms.bool(True)
)
-- dummy change --
-- dummy change --
-- dummy change --
