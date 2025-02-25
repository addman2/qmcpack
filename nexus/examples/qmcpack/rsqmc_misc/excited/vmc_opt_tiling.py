#! /usr/bin/env python

from nexus import settings,job,run_project
from nexus import generate_physical_system
from nexus import generate_pwscf
from nexus import generate_pw2qmcpack
from nexus import generate_qmcpack,vmc
from structure import *

settings(
    pseudo_dir    = '../../pseudopotentials',
    status_only   = 0,
    generate_only = 0,
    sleep         = 3,
    machine       = 'ws16'
    )

#Input structure
dia = generate_physical_system(
    units  = 'A',
    axes   = [[ 1.785,  1.785,  0.   ],
              [ 0.   ,  1.785,  1.785],
              [ 1.785,  0.   ,  1.785]],
    elem   = ['C','C'],
    pos    = [[ 0.    ,  0.    ,  0.    ],
              [ 0.8925,  0.8925,  0.8925]],
    C      = 4
    )

# Standardized Primitive cell -- run rest of the calculations on this cell
dia2_structure   = get_primitive_cell(structure=dia.structure)['structure']
# get_band_tiling and get_primitiev_cell require "SeeK-path" python libraries

# Returns commensurate optimum tiling matrix for the kpoints_rel wavevectors
# max_volfac is default to 20
tiling = get_band_tiling(structure   = dia2_structure,
                         kpoints_rel = [[0.000, 0.000, 0.000],
                                        [0.3768116, 0.,        0.3768116]])
# Numerical value for the second wavevector can be different in different computers, adjust accordingly
# All wavevectors must be on the iBZ k_path! (get_kpath output in band.py)

# K-points can also be defined using their labels (for high symmetry k-points only). E.g.:
# tiling = get_band_tiling(structure   = dia2_structure,
#                         kpoints_label = ['GAMMA', 'X'])

dia2 = generate_physical_system(
    structure    = dia2_structure,
    kgrid  = (1,1,1), 
    kshift = (0,0,0), # Assumes we study transitions from Gamma. For non-gamma tilings, use kshift appropriately
    tiling = tiling,
    C            = 4,
    )

scf = generate_pwscf(
    identifier   = 'scf',
    path         = 'diamond/scf',
    job          = job(nodes=1, app='pw.x',hours=1),
    input_type   = 'generic',
    calculation  = 'scf',
    nspin        = 2,
    input_dft    = 'lda', 
    ecutwfc      = 200,   
    conv_thr     = 1e-8, 
    nosym        = True,
    wf_collect   = True,
    system       = dia2,
    tot_magnetization = 0,
    pseudos      = ['C.BFD.upf'], 
    )

nscf = generate_pwscf(
    identifier   = 'nscf',
    path         ='diamond/nscf_opt_tiling',
    job          = job(nodes=1, app='pw.x',hours=1),
    input_type   = 'generic',
    calculation  = 'nscf',
    input_dft    = 'lda', 
    ecutwfc      = 200,
    nspin        = 2,   
    conv_thr     = 1e-8,
    nosym        = True,
    wf_collect   = True,
    system       = dia2,
    nbnd         = 8,      #a sensible nbnd value can be given 
    verbosity    = 'high', #verbosity must be set to high
    pseudos      = ['C.BFD.upf'], 
    dependencies = (scf, 'charge_density'),
)

conv = generate_pw2qmcpack(
    identifier   = 'conv',
    path         = 'diamond/nscf_opt_tiling',
    job          = job(cores=1,app='pw2qmcpack.x',hours=1),
    write_psir   = False,
    dependencies = (nscf,'orbitals'),
    )

qmc = generate_qmcpack(
    det_format     = 'old',
    identifier     = 'vmc',
    path           = 'diamond/vmc_opt_tiling',
    job            = job(cores=16,threads=16,app='qmcpack',hours = 1),
    input_type     = 'basic',
    spin_polarized = True,
    system         = dia2,
    pseudos        = ['C.BFD.xml'],
    jastrows       = [],
    calculations   = [
        vmc(
            walkers     =  16,
            warmupsteps =  20,
            blocks      = 1000,
            steps       =  10,
            substeps    =   2,
            timestep    =  .4
            )
        ],
    dependencies = (conv,'orbitals'),
    )

qmc_optical = generate_qmcpack(
    det_format     = 'old',
    identifier     = 'vmc',
    path           = 'diamond/vmc_opt_tiling_optical',
    job            = job(cores=16,threads=16,app='qmcpack',hours = 1),
    input_type     = 'basic',
    spin_polarized = True,
    system         = dia2,
    excitation     = ['up', '0 3 1 4'], #
    pseudos        = ['C.BFD.xml'],
    jastrows       = [],
    calculations   = [
        vmc(
            walkers     =  16,
            warmupsteps =  20,
            blocks      = 1000,
            steps       =  10,
            substeps    =   2,
            timestep    =  .4
            )
        ],
    dependencies = (conv,'orbitals'),
    )

run_project()
