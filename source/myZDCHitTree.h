#ifndef MYZDCHITTREE_H
#define MYZDCHITTREE_H

#include <fun4all/SubsysReco.h>

#include <map>
#include <set>
#include <string>
#include <vector>

// Forward declerations
class Fun4AllHistoManager;
class PHCompositeNode;
class TFile;
class TTree;

class myZDCHitTree : public SubsysReco
{
 public:
  //! constructor
  myZDCHitTree(const std::string &name = "myZDCHitTree", const std::string &filename = "myZDCHitTree.root");

  //! destructor
  ~myZDCHitTree() override;

  //! full initialization
  int Init(PHCompositeNode *) override;

  //! event processing method
  int process_event(PHCompositeNode *) override;

  //! end of run method
  int End(PHCompositeNode *) override;

  void AddNode(const std::string &name, const int detid = 0);

 protected:
  int nblocks;
  Fun4AllHistoManager *hm;
  std::string _filename;
  std::set<std::string> _node_postfix;
  std::map<std::string, int> _detid;
  TTree *tree;
  TFile *outfile;

 private:

  int Nhit;
  std::vector<int> layerType;
  std::vector<int> layerID;
  std::vector<int> xID;
  std::vector<int> yID;
  std::vector<float> x0;
  std::vector<float> y0;
  std::vector<float> z0;
  std::vector<float> x1;
  std::vector<float> y1;
  std::vector<float> z1;
  std::vector<float> time0;
  std::vector<float> time1;
  std::vector<float> edep;

  

};

#endif
