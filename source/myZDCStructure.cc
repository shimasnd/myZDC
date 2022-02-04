#include "myZDCStructure.h"
#include "constants.h"
#include "zdcdetid.h"

#include <Geant4/G4LogicalVolume.hh>
#include <Geant4/G4VPhysicalVolume.hh>
#include <Geant4/G4PVPlacement.hh>
#include <Geant4/G4PVReplica.hh>
#include <Geant4/G4Box.hh>
#include <Geant4/G4NistManager.hh>
#include <Geant4/G4VisAttributes.hh>
#include <Geant4/G4Color.hh>


myZDCStructure::myZDCStructure(const std::string &crystal) {
  std::cout<<"myZDCStructure:: Use FoCal Layout"<<std::endl;
  std::cout<<"myZDCStructure:: Pad Layer"<<PAD_Layer_Thickness<<"mm thick"<<std::endl;
  std::cout<<"myZDCStructure:: Pix Layer"<<PIX_Layer_Thickness<<"mm thick"<<std::endl;
  std::cout<<"myZDCStructure:: HCAL Layer"<<HCal_Layer_Thickness<<"mm thick"<<std::endl;
  std::cout<<"myZDCStructure:: Crystal uses "<<crystal<<"  (default: PbWO4)"<<std::endl;

  if(crystal=="PbWO4") _id_Crystal = _id_PbWO4;
  else if(crystal=="LYSO") _id_Crystal = _id_LYSO;
  else if(crystal=="LYSOCe") _id_Crystal = _id_LYSOCe;
  else _id_Crystal = _id_PbWO4;

  Materials();
  SetColors();
  fLayer=0;
  _z_Crystal[0]=0; 
  _z_Crystal[1]=0;
  _z_EMLayers[0]=0; 
  _z_EMLayers[1]=0;
  _z_HCSilicon[0]=0; 
  _z_HCSilicon[1]=0;
  _z_HCSci[0]=0; 
  _z_HCSci[1]=0;
  
}
myZDCStructure::~myZDCStructure() {}

void myZDCStructure::ProvideLogicalVolumesSets(std::set<G4LogicalVolume *> &ActiveLogicalVolumesSet,
					       std::set<G4LogicalVolume *> &AbsorberLogicalVolumesSet){

  ActiveLogicalVolumesSet  =  m_ActiveLogicalVolumesSet;
  AbsorberLogicalVolumesSet=  m_AbsorberLogicalVolumesSet;

  return;

}

void myZDCStructure::ProvideLogicalVolumeInfoMap(std::map<G4LogicalVolume *, int> &ActiveLogicalVolumeInfoMap,
						 std::map<G4LogicalVolume *, int> &AbsorberLogicalVolumeInfoMap){
					       
  ActiveLogicalVolumeInfoMap = m_ActiveLogicalVolumeInfoMap;
  AbsorberLogicalVolumeInfoMap = m_AbsorberLogicalVolumeInfoMap;

  return;

}

double myZDCStructure::ConstructCrystalTowers(double Start_X, double Start_Y, double Start_Z, 
					      double End_X, double End_Y, double End_Z,
					      G4VPhysicalVolume *motherPhy) {
  _z_Crystal[0] = Start_Z;

  double Center_X = (Start_X + End_X)/2.;
  double Center_Y = (Start_Y + End_Y)/2.;
  double Width_X = End_X - Start_X;
  double Width_Y = End_Y - Start_Y;

  G4Box* PIX_Silicon = new G4Box("CPIX_Silicon",	PIX_X/2.0,   PIX_Y/2.0,   PIX_Z/2.0); 
  G4Box* PIX_Glue2   = new G4Box("CPIX_Glue2",	Width_X/2.0, Width_Y/2.0, PIX_Glue2_Z/2.0);
  G4Box* PIX_FPC     = new G4Box("CPIX_FPC",	Width_X/2.0, Width_Y/2.0, PIX_FPC_Z/2.0);
  G4Box *Crystal     = new G4Box("Crystal",     CTower_X*0.5 ,CTower_Y*0.5, CTower_Z*0.5);
  G4Box *CrysEnvelope= new G4Box("CrysEnvelope",CTower_X*0.5 ,Width_Y*0.5, CTower_Z*0.5);
  G4Box *CrysBox     = new G4Box("CrysBox",     Width_X*0.5 ,Width_Y*0.5, CTower_Z*0.5);
  G4Box *PIXPlane    = new G4Box("CPIXPlane",   Width_X/2.0, Width_Y/2.0, PIX_Z/2.0);
  G4Box *PIXEnvelope = new G4Box("CPIXEnvelope",PIX_X/2.0, Width_Y/2.0, PIX_Z/2.0);

  G4LogicalVolume* lV_PIX_Silicon = new G4LogicalVolume( PIX_Silicon,     fmat_Si, "lV_Crystal_PIX_Silicon" );
  G4LogicalVolume* lV_PIX_Glue2   = new G4LogicalVolume( PIX_Glue2, 	fmat_PET, "lV_Crystal_PIX_Glue2");
  G4LogicalVolume* lV_PIX_FPC 	  = new G4LogicalVolume( PIX_FPC,  fmat_PET, "lV_Crystal_PIX_FPC");
  G4LogicalVolume* lV_Crystal     = new G4LogicalVolume( Crystal, fmat_Crystal, "lV_Crystal");
  G4LogicalVolume* lV_CrysEnvelope= new G4LogicalVolume( CrysEnvelope, fmat_World, "lV_CrysEnvelope");
  G4LogicalVolume* lV_CrysBox     = new G4LogicalVolume( CrysBox, fmat_World, "lV_CrysBox");
  G4LogicalVolume* lV_PIXPlane    = new G4LogicalVolume( PIXPlane, fmat_World, "lV_CPIXPlane");
  G4LogicalVolume* lV_PIXEnvelope = new G4LogicalVolume( PIXEnvelope, fmat_World, "lV_CPIXEnvelope");

  lV_Crystal->SetVisAttributes(fvisCrystal);
  lV_PIX_Silicon->SetVisAttributes(fvisPIX);
  lV_PIX_Glue2->SetVisAttributes(fvisDM);
  lV_PIX_FPC->SetVisAttributes(fvisDM);
  
  lV_CrysBox->SetVisAttributes(G4VisAttributes::Invisible);
  lV_CrysEnvelope->SetVisAttributes(G4VisAttributes::Invisible);
  lV_PIXPlane->SetVisAttributes(G4VisAttributes::Invisible);
  lV_PIXEnvelope->SetVisAttributes(G4VisAttributes::Invisible);
  
  std::pair<G4LogicalVolume*, int> pair_Crystal= std::make_pair(lV_Crystal, fLayer*100 + ZDCID::Crystal + ZDCID::CrystalTower);
  std::pair<G4LogicalVolume*, int> pair_CPIX = std::make_pair(lV_PIX_Silicon, fLayer*100 + ZDCID::SI_PIXEL + ZDCID::CrystalTower);
  m_ActiveLogicalVolumeInfoMap.insert(pair_Crystal);
  m_ActiveLogicalVolumeInfoMap.insert(pair_CPIX);
  std::pair<G4LogicalVolume*, int> pair_Glue2 = std::make_pair(lV_PIX_Glue2, fLayer*100 + ZDCID::Materials + ZDCID::CrystalTower);
  std::pair<G4LogicalVolume*, int> pair_FPC = std::make_pair(lV_PIX_FPC, fLayer*100 + ZDCID::Materials + ZDCID::CrystalTower);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_Glue2);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_FPC);

  m_ActiveLogicalVolumesSet.insert(lV_PIX_Silicon);
  m_ActiveLogicalVolumesSet.insert(lV_Crystal);
  m_AbsorberLogicalVolumesSet.insert(lV_PIX_Glue2);
  m_AbsorberLogicalVolumesSet.insert(lV_PIX_FPC);

  //Making PIX layers using Replica

  int NdivX = (int)(Width_X/ PIX_X);
  new G4PVReplica("PV_CPIXEnvelope", lV_PIXEnvelope, lV_PIXPlane, kXAxis, NdivX, PIX_X,0);
  int NdivY = (int)(Width_Y/ PIX_Y);
  new G4PVReplica("PV_CPIX", lV_PIX_Silicon, lV_PIXEnvelope, kYAxis, NdivY, PIX_Y,0);

  //Making Crystal Box using Replica
  new G4PVReplica("PV_CrysEnvelope", lV_CrysEnvelope, lV_CrysBox, kXAxis, nCTowerX, CTower_X,0);
  new G4PVReplica("PV_Crystal", lV_Crystal, lV_CrysEnvelope, kYAxis, nCTowerY, CTower_Y,0);

  //*********************
  //Now crete nCTowerZ+1 PIX layers with nCTowerZ Tower layers
  //*******************

  double offsetZ=0;  
  int LayerID = fLayer;

  for(int ilayer=0; ilayer<nCTowerZ+1; ilayer++){
    G4double position_Z_PIX_Silicon = Start_Z + offsetZ + PIX_Z/2.;
    G4double position_Z_PIX_Glue2   = Start_Z + offsetZ + PIX_Z + PIX_Glue2_Z/2.;
    G4double position_Z_PIX_FPC	    = Start_Z + offsetZ + PIX_Z + PIX_Glue2_Z + PIX_FPC_Z/2.;
    offsetZ += PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap;
   
    G4ThreeVector threeVect_PIX_Silicon   = G4ThreeVector(Center_X, Center_Y, position_Z_PIX_Silicon);      
    G4ThreeVector threeVect_PIX_Glue2 = G4ThreeVector(Center_X, Center_Y, position_Z_PIX_Glue2);
    G4ThreeVector threeVect_PIX_FPC  = G4ThreeVector(Center_X, Center_Y, position_Z_PIX_FPC);
      
    std::string ss_PIX_Plane = "PhysVol_CPIXPlane" + std::to_string(ilayer) + "_L" + std::to_string(LayerID);
    std::string ss_PIX_Glue2 = "PhysVol_CGlue2_"   + std::to_string(ilayer);
    std::string ss_PIX_FPC  = "PhysVol_CFPC_"      + std::to_string(ilayer);

    new G4PVPlacement(0, threeVect_PIX_Silicon, ss_PIX_Plane, lV_PIXPlane, motherPhy, false, ilayer);      
    new G4PVPlacement(0, threeVect_PIX_Glue2, ss_PIX_Glue2, 	lV_PIX_Glue2,   motherPhy, false, ilayer);
    new G4PVPlacement(0, threeVect_PIX_FPC,   ss_PIX_FPC, 	lV_PIX_FPC,     motherPhy, false, ilayer);

    LayerID  +=2; 
    offsetZ +=CTower_Z + CTower_GAP;
  }

  offsetZ = PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap;
  LayerID = fLayer+1;

  for (int ilayer =0; ilayer<nCTowerZ; ilayer++){
    G4double position_Z_Crystal = Start_Z + offsetZ + CTower_Z/2.;

    G4ThreeVector threeVect_Crystal  = G4ThreeVector(Center_X, Center_Y, position_Z_Crystal);
    std::string ss_Crystal = "PhysVol_Crystal_L"+std::to_string(LayerID);
    
    new G4PVPlacement(0, threeVect_Crystal, ss_Crystal, lV_CrysBox, motherPhy, false, ilayer);     
    
    LayerID += 2;
    offsetZ += CTower_Z + CTower_GAP + PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap;
  }
  
  fLayer += 2*nCTowerZ +1;

  _z_Crystal[1] = Start_Z + (CTower_Z + CTower_GAP) * nCTowerZ + (PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap) * (nCTowerZ +1); 
  return _z_Crystal[1];

}

