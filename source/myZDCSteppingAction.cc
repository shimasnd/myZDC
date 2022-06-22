//____________________________________________________________________________..
//
// This is a working template for the Stepping Action which needs to be implemented
// for active detectors. Most of the code is error handling and access to the G4 objects
// and our data structures. It does not need any adjustment. The only thing you need to
// do is to add the properties of the G4Hits you want to save for later analysis
// This needs to be done in 2 places, G4Hits are generated when a G4 track enters a new
// volume (or is created). Here you give it an initial value. When the G4 track leaves
// the volume the final value needs to be set.
// The places to do this is marked by //implement your own here//
//
// As guidance you can look at the total (integrated over all steps in a volume) energy
// deposit which should always be saved.
// Additionally the total ionization energy is saved - this can be removed if you are not
// interested in this. Naturally you may want remove these comments in your version
//
//____________________________________________________________________________..

#include "myZDCSteppingAction.h"

#include "myZDCDetector.h"
#include "zdcdetid.h"

#include <phparameter/PHParameters.h>

#include <g4detectors/PHG4StepStatusDecode.h>

#include <g4main/PHG4Hit.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hitv1.h>
#include <g4main/PHG4Shower.h>
#include <g4main/PHG4SteppingAction.h>
#include <g4main/PHG4TrackUserInfoV1.h>

#include <phool/getClass.h>

#include <TSystem.h>

#include <Geant4/G4ParticleDefinition.hh> 
#include <Geant4/G4ReferenceCountedHandle.hh>
#include <Geant4/G4Step.hh>
#include <Geant4/G4StepPoint.hh> 
#include <Geant4/G4StepStatus.hh>
#include <Geant4/G4String.hh> 
#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/G4ThreeVector.hh>
#include <Geant4/G4TouchableHandle.hh>
#include <Geant4/G4Track.hh>
#include <Geant4/G4TrackStatus.hh>
#include <Geant4/G4Types.hh>
#include <Geant4/G4VPhysicalVolume.hh>
#include <Geant4/G4VTouchable.hh>
#include <Geant4/G4VUserTrackInformation.hh>

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <random>

class PHCompositeNode;

//____________________________________________________________________________..
myZDCSteppingAction::myZDCSteppingAction(myZDCDetector *detector, const PHParameters *parameters)
  : PHG4SteppingAction(detector->GetName())
  , m_Detector(detector)
  , m_Params(parameters)
  , m_HitContainer(nullptr)
  , m_Hit(nullptr)
  , m_SaveHitContainer(nullptr)
  , m_SaveVolPre(nullptr)
  , m_SaveVolPost(nullptr)
  , m_SaveShower(nullptr)
  , m_SaveTrackId(-1)
  , m_SavePreStepStatus(-1)
  , m_SavePostStepStatus(-1)
  , m_ActiveFlag(m_Params->get_int_param("active"))
  , m_BlackHoleFlag(m_Params->get_int_param("blackhole"))
  , m_EdepSum(0)
  , m_EionSum(0)
  , m_LightYield(0)
  , m_nPhperMeV(0)
{
  SetNPhotonPerMeV();
  m_testcnt=0;
}

//____________________________________________________________________________..
myZDCSteppingAction::~myZDCSteppingAction()
{
  // if the last hit was a zero energie deposit hit, it is just reset
  // and the memory is still allocated, so we need to delete it here
  // if the last hit was saved, hit is a nullptr pointer which are
  // legal to delete (it results in a no operation)
  delete m_Hit;
}

