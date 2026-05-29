// Usage inside ROOT:
//   root -l 'analysis/plot_results.C("gamma_6MeV.root")'

#include <iomanip>
#include <iostream>
#include <map>
#include <string>

void plot_results(const char* fileName = "therapy_output.root")
{
  auto* file = TFile::Open(fileName, "READ");
  if (!file || file->IsZombie()) {
    std::cerr << "Cannot open " << fileName << std::endl;
    return;
  }

  auto* tree = dynamic_cast<TTree*>(file->Get("events"));
  if (!tree) {
    std::cerr << "Cannot find TTree 'events' in " << fileName << std::endl;
    return;
  }

  int cell_id = 0;
  double dose_Gy = 0.0;
  double lineal = 0.0;
  tree->SetBranchAddress("cell_id", &cell_id);
  tree->SetBranchAddress("dose_Gy", &dose_Gy);
  tree->SetBranchAddress("lineal_keV_um", &lineal);

  std::map<int, double> doseSum;
  std::map<int, double> linealSum;
  std::map<int, long long> counts;

  const auto entries = tree->GetEntries();
  for (Long64_t i = 0; i < entries; ++i) {
    tree->GetEntry(i);
    doseSum[cell_id] += dose_Gy;
    linealSum[cell_id] += lineal;
    counts[cell_id] += 1;
  }

  const std::map<int, std::string> names = {
      {0, "tumor cell"},
      {1, "normal cell in tumor region"},
      {2, "healthy normal cell"},
      {3, "weighted tumor region"}};

  std::cout << "\nSummary for " << fileName << "\n";
  std::cout << std::left << std::setw(32) << "cell score"
            << std::right << std::setw(18) << "mean dose Gy"
            << std::setw(22) << "mean lineal keV/um" << "\n";
  for (const auto& [id, name] : names) {
    if (counts[id] == 0) continue;
    std::cout << std::left << std::setw(32) << name
              << std::right << std::setw(18) << doseSum[id] / counts[id]
              << std::setw(22) << linealSum[id] / counts[id] << "\n";
  }

  auto* c1 = new TCanvas("dose_canvas", "Dose spectra", 1000, 700);
  c1->Divide(2, 2);
  const char* histNames[] = {
      "tumor_cell_dose",
      "normal_in_region_dose",
      "healthy_normal_dose",
      "tumor_region_mixed_dose"};
  for (int i = 0; i < 4; ++i) {
    c1->cd(i + 1);
    auto* h = dynamic_cast<TH1*>(file->Get(histNames[i]));
    if (h) h->Draw("hist");
  }
  c1->SaveAs((std::string(fileName) + "_dose.png").c_str());

  auto* c2 = new TCanvas("let_canvas", "LET proxy", 800, 600);
  auto* hLet = dynamic_cast<TH1*>(file->Get("step_let"));
  if (hLet) hLet->Draw("hist");
  c2->SaveAs((std::string(fileName) + "_let.png").c_str());
}
