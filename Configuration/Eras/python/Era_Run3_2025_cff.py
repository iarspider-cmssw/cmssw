import FWCore.ParameterSet.Config as cms

from Configuration.Eras.Era_Run3_2024_cff import Run3_2024
from Configuration.Eras.Modifier_run3_GEM_2025_cff import run3_GEM_2025
from Configuration.Eras.Modifier_run3_CSC_2025_cff import run3_CSC_2025
from Configuration.Eras.Modifier_stage2L1Trigger_2025_cff import stage2L1Trigger_2025
from Configuration.Eras.Modifier_run3_SiPixel_2025_cff import run3_SiPixel_2025
from Configuration.Eras.Modifier_run3_nanoAOD_2025_cff import run3_nanoAOD_2025
from Configuration.ProcessModifiers.ecal_cctiming_cff import ecal_cctiming

Run3_2025 = cms.ModifierChain(Run3_2024, run3_GEM_2025, stage2L1Trigger_2025, run3_SiPixel_2025, run3_CSC_2025, run3_nanoAOD_2025, ecal_cctiming)