void myZDCSteppingAction::SetNPhotonPerMeV(){

  double defaultval = 130.; //PbWO4
  
  std::string crystal = m_Params->get_string_param("crystal");
  std::ifstream istream;
  istream.open("myZDCcrystal.txt");
  if(!istream) {
    m_nPhperMeV = defaultval; 
    return;
  }
  std::string line;
  while(getline(istream, line)){
    if(line.find("#") != std::string::npos) continue;
    if(line.find(crystal) != std::string::npos){
      std::istringstream iss(line);
      std::string dummy;
      if(!(iss>>dummy>>m_nPhperMeV)){
	m_nPhperMeV=defaultval;
	return;
      }
      break;
    }
  }
  std::cout<<"Crystal "<<crystal<<"  photon/MeV= "<<m_nPhperMeV<<std::endl;
  return;

}

//____________________________________________________________________________..
// This is the implementation of the G4 UserSteppingAction
bool myZDCSteppingAction::UserSteppingAction(const G4Step *aStep,bool was_used)
{
  G4TouchableHandle touch = aStep->GetPreStepPoint()->GetTouchableHandle();
  G4TouchableHandle touchpost = aStep->GetPostStepPoint()->GetTouchableHandle();
  // get volume of the current step
  G4VPhysicalVolume *volume = touch->GetVolume();

  // IsInDetector(volume) returns
  //  == 0 outside of detector
  //   > 0 for hits in active volume
  //  < 0 for hits in passive material
  int whichactive = m_Detector->IsInDetector(volume);
  if (!whichactive)
  {
    return false;
  }

  
  // collect energy and track length step by step
  G4double edep = aStep->GetTotalEnergyDeposit() / GeV;
  G4double eion = (aStep->GetTotalEnergyDeposit() - aStep->GetNonIonizingEnergyDeposit()) / GeV;
  G4double light_yield = 0;
  if(whichactive>0) light_yield = GetVisibleEnergyDeposition(aStep); //GeV
  const G4Track *aTrack = aStep->GetTrack();
  // if this detector stops everything, just put all kinetic energy into edep
  if (m_BlackHoleFlag)
  {
    edep = aTrack->GetKineticEnergy() / GeV;
    G4Track *killtrack = const_cast<G4Track *>(aTrack);
    killtrack->SetTrackStatus(fStopAndKill);
  }
  // we use here only one detector in this simple example
  // if you deal with multiple detectors in this stepping action
  // the detector id can be used to distinguish between them
  // hits can easily be analyzed later according to their detector id

  /*------------------------------------------------*/
  /*--Here check the ZDC detector ID and layers-----*/
  /*------------------------------------------------*/

  int detflag = -1;
  if(whichactive>0) detflag = m_Detector->GetActiveVolumeInfo(volume);
  else if (whichactive<0) detflag = m_Detector->GetAbsorberVolumeInfo(volume);
  int detector_id = detflag%100;  
  int detector_layer = (detflag%10000)/100;
  int detector_nlyrbox =(detflag%1000000)/10000;
  int detector_system=  (detflag/1000000)*1000000;

  int xid = touch->GetCopyNumber(1);
  int yid = touch->GetCopyNumber();

  int layer_id = -1;

  if(whichactive>0){
    if(detector_system == ZDCID::CrystalTower){
      int zid = touch->GetCopyNumber(2);
      if(detector_id == ZDCID::SI_PIXEL) layer_id = zid * 2;
      else if(detector_id==ZDCID::Crystal) layer_id = zid * 2 +1;
    }else if(detector_system == ZDCID::EMLayer){
      if(detector_id == ZDCID::SI_PIXEL) layer_id = detector_layer + touch->GetCopyNumber(3);
      if(detector_id == ZDCID::SI_PAD) {
	int boxid = touch->GetCopyNumber(4);
	int zid    =touch->GetCopyNumber(3);
	int nlyr = detector_nlyrbox +1;
	layer_id = detector_layer + zid + boxid * nlyr; 
      }
    }else if (detector_system == ZDCID::HCPadLayer){
      if(detector_id == ZDCID::SI_PAD) layer_id = detector_layer + touch->GetCopyNumber(3);
    }else if (detector_system == ZDCID::HCSciLayer){
      if(detector_id == ZDCID::Scintillator){
	int boxid = touch->GetCopyNumber(4);
	int zid   = touch->GetCopyNumber(3);
	int nlyr  = detector_nlyrbox;
	layer_id = detector_layer + zid + boxid *nlyr;
      }      
    }
  }

  if(whichactive<0){
    layer_id = detector_layer;
    if(detector_system == ZDCID::HCSciLayer){
      int boxid = touch->GetCopyNumber(2);
      layer_id = detector_layer + boxid * detector_nlyrbox;
    }
  }

  if(detector_system == ZDCID::CrystalTower && light_yield>0){

    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());
  
    double mu = light_yield * 1000 * m_nPhperMeV;
    std::poisson_distribution dist(mu);
    
    light_yield = dist(engine);

    if(m_testcnt<10)
      std::cout<<mu<<"  "<<light_yield<<"   "<<seed_gen()<<"  "<<engine<<std::endl;

    m_testcnt++;
    
  }
  
  bool geantino = false;
  // the check for the pdg code speeds things up, I do not want to make
  // an expensive string compare for every track when we know
  // geantino or chargedgeantino has pid=0
  if (aTrack->GetParticleDefinition()->GetPDGEncoding() == 0 &&
      aTrack->GetParticleDefinition()->GetParticleName().find("geantino") !=
          std::string::npos)  // this also accounts for "chargedgeantino"
  {
    geantino = true;
  }
  G4StepPoint *prePoint = aStep->GetPreStepPoint();
  G4StepPoint *postPoint = aStep->GetPostStepPoint();

