#ifndef MYZDCSTRUCTURE_H
#define MYZDCSTRUCTURE_H

#include <Geant4/G4Material.hh>
#include <Geant4/G4NistManager.hh>

class G4LogicalVolume;
class G4VPhysicalVolume;

class myZDCStructure {
  public: 
  myZDCStructure();
  ~myZDCStructure();

  double ConstructCrystalTowers(double x0, double y0, double z0,
				double x1, double y1, double z1,
				G4VPhysicalVolume *mPhy);

  double ConstructEMLayers(double x0, double y0, double z0, 
			 double x1, double y1, double z1,
			 G4VPhysicalVolume *mPhy);
  double ConstructHCLayers(double x0, double y0, double z0, 
			   double x1, double y1, double z1,
			   G4VPhysicalVolume *mPhy);
  void Print();


private:

  void Materials();
  
  int fLayer;
  
  G4Material* fmat_World;
  G4Material* fmat_W;
  G4Material* fmat_PET;
  G4Material* fmat_Sci;
  G4Material* fmat_Si;
  G4Material* fmat_Pb;
  G4Material* fmat_Cu;
  G4Material* fmat_Fe;
  G4Material* fmat_Crystal;

};  

#endif
