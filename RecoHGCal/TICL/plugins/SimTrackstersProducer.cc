// Author: Felice Pantaleo, Leonardo Cristella - felice.pantaleo@cern.ch, leonardo.cristella@cern.ch
// Date: 09/2021

// user include files

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "DataFormats/Common/interface/OrphanHandle.h"

#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "DataFormats/ParticleFlowReco/interface/PFCluster.h"

#include "DataFormats/HGCalReco/interface/Trackster.h"
#include "DataFormats/HGCalReco/interface/TICLCandidate.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "DataFormats/Common/interface/ValueMap.h"
#include "SimDataFormats/Associations/interface/LayerClusterToSimClusterAssociator.h"
#include "SimDataFormats/Associations/interface/LayerClusterToCaloParticleAssociator.h"

#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/MtdSimTrackster.h"
#include "SimDataFormats/CaloAnalysis/interface/MtdSimTracksterFwd.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "SimDataFormats/TrackingAnalysis/interface/UniqueSimTrackId.h"

#include "SimDataFormats/Associations/interface/TrackToTrackingParticleAssociator.h"

#include "DataFormats/HGCalReco/interface/Common.h"

#include <CLHEP/Units/SystemOfUnits.h>

#include "TrackstersPCA.h"
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <numeric>

using namespace ticl;

class SimTrackstersProducer : public edm::stream::EDProducer<> {
public:
  explicit SimTrackstersProducer(const edm::ParameterSet&);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  void produce(edm::Event&, const edm::EventSetup&) override;
  void makePUTrackster(const std::vector<float>& inputClusterMask,
                       std::vector<float>& output_mask,
                       std::vector<Trackster>& result,
                       const edm::ProductID seed,
                       int loop_index);

  void addTrackster(const int index,
                    const std::vector<std::pair<edm::Ref<reco::CaloClusterCollection>, std::pair<float, float>>>& lcVec,
                    const std::vector<float>& inputClusterMask,
                    const float fractionCut_,
                    const float energy,
                    const int pdgId,
                    const int charge,
                    const float time,
                    const edm::ProductID seed,
                    const Trackster::IterationIndex iter,
                    std::vector<float>& output_mask,
                    std::vector<Trackster>& result,
                    int& loop_index,
                    const bool add = false);

private:
  std::string detector_;
  const bool doNose_ = false;
  const bool computeLocalTime_;
  const edm::EDGetTokenT<std::vector<reco::CaloCluster>> clusters_token_;
  const edm::EDGetTokenT<edm::ValueMap<std::pair<float, float>>> clustersTime_token_;
  const edm::EDGetTokenT<std::vector<float>> filtered_layerclusters_mask_token_;

  const edm::EDGetTokenT<std::vector<SimCluster>> simclusters_token_;
  const edm::EDGetTokenT<std::vector<CaloParticle>> caloparticles_token_;
  const edm::EDGetTokenT<MtdSimTracksterCollection> MTDSimTrackstersToken_;

  const edm::EDGetTokenT<ticl::SimToRecoCollectionWithSimClusters> associatorMapSimClusterToReco_token_;
  const edm::EDGetTokenT<ticl::SimToRecoCollection> associatorMapCaloParticleToReco_token_;
  const edm::ESGetToken<CaloGeometry, CaloGeometryRecord> geom_token_;
  hgcal::RecHitTools rhtools_;
  const float fractionCut_;
  const float qualityCutTrack_;
  const edm::EDGetTokenT<std::vector<TrackingParticle>> trackingParticleToken_;

  const edm::EDGetTokenT<std::vector<reco::Track>> recoTracksToken_;
  const StringCutObjectSelector<reco::Track> cutTk_;

  const edm::EDGetTokenT<reco::SimToRecoCollection> associatormapStRsToken_;
  const edm::EDGetTokenT<reco::RecoToSimCollection> associatormapRtSsToken_;
  const edm::EDGetTokenT<SimTrackToTPMap> associationSimTrackToTPToken_;
};

DEFINE_FWK_MODULE(SimTrackstersProducer);

