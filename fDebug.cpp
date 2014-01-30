//Fintan's C debugging suite

#include <iostream>
#include <fstream>
using namespace std;


void fdWriteFile(){
	cout << "Writing test file" <<endl;
	ofstream myfile;
	myfile.open ("fdTestFile.txt");
	myfile << "Test file!\n";
	myfile.close();
}

void fdInit(){
	cout << "Debugging on line." << endl;
}