//____________________________________________________________________________..
//
// This is the interface to the framework. You only need to define the parameters
// you use for your detector in the SetDefaultParameters() method here
// The place to do this is marked by //implement your own here//
// The parameters have no units, they need to be converted in the
// myZDCDetector::ConstructMe() method
// but the convention is as mentioned cm and deg
//____________________________________________________________________________..
//
#include "myZDCSubsystem.h"

#include "myZDCDetector.h"
#include "myZDCSteppingAction.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4SteppingAction.h> 

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNode.h> 
#include <phool/PHNodeIterator.h>
#include <phool/PHObject.h>
#include <phool/getClass.h>

using namespace std;

//_______________________________________________________________________
myZDCSubsystem::myZDCSubsystem(const std::string &name)
  : PHG4DetectorSubsystem(name)
  , m_Detector(nullptr)
  , m_SteppingAction(nullptr)
{
  // call base class method which will set up parameter infrastructure
  // and call our SetDefaultParameters() method
  InitializeParameters();
}
//_______________________________________________________________________
int myZDCSubsystem::InitRunSubsystem(PHCompositeNode *topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
  PHNodeIterator dstIter(dstNode);
  if (GetParams()->get_int_param("active"))
  {
    PHCompositeNode *DetNode = dynamic_cast<PHCompositeNode *>(dstIter.findFirst("PHCompositeNode", Name()));
    if (!DetNode)
    {
      DetNode = new PHCompositeNode(Name());
      dstNode->addNode(DetNode);
    }
    std::string g4hitnodename = "G4HIT_" + Name();
    PHG4HitContainer *g4_hits = findNode::getClass<PHG4HitContainer>(DetNode, g4hitnodename);
    if (!g4_hits)
    {
      g4_hits = new PHG4HitContainer(g4hitnodename);
      DetNode->addNode(new PHIODataNode<PHObject>(g4_hits, g4hitnodename, "PHObject"));
    }
  }
  // create detector
  m_Detector = new myZDCDetector(this, topNode, GetParams(), Name());
  m_Detector->OverlapCheck(CheckOverlap());
  // create stepping action if detector is active
  if (GetParams()->get_int_param("active"))
  {
    m_SteppingAction = new myZDCSteppingAction(m_Detector, GetParams());
  }
  m_start_time = clock();
  m_cnt = 0;
  return 0;
}
//_______________________________________________________________________
int myZDCSubsystem::process_event(PHCompositeNode *topNode)
{
  // pass top node to stepping action so that it gets
  // relevant nodes needed internally
  if (m_SteppingAction)
  {
    m_SteppingAction->SetInterfacePointers(topNode);
  }
  if(m_cnt%20==0) 
  {
    clock_t end_time = clock();
    std::cout<<"myZDCSubsystem:: "<<m_cnt<<" event processed.. : time = "<<(double)(end_time - m_start_time) / CLOCKS_PER_SEC<<" sec."<<std::endl;
  } 
  m_cnt++;

  return 0;
}
//_______________________________________________________________________
void myZDCSubsystem::Print(const string &what) const
{
  if (m_Detector)
  {
    m_Detector->Print(what);
  }
  return;
}

//_______________________________________________________________________
PHG4Detector *myZDCSubsystem::GetDetector(void) const
{
  return m_Detector;
}

//_______________________________________________________________________
void myZDCSubsystem::SetDefaultParameters()
{
  // sizes are in cm
  // angles are in rad
  // units should be converted to G4 units when used
  //implement your own here//
  set_default_double_param("place_x", 96);
  set_default_double_param("place_y", 0.);
  set_default_double_param("place_z", 3750.);
  set_default_double_param("rot_x", 0.);
  set_default_double_param("rot_y", 0.0256);
  set_default_double_param("rot_z", 0.);
  set_default_double_param("size_x", 60.);
  set_default_double_param("size_y", 60.);
  set_default_double_param("size_z", 300.);
  set_default_string_param("crystal","PbWO4");

}