SimTrackstersProducer::SimTrackstersProducer(const edm::ParameterSet& ps)
    : detector_(ps.getParameter<std::string>("detector")),
      doNose_(detector_ == "HFNose"),
      computeLocalTime_(ps.getParameter<bool>("computeLocalTime")),
      clusters_token_(consumes(ps.getParameter<edm::InputTag>("layer_clusters"))),
      clustersTime_token_(consumes(ps.getParameter<edm::InputTag>("time_layerclusters"))),
      filtered_layerclusters_mask_token_(consumes(ps.getParameter<edm::InputTag>("filtered_mask"))),
      simclusters_token_(consumes(ps.getParameter<edm::InputTag>("simclusters"))),
      caloparticles_token_(consumes(ps.getParameter<edm::InputTag>("caloparticles"))),
      MTDSimTrackstersToken_(consumes<MtdSimTracksterCollection>(ps.getParameter<edm::InputTag>("MtdSimTracksters"))),
      associatorMapSimClusterToReco_token_(
          consumes(ps.getParameter<edm::InputTag>("layerClusterSimClusterAssociator"))),
      associatorMapCaloParticleToReco_token_(
          consumes(ps.getParameter<edm::InputTag>("layerClusterCaloParticleAssociator"))),
      geom_token_(esConsumes()),
      fractionCut_(ps.getParameter<double>("fractionCut")),
      qualityCutTrack_(ps.getParameter<double>("qualityCutTrack")),
      trackingParticleToken_(
          consumes<std::vector<TrackingParticle>>(ps.getParameter<edm::InputTag>("trackingParticles"))),
      recoTracksToken_(consumes<std::vector<reco::Track>>(ps.getParameter<edm::InputTag>("recoTracks"))),
      cutTk_(ps.getParameter<std::string>("cutTk")),
      associatormapStRsToken_(consumes(ps.getParameter<edm::InputTag>("tpToTrack"))),
      associationSimTrackToTPToken_(consumes(ps.getParameter<edm::InputTag>("simTrackToTPMap"))) {
  produces<TracksterCollection>();
  produces<std::vector<float>>();
  produces<TracksterCollection>("fromCPs");
  produces<TracksterCollection>("PU");
  produces<std::vector<float>>("fromCPs");
  produces<std::map<uint, std::vector<uint>>>();
  produces<std::vector<TICLCandidate>>();
}

void SimTrackstersProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("detector", "HGCAL");
  desc.add<bool>("computeLocalTime", "false");
  desc.add<edm::InputTag>("layer_clusters", edm::InputTag("hgcalMergeLayerClusters"));
  desc.add<edm::InputTag>("time_layerclusters", edm::InputTag("hgcalMergeLayerClusters", "timeLayerCluster"));
  desc.add<edm::InputTag>("filtered_mask", edm::InputTag("filteredLayerClustersSimTracksters", "ticlSimTracksters"));
  desc.add<edm::InputTag>("simclusters", edm::InputTag("mix", "MergedCaloTruth"));
  desc.add<edm::InputTag>("caloparticles", edm::InputTag("mix", "MergedCaloTruth"));
  desc.add<edm::InputTag>("MtdSimTracksters", edm::InputTag("mix", "MergedMtdTruthST"));
  desc.add<edm::InputTag>("layerClusterSimClusterAssociator",
                          edm::InputTag("layerClusterSimClusterAssociationProducer"));
  desc.add<edm::InputTag>("layerClusterCaloParticleAssociator",
                          edm::InputTag("layerClusterCaloParticleAssociationProducer"));
  desc.add<edm::InputTag>("recoTracks", edm::InputTag("generalTracks"));
  desc.add<std::string>("cutTk",
                        "1.48 < abs(eta) < 3.0 && pt > 1. && quality(\"highPurity\") && "
                        "hitPattern().numberOfLostHits(\"MISSING_OUTER_HITS\") < 5");
  desc.add<edm::InputTag>("tpToTrack", edm::InputTag("trackingParticleRecoTrackAsssociation"));

  desc.add<edm::InputTag>("trackingParticles", edm::InputTag("mix", "MergedTrackTruth"));

  desc.add<edm::InputTag>("simTrackToTPMap", edm::InputTag("simHitTPAssocProducer", "simTrackToTP"));
  desc.add<double>("fractionCut", 0.);
  desc.add<double>("qualityCutTrack", 0.75);

  descriptions.addWithDefaultLabel(desc);
}
void SimTrackstersProducer::makePUTrackster(const std::vector<float>& inputClusterMask,
                                            std::vector<float>& output_mask,
                                            std::vector<Trackster>& result,
                                            const edm::ProductID seed,
                                            int loop_index) {
  Trackster tmpTrackster;
  for (size_t i = 0; i < output_mask.size(); i++) {
    const float remaining_fraction = output_mask[i];
    if (remaining_fraction > std::numeric_limits<float>::epsilon()) {
      tmpTrackster.vertices().push_back(i);
      tmpTrackster.vertex_multiplicity().push_back(1. / remaining_fraction);
    }
  }
  tmpTrackster.setSeed(seed, 0);
  result.push_back(tmpTrackster);
}