// Here we have to decide if we need to create a new hit.  Normally this should 
// only be neccessary if a G4 Track enters a new volume or is freshly created
// For this we look at the step status of the prePoint (beginning of the G4 Step).
// This should be either fGeomBoundary (G4 Track crosses into volume) or 
// fUndefined (G4 Track newly created)
// Sadly over the years with different G4 versions we have observed cases where
// G4 produces "impossible hits" which we try to catch here
// These errors were always rare and it is not clear if they still exist but we
// still check for them for safety. We can reproduce G4 runs identically (if given
// the sequence of random number seeds you find in the log), the printouts help
// us giving the G4 support information about those failures
// 
  switch (prePoint->GetStepStatus()){
  case fPostStepDoItProc:
    if (m_SavePostStepStatus != fGeomBoundary){
      // this is the okay case, fPostStepDoItProc called in a volume, not first thing inside
      // a new volume, just proceed here
      break;
    } else {
      // this is an impossible G4 Step print out diagnostic to help debug, not sure if
      // this is still with us
      std::cout << GetName() << ": New Hit for  " << std::endl;
      std::cout << "prestep status: "
           << PHG4StepStatusDecode::GetStepStatus(prePoint->GetStepStatus())
           << ", poststep status: "
           << PHG4StepStatusDecode::GetStepStatus(postPoint->GetStepStatus())
           << ", last pre step status: "
           << PHG4StepStatusDecode::GetStepStatus(m_SavePreStepStatus)
           << ", last post step status: "
           << PHG4StepStatusDecode::GetStepStatus(m_SavePostStepStatus) << std::endl;
      std::cout << "last track: " << m_SaveTrackId
           << ", current trackid: " << aTrack->GetTrackID() << std::endl;
      std::cout << "phys pre vol: " << volume->GetName()
           << " post vol : " << touchpost->GetVolume()->GetName() << std::endl;
      std::cout << " previous phys pre vol: " << m_SaveVolPre->GetName()
           << " previous phys post vol: " << m_SaveVolPost->GetName() << std::endl;
    }
// These are the normal cases
  case fGeomBoundary:
  case fUndefined:
    if (!m_Hit) {
      m_Hit = new PHG4Hitv1();
    }
    // here we set the entrance values in cm
    m_Hit->set_x(0, prePoint->GetPosition().x() / cm);
    m_Hit->set_y(0, prePoint->GetPosition().y() / cm);
    m_Hit->set_z(0, prePoint->GetPosition().z() / cm);
    // time in ns
    m_Hit->set_t(0, prePoint->GetGlobalTime() / nanosecond);
    // set the track ID
    m_Hit->set_trkid(aTrack->GetTrackID());
    m_SaveTrackId = aTrack->GetTrackID();
    // set the initial energy deposit
    m_EdepSum = 0;
    // implement your own here://
    // add the properties you are interested in via set_XXX methods
    // you can find existing set methods in $OFFLINE_MAIN/include/g4main/PHG4Hit.h
    // this is initialization of your value. This is not needed you can just set the final
    // value at the last step in this volume later one

    if (whichactive > 0)
    {
      m_EionSum = 0;  // assuming the ionization energy is only needed for active
                      // volumes (scintillators)
      m_LightYield = 0;
      m_Hit->set_eion(0);
      m_Hit->set_light_yield(0);
      m_SaveHitContainer = m_HitContainer;
    }
    else
    {
      m_SaveHitContainer = m_HitContainer;
      // std::cout << "implement stuff for whichactive < 0 (inactive volumes)" << std::endl;
      // gSystem->Exit(1);
    }
    // this is for the tracking of the truth info
    if (G4VUserTrackInformation *p = aTrack->GetUserInformation())
    {
      if (PHG4TrackUserInfoV1 *pp = dynamic_cast<PHG4TrackUserInfoV1 *>(p))
      {
        m_Hit->set_trkid(pp->GetUserTrackId());
	m_Hit->set_shower_id(pp->GetShower()->get_id());
	m_SaveShower=pp->GetShower();
	
      }
    }
    break;
  default:
    break;
  }

  // This section is called for every step
  // some sanity checks for inconsistencies (aka bugs) we have seen over the years
  // check if this hit was created, if not print out last post step status
  if (!m_Hit || !std::isfinite(m_Hit->get_x(0)))
  {
    std::cout << GetName() << ": hit was not created" << std::endl;
    std::cout << "prestep status: "
         << PHG4StepStatusDecode::GetStepStatus(prePoint->GetStepStatus())
         << ", poststep status: "
         << PHG4StepStatusDecode::GetStepStatus(postPoint->GetStepStatus())
         << ", last pre step status: "
         << PHG4StepStatusDecode::GetStepStatus(m_SavePreStepStatus)
         << ", last post step status: "
         << PHG4StepStatusDecode::GetStepStatus(m_SavePostStepStatus) << std::endl;
    std::cout << "last track: " << m_SaveTrackId
         << ", current trackid: " << aTrack->GetTrackID() << std::endl;
    std::cout << "phys pre vol: " << volume->GetName()
         << " post vol : " << touchpost->GetVolume()->GetName() << std::endl;
    std::cout << " previous phys pre vol: " << m_SaveVolPre->GetName()
         << " previous phys post vol: " << m_SaveVolPost->GetName() << std::endl;
    // This is fatal - a hit from nowhere. This needs to be looked at and fixed
    gSystem->Exit(1);
  }
  // check if track id matches the initial one when the hit was created
  if (aTrack->GetTrackID() != m_SaveTrackId){
    std::cout << GetName() << ": hits do not belong to the same track" << std::endl;
    std::cout << "saved track: " << m_SaveTrackId
         << ", current trackid: " << aTrack->GetTrackID()
         << ", prestep status: " << prePoint->GetStepStatus()
         << ", previous post step status: " << m_SavePostStepStatus << std::endl;
    // This is fatal - a hit from nowhere. This needs to be looked at and fixed
    gSystem->Exit(1);
  }

