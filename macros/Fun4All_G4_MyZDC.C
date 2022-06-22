#ifndef MACRO_FUN4ALLG4MYZDC_C
#define MACRO_FUN4ALLG4MYZDC_C

#include <myzdc/myZDCSubsystem.h>
#include <myzdc/myZDCNtuple.h>
#include <myzdc/myZDCHitTree.h>
//#include <myzdc/myZDCRawTowerBuilder.h>
//#include <eiczdcreco/RawTowerZDCDigitizer.h>
//#include <eiczdcreco/RawTowerZDCCalibration.h>

#include <g4detectors/PHG4DetectorSubsystem.h>

#include <g4histos/G4HitNtuple.h>

#include <g4main/PHG4ParticleGenerator.h>
#include <g4main/PHG4ParticleGeneratorBase.h>
#include <g4main/PHG4ParticleGun.h>
#include <g4main/PHG4Reco.h>
#include <g4main/PHG4SimpleEventGenerator.h>

#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>

#include <phool/recoConsts.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4detectors.so)
R__LOAD_LIBRARY(libmyZDC.so)
R__LOAD_LIBRARY(libg4histos.so)

// needs 10000 geantinos to make a decent scan plot
void Fun4All_G4_MyZDC(int nEvents = 10000)
{
  ///////////////////////////////////////////
  // Make the Server
  //////////////////////////////////////////
  Fun4AllServer *se = Fun4AllServer::instance();
  recoConsts *rc = recoConsts::instance();
  // if you want to fix the random seed to reproduce results
  // set this flag
  // nail this down so I know what the first event looks like...
  //  rc->set_IntFlag("RANDOMSEED",12345);

  //
  // Particle Generator
  //

  // the PHG4ParticleGenerator makes cones using phi and eta
  PHG4ParticleGenerator *gen = new PHG4ParticleGenerator();
  gen->set_name("geantino");
  gen->set_mom_range(1.0, 1.0);
  gen->set_z_range(0., 0.);
  // experimentally found ranges, they cover the original block
  gen->set_vtx(-50, 0, 0);
  gen->set_phi_range(-14.5 / 180 * TMath::Pi(), 14.5 / 180 * TMath::Pi());
  gen->set_eta_range(-0.26, 0.26);
  //  se->registerSubsystem(gen);

  // ParticleGun shoots right into the original MyDetector volume
  PHG4ParticleGun *gun = new PHG4ParticleGun();
  double ene = 0.1;
  double theta = 0; //for test
  //  string particle = "e-";
  string particle = "gamma";
  //  string particle = "neutron";
  //double theta = atan2(96, 3750);
  gun->set_name(particle);
  gun->set_vtx(0, 0, 0);
  gun->set_mom(ene*sin(theta), 0, ene*cos(theta));
  se->registerSubsystem(gun);

  //
  // Geant4 setup
  //
  PHG4Reco *g4Reco = new PHG4Reco();
  // setup of G4:
  //   no field
  //   no saving of geometry: it takes time and we do not do tracking
  //   so we do not need the geometry

  //  g4Reco->SetPhysicsList("FTFP_BERT_HP");
  g4Reco->set_field(0);
  g4Reco->save_DST_geometry(false);

  myZDCSubsystem *mydet = new myZDCSubsystem("MyZDC");
  mydet->SetActive();

  //for test
  mydet->set_double_param("place_z",200.);
  mydet->set_double_param("place_x",0.);
  mydet->set_double_param("rot_y",0.);
  mydet->set_double_param("size_z",300);
  mydet->set_string_param("crystal","PbWO4");

  g4Reco->registerSubsystem(mydet);

  // mydet = new MyDetectorSubsystem("MyDetector2");
  //  mydet->set_double_param("place_y",40.);
  // mydet->set_string_param("material","G4_Fe");
  // mydet->SetActive();
  // g4Reco->registerSubsystem(mydet);

  se->registerSubsystem(g4Reco);

  ///////////////////////////////////////////
  // Fun4All modules
  ///////////////////////////////////////////

  G4HitNtuple *hits = new G4HitNtuple("Hits");
  hits->AddNode("MyZDC_0", 0);
  //  se->registerSubsystem(hits);

  // myZDCNtuple *zdchits = new myZDCNtuple("Hits");
  // zdchits->AddNode("MyZDC_0", 0);
  // se->registerSubsystem(zdchits);
  
  myZDCHitTree *zdctree = new myZDCHitTree("Hits");
  zdctree->AddNode("MyZDC_0", 0);
  se->registerSubsystem(zdctree);
  
  ///////////////////////////////////////////
  // IOManagers...
  ///////////////////////////////////////////

  //Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT","G4Example02.root");
  // out->Verbosity(10);
  // se->registerOutputManager(out);

  // this (dummy) input manager just drives the event loop
  Fun4AllInputManager *in = new Fun4AllDummyInputManager("Dummy");
  se->registerInputManager(in);
  // events = 0 => run forever
  if (nEvents <= 0)
  {
    return 0;
  }
  se->run(nEvents);
  cout << "Print out Detector parameters" << endl;
  mydet->Print();
  se->End();
  delete se;
  cout << endl
       << endl;
  cout << "Now open the ntuple file with " << endl;
  cout << "  root.exe G4HitNtuple.root" << endl;
  cout << "and draw the 3d hit distribution," << endl;
  cout << "exit hits in black, entry hits in red:" << endl;
  cout << "  hitntup->Draw(\"x1:y1:z1\")" << endl;
  cout << "  hitntup->SetMarkerColor(2)" << endl;
  cout << "  hitntup->Draw(\"x0:y0:z0\",\"\",\"same\")" << endl;
  cout << endl;

  cout<<endl;
  cout<< "My Setup: " <<particle<< "  with "<<ene<<" GeV"<<endl;
  cout<<endl;
  gSystem->Exit(0);
}

#endif
