#ifndef Mixing_Base_PileUp_h
#define Mixing_Base_PileUp_h

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Sources/interface/VectorInputSource.h"
#include "FWCore/Utilities/interface/ESGetToken.h"
#include "DataFormats/Provenance/interface/EventID.h"
#include "FWCore/Framework/interface/EventPrincipal.h"
#include "FWCore/Framework/interface/SignallingProductRegistryFiller.h"
#include "FWCore/ServiceRegistry/interface/ServiceToken.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/ExceptionCollector.h"

#include "TH1F.h"

namespace CLHEP {
  class RandPoissonQ;
  class RandPoisson;
  class HepRandomEngine;
}  // namespace CLHEP

class MixingModuleConfig;
class MixingRcd;
class PileupRandomNumberGenerator;

namespace edm {
  class SecondaryEventProvider;
  class StreamID;
  class ProcessContext;

  struct PileUpConfig {
    PileUpConfig(std::string sourcename, double averageNumber, std::unique_ptr<TH1F>& histo, const bool playback)
        : sourcename_(sourcename), averageNumber_(averageNumber), histo_(histo.release()), playback_(playback) {}
    std::string sourcename_;
    double averageNumber_;
    std::shared_ptr<TH1F> histo_;
    const bool playback_;
  };

  class PileUp {
  public:
    explicit PileUp(ParameterSet const& pset,
                    const std::shared_ptr<PileUpConfig>& config,
                    edm::ConsumesCollector iC,
                    const bool mixingConfigFromDB);
    ~PileUp();

    template <typename T>
    void readPileUp(edm::EventID const& signal,
                    std::vector<edm::SecondaryEventIDAndFileInfo>& ids,
                    T eventOperator,
                    int const NumPU,
                    StreamID const&);

    template <typename T>
    void playPileUp(std::vector<edm::SecondaryEventIDAndFileInfo>::const_iterator begin,
                    std::vector<edm::SecondaryEventIDAndFileInfo>::const_iterator end,
                    std::vector<edm::SecondaryEventIDAndFileInfo>& ids,
                    T eventOperator);

    template <typename T>
    void playOldFormatPileUp(std::vector<edm::EventID>::const_iterator begin,
                             std::vector<edm::EventID>::const_iterator end,
                             std::vector<edm::SecondaryEventIDAndFileInfo>& ids,
                             T eventOperator);

    double averageNumber() const { return averageNumber_; }
    bool poisson() const { return poisson_; }
    bool doPileUp(int BX) {
      if (Source_type_ != "cosmics") {
        return none_ ? false : averageNumber_ > 0.;
      } else {
        return (BX >= minBunch_cosmics_ && BX <= maxBunch_cosmics_);
      }
    }
    void dropUnwantedBranches(std::vector<std::string> const& wantedBranches) {
      input_->dropUnwantedBranches(wantedBranches);
    }
    void beginJob(eventsetup::ESRecordsToProductResolverIndices const&);
    void beginStream(edm::StreamID);
    void endStream();
    void endStream(ExceptionCollector&);

    void beginRun(const edm::Run& run, const edm::EventSetup& setup);
    void beginLuminosityBlock(const edm::LuminosityBlock& lumi, const edm::EventSetup& setup);

    void endRun(const edm::Run& run, const edm::EventSetup& setup);
    void endLuminosityBlock(const edm::LuminosityBlock& lumi, const edm::EventSetup& setup);

    void setupPileUpEvent(const edm::EventSetup& setup);

    void reload(const edm::EventSetup& setup);

    void CalculatePileup(int MinBunch,
                         int MaxBunch,
                         std::vector<int>& PileupSelection,
                         std::vector<float>& TrueNumInteractions,
                         StreamID const&);

    //template<typename T>
    // void recordEventForPlayback(EventPrincipal const& eventPrincipal,
    //			  std::vector<edm::SecondaryEventIDAndFileInfo> &ids, T& eventOperator);

    const unsigned int& input() const { return inputType_; }
    void input(unsigned int s) { inputType_ = s; }

  private:
    std::unique_ptr<CLHEP::RandPoissonQ> const& poissonDistribution(StreamID const& streamID);
    std::unique_ptr<CLHEP::RandPoisson> const& poissonDistr_OOT(StreamID const& streamID);
    CLHEP::HepRandomEngine* randomEngine(StreamID const& streamID);
    void setRandomEngine(StreamID);
    void setRandomEngine(LuminosityBlock const&);

    unsigned int inputType_;
    std::string type_;
    std::string Source_type_;
    double averageNumber_;
    int const intAverage_;
    std::shared_ptr<TH1F> histo_;
    bool histoDistribution_;
    bool probFunctionDistribution_;
    bool poisson_;
    bool fixed_;
    bool none_;
    bool manage_OOT_;
    bool poisson_OOT_;
    bool fixed_OOT_;

