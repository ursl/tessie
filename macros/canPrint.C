void canPrint(string fourBytes) {
  char word[4];
  
  string sdata;
  vector<string> bytes = split(fourBytes, ' ');
  if (bytes.size() != 4) {
    cout << "wrong number of bytes: " << bytes.size() << endl;
    return;
  }
  for (int i = 3; i > -1; --i) {
    sdata += bytes[i];
  }
  
  int iVal(0);
  float fVal(0.0);
  std::stringstream ssI, ssF;

  ssI << std::hex << sdata;
  ssF << std::hex << sdata;

  ssI >> iVal;
  //  ssF >> fVal;
  //  fVal = static_cast<float>(iVal);

  //  memcpy(&iVal, word, sizeof iVal);

  memcpy(&fVal, &iVal, sizeof fVal);
  
  cout << "hex:   " << std::hex << iVal << " " << std::dec << endl;
  cout << "int:   " << iVal << endl;
  cout << "float: " << fVal << "" << endl;
  
}
