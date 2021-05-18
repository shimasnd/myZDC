//____________________________________________________________________________..
//
// This is a working template for the G4 Construct() method which needs to be implemented
// We wedge a method between the G4 Construct() to enable volume hierarchies on the macro
// so here it is called ConstructMe() but there is no functional difference
// Currently this installs a simple G4Box solid, creates a logical volume from it
// and places it. Put your own detector in place (just make sure all active volumes
// get inserted into the m_PhysicalVolumesSet)
// 
// Rather than using hardcoded values you should consider using the parameter class
// Parameter names and defaults are set in myZDCSubsystem::SetDefaultParameters()
// Only parameters defined there can be used (also to override in the macro)
// to avoids typos.
// IMPORTANT: parameters have no inherent units, there is a convention (cm/deg)
// but in any case you need to multiply them here with the correct CLHEP/G4 unit 
// 
// The place where you put your own detector is marked with
// //begin implement your own here://
// //end implement your own here://
// Do not forget to include the G4 includes for your volumes
//____________________________________________________________________________..

#include "myZDCDetector.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4Detector.h>

#include <Geant4/G4Box.hh>
#include <Geant4/G4Color.hh>
#include <Geant4/G4LogicalVolume.hh>
#include <Geant4/G4Material.hh>
#include <Geant4/G4PVPlacement.hh>
#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/G4VisAttributes.hh>
#include <Geant4/G4NistManager.hh>

#include <cmath>
#include <iostream>

class G4VSolid;
class PHCompositeNode;

using namespace std;

//____________________________________________________________________________..
myZDCDetector::myZDCDetector(PHG4Subsystem *subsys,
                                         PHCompositeNode *Node,
                                         PHParameters *parameters,
                                         const std::string &dnam)
  : PHG4Detector(subsys, Node, dnam)
  , m_Params(parameters)
{
}

//_______________________________________________________________
int myZDCDetector::IsInDetector(G4VPhysicalVolume *volume) const
{
  set<G4VPhysicalVolume *>::const_iterator iter = m_PhysicalVolumesSet.find(volume);
  if (iter != m_PhysicalVolumesSet.end())
  {
    return 1;
  }
  return 0;
}

//_______________________________________________________________
void myZDCDetector::ConstructMe(G4LogicalVolume *logicWorld)
{
 //begin implement your own here://
 // Do not forget to multiply the parameters with their respective CLHEP/G4 unit !
  double xdim = m_Params->get_double_param("size_x") * cm;
  double ydim = m_Params->get_double_param("size_y") * cm;
  double zdim = m_Params->get_double_param("size_z") * cm;
  G4VSolid *solidbox = new G4Box("myZDCSolid", xdim / 2., ydim / 2., zdim / 2.);
  G4LogicalVolume *logical = new G4LogicalVolume(solidbox, G4Material::GetMaterial("G4_Galactic"), "myZDCLogical");

  G4VisAttributes *vis = new G4VisAttributes(G4Color(G4Colour::Grey()));  // grey is good to see the tracks in the display
  vis->SetForceSolid(true);
  logical->SetVisAttributes(vis);
  G4RotationMatrix *rotm = new G4RotationMatrix();
  rotm->rotateX(m_Params->get_double_param("rot_x") * rad);
  rotm->rotateY(m_Params->get_double_param("rot_y") * rad);
  rotm->rotateZ(m_Params->get_double_param("rot_z") * rad);

  m_gPhy = new G4PVPlacement(
      rotm,
      G4ThreeVector(m_Params->get_double_param("place_x") * cm,
                    m_Params->get_double_param("place_y") * cm,
                    m_Params->get_double_param("place_z") * cm),
      logical, "myZDC", logicWorld, 0, false, OverlapCheck());
  // add it to the list of placed volumes so the IsInDetector method
  // picks them up
  ConstructStructure();

  m_PhysicalVolumesSet.insert(m_gPhy);
 //end implement your own here://
  return;
}

void myZDCDetector::ConstructStructure()
{

  double gsizex = m_Params->get_double_param("size_x") * cm;
  double gsizey = m_Params->get_double_param("size_y") * cm;
  double gsizez = m_Params->get_double_param("size_z") * cm;
  double esizez = 40.* cm;

  G4Material *fMat;
  G4double sizex;
  G4double sizey;
  G4double sizez;
  G4double widx;
  G4double widy;
  G4double widz;
  G4double x_c;
  G4double y_c;
  G4double z_c;

  //ECAL crystal
  fMat = G4NistManager::Instance()->FindOrBuildMaterial("G4_PbWO4");
  int nEx=15;
  int nEy=15;
  int nEz=2;
  sizex = gsizex;
  sizey = gsizey;
  sizez = esizez;
  widx = sizex/nEx;
  widy = sizey/nEy;
  widz = sizez/nEz;
  
  G4Box *ESolid = new G4Box("myZDC_ESolid",widx*0.5 ,widy*0.5, widz*0.5);
  G4LogicalVolume *ELogic = new G4LogicalVolume(ESolid, fMat, "myZDC_ELogical");

  G4VisAttributes *Evis  = new G4VisAttributes(G4Color(G4Colour::Yellow()));
  Evis->SetForceSolid(true);
  ELogic->SetVisAttributes(Evis);

  int k=0;
  for(int l=0; l<nEz; l++){
    z_c = - gsizez* 0.5 + widz*(l+0.5);
    for(int j=0; j<nEy; j++){
      y_c = - gsizey * 0.5 +widy*(j+0.5);
      for(int i=0; i<nEx; i++){
	x_c = -gsizex * 0.5 +widx*(i+0.5);
	
	std::string cellname = "myZDC_E"+std::to_string(k);
	new G4PVPlacement(0, G4ThreeVector(x_c, y_c, z_c), cellname,ELogic,m_gPhy,false,k);
	
	k++;
      }
    }
  }

  //HCAL Tungsten
  fMat = G4NistManager::Instance()->FindOrBuildMaterial("G4_W");
  int nHx=12;
  int nHy=12;
  int nHz=32;
  sizex = gsizex;
  sizey = gsizey;
  sizez = gsizez - esizez;
  widx = sizex/nHx;
  widy = sizey/nHy;
  widz = sizez/nHz;
  
  G4Box *HSolid = new G4Box("myZDC_HSolid",widx*0.5 ,widy*0.5, widz*0.5);
  G4LogicalVolume *HLogic = new G4LogicalVolume(HSolid, fMat, "myZDC_HLogical");

  G4VisAttributes *Hvis  = new G4VisAttributes(G4Color(G4Colour::Green()));
  Hvis->SetForceSolid(true);
  HLogic->SetVisAttributes(Hvis);

  k=0;
  for(int l=0; l<nHz; l++){
    z_c = - gsizez* 0.5 + esizez + widz*(l+0.5);
    for(int j=0; j<nHy; j++){
      y_c = - gsizey * 0.5 +widy*(j+0.5);
      for(int i=0; i<nHx; i++){
	x_c = -gsizex * 0.5 +widx*(i+0.5);
	
	std::string cellname = "myZDC_Z"+std::to_string(k);
	new G4PVPlacement(0, G4ThreeVector(x_c, y_c, z_c), cellname,HLogic,m_gPhy,false,k);
	k++;
      }
    }
  } 

  return;
}
//_______________________________________________________________
void myZDCDetector::Print(const std::string &what) const
{
  std::cout << "myZDC Detector:" << std::endl;
  if (what == "ALL" || what == "VOLUME")
  {
    std::cout << "Version 0.1" << std::endl;
    std::cout << "Parameters:" << std::endl;
    m_Params->Print();
  }
  return;
}
