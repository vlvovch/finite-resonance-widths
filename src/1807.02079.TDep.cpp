#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>

#include "HRGBase.h"
#include "HRGEV.h"
#include "HRGFit.h"
#include "HRGVDW/ThermalModelVDWFull.h"

#include "ThermalFISTConfig.h"

using namespace std;

#ifdef ThermalFIST_USENAMESPACE
using namespace thermalfist;
#endif

//  Timing
//  Windows
#ifdef _WIN32
#include <Windows.h>
double get_wall_time(){
    LARGE_INTEGER time,freq;
    if (!QueryPerformanceFrequency(&freq)){
        //  Handle error
        return 0;
    }
    if (!QueryPerformanceCounter(&time)){
        //  Handle error
        return 0;
    }
    return (double)time.QuadPart / freq.QuadPart;
}
double get_cpu_time(){
    FILETIME a,b,c,d;
    if (GetProcessTimes(GetCurrentProcess(),&a,&b,&c,&d) != 0){
        //  Returns total user time.
        //  Can be tweaked to include kernel times as well.
        return
            (double)(d.dwLowDateTime |
            ((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
    }else{
        //  Handle error
        return 0;
    }
}

//  Posix/Linux
#else
#include <time.h>
#include <sys/time.h>
double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}
#endif


// Temperature dependence of hadron yield ratios calculated in BW or eBW scheme relative to
// the zero width approximation, at \mu_B = 0
int main(int argc, char *argv[])
{
	// Input list of hadrons
	string listname = string(INPUT_FOLDER) + "/list/PDG2014/list-withnuclei.dat";
	
	// Hadron list objects for all three scenarios considered
	ThermalParticleSystem parts(listname);    // Zero-width
	ThermalParticleSystem partsBW(listname);  // BW
	ThermalParticleSystem partseBW(listname); // eBW

	
	ThermalModelParameters params;
	// Zero chemical potentials
	params.muB = 0.0;
	params.muQ = 0.0;
	params.muS = 0.0;
	params.muC = 0.0;
	params.V = 5000.; // Exact value doesn't matter, cancels out in ratios
	
	// Three ThermalModel instances for the three scenarios
	ThermalModelBase *model, *modelBW, *modeleBW;

	// Zero-width
	model = new ThermalModelIdeal(&parts);

	model->SetParameters(params); 
	model->SetStatistics(true);    // Include quantum statistics
	model->SetUseWidth(ThermalParticle::ZeroWidth);
	model->FillChemicalPotentials();

	// Energy-independent Breit-Wigner (BW scheme)
	modelBW = new ThermalModelIdeal(&partsBW);

	modelBW->SetParameters(params);
	modelBW->SetStatistics(true);  // Include quantum statistics
	modelBW->SetUseWidth(ThermalParticle::BWTwoGamma);
	modelBW->FillChemicalPotentials();

	// Energy-dependent Breit-Wigner (eBW scheme)
	modeleBW = new ThermalModelIdeal(&partseBW);

	modeleBW->SetParameters(params);
	modeleBW->SetStatistics(true);  // Include quantum statistics
	modeleBW->SetUseWidth(ThermalParticle::eBW);
	modeleBW->FillChemicalPotentials();

	// Names and pdgids of all hadron species analyzed
	vector<string> names;
	vector<int> pdgs;
	
	// pi+ (pions)
	names.push_back("pi");
	pdgs.push_back(211);

	// K+ (kaons)
	names.push_back("K");
	pdgs.push_back(321);

	// K0S
	names.push_back("K0S");
	pdgs.push_back(310);

	// phi
	names.push_back("phi");
	pdgs.push_back(333);

	// (anti)protons
	names.push_back("p");
	pdgs.push_back(2212);

	// (anti)Lambdas
	names.push_back("Lambda");
	pdgs.push_back(3122);
	
	// (anti)Ksis
	names.push_back("Ksi-");
	pdgs.push_back(3312);

	// (anti)Omegas
	names.push_back("Omega");
	pdgs.push_back(3334);


	// Preparing output

	// First on-screen
	printf("%15s",
		"T[MeV]"
	);

	for (int i = 0; i < names.size(); ++i) {
		printf("%15s", (names[i] + "_BW").c_str());
	}

	for (int i = 0; i < names.size(); ++i) {
		printf("%15s", (names[i] + "_eBW").c_str());
	}

	printf("\n");

	// Same output to file
	FILE *f = fopen("ZeroWidth-vs-BW-vs-eBW-vs-T.dat", "w");

	fprintf(f, "%15s",
		"T[MeV]"
	);

	for (int i = 0; i < names.size(); ++i) {
		fprintf(f, "%15s", (names[i] + "_BW").c_str());
	}

	for (int i = 0; i < names.size(); ++i) {
		fprintf(f, "%15s", (names[i] + "_eBW").c_str());
	}

	fprintf(f, "\n");

	// Output to file the temperature dependence of the p/pi ratio in the three schemes
	FILE *f2 = fopen("Widths-p-to-pi-vs-T.dat", "w");

	fprintf(f2, "%15s%15s%15s%15s\n",
		"T[MeV]",
		"p/pi_zw",
		"p/pi_BW",
		"p/pi_eBW"
	);

	double wt1 = get_wall_time();
	int iters = 0;

	// Temperature range (in GeV)
	double Tmin = 0.020;
	double Tmax = 0.2001;
	double dT   = 0.001;

	for (double T = Tmin; T <= Tmax; T += dT) {
		// Do the calculations of the densities at given temperatures for all three cases
		model->SetTemperature(T);
		model->CalculateDensities();

		modelBW->SetTemperature(T);
		modelBW->CalculateDensities();

		modeleBW->SetTemperature(T);
		modeleBW->CalculateDensities();

		// Output temperature in MeV
		printf("%15lf", T * 1000.);

		// Output BW to zero-width ratios for all yields considered
		for (int i = 0; i < names.size(); ++i) {
			printf("%15lf", modelBW->GetDensity(pdgs[i], 1) / model->GetDensity(pdgs[i], 1));
		}

		// Output eBW to zero-width ratios for all yields considered
		for (int i = 0; i < names.size(); ++i) {
			printf("%15lf", modeleBW->GetDensity(pdgs[i], 1) / model->GetDensity(pdgs[i], 1));
		}

		printf("\n");


		// Same output as above, but into file
		fprintf(f, "%15lf", T * 1000.);

		for (int i = 0; i < names.size(); ++i) {
			fprintf(f, "%15lf", modelBW->GetDensity(pdgs[i], 1) / model->GetDensity(pdgs[i], 1));
		}

		for (int i = 0; i < names.size(); ++i) {
			fprintf(f, "%15lf", modeleBW->GetDensity(pdgs[i], 1) / model->GetDensity(pdgs[i], 1));
		}

		fprintf(f, "\n");

		fflush(f);


		// Output into file the p/pi+ ratios for zero-width, BW, and eBW schemes
		fprintf(f2, "%15lf%15E%15E%15E\n", T * 1000.,
			model->GetDensity(2212, 1) / model->GetDensity(211, 1),
			modelBW->GetDensity(2212, 1) / modelBW->GetDensity(211, 1),
			modeleBW->GetDensity(2212, 1) / modeleBW->GetDensity(211, 1));

		fflush(f2);

		iters++;

	}

	// Close the output files
	fclose(f);
	fclose(f2);


	// Memory clean-up
	delete model;
	delete modelBW;
	delete modeleBW;

	
	// Timing
	double wt2 = get_wall_time();

	printf("%40s %lf s\n", "Running time:", (wt2 - wt1));
	printf("%40s %lf s\n", "Time per single temperature value:", (wt2 - wt1) / iters);

	return 0;
}
