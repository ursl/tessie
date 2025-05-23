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
void plotCold(string filename = "csv/250115-coldbox01-tessie.csv") {
  vector<csvEntry> csv;
  if (!csvRead) csv = readCsv(filename);
  
  int nstart(csv[0].time.Convert()),
    nend(csv[csv.size()-1].time.Convert()),
    nbin(nend-nstart+1);
  
  TGraph *g1;
  g1 = new TGraph();
  g1->SetName("tempW");
  g1->SetMarkerColor(kBlue);
  
  for (unsigned int i = 0; i < csv.size(); ++i) {
    int ntime = csv[i].time.Convert() - nstart;
    int atime = csv[i].time.Convert();
    if (atime < 1729.68e6) continue;
    if (atime > 1729.72e6) break;
    if (0) cout << csv[i].time.AsString()
                << " convert() = " << ntime
                << " tempW = " << csv[i].tempW
                << " absolute time = " << atime
                << endl;
    g1->AddPoint(atime, csv[i].tempW);
  }

  g1->Draw("alp");

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
      vg[it]->GetYaxis()->SetTitle("Temperature [#circC]");
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
void corrTemps(string filename = "/Users/ursl/data/tessie/231103-tessie.csv") {
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

  TLegend *tleg = newLegend(0.15, 0.15, 0.4, 0.5);
  for (int it = 0; it < 8; ++it) {
    TLegendEntry *tle = tleg->AddEntry(vg[it], Form("TEC %d", it+1));
    tle->SetTextSize(0.03);
    if (0 == it) {
      vg[it]->Draw("alp");
      vg[it]->GetXaxis()->SetTitle("time");
      vg[it]->GetYaxis()->SetTitle("Temperature [#circC]");
    } else {
      vg[it]->Draw("lp");
    }
  }
  
  //  TFile *as = TFile::Open("/Users/ursl/data/tessie/MonitorDQM_02-11-23_15h00m28.root");
  TFile *as = TFile::Open("/Users/ursl/data/tessie/MonitorDQM_03-11-23_10h59m34.root");
  as->cd("Detector/Board_0/OpticalGroup_0/Hybrid_0");
  TGraph *chip13 = ((TGraph*)gDirectory->Get("Chip_13/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(13)"));
  chip13->SetMarkerColor(kRed);  chip13->SetLineColor(kRed);
  chip13->Draw("pl");
  TLegendEntry *tle = tleg->AddEntry(chip13, "Chip 13");
  tle->SetTextSize(0.03);

  TGraph *chip14 = ((TGraph*)gDirectory->Get("Chip_14/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(14)"));
  chip14->SetMarkerColor(kBlue);  chip14->SetLineColor(kBlue);
  chip14->Draw("pl");
  tle = tleg->AddEntry(chip14, "Chip 14");
  tle->SetTextSize(0.03);

  TGraph *chip15 = ((TGraph*)gDirectory->Get("Chip_15/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(15)"));
  chip15->SetMarkerColor(kCyan);  chip15->SetLineColor(kCyan);
  chip15->Draw("pl");
  tle = tleg->AddEntry(chip15, "Chip 15");  
  tle->SetTextSize(0.03);
  
  tleg->SetHeader("2023/11/03");
  tleg->Draw();
  vg[0]->GetXaxis()->SetTimeDisplay(1);
  vg[0]->GetXaxis()->SetLimits(1699005400, 1699008500.);
  vg[0]->SetMaximum(40.);
  vg[0]->SetMinimum(-48.);
  
  tl->SetTextSize(0.03);
  tl->DrawLatex(0.80, 0.91, "NP/UL");

  string pdfname = "temperatures-" + filename.substr(filename.rfind("/")+1);
  replaceAll(pdfname, ".csv", ".pdf");
  c0.SaveAs(pdfname.c_str());

  TGraph *gcorr = new TGraph();
  
  int ix, tx, ti;
  for (int i = 0; i < chip15->GetN(); ++i) {
    ix = chip15->GetPointX(i);

    for (int t = 0; t < vg[7]->GetN(); ++t) {
      tx = vg[7]->GetPointX(t);
      if (tx == ix) {
        ti = t;
        break;
      }
    }
    
    double iy = chip15->GetPointY(i);
    double ty = vg[7]->GetPointY(ti);
    gcorr->AddPoint(ty, iy);
    //  m->SetMarkerColor(i);;
    cout << ix << "/" << tx << " date: " << ":  iy = " << iy << " ty = " << ty << endl;
  }

  c0.Clear();
  gcorr->SetMarkerStyle(24);
  gcorr->SetMarkerSize(0.6);
  gcorr->Draw("ap");
  gcorr->GetXaxis()->SetTitle("TEC8 temperature [#circC]");
  gcorr->GetYaxis()->SetTitle("chip15 temperature [#circC]");

  if (0) {
    double x, y;
    TMarker *m(0); 
    for (int i = 1; i < gcorr->GetN(); ++i) {
      gcorr->GetPoint(i, x, y);
      m = new TMarker(x, y, 24);
      cout << "Marker at " << x << "/" << y << endl;
      if (i < 100) {
        m->SetMarkerColor(kRed);
      } else if (i < 200) {
        m->SetMarkerColor(kBlue);
      } else {
        m->SetMarkerColor(kGreen);
      }
      m->SetMarkerSize(1);
      //    m->DrawMarker(x, y);
      m->Draw();
    }
  }

  string pdfname2 = "corr-" + filename.substr(filename.rfind("/")+1);
  replaceAll(pdfname2, ".csv", ".pdf");
  c0.SaveAs(pdfname2.c_str());


  
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


// ----------------------------------------------------------------------
void corrTempM2TempHDI(string filename = "/Users/ursl/data/tessie/231103b-tessie.csv") {

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

  TFile *md0 = TFile::Open("/Users/ursl/data/tessie/MonitorDQM_03-11-23_14h46m45.root");
  md0->cd("Detector/Board_0/OpticalGroup_0/Hybrid_0");
  TGraph *chip14 = ((TGraph*)gDirectory->Get("Chip_14/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(14)"));
  for (int i = 0; i < chip14->GetN(); ++i) {
    double iy = chip14->GetPointY(i);
    if (iy < -40.) chip14->SetPointY(i, chip14->GetPointY(i-1));
  }
  chip14->SetMarkerColor(kBlack);  chip14->SetLineColor(kBlack); chip14->SetLineWidth(2);
  chip14->Draw("p");

  TGraph *chip15 = ((TGraph*)gDirectory->Get("Chip_15/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(15)"));
  for (int i = 0; i < chip15->GetN(); ++i) {
    double iy = chip15->GetPointY(i);
    if (iy < -40.) chip15->SetPointY(i, chip15->GetPointY(i-1));
  }
  chip15->SetMarkerColor(kGray+1);  chip15->SetLineColor(kGray+1); chip15->SetLineWidth(2);
  chip15->Draw("p");
  
  TFile *md1 = TFile::Open("/Users/ursl/data/tessie/MonitorDQM_03-11-23_14h56m28.root");
  md1->cd("Detector/Board_0/OpticalGroup_0/Hybrid_0");
  TGraph *chip14a = ((TGraph*)gDirectory->Get("Chip_14/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(14)"));
  chip14a->SetMarkerColor(kBlack);  chip14a->SetLineColor(kBlack); chip14a->SetLineWidth(2);
  chip14a->Draw("p");
  TGraph *chip15a = ((TGraph*)gDirectory->Get("Chip_15/D_B(0)_O(0)_H(0)_DQM_INTERNAL_NTC_Chip(15)"));
  chip15a->SetMarkerColor(kGray+1);  chip15a->SetLineColor(kGray+1); chip15a->SetLineWidth(2);
  chip15a->Draw("p");
  
  TLegend *tleg = newLegend(0.15, 0.12, 0.4, 0.47);
  for (int it = 0; it < 8; ++it) {
    TLegendEntry *tle(0);
    if (it == 0) {
      tle = tleg->AddEntry(vg[it], Form("HDI PT1000"));
    } else if (it == 7) {
      tle = tleg->AddEntry(vg[it], Form("TEC %d", it+1));
    } else {
      tle = tleg->AddEntry(vg[it], Form("TEC %d (empty)", it+1));
    }
    tle->SetTextSize(0.03);
    if (0 == it) {
      vg[it]->Draw("alp");
      vg[it]->GetXaxis()->SetTitle("time");
      vg[it]->GetYaxis()->SetTitle("Temperature [#circC]");
    } else {
      vg[it]->Draw("lp");
    }
  }
  
  TLegendEntry *tle = tleg->AddEntry(chip14, "Chip 14 (HDI r/o)");
  tle->SetTextSize(0.03);
  tle = tleg->AddEntry(chip15, "Chip 15 (HDI r/o)");
  tle->SetTextSize(0.03);
  
  tleg->SetHeader("2023/11/03");
  tleg->Draw();
  vg[0]->GetXaxis()->SetTimeDisplay(1);
  vg[0]->GetXaxis()->SetLimits(1699018796, 1699022316);
  vg[0]->SetMaximum(40.);
  vg[0]->SetMinimum(-48.);
  
  pl->SetLineColor(kBlack);
  pl->SetLineWidth(2);
  pl->SetLineStyle(kDashed);
  pl->DrawLine(1699018796, -35., 1699022316., -35.);

  tl->SetTextSize(0.03);
  tl->DrawLatex(0.80, 0.91, "NP/UL");

  chip14->Draw("pl");
  chip14a->Draw("pl");
  chip15->Draw("pl");
  chip15a->Draw("pl");
  
  string pdfname = "temperaturesHDI-" + filename.substr(filename.rfind("/")+1);
  replaceAll(pdfname, ".csv", ".pdf");
  c0.SaveAs(pdfname.c_str());


  TGraph *gcorr = new TGraph();
  
  int ix, tx, ti;
  for (int i = 0; i < vg[0]->GetN(); ++i) {
    ix = vg[0]->GetPointX(i);

    for (int t = 0; t < vg[7]->GetN(); ++t) {
      tx = vg[7]->GetPointX(t);
      if (tx == ix) {
        ti = t;
        break;
      }
    }
    
    double iy = vg[0]->GetPointY(i);
    double ty = vg[7]->GetPointY(ti);
    gcorr->AddPoint(iy, ty);
    //  m->SetMarkerColor(i);;
    cout << ix << "/" << tx << " date: " << ":  iy = " << iy << " ty = " << ty << endl;
  }

  c0.Clear();
  gcorr->SetMarkerStyle(24);
  gcorr->SetMarkerSize(0.6);
  gcorr->Draw("ap");
  gcorr->GetXaxis()->SetLimits(-38., 32.);
  gcorr->GetYaxis()->SetLimits(-40., 25.);
  gcorr->GetXaxis()->SetTitle("HDI temperature [#circC]");
  gcorr->GetYaxis()->SetTitle("TEC8 temperature [#circC]");

  tl->SetTextSize(0.03);
  tl->DrawLatex(0.80, 0.91, "NP/UL");

  pl->SetLineWidth(2);
  pl->SetLineStyle(kDashed);
  pl->SetLineColor(kRed);
  pl->DrawLine(-38., -38., 25.8, 25.8);
  
  if (0) {
    double x, y;
    TMarker *m(0); 
    for (int i = 1; i < gcorr->GetN(); ++i) {
      gcorr->GetPoint(i, x, y);
      m = new TMarker(x, y, 24);
      cout << "Marker at " << x << "/" << y << endl;
      if (i < 100) {
        m->SetMarkerColor(kRed);
      } else if (i < 200) {
        m->SetMarkerColor(kBlue);
      } else {
        m->SetMarkerColor(kGreen);
      }
      m->SetMarkerSize(1);
      //    m->DrawMarker(x, y);
      m->Draw();
    }
  }

  string pdfname2 = "corrHDI-" + filename.substr(filename.rfind("/")+1);
  replaceAll(pdfname2, ".csv", ".pdf");
  c0.SaveAs(pdfname2.c_str());

}


// ----------------------------------------------------------------------
void plotMeltDown(string filename = "csv/240822-tessie.csv", int zoomStart = -1) {
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

  TGraph *gWater = new TGraph(); gWater->SetName("gTempWater"); gWater->SetMarkerStyle(24); gWater->SetMarkerSize(0.5);
  TGraph *gTecOn = new TGraph(); gTecOn->SetName("gTecOn");

  
  for (unsigned int i = 0; i < csv.size(); ++i) {
    int ntime = csv[i].time.Convert() - nstart;
    cout << csv[i].time.AsString()
         << " convert() = " << ntime
         << " tempM[3] = " << csv[i].tempM[3]
         << endl;
    if (ntime > zoomStart) {
      for (int it = 0; it < 8; ++it) {
        vg[it]->AddPoint(ntime, csv[i].tempM[it]);
      }                   
      
      gWater->AddPoint(ntime, csv[i].tempW);
      gTecOn->AddPoint(ntime, 40. + 20.*csv[i].setOn[0]);
    }
  }

  shrinkPad(0.1, 0.12);

  TLegend *tleg = newLegend(0.2, 0.12, 0.35, 0.40);
  tleg->SetTextSize(0.03);
  tleg->SetFillStyle(1000);
  gTecOn->Draw("alp");
  gTecOn->SetMinimum(-25.);
  gTecOn->SetMaximum(80.);
  gTecOn->GetXaxis()->SetTitle(Form("seconds after %s", csv[0].time.AsString()));
  gTecOn->GetYaxis()->SetTitle("Temperature [#circC]");
  for (int it = 0; it < 8; ++it) {
    tleg->AddEntry(vg[it], Form("TEC %d", it+1));
    if (0 == it) {
      vg[it]->Draw("l");
    } else {
      vg[it]->Draw("l");
    }
  }

  gWater->Draw("p");
  gTecOn->Draw("lp");
  tleg->AddEntry(gWater, "T(water)");

  if (zoomStart < 0) tl->DrawLatex(0.2, 0.62, "TEC power");
  
  string header(filename);
  replaceAll(header, "csv/", "");             
  replaceAll(header, "-tessie.csv", "");             
  tleg->SetHeader(header.c_str());
  tleg->Draw();
  string pdfname = filename;

  if (zoomStart > 0) {
    replaceAll(pdfname, "-tessie", Form("-tessie-%d", zoomStart));
  }

  replaceAll(pdfname, ".csv", ".pdf");
  c0.SaveAs(pdfname.c_str());
}


// ----------------------------------------------------------------------
void plotAllMeltdowns() {
  plotMeltDown("csv/240821-tessie.csv");
  plotMeltDown("csv/240821-tessie.csv", 4200);
  plotMeltDown("csv/240822-tessie.csv");
  plotMeltDown("csv/240822-tessie.csv", 2800);
}
