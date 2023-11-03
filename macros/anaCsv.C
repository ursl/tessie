#include <fstream>

struct csvEntry {
  TDatime time;
  double  temp, rh, dp, tempW;
  vector<int>    setOn;
  vector<double> setV;
  vector<double> setT;
  vector<double> tempM;
};

bool csvRead(false);


// ----------------------------------------------------------------------
TDatime parseTime(string sline) {
  int year, month, day, hour, min, sec, msec;
  sscanf(sline.c_str(), "%d/%d/%d_%d:%d:%d.%d",
         &year, &month, &day, &hour, &min, &sec, &msec);
  return TDatime(year, month, day, hour, min, sec);
}


// ----------------------------------------------------------------------
vector<csvEntry> readCsv(string filename) {
  bool DBX(false);
  vector<csvEntry> csvLines;

  int icnt(0); 
  
  ifstream INS;
  string sline;
  INS.open(filename);
  while (getline(INS, sline)) {
    ++icnt;
    
    // -- parse CSV line
    std::vector<std::string>   result;
    std::stringstream   lineStream(sline);
    std::string         cell;
    
    while(std::getline(lineStream, cell, ','))  {
      result.push_back(cell);
    }

    // -- fill csvEntry
    csvEntry a;
    a.time  = parseTime(result[0]);
    a.temp  = stod(result[1]);
    a.rh    = stod(result[2]);
    a.dp    = stod(result[3]);
    a.tempW = stod(result[4]);

    if (DBX) cout << "a.time = " << a.time.AsString()
                  << " temp = " << a.temp
                  << " RH = " << a.rh
                  << " dp = " << a.dp
                  << " tempW = " << a.tempW
                  << endl;
    
    int nstart(5);
    for (unsigned int i = nstart; i < nstart+8; ++i) {
      a.setOn.push_back(stoi(result[i]));
      if (DBX) cout << "setOn[" << i - nstart << "] = " << a.setOn[i-nstart] << endl;
    }
    nstart = 13;
    for (unsigned int i = nstart; i < nstart+8; ++i) {
      a.setV.push_back(stod(result[i]));
      if (DBX) cout << "setV[" << i - nstart << "] = " << a.setV[i-nstart] << endl;
    }
    nstart = 21;
    for (unsigned int i = nstart; i < nstart+8; ++i) {
      a.setT.push_back(stod(result[i]));
      if (DBX) cout << "setT[" << i - nstart << "] = " << a.setT[i-nstart] << endl;
    }
    nstart = 29;
    for (unsigned int i = nstart; i < nstart+8; ++i) {
      a.tempM.push_back(stod(result[i]));
      if (DBX) cout << "tempM[" << i - nstart << "] = " << a.tempM[i-nstart] << endl;
    }

    if (DBX) {
      for (unsigned int i = 1; i < result.size(); ++i) {
        cout << result[i] << " "; 
      }
      cout << endl;
      if (2 == icnt) break;
    }

    csvLines.push_back(a);
  }
  
  return csvLines;
}


// ----------------------------------------------------------------------
void plotTempM(string filename = "csv/230120-tessie.csv") {
  vector<csvEntry> csv;
  if (!csvRead) csv = readCsv(filename);

  int nstart(csv[0].time.Convert()),
    nend(csv[csv.size()-1].time.Convert()),
    nbin(nend-nstart+1);
  
  vector<TGraph*> vg;
  for (int it = 0; it < 8; ++it) {
    TGraph *g1;
    vg.push_back(g1 = new TGraph());
    g1->SetName(Form("gtempM%d", it+1));
    
    if (0 == it) g1->SetMarkerColor(kBlue);
    if (1 == it) g1->SetMarkerColor(kBlue+2);
    if (2 == it) g1->SetMarkerColor(kRed);
    if (3 == it) g1->SetMarkerColor(kRed+2);
    if (4 == it) g1->SetMarkerColor(kGreen+1);
    if (5 == it) g1->SetMarkerColor(kGreen+3);
    if (6 == it) g1->SetMarkerColor(kOrange+1);
    if (7 == it) g1->SetMarkerColor(kOrange+3);

    if (0 == it) g1->SetLineColor(kBlue);
    if (1 == it) g1->SetLineColor(kBlue+2);
    if (2 == it) g1->SetLineColor(kRed);
    if (3 == it) g1->SetLineColor(kRed+2);
    if (4 == it) g1->SetLineColor(kGreen+1);
    if (5 == it) g1->SetLineColor(kGreen+3);
    if (6 == it) g1->SetLineColor(kOrange+1);
    if (7 == it) g1->SetLineColor(kOrange+3);
  }

  
  for (unsigned int i = 0; i < csv.size(); ++i) {
    int ntime = csv[i].time.Convert() - nstart;
    cout << csv[i].time.AsString()
         << " convert() = " << ntime
         << " tempM[3] = " << csv[i].tempM[3]
         << endl;
    for (int it = 0; it < 8; ++it) {
      vg[it]->AddPoint(ntime, csv[i].tempM[it]);
    }                   
  }

  TLegend *tleg = newLegend(0.12, 0.2, 0.4, 0.6);
  for (int it = 0; it < 8; ++it) {
    tleg->AddEntry(vg[it], Form("TEC %d", it+1));
    if (0 == it) {
      vg[it]->Draw("alp");
      vg[it]->GetXaxis()->SetTitle("seconds since start");
      vg[it]->GetYaxis()->SetTitle("Temperature [C]");
    } else {
      vg[it]->Draw("lp");
    }
  }
  tleg->SetHeader("V_{ctrl} = 4V");
  tleg->Draw();
  string pdfname = filename;
  replaceAll(pdfname, ".csv", ".pdf");
  c0.SaveAs(pdfname.c_str());
}


