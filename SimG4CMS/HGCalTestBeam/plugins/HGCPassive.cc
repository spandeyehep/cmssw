///////////////////////////////////////////////////////////////////////////////
// File: HGCPassive.cc
// copied from SimG4HGCalValidation
// Description: Main analysis class for HGCal Validation of G4 Hits
///////////////////////////////////////////////////////////////////////////////

#include "HGCPassive.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "CLHEP/Units/GlobalPhysicalConstants.h"
#include "CLHEP/Units/GlobalSystemOfUnits.h"
#include "G4TransportationManager.hh"

#include "SimG4Core/Physics/interface/G4ProcessTypeEnumerator.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <utility>

//#define EDM_ML_DEBUG

HGCPassive::HGCPassive(const edm::ParameterSet& p) : topPV_(nullptr), topLV_(nullptr), count_(0), init_(false) {
  edm::ParameterSet m_Passive = p.getParameter<edm::ParameterSet>("HGCPassive");
  LVNames_ = m_Passive.getParameter<std::vector<std::string> >("LVNames");
  motherName_ = m_Passive.getParameter<std::string>("MotherName");

  //spandey
  std::string saveFile = m_Passive.getUntrackedParameter<std::string>("SaveInFile", "None");

  count_ = 0;
  event_ = 0;
  foundHadInt = false;
  
  if (saveFile != "None") {
    saveToTree = true;
    tree = bookTree (saveFile);
    edm::LogVerbatim("HGCSim") << "Instantiate CheckSecondary with first"
			       << " hadronic interaction information"
			       << " to be saved in file " << saveFile << std::endl;
  } else {
    saveToTree = false;
    edm::LogVerbatim("HGCSim") << "Instantiate CheckSecondary with first"
			       << " hadronic interaction information"
			       << " not saved" << std::endl;
  }


#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCSim") << "Name of the mother volume " << motherName_;
  unsigned k(0);
#endif
  for (const auto& name : LVNames_) {
    produces<edm::PassiveHitContainer>(Form("%sPassiveHits", name.c_str()));
#ifdef EDM_ML_DEBUG
    edm::LogVerbatim("HGCSim") << "Collection name[" << k << "] " << name;
    ++k;
#endif
  }
}

HGCPassive::~HGCPassive() {}


TTree* HGCPassive::bookTree(std::string fileName) {

  file = new TFile (fileName.c_str(), "RECREATE");
  file->cd();

  TTree * t1 = new TTree("T1", "First Hadronic interaction info");
  t1->Branch("x",  &x_);
  t1->Branch("y",  &y_);
  t1->Branch("z",  &z_);
  t1->Branch("event",  &event_);
  t1->Branch("foundHadInt",  &foundHadInt);
  t1->Branch("process", &process);
  t1->Branch("nsec", &nsec);
  t1->Branch("sec_pdgID", &sec_pdgID);
  t1->Branch("sec_charge", &sec_charge);
  t1->Branch("sec_kin", &sec_kin);
  t1->Branch("sec_x", &sec_x);
  t1->Branch("sec_y", &sec_y);
  t1->Branch("sec_z", &sec_z);

  return t1;
}

void HGCPassive::endTree() {

  edm::LogVerbatim("HGCSim") << "Save the Secondary Tree " 
			     << tree->GetName() << " (" << tree
			     << ") in file " << file->GetName() << " ("
			     << file << ")" << std::endl;


  file->cd();
  tree->Write();
  file->Close();
  delete file;
}


void HGCPassive::produce(edm::Event& e, const edm::EventSetup&) {
  for (unsigned int k = 0; k < LVNames_.size(); ++k) {
    std::unique_ptr<edm::PassiveHitContainer> hgcPH(new edm::PassiveHitContainer);
    endOfEvent(*hgcPH, k);
    e.put(std::move(hgcPH), Form("%sPassiveHits", LVNames_[k].c_str()));
  }
}

