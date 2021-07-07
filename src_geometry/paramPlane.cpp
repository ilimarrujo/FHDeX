#include "paramPlane.H"
#include <math.h>

#include "common_functions.H"


using namespace amrex;
using namespace std;
double getTheta(double nx, double ny, double nz)
{

    double r = sqrt(nx*nx+ny*ny+nz*nz);
    return acos(nz/r);


}

double getPhi(double nx, double ny, double nz)
{
    return atan2(ny,nx);
}

void BuildParamplanes(paramPlane* paramPlaneList, const int paramplanes, const Real* domainLo, const Real* domainHi)
{

		double theta, phi;
    Real xl = prob_hi[0] - prob_lo[0];
    Real yl = prob_hi[1] - prob_lo[1];
    Real zl = prob_hi[2] - prob_lo[2];

		//Domain boundaries		
		// Initialize Boundaries when possible
		// Likely redundant		
		for(int i=0; i<6; i++) {
			for(int l=0;l<nspecies;l++) {
	    	paramPlaneList[i].densityLeft[l]  = rho0*Yk0[l]/(mass[l]*particle_neff);
	    	paramPlaneList[i].densityRight[l] = rho0*Yk0[l]/(mass[l]*particle_neff);
	    }
	    paramPlaneList[i].temperatureLeft  = T_init[0];
	    paramPlaneList[i].temperatureRight = T_init[0];
	    
	    paramPlaneList[i].specularityLeft   = 0;
    	paramPlaneList[i].momentumConsLeft  = 1;
	    paramPlaneList[i].sourceLeft 			  = 0;
    	paramPlaneList[i].sinkLeft   			  = 0;
    	
    	paramPlaneList[i].specularityRight 	= 0;
    	paramPlaneList[i].momentumConsRight = 1;
      paramPlaneList[i].sourceRight 			= 0;
    	paramPlaneList[i].sinkRight   			= 0;
	    
	    paramPlaneList[i].boundary = i+1;
	  }
		

//domainLo x plane
		paramPlaneList[0].x0 = domainLo[0];
    paramPlaneList[0].y0 = domainLo[1];
    paramPlaneList[0].z0 = domainLo[2];
    
    paramPlaneList[0].area = zl*yl;

    paramPlaneList[0].ux = 0;
    paramPlaneList[0].uy = 1;
    paramPlaneList[0].uz = 0;

    paramPlaneList[0].vx = 0;
    paramPlaneList[0].vy = 0;
    paramPlaneList[0].vz = 1;

    paramPlaneList[0].uTop = domainHi[1] - domainLo[1];
    paramPlaneList[0].vTop = domainHi[2] - domainLo[2];

    paramPlaneList[0].lnx = -1;
    paramPlaneList[0].lny = 0;
    paramPlaneList[0].lnz = 0;
    
    paramPlaneList[0].rnx = 1;
    paramPlaneList[0].rny = 0;
    paramPlaneList[0].rnz = 0;

    if(bc_vel_lo[0] == -1)
    {   
        paramPlaneList[0].periodicity = 1;
        paramPlaneList[0].porosityLeft = 1;
        paramPlaneList[0].porosityRight = 1;
    }
    else if(bc_vel_lo[0] == 2)
    {
        paramPlaneList[0].periodicity = 0;
        paramPlaneList[0].porosityLeft = 0;
        paramPlaneList[0].porosityRight = 0;
        paramPlaneList[0].specularityLeft = 1;
        paramPlaneList[0].specularityRight = 1;

       // paramPlaneList[0].x0 = domainLo[0] + sigma[0]/2.0;
    }
    else if(bc_vel_lo[0] == 3)
    {
				paramPlaneList[0].periodicity = 1;
        paramPlaneList[0].porosityLeft = 1;
        paramPlaneList[0].porosityRight = 1;   
    }
    else{

        paramPlaneList[0].periodicity = 0;
        paramPlaneList[0].porosityLeft = 0;
        paramPlaneList[0].porosityRight = 0;
        paramPlaneList[0].specularityLeft = 1;
        paramPlaneList[0].specularityRight = 1;
    }
        
    theta = getTheta(paramPlaneList[0].lnx, paramPlaneList[0].lny, paramPlaneList[0].lnz);
    phi   = getPhi(paramPlaneList[0].lnx, paramPlaneList[0].lny, paramPlaneList[0].lnz);

    paramPlaneList[0].cosThetaLeft = cos(theta);
    paramPlaneList[0].sinThetaLeft = sin(theta);
    paramPlaneList[0].cosPhiLeft = cos(phi);
    paramPlaneList[0].sinPhiLeft = sin(phi);

    theta = getTheta(paramPlaneList[0].rnx, paramPlaneList[0].rny, paramPlaneList[0].rnz);
    phi   = getPhi(paramPlaneList[0].rnx, paramPlaneList[0].rny, paramPlaneList[0].rnz);

    paramPlaneList[0].cosThetaRight = cos(theta);
    paramPlaneList[0].sinThetaRight = sin(theta);
    paramPlaneList[0].cosPhiRight = cos(phi);
    paramPlaneList[0].sinPhiRight = sin(phi);

    paramPlaneList[0].fxLeftAv = 0;
    paramPlaneList[0].fyLeftAv = 0;
    paramPlaneList[0].fzLeftAv = 0;

    paramPlaneList[0].fxRightAv = 0;
    paramPlaneList[0].fyRightAv = 0;
    paramPlaneList[0].fzRightAv = 0;

//domainHi x plane
    paramPlaneList[1].x0 = domainHi[0];
    paramPlaneList[1].y0 = domainLo[1];
    paramPlaneList[1].z0 = domainLo[2];
    
    paramPlaneList[1].area = zl*yl;

    paramPlaneList[1].ux = 0;
    paramPlaneList[1].uy = 1;
    paramPlaneList[1].uz = 0;

    paramPlaneList[1].vx = 0;
    paramPlaneList[1].vy = 0;
    paramPlaneList[1].vz = 1;

    paramPlaneList[1].uTop = domainHi[1] - domainLo[1];
    paramPlaneList[1].vTop = domainHi[2] - domainLo[2];

    paramPlaneList[1].lnx = -1;
    paramPlaneList[1].lny = 0;
    paramPlaneList[1].lnz = 0;
    
    paramPlaneList[1].rnx = 1;
    paramPlaneList[1].rny = 0;
    paramPlaneList[1].rnz = 0;

    if(bc_vel_hi[0] == -1)
    {   
        paramPlaneList[1].periodicity = 1;
        paramPlaneList[1].porosityLeft = 1;
        paramPlaneList[1].porosityRight = 1;
    }
    else if (bc_vel_hi[0] == 2) 
    {
        paramPlaneList[1].periodicity = 0;
        paramPlaneList[1].porosityLeft = 0;
        paramPlaneList[1].porosityRight = 0;
        paramPlaneList[1].specularityLeft = 1;
        paramPlaneList[1].specularityRight = 1;

      //  paramPlaneList[1].x0 = domainHi[0] - sigma[0]/2.0;
    }
    else if(bc_vel_hi[0] == 3)
    {
				paramPlaneList[1].periodicity = 1;
        paramPlaneList[1].porosityLeft = 1;
        paramPlaneList[1].porosityRight = 1;   
    }
    else{
        paramPlaneList[1].periodicity = 0;
        paramPlaneList[1].porosityLeft = 0;
        paramPlaneList[1].porosityRight = 0;
        paramPlaneList[1].specularityLeft = 1;
        paramPlaneList[1].specularityRight = 1;

    }

    theta = getTheta(paramPlaneList[1].lnx, paramPlaneList[1].lny, paramPlaneList[1].lnz);
    phi   = getPhi(paramPlaneList[1].lnx, paramPlaneList[1].lny, paramPlaneList[1].lnz);

    paramPlaneList[1].cosThetaLeft = cos(theta);
    paramPlaneList[1].sinThetaLeft = sin(theta);
    paramPlaneList[1].cosPhiLeft = cos(phi);
    paramPlaneList[1].sinPhiLeft = sin(phi);

    theta = getTheta(paramPlaneList[1].rnx, paramPlaneList[1].rny, paramPlaneList[1].rnz);
    phi   = getPhi(paramPlaneList[1].rnx, paramPlaneList[1].rny, paramPlaneList[1].rnz);

    paramPlaneList[1].cosThetaRight = cos(theta);
    paramPlaneList[1].sinThetaRight = sin(theta);
    paramPlaneList[1].cosPhiRight = cos(phi);
    paramPlaneList[1].sinPhiRight = sin(phi);

    paramPlaneList[1].fxLeftAv = 0;
    paramPlaneList[1].fyLeftAv = 0;
    paramPlaneList[1].fzLeftAv = 0;

    paramPlaneList[1].fxRightAv = 0;
    paramPlaneList[1].fyRightAv = 0;
    paramPlaneList[1].fzRightAv = 0;

//domainLo y plane
    paramPlaneList[2].x0 = domainLo[0];
    paramPlaneList[2].y0 = domainLo[1];
    paramPlaneList[2].z0 = domainLo[2];
    
    paramPlaneList[2].area = xl*zl;

    paramPlaneList[2].ux = 1;
    paramPlaneList[2].uy = 0;
    paramPlaneList[2].uz = 0;

    paramPlaneList[2].vx = 0;
    paramPlaneList[2].vy = 0;
    paramPlaneList[2].vz = 1;

    paramPlaneList[2].uTop = domainHi[0] - domainLo[0];
    paramPlaneList[2].vTop = domainHi[2] - domainLo[2];

    paramPlaneList[2].lnx = 0;
    paramPlaneList[2].lny = 1;
    paramPlaneList[2].lnz = 0;
    
    paramPlaneList[2].rnx = 0;
    paramPlaneList[2].rny = -1;
    paramPlaneList[2].rnz = 0;

    if(bc_vel_lo[1] == -1)
    {   
        paramPlaneList[2].periodicity = 1;
        paramPlaneList[2].porosityLeft = 1;
        paramPlaneList[2].porosityRight = 1;
    }
    else if (bc_vel_lo[1] == 2) 
    {
        paramPlaneList[2].periodicity = 0;
        paramPlaneList[2].porosityLeft = 0;
        paramPlaneList[2].porosityRight = 0;
        paramPlaneList[2].specularityLeft = 1;
        paramPlaneList[2].specularityRight = 1;

       // paramPlaneList[2].y0 = domainLo[1] + sigma[0]/2.0;
    }
    else if(bc_vel_lo[1] == 3)
    {
				paramPlaneList[2].periodicity = 1;
        paramPlaneList[2].porosityLeft = 1;
        paramPlaneList[2].porosityRight = 1;   
    }
    else{
        paramPlaneList[2].periodicity = 0;
        paramPlaneList[2].porosityLeft = 0;
        paramPlaneList[2].porosityRight = 0;
        paramPlaneList[2].specularityLeft = 1;
        paramPlaneList[2].specularityRight = 1;
    }

    theta = getTheta(paramPlaneList[2].lnx, paramPlaneList[2].lny, paramPlaneList[2].lnz);
    phi   = getPhi(paramPlaneList[2].lnx, paramPlaneList[2].lny, paramPlaneList[2].lnz);

    paramPlaneList[2].cosThetaLeft = cos(theta);
    paramPlaneList[2].sinThetaLeft = sin(theta);
    paramPlaneList[2].cosPhiLeft = cos(phi);
    paramPlaneList[2].sinPhiLeft = sin(phi);

    theta = getTheta(paramPlaneList[2].rnx, paramPlaneList[2].rny, paramPlaneList[2].rnz);
    phi   = getPhi(paramPlaneList[2].rnx, paramPlaneList[2].rny, paramPlaneList[2].rnz);

    paramPlaneList[2].cosThetaRight = cos(theta);
    paramPlaneList[2].sinThetaRight = sin(theta);
    paramPlaneList[2].cosPhiRight = cos(phi);
    paramPlaneList[2].sinPhiRight = sin(phi);

    paramPlaneList[2].fxLeftAv = 0;
    paramPlaneList[2].fyLeftAv = 0;
    paramPlaneList[2].fzLeftAv = 0;

    paramPlaneList[2].fxRightAv = 0;
    paramPlaneList[2].fyRightAv = 0;
    paramPlaneList[2].fzRightAv = 0;

//domianHi y plane
    paramPlaneList[3].x0 = domainLo[0];
    paramPlaneList[3].y0 = domainHi[1];
    paramPlaneList[3].z0 = domainLo[2];
    
    paramPlaneList[3].area = xl*zl;

    paramPlaneList[3].ux = 1;
    paramPlaneList[3].uy = 0;
    paramPlaneList[3].uz = 0;

    paramPlaneList[3].vx = 0;
    paramPlaneList[3].vy = 0;
    paramPlaneList[3].vz = 1;

    paramPlaneList[3].uTop = domainHi[0] - domainLo[0];
    paramPlaneList[3].vTop = domainHi[2] - domainLo[2];

    paramPlaneList[3].lnx = 0;
    paramPlaneList[3].lny = 1;
    paramPlaneList[3].lnz = 0;
    
    paramPlaneList[3].rnx = 0;
    paramPlaneList[3].rny = -1;
    paramPlaneList[3].rnz = 0;

    if(bc_vel_hi[1] == -1)
    {   
        paramPlaneList[3].periodicity = 1;
        paramPlaneList[3].porosityLeft = 1;
        paramPlaneList[3].porosityRight = 1;
    }
    else if (bc_vel_hi[1] == 2) 
    {
        paramPlaneList[3].periodicity = 0;
        paramPlaneList[3].porosityLeft = 0;
        paramPlaneList[3].porosityRight = 0;
        paramPlaneList[3].specularityLeft = 1;
        paramPlaneList[3].specularityRight = 1;

       // paramPlaneList[3].y0 = domainHi[1] - sigma[0]/2.0;
    }
    else if(bc_vel_hi[1] == 3)
    {
				paramPlaneList[3].periodicity = 1;
        paramPlaneList[3].porosityLeft = 1;
        paramPlaneList[3].porosityRight = 1;   
    }
    else{
        paramPlaneList[3].periodicity = 0;
        paramPlaneList[3].porosityLeft = 0;
        paramPlaneList[3].porosityRight = 0;
        paramPlaneList[3].specularityLeft = 1;
        paramPlaneList[3].specularityRight = 1;

    }

    theta = getTheta(paramPlaneList[3].lnx, paramPlaneList[3].lny, paramPlaneList[3].lnz);
    phi   = getPhi(paramPlaneList[3].lnx, paramPlaneList[3].lny, paramPlaneList[3].lnz);

    paramPlaneList[3].cosThetaLeft = cos(theta);
    paramPlaneList[3].sinThetaLeft = sin(theta);
    paramPlaneList[3].cosPhiLeft = cos(phi);
    paramPlaneList[3].sinPhiLeft = sin(phi);

    theta = getTheta(paramPlaneList[3].rnx, paramPlaneList[3].rny, paramPlaneList[3].rnz);
    phi   = getPhi(paramPlaneList[3].rnx, paramPlaneList[3].rny, paramPlaneList[3].rnz);

    paramPlaneList[3].cosThetaRight = cos(theta);
    paramPlaneList[3].sinThetaRight = sin(theta);
    paramPlaneList[3].cosPhiRight = cos(phi);
    paramPlaneList[3].sinPhiRight = sin(phi);

    paramPlaneList[3].fxLeftAv = 0;
    paramPlaneList[3].fyLeftAv = 0;
    paramPlaneList[3].fzLeftAv = 0;

    paramPlaneList[3].fxRightAv = 0;
    paramPlaneList[3].fyRightAv = 0;
    paramPlaneList[3].fzRightAv = 0;

//domainLo z plane
    paramPlaneList[4].x0 = domainLo[0];
    paramPlaneList[4].y0 = domainLo[1];
    paramPlaneList[4].z0 = domainLo[2];
    
    paramPlaneList[4].area = xl*yl;

    paramPlaneList[4].ux = 1;
    paramPlaneList[4].uy = 0;
    paramPlaneList[4].uz = 0;

    paramPlaneList[4].vx = 0;
    paramPlaneList[4].vy = 1;
    paramPlaneList[4].vz = 0;

    paramPlaneList[4].uTop = domainHi[0] - domainLo[0];
    paramPlaneList[4].vTop = domainHi[1] - domainLo[1];

    paramPlaneList[4].lnx = 0;
    paramPlaneList[4].lny = 0;
    paramPlaneList[4].lnz = 1;
    
    paramPlaneList[4].rnx = 0;
    paramPlaneList[4].rny = 0;
    paramPlaneList[4].rnz = -1;

    if(bc_vel_lo[2] == -1)
    {   
        paramPlaneList[4].periodicity = 1;
        paramPlaneList[4].porosityLeft = 1;
        paramPlaneList[4].porosityRight = 1;
    }
    else if (bc_vel_lo[2] == 2) 
    {
        paramPlaneList[4].periodicity = 0;
        paramPlaneList[4].porosityLeft = 0;
        paramPlaneList[4].porosityRight = 0;
        paramPlaneList[4].specularityLeft = 1;
        paramPlaneList[4].specularityRight = 1;

       // paramPlaneList[4].z0 = domainLo[2] + sigma[0]/2.0;
    }
    else if(bc_vel_lo[2] == 3)
    {
				paramPlaneList[4].periodicity = 1;
        paramPlaneList[4].porosityLeft = 1;
        paramPlaneList[4].porosityRight = 1;   
    }
    else{
        paramPlaneList[4].periodicity = 0;
        paramPlaneList[4].porosityLeft = 0;
        paramPlaneList[4].porosityRight = 0;
        paramPlaneList[4].specularityLeft = 1;
        paramPlaneList[4].specularityRight = 1;


    }

    theta = getTheta(paramPlaneList[4].lnx, paramPlaneList[4].lny, paramPlaneList[4].lnz);
    phi   = getPhi(paramPlaneList[4].lnx, paramPlaneList[4].lny, paramPlaneList[4].lnz);

    paramPlaneList[4].cosThetaLeft = cos(theta);
    paramPlaneList[4].sinThetaLeft = sin(theta);
    paramPlaneList[4].cosPhiLeft = cos(phi);
    paramPlaneList[4].sinPhiLeft = sin(phi);

    theta = getTheta(paramPlaneList[4].rnx, paramPlaneList[4].rny, paramPlaneList[4].rnz);
    phi   = getPhi(paramPlaneList[4].rnx, paramPlaneList[4].rny, paramPlaneList[4].rnz);

    paramPlaneList[4].cosThetaRight = cos(theta);
    paramPlaneList[4].sinThetaRight = sin(theta);
    paramPlaneList[4].cosPhiRight = cos(phi);
    paramPlaneList[4].sinPhiRight = sin(phi);

    paramPlaneList[4].fxLeftAv = 0;
    paramPlaneList[4].fyLeftAv = 0;
    paramPlaneList[4].fzLeftAv = 0;

    paramPlaneList[4].fxRightAv = 0;
    paramPlaneList[4].fyRightAv = 0;
    paramPlaneList[4].fzRightAv = 0;

//domianHi z plane
    paramPlaneList[5].x0 = domainLo[0];
    paramPlaneList[5].y0 = domainLo[1];
    paramPlaneList[5].z0 = domainHi[2];
    
    paramPlaneList[5].area = xl*yl;

    paramPlaneList[5].ux = 1;
    paramPlaneList[5].uy = 0;
    paramPlaneList[5].uz = 0;

    paramPlaneList[5].vx = 0;
    paramPlaneList[5].vy = 1;
    paramPlaneList[5].vz = 0;

    paramPlaneList[5].uTop = domainHi[0] - domainLo[0];
    paramPlaneList[5].vTop = domainHi[1] - domainLo[1];

    paramPlaneList[5].lnx = 0;
    paramPlaneList[5].lny = 0;
    paramPlaneList[5].lnz = 1;
    
    paramPlaneList[5].rnx = 0;
    paramPlaneList[5].rny = 0;
    paramPlaneList[5].rnz = -1;

    paramPlaneList[5].velx=0;
    paramPlaneList[5].vely=0;
    paramPlaneList[5].velz=0;

    paramPlaneList[5].c0=0;
    paramPlaneList[5].resomg=0;
    paramPlaneList[5].agraph=0;
    paramPlaneList[5].bgraph=0;
    paramPlaneList[5].a0graph=0;
    paramPlaneList[5].b0graph=0;
    paramPlaneList[5].coltime=0;

    paramPlaneList[5].besslist;
    paramPlaneList[5].dbesslist;

    if(bc_vel_hi[2] == -1)
    {   
        paramPlaneList[5].periodicity = 1;
        paramPlaneList[5].porosityLeft = 1;
        paramPlaneList[5].porosityRight = 1;
    }
    else if (bc_vel_hi[2] == 2) 
    {
        paramPlaneList[5].periodicity = 0;
        paramPlaneList[5].porosityLeft = 0;
        paramPlaneList[5].porosityRight = 0;
        paramPlaneList[5].specularityLeft = 1;
        paramPlaneList[5].specularityRight = 1;

       // paramPlaneList[5].z0 = domainHi[2] - sigma[0]/2.0;
    }
    else if(bc_vel_hi[2] == 3)
    {
				paramPlaneList[5].periodicity = 1;
        paramPlaneList[5].porosityLeft = 1;
        paramPlaneList[5].porosityRight = 1;   
    }
    else{
        paramPlaneList[5].periodicity = 0;
        paramPlaneList[5].porosityLeft = 0;
        paramPlaneList[5].porosityRight = 0;
        paramPlaneList[5].specularityLeft = 1;
        paramPlaneList[5].specularityRight = 1;

    }

    theta = getTheta(paramPlaneList[5].lnx, paramPlaneList[5].lny, paramPlaneList[5].lnz);
    phi   = getPhi(paramPlaneList[5].lnx, paramPlaneList[5].lny, paramPlaneList[5].lnz);

    paramPlaneList[5].cosThetaLeft = cos(theta);
    paramPlaneList[5].sinThetaLeft = sin(theta);
    paramPlaneList[5].cosPhiLeft = cos(phi);
    paramPlaneList[5].sinPhiLeft = sin(phi);

    theta = getTheta(paramPlaneList[5].rnx, paramPlaneList[5].rny, paramPlaneList[5].rnz);
    phi   = getPhi(paramPlaneList[5].rnx, paramPlaneList[5].rny, paramPlaneList[5].rnz);

    paramPlaneList[5].cosThetaRight = cos(theta);
    paramPlaneList[5].sinThetaRight = sin(theta);
    paramPlaneList[5].cosPhiRight = cos(phi);
    paramPlaneList[5].sinPhiRight = sin(phi);

    paramPlaneList[5].fxLeftAv = 0;
    paramPlaneList[5].fyLeftAv = 0;
    paramPlaneList[5].fzLeftAv = 0;

    paramPlaneList[5].fxRightAv = 0;
    paramPlaneList[5].fyRightAv = 0;
    paramPlaneList[5].fzRightAv = 0;
    /*
    for(int i=0; i<6; i++) {
			cout << "Plane: " << i << "\n";
    	cout << paramPlaneList[i].temperatureLeft << "\n";
    	cout << paramPlaneList[i].temperatureRight << "\n";
    	cout << paramPlaneList[i].densityLeft[0] << "\n";
    	cout << paramPlaneList[i].densityRight[0] << "\n";
    }*/

		// Normalize Yk at boundaries
		if(bc_mass_lo[0]>0) {
			Real Yktot = 0.;
			for(int l=0; l<nspecies; l++) { Yktot += bc_Yk_x_lo[l]; }
			for(int l=0; l<nspecies; l++) { bc_Yk_x_lo[l] /= Yktot; }
		}
		if(bc_mass_hi[0]>0) {
			Real Yktot = 0.;
			for(int l=0; l<nspecies; l++) { Yktot += bc_Yk_x_hi[l]; }
			for(int l=0; l<nspecies; l++) { bc_Yk_x_hi[l] /= Yktot; }
		}
		if(bc_mass_lo[1]>0) {
			Real Yktot = 0.;
			for(int l=0; l<nspecies; l++) { Yktot += bc_Yk_y_lo[l]; }
			for(int l=0; l<nspecies; l++) { bc_Yk_y_lo[l] /= Yktot; }
		}
		if(bc_mass_hi[1]>0) {
			Real Yktot = 0.;
			for(int l=0; l<nspecies; l++) { Yktot += bc_Yk_y_hi[l]; }
			for(int l=0; l<nspecies; l++) { bc_Yk_y_hi[l] /= Yktot; }
		}
		if(bc_mass_lo[2]>0) {
			Real Yktot = 0.;
			for(int l=0; l<nspecies; l++) { Yktot += bc_Yk_z_lo[l]; }
			for(int l=0; l<nspecies; l++) { bc_Yk_z_lo[l] /= Yktot; }
		}
		if(bc_mass_hi[2]>0) {
			Real Yktot = 0.;
			for(int l=0; l<nspecies; l++) { Yktot += bc_Yk_z_hi[l]; }
			for(int l=0; l<nspecies; l++) { bc_Yk_z_hi[l] /= Yktot; }
		}

    // Thermal/Conc BC
    // Adiabatic not considered here
    // Implemented Lees-Edwards BC for unbounded shear (bc_vel==3)
    int p = 0;
    for(int i=0; i<3 ; i++) {
			// Lower (Left, Bottom, Back)
			paramPlaneList[p].specularityLeft = 0;
    	paramPlaneList[p].specularityRight = 0;
    	paramPlaneList[p].porosityLeft = 1;
    	paramPlaneList[p].porosityRight = 1;
    	paramPlaneList[p].sourceLeft = 0;
    	paramPlaneList[p].sinkLeft = 0;
      paramPlaneList[p].sourceRight = 0;
	    paramPlaneList[p].sinkRight = 0;
    	paramPlaneList[p].periodicity = 0;
	    paramPlaneList[p].temperatureLeft  = T_init[0];
	    paramPlaneList[p].temperatureRight = T_init[0];

			for (int l=0; l<nspecies; l++) {
				if(i==0) {
					paramPlaneList[p].densityLeft[l] = -1;
					paramPlaneList[p].densityRight[l] = -1;
				} else if(i==1) {
					paramPlaneList[p].densityLeft[l] = -1;
					paramPlaneList[p].densityRight[l] = -1;
				} else {
					paramPlaneList[p].densityLeft[l]  = -1;
					paramPlaneList[p].densityRight[l] = -1;
				}
			}

			// Periodic
			if(bc_mass_lo[i] == -1 &&
				bc_therm_lo[i] == -1 &&
				(bc_vel_lo[i] == -1 || bc_vel_lo[i] == 3)) {
				paramPlaneList[p].periodicity = 1;

			// Solid Wall
			} else if (bc_vel_lo[i] == 1 ||
				bc_vel_lo[i] == 2 ||
				bc_therm_lo[i] == 2 ||
				bc_mass_lo[i] == 1) {
				paramPlaneList[p].porosityLeft = 0;
				paramPlaneList[p].porosityRight = 0;

				// Thermal Wall
				if(bc_therm_lo[i] == 2) {
					paramPlaneList[p].temperatureLeft = t_lo[i];
    			paramPlaneList[p].temperatureRight = t_lo[i];
    		// Full slip
    		} else if(bc_vel_lo[i] == 1) {
    			paramPlaneList[p].specularityLeft = 1;
    			paramPlaneList[p].specularityRight = 1;
    		}
    		
    		// Concentration Wall
    		if(bc_mass_lo[i] == 1) {
					for (int l=0; l<nspecies; l++) {
						if(i==0) {
							paramPlaneList[p].densityLeft[l] = 
								bc_Yk_x_lo[l];
							paramPlaneList[p].densityRight[l] = 
								bc_Yk_x_lo[l];
						} else if(i==1) {
							paramPlaneList[p].densityLeft[l] = 
								bc_Yk_y_lo[l];
							paramPlaneList[p].densityRight[l] = 
								bc_Yk_y_lo[l];
						} else {
							paramPlaneList[p].densityLeft[l]  = 
								bc_Yk_z_lo[l];
							paramPlaneList[p].densityRight[l] = 
								bc_Yk_z_lo[l];
						}
					}
    		}

    	// Reservoir
    	} else if (bc_mass_lo[i] == 2) {
  	  	paramPlaneList[p].sourceLeft = 0;
 	     	paramPlaneList[p].sourceRight = 1;
 	     	paramPlaneList[p].sinkLeft = 1;
	    	paramPlaneList[p].sinkRight = 0;
				for (int l=0; l<nspecies; l++) {
					if(i==0) {
						paramPlaneList[p].densityLeft[l]  = 
							rho_lo[i]*bc_Yk_x_lo[l]/(mass[l]*particle_neff);
						paramPlaneList[p].densityRight[l] = 
							rho_lo[i]*bc_Yk_x_lo[l]/(mass[l]*particle_neff);
					} else if(i==1) {
						paramPlaneList[p].densityLeft[l]  = 
							rho_lo[i]*bc_Yk_y_lo[l]/(mass[l]*particle_neff);
						paramPlaneList[p].densityRight[l] = 
							rho_lo[i]*bc_Yk_y_lo[l]/(mass[l]*particle_neff);
					} else {
						paramPlaneList[p].densityLeft[l]  = 
							rho_lo[i]*bc_Yk_z_lo[l]/(mass[l]*particle_neff);
						paramPlaneList[p].densityRight[l] = 
							rho_lo[i]*bc_Yk_z_lo[l]/(mass[l]*particle_neff);
					}
				}
				
				if(bc_therm_lo[i] == 2) {
					paramPlaneList[p].temperatureLeft = t_lo[i];
    			paramPlaneList[p].temperatureRight = t_lo[i];					
				}
			}
    	
			// Upper Boundary (Right, Top, Front)
			paramPlaneList[p+1].specularityLeft = 0;
			paramPlaneList[p+1].specularityRight = 0;
			paramPlaneList[p+1].sourceLeft = 0;
    	paramPlaneList[p+1].sinkLeft = 0;
      paramPlaneList[p+1].sourceRight = 0;
	    paramPlaneList[p+1].sinkRight = 0;    	
    	paramPlaneList[p+1].porosityLeft = 1;
    	paramPlaneList[p+1].porosityRight = 1;
    	paramPlaneList[p+1].periodicity = 0;
	    paramPlaneList[p+1].temperatureLeft  = T_init[0];
	    paramPlaneList[p+1].temperatureRight = T_init[0];
	    
			for (int l=0; l<nspecies; l++) {
				if(i==0) {
					paramPlaneList[p+1].densityLeft[l] = -1;
					paramPlaneList[p+1].densityRight[l] = -1;
				} else if(i==1) {
					paramPlaneList[p+1].densityLeft[l] = -1;
					paramPlaneList[p+1].densityRight[l] = -1;
				} else {
					paramPlaneList[p+1].densityLeft[l]  = -1;
					paramPlaneList[p+1].densityRight[l] = -1;
				}
			}

			// Periodic
			if(bc_mass_hi[i] == -1 &&
				bc_therm_hi[i] == -1 &&
				(bc_vel_hi[i] == -1 || bc_vel_hi[i] == 3)) {
				paramPlaneList[p+1].periodicity = 1;

			// Solid Wall
			} else if (bc_vel_hi[i] == 1 ||
				bc_vel_hi[i] == 2 ||
				bc_therm_hi[i] == 2 ||
				bc_mass_hi[i] == 1) {
				paramPlaneList[p+1].porosityLeft = 0;
				paramPlaneList[p+1].porosityRight = 0;

				if(bc_therm_hi[i] == 2) {
					paramPlaneList[p+1].temperatureLeft = t_hi[i];
    			paramPlaneList[p+1].temperatureRight = t_hi[i];
    		} else if(bc_vel_hi[i] == 2) {
    			paramPlaneList[p+1].specularityLeft = 1;
    			paramPlaneList[p+1].specularityRight = 1;
    		}

				// Concentration wall
    		if(bc_mass_hi[i] == 1) {    		
 					for (int l=0; l<nspecies; l++) {
						if(i==0) {
							paramPlaneList[p+1].densityLeft[l] = 
								bc_Yk_x_hi[l];
							paramPlaneList[p+1].densityRight[l] = 
								bc_Yk_x_hi[l];
						} else if(i==1) {
							paramPlaneList[p+1].densityLeft[l] = 
								bc_Yk_y_hi[l];
							paramPlaneList[p+1].densityRight[l] = 
								bc_Yk_y_hi[l];
						} else {
							paramPlaneList[p+1].densityLeft[l]  = 
								bc_Yk_z_hi[l];
							paramPlaneList[p+1].densityRight[l] = 
								bc_Yk_z_hi[l];
						}
					}
    		}

    	// Reservoir
    	} else if (bc_mass_hi[i] == 2) {
  	  	paramPlaneList[p+1].sourceLeft = 1;
 	     	paramPlaneList[p+1].sourceRight = 0;
 	     	paramPlaneList[p+1].sinkLeft = 0;
	    	paramPlaneList[p+1].sinkRight = 1;

				for (int l=0; l<nspecies; l++) {
					if(i==0) {
						paramPlaneList[p+1].densityLeft[l]  = 
							rho_hi[i]*bc_Yk_x_hi[l]/(mass[l]*particle_neff);
						paramPlaneList[p+1].densityRight[l] = 
							rho_hi[i]*bc_Yk_x_hi[l]/(mass[l]*particle_neff);
					} else if(i==1) {
						paramPlaneList[p+1].densityLeft[l]  = 
							rho_hi[i]*bc_Yk_y_hi[l]/(mass[l]*particle_neff);
						paramPlaneList[p+1].densityRight[l] = 
							rho_hi[i]*bc_Yk_y_hi[l]/(mass[l]*particle_neff);
					} else {
						paramPlaneList[p+1].densityLeft[l]  = 
							rho_hi[i]*bc_Yk_z_hi[l]/(mass[l]*particle_neff);
						paramPlaneList[p+1].densityRight[l] = 
							rho_hi[i]*bc_Yk_z_hi[l]/(mass[l]*particle_neff);
					}
				}
				if(bc_therm_hi[i] == 2) {
					paramPlaneList[p+1].temperatureLeft = t_hi[i];
    			paramPlaneList[p+1].temperatureRight = t_hi[i];					
				}
			}
    	p += 2;
    }
    
    /*
    for(int l=0; l<6; l++) {
    	amrex::Print() << "I am plane: " << l << "\n";
    	amrex::Print() << "Periodic: " << paramPlaneList[l].periodicity << "\n";
    	amrex::Print() << "SpecL: " << paramPlaneList[l].specularityLeft << "\n";
    	amrex::Print() << "SpecR: " << paramPlaneList[l].specularityRight << "\n";
    	amrex::Print() << "SinkL: " << paramPlaneList[l].sinkLeft << "\n";
    	amrex::Print() << "SinkR: " << paramPlaneList[l].sinkRight << "\n";
    	amrex::Print() << "SourceR: " << paramPlaneList[l].sourceRight << "\n";
    	amrex::Print() << "SourceL: " << paramPlaneList[l].sourceLeft << "\n";
    	amrex::Print() << "PoroL: " << paramPlaneList[l].porosityLeft << "\n";
    	amrex::Print() << "PoroR: " << paramPlaneList[l].porosityRight << "\n";
    	amrex::Print() << "TL: " << paramPlaneList[l].temperatureLeft << "\n";
    	amrex::Print() << "TR: " << paramPlaneList[l].temperatureRight << "\n";
    	amrex::Print() << "rhoL: " << paramPlaneList[l].densityLeft[0] << "\n";
    	amrex::Print() << "rhoR: " << paramPlaneList[l].densityRight[0] << "\n";    	
    }*/

    std::ifstream planeFile("paramplanes.dat");
    int fileCount;
    planeFile >> fileCount;
    
    int totalCount = 6+fileCount;

    for(int i=6; i<totalCount; i++)
    {

        planeFile >> paramPlaneList[i].x0;

        Print() << "surface " << i << " xo " << paramPlaneList[i].x0 << "\n";

        planeFile >> paramPlaneList[i].y0;
        planeFile >> paramPlaneList[i].z0;

        planeFile >> paramPlaneList[i].ux;
        planeFile >> paramPlaneList[i].uy;
        planeFile >> paramPlaneList[i].uz;

        planeFile >> paramPlaneList[i].vx;
        planeFile >> paramPlaneList[i].vy;
        planeFile >> paramPlaneList[i].vz;

        planeFile >> paramPlaneList[i].uTop;
        planeFile >> paramPlaneList[i].vTop;

        planeFile >> paramPlaneList[i].rnx;
        planeFile >> paramPlaneList[i].rny;
        planeFile >> paramPlaneList[i].rnz;

        planeFile >> paramPlaneList[i].lnx;
        planeFile >> paramPlaneList[i].lny;
        planeFile >> paramPlaneList[i].lnz;
        
        planeFile >> paramPlaneList[i].porosityRight;
        planeFile >> paramPlaneList[i].specularityRight;
        planeFile >> paramPlaneList[i].temperatureRight;
        for(int j=0; j < nspecies; j++)
        {
            planeFile >> paramPlaneList[i].densityRight[j];
        }
        planeFile >> paramPlaneList[i].sourceRight;
        planeFile >> paramPlaneList[i].sinkRight;
        planeFile >> paramPlaneList[i].momentumConsRight;

        planeFile >> paramPlaneList[i].porosityLeft;
        planeFile >> paramPlaneList[i].specularityLeft;
        planeFile >> paramPlaneList[i].temperatureLeft;
        for(int j=0; j < nspecies; j++)
        {
           planeFile >> paramPlaneList[i].densityLeft[j];
        }
        planeFile >> paramPlaneList[i].sourceLeft;
        planeFile >> paramPlaneList[i].sinkLeft;
        planeFile >> paramPlaneList[i].momentumConsLeft;

        planeFile >> paramPlaneList[i].periodicity;

        planeFile >> paramPlaneList[i].area;

        paramPlaneList[i].boundary = i+1;
            
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

        paramPlaneList[i].fxLeftAv = 0;
        paramPlaneList[i].fyLeftAv = 0;
        paramPlaneList[i].fzLeftAv = 0;

        paramPlaneList[i].fxRightAv = 0;
        paramPlaneList[i].fyRightAv = 0;
        paramPlaneList[i].fzRightAv = 0;        

    }

    planeFile.close();
}