// ----------------------------------------------------------------------
void corrTemps(string filename = "/Users/ursl/data/tessie/tessie.csv") {
  // gFile->cd("Detector/Board_0/OpticalGroup_0/Hybrid_0")
  // ((TGraph*)gDirectory->Get("Chip_12/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(12)"))
  // ((TGraph*)gDirectory->Get("Chip_13/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(13)"))
  // ((TGraph*)gDirectory->Get("Chip_14/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(14)"))
  // ((TGraph*)gDirectory->Get("Chip_15/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(15)"))

  shrinkPad(0.1, 0.12);
  
  vector<csvEntry> csv;
  if (!csvRead) csv = readCsv(filename);

  int nstart(csv[0].time.Convert()),
    nend(csv[csv.size()-1].time.Convert()),
    nbin(nend-nstart+1);
  
  vector<TGraph*> vg;
  for (int it = 0; it < 8; ++it) {
    TGraph *g1;
    vg.push_back(g1 = new TGraph());
    g1->SetName(Form("gtempM%d", it+1));
    
    if (0 == it) g1->SetMarkerColor(kBlue);
    if (1 == it) g1->SetMarkerColor(kBlue+3);
    if (2 == it) g1->SetMarkerColor(kRed);
    if (3 == it) g1->SetMarkerColor(kRed+2);
    if (4 == it) g1->SetMarkerColor(kGreen+1);
    if (5 == it) g1->SetMarkerColor(kGreen+3);
    if (6 == it) g1->SetMarkerColor(kOrange+1);
    if (7 == it) g1->SetMarkerColor(kMagenta);

    if (0 == it) g1->SetLineColor(kBlue);
    if (1 == it) g1->SetLineColor(kBlue+3);
    if (2 == it) g1->SetLineColor(kRed);
    if (3 == it) g1->SetLineColor(kRed+2);
    if (4 == it) g1->SetLineColor(kGreen+1);
    if (5 == it) g1->SetLineColor(kGreen+3);
    if (6 == it) g1->SetLineColor(kOrange+1);
    if (7 == it) g1->SetLineColor(kMagenta);
  }

  
  for (unsigned int i = 0; i < csv.size(); ++i) {
    int ntime = csv[i].time.Convert() - nstart;
    int atime = csv[i].time.Convert();
    cout << csv[i].time.AsString()
         << " convert() = " << ntime
         << " tempM[3] = " << csv[i].tempM[3]
         << " absolute time = " << atime
         << endl;
    for (int it = 0; it < 8; ++it) {
      vg[it]->AddPoint(atime, csv[i].tempM[it]);
    }                   
  }

  TLegend *tleg = newLegend(0.12, 0.2, 0.4, 0.7);
  for (int it = 0; it < 8; ++it) {
    tleg->AddEntry(vg[it], Form("TEC %d", it+1));
    if (0 == it) {
      vg[it]->Draw("alp");
      vg[it]->GetXaxis()->SetTitle("time");
      vg[it]->GetYaxis()->SetTitle("Temperature [C]");
    } else {
      vg[it]->Draw("lp");
    }
  }
  
  TFile *as = TFile::Open("/Users/ursl/data/tessie/MonitorDQM_02-11-23_15h00m28.root");
  as->cd("Detector/Board_0/OpticalGroup_0/Hybrid_0");
  TGraph *chip13 = ((TGraph*)gDirectory->Get("Chip_13/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(13)"));
  chip13->SetMarkerColor(kRed);  chip13->SetLineColor(kRed);
  chip13->Draw("pl");
  tleg->AddEntry(chip13, "Chip 13");

  TGraph *chip14 = ((TGraph*)gDirectory->Get("Chip_14/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(14)"));
  chip14->SetMarkerColor(kBlue);  chip14->SetLineColor(kBlue);
  chip14->Draw("pl");
  tleg->AddEntry(chip14, "Chip 14");

  TGraph *chip15 = ((TGraph*)gDirectory->Get("Chip_15/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(15)"));
  chip15->SetMarkerColor(kCyan);  chip15->SetLineColor(kCyan);
  chip15->Draw("pl");
  tleg->AddEntry(chip15, "Chip 15");

  
  tleg->SetHeader("2023/11/02");
  tleg->Draw();
  vg[0]->GetXaxis()->SetTimeDisplay(1);
  vg[0]->GetXaxis()->SetLimits(1698932500., 1698936696.);
  vg[0]->SetMaximum(15.);
  vg[0]->SetMinimum(-50.);
  
  tl->SetTextSize(0.03);
  tl->DrawLatex(0.77, 0.91, "AS/NP/UL");

  string pdfname = filename;
  replaceAll(pdfname, ".csv", ".pdf");
  c0.SaveAs(pdfname.c_str());

  TFile *fOut = TFile::Open("tessie.root", "RECREATE");
  for (int it = 0; it < 8; ++it) {
    fOut->Append(vg[it]);
  }
  as->Remove(chip13); fOut->Append(chip13); 
  as->Remove(chip14); fOut->Append(chip14);
  as->Remove(chip15); fOut->Append(chip15);

  fOut->Write();
  fOut->Close();
  
}
