#include "INS_functions.H"
#include <iostream>
#include <fstream>
#include<stdio.h>

#include "common_functions.H"
#include "gmres_functions.H"

#include "common_namespace_declarations.H"

#include "gmres_namespace_declarations.H"


#include "species.H"
#include "paramPlane.H"

#include "particle_functions.H"

//#include "electrostatic.H"

using namespace std;
//using namespace amrex;

// argv contains the name of the inputs file entered at the command line
void main_driver(const char* argv)
{
    remove("out.csv");
    // store the current time so we can later compute total run time.
  
    Real strt_time = ParallelDescriptor::second();

	std::string inputs_file = argv;
	Print() << argv << std::endl;
	//Abort();
    
    // read in parameters from inputs file into F90 modules
    // we use "+1" because of amrex_string_c_to_f expects a null char termination
    read_common_namelist(inputs_file.c_str(),inputs_file.size()+1);

    // copy contents of F90 modules to C++ namespaces
    InitializeCommonNamespace();
    InitializeGmresNamespace();

    

    const int n_rngs = 1;

    int fhdSeed = 0;
    int particleSeed = 0;
    int selectorSeed = 0;
    int thetaSeed = 0;
    int phiSeed = 0;
    int generalSeed = 0;

    if(seed > 0)
    {
        fhdSeed = ParallelDescriptor::MyProc() + 1 + seed;
        particleSeed = 2*ParallelDescriptor::MyProc() + 2 + seed;
        selectorSeed = 3*ParallelDescriptor::MyProc() + 3 + seed;
        thetaSeed = 4*ParallelDescriptor::MyProc() + 4 + seed;
        phiSeed = 5*ParallelDescriptor::MyProc() + 5 + seed;
        generalSeed = 6*ParallelDescriptor::MyProc() + 6 + seed;
    }

    //Initialise rngs
    rng_initialize(&fhdSeed,&particleSeed,&selectorSeed,&thetaSeed,&phiSeed,&generalSeed);

    // is the problem periodic?
    Vector<int> is_periodic(AMREX_SPACEDIM,0);  // set to 0 (not periodic) by default
    for (int i=0; i<AMREX_SPACEDIM; ++i) {
        if (bc_vel_lo[i] == -1 && bc_vel_hi[i] == -1) {
            is_periodic[i] = 1;
        }
    }

    // make BoxArray and Geometry

    BoxArray ba;
    Geometry geom;
    
    IntVect dom_lo(AMREX_D_DECL(           0,            0,            0));
    IntVect dom_hi(AMREX_D_DECL(n_cells[0]-1, n_cells[1]-1, n_cells[2]-1));
    Box domain(dom_lo, dom_hi);

    ba.define(domain);

    // Break up boxarray "ba" into chunks no larger than "max_grid_size" along a direction
    // note we are converting "Vector<int> max_grid_size" to an IntVect
    ba.maxSize(IntVect(max_grid_size));

    RealBox real_box({AMREX_D_DECL(prob_lo[0],prob_lo[1],prob_lo[2])},
                     {AMREX_D_DECL(prob_hi[0],prob_hi[1],prob_hi[2])});


    // This defines a Geometry object
    geom.define(domain,&real_box,CoordSys::cartesian,is_periodic.data());

    // how boxes are distrubuted among MPI processes
    // AJN needs to be fi
    DistributionMapping dmap(ba);

    Print() << geom << "\n";
    Print() << domain << "\n";

    const Real* dx = geom.CellSize();


    MultiFab cellVols(ba, dmap, 1, 0);

#if (AMREX_SPACEDIM == 2)
    cellVols.setVal(dx[0]*dx[1]*cell_depth);
#elif (AMREX_SPACEDIM == 3)
    cellVols.setVal(dx[0]*dx[1]*dx[2]);
#endif

    getCellVols(cellVols, geom, 1000);

    const RealBox& realDomain = geom.ProbDomain();

    Real dt = fixed_dt;
    Real dtinv = 1.0/dt;

    ifstream paramPlaneFile("paramplanes.dat");
    int paramPlaneCount;
    double effectiveVol;

    paramPlaneFile >> paramPlaneCount;
    paramPlaneFile >> effectiveVol;

    //Print() << "sc: " << paramPlaneCount << ", ev: " << effectiveVol << "\n";

#if (BL_SPACEDIM == 3)
    paramPlaneCount = paramPlaneCount + 6;
    //paramPlaneCount = 6;
    paramPlane paramPlaneList[paramPlaneCount];
    BuildParamplanes(paramPlaneList,paramPlaneCount,realDomain.lo(),realDomain.hi());

#endif
#if (BL_SPACEDIM == 2)
    paramPlaneCount = paramPlaneCount + 4;
    paramPlane paramPlaneList[paramPlaneCount];
    BuildParamplanes(paramPlaneList,paramPlaneCount,realDomain.lo(),realDomain.hi());
#endif

   //Add interior boundaries

    double phi, theta;
#if (BL_SPACEDIM == 3)
    for(int i=6; i<paramPlaneCount; i++)
    {
#endif
#if (BL_SPACEDIM == 2)
    for(int i=4; i<paramPlaneCount; i++)
    {
#endif

        paramPlaneFile >> paramPlaneList[i].x0;
        paramPlaneFile >> paramPlaneList[i].y0;
#if (BL_SPACEDIM == 3)
        paramPlaneFile >> paramPlaneList[i].z0;
#endif

        paramPlaneFile >> paramPlaneList[i].ux;
        paramPlaneFile >> paramPlaneList[i].uy;
#if (BL_SPACEDIM == 3)
        paramPlaneFile >> paramPlaneList[i].uz;
#endif
        paramPlaneFile >> paramPlaneList[i].vx;
        paramPlaneFile >> paramPlaneList[i].vy;
#if (BL_SPACEDIM == 3)
        paramPlaneFile >> paramPlaneList[i].vz;
#endif
        paramPlaneFile >> paramPlaneList[i].uTop;
#if (BL_SPACEDIM == 3)
        paramPlaneFile >> paramPlaneList[i].vTop;
#endif
        //Print() << paramPlaneList[i].x0 << ", " << paramPlaneList[i].y0 << ", " << paramPlaneList[i].z0 << "\n";

        paramPlaneFile >> paramPlaneList[i].rnx;
        paramPlaneFile >> paramPlaneList[i].rny;
#if (BL_SPACEDIM == 3)
        paramPlaneFile >> paramPlaneList[i].rnz;
#endif
        paramPlaneFile >> paramPlaneList[i].lnx;
        paramPlaneFile >> paramPlaneList[i].lny;
#if (BL_SPACEDIM == 3)
        paramPlaneFile >> paramPlaneList[i].lnz;
#endif
        paramPlaneFile >> paramPlaneList[i].porosityRight;
        paramPlaneFile >> paramPlaneList[i].specularityRight;
        paramPlaneFile >> paramPlaneList[i].temperatureRight;
        paramPlaneFile >> paramPlaneList[i].momentumConsRight;

        paramPlaneFile >> paramPlaneList[i].porosityLeft;
        paramPlaneFile >> paramPlaneList[i].specularityLeft;
        paramPlaneFile >> paramPlaneList[i].temperatureLeft;
        paramPlaneFile >> paramPlaneList[i].momentumConsLeft;

        paramPlaneFile >> paramPlaneList[i].periodicity;

#if (BL_SPACEDIM == 3)
        theta = getTheta(paramPlaneList[i].lnx, paramPlaneList[i].lny, paramPlaneList[i].lnz);
        phi   = getPhi(paramPlaneList[i].lnx, paramPlaneList[i].lny, paramPlaneList[i].lnz);

        paramPlaneList[i].cosThetaLeft = cos(theta);
        paramPlaneList[i].sinThetaLeft = sin(theta);
        paramPlaneList[i].cosPhiLeft = cos(phi);
        paramPlaneList[i].sinPhiLeft = sin(phi);

        theta = getTheta(paramPlaneList[i].rnx, paramPlaneList[i].rny, paramPlaneList[i].rnz);
        phi   = getPhi(paramPlaneList[i].rnx, paramPlaneList[i].rny, paramPlaneList[i].rnz);

        paramPlaneList[i].cosThetaRight = cos(theta);
        paramPlaneList[i].sinThetaRight = sin(theta);
        paramPlaneList[i].cosPhiRight = cos(phi);
        paramPlaneList[i].sinPhiRight = sin(phi);

#endif
#if (BL_SPACEDIM == 2)
        theta = getTheta(paramPlaneList[i].lnx, paramPlaneList[i].lny, 0);

        paramPlaneList[i].cosThetaLeft = cos(theta);
        paramPlaneList[i].sinThetaLeft = sin(theta);

        theta = getTheta(paramPlaneList[i].rnx, paramPlaneList[i].rny, 0);

        paramPlaneList[i].cosThetaRight = cos(theta);
        paramPlaneList[i].sinThetaRight = sin(theta);
#endif
        paramPlaneList[i].fxLeftAv = 0;
        paramPlaneList[i].fyLeftAv = 0;
        paramPlaneList[i].fzLeftAv = 0;

        paramPlaneList[i].fxRightAv = 0;
        paramPlaneList[i].fyRightAv = 0;
        paramPlaneList[i].fzRightAv = 0;
    }

    paramPlaneFile.close();

    const int* lims = domain.hiVect();

    // AJN - get rid of collision stuff?
#if (AMREX_SPACEDIM == 2)
    int totalCollisionCells = (lims[0]+1)*(lims[1]+1);
#elif (AMREX_SPACEDIM == 3)
    int totalCollisionCells = (lims[0]+1)*(lims[1]+1)*(lims[2]+1);
#endif

#if (AMREX_SPACEDIM == 2)
    double domainVol = (prob_hi[0] - prob_lo[0])*(prob_hi[1] - prob_lo[1])*cell_depth;
#elif (AMREX_SPACEDIM == 3)
    double domainVol = (prob_hi[0] - prob_lo[0])*(prob_hi[1] - prob_lo[1])*(prob_hi[2] - prob_lo[2]);
#endif

    //Species type defined in species.H
    //array of length nspecies

    int flux[2];

    species dsmcParticle[nspecies];

    double realParticles = 0;
    double simParticles = 0;

    for(int i=0;i<nspecies;i++) {
        
        dsmcParticle[i].m = mass[i];
        dsmcParticle[i].d = diameter[i];

        dsmcParticle[i].Neff = particle_neff;
        dsmcParticle[i].R = k_B/dsmcParticle[i].m;
        dsmcParticle[i].T = T_init[0];

        if(particle_count[i] >= 0) {
            // adjust number of particles up so there is the same number per box            
            dsmcParticle[i].ppb = (int)amrex::Math::ceil((double)particle_count[i]/(double)ba.size());
            dsmcParticle[i].total = dsmcParticle[i].ppb*ba.size();
            dsmcParticle[i].n0 = dsmcParticle[i].total/effectiveVol;

            Print() << "Species " << i << " count adjusted to " << dsmcParticle[i].total << "\n";
        }
        else {
            // if particle count is negative, we instead compute the number of particles based on particle density and particle_neff
            dsmcParticle[i].total = (int)amrex::Math::ceil(particle_n0[i]*effectiveVol/particle_neff);
                Print() << "TOTAL: " << dsmcParticle[i].total << ", " << particle_n0[i] << "\n";

            // adjust number of particles up so there is the same number per box  
            dsmcParticle[i].ppb = (int)amrex::Math::ceil((double)dsmcParticle[i].total/(double)ba.size());
            dsmcParticle[i].total = dsmcParticle[i].ppb*ba.size();
            dsmcParticle[i].n0 = (dsmcParticle[i].total/effectiveVol)*particle_neff;

            Print() << "Species " << i << " n0 adjusted to " << dsmcParticle[i].n0 << "\n";
            Print() << "Effective volume: " << effectiveVol << "\n";
        }

        Print() << "Species " << i << " particles per box: " <<  dsmcParticle[i].ppb << "\n";

        realParticles = realParticles + dsmcParticle[i].total*particle_neff;
        simParticles = simParticles + dsmcParticle[i].total;
    }
    
    Print() << "Total real particles: " << realParticles << "\n";
    Print() << "Total sim particles: " << simParticles << "\n";

    Print() << "Sim particles per box: " << simParticles/(double)ba.size() << "\n";

    Print() << "Collision cells: " << totalCollisionCells << "\n";
    Print() << "Sim particles per cell: " << simParticles/totalCollisionCells << "\n";


    // MFs for storing particle statistics
    
    //Members
    //Density
    //velx
    //vely
    //velz
    //Temperature
    //jx
    //jy
    //jz
    //energyDensity
    //pressure
    //Cx
    //Cy
    //Cz
    MultiFab particleInstant(ba, dmap, 14, 0);

    //Members
    //Density
    //velx
    //vely
    //velz
    //Temperature
    //jx
    //jy
    //jz
    //energyDensity
    //pressure
    //Cx
    //Cy
    //Cz    
    MultiFab particleMeans(ba, dmap, 14, 0);

    //Members
    //Density
    //velx
    //vely
    //velz
    //Temperature
    //jx
    //jy
    //jz
    //energyDensity
    //pressure
    //GVar
    //KGCross
    //KRhoCross
    //RhoGCross
    //Cx
    //Cy
    //Cz 
   
    MultiFab particleVars(ba, dmap, 18, 0);


    MultiFab collisionPairs(ba, dmap, 1, 0);    
    MultiFab collisionFactor(ba, dmap, 1, 0);


    collisionFactor.setVal(0);
    collisionPairs.setVal(0);
   

    //Particles! Build on geom & box array for collision cells/ poisson grid?
    DsmcParticleContainer particles(geom, dmap, ba, crange);
    //create particles
    particles.InitParticles(dsmcParticle,dx);

    //This will cause problems for cells with less than 2 particles. No need to run this for now.
    //particles.InitializeFields(particleInstant, cellVols, dsmcParticle[0]);

    //setup initial DSMC collision parameters
    particles.InitCollisionCells(collisionPairs, collisionFactor, cellVols, dsmcParticle[0], dt);

    int statsCount = 1;
    double time = 0;
    //Time stepping loop
	//remove("out.txt");
	paramPlaneList[5].bgraph=0;
	paramPlaneList[5].bgraph=0;
    for(int step=1;step<=max_step;++step)
    {


        if(move_tog==1)
        {
	   particles.MoveParticlesDSMC(dt,paramPlaneList, paramPlaneCount, time, flux);
           particles.Redistribute();

           particles.ReBin();
        }

        if(sr_tog==1)
        {
            particles.CollideParticles(collisionPairs, collisionFactor, cellVols, dsmcParticle[0], dt);
        }

        //Start collecting statistics after step n_steps_skip
        if(step == n_steps_skip)
        {
            particleMeans.setVal(0.0);
            particleVars.setVal(0);

	    // Print() << "Resetting stat collection.\n";

            statsCount = 1;
        }
       
        particles.EvaluateStats(particleInstant, particleMeans, particleVars, cellVols, dsmcParticle[0], dt,statsCount);
        statsCount++;

        if (plot_int > 0 && step%plot_int == 0)
        {
           
	  //WritePlotFile(step,time,geom,particleInstant, particleMeans, particleVars, cellVols, particles);
        }

        if(step%1 == 0)
        {    
	  amrex::Print() << "Advanced step " << step<<", " << domega<< "\n";
        }
        
        time = time + dt;

    }
    // Call the timer again and compute the maximum difference between the start time 
    // and stop time over all processors
    Real stop_time = ParallelDescriptor::second() - strt_time;
    ParallelDescriptor::ReduceRealMax(stop_time);
    amrex::Print() << "Run time = " << stop_time << std::endl;
}
