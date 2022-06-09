#include "MFsurfchem_functions.H"
#include "AMReX_ParmParse.H"

AMREX_GPU_MANAGED int MFsurfchem::n_ads_spec;
AMREX_GPU_MANAGED GpuArray<amrex::Real, MAX_SPECIES> MFsurfchem::surfcov0;

AMREX_GPU_MANAGED amrex::Real MFsurfchem::surf_site_num_dens;

AMREX_GPU_MANAGED GpuArray<amrex::Real, MAX_SPECIES> MFsurfchem::ads_rate_const;
AMREX_GPU_MANAGED GpuArray<amrex::Real, MAX_SPECIES> MFsurfchem::des_rate;

void InitializeMFSurfchemNamespace()
{
    // extract inputs parameters
    ParmParse pp;

    n_ads_spec = 0;
    // get the number of species that undergoes adsoprtion/desorption
    pp.query("n_ads_spec",n_ads_spec);

    // if n_ads_spec is set to 0 or not defined in the inputs file, quit the routine
    if (n_ads_spec==0) return;

    std::vector<amrex::Real> surfcov0_tmp(MAX_SPECIES);
    // get initial surface coverage
    pp.getarr("surfcov0",surfcov0_tmp,0,n_ads_spec);
    for (int m=0;m<n_ads_spec;m++) surfcov0[m] = surfcov0_tmp[m];

    surf_site_num_dens = 0.;
    // get number density of adsorption sites on the lattice
    pp.query("surf_site_num_dens",surf_site_num_dens);

    std::vector<amrex::Real> ads_rate_const_tmp(MAX_SPECIES);
    // get adsorption rate const 
    pp.getarr("ads_rate_const",ads_rate_const_tmp,0,n_ads_spec);
    for (int m=0;m<n_ads_spec;m++) ads_rate_const[m] = ads_rate_const_tmp[m];

    std::vector<amrex::Real> des_rate_tmp(MAX_SPECIES);
    // get desoprtion rate
    pp.getarr("des_rate",des_rate_tmp,0,n_ads_spec);
    for (int m=0;m<n_ads_spec;m++) des_rate[m] = des_rate_tmp[m];
    
    return;
}

void init_surfcov(MultiFab& surfcov, const amrex::Real* dx)
{
    for (MFIter mfi(surfcov,false); mfi.isValid(); ++mfi)
    {
        const Box& bx = mfi.tilebox();
        Dim3 lo = lbound(bx);
        Dim3 hi = ubound(bx);
        const Array4<Real> & surfcov_arr = surfcov.array(mfi);

        amrex::ParallelForRNG(bx, [=] AMREX_GPU_DEVICE (int i, int j, int k, RandomEngine const& engine) noexcept
        {
            if (k==0) {
                for (int m=0;m<n_ads_spec;m++) {
		            surfcov_arr(i,j,k,m) = surfcov0[m];
                }
            } else {
                for (int m=0;m<n_ads_spec;m++) {
                    surfcov_arr(i,j,k,m) = 0.;
                }
            }
        });
    }

    return;
}

void sample_MFsurfchem(MultiFab& cu, MultiFab& prim, MultiFab& surfcov, MultiFab& dNadsdes,
                       const amrex::Real* dx, const amrex::Real dt)
{
    for (MFIter mfi(cu,false); mfi.isValid(); ++mfi)
    {
        const Box& bx = mfi.tilebox();
        Dim3 lo = lbound(bx);
        Dim3 hi = ubound(bx);
        const Array4<Real> & cu_arr = cu.array(mfi);
        const Array4<Real> & prim_arr = prim.array(mfi);
        const Array4<Real> & surfcov_arr = surfcov.array(mfi);
        const Array4<Real> & dNadsdes_arr = dNadsdes.array(mfi);

        amrex::Real Ntot = surf_site_num_dens*dx[0]*dx[1];  // total number of reactive sites

        amrex::ParallelForRNG(bx, [=] AMREX_GPU_DEVICE (int i, int j, int k, RandomEngine const& engine) noexcept
        {
            if (k==0) {
                amrex:: Real sumtheta = 0.;
                for (int m=0;m<n_ads_spec;m++) {
                    sumtheta += surfcov_arr(i,j,k,m);
                }

                amrex:: Real tempratio = prim_arr(i,j,k,4)/T_init[0];

                for (int m=0;m<n_ads_spec;m++) {
                    amrex::Real dens = cu_arr(i,j,k,5+m);   // mass density
                    dens *= AVONUM/molmass[m];              // number density

                    amrex::Real theta = surfcov_arr(i,j,k,m);

                    amrex::Real meanNads = ads_rate_const[m]*dens*(1-sumtheta)*Ntot*dt*pow(tempratio,-0.5*dof[m]);
                    amrex::Real Nads = RandomPoisson(meanNads,engine);

                    amrex::Real meanNdes = des_rate[m]*theta*Ntot*dt;
                    amrex::Real Ndes = RandomPoisson(meanNdes,engine);

                    dNadsdes_arr(i,j,k,m) = Nads-Ndes;
                }
            }
        });
    }

    return;
}

void update_MFsurfchem(MultiFab& cu, MultiFab& surfcov, MultiFab& dNadsdes,
                       const amrex::Real* dx)
{
    for (MFIter mfi(cu,false); mfi.isValid(); ++mfi)
    {
        const Box& bx = mfi.tilebox();
        Dim3 lo = lbound(bx);
        Dim3 hi = ubound(bx);
        const Array4<Real> & cu_arr = cu.array(mfi);
        const Array4<Real> & surfcov_arr = surfcov.array(mfi);
        const Array4<Real> & dNadsdes_arr = dNadsdes.array(mfi);

        amrex::Real Ntot = surf_site_num_dens*dx[0]*dx[1];  // total number of reactive sites

        amrex::ParallelFor(bx, [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
        {
            if (k==0) {
                for (int m=0;m<n_ads_spec;m++) {
                    amrex::Real dN = dNadsdes_arr(i,j,k,m);
                    amrex::Real factor = molmass[m]/AVONUM/(dx[0]*dx[1]*dx[2]);

                    surfcov_arr(i,j,k,m) += dN/Ntot;
                    cu_arr(i,j,k,0) -= factor*dN;
                    cu_arr(i,j,k,5+m) -= factor*dN;
                }
            }
        });
    }

    return;
}
