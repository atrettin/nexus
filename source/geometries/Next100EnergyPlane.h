// ----------------------------------------------------------------------------
// nexus | Next100EnergyPlane.h
//
// Energy plane geometry of the NEXT-100 detector.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef NEXT100_ENERGY_PLANE
#define NEXT100_ENERGY_PLANE

#include <vector>
#include <G4Navigator.hh>
#include <G4RotationMatrix.hh>

#include "CylinderPointSampler2020.h"
#include "PmtR11410.h"

class G4Material;
class G4LogicalVolume;
class G4GenericMessenger;


namespace nexus {

  /// This is a class to place all the components of the energy plane

  class Next100EnergyPlane: public BaseGeometry
  {

  public:
    /// Constructor
    Next100EnergyPlane();

    /// Destructor
    ~Next100EnergyPlane();

    /// Sets the Logical Volume where Inner Elements will be placed
    void SetMotherLogicalVolume(G4LogicalVolume* mother_logic);

    /// Sets the z position of the surface of the sapphire windows
    void SetSapphireSurfaceZPos(G4double z);

    /// Generate a vertex within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;

    // Builder
    void Construct();


  private:
    void GeneratePositions();
    void PrintPMTPositions() const;

  private:

    // Mother Logical Volume of the whole Energy PLane
    G4LogicalVolume* mother_logic_;

    // Dimensions
    const G4int num_PMTs_;
    const G4double copper_plate_thickn_, copper_plate_diam_;
    const G4double gas_hole_diam_;
    // const G4double hole_up_posx_, hole_up_posy_;
    // const G4double hole_lat1_posx_, hole_lat1_posy_;
    // const G4double hole_lat2_posx_, hole_lat2_posy_;
    const G4double hut_int_diam_, hut_thickn_, hut_hole_length_;
    const G4double hut_length_long_, hut_length_medium_, hut_length_short_;
    const G4int last_hut_long_, last_hut_medium_;
    const G4double hole_diam_front_, hole_diam_rear_;
    const G4double hole_length_front_, hole_length_rear_;
    const G4double sapphire_window_thickn_, optical_pad_thickn_, tpb_thickn_;
    const G4double pmt_stand_out_;
    const G4double internal_pmt_base_diam_, internal_pmt_base_thickn_;

    // Visibility of the energy plane
    G4bool visibility_, verbosity_;

    // Geometry Navigator
    G4Navigator* geom_navigator_;

    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

    // PMT
    PmtR11410*  pmt_;

    G4double end_of_sapphire_posz_;
    G4double copper_plate_posz_;
    G4double vacuum_posz_;
    std::vector<G4ThreeVector> pmt_positions_;
    std::vector<G4ThreeVector> short_hut_pos_, medium_hut_pos_, long_hut_pos_;
    G4double pmt_zpos_;
    G4RotationMatrix* pmt_rot_;
    G4double rot_angle_;

    // Vertex generators
    CylinderPointSampler2020* copper_gen_;
    CylinderPointSampler2020* sapphire_window_gen_;
    CylinderPointSampler2020* optical_pad_gen_;
    CylinderPointSampler2020* internal_pmt_base_gen_;
    CylinderPointSampler2020* external_pmt_base_gen_;

  };

} //end namespace nexus
#endif
