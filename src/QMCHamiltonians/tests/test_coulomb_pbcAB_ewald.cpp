//////////////////////////////////////////////////////////////////////////////////////
// This file is distributed under the University of Illinois/NCSA Open Source License.
// See LICENSE file in top directory for details.
//
// Copyright (c) 2016 Jeongnim Kim and QMCPACK developers.
//
// File developed by: Mark Dewing, markdewing@gmail.com, University of Illinois at Urbana-Champaign
//
// File created by: Mark Dewing, markdewing@gmail.com, University of Illinois at Urbana-Champaign
//////////////////////////////////////////////////////////////////////////////////////


#include "catch.hpp"

#include "OhmmsData/Libxml2Doc.h"
#include "OhmmsPETE/OhmmsMatrix.h"
#include "Lattice/ParticleBConds.h"
#include "Particle/ParticleSet.h"
#include "Particle/DistanceTableData.h"
#ifndef ENABLE_SOA
#include "Particle/SymmetricDistanceTableData.h"
#endif
#include "QMCApp/ParticleSetPool.h"
#include "QMCHamiltonians/CoulombPBCAB.h"
#include "QMCHamiltonians/CoulombPBCAA.h"
#include "LongRange/EwaldHandler3D.h"

#include <stdio.h>
#include <string>

using std::string;

