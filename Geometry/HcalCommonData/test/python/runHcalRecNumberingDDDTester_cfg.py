import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Run3_DDD_cff import Run3_DDD

process = cms.Process("HcalRecNumberingTest",Run3_DDD)

process.load('Configuration.Geometry.GeometryExtended2021Reco_cff')
#process.load('Configuration.Geometry.GeometryExtendedRun4D41Reco_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')

if 'MessageLogger' in process.__dict__:
    process.MessageLogger.HCalGeom=dict()

process.source = cms.Source("EmptySource")
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
    )

process.hpa = cms.EDAnalyzer("HcalRecNumberingTester")

process.Timing = cms.Service("Timing")
process.SimpleMemoryCheck = cms.Service("SimpleMemoryCheck")

process.p1 = cms.Path(process.hpa)
