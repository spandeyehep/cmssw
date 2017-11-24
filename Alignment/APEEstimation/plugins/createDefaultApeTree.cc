
// -*- C++ -*-
//
// Package:    DefaultApeTree
// Class:      DefaultApeTree
// 
/**\class DefaultApeTree DefaultApeTree.cc Alignment/APEEstimation/src/createDefaultApeTree.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Marius Teroerde (code from ApeEstimator.cc and
//                   ApeEstimatorSummary.cc)
//         Created:  Tue Nov 14 11:43 CET 2017
//
//


// system include files
#include <memory>
#include <fstream>
#include <sstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/EDMException.h"

#include "CLHEP/Matrix/SymMatrix.h"

#include "CondFormats/AlignmentRecord/interface/TrackerAlignmentErrorExtendedRcd.h"
#include "CondFormats/Alignment/interface/AlignmentErrorsExtended.h"

//...............
#include "Alignment/APEEstimation/interface/TrackerSectorStruct.h"
#include "Alignment/APEEstimation/interface/ReducedTrackerTreeVariables.h"

#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/SiStripDetId/interface/SiStripDetId.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"
#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"


#include "TString.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TTree.h"
#include "TMath.h"
//
// class declaration
//

class DefaultApeTree : public edm::one::EDAnalyzer<> {
  public:
    explicit DefaultApeTree(const edm::ParameterSet&);
    ~DefaultApeTree() override;
  
  
  private:
    void beginJob() override ;
    void analyze(const edm::Event&, const edm::EventSetup&) override;
    void endJob() override ;
    
    void sectorBuilder();
    bool checkIntervalsForSectors(const unsigned int sectorCounter, const std::vector<double>&)const;
    bool checkModuleIds(const unsigned int, const std::vector<unsigned int>&)const;
    bool checkModuleBools(const bool, const std::vector<unsigned int>&)const;
    bool checkModuleDirections(const int, const std::vector<int>&)const;
    bool checkModulePositions(const float, const std::vector<double>&)const;
    
    // ----------member data ---------------------------
    const edm::ParameterSet parameterSet_;
    
    std::map<unsigned int, TrackerSectorStruct> m_tkSector_;
    std::map<unsigned int, ReducedTrackerTreeVariables> m_tkTreeVar_;
};              

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
DefaultApeTree::DefaultApeTree(const edm::ParameterSet& iConfig):
parameterSet_(iConfig)
{
}


DefaultApeTree::~DefaultApeTree()
{
}


//
// member functions
//
void
DefaultApeTree::sectorBuilder()
{
  // Same procedure as in ApeEstimator.cc
  TFile* tkTreeFile(TFile::Open((parameterSet_.getParameter<std::string>("TrackerTreeFile")).c_str()));
  if(tkTreeFile){
    edm::LogInfo("SectorBuilder")<<"TrackerTreeFile OK";
  }else{
    edm::LogError("SectorBuilder")<<"TrackerTreeFile not found";
    return;
  }
  TTree* tkTree(nullptr);
  tkTreeFile->GetObject("TrackerTreeGenerator/TrackerTree/TrackerTree",tkTree);
  if(tkTree){
    edm::LogInfo("SectorBuilder")<<"TrackerTree OK";
  }else{
    edm::LogError("SectorBuilder")<<"TrackerTree not found in file";
    return;
  }
  UInt_t rawId(999), subdetId(999), layer(999), side(999), half(999), rod(999), ring(999), petal(999),
         blade(999), panel(999), outerInner(999), module(999), nStrips(999);
  Bool_t isDoubleSide(false), isRPhi(false), isStereo(false);
  Int_t uDirection(999), vDirection(999), wDirection(999);
  Float_t posR(999.F), posPhi(999.F), posEta(999.F), posX(999.F), posY(999.F), posZ(999.F); 
  tkTree->SetBranchAddress("RawId", &rawId);
  tkTree->SetBranchAddress("SubdetId", &subdetId);
  tkTree->SetBranchAddress("Layer", &layer);
  tkTree->SetBranchAddress("Side", &side);
  tkTree->SetBranchAddress("Half", &half);
  tkTree->SetBranchAddress("Rod", &rod);
  tkTree->SetBranchAddress("Ring", &ring);
  tkTree->SetBranchAddress("Petal", &petal);
  tkTree->SetBranchAddress("Blade", &blade);
  tkTree->SetBranchAddress("Panel", &panel);
  tkTree->SetBranchAddress("OuterInner", &outerInner);
  tkTree->SetBranchAddress("Module", &module);
  tkTree->SetBranchAddress("NStrips", &nStrips);
  tkTree->SetBranchAddress("IsDoubleSide", &isDoubleSide);
  tkTree->SetBranchAddress("IsRPhi", &isRPhi);
  tkTree->SetBranchAddress("IsStereo", &isStereo);
  tkTree->SetBranchAddress("UDirection", &uDirection);
  tkTree->SetBranchAddress("VDirection", &vDirection);
  tkTree->SetBranchAddress("WDirection", &wDirection);
  tkTree->SetBranchAddress("PosR", &posR);
  tkTree->SetBranchAddress("PosPhi", &posPhi);
  tkTree->SetBranchAddress("PosEta", &posEta);
  tkTree->SetBranchAddress("PosX", &posX);
  tkTree->SetBranchAddress("PosY", &posY);
  tkTree->SetBranchAddress("PosZ", &posZ);
  
  Int_t nModules(tkTree->GetEntries());
  TrackerSectorStruct allSectors;
  
  //Loop over all Sectors
  unsigned int sectorCounter(1);
  std::vector<edm::ParameterSet> v_sectorDef(parameterSet_.getParameter<std::vector<edm::ParameterSet> >("Sectors"));
  edm::LogInfo("SectorBuilder")<<"There are "<<v_sectorDef.size()<<" Sectors defined";
  std::vector<edm::ParameterSet>::const_iterator i_parSet;
  for(i_parSet = v_sectorDef.begin(); i_parSet != v_sectorDef.end();++i_parSet, ++sectorCounter){
    const edm::ParameterSet& parSet = *i_parSet;
    const std::string& sectorName(parSet.getParameter<std::string>("name"));
    std::vector<unsigned int> v_rawId(parSet.getParameter<std::vector<unsigned int> >("rawId")),
            v_subdetId(parSet.getParameter<std::vector<unsigned int> >("subdetId")),
            v_layer(parSet.getParameter<std::vector<unsigned int> >("layer")),
            v_side(parSet.getParameter<std::vector<unsigned int> >("side")),
            v_half(parSet.getParameter<std::vector<unsigned int> >("half")),
            v_rod(parSet.getParameter<std::vector<unsigned int> >("rod")),
            v_ring(parSet.getParameter<std::vector<unsigned int> >("ring")),
            v_petal(parSet.getParameter<std::vector<unsigned int> >("petal")),
            v_blade(parSet.getParameter<std::vector<unsigned int> >("blade")),
            v_panel(parSet.getParameter<std::vector<unsigned int> >("panel")),
            v_outerInner(parSet.getParameter<std::vector<unsigned int> >("outerInner")),
            v_module(parSet.getParameter<std::vector<unsigned int> >("module")),
            v_nStrips(parSet.getParameter<std::vector<unsigned int> >("nStrips")),
            v_isDoubleSide(parSet.getParameter<std::vector<unsigned int> >("isDoubleSide")),
            v_isRPhi(parSet.getParameter<std::vector<unsigned int> >("isRPhi")),
            v_isStereo(parSet.getParameter<std::vector<unsigned int> >("isStereo"));
    std::vector<int>  v_uDirection(parSet.getParameter<std::vector<int> >("uDirection")),
            v_vDirection(parSet.getParameter<std::vector<int> >("vDirection")),
            v_wDirection(parSet.getParameter<std::vector<int> >("wDirection"));
    std::vector<double> v_posR(parSet.getParameter<std::vector<double> >("posR")),
            v_posPhi(parSet.getParameter<std::vector<double> >("posPhi")),
            v_posEta(parSet.getParameter<std::vector<double> >("posEta")),
            v_posX(parSet.getParameter<std::vector<double> >("posX")),
            v_posY(parSet.getParameter<std::vector<double> >("posY")),
            v_posZ(parSet.getParameter<std::vector<double> >("posZ"));
    
    if(!this->checkIntervalsForSectors(sectorCounter,v_posR) || !this->checkIntervalsForSectors(sectorCounter,v_posPhi) ||
       !this->checkIntervalsForSectors(sectorCounter,v_posEta) || !this->checkIntervalsForSectors(sectorCounter,v_posX) ||
       !this->checkIntervalsForSectors(sectorCounter,v_posY)   || !this->checkIntervalsForSectors(sectorCounter,v_posZ)){
      continue;
    }
    
    
    TrackerSectorStruct tkSector;
    tkSector.name = sectorName;
    
    ReducedTrackerTreeVariables tkTreeVar;
    
    //Loop over all Modules
    for(Int_t module = 0; module < nModules; ++module){
      tkTree->GetEntry(module);
      
      if(sectorCounter==1){
        tkTreeVar.subdetId = subdetId;
        tkTreeVar.nStrips = nStrips;
        tkTreeVar.uDirection = uDirection;
        tkTreeVar.vDirection = vDirection;
        tkTreeVar.wDirection = wDirection;
        m_tkTreeVar_[rawId] = tkTreeVar;
      }
      //Check if modules from Sector builder equal those from TrackerTree
      if(!this->checkModuleIds(rawId,v_rawId))continue;
      if(!this->checkModuleIds(subdetId,v_subdetId))continue;
      if(!this->checkModuleIds(layer,v_layer))continue;
      if(!this->checkModuleIds(side,v_side))continue;
      if(!this->checkModuleIds(half,v_half))continue;
      if(!this->checkModuleIds(rod,v_rod))continue;
      if(!this->checkModuleIds(ring,v_ring))continue;
      if(!this->checkModuleIds(petal,v_petal))continue;
      if(!this->checkModuleIds(blade,v_blade))continue;
      if(!this->checkModuleIds(panel,v_panel))continue;
      if(!this->checkModuleIds(outerInner,v_outerInner))continue;
      if(!this->checkModuleIds(module,v_module))continue;
      if(!this->checkModuleIds(nStrips,v_nStrips))continue;
      if(!this->checkModuleBools(isDoubleSide,v_isDoubleSide))continue;
      if(!this->checkModuleBools(isRPhi,v_isRPhi))continue;
      if(!this->checkModuleBools(isStereo,v_isStereo))continue;
      if(!this->checkModuleDirections(uDirection,v_uDirection))continue;
      if(!this->checkModuleDirections(vDirection,v_vDirection))continue;
      if(!this->checkModuleDirections(wDirection,v_wDirection))continue;
      if(!this->checkModulePositions(posR,v_posR))continue;
      if(!this->checkModulePositions(posPhi,v_posPhi))continue;
      if(!this->checkModulePositions(posEta,v_posEta))continue;
      if(!this->checkModulePositions(posX,v_posX))continue;
      if(!this->checkModulePositions(posY,v_posY))continue;
      if(!this->checkModulePositions(posZ,v_posZ))continue;
      
      tkSector.v_rawId.push_back(rawId);
      bool moduleSelected(false);
      for(std::vector<unsigned int>::const_iterator i_rawId = allSectors.v_rawId.begin();
          i_rawId != allSectors.v_rawId.end(); ++i_rawId){
        if(rawId == *i_rawId)moduleSelected = true;
      }
      if(!moduleSelected)allSectors.v_rawId.push_back(rawId);
    }
    
    // Stops you from combining pixel and strip detector into one sector
    bool isPixel(false);
    bool isStrip(false);
    for(std::vector<unsigned int>::const_iterator i_rawId = tkSector.v_rawId.begin(); i_rawId != tkSector.v_rawId.end(); ++i_rawId){
      if(m_tkTreeVar_[*i_rawId].subdetId==PixelSubdetector::PixelBarrel || m_tkTreeVar_[*i_rawId].subdetId==PixelSubdetector::PixelEndcap){
        isPixel = true;
      }
      if(m_tkTreeVar_[*i_rawId].subdetId==StripSubdetector::TIB || m_tkTreeVar_[*i_rawId].subdetId==StripSubdetector::TOB ||
         m_tkTreeVar_[*i_rawId].subdetId==StripSubdetector::TID || m_tkTreeVar_[*i_rawId].subdetId==StripSubdetector::TEC){
        isStrip = true;
      }
    }

    if(isPixel && isStrip){
      edm::LogError("SectorBuilder")<<"Incorrect Sector Definition: there are pixel and strip modules within one sector"
                                     <<"\n... sector selection is not applied, sector "<<sectorCounter<<" is not built";
      continue;
    }
    tkSector.isPixel = isPixel;
    
    m_tkSector_[sectorCounter] = tkSector;
    edm::LogInfo("SectorBuilder")<<"There are "<<tkSector.v_rawId.size()<<" Modules in Sector "<<sectorCounter;
  }
  return;
}


// Checking methods copied from ApeEstimator.cc
bool
DefaultApeTree::checkIntervalsForSectors(const unsigned int sectorCounter, const std::vector<double>& v_id)const
{
    
  if(v_id.empty())return true;
  if(v_id.size()%2==1){
    edm::LogError("SectorBuilder")<<"Incorrect Sector Definition: Position Vectors need even number of arguments (Intervals)"
                                     <<"\n... sector selection is not applied, sector "<<sectorCounter<<" is not built";
    return false;
  }
  int entry(1); double intervalBegin(999.);
  for(std::vector<double>::const_iterator i_id = v_id.begin(); i_id != v_id.end(); ++i_id, ++entry){
    if(entry%2==1)intervalBegin = *i_id;
    if(entry%2==0 && intervalBegin>*i_id){
      edm::LogError("SectorBuilder")<<"Incorrect Sector Definition (Position Vector Intervals): \t"
                                    <<intervalBegin<<" is bigger than "<<*i_id<<" but is expected to be smaller"
                                    <<"\n... sector selection is not applied, sector "<<sectorCounter<<" is not built";
      return false;
    }
  }
  return true;
}

bool
DefaultApeTree::checkModuleIds(const unsigned int id, const std::vector<unsigned int>& v_id)const
{
    
  if(v_id.empty())return true;
  for(std::vector<unsigned int>::const_iterator i_id = v_id.begin(); i_id != v_id.end(); ++i_id){
    if(id==*i_id)return true;
  }
  return false;
}

bool
DefaultApeTree::checkModuleBools(const bool id, const std::vector<unsigned int>& v_id)const
{
    
  if(v_id.empty())return true;
  for(std::vector<unsigned int>::const_iterator i_id = v_id.begin(); i_id != v_id.end(); ++i_id){
    if(1==*i_id && id)return true;
    if(2==*i_id && !id)return true;
  }
  return false;
}

bool
DefaultApeTree::checkModuleDirections(const int id, const std::vector<int>& v_id)const
{
    
  if(v_id.empty())return true;
  for(std::vector<int>::const_iterator i_id = v_id.begin(); i_id != v_id.end(); ++i_id){
    if(id==*i_id)return true;
  }
  return false;
}

bool
DefaultApeTree::checkModulePositions(const float id, const std::vector<double>& v_id)const
{
    
  if(v_id.empty())return true;
  int entry(1); double intervalBegin(999.);
  for(std::vector<double>::const_iterator i_id = v_id.begin(); i_id != v_id.end(); ++i_id, ++entry){
    if(entry%2==1)intervalBegin = *i_id;
    if(entry%2==0 && id>=intervalBegin && id<*i_id)return true;
  }
  return false;
}



// ------------ method called to for each event  ------------
void
DefaultApeTree::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  // Same procedure as in ApeEstimatorSummary.cc minus reading of baseline tree
  
  // Load APEs from the GT and write them to root files similar to the ones from calculateAPE()
  edm::ESHandle<AlignmentErrorsExtended> alignmentErrors;
  iSetup.get<TrackerAlignmentErrorExtendedRcd>().get(alignmentErrors);  
  
  // Set up root file for default APE values
  const std::string defaultFileName(parameterSet_.getParameter<std::string>("resultFile"));
  TFile* defaultFile = new TFile(defaultFileName.c_str(),"RECREATE");
  
  // Naming in the root files has to be iterTreeX to be consistent for the plotting tool
  TTree* defaultTreeX(nullptr);
  TTree* defaultTreeY(nullptr);
  defaultFile->GetObject("iterTreeX;1",defaultTreeX);
  defaultFile->GetObject("iterTreeY;1",defaultTreeY);
  // The same for TTree containing the names of the sectors (no additional check, since always handled exactly as defaultTree)
  TTree* sectorNameTree(nullptr);
  defaultFile->GetObject("nameTree;1",sectorNameTree);
  
  edm::LogInfo("DefaultAPETree")<<"APE Tree is being created";
  defaultTreeX = new TTree("iterTreeX","Tree for default APE x values from GT");
  defaultTreeY = new TTree("iterTreeY","Tree for default APE y values from GT");     
  sectorNameTree = new TTree("nameTree","Tree with names of sectors");
  
    
  // Assign the information stored in the trees to arrays
  double a_defaultSectorX[16589];
  double a_defaultSectorY[16589];
  
  std::string* a_sectorName[16589];
  std::map<unsigned int, TrackerSectorStruct>::const_iterator i_sector;
  for(i_sector = m_tkSector_.begin(); i_sector != m_tkSector_.end(); ++i_sector){
    const unsigned int iSector(i_sector->first);
    const bool pixelSector(i_sector->second.isPixel);
    
    a_defaultSectorX[iSector] = -99.;
    a_defaultSectorY[iSector] = -99.;
    
    a_sectorName[iSector] = nullptr;
    std::stringstream ss_sector;
    std::stringstream ss_sectorSuffixed;
    ss_sector << "Ape_Sector_" << iSector;
  
    ss_sectorSuffixed << ss_sector.str() << "/D";
    defaultTreeX->Branch(ss_sector.str().c_str(), &a_defaultSectorX[iSector], ss_sectorSuffixed.str().c_str());
    if(pixelSector){
      defaultTreeY->Branch(ss_sector.str().c_str(), &a_defaultSectorY[iSector], ss_sectorSuffixed.str().c_str());
    }
    sectorNameTree->Branch(ss_sector.str().c_str(), &a_sectorName[iSector], 32000, 00);  
  }
  
  for(std::map<unsigned int,TrackerSectorStruct>::iterator i_sector = m_tkSector_.begin(); i_sector != m_tkSector_.end(); ++i_sector){
    const std::string& name(i_sector->second.name);      
    a_sectorName[(*i_sector).first] = new std::string(name);
  }
  
  // Loop over sectors for getting default APE
  
  for(std::map<unsigned int,TrackerSectorStruct>::iterator i_sector = m_tkSector_.begin(); i_sector != m_tkSector_.end(); ++i_sector){
    
    double defaultApeX(0.);
    double defaultApeY(0.);
    unsigned int nModules(0);
    std::vector<unsigned int>::const_iterator i_rawId;
    for(i_rawId = (*i_sector).second.v_rawId.begin(); i_rawId != (*i_sector).second.v_rawId.end(); ++i_rawId){
      std::vector<AlignTransformErrorExtended> alignErrors = alignmentErrors->m_alignError;
      for(std::vector<AlignTransformErrorExtended>::const_iterator i_alignError = alignErrors.begin(); i_alignError != alignErrors.end(); ++i_alignError){
        if(*i_rawId ==  i_alignError->rawId()){
          CLHEP::HepSymMatrix errMatrix = i_alignError->matrix();
          defaultApeX += errMatrix[0][0];
          defaultApeY += errMatrix[1][1];
          nModules++;
        }
      }
  }
  a_defaultSectorX[(*i_sector).first] = defaultApeX/nModules;
  a_defaultSectorY[(*i_sector).first] = defaultApeY/nModules;
  
  }
  sectorNameTree->Fill();
  sectorNameTree->Write("nameTree");
  defaultTreeX->Fill();
  defaultTreeX->Write("iterTreeX");
  defaultTreeY->Fill();
  defaultTreeY->Write("iterTreeY");
  
  defaultFile->Close(); 
}
  



// ------------ method called once each job just before starting event loop  ------------
void 
DefaultApeTree::beginJob()
{
    
  this->sectorBuilder();
}

// ------------ method called once each job just after ending the event loop  ------------
void 
DefaultApeTree::endJob() 
{
     
}

//define this as a plug-in
DEFINE_FWK_MODULE(DefaultApeTree);