namespace qmcplusplus
{
TEST_CASE("Coulomb PBC A-B Ewald3D", "[hamiltonian]")
{
  Communicate* c;
  OHMMS::Controller->initialize(0, NULL);
  c = OHMMS::Controller;

  CrystalLattice<OHMMS_PRECISION, OHMMS_DIM> Lattice;
  Lattice.BoxBConds = true; // periodic
  Lattice.R.diagonal(1.0);
  Lattice.reset();


  ParticleSet ions;
  ParticleSet elec;

  ions.setName("ion");
  ions.create(1);
  ions.R[0][0] = 0.0;
  ions.R[0][1] = 0.0;
  ions.R[0][2] = 0.0;

  //  ions.Lattice.LR_dim_cutoff=40;

  SpeciesSet& ion_species       = ions.getSpeciesSet();
  int pIdx                      = ion_species.addSpecies("H");
  int pChargeIdx                = ion_species.addAttribute("charge");
  ion_species(pChargeIdx, pIdx) = 1;
  ions.Lattice = Lattice;
  ions.createSK();


  elec.Lattice = Lattice;
  elec.setName("elec");
  elec.create(1);
  elec.R[0][0] = 0.5;
  elec.R[0][1] = 0.0;
  elec.R[0][2] = 0.0;

  //  elec.Lattice.LR_dim_cutoff=40;

  SpeciesSet& tspecies         = elec.getSpeciesSet();
  int upIdx                    = tspecies.addSpecies("u");
  int downIdx                  = tspecies.addSpecies("d");
  int chargeIdx                = tspecies.addAttribute("charge");
  int massIdx                  = tspecies.addAttribute("mass");
  tspecies(chargeIdx, upIdx)   = -1;
  tspecies(chargeIdx, downIdx) = -1;
  tspecies(massIdx, upIdx)     = 1.0;
  tspecies(massIdx, downIdx)   = 1.0;

  elec.createSK();

#ifdef ENABLE_SOA
  elec.addTable(ions, DT_SOA);
#else
  elec.addTable(ions, DT_AOS);
#endif
  elec.update();


  ParticleSetPool ptcl = ParticleSetPool(c);

  LRCoulombSingleton::CoulombHandler = new EwaldHandler3D(ions);
  LRCoulombSingleton::CoulombHandler->initBreakup(ions);


  CoulombPBCAB cab = CoulombPBCAB(ions, elec);

  // Self energy plus Background charge term
  double consts = cab.evalConsts();
  REQUIRE(consts == Approx(0.0));

  double val_ei = cab.evaluate(elec);
  REQUIRE(val_ei == Approx(-0.008302)); //Not validated

  CoulombPBCAA caa_elec = CoulombPBCAA(elec, false);
  CoulombPBCAA caa_ion  = CoulombPBCAA(ions, false);
  double val_ee         = caa_elec.evaluate(elec);
  double val_ii         = caa_ion.evaluate(ions);
  double sum            = val_ee + val_ii + val_ei;

  REQUIRE(val_ee == Approx(-1.366567));
  REQUIRE(val_ii == Approx(-1.366567));
  REQUIRE(sum == Approx(-2.741436)); // Can be validated via Ewald summation elsewhere
                                     // -2.74136517454081

  delete LRCoulombSingleton::CoulombHandler;
  LRCoulombSingleton::CoulombHandler = 0;
}

TEST_CASE("Coulomb PBC A-B BCC H Ewald3D", "[hamiltonian]")
{
  Communicate* c;
  OHMMS::Controller->initialize(0, NULL);
  c = OHMMS::Controller;

  CrystalLattice<OHMMS_PRECISION, OHMMS_DIM> Lattice;
  Lattice.BoxBConds = true; // periodic
  Lattice.R.diagonal(3.77945227);
  Lattice.reset();


  ParticleSet ions;
  ParticleSet elec;

  ions.setName("ion");
  ions.create(2);
  ions.R[0][0] = 0.0;
  ions.R[0][1] = 0.0;
  ions.R[0][2] = 0.0;
  ions.R[1][0] = 1.88972614;
  ions.R[1][1] = 1.88972614;
  ions.R[1][2] = 1.88972614;


  SpeciesSet& ion_species       = ions.getSpeciesSet();
  int pIdx                      = ion_species.addSpecies("H");
  int pChargeIdx                = ion_species.addAttribute("charge");
  ion_species(pChargeIdx, pIdx) = 1;
  ions.Lattice = Lattice;
  ions.createSK();


  elec.Lattice = Lattice;
  elec.setName("elec");
  elec.create(2);
  elec.R[0][0] = 0.5;
  elec.R[0][1] = 0.0;
  elec.R[0][2] = 0.0;
  elec.R[1][0] = 0.0;
  elec.R[1][1] = 0.5;
  elec.R[1][2] = 0.0;


  SpeciesSet& tspecies         = elec.getSpeciesSet();
  int upIdx                    = tspecies.addSpecies("u");
  int downIdx                  = tspecies.addSpecies("d");
  int chargeIdx                = tspecies.addAttribute("charge");
  int massIdx                  = tspecies.addAttribute("mass");
  tspecies(chargeIdx, upIdx)   = -1;
  tspecies(chargeIdx, downIdx) = -1;
  tspecies(massIdx, upIdx)     = 1.0;
  tspecies(massIdx, downIdx)   = 1.0;

  elec.createSK();

#ifdef ENABLE_SOA
  elec.addTable(ions, DT_SOA);
#else
  elec.addTable(ions, DT_AOS);
#endif
  elec.update();


  ParticleSetPool ptcl = ParticleSetPool(c);

  LRCoulombSingleton::CoulombHandler = new EwaldHandler3D(ions);
  LRCoulombSingleton::CoulombHandler->initBreakup(ions);

  CoulombPBCAB cab = CoulombPBCAB(ions, elec);

  // Background charge term
  double consts = cab.evalConsts();
  REQUIRE(consts == Approx(0.0));


  double val_ei = cab.evaluate(elec);
  REQUIRE(val_ei == Approx(-2.223413)); //Not validated


  CoulombPBCAA caa_elec = CoulombPBCAA(elec, false);
  CoulombPBCAA caa_ion  = CoulombPBCAA(ions, false);
  double val_ee         = caa_elec.evaluate(elec);
  double val_ii         = caa_ion.evaluate(ions);
  double sum            = val_ee + val_ii + val_ei;

  REQUIRE(val_ee == Approx(-0.012808));
  REQUIRE(val_ii == Approx(-0.907659));
  REQUIRE(sum == Approx(-3.143880)); // Can be validated via Ewald summation elsewhere
                                     // -3.14349127313640

  delete LRCoulombSingleton::CoulombHandler;
  LRCoulombSingleton::CoulombHandler = 0;
}

} // namespace qmcplusplus
