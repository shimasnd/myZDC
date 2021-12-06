#include "myZDCRawTowerBuilder.h"
#include "zdcdetid.h"

#include <calobase/RawTowerContainer.h>
#include <calobase/RawTowerv2.h>

#include <calobase/RawTower.h>                 // for RawTower
#include <calobase/RawTowerDefs.h>             // for convert_name_to_caloid
#include <calobase/RawTowerGeom.h>             // for RawTowerGeom
#include <calobase/RawTowerGeomv3.h>
#include <calobase/RawTowerGeomContainer.h>    // for RawTowerGeomContainer
#include <calobase/RawTowerGeomContainerv1.h>

#include <g4main/PHG4Hit.h>
#include <g4main/PHG4HitContainer.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/SubsysReco.h>                // for SubsysReco

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNode.h>                      // for PHNode
#include <phool/PHNodeIterator.h>
#include <phool/PHObject.h>                    // for PHObject
#include <phool/getClass.h>
#include <phool/phool.h>                       // for PHWHERE

#include <TRotation.h>
#include <TVector3.h>

#include <cstdlib>                            // for exit
#include <exception>                           // for exception
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>                             // for pair, make_pair
#include <bitset>

using namespace std;

myZDCRawTowerBuilder::myZDCRawTowerBuilder(const std::string &name)
  : SubsysReco(name)
  , m_Towers(nullptr)
  , m_Geoms(nullptr)
  , m_Detector("NONE")
  , m_SubDetector("NONE")
  , m_MappingTowerFile("default.txt")
  , m_CaloId(RawTowerDefs::NONE)
  , m_GlobalPlaceInX(0)
  , m_GlobalPlaceInY(0)
  , m_GlobalPlaceInZ(0)
  , m_RotInX(0)
  , m_RotInY(0)
  , m_RotInZ(0)
  , m_Emin(1e-9)
  , m_TowerDepth(0)
  , m_ThicknessAbsorber(0)
  , m_ThicknessScintilator(0)
  , m_NLayers(0)
  , m_NLayersPerTowerSeg(0)
  , m_NTowerSeg(0)
{
}

