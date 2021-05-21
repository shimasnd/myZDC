#include "myZDCStructure.h"
#include "constants.h"

#include <Geant4/G4LogicalVolume.hh>
#include <Geant4/G4VPhysicalVolume.hh>
#include <Geant4/G4PVPlacement.hh>
#include <Geant4/G4Box.hh>
#include <Geant4/G4VisAttributes.hh>
#include <Geant4/G4Color.hh>

myZDCStructure::myZDCStructure() {
  std::cout<<"myZDCStructure:: Use FoCal Layout"<<std::endl;
  std::cout<<"myZDCStructure:: Pad Layer"<<PAD_Layer_Thickness<<"mm thick"<<std::endl;
  std::cout<<"myZDCStructure:: Pix Layer"<<PIX_Layer_Thickness<<"mm thick"<<std::endl;
  std::cout<<"myZDCStructure:: HCAL Layer"<<HCal_Layer_Thickness<<"mm thick"<<std::endl;
  Materials();
  fLayer=0;
  
}
myZDCStructure::~myZDCStructure() {}

double myZDCStructure::ConstructCrystalTowers(double Start_X, double Start_Y, double Start_Z, 
					      double End_X, double End_Y, double End_Z,
					      G4VPhysicalVolume *motherPhy) {

  double Center_X = (Start_X + End_X)/2.;
  double Center_Y = (Start_Y + End_Y)/2.;
  double Width_X = End_X - Start_X;
  double Width_Y = End_Y - Start_Y;

  G4Box* PIX_Glue1   = new G4Box("CPIX_Glue1",	Width_X/2.0, Width_Y/2.0, PIX_Glue1_Z/2.0);
  G4Box* PIX_Silicon = new G4Box("CPIX_Silicon",	PIX_X/2.0,   PIX_Y/2.0,   PIX_Z/2.0); 
  G4Box* PIX_Glue2   = new G4Box("CPIX_Glue2",	Width_X/2.0, Width_Y/2.0, PIX_Glue2_Z/2.0);
  G4Box* PIX_FPC     = new G4Box("CPIX_FPC",	Width_X/2.0, Width_Y/2.0, PIX_FPC_Z/2.0);
  G4Box *Crystal     = new G4Box("Crystal",     CTower_X*0.5 ,CTower_Y*0.5, CTower_Z*0.5);

  G4LogicalVolume* lV_PIX_Glue1   = new G4LogicalVolume( PIX_Glue1, 	fmat_PET, "LVol_CPIX_Glue1");
  G4LogicalVolume* lV_PIX_Silicon = new G4LogicalVolume( PIX_Silicon,     fmat_Si, "LVol_CPIX_Silicon" );
  G4LogicalVolume* lV_PIX_Glue2   = new G4LogicalVolume( PIX_Glue2, 	fmat_PET, "LVol_CPIX_Glue2");
  G4LogicalVolume* lV_PIX_FPC 	  = new G4LogicalVolume( PIX_FPC,  fmat_PET, "LVol_CPIX_FPC");
  G4LogicalVolume* lV_Crystal     = new G4LogicalVolume( Crystal, fmat_Crystal, "LVol_Crystal");

  G4VisAttributes *visC = new G4VisAttributes(G4Color(G4Colour::Yellow()));
  visC->SetForceSolid(true);
  lV_Crystal->SetVisAttributes(visC);
  
  G4VisAttributes *visPIX = new G4VisAttributes(G4Color(G4Colour::Magenta()));
  visPIX->SetForceSolid(true);
  lV_PIX_Silicon->SetVisAttributes(visPIX);

  G4VisAttributes *visDM = new G4VisAttributes(G4Color(G4Colour::Grey()));
  visDM->SetForceSolid(true);
  lV_PIX_Glue1->SetVisAttributes(visDM);
  lV_PIX_Glue2->SetVisAttributes(visDM);
  lV_PIX_FPC->SetVisAttributes(visDM);
  
  //*********************
  //Now crete nCTowerZ+1 PIX layers with nCTowerZ Tower layers
  //*******************

  double offsetZ=0;  
  int LayerID = fLayer;

  for(int ilayer=0; ilayer<nCTowerZ+1; ilayer++){
    
    G4double position_Z_PIX_Glue1   = Start_Z + offsetZ + PIX_Glue1_Z/2.; 
    if(ilayer>0) offsetZ += PIX_Glue1_Z;
    G4double position_Z_PIX_Silicon = Start_Z + offsetZ + PIX_Z/2.;
    G4double position_Z_PIX_Glue2   = Start_Z + offsetZ + PIX_Z + PIX_Glue2_Z/2.;
    G4double position_Z_PIX_FPC	    = Start_Z + offsetZ + PIX_Z + PIX_Glue2_Z + PIX_FPC_Z/2.;
    offsetZ += PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap;

    for(int iy = 0; iy < NpixY; iy++){
      for(int ix = 0; ix < NpixX; ix++){
	G4double position_X_PIX_Silicon = Start_X + ix*PIX_X + PIX_X/2;
	G4double position_Y_PIX_Silicon = Start_Y + iy*PIX_Y + PIX_Y/2;
	G4ThreeVector threeVect_PIX_Silicon   = G4ThreeVector(position_X_PIX_Silicon, position_Y_PIX_Silicon, position_Z_PIX_Silicon);
      
	G4int IDnumber_PIX = ix + NpadX*iy + NpadX*NpadY*ilayer;
	std::string ss_PIX = "PhysVol_CPIX_" + std::to_string(IDnumber_PIX) + "_L" + std::to_string(LayerID);
      
	new G4PVPlacement(0, threeVect_PIX_Silicon, ss_PIX, lV_PIX_Silicon, motherPhy, false, IDnumber_PIX);
      }
    }
  
    G4ThreeVector threeVect_PIX_Glue1 = G4ThreeVector(Center_X, Center_Y, position_Z_PIX_Glue1);
    G4ThreeVector threeVect_PIX_Glue2 = G4ThreeVector(Center_X, Center_Y, position_Z_PIX_Glue2);
    G4ThreeVector threeVect_PIX_FPC  = G4ThreeVector(Center_X, Center_Y, position_Z_PIX_FPC);
      
    G4int IDnumber_PIX_Glue1 	    = ilayer;
    G4int IDnumber_PIX_Glue2 	    = ilayer;
    G4int IDnumber_PIX_FPC 	    = ilayer;
    std::string ss_PIX_Glue1 = "PhysVol_CGlue1_"   + std::to_string(IDnumber_PIX_Glue1);
    std::string ss_PIX_Glue2 = "PhysVol_CGlue2_"   + std::to_string(IDnumber_PIX_Glue2);
    std::string ss_PIX_FPC  = "PhysVol_CFPC_"      + std::to_string(IDnumber_PIX_FPC);
      
    if(ilayer>0) new G4PVPlacement(0, threeVect_PIX_Glue1, ss_PIX_Glue1, 	lV_PIX_Glue1,   motherPhy, false, IDnumber_PIX_Glue1);
    new G4PVPlacement(0, threeVect_PIX_Glue2, ss_PIX_Glue2, 	lV_PIX_Glue2,   motherPhy, false, IDnumber_PIX_Glue2);
    new G4PVPlacement(0, threeVect_PIX_FPC,   ss_PIX_FPC, 	lV_PIX_FPC,     motherPhy, false, IDnumber_PIX_FPC);

    LayerID  +=2; 
    offsetZ +=CTower_Z;
  }

  offsetZ = PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap;
  LayerID = fLayer+1;

  for (int ilayer =0; ilayer<nCTowerZ; ilayer++){
    G4double position_Z_Crystal = Start_Z + offsetZ + CTower_Z/2.;

    for(int iX=0; iX<nCTowerX; iX++){
      for(int iY=0; iY<nCTowerY; iY++){
	G4double position_X_Crystal = Start_X + (iX+0.5)*CTower_X;
	G4double position_Y_Crystal = Start_Y + (iY+0.5)*CTower_Y;
  
	G4ThreeVector threeVect_Crystal  = G4ThreeVector(position_X_Crystal, position_Y_Crystal, position_Z_Crystal);
	G4int IDnumber_Crystal = iX+ iY*nCTowerX + ilayer * nCTowerX*nCTowerY;
	std::string ss_Crystal = "PhysVol_Crystal_" + std::to_string(IDnumber_Crystal) +"_L"+std::to_string(LayerID);

	new G4PVPlacement(0, threeVect_Crystal, ss_Crystal, lV_Crystal, motherPhy, false, IDnumber_Crystal);
       
      }
    }
    LayerID += 2;
    offsetZ += CTower_Z + PIX_Glue1_Z + PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap;
  }
  
  fLayer += 2*nCTowerZ +1;

  return Start_Z + (CTower_Z + PIX_Glue1_Z) * nCTowerZ + (PIX_Z + PIX_Glue2_Z + PIX_FPC_Z + PIX_AirGap) * (nCTowerZ +1);

}