double myZDCStructure::ConstructEMLayers(double Start_X, double Start_Y, double Start_Z, 
			     double End_X, double End_Y, double End_Z,
			     G4VPhysicalVolume *motherPhy) {
  
  _z_EMLayers[0] = Start_Z;

  double Center_X = (Start_X + End_X)/2.;
  double Center_Y = (Start_Y + End_Y)/2.;
  double Width_X = End_X - Start_X;
  double Width_Y = End_Y - Start_Y;
  double PadOnlyThickness = PAD_Layer_Thickness * NPadOnlyLayers;

  //*****************************************************************************************
  //G4box is the material
  //Ignoring the width coming from the Guard Ring Thickness. 
  //PAD layer
  G4Box* PAD_W 		= new G4Box("PPAD_W",		Width_X/2.0, Width_Y/2.0, PAD_Absorber_Z/2.0);
  G4Box* PAD_Glue1 	= new G4Box("PPAD_Glue1",	Width_X/2.0, Width_Y/2.0, PAD_Glue1_Z/2.0);
  G4Box* PAD_Silicon 	= new G4Box("PPAD_Silicon",	PAD_X/2.0, 	     PAD_Y/2.0, 	  PAD_Z/2.0); 
  G4Box* PAD_Glue2 	= new G4Box("PPAD_Glue2",	Width_X/2.0, Width_Y/2.0, PAD_Glue2_Z/2.0);
  G4Box* PAD_FPC 	= new G4Box("PPAD_FPC",		Width_X/2.0, Width_Y/2.0, PAD_FPC_Z/2.0);
  G4Box *PAD_Plane    = new G4Box("PAD_Plane",    Width_X/2.0, Width_Y/2.0, PAD_Z/2.0);
  G4Box *PAD_Envelope = new G4Box("PAD_Envelope", PAD_X/2.0, Width_Y/2.0, PAD_Z/2.0);
  G4Box *PAD_Layer    = new G4Box("PAD_Layer",  Width_X/2.0, Width_Y/2.0, PAD_Layer_Thickness/2.0);
  G4Box *PADonlyBox   = new G4Box("PADonlyBox", Width_X/2.0, Width_Y/2.0, PadOnlyThickness/2.0);

  //PIX layer
  G4Box* PIX_W 		= new G4Box("PPIX_W",		Width_X/2.0, Width_Y/2.0, PIX_Absorber_Z/2.0);
  G4Box* PIX_Glue1 	= new G4Box("PPIX_Glue1",	Width_X/2.0, Width_Y/2.0, PIX_Glue1_Z/2.0);
  G4Box* PIX_Silicon 	= new G4Box("PPIX_Silicon",	PIX_X/2.0, 	     PIX_Y/2.0, 	  PIX_Z/2.0); 
  G4Box* PIX_Glue2 	= new G4Box("PPIX_Glue2",	Width_X/2.0, Width_Y/2.0, PIX_Glue2_Z/2.0);
  G4Box* PIX_FPC 	= new G4Box("PPIX_FPC",		Width_X/2.0, Width_Y/2.0, PIX_FPC_Z/2.0);
  G4Box *PIX_Plane    = new G4Box("PIX_Plane",    Width_X/2.0, Width_Y/2.0, PIX_Z/2.0);
  G4Box *PIX_Envelope = new G4Box("PIX_Envelope", PIX_X/2.0, Width_Y/2.0, PIX_Z/2.0);
  G4Box *PIX_Layer    = new G4Box("PIX_Layer",  Width_X/2.0, Width_Y/2.0, PIX_Layer_Thickness/2.0);

  //*****************************************************************************************
  //   Logical volumes
  //*****************************************************************************************
  //PAD
  G4LogicalVolume* lV_PAD_W 	  = new G4LogicalVolume( PAD_W,        fmat_W,   "lV_PAD_W");
  G4LogicalVolume* lV_PAD_Glue1   = new G4LogicalVolume( PAD_Glue1,    fmat_PET, "lV_PAD_Glue1" );
  G4LogicalVolume* lV_PAD_Silicon = new G4LogicalVolume( PAD_Silicon,  fmat_Si,  "lV_PAD_Silicon" );
  G4LogicalVolume* lV_PAD_Glue2   = new G4LogicalVolume( PAD_Glue2,    fmat_PET, "lV_PAD_Glue2");
  G4LogicalVolume* lV_PAD_FPC 	  = new G4LogicalVolume( PAD_FPC,      fmat_PET, "lV_PAD_FPC");
  G4LogicalVolume* lV_PADPlane    = new G4LogicalVolume( PAD_Plane,    fmat_World, "lV_PADPlane");
  G4LogicalVolume* lV_PADEnvelope = new G4LogicalVolume( PAD_Envelope, fmat_World, "lV_PADEnvelope");
  G4LogicalVolume* lV_PADLayer    = new G4LogicalVolume( PAD_Layer,    fmat_World, "lV_PADLayer");
  G4LogicalVolume* lV_PADonlyBox  = new G4LogicalVolume( PADonlyBox,   fmat_World, "lV_PADonlyBox");

  //PIX
  G4LogicalVolume* lV_PIX_W 	= new G4LogicalVolume( PIX_W, 	    fmat_W, "lV_PIX_W");
  G4LogicalVolume* lV_PIX_Glue1 	= new G4LogicalVolume( PIX_Glue1,   fmat_PET, "lV_PIX_Glue1");
  G4LogicalVolume* lV_PIX_Silicon	= new G4LogicalVolume( PIX_Silicon, fmat_Si, "lV_PIX_Silicon" );
  G4LogicalVolume* lV_PIX_Glue2 	= new G4LogicalVolume( PIX_Glue2,   fmat_PET, "lV_PIX_Glue2" );
  G4LogicalVolume* lV_PIX_FPC 	= new G4LogicalVolume( PIX_FPC,  fmat_PET, "lV_PIX_FPC");
  G4LogicalVolume* lV_PIXPlane    = new G4LogicalVolume( PIX_Plane, fmat_World, "lV_PIXPlane");
  G4LogicalVolume* lV_PIXEnvelope = new G4LogicalVolume( PIX_Envelope, fmat_World, "lV_PIXEnvelope");
  G4LogicalVolume* lV_PIXLayer    = new G4LogicalVolume( PIX_Layer, fmat_World, "lV_PIXLayer");

  lV_PAD_W->SetVisAttributes(fvisW);
  lV_PIX_W->SetVisAttributes(fvisW);
		  
  lV_PAD_Silicon->SetVisAttributes(fvisPAD);
  lV_PIX_Silicon->SetVisAttributes(fvisPIX);

  lV_PAD_Glue1->SetVisAttributes(fvisDM);
  lV_PAD_Glue2->SetVisAttributes(fvisDM);
  lV_PAD_FPC->SetVisAttributes(fvisDM);
  lV_PIX_Glue1->SetVisAttributes(fvisDM);
  lV_PIX_Glue2->SetVisAttributes(fvisDM);
  lV_PIX_FPC->SetVisAttributes(fvisDM);

  lV_PADPlane->SetVisAttributes(G4VisAttributes::Invisible);
  lV_PADEnvelope->SetVisAttributes(G4VisAttributes::Invisible);
  lV_PADLayer->SetVisAttributes(G4VisAttributes::Invisible);
  lV_PADonlyBox->SetVisAttributes(G4VisAttributes::Invisible);
  lV_PIXPlane->SetVisAttributes(G4VisAttributes::Invisible);
  lV_PIXEnvelope->SetVisAttributes(G4VisAttributes::Invisible);
  lV_PIXLayer->SetVisAttributes(G4VisAttributes::Invisible);

  int infoval=0;
  infoval = ZDCID::EMLayer+ NPadOnlyLayers * 10000 + fLayer*100 + ZDCID::SI_PAD;
  std::pair<G4LogicalVolume*, int> pair_PAD_Si = std::make_pair(lV_PAD_Silicon, infoval);
  infoval = ZDCID::EMLayer+ NPadOnlyLayers * 10000 + fLayer*100 + ZDCID::SI_PIXEL;
  std::pair<G4LogicalVolume*, int> pair_PIX_Si = std::make_pair(lV_PIX_Silicon, infoval);
  m_ActiveLogicalVolumeInfoMap.insert(pair_PAD_Si);
  m_ActiveLogicalVolumeInfoMap.insert(pair_PIX_Si);
  infoval = ZDCID::EMLayer+ NPadOnlyLayers * 10000 + fLayer*100 + ZDCID::Abs_Tungsten;
  std::pair<G4LogicalVolume*, int> pair_PAD_W = std::make_pair(lV_PAD_W, infoval);
  std::pair<G4LogicalVolume*, int> pair_PIX_W = std::make_pair(lV_PIX_W, infoval);
  infoval = ZDCID::EMLayer+ NPadOnlyLayers * 10000 + fLayer*100 + ZDCID::Materials;
  std::pair<G4LogicalVolume*, int> pair_PAD_Glue1 = std::make_pair(lV_PAD_Glue1, infoval);
  std::pair<G4LogicalVolume*, int> pair_PIX_Glue1 = std::make_pair(lV_PIX_Glue1, infoval);
  std::pair<G4LogicalVolume*, int> pair_PAD_Glue2 = std::make_pair(lV_PAD_Glue2, infoval);
  std::pair<G4LogicalVolume*, int> pair_PIX_Glue2 = std::make_pair(lV_PIX_Glue2, infoval);
  std::pair<G4LogicalVolume*, int> pair_PAD_FPC = std::make_pair(lV_PAD_FPC, infoval);
  std::pair<G4LogicalVolume*, int> pair_PIX_FPC = std::make_pair(lV_PIX_FPC, infoval);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_PAD_W);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_PIX_W);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_PAD_Glue1);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_PIX_Glue1);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_PAD_Glue2);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_PIX_Glue2);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_PAD_FPC);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_PIX_FPC);
  
  m_ActiveLogicalVolumesSet.insert(lV_PAD_Silicon);
  m_ActiveLogicalVolumesSet.insert(lV_PIX_Silicon);
  m_AbsorberLogicalVolumesSet.insert(lV_PAD_W);
  m_AbsorberLogicalVolumesSet.insert(lV_PAD_Glue1);
  m_AbsorberLogicalVolumesSet.insert(lV_PAD_Glue2);
  m_AbsorberLogicalVolumesSet.insert(lV_PAD_FPC);
  m_AbsorberLogicalVolumesSet.insert(lV_PIX_W);
  m_AbsorberLogicalVolumesSet.insert(lV_PIX_Glue1);
  m_AbsorberLogicalVolumesSet.insert(lV_PIX_Glue2);
  m_AbsorberLogicalVolumesSet.insert(lV_PIX_FPC);

  //Construct a PAD/PIX plane using Replica
  new G4PVReplica("PV_PADEnvelope", lV_PADEnvelope, lV_PADPlane, kXAxis, NpadX, PAD_X,0);
  new G4PVReplica("PV_PAD", lV_PAD_Silicon, lV_PADEnvelope, kYAxis, NpadY, PAD_Y,0);

  new G4PVReplica("PV_PIXEnvelope", lV_PIXEnvelope, lV_PIXPlane, kXAxis, NpixX, PIX_X,0);
  new G4PVReplica("PV_PIX", lV_PIX_Silicon, lV_PIXEnvelope, kYAxis, NpixY, PIX_Y,0);

  //Constract a PAD/PIX layer including glue etc.
  G4double posZ_PAD_Absorber  = -0.5 * PAD_Layer_Thickness + PAD_Absorber_Z/2;
  G4double posZ_PAD_Glue1     = -0.5 * PAD_Layer_Thickness + PAD_Absorber_Z + PAD_Glue1_Z/2 ;
  G4double posZ_PAD_Silicon   = -0.5 * PAD_Layer_Thickness + PAD_Absorber_Z + PAD_Glue1_Z + PAD_Z/2 ;
  G4double posZ_PAD_Glue2     = -0.5 * PAD_Layer_Thickness + PAD_Absorber_Z + PAD_Glue1_Z + PAD_Z + PAD_Glue2_Z/2 ;
  G4double posZ_PAD_FPC	      = -0.5 * PAD_Layer_Thickness + PAD_Absorber_Z + PAD_Glue1_Z + PAD_Z + PAD_Glue2_Z +PAD_FPC_Z/2 ;
  
  G4ThreeVector threeVect_PAD_Silicon = G4ThreeVector(0, 0, posZ_PAD_Silicon);
  G4ThreeVector threeVect_PAD_W       = G4ThreeVector(0, 0, posZ_PAD_Absorber);
  G4ThreeVector threeVect_PAD_Glue1   = G4ThreeVector(0, 0, posZ_PAD_Glue1);
  G4ThreeVector threeVect_PAD_Glue2   = G4ThreeVector(0, 0, posZ_PAD_Glue2);
  G4ThreeVector threeVect_PAD_FPC     = G4ThreeVector(0, 0, posZ_PAD_FPC);

  new G4PVPlacement(0, threeVect_PAD_W,       lV_PAD_W,    "PV_PAD_W", lV_PADLayer,false,0);  
  new G4PVPlacement(0, threeVect_PAD_Glue1,   lV_PAD_Glue1,"PV_PAD_Glue1", lV_PADLayer,false,0);  
  new G4PVPlacement(0, threeVect_PAD_Silicon, lV_PADPlane, "PV_PAD_Plane", lV_PADLayer,false,0);  
  new G4PVPlacement(0, threeVect_PAD_Glue2,   lV_PAD_Glue2,"PV_PAD_Glue2", lV_PADLayer,false,0);  
  new G4PVPlacement(0, threeVect_PAD_FPC,     lV_PAD_FPC,  "PV_PAD_FPC", lV_PADLayer,false,0);  

  G4double posZ_PIX_Absorber  = -0.5 * PIX_Layer_Thickness + PIX_Absorber_Z/2;
  G4double posZ_PIX_Glue1     = -0.5 * PIX_Layer_Thickness + PIX_Absorber_Z + PIX_Glue1_Z/2 ;
  G4double posZ_PIX_Silicon   = -0.5 * PIX_Layer_Thickness + PIX_Absorber_Z + PIX_Glue1_Z + PIX_Z/2 ;
  G4double posZ_PIX_Glue2     = -0.5 * PIX_Layer_Thickness + PIX_Absorber_Z + PIX_Glue1_Z + PIX_Z + PIX_Glue2_Z/2 ;
  G4double posZ_PIX_FPC	      = -0.5 * PIX_Layer_Thickness + PIX_Absorber_Z + PIX_Glue1_Z + PIX_Z + PIX_Glue2_Z +PIX_FPC_Z/2 ;
  
  G4ThreeVector threeVect_PIX_Silicon = G4ThreeVector(0, 0, posZ_PIX_Silicon);
  G4ThreeVector threeVect_PIX_W       = G4ThreeVector(0, 0, posZ_PIX_Absorber);
  G4ThreeVector threeVect_PIX_Glue1   = G4ThreeVector(0, 0, posZ_PIX_Glue1);
  G4ThreeVector threeVect_PIX_Glue2   = G4ThreeVector(0, 0, posZ_PIX_Glue2);
  G4ThreeVector threeVect_PIX_FPC     = G4ThreeVector(0, 0, posZ_PIX_FPC);

  new G4PVPlacement(0, threeVect_PIX_W,       lV_PIX_W,    "PV_PIX_W", lV_PIXLayer,false,0);  
  new G4PVPlacement(0, threeVect_PIX_Glue1,   lV_PIX_Glue1,"PV_PIX_Glue1", lV_PIXLayer,false,0);  
  new G4PVPlacement(0, threeVect_PIX_Silicon, lV_PIXPlane, "PV_PIX_Plane", lV_PIXLayer,false,0);  
  new G4PVPlacement(0, threeVect_PIX_Glue2,   lV_PIX_Glue2,"PV_PIX_Glue2", lV_PIXLayer,false,0);  
  new G4PVPlacement(0, threeVect_PIX_FPC,     lV_PIX_FPC,  "PV_PIX_FPC", lV_PIXLayer,false,0);  

  //construct PAD only layers
  new G4PVReplica("PV_PADLayers", lV_PADLayer, lV_PADonlyBox, kZAxis, NPadOnlyLayers, PAD_Layer_Thickness,0);

  //*****************************************************************************************
  //The PAD and PIX layer thicknesses can be different - this variable remembers the previous layer thickness either PIX or PAD
  //It is changed at the end of the loops
  G4double TotalLayerThickness = 0;

  for(G4int ilayer=0; ilayer<NumberPIX; ilayer++){

    G4double posZ_PAD_Box = Start_Z + TotalLayerThickness + 0.5*PadOnlyThickness;
    G4ThreeVector threeVect_PAD_Box = G4ThreeVector(Center_X, Center_Y, posZ_PAD_Box);
    std::string ss_PADBox = "PADOnlyBox" + std::to_string(ilayer);
    new G4PVPlacement(0,threeVect_PAD_Box, ss_PADBox, lV_PADonlyBox, motherPhy, false, ilayer);
    TotalLayerThickness += PadOnlyThickness;
    
    G4double posZ_PIX_Layer  = Start_Z + TotalLayerThickness + 0.5 *PIX_Layer_Thickness;
    G4ThreeVector threeVect_PIX_Layer = G4ThreeVector(Center_X, Center_Y, posZ_PIX_Layer);
    std::string ss_PIX = "PixLayer" + std::to_string(ilayer);
    new G4PVPlacement(0, threeVect_PIX_Layer,ss_PIX, lV_PIXLayer, motherPhy, false, (NPadOnlyLayers+1) * (ilayer+1) -1);
    TotalLayerThickness += PIX_Layer_Thickness;

  }
   
  if(NumberPAD > NPadOnlyLayers * NumberPIX){
    int NPadtoAdd = NumberPAD - NPadOnlyLayers * NumberPIX;
    for(G4int ilayer =0; ilayer<NPadtoAdd; ilayer++){
      G4double posZ_PAD_Layer  = Start_Z + TotalLayerThickness + 0.5 *PAD_Layer_Thickness;
      G4ThreeVector threeVect_PAD_Layer = G4ThreeVector(Center_X, Center_Y, posZ_PAD_Layer);
      std::string ss_PAD = "PADLayer" + std::to_string(ilayer);
      new G4PVPlacement(0, threeVect_PAD_Layer,ss_PAD, lV_PADLayer, motherPhy, false, ilayer);
      TotalLayerThickness += PAD_Layer_Thickness;
    }
  }
 
  fLayer += NumberOfLayers;
  
  _z_EMLayers[1] = Start_Z+TotalLayerThickness;
  return _z_EMLayers[1];
 
}

