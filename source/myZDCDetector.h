// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef MYZDCDETECTOR_H
#define MYZDCDETECTOR_H

#include <g4main/PHG4Detector.h>

#include <map>
#include <set>
#include <string>  // for string

class G4LogicalVolume;
class G4VPhysicalVolume;
class PHCompositeNode;
class PHG4Subsystem;
class PHParameters;

class myZDCDetector : public PHG4Detector
{
 public:
  //! constructor
  myZDCDetector(PHG4Subsystem *subsys, PHCompositeNode *Node, PHParameters *parameters, const std::string &dnam);

  //! destructor
  virtual ~myZDCDetector() {}

  //! construct
  void ConstructMe(G4LogicalVolume *world) override;

  void Print(const std::string &what = "ALL") const override;

  //!@name volume accessors
  //@{
  int IsInDetector(G4VPhysicalVolume *) const;
  //@}
  int GetActiveVolumeInfo(G4VPhysicalVolume *volume);
  int GetAbsorberVolumeInfo(G4VPhysicalVolume *volume);

  void SuperDetector(const std::string &name) { m_SuperDetector = name; }
  const std::string SuperDetector() const { return m_SuperDetector; }

 private:
  PHParameters *m_Params;

  std::set<G4LogicalVolume *> m_ActiveLogicalVolumesSet;
  std::set<G4LogicalVolume *> m_AbsorberLogicalVolumesSet;

  // active volumes
  //  std::set<G4VPhysicalVolume *> m_PhysicalVolumesSet;
  std::map<G4LogicalVolume*, int> m_ActiveLogicalVolumeInfoMap;
  std::map<G4LogicalVolume*, int> m_AbsorberLogicalVolumeInfoMap;


  std::string m_SuperDetector;
};

#endif // MYZDCDETECTOR_H