void SimTrackstersProducer::addTrackster(
    const int index,
    const std::vector<std::pair<edm::Ref<reco::CaloClusterCollection>, std::pair<float, float>>>& lcVec,
    const std::vector<float>& inputClusterMask,
    const float fractionCut_,
    const float energy,
    const int pdgId,
    const int charge,
    const float time,
    const edm::ProductID seed,
    const Trackster::IterationIndex iter,
    std::vector<float>& output_mask,
    std::vector<Trackster>& result,
    int& loop_index,
    const bool add) {
  Trackster tmpTrackster;
  if (lcVec.empty()) {
    result[index] = tmpTrackster;
    return;
  }

  tmpTrackster.vertices().reserve(lcVec.size());
  tmpTrackster.vertex_multiplicity().reserve(lcVec.size());
  for (auto const& [lc, energyScorePair] : lcVec) {
    if (inputClusterMask[lc.index()] > 0) {
      float fraction = energyScorePair.first / lc->energy();
      if (fraction < fractionCut_)
        continue;
      tmpTrackster.vertices().push_back(lc.index());
      output_mask[lc.index()] -= fraction;
      tmpTrackster.vertex_multiplicity().push_back(1. / fraction);
    }
  }

  tmpTrackster.setIdProbability(tracksterParticleTypeFromPdgId(pdgId, charge), 1.f);
  tmpTrackster.setRegressedEnergy(energy);
  tmpTrackster.setIteration(iter);
  tmpTrackster.setSeed(seed, index);
  tmpTrackster.setBoundaryTime(time);
  if (add) {
    result[index] = tmpTrackster;
    loop_index += 1;
  } else {
    result.push_back(tmpTrackster);
  }
}

