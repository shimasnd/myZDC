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
#include "myZDCStructure.h"

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
#include <Geant4/G4Material.hh>

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

  G4LogicalVolume *lvolume = volume->GetLogicalVolume();
  set<G4LogicalVolume *>::const_iterator iter_active  
    = m_ActiveLogicalVolumesSet.find(lvolume);
  if (iter_active != m_ActiveLogicalVolumesSet.end())  return 1;
  set<G4LogicalVolume *>::const_iterator iter_absorber  
    = m_AbsorberLogicalVolumesSet.find(lvolume);
  if (iter_absorber != m_AbsorberLogicalVolumesSet.end())  return -1;
  
  return 0;
}

int myZDCDetector::GetActiveVolumeInfo(G4VPhysicalVolume *volume){

  G4LogicalVolume *lvolume = volume->GetLogicalVolume();
  int lvinfo = m_ActiveLogicalVolumeInfoMap[lvolume];
  return lvinfo;
}

int myZDCDetector::GetAbsorberVolumeInfo(G4VPhysicalVolume *volume){

  G4LogicalVolume *lvolume = volume->GetLogicalVolume();
  int lvinfo = m_AbsorberLogicalVolumeInfoMap[lvolume];
  return lvinfo;
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
  G4LogicalVolume *logical = new G4LogicalVolume(solidbox, G4Material::GetMaterial("G4_AIR"), "myZDCLogical");
  //  G4LogicalVolume *logical = new G4LogicalVolume(solidbox, G4Material::GetMaterial("G4_Galactic"), "myZDCLogical");

  logical->SetVisAttributes(G4VisAttributes::Invisible);
  G4RotationMatrix *rotm = new G4RotationMatrix();
  rotm->rotateX(m_Params->get_double_param("rot_x") * rad);
  rotm->rotateY(m_Params->get_double_param("rot_y") * rad);
  rotm->rotateZ(m_Params->get_double_param("rot_z") * rad);
  
  G4VPhysicalVolume *gPhy = new G4PVPlacement(
      rotm,
      G4ThreeVector(m_Params->get_double_param("place_x") * cm,
                    m_Params->get_double_param("place_y") * cm,
                    m_Params->get_double_param("place_z") * cm),
      logical, "myZDC", logicWorld, 0, false, OverlapCheck());

  string mat_crystal = m_Params->get_string_param("crystal");
  myZDCStructure *mzs = new myZDCStructure(mat_crystal);
  double endz = -zdim/2.;
  cout<<"Z size: "<<zdim<<endl;

  endz = mzs->ConstructCrystalTowers(-xdim/2.,-ydim/2.,-zdim/2.,
  				     xdim/2., ydim/2., zdim/2., gPhy);
  std::cout<<"After Crystal part                   :"<<endz<<std::endl;
  
  endz = mzs->ConstructEMLayers(-xdim *0.5, -ydim*0.5, endz,
  				xdim*0.5, ydim*0.5, zdim*0.5, gPhy);
  std::cout<<"After EM Layers part                 :"<<endz<<std::endl;

  endz = mzs->ConstructHCSiliconLayers(-xdim *0.5, -ydim*0.5, endz + 20.*mm,
  				       xdim*0.5, ydim*0.5, zdim*0.5, gPhy);
  std::cout<<"After HCal Silicon part              :"<<endz<<std::endl;

  endz = mzs->ConstructHCSciLayers(-xdim *0.5, -ydim*0.5, endz + 20.*mm,
  				   xdim*0.5, ydim*0.5, zdim*0.5, gPhy);
  std::cout<<"After HCal Sci part  (incl. 2cm gap) :"<<endz<<std::endl;
  
  mzs->ProvideLogicalVolumesSets(m_ActiveLogicalVolumesSet, 
  				 m_AbsorberLogicalVolumesSet);
  mzs->ProvideLogicalVolumeInfoMap(m_ActiveLogicalVolumeInfoMap,
				   m_AbsorberLogicalVolumeInfoMap);

  // mzs->PrintTowerMap("Crystal");
  // mzs->PrintTowerMap("SiPixel");
  // mzs->PrintTowerMap("SiPad");
  //  mzs->PrintTowerMap("Sci");

 //end implement your own here://
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

