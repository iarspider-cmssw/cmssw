import FWCore.ParameterSet.Config as cms

process = cms.Process("COPY")

import FWCore.Framework.test.cmsExceptionsFatal_cff
process.options = FWCore.Framework.test.cmsExceptionsFatal_cff.options

process.load("FWCore.MessageLogger.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

from IOPool.Input.modules import PoolSource
process.source = PoolSource(fileNames = 'file:myout.root')

process.a1 = cms.EDAnalyzer("StreamThingAnalyzer",
    product_to_get = cms.string('m1'),
    inChecksum = cms.untracked.string('out')
)

from FWCore.Framework.modules import RunLumiEventAnalyzer
process.test = RunLumiEventAnalyzer(
    verbose = False,
    expectedRunLumiEvents = [
      1, 0, 0,
      1, 1, 0,
      1, 1, 10123456789,
      1, 1, 10123456790,
      1, 1, 10123456791,
      1, 1, 10123456792,
      1, 1, 10123456793,
      1, 1, 10123456794,
      1, 1, 10123456795,
      1, 1, 10123456796,
      1, 1, 10123456797,
      1, 1, 10123456798,
      1, 1, 10123456799,
      1, 1, 10123456800,
      1, 1, 10123456801,
      1, 1, 10123456802,
      1, 1, 10123456803,
      1, 1, 10123456804,
      1, 1, 10123456805,
      1, 1, 10123456806,
      1, 1, 10123456807,
      1, 1, 10123456808,
      1, 1, 10123456809,
      1, 1, 10123456810,
      1, 1, 10123456811,
      1, 1, 10123456812,
      1, 1, 10123456813,
      1, 1, 10123456814,
      1, 1, 10123456815,
      1, 1, 10123456816,
      1, 1, 10123456817,
      1, 1, 10123456818,
      1, 1, 10123456819,
      1, 1, 10123456820,
      1, 1, 10123456821,
      1, 1, 10123456822,
      1, 1, 10123456823,
      1, 1, 10123456824,
      1, 1, 10123456825,
      1, 1, 10123456826,
      1, 1, 10123456827,
      1, 1, 10123456828,
      1, 1, 10123456829,
      1, 1, 10123456830,
      1, 1, 10123456831,
      1, 1, 10123456832,
      1, 1, 10123456833,
      1, 1, 10123456834,
      1, 1, 10123456835,
      1, 1, 10123456836,
      1, 1, 10123456837,
      1, 1, 10123456838,
      1, 1, 0,
      1, 0, 0
    ],
    expectedEndingIndex = 162
)

from IOPool.Streamer.modules import EventStreamFileWriter
process.out = EventStreamFileWriter(
    fileName = 'teststreamfile_copy.dat',
    compression_level = 1,
    use_compression = True,
    max_event_size = 7000000
)

from IOPool.Output.modules import PoolOutputModule
process.outp = PoolOutputModule(fileName = cms.untracked.string('myout2.root'))

process.e = cms.EndPath(process.test*process.a1*process.out*process.outp)
