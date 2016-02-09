#ifndef SIMPLEMODEL_H
#define SIMPLEMODEL_H
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<gsl/gsl_sf_ellint.h>
#define pi 3.1415926535897932384626433832795028841971693993751

// SetupStar input parameter keys
// **********************
// Model 1
// **********************
#define KEY_SS_GRID_SIZE 0 // Star setup -- number of grid points per row
#define KEY_SS_LIMB_DARKENING_1 1 // Star setup -- stellar limb darkening 1
#define KEY_SS_LIMB_DARKENING_2 2 // Star setup -- stellar limb darkening 2
#define KEY_SS_GRAVITY_DARKENING_1 3 // Star setup -- stellar gravity darkening profile
#define KEY_SS_FLATTENING 4 // Star setup -- stellar flattening
// **********************


// PlanetSetup input parameter keys
// **********************
// **********************
#define KEY_PS_GRID_SIZE 0 // Planet setup -- number of points in grid (TODO?)
#define KEY_PS_IMPACT 1 // Planet setup -- impact parameter
#define KEY_PS_RPRS 2 // Planet setup -- Radius of planet / radius of star
#define KEY_PS_SEMI_MAJOR_AXIS 3 // Planet setup -- semi major axis in R star
#define KEY_PS_OBLIQUITY 4 // Planet setup -- obliquity
#define KEY_PS_ECCENTRICITY 5 // Planet setup -- eccentricity
// **********************


using namespace std;
class SimpleModel{
	public:
		SimpleModel();
		~SimpleModel();
		void SetupStar(double *star_params, int np){
			this->star_grid_size = (int)star_params[KEY_SS_GRID_SIZE];
			this->star_pixel_size = 1.0;
			if(this->star_grid_size % 2 == 0){
		//		printf("Even sized grid detected, moving to odd size\n");
				this->star_grid_size += 1;
			}
			this->star_grid_size_half = (this->star_grid_size-1)/2;
			this->star_flux_map = (double*)calloc((this->star_grid_size*this->star_grid_size),sizeof(double));

			float mu = 0;
			long double total_flux = 0;
			for(int x=0;x<this->star_grid_size;x++){
				for(int y = 0;y<this->star_grid_size;y++){
					mu = sqrt(pow(x-this->star_grid_size_half,2)+ pow(y-this->star_grid_size_half,2))/star_grid_size_half;
					// QUADRATIC LIMB DARKENING LAW
					if(mu <= 1.0){
						this->star_flux_map[x+y*this->star_grid_size] =  1.0-(star_params[KEY_SS_LIMB_DARKENING_1]*(mu))-(star_params[KEY_SS_LIMB_DARKENING_2]*mu*mu);
						total_flux += star_flux_map[x+y*this->star_grid_size]*this->star_pixel_size;
					}else{
						this->star_flux_map[x+y*this->star_grid_size] =  0;
					}
					//printf("%f ",this->star_flux_map[x+y*this->star_grid_size]);
				}
		//		printf("\n");
			}
			this->star_total_flux = (double)total_flux;
		//	printf("Star setup complete\n");
		};
		void SetupPlanet(double *planet_params, int np){
			this->planet_grid_size = (int)planet_params[KEY_PS_GRID_SIZE];
			if(this->planet_grid_size % 2 == 0){
			//	printf("Even sized grid detected, moving to odd size\n");
				this->planet_grid_size += 1;
			}
			this->planet_oppacity_map = (double*)calloc((this->planet_grid_size*this->planet_grid_size),sizeof(double));
			this->planet_grid_size_half = (this->planet_grid_size-1)/2;
			this->rp_rs = planet_params[KEY_PS_RPRS];
			this->planet_pixel_size = this->rp_rs*this->rp_rs;
			this->semi_major = planet_params[KEY_PS_SEMI_MAJOR_AXIS];
			this->impact_parameter = planet_params[KEY_PS_IMPACT];
			this->obliquity = planet_params[KEY_PS_OBLIQUITY];
			double mu;
			for(int x=0;x<this->planet_grid_size;x++){
				for(int y = 0;y<this->planet_grid_size;y++){
					mu = sqrt(pow(x-this->planet_grid_size_half,2) + pow(y-this->planet_grid_size_half,2))/planet_grid_size_half;
					if(mu <= 1.0){
						this->planet_oppacity_map[x+y*this->planet_grid_size] =  1.0;
					}else{
						this->planet_oppacity_map[x+y*this->planet_grid_size] =  0;
					}
				}
			}
		//	printf("Planet setup complete\n");
		};
		void RelativeFlux(double *phase, int np, double *flux_out, int npo){
		//	printf("Starting integration\n");
			double planet_position_x = 0;
			double planet_position_y = 0;
			double current_flux = 0;
			for(int a=0; a<np;a++){
				current_flux = 0;
				planet_position_x = this->semi_major*sin(phase[a]);
				planet_position_y = this->impact_parameter + tan(this->obliquity)*planet_position_x;
				for(int x=0;x<this->planet_grid_size;x++){
					for(int y=0;y<this->planet_grid_size;y++){
						int star_x = (((x-this->planet_grid_size_half)/this->planet_grid_size)*this->rp_rs+planet_position_x)*1.5*this->star_grid_size;
						int star_y = (((y-this->planet_grid_size_half)/this->planet_grid_size)*this->rp_rs+planet_position_y)*1.5*this->star_grid_size;
						if(star_x >= 0 && star_x < this->star_grid_size && star_y >= 0 && star_y < this->star_grid_size){
							current_flux += this->planet_pixel_size*this->planet_oppacity_map[x+y*this->planet_grid_size]*this->star_flux_map[star_x + this->star_grid_size*star_y];
						}
					}
				}
				flux_out[a] = (this->star_total_flux - current_flux)/this->star_total_flux;
			}
		//	printf("Flux integration complete\n");
		};
	private:
		double *star_flux_map;
		double *planet_oppacity_map;
		double star_total_flux;
		double rp_rs; // Radius of planet over radius of star
		double star_pixel_size;
		double planet_pixel_size;
		double semi_major;
		double impact_parameter;
		double obliquity;
		int star_grid_size;
		int star_grid_size_half;
		int planet_grid_size;
		int planet_grid_size_half;

};

#endif