int myZDCRawTowerBuilder::InitRun(PHCompositeNode *topNode)
{
  PHNodeIterator iter(topNode);

  // Looking for the DST node
  PHCompositeNode *dstNode;
  dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
  if (!dstNode)
  {
    std::cout << PHWHERE << "DST Node missing, doing nothing." << std::endl;
    exit(1);
  }

  try
  {
    CreateNodes(topNode);
  }
  catch (std::exception &e)
  {
    std::cout << e.what() << std::endl;
    //exit(1);
  }

  try
  {
    ReadGeometryFromTable();
  }
  catch (std::exception &e)
  {
    std::cout << e.what() << std::endl;
    //exit(1);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int myZDCRawTowerBuilder::process_event(PHCompositeNode *topNode)
{
  // get hits
  string NodeNameHits = "G4HIT_" + m_Detector;
  PHG4HitContainer *g4hit = findNode::getClass<PHG4HitContainer>(topNode, NodeNameHits);
  if (!g4hit)
  {
    cout << "Could not locate g4 hit node " << NodeNameHits << endl;
    exit(1);
  }

  // loop over all hits in the event
  PHG4HitContainer::ConstIterator hiter;
  PHG4HitContainer::ConstRange hit_begin_end = g4hit->getHits();

  for (hiter = hit_begin_end.first; hiter != hit_begin_end.second; hiter++)
  {
    PHG4Hit *g4hit_i = hiter->second;

    // Don't include hits with zero energy
    if (g4hit_i->get_edep() <= 0 && g4hit_i->get_edep() != -1) continue;
    if (g4hit_i->get_hit_type() != m_SubDetID) continue;

    /* encode CaloTowerID from i (xid), j (yid) index of tower / hit and calorimeter ID */
    int towerid = g4hit_i->get_index_k();
    int layerid = g4hit_i->get_layer();
    RawTowerDefs::keytype calotowerid = RawTowerDefs::encode_towerid_zdc(m_CaloId,
								     g4hit_i->get_index_i(),
								     g4hit_i->get_index_j(),
								     towerid
								     );
    
  
    /* add the energy to the corresponding tower */
    RawTowerv2 *tower = dynamic_cast<RawTowerv2 *>(m_Towers->getTower(calotowerid));
    if (!tower)
    {
      tower = new RawTowerv2(calotowerid);
      tower->set_energy(0);
      m_Towers->AddTower(tower->get_id(), tower);
      if (Verbosity() > 2) 
      {
        std::cout << "in: " <<  g4hit_i->get_index_i() << "\t" << g4hit_i->get_index_j() << "\t" << g4hit_i->get_index_k() << std::endl;
        std::cout << "decoded: " <<  tower->get_bineta() << "\t" << tower->get_binphi()  << "\t" << tower->get_binl()  << std::endl;
      }
    }
    tower->add_ecell(layerid, g4hit_i->get_light_yield());
    tower->set_energy(tower->get_energy() + g4hit_i->get_light_yield());
    tower->add_eshower(g4hit_i->get_shower_id(), g4hit_i->get_edep());
  }

  float towerE = 0.;

  if (Verbosity())
  {
    towerE = m_Towers->getTotalEdep();
  }

  m_Towers->compress(m_Emin);
  if (Verbosity())
  {
    std::cout << "storing towers: "<< m_Towers->size() << std::endl;
    cout << "Energy lost by dropping towers with less than " << m_Emin
         << " energy, lost energy: " << towerE - m_Towers->getTotalEdep() << endl;
    m_Towers->identify();
    RawTowerContainer::ConstRange begin_end = m_Towers->getTowers();
    RawTowerContainer::ConstIterator iter;
    for (iter = begin_end.first; iter != begin_end.second; ++iter)
    {
      iter->second->identify();
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int myZDCRawTowerBuilder::End(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void myZDCRawTowerBuilder::Detector(const std::string &d)
{
  m_Detector = d;

}

void myZDCRawTowerBuilder::SubDetector(const std::string &d)
{
  
  m_SubDetector = d;

  if(d == "ZDC_Crystal")
    m_SubDetID = ZDCID::Crystal;
  else if(d == "ZDC_SiPixel")
    m_SubDetID = ZDCID::SI_PIXEL;
  else if(d == "ZDC_SiPad")
    m_SubDetID = ZDCID::SI_PAD;
  else if(d == "ZDC_Sci")
    m_SubDetID = ZDCID::Scintillator;
  else
  {
    std::cout<<"Invalid ZDC subdetector name in myZDCRawTowerBuilder"
	     <<std::endl;
    exit(1);
  }
  
  m_CaloId = RawTowerDefs::convert_name_to_caloid(d);
  
}

void myZDCRawTowerBuilder::CreateNodes(PHCompositeNode *topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode *runNode = static_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "RUN"));
  if (!runNode)
  {
    std::cerr << PHWHERE << "Run Node missing, doing nothing." << std::endl;
    throw std::runtime_error("Failed to find Run node in myZDCRawTowerBuilder::CreateNodes");
  }

  PHCompositeNode *dstNode = static_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
  if (!dstNode)
  {
    std::cerr << PHWHERE << "DST Node missing, doing nothing." << std::endl;
    throw std::runtime_error("Failed to find DST node in myZDCRawTowerBuilder::CreateNodes");
  }

  // Create the tower geometry node on the tree
  m_Geoms = new RawTowerGeomContainerv1(m_CaloId);
  string NodeNameTowerGeometries = "TOWERGEOM_" + m_SubDetector;

  PHIODataNode<PHObject> *geomNode = new PHIODataNode<PHObject>(m_Geoms, NodeNameTowerGeometries, "PHObject");
  runNode->addNode(geomNode);

  // Find detector node (or create new one if not found)
  PHNodeIterator dstiter(dstNode);
  PHCompositeNode *DetNode = dynamic_cast<PHCompositeNode *>(dstiter.findFirst(
      "PHCompositeNode", m_SubDetector));
  if (!DetNode)
  {
    DetNode = new PHCompositeNode(m_SubDetector);
    dstNode->addNode(DetNode);
  }

  // Create the tower nodes on the tree
  m_Towers = new RawTowerContainer(m_CaloId);
  string NodeNameTowers;
  if (m_SimTowerNodePrefix.empty())
  {
    // no prefix, consistent with older convension
    NodeNameTowers = "TOWER_" + m_SubDetector;
  }
  else
  {
    NodeNameTowers = "TOWER_" + m_SimTowerNodePrefix + "_" + m_SubDetector;
  }

  PHIODataNode<PHObject> *towerNode = new PHIODataNode<PHObject>(m_Towers, NodeNameTowers, "PHObject");
  DetNode->addNode(towerNode);

  return;
}

bool myZDCRawTowerBuilder::ReadGeometryFromTable()
{
  /* Stream to read table from file */
  ifstream istream_mapping;

  if(m_MappingTowerFile.find(m_SubDetector)==std::string::npos){
    cerr<<"CaloTowerGeomManager::ReadGeometryFileFromTable - ERROR unmatched mapping file: SubDetector "<<m_SubDetector<<" : mapping file : "<<m_MappingTowerFile<<endl;
  }

  /* Open the datafile, if it won't open return an error */
  if (!istream_mapping.is_open())
  {
    istream_mapping.open(m_MappingTowerFile);
    if (!istream_mapping)
    {
      cerr << "CaloTowerGeomManager::ReadGeometryFromTable - ERROR Failed to open mapping file " << m_MappingTowerFile << endl;
      exit(1);
    }
  }
  
  string line_mapping;

  while (getline(istream_mapping, line_mapping))
  {
    /* Skip lines starting with / including a '#' */
    if (line_mapping.find("#") != string::npos)
    {
      if (Verbosity() > 0)
      {
        cout << "myZDCRawTowerBuilder: SKIPPING line in mapping file: " << line_mapping << endl;
      }
      continue;
    }
    istringstream iss(line_mapping);

    /* If line starts with keyword Tower, add to tower positions */
    if (line_mapping.find("Tower ") != string::npos)
    {
      unsigned idx_L, idx_x, idx_y;
      double pos_x, pos_y, pos_z;
      double size_x, size_y, size_z;
      string dummys;

      /* read string- break if error */
      if (!(iss >> dummys >> idx_L >> idx_x >> idx_y >> pos_x >> pos_y >> pos_z >> size_x >> size_y >> size_z))
      {
        cerr << "ERROR in myZDCRawTowerBuilder: Failed to read line in mapping file " << m_MappingTowerFile << endl;
        exit(1);
      }

      //      for (int il = 0; il < m_NTowerSeg; il++){
        /* Construct unique Tower ID */
      unsigned int temp_id = RawTowerDefs::encode_towerid_zdc(m_CaloId, idx_x, idx_y, idx_L);

      /* Create tower geometry object */
      RawTowerGeom *temp_geo = new RawTowerGeomv3(temp_id);
      temp_geo->set_center_x(pos_x);
      temp_geo->set_center_y(pos_y);
      temp_geo->set_center_z(pos_z);
      temp_geo->set_size_x(size_x);
      temp_geo->set_size_y(size_y);
      temp_geo->set_size_z(size_z);
  
      // float blocklength = m_NLayersPerTowerSeg*(m_ThicknessAbsorber+m_ThicknessScintilator);
      // temp_geo->set_center_z(pos_z-0.5*size_z+(il+0.5)*blocklength);
      // temp_geo->set_size_z(m_NLayersPerTowerSeg*(m_ThicknessAbsorber+m_ThicknessScintilator));
      // temp_geo->set_tower_type((int) type);

      /* Insert this tower into position map */
      m_Geoms->add_tower_geometry(temp_geo);
	//    }
    }

    /* If line does not start with keyword Tower, read as parameter */
    else
    {
      /* If this line is not a comment and not a tower, save parameter as string / value. */
      string parname;
      double parval;

      /* read string- break if error */
      if (!(iss >> parname >> parval))
      {
        cerr << "ERROR in myZDCRawTowerBuilder: Failed to read line in mapping file " << m_MappingTowerFile << endl;
        exit(1);
      }

      m_GlobalParameterMap.insert(make_pair(parname, parval));
      
      /* Update member variables for global parameters based on parsed parameter file */
      std::map<string, double>::iterator parit;

      parit = m_GlobalParameterMap.find("Gx0");
      if (parit != m_GlobalParameterMap.end())
        m_GlobalPlaceInX = parit->second;

      parit = m_GlobalParameterMap.find("Gy0");
      if (parit != m_GlobalParameterMap.end())
        m_GlobalPlaceInY = parit->second;

      parit = m_GlobalParameterMap.find("Gz0");
      if (parit != m_GlobalParameterMap.end())
        m_GlobalPlaceInZ = parit->second;

      parit = m_GlobalParameterMap.find("Grot_x");
      if (parit != m_GlobalParameterMap.end())
        m_RotInX = parit->second;

      parit = m_GlobalParameterMap.find("Grot_y");
      if (parit != m_GlobalParameterMap.end())
        m_RotInY = parit->second;

      parit = m_GlobalParameterMap.find("Grot_z");
      if (parit != m_GlobalParameterMap.end())
        m_RotInZ = parit->second;

      
      if (Verbosity() > 1)
      {
        // std::cout << "RawTowerBuilder LHCal - Absorber: " << m_ThicknessAbsorber << " cm\t Scintilator: "<< m_ThicknessScintilator << " cm\t #Layers: " << m_NLayers << "\t layers per segment: " << m_NLayersPerTowerSeg <<  "\t #segment: "<< m_NTowerSeg << std::endl;
      }
    }
  }
    
  /* Correct tower geometries for global calorimter translation / rotation 
   * after reading parameters from file */
  RawTowerGeomContainer::ConstRange all_towers = m_Geoms->get_tower_geometries();

  for (RawTowerGeomContainer::ConstIterator it = all_towers.first;
       it != all_towers.second; ++it)
  {
    double x_temp = it->second->get_center_x();
    double y_temp = it->second->get_center_y();
    double z_temp = it->second->get_center_z();

    /* Rotation */
    TRotation rot;
    rot.RotateX(m_RotInX);
    rot.RotateY(m_RotInY);
    rot.RotateZ(m_RotInZ);

    TVector3 v_temp_r(x_temp, y_temp, z_temp);
    v_temp_r.Transform(rot);

    /* Translation */
    double x_temp_rt = v_temp_r.X() + m_GlobalPlaceInX;
    double y_temp_rt = v_temp_r.Y() + m_GlobalPlaceInY;
    double z_temp_rt = v_temp_r.Z() + m_GlobalPlaceInZ;

    /* Update tower geometry object */
    it->second->set_center_x(x_temp_rt);
    it->second->set_center_y(y_temp_rt);
    it->second->set_center_z(z_temp_rt);

    if (Verbosity() > 2)
    {
      cout << "* Local tower x y z : " << x_temp << " " << y_temp << " " << z_temp << endl;
      cout << "* Globl tower x y z : " << x_temp_rt << " " << y_temp_rt << " " << z_temp_rt << endl;
    }
  }

  return true;
}