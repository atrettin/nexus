// ----------------------------------------------------------------------------
//  $Id$
//
//  Authors: <miquel.nebot@ific.uv.es>
//  Created: 18 Sept 2013
//  
//  Copyright (c) 2013 NEXT Collaboration
// ---------------------------------------------------------------------------- 

#include "NextNewInnerElements.h"
#include "IonizationSD.h"
#include "PmtSD.h"
#include "UniformElectricDriftField.h"
#include "OpticalMaterialProperties.h"
#include "MaterialsList.h"
#include <G4GenericMessenger.hh>

#include <G4LogicalVolume.hh>
#include <G4Tubs.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>
#include <G4VisAttributes.hh>
#include <G4Region.hh>
#include <G4UserLimits.hh>
#include <G4SDManager.hh>

namespace nexus {

  NextNewInnerElements::NextNewInnerElements():
    _active_diam   (47.8 * cm),//From NextNewFieldCage: 2*_tube_in_rad-_reflector_thickness
    _active_length (58. * cm),//From NextNewFieldCage:_tube_length=Drift volume (51cm) + Buffer (7cm) 
    _tracking_displ (13.36 * cm),//??????????
    _energy_displ (9.5 * cm),//???????????
    _el_gap (1.4 * mm),
    _grid_thickness (.1 * mm),//????????
    _el_grid_transparency (.88),
    _cathode_grid_transparency (.98)
  {

    // Field Cage
    _field_cage = new NextNewFieldCage();
    // Energy Plane
    _energy_plane = new NextNewEnergyPlane();
    // Tracking Plane
    _tracking_plane = new NextNewTrackingPlane();

    /// Messenger
    _msg = new G4GenericMessenger(this, "/Geometry/NextNew/", "Control commands of geometry NextNew.");

    _msg->DeclareProperty("grids_vis", _grids_visibility, "Grids Visibility");
    
    G4GenericMessenger::Command& step_cmd = _msg->DeclareProperty("max_step_size", _max_step_size, "Maximum Step Size");
    step_cmd.SetUnitCategory("Length");
    step_cmd.SetParameterName("max_step_size", false);
    step_cmd.SetRange("max_step_size>0.");

  }
   
  void NextNewInnerElements::SetLogicalVolume(G4LogicalVolume* mother_logic)
  {
    _mother_logic = mother_logic;
  }

  void NextNewInnerElements::Construct()
  {
    // Reading material
    _gas = _mother_logic->GetMaterial();
    _pressure =    _gas->GetPressure();
    _temperature = _gas->GetTemperature();
    
    // Field Cage
    _field_cage->Construct();
    G4LogicalVolume* field_cage_logic = _field_cage->GetLogicalVolume();
    G4ThreeVector    field_cage_pos =   _field_cage->GetPosition();
    G4PVPlacement*   field_cage_logic_physi = new G4PVPlacement(0, field_cage_pos, field_cage_logic,
							   "FIELD_CAGE", _mother_logic, false, 0);
    // EL Region
    BuildELRegion();
    // Cathode Grid
    BuildCathodeGrid();
    // ACTIVE region
    BuildActive(); 
    // Energy Plane
    _energy_plane->SetLogicalVolume(_mother_logic);
    _energy_plane->Construct();
    // Tracking Plane
    _tracking_plane->SetLogicalVolume(_mother_logic);
    _tracking_plane->Construct();
  }

  NextNewInnerElements::~NextNewInnerElements()
  {
    delete _field_cage;
    delete _energy_plane;
    delete _tracking_plane;
    delete _active_gen;
  }

  void NextNewInnerElements::BuildELRegion()
  {
    /// EL GAP
    G4double el_gap_diam = _active_diam;
    G4double el_gap_z_pos = 25.5*cm;// (160.*cm / 2.) - _trk_displ;
    G4Tubs* el_gap_solid = new G4Tubs("EL_GAP", 0., el_gap_diam/2., _el_gap/2., 0, twopi);
    G4LogicalVolume* el_gap_logic = new G4LogicalVolume(el_gap_solid, _gas, "EL_GAP");
    G4PVPlacement* el_gap_physi = new G4PVPlacement(0, G4ThreeVector(0., 0., el_gap_z_pos), el_gap_logic,
						    "EL_GAP", _mother_logic, false, 0);

    // Define EL electric field
    UniformElectricDriftField* el_field = new UniformElectricDriftField();
    el_field->SetCathodePosition(el_gap_z_pos - _el_gap/2.);
    el_field->SetAnodePosition  (el_gap_z_pos + _el_gap/2.);
    el_field->SetDriftVelocity(5. * mm/microsecond);
    //el_field->SetFieldStrength(20 * kilovolt);
    el_field->SetTransverseDiffusion(1. * mm/sqrt(cm));
    el_field->SetLongitudinalDiffusion(1. * mm/sqrt(cm));
    G4Region* el_region = new G4Region("EL_REGION");
    el_region->SetUserInformation(el_field);
    el_region->AddRootLogicalVolume(el_gap_logic);

    /// EL GRIDS
    G4Material* fgrid_mat = MaterialsList::FakeDielectric(_gas, "el_grid_mat");
    fgrid_mat->SetMaterialPropertiesTable(OpticalMaterialProperties::FakeGrid(_pressure, _temperature, 
									      _el_grid_transparency, _grid_thickness));
    
    // Dimensions & position
    G4double grid_diam = _active_diam;
    G4double posz1 = el_gap_z_pos - _el_gap/2. - _grid_thickness/2.;
    G4double posz2 = el_gap_z_pos + _el_gap/2. + _grid_thickness/2.;
    _el_grid_ref_z = posz1;

    G4Tubs* diel_grid_solid = new G4Tubs("EL_GRID", 0., grid_diam/2., _grid_thickness/2., 0, twopi);
    G4LogicalVolume* diel_grid_logic = new G4LogicalVolume(diel_grid_solid, fgrid_mat, "EL_GRID");
    G4PVPlacement* diel_grid_physi;
    diel_grid_physi = new G4PVPlacement(0, G4ThreeVector(0.,0.,posz1), diel_grid_logic, "EL_GRID_1",
					_mother_logic, false, 0);
    diel_grid_physi = new G4PVPlacement(0, G4ThreeVector(0.,0.,posz2), diel_grid_logic, "EL_GRID_2",
					_mother_logic, false, 1);

    // /// Visibilities
    // if (_grids_visibility) {
    //   G4VisAttributes light_blue(G4Colour(0.5, 0.5, .8));
    //   light_blue.SetForceSolid(true);
    //   el_gap_logic->SetVisAttributes(light_blue);
    //   // grids are white
    // }
    // else {
    //   el_gap_logic->SetVisAttributes(G4VisAttributes::Invisible);
    //   diel_grid_logic->SetVisAttributes(G4VisAttributes::Invisible);
    // }
  }