void HGCPassive::update(const BeginOfRun* run) {
  topPV_ = getTopPV();
  if (topPV_ == nullptr) {
    edm::LogWarning("HGCSim") << "Cannot find top level volume\n";
  } else {
    init_ = true;
    const G4LogicalVolumeStore* lvs = G4LogicalVolumeStore::GetInstance();
    for (auto lvcite : *lvs) {
      findLV(lvcite);
    }

#ifdef EDM_ML_DEBUG
    edm::LogVerbatim("HGCSim") << "HGCPassive::Finds " << mapLV_.size() << " logical volumes";
    unsigned int k(0);
    for (const auto& lvs : mapLV_) {
      edm::LogVerbatim("HGCSim") << "Entry[" << k << "] " << lvs.first << ": (" << (lvs.second).first << ", "
                                 << (lvs.second).second << ")";
      ++k;
    }
#endif
  }

}

//=================================================================== per EVENT
void HGCPassive::update(const BeginOfEvent* evt) {
  int iev = (*evt)()->GetEventID();
  edm::LogVerbatim("HGCSim") << "HGCPassive: =====> Begin event = " << iev << std::endl;

  ++count_;
  store_.clear();
  nsec = 0;
  process = 0;
  sec_pdgID.clear();
  sec_charge.clear();
  sec_kin.clear();
  sec_x.clear();
  sec_y.clear();
  sec_z.clear();
  x_ = -999999.0;
  y_ = -999999.0;
  z_ = -999999.0;
  ++event_;
  foundHadInt = false;


}