void SimTrackstersProducer::produce(edm::Event& evt, const edm::EventSetup& es) {
  auto result = std::make_unique<TracksterCollection>();
  auto output_mask = std::make_unique<std::vector<float>>();
  auto result_fromCP = std::make_unique<TracksterCollection>();
  auto resultPU = std::make_unique<TracksterCollection>();
  auto output_mask_fromCP = std::make_unique<std::vector<float>>();
  auto cpToSc_SimTrackstersMap = std::make_unique<std::map<uint, std::vector<uint>>>();
  auto result_ticlCandidates = std::make_unique<std::vector<TICLCandidate>>();

  const auto& layerClustersHandle = evt.getHandle(clusters_token_);
  const auto& layerClustersTimesHandle = evt.getHandle(clustersTime_token_);
  const auto& inputClusterMaskHandle = evt.getHandle(filtered_layerclusters_mask_token_);

  // Validate input collections
  if (!layerClustersHandle.isValid() || !layerClustersTimesHandle.isValid() || !inputClusterMaskHandle.isValid()) {
    edm::LogWarning("SimTrackstersProducer") << "Missing input collections. Producing empty outputs.";

    evt.put(std::move(result_ticlCandidates));
    evt.put(std::move(output_mask));
    evt.put(std::move(result_fromCP), "fromCPs");
    evt.put(std::move(resultPU), "PU");
    evt.put(std::move(output_mask_fromCP), "fromCPs");
    evt.put(std::move(cpToSc_SimTrackstersMap));
    return;
  }

  // Proceed if inputs are valid
  const auto& layerClusters = *layerClustersHandle;
  const auto& layerClustersTimes = *layerClustersTimesHandle;
  const auto& inputClusterMask = *inputClusterMaskHandle;

  output_mask->resize(layerClusters.size(), 1.f);
  output_mask_fromCP->resize(layerClusters.size(), 1.f);

  const auto& simclusters = evt.get(simclusters_token_);
  edm::Handle<std::vector<CaloParticle>> caloParticles_h;
  evt.getByToken(caloparticles_token_, caloParticles_h);
  const auto& caloparticles = *caloParticles_h;

  edm::Handle<MtdSimTracksterCollection> MTDSimTracksters_h;
  evt.getByToken(MTDSimTrackstersToken_, MTDSimTracksters_h);

  const auto& simClustersToRecoColl = evt.get(associatorMapSimClusterToReco_token_);
  const auto& caloParticlesToRecoColl = evt.get(associatorMapCaloParticleToReco_token_);

  edm::Handle<std::vector<TrackingParticle>> trackingParticles_h;
  evt.getByToken(trackingParticleToken_, trackingParticles_h);
  edm::Handle<std::vector<reco::Track>> recoTracks_h;
  evt.getByToken(recoTracksToken_, recoTracks_h);
  const auto& TPtoRecoTrackMap = evt.get(associatormapStRsToken_);
  const auto& simTrackToTPMap = evt.get(associationSimTrackToTPToken_);
  const auto& recoTracks = *recoTracks_h;

  const auto& geom = es.getData(geom_token_);
  rhtools_.setGeometry(geom);
  const auto num_simclusters = simclusters.size();
  result->reserve(num_simclusters);  // Conservative size, will call shrink_to_fit later
  const auto num_caloparticles = caloparticles.size();
  result_fromCP->resize(num_caloparticles);
  std::map<uint, uint> SimClusterToCaloParticleMap;
  int loop_index = 0;
  for (const auto& [key, lcVec] : caloParticlesToRecoColl) {
    auto const& cp = *(key);
    auto cpIndex = &cp - &caloparticles[0];
    for (const auto& scRef : cp.simClusters()) {
      auto const& sc = *(scRef);
      auto const scIndex = &sc - &simclusters[0];
      SimClusterToCaloParticleMap[scIndex] = cpIndex;
    }

    auto regr_energy = cp.energy();
    std::vector<uint> scSimTracksterIdx;
    scSimTracksterIdx.reserve(cp.simClusters().size());

    // Create a Trackster from the object entering HGCal
    if (cp.g4Tracks()[0].crossedBoundary()) {
      regr_energy = cp.g4Tracks()[0].getMomentumAtBoundary().energy();
      float time = cp.g4Tracks()[0].getPositionAtBoundary().t() *
                   CLHEP::s;  // Geant4 time is in seconds, convert to ns (CLHEP::s = 1e9)
      addTrackster(cpIndex,
                   lcVec,
                   inputClusterMask,
                   fractionCut_,
                   regr_energy,
                   cp.pdgId(),
                   cp.charge(),
                   time,
                   key.id(),
                   ticl::Trackster::SIM,
                   *output_mask,
                   *result,
                   loop_index);
    } else {
      for (const auto& scRef : cp.simClusters()) {
        const auto& it = simClustersToRecoColl.find(scRef);
        if (it == simClustersToRecoColl.end())
          continue;
        const auto& lcVec = it->val;
        auto const& sc = *(scRef);
        auto const scIndex = &sc - &simclusters[0];

        addTrackster(scIndex,
                     lcVec,
                     inputClusterMask,
                     fractionCut_,
                     sc.g4Tracks()[0].getMomentumAtBoundary().energy(),
                     sc.pdgId(),
                     sc.charge(),
                     sc.g4Tracks()[0].getPositionAtBoundary().t() *
                         CLHEP::s,  // Geant4 time is in seconds, convert to ns (CLHEP::s = 1e9)
                     scRef.id(),
                     ticl::Trackster::SIM,
                     *output_mask,
                     *result,
                     loop_index);

        if (result->empty())
          continue;
        const auto index = result->size() - 1;
        if (std::find(scSimTracksterIdx.begin(), scSimTracksterIdx.end(), index) == scSimTracksterIdx.end()) {
          scSimTracksterIdx.emplace_back(index);
        }
      }
      scSimTracksterIdx.shrink_to_fit();
    }
    float time = cp.simTime();
    // Create a Trackster from any CP
    addTrackster(cpIndex,
                 lcVec,
                 inputClusterMask,
                 fractionCut_,
                 regr_energy,
                 cp.pdgId(),
                 cp.charge(),
                 time,
                 key.id(),
                 ticl::Trackster::SIM_CP,
                 *output_mask_fromCP,
                 *result_fromCP,
                 loop_index,
                 true);

    if (result_fromCP->empty())
      continue;
    const auto index = loop_index - 1;
    if (cpToSc_SimTrackstersMap->find(index) == cpToSc_SimTrackstersMap->end()) {
      (*cpToSc_SimTrackstersMap)[index] = scSimTracksterIdx;
    }
  }
  // TODO: remove time computation from PCA calculation and
  //       store time from boundary position in simTracksters
  ticl::assignPCAtoTracksters(*result,
                              layerClusters,
                              layerClustersTimes,
                              rhtools_.getPositionLayer(rhtools_.lastLayerEE(doNose_)).z(),
                              rhtools_,
                              computeLocalTime_);
  result->shrink_to_fit();
  ticl::assignPCAtoTracksters(*result_fromCP,
                              layerClusters,
                              layerClustersTimes,
                              rhtools_.getPositionLayer(rhtools_.lastLayerEE(doNose_)).z(),
                              rhtools_,
                              computeLocalTime_);

  makePUTrackster(inputClusterMask, *output_mask, *resultPU, caloParticles_h.id(), 0);

  auto simTrackToRecoTrack = [&](UniqueSimTrackId simTkId) -> std::pair<int, float> {
    int trackIdx = -1;
    float quality = 0.f;
    auto ipos = simTrackToTPMap.mapping.find(simTkId);
    if (ipos != simTrackToTPMap.mapping.end()) {
      auto jpos = TPtoRecoTrackMap.find((ipos->second));
      if (jpos != TPtoRecoTrackMap.end()) {
        auto& associatedRecoTracks = jpos->val;
        if (!associatedRecoTracks.empty()) {
          // associated reco tracks are sorted by decreasing quality
          if (associatedRecoTracks[0].second > qualityCutTrack_) {
            trackIdx = &(*associatedRecoTracks[0].first) - &recoTracks[0];
            quality = associatedRecoTracks[0].second;
          }
        }
      }
    }
    return {trackIdx, quality};
  };

  // Creating the map from TrackingParticle to SimTrackstersFromCP
  auto& simTrackstersFromCP = *result_fromCP;
  for (unsigned int i = 0; i < simTrackstersFromCP.size(); ++i) {
    if (simTrackstersFromCP[i].vertices().empty())
      continue;
    const auto& simTrack = caloparticles[simTrackstersFromCP[i].seedIndex()].g4Tracks()[0];
    UniqueSimTrackId simTkIds(simTrack.trackId(), simTrack.eventId());
    auto bestAssociatedRecoTrack = simTrackToRecoTrack(simTkIds);
    if (bestAssociatedRecoTrack.first != -1 and bestAssociatedRecoTrack.second > qualityCutTrack_) {
      auto trackIndex = bestAssociatedRecoTrack.first;
      simTrackstersFromCP[i].setTrackIdx(trackIndex);
    }
  }

  auto& simTracksters = *result;
  // Creating the map from TrackingParticle to SimTrackster
  std::unordered_map<unsigned int, std::vector<unsigned int>> TPtoSimTracksterMap;
  for (unsigned int i = 0; i < simTracksters.size(); ++i) {
    const auto& simTrack = (simTracksters[i].seedID() == caloParticles_h.id())
                               ? caloparticles[simTracksters[i].seedIndex()].g4Tracks()[0]
                               : simclusters[simTracksters[i].seedIndex()].g4Tracks()[0];
    UniqueSimTrackId simTkIds(simTrack.trackId(), simTrack.eventId());
    auto bestAssociatedRecoTrack = simTrackToRecoTrack(simTkIds);
    if (bestAssociatedRecoTrack.first != -1 and bestAssociatedRecoTrack.second > qualityCutTrack_) {
      auto trackIndex = bestAssociatedRecoTrack.first;
      simTracksters[i].setTrackIdx(trackIndex);
    }
  }

  edm::OrphanHandle<std::vector<Trackster>> simTracksters_h = evt.put(std::move(result));

  // map between simTrack and Mtd SimTracksters to loop on them only one
  std::unordered_map<unsigned int, const MtdSimTrackster*> SimTrackToMtdST;
  for (unsigned int i = 0; i < MTDSimTracksters_h->size(); ++i) {
    const auto& simTrack = (*MTDSimTracksters_h)[i].g4Tracks()[0];
    SimTrackToMtdST[simTrack.trackId()] = &((*MTDSimTracksters_h)[i]);
  }

  result_ticlCandidates->resize(result_fromCP->size());
  std::vector<int> toKeep;
  for (size_t i = 0; i < simTracksters_h->size(); ++i) {
    const auto& simTrackster = (*simTracksters_h)[i];
    int cp_index = (simTrackster.seedID() == caloParticles_h.id())
                       ? simTrackster.seedIndex()
                       : SimClusterToCaloParticleMap[simTrackster.seedIndex()];
    auto const& tCP = (*result_fromCP)[cp_index];
    if (!tCP.vertices().empty()) {
      auto trackIndex = tCP.trackIdx();

      auto& cand = (*result_ticlCandidates)[cp_index];
      cand.addTrackster(edm::Ptr<Trackster>(simTracksters_h, i));
      if (trackIndex != -1 && caloparticles[cp_index].charge() != 0)
        cand.setTrackPtr(edm::Ptr<reco::Track>(recoTracks_h, trackIndex));
      toKeep.push_back(cp_index);
    }
  }

  auto isHad = [](int pdgId) {
    pdgId = std::abs(pdgId);
    if (pdgId == 111)
      return false;
    return (pdgId > 100 and pdgId < 900) or (pdgId > 1000 and pdgId < 9000);
  };

  for (size_t i = 0; i < result_ticlCandidates->size(); ++i) {
    auto cp_index = (*result_fromCP)[i].seedIndex();
    if (cp_index < 0)
      continue;
    auto& cand = (*result_ticlCandidates)[i];
    const auto& cp = caloparticles[cp_index];
    float rawEnergy = 0.f;
    float regressedEnergy = 0.f;

    const auto& simTrack = cp.g4Tracks()[0];
    auto pos = SimTrackToMtdST.find(simTrack.trackId());
    if (pos != SimTrackToMtdST.end()) {
      auto MTDst = pos->second;
      // TODO: once the associators have been implemented check if the MTDst is associated with a reco before adding the MTD time
      cand.setMTDTime(MTDst->time(), 0);
    }

    cand.setTime(cp.simTime(), 0);

    for (const auto& trackster : cand.tracksters()) {
      rawEnergy += trackster->raw_energy();
      regressedEnergy += trackster->regressed_energy();
    }
    cand.setRawEnergy(rawEnergy);

    auto pdgId = cp.pdgId();
    auto charge = cp.charge();
    if (cand.trackPtr().isNonnull()) {
      auto const& track = cand.trackPtr().get();
      if (std::abs(pdgId) == 13) {
        cand.setPdgId(pdgId);
      } else {
        cand.setPdgId((isHad(pdgId) ? 211 : 11) * charge);
      }
      cand.setCharge(charge);
      math::XYZTLorentzVector p4(regressedEnergy * track->momentum().unit().x(),
                                 regressedEnergy * track->momentum().unit().y(),
                                 regressedEnergy * track->momentum().unit().z(),
                                 regressedEnergy);
      cand.setP4(p4);
    } else {  // neutral candidates
      // a neutral candidate with a charged CaloParticle is charged without a reco track associated with it
      // set the charge = 0, but keep the real pdgId to keep track of that
      if (charge != 0)
        cand.setPdgId(isHad(pdgId) ? 211 : 11);
      else if (pdgId == 111)
        cand.setPdgId(pdgId);
      else
        cand.setPdgId(isHad(pdgId) ? 130 : 22);
      cand.setCharge(0);

      auto particleType = tracksterParticleTypeFromPdgId(cand.pdgId(), 1);
      cand.setIdProbability(particleType, 1.f);

      const auto& simTracksterFromCP = (*result_fromCP)[i];
      float regressedEnergy = simTracksterFromCP.regressed_energy();
      math::XYZTLorentzVector p4(regressedEnergy * simTracksterFromCP.barycenter().unit().x(),
                                 regressedEnergy * simTracksterFromCP.barycenter().unit().y(),
                                 regressedEnergy * simTracksterFromCP.barycenter().unit().z(),
                                 regressedEnergy);
      cand.setP4(p4);
    }
  }

  std::vector<int> all_nums(result_fromCP->size());  // vector containing all caloparticles indexes
  std::iota(all_nums.begin(), all_nums.end(), 0);    // fill the vector with consecutive numbers starting from 0

  std::vector<int> toRemove;
  std::set_difference(all_nums.begin(), all_nums.end(), toKeep.begin(), toKeep.end(), std::back_inserter(toRemove));
  std::sort(toRemove.begin(), toRemove.end(), [](int x, int y) { return x > y; });
  for (auto const& r : toRemove) {
    result_fromCP->erase(result_fromCP->begin() + r);
    result_ticlCandidates->erase(result_ticlCandidates->begin() + r);
  }
  evt.put(std::move(result_ticlCandidates));
  evt.put(std::move(output_mask));
  evt.put(std::move(result_fromCP), "fromCPs");
  evt.put(std::move(resultPU), "PU");
  evt.put(std::move(output_mask_fromCP), "fromCPs");
  evt.put(std::move(cpToSc_SimTrackstersMap));
}
