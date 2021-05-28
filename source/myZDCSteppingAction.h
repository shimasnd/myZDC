// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef MYZDCSTEPPINGACTION_H
#define MYZDCSTEPPINGACTION_H

#include <g4main/PHG4SteppingAction.h>

class myZDCDetector;

class G4Step;
class G4VPhysicalVolume;
class PHCompositeNode;
class PHG4Hit;
class PHG4HitContainer;
class PHParameters;

class myZDCSteppingAction : public PHG4SteppingAction
{
 public:
  //! constructor
  myZDCSteppingAction(myZDCDetector*, const PHParameters* parameters);

  //! destructor
  virtual ~myZDCSteppingAction();

  //! stepping action
  virtual bool UserSteppingAction(const G4Step*, bool);

  //! reimplemented from base class
  virtual void SetInterfacePointers(PHCompositeNode*);

 private:
  //! pointer to the detector
  myZDCDetector* m_Detector;
  const PHParameters* m_Params;
  //! pointer to hit container
  PHG4HitContainer* m_HitContainer;
  PHG4Hit* m_Hit;
  PHG4HitContainer* m_SaveHitContainer;
  G4VPhysicalVolume* m_SaveVolPre;
  G4VPhysicalVolume* m_SaveVolPost;

  int m_SaveTrackId;
  int m_SavePreStepStatus;
  int m_SavePostStepStatus;
  int m_ActiveFlag;
  int m_BlackHoleFlag;
  double m_EdepSum;
  double m_EionSum;

 
};

#endif // MYZDCSTEPPINGACTION_H
