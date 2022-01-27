#include "myZDCHitTree.h"

#include <g4main/PHG4Hit.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4InEvent.h>
#include <g4main/PHG4Particle.h>

#include <fun4all/Fun4AllHistoManager.h>
#include <fun4all/SubsysReco.h>           // for SubsysReco

#include <phool/getClass.h>

#include <TFile.h>
#include <TH1.h>
#include <TTree.h>

#include <sstream>
#include <utility>    

using namespace std;

myZDCHitTree::myZDCHitTree(const std::string &name, const std::string &filename) 
  : SubsysReco(name)
  , nblocks(0)
  , hm(nullptr)
  , _filename(filename)
  , tree(nullptr)
  , outfile(nullptr)
  , Nhit(0)
  , Ntrack(0)
{
}

myZDCHitTree::~myZDCHitTree(){
  //  delete ntup;
  delete hm;
}

int myZDCHitTree::Init(PHCompositeNode *)
 {
   hm = new Fun4AllHistoManager(Name());
   outfile = new TFile(_filename.c_str(), "RECREATE");
   tree = new TTree("zdchit", "Collection of myZDC G4Hits");

   tree->Branch("Nhit", &Nhit, "Nhit/I");
   tree->Branch("layerType", &layerType);
   tree->Branch("layerID", &layerID);
   tree->Branch("xID", &xID);
   tree->Branch("yID", &yID);
   tree->Branch("x0", &x0);
   tree->Branch("y0", &y0);
   tree->Branch("z0", &z0);
   tree->Branch("x1", &x1);
   tree->Branch("y1", &y1);
   tree->Branch("z1", &z1);
   tree->Branch("time0", &time0);
   tree->Branch("time1", &time1);
   tree->Branch("edep", &edep);
   tree->Branch("lightyield",&lightyield);

   tree->Branch("Ntrack",&Ntrack, "Ntrack/I");
   tree->Branch("trk_px", &trk_px);
   tree->Branch("trk_py", &trk_py);
   tree->Branch("trk_pz", &trk_pz);
   tree->Branch("trk_e", &trk_e);
   tree->Branch("trk_pid", &trk_pid);

   return 0;
 }
 
 int myZDCHitTree::process_event(PHCompositeNode *topNode)
 {
   ostringstream nodename;
   set<string>::const_iterator iter;
   for (iter = _node_postfix.begin(); iter != _node_postfix.end(); ++iter)
   {

     nodename.str("");
     nodename << "G4HIT_" << *iter;
     PHG4HitContainer *hits = findNode::getClass<PHG4HitContainer>(topNode, nodename.str());
     if (!hits) return 0;
     
     Nhit = 0;

     PHG4HitContainer::ConstRange hit_range = hits->getHits();
     for (PHG4HitContainer::ConstIterator hit_iter = hit_range.first; hit_iter != hit_range.second; hit_iter++){
       if(hit_iter->second->get_hit_type()<0) continue;
       if(hit_iter->second->get_hit_type()>5) continue; //to skip absorber hits
       Nhit++;

       layerType.push_back(hit_iter->second->get_hit_type());
       layerID.push_back(hit_iter->second->get_layer());
       xID.push_back(hit_iter->second->get_index_i());
       yID.push_back(hit_iter->second->get_index_j());
       x0.push_back(hit_iter->second->get_x(0));
       y0.push_back(hit_iter->second->get_y(0));
       z0.push_back(hit_iter->second->get_z(0));
       x1.push_back(hit_iter->second->get_x(1));
       y1.push_back(hit_iter->second->get_y(1));
       z1.push_back(hit_iter->second->get_z(1));
       time0.push_back(hit_iter->second->get_t(0));
       time1.push_back(hit_iter->second->get_t(1));
       edep.push_back(hit_iter->second->get_edep());
       lightyield.push_back(hit_iter->second->get_light_yield());
     }
     
   }

   PHG4InEvent *track_truthinfo 
     = findNode::getClass<PHG4InEvent>(topNode, "PHG4INEVENT");
      
   if(track_truthinfo) {
     
     Ntrack=0;
     
     pair<multimap<int, PHG4Particle *>::const_iterator, multimap<int, PHG4Particle *>::const_iterator > particlebegin_end = track_truthinfo->GetParticles();
     multimap<int,PHG4Particle *>::const_iterator particle_iter;

     for (particle_iter = particlebegin_end.first; particle_iter != particlebegin_end.second; ++particle_iter){
       
       /// Get this truth particle
       const PHG4Particle *truth = particle_iter->second;
       
       Ntrack++;
       /// Get this particles momentum, etc.
       trk_px.push_back(truth->get_px());
       trk_py.push_back(truth->get_py());
       trk_pz.push_back(truth->get_pz());
       trk_e.push_back(truth->get_e());
       trk_pid.push_back(truth->get_pid());       
     }
   }

   tree->Fill();
   
   layerType.clear();
   layerID.clear();
   xID.clear();
   yID.clear();
   x0.clear();
   y0.clear();
   z0.clear();
   x1.clear();
   y1.clear();
   z1.clear();
   time0.clear();
   time1.clear();
   edep.clear();
   lightyield.clear();
   trk_px.clear();
   trk_py.clear();
   trk_pz.clear();
   trk_e.clear();
   trk_pid.clear();

   return 0;
 }
 
 int myZDCHitTree::End(PHCompositeNode *topNode)
 {
   outfile->cd();
   tree->Write();
   outfile->Write();
   outfile->Close();
   delete outfile;
   hm->dumpHistos(_filename, "UPDATE");
   return 0;
 }
 
 void myZDCHitTree::AddNode(const std::string &name, const int detid)
 {
   _node_postfix.insert(name);
   _detid[name] = detid;
   return;
 }