double myZDCStructure::ConstructHCSiliconLayers(double Start_X, double Start_Y, double Start_Z, 
					    double End_X, double End_Y, double End_Z,
					    G4VPhysicalVolume *motherPhy) {

  _z_HCSilicon[0] = Start_Z;

  double Center_X = (Start_X + End_X)/2.;
  double Center_Y = (Start_Y + End_Y)/2.;
  double Width_X = End_X - Start_X;
  double Width_Y = End_Y - Start_Y;
  double TotalLayerThickness = HCal_Si_Layer_Thickness * HCALSiNumberOfLayers;

  G4Box* HCal_Absorber	= new G4Box("HCal_SiAbsorber",	Width_X/2.0, Width_Y/2.0, HCAL_Z_Absorber/2.0);
  G4Box* HCal_Layer     = new G4Box("HCal_SiLayer",       Width_X/2.0, Width_Y/2.0, HCal_Si_Layer_Thickness/2.0);
  G4Box* PAD_Glue1 	= new G4Box("HPAD_Glue1",	Width_X/2.0, Width_Y/2.0, PAD_Glue1_Z/2.0);
  G4Box* PAD_Silicon 	= new G4Box("HPAD_Silicon",	PAD_X/2.0, 	     PAD_Y/2.0, 	  PAD_Z/2.0); 
  G4Box* PAD_Glue2 	= new G4Box("HPAD_Glue2",	Width_X/2.0, Width_Y/2.0, PAD_Glue2_Z/2.0);
  G4Box* PAD_FPC 	= new G4Box("HPAD_FPC",		Width_X/2.0, Width_Y/2.0, PAD_FPC_Z/2.0);
  G4Box *PAD_Plane    = new G4Box("HPAD_Plane",    Width_X/2.0, Width_Y/2.0, PAD_Z/2.0);
  G4Box *PAD_Envelope = new G4Box("HPAD_Envelope", PAD_X/2.0, Width_Y/2.0, PAD_Z/2.0);
  G4Box *HC_Si_Box    = new G4Box("HC_Si_Box",     Width_X/2., Width_Y/2., TotalLayerThickness/2.);
  
  G4LogicalVolume* lV_HCal_Absorber= new G4LogicalVolume( HCal_Absorber, fmat_Pb, "lV_HCal_SiAbsorber");
  G4LogicalVolume* lV_HCal_Layer  = new G4LogicalVolume( HCal_Layer, fmat_World, "lV_HCal_SiLayer");
  G4LogicalVolume* lV_PAD_Glue1   = new G4LogicalVolume( PAD_Glue1, 	fmat_PET, "lV_HCal_PAD_Glue1" );
  G4LogicalVolume* lV_PAD_Silicon = new G4LogicalVolume( PAD_Silicon,     fmat_Si, "lV_HCal_PAD_Silicon" );
  G4LogicalVolume* lV_PAD_Glue2   = new G4LogicalVolume( PAD_Glue2, 	fmat_PET, "lV_HCal_PAD_Glue2");
  G4LogicalVolume* lV_PAD_FPC 	  = new G4LogicalVolume( PAD_FPC,  fmat_PET, "lV_HCal_PAD_FPC");
  G4LogicalVolume* lV_PADPlane    = new G4LogicalVolume( PAD_Plane, fmat_World, "lV_HPADPlane");
  G4LogicalVolume* lV_PADEnvelope = new G4LogicalVolume( PAD_Envelope, fmat_World, "lV_HPADEnvelope");
  G4LogicalVolume* lV_HC_Si_Box   = new G4LogicalVolume( HC_Si_Box, fmat_World, "lV_HC_Si_Box");

  lV_HCal_Absorber->SetVisAttributes(fvisPb);
  lV_PAD_Silicon->SetVisAttributes(fvisPAD);
  lV_PAD_Glue1->SetVisAttributes(fvisDM);
  lV_PAD_Glue2->SetVisAttributes(fvisDM);
  lV_PAD_FPC->SetVisAttributes(fvisDM);

  lV_PADPlane->SetVisAttributes(G4VisAttributes::Invisible);
  lV_PADEnvelope->SetVisAttributes(G4VisAttributes::Invisible);
  lV_HCal_Layer->SetVisAttributes(G4VisAttributes::Invisible);
  lV_HC_Si_Box->SetVisAttributes(G4VisAttributes::Invisible);

  int infoval = ZDCID::HCPadLayer + fLayer*100 + ZDCID::SI_PAD;
  std::pair<G4LogicalVolume*, int> pair_PAD= std::make_pair(lV_PAD_Silicon, infoval);
  m_ActiveLogicalVolumeInfoMap.insert(pair_PAD);
  infoval = ZDCID::HCPadLayer + fLayer*100 +ZDCID::Abs_Pb;
  std::pair<G4LogicalVolume*, int> pair_Pb = std::make_pair(lV_HCal_Absorber,infoval);
  infoval = ZDCID::HCPadLayer + fLayer*100 +ZDCID::Materials;
  std::pair<G4LogicalVolume*, int> pair_Glue1 = std::make_pair(lV_PAD_Glue1,infoval);
  std::pair<G4LogicalVolume*, int> pair_Glue2 = std::make_pair(lV_PAD_Glue2,infoval);
  std::pair<G4LogicalVolume*, int> pair_FPC = std::make_pair(lV_PAD_FPC,infoval);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_Pb);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_Glue1);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_Glue2);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_FPC);

  m_ActiveLogicalVolumesSet.insert(lV_PAD_Silicon);
  m_AbsorberLogicalVolumesSet.insert(lV_HCal_Absorber);
  m_AbsorberLogicalVolumesSet.insert(lV_PAD_Glue1);
  m_AbsorberLogicalVolumesSet.insert(lV_PAD_Glue2);
  m_AbsorberLogicalVolumesSet.insert(lV_PAD_FPC);

  //Construct a PADplane using Replica
  new G4PVReplica("PV_PADEnvelope", lV_PADEnvelope, lV_PADPlane, kXAxis, NpadX, PAD_X,0);
  new G4PVReplica("PV_PAD", lV_PAD_Silicon, lV_PADEnvelope, kYAxis, NpadY, PAD_Y,0);

  //Construct a Layer
  G4double posZ_HC_Absorber  = -0.5 * HCal_Si_Layer_Thickness + HCAL_Z_Absorber/2;
  G4double posZ_HC_Glue1     = -0.5 * HCal_Si_Layer_Thickness + HCAL_Z_Absorber+ PAD_Glue1_Z/2 ;
  G4double posZ_HC_Silicon   = -0.5 * HCal_Si_Layer_Thickness + HCAL_Z_Absorber + PAD_Glue1_Z + PAD_Z/2 ;
  G4double posZ_HC_Glue2     = -0.5 * HCal_Si_Layer_Thickness + HCAL_Z_Absorber + PAD_Glue1_Z + PAD_Z + PAD_Glue2_Z/2 ;
  G4double posZ_HC_FPC	     = -0.5 * HCal_Si_Layer_Thickness + HCAL_Z_Absorber + PAD_Glue1_Z + PAD_Z + PAD_Glue2_Z +PAD_FPC_Z/2 ;

  G4ThreeVector threeVect_HC_Absorber = G4ThreeVector(0, 0, posZ_HC_Absorber);
  G4ThreeVector threeVect_HC_Glue1   = G4ThreeVector(0, 0, posZ_HC_Glue1);  
  G4ThreeVector threeVect_HC_Silicon = G4ThreeVector(0, 0, posZ_HC_Silicon);
  G4ThreeVector threeVect_HC_Glue2   = G4ThreeVector(0, 0, posZ_HC_Glue2);
  G4ThreeVector threeVect_HC_FPC     = G4ThreeVector(0, 0, posZ_HC_FPC);

  new G4PVPlacement(0, threeVect_HC_Absorber, lV_HCal_Absorber,    "PV_PAD_W", lV_HCal_Layer,false,0);  
  new G4PVPlacement(0, threeVect_HC_Glue1,   lV_PAD_Glue1,"PV_PAD_Glue1", lV_HCal_Layer,false,0);  
  new G4PVPlacement(0, threeVect_HC_Silicon, lV_PADPlane, "PV_PAD_Plane", lV_HCal_Layer,false,0);  
  new G4PVPlacement(0, threeVect_HC_Glue2,   lV_PAD_Glue2,"PV_PAD_Glue2", lV_HCal_Layer,false,0);  
  new G4PVPlacement(0, threeVect_HC_FPC,     lV_PAD_FPC,  "PV_PAD_FPC", lV_HCal_Layer,false,0);  

  //construct a Box of layers
  new G4PVReplica("HC_Layers", lV_HCal_Layer, lV_HC_Si_Box,kZAxis, HCALSiNumberOfLayers, HCal_Si_Layer_Thickness,0);

  G4ThreeVector threeVect_HC_Si_Box = G4ThreeVector(Center_X, Center_Y, Start_Z + TotalLayerThickness *0.5);
  new G4PVPlacement(0, threeVect_HC_Si_Box, "PV_HC_Si_Box", lV_HC_Si_Box, motherPhy, false, 0);
  
  fLayer += HCALSiNumberOfLayers;

  _z_HCSilicon[1] = Start_Z + TotalLayerThickness;
  return _z_HCSilicon[1];
  
}