    bool PU_Study_;
    std::string Study_type_;

    int intFixed_OOT_;
    int intFixed_ITPU_;

    int minBunch_cosmics_;
    int maxBunch_cosmics_;

    edm::ESGetToken<MixingModuleConfig, MixingRcd> configToken_;
    size_t fileNameHash_;
    std::shared_ptr<const ProductRegistry> productRegistry_;
    std::unique_ptr<VectorInputSource> const input_;
    std::shared_ptr<ProcessConfiguration> processConfiguration_;
    std::shared_ptr<ProcessContext> processContext_;
    std::shared_ptr<StreamContext> streamContext_;
    std::unique_ptr<EventPrincipal> eventPrincipal_;
    std::shared_ptr<LuminosityBlockPrincipal> lumiPrincipal_;
    std::shared_ptr<RunPrincipal> runPrincipal_;
    PileupRandomNumberGenerator* randomGenerator_ = nullptr;
    std::optional<ServiceToken> serviceToken_;
    std::unique_ptr<SecondaryEventProvider> provider_;
    std::unique_ptr<CLHEP::RandPoissonQ> PoissonDistribution_;
    std::unique_ptr<CLHEP::RandPoisson> PoissonDistr_OOT_;
    CLHEP::HepRandomEngine* randomEngine_;

    //TH1F *h1f;
    //TH1F *hprobFunction;
    //TFile *probFileHisto;

    //playback info
    bool playback_;

    // sequential reading
    bool sequential_;
  };

  template <typename T>
  class RecordEventID {
  private:
    std::vector<edm::SecondaryEventIDAndFileInfo>& ids_;
    T& eventOperator_;
    int eventCount;

  public:
    RecordEventID(std::vector<edm::SecondaryEventIDAndFileInfo>& ids, T& eventOperator)
        : ids_(ids), eventOperator_(eventOperator), eventCount(1) {}
    bool operator()(EventPrincipal const& eventPrincipal, size_t fileNameHash) {
      bool used = eventOperator_(eventPrincipal, eventCount);
      if (used) {
        ++eventCount;
        ids_.emplace_back(eventPrincipal.id(), fileNameHash);
      }
      return used;
    }
  };

  /*! Generates events from a VectorInputSource.
   *  This function decides which method of VectorInputSource 
   *  to call: sequential, random, or pre-specified.
   *  The ids are either ids to read or ids to store while reading.
   *  eventOperator has a type that matches the eventOperator in
   *  VectorInputSource::loopRandom.
   *
   *  The "signal" event is optionally used to restrict 
   *  the secondary events used for pileup and mixing.
   */
  template <typename T>
  void PileUp::readPileUp(edm::EventID const& signal,
                          std::vector<edm::SecondaryEventIDAndFileInfo>& ids,
                          T eventOperator,
                          int const pileEventCnt,
                          StreamID const& streamID) {
    // One reason PileUp is responsible for recording event IDs is
    // that it is the one that knows how many events will be read.
    ids.reserve(pileEventCnt);
    RecordEventID<T> recorder(ids, eventOperator);
    int read = 0;
    CLHEP::HepRandomEngine* engine = (sequential_ ? nullptr : randomEngine(streamID));
    read = input_->loopOverEvents(*eventPrincipal_, fileNameHash_, pileEventCnt, recorder, engine, &signal);
    if (read != pileEventCnt)
      edm::LogWarning("PileUp") << "Could not read enough pileup events: only " << read << " out of " << pileEventCnt
                                << " requested.";
  }

  template <typename T>
  void PileUp::playPileUp(std::vector<edm::SecondaryEventIDAndFileInfo>::const_iterator begin,
                          std::vector<edm::SecondaryEventIDAndFileInfo>::const_iterator end,
                          std::vector<edm::SecondaryEventIDAndFileInfo>& ids,
                          T eventOperator) {
    //TrueNumInteractions.push_back( end - begin ) ;
    RecordEventID<T> recorder(ids, eventOperator);
    input_->loopSpecified(*eventPrincipal_, fileNameHash_, begin, end, recorder);
  }

  template <typename T>
  void PileUp::playOldFormatPileUp(std::vector<edm::EventID>::const_iterator begin,
                                   std::vector<edm::EventID>::const_iterator end,
                                   std::vector<edm::SecondaryEventIDAndFileInfo>& ids,
                                   T eventOperator) {
    //TrueNumInteractions.push_back( end - begin ) ;
    RecordEventID<T> recorder(ids, eventOperator);
    input_->loopSpecified(*eventPrincipal_, fileNameHash_, begin, end, recorder);
  }

}  // namespace edm

#endif