double myZDCStructure::ConstructEMLayers(double Start_X, double Start_Y, double Start_Z, 
			     double End_X, double End_Y, double End_Z,
			     G4VPhysicalVolume *motherPhy) {

  double Center_X = (Start_X + End_X)/2.;
  double Center_Y = (Start_Y + End_Y)/2.;
  double Width_X = End_X - Start_X;
  double Width_Y = End_Y - Start_Y;

  //*****************************************************************************************
  //G4box is the material
  //Ignoring the width coming from the Guard Ring Thickness. 
  //PAD layer
  G4Box* PAD_W 		= new G4Box("PPAD_W",		Width_X/2.0, Width_Y/2.0, PAD_Absorber_Z/2.0);
  G4Box* PAD_Glue1 	= new G4Box("PPAD_Glue1",	Width_X/2.0, Width_Y/2.0, PAD_Glue1_Z/2.0);
  G4Box* PAD_Silicon 	= new G4Box("PPAD_Silicon",	PAD_X/2.0, 	     PAD_Y/2.0, 	  PAD_Z/2.0); 
  G4Box* PAD_Glue2 	= new G4Box("PPAD_Glue2",	Width_X/2.0, Width_Y/2.0, PAD_Glue2_Z/2.0);
  G4Box* PAD_FPC 	= new G4Box("PPAD_FPC",		Width_X/2.0, Width_Y/2.0, PAD_FPC_Z/2.0);
  //PIX layer
  G4Box* PIX_W 		= new G4Box("PPIX_W",		Width_X/2.0, Width_Y/2.0, PIX_Absorber_Z/2.0);
  G4Box* PIX_Glue1 	= new G4Box("PPIX_Glue1",	Width_X/2.0, Width_Y/2.0, PIX_Glue1_Z/2.0);
  G4Box* PIX_Silicon 	= new G4Box("PPIX_Silicon",	PIX_X/2.0, 	     PIX_Y/2.0, 	  PIX_Z/2.0); 
  G4Box* PIX_Glue2 	= new G4Box("PPIX_Glue2",	Width_X/2.0, Width_Y/2.0, PIX_Glue2_Z/2.0);
  G4Box* PIX_FPC 	= new G4Box("PPIX_FPC",		Width_X/2.0, Width_Y/2.0, PIX_FPC_Z/2.0);

  //*****************************************************************************************
  //   Logical volumes
  //*****************************************************************************************
  //PAD
  G4LogicalVolume* logVol_PAD_W 	= new G4LogicalVolume( PAD_W, 		fmat_W, "LogVol_PAD_W");
  G4LogicalVolume* logVol_PAD_Glue1 	= new G4LogicalVolume( PAD_Glue1, 	fmat_PET, "LogVol_PAD_Glue1" );
  G4LogicalVolume* logVol_PAD_Silicon	= new G4LogicalVolume( PAD_Silicon,     fmat_Si, "LogVol_PAD_Silicon" );
  G4LogicalVolume* logVol_PAD_Glue2 	= new G4LogicalVolume( PAD_Glue2, 	fmat_PET, "LogVol_PAD_Glue2");
  G4LogicalVolume* logVol_PAD_FPC 	= new G4LogicalVolume( PAD_FPC,  fmat_PET, "LogVol_PAD_FPC");
 
  //PIX
  G4LogicalVolume* logVol_PIX_W 	= new G4LogicalVolume( PIX_W, 	    fmat_W, "LogVol_PIX_W");
  G4LogicalVolume* logVol_PIX_Glue1 	= new G4LogicalVolume( PIX_Glue1,   fmat_PET, "LogVol_PIX_Glue1");
  G4LogicalVolume* logVol_PIX_Silicon	= new G4LogicalVolume( PIX_Silicon, fmat_Si, "LogVol_PIX_Silicon" );
  G4LogicalVolume* logVol_PIX_Glue2 	= new G4LogicalVolume( PIX_Glue2,   fmat_PET, "LogVol_PIX_Glue2" );
  G4LogicalVolume* logVol_PIX_FPC 	= new G4LogicalVolume( PIX_FPC,  fmat_PET, "LogVol_PIX_FPC");

  G4VisAttributes *visW = new G4VisAttributes(G4Color(0.5,0.5,0.8,0.6));
  visW->SetForceSolid(true);
  logVol_PAD_W->SetVisAttributes(visW);
  logVol_PIX_W->SetVisAttributes(visW);
		  
  G4VisAttributes *visPAD = new G4VisAttributes(G4Color(G4Colour::Cyan()));
  visPAD->SetForceSolid(true);
  logVol_PAD_Silicon->SetVisAttributes(visPAD);

  G4VisAttributes *visPIX = new G4VisAttributes(G4Color(G4Colour::Magenta()));
  visPIX->SetForceSolid(true);
  logVol_PIX_Silicon->SetVisAttributes(visPIX);

  G4VisAttributes *visDM = new G4VisAttributes(G4Color(G4Colour::Grey()));
  visDM->SetForceSolid(true);
  logVol_PAD_Glue1->SetVisAttributes(visDM);
  logVol_PAD_Glue2->SetVisAttributes(visDM);
  logVol_PAD_FPC->SetVisAttributes(visDM);
  logVol_PIX_Glue1->SetVisAttributes(visDM);
  logVol_PIX_Glue2->SetVisAttributes(visDM);
  logVol_PIX_FPC->SetVisAttributes(visDM);

  //*****************************************************************************************
  G4int IPADlayer = 0;
  G4int IPIXlayer = 0;
  //The PAD and PIX layer thicknesses can be different - this variable remembers the previous layer thickness either PIX or PAD
  //It is changed at the end of the loops
  G4double TotalLayerThickness = 0;

  for(G4int ilayer=0; ilayer<NumberOfLayers; ilayer++){
    if(TotalLayerThickness > End_Z) break;
    //This is the PAD layer
    if(ilayer%2 == 0 || IPIXlayer == NumberPIX){
      G4double position_Z_PAD_Absorber	= Start_Z + PAD_Absorber_Z/2 + TotalLayerThickness;
      G4double position_Z_PAD_Glue1	= Start_Z + PAD_Absorber_Z + PAD_Glue1_Z/2 + TotalLayerThickness;
      G4double position_Z_PAD_Silicon	= Start_Z + PAD_Absorber_Z + PAD_Glue1_Z + PAD_Z/2 + TotalLayerThickness;
      G4double position_Z_PAD_Glue2	= Start_Z + PAD_Absorber_Z + PAD_Glue1_Z + PAD_Z + PAD_Glue2_Z/2 + TotalLayerThickness;
      G4double position_Z_PAD_FPC	= Start_Z + PAD_Absorber_Z + PAD_Glue1_Z + PAD_Z + PAD_Glue2_Z + PAD_FPC_Z/2 + TotalLayerThickness;

      //      G4cout << ilayer << " PAD " << position_Z_PAD_Silicon << "  " << TotalLayerThickness << G4endl;
      for(int iy = 0; iy < NpadY; iy++){
	for(int ix = 0; ix < NpadX; ix++){
	  G4double position_X_PAD_Silicon = Start_X + ix*PAD_X + PAD_X/2;
	  G4double position_Y_PAD_Silicon = Start_Y + iy*PAD_Y + PAD_Y/2;
	  G4ThreeVector    threeVect_PAD_Silicon   = G4ThreeVector(position_X_PAD_Silicon, position_Y_PAD_Silicon, position_Z_PAD_Silicon);
	  
	  G4int IDnumber_PAD = ix + NpadX*iy + NpadX*NpadY*IPADlayer;
	  std::string ss_PAD = "PhysVol_PAD_" + std::to_string(IDnumber_PAD) + "_L" + std::to_string(ilayer);

	  new G4PVPlacement(0, threeVect_PAD_Silicon, ss_PAD, logVol_PAD_Silicon, motherPhy, false, IDnumber_PAD);
	}
      }
      
      G4ThreeVector threeVect_PAD_W 	= G4ThreeVector(Center_X, Center_Y, position_Z_PAD_Absorber);
      G4ThreeVector threeVect_PAD_Glue1 = G4ThreeVector(Center_X, Center_Y, position_Z_PAD_Glue1);
      G4ThreeVector threeVect_PAD_Glue2 = G4ThreeVector(Center_X, Center_Y, position_Z_PAD_Glue2);
      G4ThreeVector threeVect_PAD_FPC 	= G4ThreeVector(Center_X, Center_Y, position_Z_PAD_FPC);
            
      G4int IDnumber_PAD_Absorber 	= ilayer;
      G4int IDnumber_PAD_Glue1 		= ilayer;
      G4int IDnumber_PAD_Glue2 		= ilayer;
      G4int IDnumber_PAD_FPC 		= ilayer;
      
      std::string ss_PAD_Absorber = "PhysVol_Absorber_" + std::to_string(IDnumber_PAD_Absorber);
      std::string ss_PAD_Glue1    = "PhysVol_Glue1_"    + std::to_string(IDnumber_PAD_Glue1);
      std::string ss_PAD_Glue2    = "PhysVol_Glue2_"    + std::to_string(IDnumber_PAD_Glue2);
      std::string ss_PAD_FPC      = "PhysVol_FPC"       + std::to_string(IDnumber_PAD_FPC);

      new G4PVPlacement(0, threeVect_PAD_W,    ss_PAD_Absorber, logVol_PAD_W,       motherPhy, false, IDnumber_PAD_Absorber);
      new G4PVPlacement(0, threeVect_PAD_Glue1,ss_PAD_Glue1,    logVol_PAD_Glue1,   motherPhy, false, IDnumber_PAD_Glue1);
      new G4PVPlacement(0, threeVect_PAD_Glue2,ss_PAD_Glue2,    logVol_PAD_Glue2,   motherPhy, false, IDnumber_PAD_Glue2);
      new G4PVPlacement(0, threeVect_PAD_FPC,  ss_PAD_FPC, 	logVol_PAD_FPC,     motherPhy, false, IDnumber_PAD_FPC);
      IPADlayer++;
      //Which layer thickness
      TotalLayerThickness += PAD_Layer_Thickness;
    }
    else if(ilayer%2 == 1 && IPIXlayer < NumberPIX){
      G4double position_Z_PIX_Absorber	= Start_Z + PIX_Absorber_Z/2 + TotalLayerThickness;
      G4double position_Z_PIX_Glue1	= Start_Z + PIX_Absorber_Z + PIX_Glue1_Z/2 + TotalLayerThickness;
      G4double position_Z_PIX_Silicon	= Start_Z + PIX_Absorber_Z + PIX_Glue1_Z + PIX_Z/2 + TotalLayerThickness;
      G4double position_Z_PIX_Glue2	= Start_Z + PIX_Absorber_Z + PIX_Glue1_Z + PIX_Z + PIX_Glue2_Z/2 + TotalLayerThickness;
      G4double position_Z_PIX_FPC	= Start_Z + PIX_Absorber_Z + PIX_Glue1_Z + PIX_Z + PIX_Glue2_Z + PIX_FPC_Z/2 + TotalLayerThickness;
      //      G4cout << ilayer << " PIX " << position_Z_PIX_Silicon << "  " << TotalLayerThickness << G4endl;
      for(int iy = 0; iy < NpixY; iy++){
	for(int ix = 0; ix < NpixX; ix++){
	  G4double position_X_PIX_Silicon = Start_X + ix*PIX_X + PIX_X/2;
	  G4double position_Y_PIX_Silicon = Start_Y + iy*PIX_Y + PIX_Y/2;
	  G4ThreeVector    threeVect_PIX_Silicon = G4ThreeVector(position_X_PIX_Silicon, position_Y_PIX_Silicon, position_Z_PIX_Silicon);
	  
	  G4int IDnumber_PIX = ix + NpixX*iy + NpixX*NpixY*IPIXlayer;
	  //G4cout << "PIX " << IDnumber_PIX << "  " << ix << "  " << iy << "  " << IPIXlayer << G4endl;
	  std::string ss_PIX = "PhysVol_PIX_" + std::to_string(IDnumber_PIX) + "_L" + std::to_string(ilayer);

	  new G4PVPlacement(0, threeVect_PIX_Silicon, ss_PIX, logVol_PIX_Silicon,  motherPhy, false, IDnumber_PIX);
	}
      }
      
      G4ThreeVector threeVect_PIX_W     = G4ThreeVector(Center_X, Center_Y, position_Z_PIX_Absorber);
      G4ThreeVector threeVect_PIX_Glue1 = G4ThreeVector(Center_X, Center_Y, position_Z_PIX_Glue1);
      G4ThreeVector threeVect_PIX_Glue2 = G4ThreeVector(Center_X, Center_Y, position_Z_PIX_Glue2);
      G4ThreeVector threeVect_PIX_FPC 	= G4ThreeVector(Center_X, Center_Y, position_Z_PIX_FPC);
      
      G4int IDnumber_PIX_Absorber 	= ilayer;
      G4int IDnumber_PIX_Glue1 		= ilayer;
      G4int IDnumber_PIX_Glue2 		= ilayer;
      G4int IDnumber_PIX_FPC 		= ilayer;
      std::string ss_PIX_Absorber = "PhysVol_Absorber_" + std::to_string(IDnumber_PIX_Absorber);
      std::string ss_PIX_Glue1    = "PhysVol_Glue1_"    + std::to_string(IDnumber_PIX_Glue1);
      std::string ss_PIX_Glue2    = "PhysVol_Glue2_"    + std::to_string(IDnumber_PIX_Glue2);
      std::string ss_PIX_FPC = "PhysVol_FPC"       + std::to_string(IDnumber_PIX_FPC);
      
      new G4PVPlacement(0, threeVect_PIX_W,     ss_PIX_Absorber,logVol_PIX_W,  	     motherPhy, false, IDnumber_PIX_Absorber);
      new G4PVPlacement(0, threeVect_PIX_Glue1, ss_PIX_Glue1, 	logVol_PIX_Glue1,    motherPhy, false, IDnumber_PIX_Glue1);
      new G4PVPlacement(0, threeVect_PIX_Glue2, ss_PIX_Glue2, 	logVol_PIX_Glue2,    motherPhy, false, IDnumber_PIX_Glue2);
      new G4PVPlacement(0, threeVect_PIX_FPC, 	ss_PIX_FPC, 	logVol_PIX_FPC,     motherPhy, false, IDnumber_PIX_FPC);
      IPIXlayer++;
      //Which layer thickness
      TotalLayerThickness += PIX_Layer_Thickness;
    }
  }
    
  return Start_Z+TotalLayerThickness;
 
}