double myZDCStructure::ConstructHCSciLayers(double Start_X, double Start_Y, double Start_Z, 
					    double End_X, double End_Y, double End_Z,
					    G4VPhysicalVolume *motherPhy) {

  _z_HCSci[0] = Start_Z;

  double Center_X = (Start_X + End_X)/2.;
  double Center_Y = (Start_Y + End_Y)/2.;
  double Width_X = End_X - Start_X;
  double Width_Y = End_Y - Start_Y;
  double TotalTowerThickness = HCal_Layer_Thickness * NLayersHCALTower;
  
  //HCal tower
  G4Box* HCal_Absorber	= new G4Box("HCal_Absorber",	Width_X/2.0, Width_Y/2.0, HCAL_Z_Absorber/2.0);
  G4Box* HCal_Scintillator= new G4Box("HCal_Scintillator",HCAL_X_Tower/2.0, HCAL_Y_Tower/2.0, HCAL_Z_Scintillator/2.0);
  G4Box* HCal_Gap	= new G4Box("HCal_Gap",	        Width_X/2.0, Width_Y/2.0, HCAL_Z_Gap/2.0);
  G4Box* HCal_Layer     = new G4Box("HCal_Layer",       Width_X/2.0, Width_Y/2.0, HCal_Layer_Thickness/2.0);
  G4Box* HCal_SciPlane     = new G4Box("HCal_SciPlane",       Width_X/2.0, Width_Y/2.0, HCAL_Z_Scintillator/2.0);
  G4Box* HCal_SciEnvelope  = new G4Box("HCal_SciEnvelope",       HCAL_X_Tower/2.0, Width_Y/2.0, HCAL_Z_Scintillator/2.0);
  G4Box* HCal_Box     = new G4Box("HCal_Box",       Width_X/2.0, Width_Y/2.0, TotalTowerThickness/2.);

  //HCal volumes
  G4LogicalVolume* lV_HCal_Absorber	= new G4LogicalVolume( HCal_Absorber, 	 fmat_Pb, "lV_HCal_Absorber");
  G4LogicalVolume* lV_HCal_Scintillator   = new G4LogicalVolume( HCal_Scintillator, fmat_Sci,"lV_HCal_Scintillator");
  G4LogicalVolume* lV_HCal_Gap	= new G4LogicalVolume( HCal_Gap, 	  fmat_World, "lV_HCal_Gap");
  G4LogicalVolume* lV_HCal_Layer  = new G4LogicalVolume(HCal_Layer, fmat_World, "lV_HCal_Layer");
  G4LogicalVolume* lV_HCal_SciPlane  = new G4LogicalVolume(HCal_SciPlane, fmat_World, "lV_HCal_SciPlane");
  G4LogicalVolume* lV_HCal_SciEnvelope  = new G4LogicalVolume(HCal_SciEnvelope, fmat_World, "lV_HCal_SciEnvelope");
  G4LogicalVolume* lV_HCal_Box  = new G4LogicalVolume(HCal_Box, fmat_World, "lV_HCal_Box");

  lV_HCal_Absorber->SetVisAttributes(fvisPb);
  lV_HCal_Scintillator->SetVisAttributes(fvisSci);

  lV_HCal_Gap->SetVisAttributes(G4VisAttributes::Invisible);
  lV_HCal_Layer->SetVisAttributes(G4VisAttributes::Invisible);
  lV_HCal_SciPlane->SetVisAttributes(G4VisAttributes::Invisible);
  lV_HCal_SciEnvelope->SetVisAttributes(G4VisAttributes::Invisible);
  lV_HCal_Box->SetVisAttributes(G4VisAttributes::Invisible);

  int infoval = ZDCID::HCSciLayer +NLayersHCALTower*10000 + fLayer*100 + ZDCID::Scintillator;
  std::pair<G4LogicalVolume*, int> pair_Scint= std::make_pair(lV_HCal_Scintillator, infoval);
  m_ActiveLogicalVolumeInfoMap.insert(pair_Scint);
  infoval = ZDCID::HCSciLayer + NLayersHCALTower*10000 + fLayer*100 + ZDCID::Abs_Pb;
  std::pair<G4LogicalVolume*, int> pair_Abs = std::make_pair(lV_HCal_Absorber, infoval);
  m_AbsorberLogicalVolumeInfoMap.insert(pair_Abs);

  m_ActiveLogicalVolumesSet.insert(lV_HCal_Scintillator);
  m_AbsorberLogicalVolumesSet.insert(lV_HCal_Absorber);

  new G4PVReplica("PV_HCSciEnvelope",lV_HCal_SciEnvelope, lV_HCal_SciPlane, kXAxis, HCALNumberOfTowersX, HCAL_X_Tower, 0);
  new G4PVReplica("PV_HCScintillator",lV_HCal_Scintillator, lV_HCal_SciEnvelope, kYAxis, HCALNumberOfTowersY, HCAL_Y_Tower,0);

  G4double posZ_HCal_Absorber    = -0.5 * HCal_Layer_Thickness + HCAL_Z_Absorber/2 ;
  G4double posZ_HCal_Scintillator = -0.5 * HCal_Layer_Thickness + HCAL_Z_Absorber + HCAL_Z_Scintillator/2 ;
  G4double posZ_HCal_Gap         = -0.5 * HCal_Layer_Thickness + HCAL_Z_Absorber + HCAL_Z_Scintillator + HCAL_Z_Gap/2 ;

  G4ThreeVector threeVect_LogV_HCal_Absorber    = G4ThreeVector(0, 0, posZ_HCal_Absorber);
  G4ThreeVector threeVect_LogV_HCal_Scintillator = G4ThreeVector(0, 0, posZ_HCal_Scintillator);
  G4ThreeVector threeVect_LogV_HCal_Gap         = G4ThreeVector(0, 0, posZ_HCal_Gap);
  new G4PVPlacement(0, threeVect_LogV_HCal_Scintillator, lV_HCal_SciPlane,"PV_HCal_Sci", lV_HCal_Layer,false,0);  
  new G4PVPlacement(0, threeVect_LogV_HCal_Absorber,    lV_HCal_Absorber,"PV_HCal_Abs", lV_HCal_Layer,false,0);
  new G4PVPlacement(0, threeVect_LogV_HCal_Gap,         lV_HCal_Gap,     "PV_HCal_Gap", lV_HCal_Layer,false,0);
  
  new G4PVReplica("PV_HCal_Layers",lV_HCal_Layer, lV_HCal_Box, kZAxis, NLayersHCALTower, HCal_Layer_Thickness,0);

  double TotalThicknessCreated = 0;
  for(int ibox =0; ibox<HCALNumberOfTowersZ; ibox++){
    G4double posZ_HCal_Box = Start_Z + TotalThicknessCreated + TotalTowerThickness * 0.5;
    G4ThreeVector threeVect_LogV_HCal_Box  = G4ThreeVector(Center_X, Center_Y, posZ_HCal_Box);
    std::string ss_HCAL_Box = "PV_HCal_Box" + std::to_string(ibox);
    new G4PVPlacement(0, threeVect_LogV_HCal_Box, ss_HCAL_Box, lV_HCal_Box,motherPhy,false,ibox);
    
    TotalThicknessCreated += TotalTowerThickness;
    TotalThicknessCreated += HCAL_Tower_Gap;
  }

  fLayer += NLayersHCALTower;

  _z_HCSci[1] = Start_Z+TotalThicknessCreated;
  return _z_HCSci[1];
}

