#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>

#include "HRGBase.h"
#include "HRGEV.h"
#include "HRGFit.h"

#include "ThermalFISTConfig.h"

using namespace std;

#ifdef ThermalFIST_USENAMESPACE
using namespace thermalfist;
#endif

// Thermal fits to ALICE Pb-Pb 2.76 TeV data at different centralities
// within the thermal model using zero-width, BW, and eBW schemes 
// for finite resonance widths modeling
int main(int argc, char *argv[])
{
	// Input list of hadrons
	string listname = string(INPUT_FOLDER) + "/list/PDG2014/list-withnuclei.dat";

	// Hadron list objects for all three scenarios considered
	ThermalParticleSystem parts(listname);    // Zero-width
	ThermalParticleSystem partsBW(listname);  // BW
	ThermalParticleSystem partseBW(listname); // eBW


	ThermalModelParameters params;
	// Chemical potentials are fixed to zero
	params.muB = 0.0;
	params.muQ = 0.0;
	params.muS = 0.0;
	params.muC = 0.0;

	// Initial temperature value in fits
	params.T   = 0.155;

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


	// Different data to be fitted, 
	vector<string> datafilenames;
	// Names for output
	vector<string> outputnames;

	// 0-10%, including light nuclei
	datafilenames.push_back(string(INPUT_FOLDER) + "/../../input/data/ALICE-PbPb2.76TeV-0-10-all-symmetrized-nuclei.dat");
	outputnames.push_back("0-10-with-nuclei");

	// 0-10%, no light nuclei
	datafilenames.push_back(string(INPUT_FOLDER) + "/../../input/data/ALICE-PbPb2.76TeV-0-10-all-symmetrized.dat");
	outputnames.push_back("0-10");

	// 10-20%, no light nuclei
	datafilenames.push_back(string(INPUT_FOLDER) + "/../../input/data/ALICE-PbPb2.76TeV-10-20-all-symmetrized.dat");
	outputnames.push_back("10-20");

	// 20-40%, no light nuclei
	datafilenames.push_back(string(INPUT_FOLDER) + "/../../input/data/ALICE-PbPb2.76TeV-20-40-all-symmetrized.dat");
	outputnames.push_back("20-40");

	// 40-60%, no light nuclei
	datafilenames.push_back(string(INPUT_FOLDER) + "/../../input/data/ALICE-PbPb2.76TeV-40-60-all-symmetrized.dat");
	outputnames.push_back("40-60");

	// 60-80%, no light nuclei
	datafilenames.push_back(string(INPUT_FOLDER) + "/../../input/data/ALICE-PbPb2.76TeV-60-80-all-symmetrized.dat");
	outputnames.push_back("60-80");


	// Loop over all data to be fitted
	for (int ind = 0; ind < datafilenames.size(); ++ind) {
		// Load the data from file
		vector<FittedQuantity> data = ThermalModelFit::loadExpDataFromFile(datafilenames[ind]);

		// Fit within zero-width scheme
		ThermalModelFit fitter(model);

		// Do not fit muB, it is fixed to zero
		// For other fit parameters default settings are fine
		fitter.SetParameterFitFlag("muB", false); 

		// Set the data to be fitted
		fitter.SetQuantities(data);

		printf("Fitting the %s data using the zero-width scheme\n", outputnames[ind].c_str());

		// Performs the fit, the parameter suppresses the intermediate extra output
		fitter.PerformFit(false);

		// Print the fit result on screen
		fitter.PrintFitLog(); 

		// Now print the results to file
		fitter.PrintYieldsTable(std::string(outputnames[ind] + ".ZW.out"));
		std::string comment = "Thermal fit to " + datafilenames[ind] + " within " + fitter.model()->TAG();
		fitter.PrintFitLog(std::string(outputnames[ind] + ".ZW.txt"), comment);

		printf("\n--------------------------------------------------------\n");


		// Same procedure using the BW scheme
		fitter = ThermalModelFit(modelBW);
		fitter.SetParameterFitFlag("muB", false);
		fitter.SetQuantities(data);
		printf("Fitting the %s data using the BW scheme\n", outputnames[ind].c_str());
		fitter.PerformFit(false);
		fitter.PrintFitLog();
		fitter.PrintYieldsTable(std::string(outputnames[ind] + ".BW.out"));
		comment = "Thermal fit to " + datafilenames[ind] + " within " + fitter.model()->TAG();
		fitter.PrintFitLog(std::string(outputnames[ind] + ".BW.txt"), comment);
		printf("\n--------------------------------------------------------\n");

		// Same procedure using the eBW scheme
		fitter = ThermalModelFit(modeleBW);
		fitter.SetParameterFitFlag("muB", false);
		fitter.SetQuantities(data);
		printf("Fitting the %s data using the eBW scheme\n", outputnames[ind].c_str());
		fitter.PerformFit(false);
		fitter.PrintFitLog();
		fitter.PrintYieldsTable(std::string(outputnames[ind] + ".eBW.out"));
		comment = "Thermal fit to " + datafilenames[ind] + " within " + fitter.model()->TAG();
		fitter.PrintFitLog(std::string(outputnames[ind] + ".eBW.txt"), comment);
		printf("\n--------------------------------------------------------\n");
	}

	// Memory clean-up
	delete model;
	delete modelBW;
	delete modeleBW;

	return 0;
}
