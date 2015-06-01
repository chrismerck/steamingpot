// swmath.h
//
// several useful math utilities
//


#define true 1
#define false 0
#define PI 3.14159

double weighted_angle_average(double a1, double a2, double weight) {
	// weight = 1 ==> a1
	// weight = 0 ==> a2
	
	if (a2 - a1 > PI) a1 += 2*PI;
	if (a1 - a2 > PI) a2 += 2*PI;
	double a3 = weight * a1 + (1-weight)*a2;
	if (a3 > PI) a3 -= 2*PI;
	if(isnan(a3)){
		printf("IS NAN!!!\n");
		a3=0;
	}



	return a3;
}

double weighted_average(double a1, double a2, double weight) {
	// weight = 1 ==> a1
	// weight = 0 ==> a2
	
	double a3 = weight * a1 + (1-weight)*a2;
	if(isnan(a3)){
		printf("IS NAN!!!\n");
		a3=0;
	}

	return a3;
}


// modulo for doubles
double mod_double(double t, double period) {
	if (t >= period)
		return mod_double(t-period,period);
	if (t < 0)
		return mod_double(t+period,period);
	if(isnan(t)){
		printf("IS NAN!!!\n");
		t=0;
	}

	return t;
}

double signed_square(double x) {
	if(isnan(x)){
		printf("IS NAN!!!\n");
		x=0;
	}

    if (x>0)
        return x*x;
    else
        return -x*x;
}

double swabs(double x) {
	if(isnan(x)){
		printf("IS NAN!!!\n");
		x=0;
	}

    if (x>0)
        return x;
    else
        return -x;
}

double unpack_float(uint8_t packed, double min, double max) {
	if(isnan(packed/255.*(max-min)+min)){
		printf("IS NAN!!!\n");
		return 0;
	}

    return packed/255.*(max-min)+min;
}

// SWParticle
typedef struct {
    double x;   // position
    double v;   // velicity
    double A;   // acceleration coef
    double A2;  // sencond order acc coef
    double B;   // drag coef
    double dt;  // timestep
} SWParticle;

void swparticle_init(SWParticle * p) {
    p->x = p->v = 0;
    p->A = 5;
    p->A2 = .02;
    p->B = 4;
    p->dt = .01;
}

double swparticle_update(SWParticle * p, double raw_x) {
    int i;
    for (i = 0; i < 16; i++) {
    p->v += p->A*(raw_x - p->x)*p->dt;
    p->v += p->A2*signed_square(raw_x - p->x)*p->dt;
    p->v += -p->B*p->v*p->dt;
    p->x += p->v*p->dt;
    }
    return p->x;
}