double myZDCStructure::ConstructHCLayers(double Start_X, double Start_Y, double Start_Z, 
			     double End_X, double End_Y, double End_Z,
			     G4VPhysicalVolume *motherPhy) {

  double Center_X = (Start_X + End_X)/2.;
  double Center_Y = (Start_Y + End_Y)/2.;
  double Width_X = End_X - Start_X;
  double Width_Y = End_Y - Start_Y;

  //HCal tower
  G4Box* HCal_Absorber	= new G4Box("HCal_Absorber",	Width_X/2.0, Width_Y/2.0, HCAL_Z_Absorber/2.0);
  G4Box* HCal_Scintilator= new G4Box("HCal_Scintilator",HCAL_X_Tower/2.0, HCAL_Y_Tower/2.0, HCAL_Z_Scintilator/2.0);
  G4Box* HCal_Gap	= new G4Box("HCal_Gap",	        Width_X/2.0, Width_Y/2.0, HCAL_Z_Gap/2.0);

  //HCal volumes
  G4LogicalVolume* logVol_HCal_Absorber	= new G4LogicalVolume( HCal_Absorber, 	 fmat_Pb, "logVol_HCal_Absorber");
  G4LogicalVolume* logVol_HCal_Scintilator   = new G4LogicalVolume( HCal_Scintilator, fmat_Sci,"logVol_HCal_Scintilator");
  G4LogicalVolume* logVol_HCal_Gap	= new G4LogicalVolume( HCal_Gap, 	  fmat_World, "logVol_HCal_Gap");
  
  G4VisAttributes *visPb = new G4VisAttributes(G4Color(.3,.3,.3,0.6));
  visPb->SetForceSolid(true);
  logVol_HCal_Absorber->SetVisAttributes(visPb);
  G4VisAttributes *visSci = new G4VisAttributes(G4Color(G4Colour::Green()));
  visSci->SetForceSolid(true);
  logVol_HCal_Scintilator->SetVisAttributes(visSci);

  logVol_HCal_Gap->SetVisAttributes(G4VisAttributes::Invisible);

  double TotalLayerThickness = 0;

  for(G4int iLayer = 0; iLayer < HCALNumberOfLayers; iLayer++){

    if(TotalLayerThickness > End_Z) break;
    
    G4double position_Z_HCal_Absorber    = Start_Z + HCAL_Z_Absorber/2 + TotalLayerThickness;
    G4double position_Z_HCal_Scintilator = Start_Z + HCAL_Z_Absorber + HCAL_Z_Scintilator/2 + TotalLayerThickness;
    G4double position_Z_HCal_Gap         = Start_Z + HCAL_Z_Absorber + HCAL_Z_Scintilator + HCAL_Z_Gap/2 + TotalLayerThickness;
    for(G4int iX = 0; iX < HCALNumberOfTowersX; iX++){
      for(G4int iY = 0; iY < HCALNumberOfTowersY; iY++){
	
	G4double position_X_Tower = Start_X  + iX*HCAL_X_Tower + HCAL_X_Tower/2;
	G4double position_Y_Tower = Start_Y  + iY*HCAL_Y_Tower + HCAL_Y_Tower/2;
                
	G4ThreeVector threeVect_LogV_HCal_Scintilator   = G4ThreeVector(position_X_Tower, position_Y_Tower, position_Z_HCal_Scintilator);
	G4int IDnumber_HCal_Scintilator = iX + iY * HCALNumberOfTowersX + iLayer * HCALNumberOfTowersX * HCALNumberOfTowersY;
	std::string ss_Sci = "PhysVol_HCal_Sci_" + std::to_string(IDnumber_HCal_Scintilator) + "_L" + std::to_string(iLayer);

	new G4PVPlacement(0,threeVect_LogV_HCal_Scintilator, ss_Sci, logVol_HCal_Scintilator,motherPhy,false, IDnumber_HCal_Scintilator);
      }
    }

    G4ThreeVector threeVect_LogV_HCal_Absorber= G4ThreeVector(Center_X, Center_Y, position_Z_HCal_Absorber);
    G4ThreeVector threeVect_LogV_HCal_Gap     = G4ThreeVector(Center_X, Center_Y, position_Z_HCal_Gap);
    G4int IDnumber_HCal_Absorber    = iLayer;
    G4int IDnumber_HCal_Gap         = iLayer;
    std::string ss_Abs = "PhysVol_HCal_Abs" + std::to_string(iLayer);
    std::string ss_Gap = "PhysVol_HCal_Gap" + std::to_string(iLayer);
    new G4PVPlacement(0, threeVect_LogV_HCal_Absorber, ss_Abs, logVol_HCal_Absorber, motherPhy,false, IDnumber_HCal_Absorber);
    new G4PVPlacement(0, threeVect_LogV_HCal_Gap, ss_Gap, logVol_HCal_Gap, motherPhy,false, IDnumber_HCal_Gap);
    
    TotalLayerThickness += HCal_Layer_Thickness;
  }
  
  return Start_Z+TotalLayerThickness;
}

void myZDCStructure::Materials(){

  //*****************************************************************************************
  //Unique materials
  //*****************************************************************************************
  G4NistManager* material_Man = G4NistManager::Instance();  //NistManager: start element destruction
  
  fmat_World = material_Man->FindOrBuildMaterial("G4_AIR");

  fmat_Crystal = material_Man->FindOrBuildMaterial("G4_PbWO4");

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
  fmat_Sci = new G4Material("Scintilator",density,2);   //
  fmat_Sci->AddElement(elC,8);
  fmat_Sci->AddElement(elH,8);
  
  //Other materials
  fmat_Si = material_Man->FindOrBuildMaterial("G4_Si");
  fmat_Pb = material_Man->FindOrBuildMaterial("G4_Pb");
  fmat_Cu = material_Man->FindOrBuildMaterial("G4_Cu");
  fmat_Fe = material_Man->FindOrBuildMaterial("G4_Fe");
  return;
}

void myZDCStructure::Print(){

  std::cout<<"This is myZDCStructure"<<std::endl;

}