// //=================================================================== each
// STEP
void HGCPassive::update(const G4Step* aStep) {
  if (aStep != nullptr) {

    ///from spandey
    const G4TrackVector* tkV  = aStep->GetSecondary();
    G4Track* thTk = aStep->GetTrack();
    const G4StepPoint* postStepPoint = aStep->GetPostStepPoint();
    if (tkV != nullptr && postStepPoint != nullptr) {
      int nsc = (*tkV).size();
      const G4VProcess*  proc = postStepPoint->GetProcessDefinedStep();
      G4ProcessTypeEnumerator* typeEnumerator = new G4ProcessTypeEnumerator();
      if (proc != nullptr) {
	// G4ProcessType type = proc->GetProcessType();
	int procid = typeEnumerator->processIdLong(proc);
	std::string name = proc->GetProcessName();
	if(thTk->GetParentID() <= 0 && procid >= 121 && procid <= 151) {

	  foundHadInt = true;
	  process = procid;

	  x_ = postStepPoint->GetPosition().getX()/10.0; // Convert to cm
	  y_ = postStepPoint->GetPosition().getY()/10.0; // Convert to cm
	  z_ = (postStepPoint->GetPosition().getZ() - 25000.0)/10.0; // Bring to HGCAL front then Convert to cm
	  edm::LogVerbatim("HGCSim") <<"*** First hadronic interaction ****"  <<std::endl
				     <<" Particle =  "<<thTk->GetDefinition()->GetParticleName()
				     <<" Process ID = "<<procid
				     <<" Process name = "<<typeEnumerator->processG4Name(procid)
				     <<" # of secondaries = "<<nsc
				     <<" at (x,y,z) = ("<<x_<<" ,"<<y_<<" ,"<<z_<<")"<<std::endl;

	  for(int i = 0; i < nsc; i++) {
	    G4Track *tk = (*tkV)[i];
	    if(tk->GetCreatorProcess()->GetProcessName() == "hIoni") continue;
	    nsec++;
	    sec_pdgID.push_back(tk->GetDefinition()->GetPDGEncoding());
	    sec_charge.push_back(tk->GetDefinition()->GetPDGCharge());
	    sec_x.push_back(tk->GetPosition().getX()/10.0);  // Convert to cm
	    sec_y.push_back(tk->GetPosition().getY()/10.0);  // Convert to cm
	    sec_z.push_back((tk->GetPosition().getZ() - 25000.0)/10.0); // Bring to HGCAL front then Convert to cm
	    // CE-E proto front is at z = 25000 mm in the simulation and we want it be at z = 0 mm.
	    sec_kin.push_back(tk->GetKineticEnergy()/GeV);
#ifdef EDM_ML_DEBUG  
	    edm::LogVerbatim("HGCSim") << " ID " << tk->GetTrackID() 
				       << " ParentID  "<< tk->GetParentID()
				       << " Status "<< tk->GetTrackStatus() 
				       << " Particle "<< tk->GetDefinition()->GetParticleName()
				       << " PDGID "<< tk->GetDefinition()->GetPDGEncoding()
				       << " PDGCharge "<< tk->GetDefinition()->GetPDGCharge()
				       << " Position(mm) " << tk->GetPosition() 
				       << " KE " << tk->GetKineticEnergy()/GeV<<" GeV "<<std::endl;
#endif
	        
	  }


	}
      }
    }





    //From sunanda
    G4VSensitiveDetector* curSD = aStep->GetPreStepPoint()->GetSensitiveDetector();
    const G4VTouchable* touchable = aStep->GetPreStepPoint()->GetTouchable();

    if (curSD == nullptr) {
      G4LogicalVolume* plv = touchable->GetVolume()->GetLogicalVolume();
      auto it = (init_) ? mapLV_.find(plv) : findLV(plv);
      double time = aStep->GetTrack()->GetGlobalTime();
      double energy = (aStep->GetTotalEnergyDeposit()) / CLHEP::GeV;

      unsigned int copy(0);
      if (((aStep->GetPostStepPoint() == nullptr) || (aStep->GetTrack()->GetNextVolume() == nullptr)) &&
          (aStep->IsLastStepInVolume())) {
#ifdef EDM_ML_DEBUG
        edm::LogVerbatim("HGCSim") << plv->GetName() << " F|L Step " << aStep->IsFirstStepInVolume() << ":"
                                   << aStep->IsLastStepInVolume() << " Position"
                                   << aStep->GetPreStepPoint()->GetPosition() << " Track "
                                   << aStep->GetTrack()->GetDefinition()->GetParticleName() << " at"
                                   << aStep->GetTrack()->GetPosition() << " Volume " << aStep->GetTrack()->GetVolume()
                                   << ":" << aStep->GetTrack()->GetNextVolume() << " Status "
                                   << aStep->GetTrack()->GetTrackStatus() << " KE "
                                   << aStep->GetTrack()->GetKineticEnergy() << " Deposit "
                                   << aStep->GetTotalEnergyDeposit() << " Map " << (it != mapLV_.end());
#endif
        energy += (aStep->GetPreStepPoint()->GetKineticEnergy() / CLHEP::GeV);

	//spandey
	// leakage information (asign different copy as identifier)
	double zpos = (double)aStep->GetPreStepPoint()->GetPosition().getZ();
	if(zpos < 25000.0) { // before EE
	    copy = 0;
	}
	else if(zpos >= 25000.0 && zpos < 25630.0) { /// transverse leakage in EE
	  copy = 10;
	}
	else if(zpos >= 25630.0 && zpos < 26610.0) { /// transverse leakage in FH
	  copy = 20;
	}
	else if(zpos >= 26610.0 && zpos < 28115) { /// transverse leakage in AH
	  copy = 30;
	}
	else { /// longitudinal leakage
	  copy = 40;
	}




      } else {
        time = (aStep->GetPostStepPoint()->GetGlobalTime());
        copy = (unsigned int)(touchable->GetReplicaNumber(0) + 1000 * touchable->GetReplicaNumber(1));
      }
      if (it != mapLV_.end()) {
        storeInfo(it, plv, copy, time, energy, true);
      } else if (topLV_ != nullptr) {
        auto itr = findLV(topLV_);
        if (itr != mapLV_.end()) {
          storeInfo(itr, topLV_, copy, time, energy, true);
        }
      }
    }  // if (curSD==NULL)

    // Now for the mother volumes
    int level = (touchable->GetHistoryDepth());
    if (level > 0) {
      double energy = (aStep->GetTotalEnergyDeposit()) / CLHEP::GeV;
      double time = (aStep->GetTrack()->GetGlobalTime());

      for (int i = level; i > 0; --i) {
        G4LogicalVolume* plv = touchable->GetVolume(i)->GetLogicalVolume();
        auto it = (init_) ? mapLV_.find(plv) : findLV(plv);
#ifdef EDM_ML_DEBUG
        edm::LogVerbatim("HGCSim") << "Level: " << level << ":" << i << " " << plv->GetName() << " flag in the List "
                                   << (it != mapLV_.end());
#endif
        if (it != mapLV_.end()) {
          unsigned int copy =
              (i == level) ? 0
                           : (unsigned int)(touchable->GetReplicaNumber(i) + 1000 * touchable->GetReplicaNumber(i + 1));
          storeInfo(it, plv, copy, time, energy, false);
        }
      }
    }
  }  // if (aStep != NULL)

}  // end update aStep

