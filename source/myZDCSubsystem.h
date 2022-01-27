// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef MYZDCSUBSYSTEM_H
#define MYZDCSUBSYSTEM_H

#include <g4detectors/PHG4DetectorSubsystem.h>
#include <time.h>

class PHCompositeNode;
class PHG4Detector;
class myZDCDetector;
class PHG4SteppingAction;

/**
   * \brief Detector Subsystem module
   *
   * The detector is constructed and registered via myZDCDetector
   *
   *
   * \see myZDCDetector
   * \see myZDCSubsystem
   *
   */
class myZDCSubsystem : public PHG4DetectorSubsystem
{
 public:
  //! constructor
  myZDCSubsystem(const std::string& name = "myZDC");

  //! destructor
  virtual ~myZDCSubsystem() {}

  /*!
  creates relevant hit nodes that will be populated by the stepping action and stored in the output DST
  */
  int InitRunSubsystem(PHCompositeNode*) override;

  //! event processing
  /*!
  get all relevant nodes from top nodes (namely hit list)
  and pass that to the stepping action
  */
  int process_event(PHCompositeNode*) override;

  //! accessors (reimplemented)
  PHG4Detector* GetDetector() const override;

  PHG4SteppingAction* GetSteppingAction() const override { return m_SteppingAction; }
  //! Print info (from SubsysReco)
  void Print(const std::string& what = "ALL") const override;

 protected:
  // \brief Set default parameter values
  void SetDefaultParameters() override;

 private:
  //! detector construction
  /*! derives from PHG4Detector */
  myZDCDetector  *m_Detector;
  clock_t m_start_time;
  int m_cnt;

  //! particle tracking "stepping" action
  /*! derives from PHG4SteppingActions */
  PHG4SteppingAction *m_SteppingAction;
};

#endif // MYZDCSUBSYSTEM_H