// We need to cache a few things from one step to the next
// to identify impossible hits and subsequent debugging printout
  m_SavePreStepStatus = prePoint->GetStepStatus();
  m_SavePostStepStatus = postPoint->GetStepStatus();
  m_SaveVolPre = volume;
  m_SaveVolPost = touchpost->GetVolume();
  // here we just update the exit values, it will be overwritten
  // for every step until we leave the volume or the particle
  // ceases to exist
  // sum up the energy to get total deposited
  m_EdepSum += edep;
  if (whichactive > 0) {
    m_EionSum += eion;
    m_LightYield += light_yield;
  }

  // if any of these conditions is true this is the last step in
  // this volume and we need to save the hit
  // postPoint->GetStepStatus() == fGeomBoundary: track leaves this volume
  // postPoint->GetStepStatus() == fWorldBoundary: track leaves this world
  // (happens when your detector goes outside world volume)
  // postPoint->GetStepStatus() == fAtRestDoItProc: track stops (typically
  // aTrack->GetTrackStatus() == fStopAndKill is also set)
  // aTrack->GetTrackStatus() == fStopAndKill: track ends

  if (postPoint->GetStepStatus() == fGeomBoundary ||
      postPoint->GetStepStatus() == fWorldBoundary ||
      postPoint->GetStepStatus() == fAtRestDoItProc ||
      aTrack->GetTrackStatus() == fStopAndKill){
    // save only hits with energy deposit (or geantino)
    if (m_EdepSum > 0 || geantino){
      // update values at exit coordinates and set keep flag
      // of track to keep
      m_Hit->set_x(1, postPoint->GetPosition().x() / cm);
      m_Hit->set_y(1, postPoint->GetPosition().y() / cm);
      m_Hit->set_z(1, postPoint->GetPosition().z() / cm);
      m_Hit->set_t(1, postPoint->GetGlobalTime() / nanosecond);
      if (G4VUserTrackInformation *p = aTrack->GetUserInformation()){
        if (PHG4TrackUserInfoV1 *pp = dynamic_cast<PHG4TrackUserInfoV1 *>(p)){
          pp->SetKeep(1);  // we want to keep the track
        }
      }
      
      if (geantino){
 //implement your own here://
 // if you want to do something special for geantinos (normally you do not)
        m_Hit->set_edep(-1);  // only energy=0 g4hits get dropped, this way
                              // geantinos survive the g4hit compression
        if (whichactive > 0){
          m_Hit->set_eion(-1);
	  m_Hit->set_light_yield(-1);
        }
      } else {
	  m_Hit->set_edep(m_EdepSum);
      }
 //implement your own here://
 // what you set here will be saved in the output
      m_Hit->set_layer(layer_id);
      m_Hit->set_index_i(xid);
      m_Hit->set_index_j(yid);
      m_Hit->set_hit_type(detector_id);
      m_Hit->set_eion(m_EionSum);
      m_Hit->set_light_yield(m_LightYield);
      
      m_SaveHitContainer->AddHit(detector_id, m_Hit);
      if (m_SaveShower)
      {
	m_SaveShower->add_g4hit_id(m_SaveHitContainer->GetID(), m_Hit->get_hit_id());
      }
    
      // ownership has been transferred to container, set to null
      // so we will create a new hit for the next track
      m_Hit = nullptr;
      
    }else{
      // if this hit has no energy deposit, just reset it for reuse
      // this means we have to delete it in the dtor. If this was
      // the last hit we processed the memory is still allocated
      m_Hit->Reset();
    }
  }
  // return true to indicate the hit was used
  return true;
}

//____________________________________________________________________________..
void myZDCSteppingAction::SetInterfacePointers(PHCompositeNode *topNode)
{
  std::string hitnodename = "G4HIT_" + m_Detector->GetName();
  // now look for the map and grab a pointer to it.
  m_HitContainer = findNode::getClass<PHG4HitContainer>(topNode, hitnodename);
  // if we do not find the node we need to make it.
  if (!m_HitContainer)
  {
    std::cout << "myZDCSteppingAction::SetTopNode - unable to find "
              << hitnodename << std::endl;
  }
}
