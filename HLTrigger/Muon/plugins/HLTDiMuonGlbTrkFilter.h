#ifndef HLTrigger_Muon_HLTDiMuonGlbTrkFilter_h
#define HLTrigger_Muon_HLTDiMuonGlbTrkFilter_h

// author D.Kovalskyi

#include "DataFormats/RecoCandidate/interface/RecoChargedCandidateFwd.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "HLTrigger/HLTcore/interface/HLTFilter.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

namespace edm {
  class ConfigurationDescriptions;
}

class HLTDiMuonGlbTrkFilter : public HLTFilter {
public:
  HLTDiMuonGlbTrkFilter(const edm::ParameterSet&);
  ~HLTDiMuonGlbTrkFilter() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  bool hltFilter(edm::Event&,
                 const edm::EventSetup&,
                 trigger::TriggerFilterObjectWithRefs& filterproduct) const override;

private:
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> const idealMagneticFieldRecordToken_;
  // WARNING: two input collection represent should be aligned and represent
  // the same list of muons, just stored in different containers
  edm::InputTag m_muonsTag;                             // input collection of muons
  edm::EDGetTokenT<reco::MuonCollection> m_muonsToken;  // input collection of muons
  edm::InputTag m_candsTag;                             // input collection of candidates to be referenced
  edm::EDGetTokenT<reco::RecoChargedCandidateCollection> m_candsToken;  // input collection of candidates to be referenced
  int m_minTrkHits;
  int m_minMuonHits;
  unsigned int m_allowedTypeMask;
  unsigned int m_requiredTypeMask;
  double m_maxNormalizedChi2;
  double m_minDR2;
  double m_minPtMuon1;
  double m_minPtMuon2;
  double m_maxEtaMuon;
  double m_maxYDimuon;
  double m_minMass;
  double m_maxMass;
  int m_chargeOpt;
  double m_maxDCAMuMu;
  double m_maxdEtaMuMu;
  muon::SelectionType m_trkMuonId;
  bool m_saveTags;
};

#endif
