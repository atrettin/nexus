// ----------------------------------------------------------------------------
// nexus | Next100Vessel.h
//
// Vessel of the NEXT-100 geometry.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef NEXT100_VESSEL
#define NEXT100_VESSEL

#include "BaseGeometry.h"
#include "CylinderPointSampler.h"
#include "SpherePointSampler.h"

#include <G4Navigator.hh>


class G4GenericMessenger;
class G4VPhysicalVolume;

namespace nexus {

  class Next100Vessel: public BaseGeometry
  {
  public:
    /// Constructor
    Next100Vessel(const G4double nozzle_ext_diam,
		  const G4double up_nozzle_ypos,
		  const G4double central_nozzle_ypos,
		  const G4double down_nozzle_ypos,
		  const G4double bottom_nozzle_ypos);

    /// Destructor
    ~Next100Vessel();

    /// Generate a vertex within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;

    /// Returns the logical and physical volume of the inner object
    G4LogicalVolume* GetInternalLogicalVolume();
    G4VPhysicalVolume* GetInternalPhysicalVolume();

    /// Builder
    void Construct();



  private:
    // Dimensions
    G4double vessel_in_rad_, vessel_body_length_, vessel_length_, vessel_thickness_;
    G4double distance_gate_body_end_;
    G4double endcap_in_rad_, endcap_theta_, endcap_thickness_, endcap_in_z_width_;
    G4double flange_out_rad_, flange_length_, flange_z_pos_;
    G4double large_nozzle_length_, small_nozzle_length_;
    G4double sc_yield_, e_lifetime_;
    G4double pressure_, temperature_;

    // Visibility of the shielding
    G4bool visibility_;

    // Dimensions coming from outside
    G4double nozzle_ext_diam_, up_nozzle_ypos_, central_nozzle_ypos_;
    G4double down_nozzle_ypos_, bottom_nozzle_ypos_;


    // Internal logical and physical volumes
    G4LogicalVolume* internal_logic_vol_;
    G4VPhysicalVolume* internal_phys_vol_;

    // Vertex generators
    CylinderPointSampler* body_gen_;
    SpherePointSampler*   tracking_endcap_gen_;
    SpherePointSampler*   energy_endcap_gen_;
    CylinderPointSampler* tracking_flange_gen_;
    CylinderPointSampler* energy_flange_gen_;

    G4double perc_endcap_vol_;

    // Geometry Navigator
    G4Navigator* geom_navigator_;

    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

    // Type of gas being used
    G4String gas_;
    G4int helium_mass_num_;
    G4double xe_perc_;

  };

} // end namespace nexus

#endif
