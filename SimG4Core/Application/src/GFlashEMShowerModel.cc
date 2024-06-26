//
// initial setup : E.Barberio & Joanna Weng
// big changes : Soon Jun & Dongwook Jang
// V.Ivanchenko rename the class, cleanup, and move
//              to SimG4Core/Application - 2012/08/14
//
#include "SimG4Core/Application/interface/SteppingAction.h"
#include "SimG4Core/Application/interface/GFlashEMShowerModel.h"

#include "SimGeneral/GFlash/interface/GflashEMShowerProfile.h"
#include "SimGeneral/GFlash/interface/GflashHit.h"

#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4VProcess.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4TransportationManager.hh"
#include "G4EventManager.hh"
#include "G4FastSimulationManager.hh"
#include "G4TouchableHandle.hh"
#include "G4VSensitiveDetector.hh"
#include <CLHEP/Units/SystemOfUnits.h>
using CLHEP::cm;
using CLHEP::GeV;

GFlashEMShowerModel::GFlashEMShowerModel(const G4String& modelName,
                                         G4Envelope* envelope,
                                         const edm::ParameterSet& parSet)
    : G4VFastSimulationModel(modelName, envelope), theParSet(parSet) {
  theWatcherOn = parSet.getParameter<bool>("watcherOn");

  theProfile = new GflashEMShowerProfile(parSet);
  theRegion = reinterpret_cast<G4Region*>(envelope);

  theGflashStep = new G4Step();
  theGflashTouchableHandle = new G4TouchableHistory();
  theGflashNavigator = new G4Navigator();
}

// --------------------------------------------------------------------------

GFlashEMShowerModel::~GFlashEMShowerModel() {
  delete theProfile;
  delete theGflashStep;
}

G4bool GFlashEMShowerModel::IsApplicable(const G4ParticleDefinition& particleType) {
  return (&particleType == G4Electron::Electron() || &particleType == G4Positron::Positron());
}

// --------------------------------------------------------------------------
G4bool GFlashEMShowerModel::ModelTrigger(const G4FastTrack& fastTrack) {
  // Mininum energy cutoff to parameterize
  if (fastTrack.GetPrimaryTrack()->GetKineticEnergy() < Gflash::energyCutOff) {
    return false;
  }
  if (excludeDetectorRegion(fastTrack)) {
    return false;
  }

  // This will be changed accordingly when the way
  // dealing with CaloRegion changes later.
  const G4TouchableHistory* touch = static_cast<const G4TouchableHistory*>(fastTrack.GetPrimaryTrack()->GetTouchable());
  const G4VPhysicalVolume* pCurrentVolume = touch->GetVolume();
  if (pCurrentVolume == nullptr) {
    return false;
  }

  const G4LogicalVolume* lv = pCurrentVolume->GetLogicalVolume();
  if (lv->GetRegion() != theRegion) {
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
void GFlashEMShowerModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) {
  // Kill the parameterised particle:
  fastStep.KillPrimaryTrack();
  fastStep.ProposePrimaryTrackPathLength(0.0);

  // Input variables for GFlashEMShowerProfile with showerType = 1,5
  // Shower starts inside crystals
  G4double energy = fastTrack.GetPrimaryTrack()->GetKineticEnergy() / GeV;
  G4double globalTime = fastTrack.GetPrimaryTrack()->GetStep()->GetPostStepPoint()->GetGlobalTime();
  G4double charge = fastTrack.GetPrimaryTrack()->GetStep()->GetPreStepPoint()->GetCharge();
  G4ThreeVector position = fastTrack.GetPrimaryTrack()->GetPosition() / cm;
  G4ThreeVector momentum = fastTrack.GetPrimaryTrack()->GetMomentum() / GeV;
  G4int showerType = Gflash::findShowerType(position);

  // Do actual parameterization
  // The result of parameterization is gflashHitList
  theProfile->initialize(showerType, energy, globalTime, charge, position, momentum);
  theProfile->parameterization();

  // Make hits
  makeHits(fastTrack);
}

// ---------------------------------------------------------------------------
void GFlashEMShowerModel::makeHits(const G4FastTrack& fastTrack) {
  std::vector<GflashHit>& gflashHitList = theProfile->getGflashHitList();

  theGflashStep->SetTrack(const_cast<G4Track*>(fastTrack.GetPrimaryTrack()));

  theGflashStep->GetPostStepPoint()->SetProcessDefinedStep(
      const_cast<G4VProcess*>(fastTrack.GetPrimaryTrack()->GetStep()->GetPostStepPoint()->GetProcessDefinedStep()));
  theGflashNavigator->SetWorldVolume(
      G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume());

  std::vector<GflashHit>::const_iterator spotIter = gflashHitList.begin();
  std::vector<GflashHit>::const_iterator spotIterEnd = gflashHitList.end();

  for (; spotIter != spotIterEnd; ++spotIter) {
    // Put touchable for each hit so that touchable history
    //     keeps track of each step.
    theGflashNavigator->LocateGlobalPointAndUpdateTouchableHandle(
        spotIter->getPosition(), G4ThreeVector(0, 0, 0), theGflashTouchableHandle, false);
    updateGflashStep(spotIter->getPosition(), spotIter->getTime());

    // If there is a watcher defined in a job and the flag is turned on
    if (theWatcherOn) {
      SteppingAction* userSteppingAction = (SteppingAction*)G4EventManager::GetEventManager()->GetUserSteppingAction();
      userSteppingAction->m_g4StepSignal(theGflashStep);
    }

    // Send G4Step information to Hit/Digi if the volume is sensitive
    // Copied from G4SteppingManager.cc

    G4VPhysicalVolume* aCurrentVolume = theGflashStep->GetPreStepPoint()->GetPhysicalVolume();
    if (aCurrentVolume == nullptr) {
      continue;
    }

    G4LogicalVolume* lv = aCurrentVolume->GetLogicalVolume();
    if (lv->GetRegion() != theRegion) {
      continue;
    }

    theGflashStep->GetPreStepPoint()->SetSensitiveDetector(aCurrentVolume->GetLogicalVolume()->GetSensitiveDetector());
    G4VSensitiveDetector* aSensitive = theGflashStep->GetPreStepPoint()->GetSensitiveDetector();

    if (aSensitive == nullptr) {
      continue;
    }

    theGflashStep->SetTotalEnergyDeposit(spotIter->getEnergy());
    aSensitive->Hit(theGflashStep);
  }
}

// ---------------------------------------------------------------------------
void GFlashEMShowerModel::updateGflashStep(const G4ThreeVector& spotPosition, G4double timeGlobal) {
  theGflashStep->GetPostStepPoint()->SetGlobalTime(timeGlobal);
  theGflashStep->GetPreStepPoint()->SetPosition(spotPosition);
  theGflashStep->GetPostStepPoint()->SetPosition(spotPosition);
  theGflashStep->GetPreStepPoint()->SetTouchableHandle(theGflashTouchableHandle);
}

// ---------------------------------------------------------------------------
G4bool GFlashEMShowerModel::excludeDetectorRegion(const G4FastTrack& fastTrack) {
  //exclude regions where geometry are complicated
  //+- one supermodule around the EB/EE boundary: 1.479 +- 0.0174*5
  G4double eta = fastTrack.GetPrimaryTrack()->GetPosition().pseudoRapidity();
  return (std::fabs(eta) > 1.392 && std::fabs(eta) < 1.566);
}

// ---------------------------------------------------------------------------