void myZDCStructure::Materials(){

  //*****************************************************************************************
  //Unique materials
  //*****************************************************************************************
  G4NistManager* material_Man = G4NistManager::Instance();  //NistManager: start element destruction
  
  fmat_World = material_Man->FindOrBuildMaterial("G4_AIR");

  //The definition of the W alloy
  fmat_W = new G4Material("tungsten",18.73 *g/cm3,3);
  //G4Material* material_tungsten = new G4Material("tungsten",19.3 *g/cm3,1);
  G4Element* W  = material_Man->FindOrBuildElement(74);//density: 19.3  I:727
  G4Element* Ni = material_Man->FindOrBuildElement(28);//density: 8.902   I:311
  G4Element* Cu = material_Man->FindOrBuildElement(29);//G4_Cu  8.96   I:322
  fmat_W->AddElement(W,94.3 *perCent);    //the percentage of materialal originally 100 --> 100./106
  fmat_W->AddElement(Ni,3.77 *perCent);   // 4. --> 4./106
  fmat_W->AddElement(Cu,1.89 *perCent);   //2. -->2./106.
  
  //Definition of the Epoxy Glue
  fmat_PET = new G4Material("PET",1.38*g/cm3,3);
  G4Element* O = material_Man->FindOrBuildElement(8);
  G4Element* elH = new G4Element("Hydrogen","H", 1, 1.00794 *g/mole);
  G4Element* elC = new G4Element("Carbon","C", 6, 12.011 *g/mole);
  fmat_PET->AddElement(elC,10);
  fmat_PET->AddElement(elH,8);
  fmat_PET->AddElement(O,4);
  
  //Definition of the scintillator
  G4double density= 1.032 *g/cm3;       //to define the dencity on my own
  fmat_Sci = new G4Material("Scintillator",density,2);   //
  fmat_Sci->AddElement(elC,8);
  fmat_Sci->AddElement(elH,8);
  
  //Other materials
  fmat_Si = material_Man->FindOrBuildMaterial("G4_Si");
  fmat_Pb = material_Man->FindOrBuildMaterial("G4_Pb");
  fmat_Cu = material_Man->FindOrBuildMaterial("G4_Cu");
  fmat_Fe = material_Man->FindOrBuildMaterial("G4_Fe");

  //Definition of LYSO
  G4Element* Lu = material_Man->FindOrBuildElement("Lu"); // new G4Element("Lutetium","Lu",71,174.97*g/mole);
  G4Element* Y  = material_Man->FindOrBuildElement("Y"); // new G4Element("Yttrium","Y",39,88.905*g/mole);
  G4Element* Si = material_Man->FindOrBuildElement("Si");
  G4Material* mat_LYSO = new G4Material("LYSO",7.4*g/cm3,4);
  G4double totLyso  = 1.8 + 0.2 + 1 + 5;
  mat_LYSO->AddElement(Lu,1.8/totLyso);
  mat_LYSO->AddElement(Y,0.2/totLyso);
  mat_LYSO->AddElement(Si,1./totLyso);
  mat_LYSO->AddElement(O,5./totLyso);

  G4Material* mat_LYSOCe = new G4Material("LYSOCe", 7.2*g/cm3, 2);
  G4Element* Ce = material_Man->FindOrBuildElement("Ce");
  mat_LYSOCe->AddMaterial(mat_LYSO,0.995);
  mat_LYSOCe->AddElement(Ce, 0.005);

  //PbWO4
  G4Material* mat_PbWO4 = material_Man->FindOrBuildMaterial("G4_PbWO4");
  
  if(_id_Crystal == _id_PbWO4) fmat_Crystal = mat_PbWO4;
  if(_id_Crystal == _id_LYSO)  fmat_Crystal = mat_LYSO;
  if(_id_Crystal == _id_LYSOCe) fmat_Crystal = mat_LYSOCe;

  return;
}

