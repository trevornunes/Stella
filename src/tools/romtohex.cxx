/**
  Simple program that produces a hex list of a binary object file

  @author  Bradford W. Mott
  @version $Id: romtohex.cxx 1908 2009-11-22 02:32:20Z stephena $
*/

#include <iomanip>
#include <fstream>
#include <iostream>
using namespace std;

int main(int ac, char* av[])
{
  ifstream in;
  in.open("scrom.bin");
  if(in.is_open())
  {
    in.seekg(0, ios::end);
    int len = (int)in.tellg();
    in.seekg(0, ios::beg);

    unsigned char* data = new unsigned char[len];
    in.read((char*)data, len);
    in.close();

    cout << "SIZE = " << (len - 2) << endl << "  ";

    // Skip first two bytes; they shouldn't be used
    for(int t = 2; t < len; ++t)
    {
      cout << "0x" << setw(2) << setfill('0') << hex << (int)data[t];
      if(t < len - 1)
        cout << ", ";
      if(((t-2) % 8) == 7)
        cout << endl << "  ";
    }
    cout << endl;
    delete[] data;
  }
}