  void NextNewInnerElements::BuildCathodeGrid()
  {
    G4Material* fgrid_mat = MaterialsList::FakeDielectric(_gas, "cath_grid_mat");
    fgrid_mat->SetMaterialPropertiesTable(OpticalMaterialProperties::FakeGrid(_pressure, _temperature, 
									      _cathode_grid_transparency, _grid_thickness));
    
    // Dimensions & position
    G4double grid_diam = _active_diam;
    G4double posz = _el_grid_ref_z - _grid_thickness - _active_length;
    //G4cout << G4endl << "Cathode Grid posz: " << posz/cm << G4endl;
    // Building the grid
    G4Tubs* diel_grid_solid = new G4Tubs("CATH_GRID", 0., grid_diam/2., _grid_thickness/2., 0, twopi);
    G4LogicalVolume* diel_grid_logic = new G4LogicalVolume(diel_grid_solid, fgrid_mat, "CATH_GRID");
    G4PVPlacement* diel_grid_physi = new G4PVPlacement(0, G4ThreeVector(0.,0.,posz), diel_grid_logic, "CATH_GRID",
						       _mother_logic, false, 0);
    /// Visibilities
    // Grid is white
    // if (!_grids_visibility) {
    //   diel_grid_logic->SetVisAttributes(G4VisAttributes::Invisible);
    //}
  }

  void NextNewInnerElements::BuildActive()
  {
    G4double active_z_pos = _el_grid_ref_z - _grid_thickness/2. - _active_length/2.;
    G4Tubs* active_solid = new G4Tubs("ACTIVE",  0., _active_diam/2., _active_length/2., 0, twopi);
    G4LogicalVolume* active_logic = new G4LogicalVolume(active_solid, _gas, "ACTIVE");
    G4PVPlacement* active_physi = new G4PVPlacement(0, G4ThreeVector(0., 0., active_z_pos), active_logic,
						    "ACTIVE", _mother_logic, false, 0);
    // Limit the step size in this volume for better tracking precision
    active_logic->SetUserLimits(new G4UserLimits(_max_step_size));
  
    // Set the volume as an ionization sensitive detector
    IonizationSD* ionisd = new IonizationSD("/NEXTNEW/ACTIVE");
    active_logic->SetSensitiveDetector(ionisd);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionisd);
    
    //Define a drift field for this volume
    UniformElectricDriftField* field = new UniformElectricDriftField();
    field->SetCathodePosition(active_z_pos - _active_length/2.);
    field->SetAnodePosition(active_z_pos + _active_length/2.);
    field->SetDriftVelocity(1. * mm/microsecond);
    field->SetTransverseDiffusion(1. * mm/sqrt(cm));
    field->SetLongitudinalDiffusion(.5 * mm/sqrt(cm));  
    G4Region* drift_region = new G4Region("DRIFT");
    drift_region->SetUserInformation(field);
    drift_region->AddRootLogicalVolume(active_logic);
    
    /// Visibilities
    active_logic->SetVisAttributes(G4VisAttributes::Invisible);

    // VERTEX GENERATOR
    _active_gen  = new CylinderPointSampler(0., _active_length, _active_diam/2.,
					    0., G4ThreeVector (0., 0., active_z_pos));
  }
  
  G4ThreeVector NextNewInnerElements::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0.,0.,0.);

    // Field Cage region
    if (region == "FIELD_CAGE") {
      vertex = _field_cage->GenerateVertex(region);
    }
    // Active region
    else if (region == "ACTIVE") {
      vertex = _active_gen->GenerateVertex(TUBE_VOLUME);
    }
    // Energy Plane regions
    else if ( (region == "CARRIER_PLATE") || (region == "ENCLOSURE_BODY") ||
	      (region == "ENCLOSURE_WINDOW") || (region == "PMT_BODY") ) {
      vertex = _energy_plane->GenerateVertex(region);
    }
    // Tracking Plane regions
    else if ( (region == "TRK_SUPPORT") || (region == "DICE_BOARD") ) {
      vertex = _tracking_plane->GenerateVertex(region);
    }
    
    return vertex;
  }
}//end namespace nexus