void myZDCStructure::Print(){

  std::cout<<"This is myZDCStructure"<<std::endl;

}

void myZDCStructure::SetColors(){

  fvisCrystal = new G4VisAttributes(G4Color(G4Colour::Yellow()));
  fvisPIX = new G4VisAttributes(G4Color(G4Colour::Magenta()));
  fvisPAD = new G4VisAttributes(G4Color(G4Colour::Cyan()));
  fvisDM = new G4VisAttributes(G4Color(G4Colour::Grey()));
  fvisW = new G4VisAttributes(G4Color(0.5,0.5,0.8,0.6));
  fvisPb = new G4VisAttributes(G4Color(.3,.3,.3,0.6));
  fvisSci = new G4VisAttributes(G4Color(G4Colour::Green()));

  fvisCrystal->SetForceSolid(true);
  fvisPIX->SetForceSolid(true);
  fvisPAD->SetForceSolid(true);
  fvisDM->SetForceSolid(true);
  fvisW->SetForceSolid(true);
  fvisPb->SetForceSolid(true);
  fvisSci->SetForceSolid(true);

  return;

}

void myZDCStructure::PrintTowerMap(const std::string &d){
  
  std::ofstream map_file;
  std::string filename = "ZDC_"+d+"_mapping.txt";
  map_file.open(filename,std::ios::out);
  
  map_file<<"# ZDC_"<<d<<std::endl;
  map_file<<"Gx0 "<<"\t"<<"-962.4"<<std::endl;
  map_file<<"Gy0 "<<"\t"<<"0."<<std::endl;
  map_file<<"Gz0 "<<"\t"<<"37000."<<std::endl;
  map_file<<"Grot_x "<<"\t"<<"0."<<std::endl;
  map_file<<"Grot_y "<<"\t"<<"-0.025"<<std::endl;
  map_file<<"Grot_z "<<"\t"<<"0."<<std::endl;

  double offsetZ=0;
  if(d=="Crystal"){

    offsetZ=0;

    map_file<<"#Tower ID <-> Layer ID"<<std::endl;
    for(int iL=0; iL<nCTowerZ; iL++)
      map_file<<"iT_iL "<<iL<<"\t"<<2*iL+1<<std::endl;
    
    map_file<<"#Tower "<<"\t"<<"iLayer "<<"\t"<<"iX "<<"\t"<<"iY "<<"\t"
	    <<"x_cnt " <<"\t"<<"y_cnt " <<"\t"<<"z_cnt "<<"\t"
	    <<"size_x "<<"\t"<<"size_y "<<"\t"<<"size_z"<<std::endl;
    
    for(int iL=0; iL < nCTowerZ; iL++){
      offsetZ += (PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap);
      double z_cnt = offsetZ + CTower_Z/2;

      for(int ix = 0; ix < nCTowerX; ix++){
	double x_cnt = (ix - nCTowerX/2) * CTower_X + CTower_X/2.;

	for(int iy = 0; iy < nCTowerY; iy++){
	  double y_cnt = (iy - nCTowerY/2) * CTower_Y + CTower_Y/2.;

	  map_file<<"Tower "<<iL<<"\t"<<ix<<"\t"<<iy<<"\t"
		  <<x_cnt<<"\t"<<y_cnt<<"\t"<<z_cnt<<"\t"
		  <<CTower_X<<"\t"<<CTower_Y<<"\t"<<CTower_Z<<std::endl;
	  
	}
      }
      
      offsetZ += CTower_Z;
      offsetZ += CTower_GAP;
    }
  }else if (d=="SiPixel"){
    
    offsetZ = 0;
    int nPixLayer = 1 + nCTowerZ + NumberPIX;

    map_file<<"#Tower ID <-> Layer ID"<<std::endl;
    for(int iL=0; iL<nPixLayer; iL++){
      if(iL<=nCTowerZ) 
	map_file<<"iT_iL "<<iL<<"\t"<<2*iL<<std::endl;
      else 
	map_file<<"iT_iL "<<iL<<"\t"
		<<nCTowerZ*2 + (iL-nCTowerZ) * (NPadOnlyLayers + 1)<<std::endl;
    }

    map_file<<"#Tower "<<"\t"<<"iLayer "<<"\t"<<"iX "<<"\t"<<"iY "<<"\t"
	    <<"x_cnt " <<"\t"<<"y_cnt " <<"\t"<<"z_cnt "<<"\t"
	    <<"size_x "<<"\t"<<"size_y "<<"\t"<<"size_z"<<std::endl;
    
    for(int iL=0; iL < nPixLayer; iL++){

      if(iL > nCTowerZ ) offsetZ += (PIX_Absorber_Z + PIX_Glue1_Z);

      double z_cnt = offsetZ + PIX_Z/2;

      offsetZ += (PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap);
      if(iL<nCTowerZ) 
	offsetZ += (CTower_Z + CTower_GAP);
      else 
	offsetZ += (PAD_Layer_Thickness * NPadOnlyLayers);

      for(int ix = 0; ix < NpixX; ix++){
	double x_cnt = (ix - NpixX/2) * PIX_X + PIX_X/2.;

	for(int iy = 0; iy < NpixY; iy++){
	  double y_cnt = (iy - NpixY/2) * PIX_Y + PIX_Y/2.;

	  map_file<<"Tower "<<iL<<"\t"<<ix<<"\t"<<iy<<"\t"
		  <<x_cnt<<"\t"<<y_cnt<<"\t"<<z_cnt<<"\t"
		  <<PIX_X<<"\t"<<PIX_Y<<"\t"<<PIX_Z<<std::endl;
	  
	}
      }
    }
    
  }else if(d=="SiPad"){
    
    offsetZ = _z_EMLayers[0] - _z_Crystal[0];
    int iTower=0;

    map_file<<"#Tower ID <-> Layer ID"<<std::endl;
    for(int iB=0; iB<NumberPAD/NPadOnlyLayers; iB++){
      for(int iL=0; iL<NPadOnlyLayers; iL++){
	map_file<<"iT_iL "<<iTower<<"\t"
		<<2*nCTowerZ + iB*(NPadOnlyLayers+1)+iL+1<<std::endl;
	iTower++;
      }
    }
    for(int iL=0; iL<HCALSiNumberOfLayers; iL++){
      map_file<<"iT_iL "<<iTower<<"\t"
	      <<2*nCTowerZ+1+NumberOfLayers+iL<<std::endl;
      iTower++;
    }


    map_file<<"#Tower "<<"\t"<<"iLayer "<<"\t"<<"iX "<<"\t"<<"iY "<<"\t"
	    <<"x_cnt " <<"\t"<<"y_cnt " <<"\t"<<"z_cnt "<<"\t"
	    <<"size_x "<<"\t"<<"size_y "<<"\t"<<"size_z"<<std::endl;
    
    offsetZ += (PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap);

    iTower =0;
    for(int iB=0; iB<NumberPAD/NPadOnlyLayers; iB++){
      for(int iL=0; iL<NPadOnlyLayers; iL++){
	
	offsetZ += (PAD_Absorber_Z + PAD_Glue1_Z);      
	double z_cnt = offsetZ + PAD_Z/2;
	offsetZ += (PAD_Z + PAD_Glue2_Z + PAD_FPC_Z + PAD_AirGap);
       
	for(int ix = 0; ix < NpadX; ix++){
	  double x_cnt = (ix - NpadX/2) * PAD_X + PAD_X/2.;

	  for(int iy = 0; iy < NpadY; iy++){
	    double y_cnt = (iy - NpadY/2) * PAD_Y + PAD_Y/2.;
	    
	    map_file<<"Tower "<<iTower<<"\t"<<ix<<"\t"<<iy<<"\t"
		    <<x_cnt<<"\t"<<y_cnt<<"\t"<<z_cnt<<"\t"
		    <<PAD_X<<"\t"<<PAD_Y<<"\t"<<PAD_Z<<std::endl;
	    
	  }
	}	
	iTower++;
	
      }
      
      offsetZ +=PIX_Layer_Thickness;
    }

    offsetZ = _z_HCSilicon[0] - _z_Crystal[0];
   
    for(int iL=0; iL<HCALSiNumberOfLayers; iL++){
      offsetZ += (HCAL_Z_Absorber + PAD_Glue1_Z);
      double z_cnt = offsetZ + PAD_Z/2;
      offsetZ += (PAD_Z + PAD_Glue2_Z + PAD_FPC_Z + PAD_AirGap);

      for(int ix = 0; ix < NpadX; ix++){
	double x_cnt = (ix - NpadX/2) * PAD_X + PAD_X/2.;
	
	for(int iy = 0; iy < NpadY; iy++){
	  double y_cnt = (iy - NpadY/2) * PAD_Y + PAD_Y/2.;
	  
	  map_file<<"Tower "<<iTower<<"\t"<<ix<<"\t"<<iy<<"\t"
		  <<x_cnt<<"\t"<<y_cnt<<"\t"<<z_cnt<<"\t"
		  <<PAD_X<<"\t"<<PAD_Y<<"\t"<<PAD_Z<<std::endl;
	  
	}
      }	
      iTower++;
      
    }
    
  }else if(d=="Sci"){

    offsetZ = _z_HCSci[0] - _z_Crystal[0];

    map_file<<"#Tower ID <-> Layer ID"<<std::endl;
    for(int iB=0; iB<HCALNumberOfTowersZ; iB++){
      map_file<<"iT_iL "<<iB<<"\t"
	      <<2*nCTowerZ + 1 + NumberOfLayers + HCALSiNumberOfLayers 
	+ iB * NLayersHCALTower<<std::endl;
    }

    map_file<<"#Tower "<<"\t"<<"iLayer "<<"\t"<<"iX "<<"\t"<<"iY "<<"\t"
	    <<"x_cnt " <<"\t"<<"y_cnt " <<"\t"<<"z_cnt "<<"\t"
	    <<"size_x "<<"\t"<<"size_y "<<"\t"<<"size_z"<<std::endl;

    double HCTower_Z = NLayersHCALTower * HCal_Layer_Thickness;
    std::cout<<"HC mapping: "<<offsetZ<<" "<<HCTower_Z<<std::endl;

    for(int iB= 0; iB< HCALNumberOfTowersZ; iB++){
      
      double z_cnt = offsetZ + HCTower_Z/2.;
      offsetZ += HCTower_Z;

      for(int ix = 0; ix < HCALNumberOfTowersX; ix++){
	double x_cnt = (ix - HCALNumberOfTowersX/2) * HCAL_X_Tower + HCAL_X_Tower/2.;

	for(int iy = 0; iy < HCALNumberOfTowersY; iy++){
	  double y_cnt = (iy - HCALNumberOfTowersY/2) * HCAL_Y_Tower + HCAL_Y_Tower/2.;
		  
	  map_file<<"Tower "<<iB<<"\t"<<ix<<"\t"<<iy<<"\t"
		  <<x_cnt<<"\t"<<y_cnt<<"\t"<<z_cnt<<"\t"
		  <<HCAL_X_Tower<<"\t"<<HCAL_Y_Tower<<"\t"<<HCTower_Z<<std::endl;
	  
	}
      }	
      
      offsetZ += HCAL_Tower_Gap;
    }
  }
    
  map_file.close();
  
}