//================================================================ End of EVENT

void HGCPassive::endOfEvent(edm::PassiveHitContainer& hgcPH, unsigned int k) {
#ifdef EDM_ML_DEBUG
  unsigned int kount(0);
#endif
  for (const auto& element : store_) {
    G4LogicalVolume* lv = (element.first).first;
    auto it = mapLV_.find(lv);
    if (it != mapLV_.end()) {
      if ((it->second).first == k) {
        PassiveHit hit(
            (it->second).second, (element.first).second, (element.second)[1], (element.second)[2], (element.second)[0]);
        hgcPH.push_back(hit);
#ifdef EDM_ML_DEBUG
        edm::LogVerbatim("HGCSim") << "HGCPassive[" << k << "] Hit[" << kount << "] " << hit;
        ++kount;
#endif
      }
    }
  }
}


// ======================================= End of event by spandey
void HGCPassive::update(const EndOfEvent * evt) {
  if (saveToTree) tree->Fill();

}

// ======================================= End of Run by spandey
void HGCPassive::update(const EndOfRun * run) {
    if (saveToTree)     endTree();

}


G4VPhysicalVolume* HGCPassive::getTopPV() {
  return G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume();
}

HGCPassive::volumeIterator HGCPassive::findLV(G4LogicalVolume* plv) {
  auto itr = mapLV_.find(plv);
  if (itr == mapLV_.end()) {
    std::string name = plv->GetName();
    for (unsigned int k = 0; k < LVNames_.size(); ++k) {
      if (name.find(LVNames_[k]) != std::string::npos) {
        mapLV_[plv] = std::pair<unsigned int, std::string>(k, name);
        itr = mapLV_.find(plv);
        break;
      }
    }
  }
  if (topLV_ == nullptr) {
    if (std::string(plv->GetName()) == motherName_)
      topLV_ = plv;
  }
  return itr;
}

void HGCPassive::storeInfo(const HGCPassive::volumeIterator it,
                           G4LogicalVolume* plv,
                           unsigned int copy,
                           double time,
                           double energy,
                           bool flag) {
  std::pair<G4LogicalVolume*, unsigned int> key(plv, copy);
  auto itr = store_.find(key);
  double ee = (flag) ? energy : 0;
  if (itr == store_.end()) {
    store_[key] = {{time, energy, energy}};
  } else {
    (itr->second)[1] += ee;
    (itr->second)[2] += energy;
  }
#ifdef EDM_ML_DEBUG
  itr = store_.find(key);
  edm::LogVerbatim("HGCSim") << "HGCPassive: Element " << (it->second).first << ":" << (it->second).second << ":"
                             << copy << " T " << (itr->second)[0] << " E " << (itr->second)[1] << ":"
                             << (itr->second)[2];
#endif
}

DEFINE_SIMWATCHER(HGCPassive);
