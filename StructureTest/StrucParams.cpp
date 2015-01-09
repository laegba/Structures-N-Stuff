/*

    StrucParams.cpp reads and parses structures file  to set up calc
    Copyright (C) 2015  Michael Salay,      Mike.Salay at gmail

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Keywords "str","typ","pos","spr","anc","2D","3D"
      2D and 3D handled directly rather than as function
    Will add for, fot, and ant (force, time-dependent force, time-dependent anchor)
    
    Input file format specified in README file.

    Do input checking and exception catching if time.
    
    
    The input reading part should be made a separate class:
       Options should be:
         - read entire file as char vector, parsing by delimiters
           - EDF format?
         - read entire file as string vector
         - read entire file as a vector of string vectors (HOW THIS WORKS)
         - read each line in a file file sequentially as a string vector (for big files)
            Would have to transfer keyword list as argument so that it would know which
            lines to return
 */
 

#include "StrucParams.h"

using namespace std;
const float PI=2*acos(0); // define PI
const float deg2rad= PI/180; // conversion factor from degrees t


/* Structure Input Processsor */
StrucParams::StrucParams(string filePath)
{

  /* Set Defaults */
  Dimension=2; Size=3; j2=1; k1=2; k2=2; // j1 is always 0
  pos0.resize(6,0.);// Reference position and velocity
  Spring=50.;
  Damp=1.;
  
//  cout << "Processed Keyword Lines: " << endl;

   
  /* Read Input to Vector */
  ifstream file(filePath);
  while (!file.eof())
  {
    getline(file,lineText);
    stringstream ss(lineText);
    while (ss >> inp) line.push_back(inp);
    input.push_back(line);
    if (line.size() > 0) parseline();   
    line.clear();
  }
  input.pop_back(); // remove extra entry
  
  if (Dimension == 3){ 
    Size=6;
    j2=2; 
    k1=3; 
    k2=5;
    pos0.resize(12,0.);
  }
  
//  displayinput();      // Display raw input to test
  input.clear();         // Clear Input String Array
  calcStrucPositions();  // Position parts
//  Summary();             // Print Input Summary 
}



/* Supporting Functions*/


void StrucParams::parseline()
{ 
  isKey="-";

  if (line[0] == "str") {
    str();
  } else if (line[0] == "typ") {
    typ();
  } else if (line[0] == "pos") {
    pos();
  } else if (line[0] == "spr") {
    spr();
  } else if (line[0] == "anc") {
    anc();
  } else if (line[0] == "2D") {
    isKey="2D";
    Dimension=2;
  } else if (line[0] == "3D") {
    isKey="3D";
    Dimension=3;
  }
//  if (isKey.compare("-") != 0) printline();
}


void StrucParams::printline()
{ 
  cout << isKey << ": ";
   for (auto d : line)
    {
      cout << d << ' ';
    } 
    cout << endl;  // end line on display 
}


void StrucParams::displayinput()
{
  cout << endl;
  cout << "#########   INPUT ECHO FOLLOWS   #########" << endl;
  for (auto c : input)
  {
    for (auto d : c)
    {
      cout << d << ' ';
    } 
    cout << endl;  // to split lines
  }
  cout << endl;
}


void StrucParams::Summary()
{ 
  /* Struc Input Summary */
  cout << endl << endl;
  cout << "###  Structures Summary   ###" << endl;
  cout << endl;
  cout << "  Dimension: " << Dimension << "D, Position Array Size: " << Size << endl;
  cout << endl;
  cout << "  POS reference position and velocity summary:" << endl;
  cout << "   ";
  for (auto c : pos0)
      cout << " " << c;
  cout << endl << endl;
  
  cout << "  SPR spring parameters summary:" << endl;
  cout << "    " << Spring << " " << Damp;
      
  cout << endl << endl;
  
  cout << "  TYP structure type summary:" << endl;
  for (auto c : type)
  {
    cout << "   ";
    for (auto d : c.info)
        cout << " " << d;
    for (auto d : c.valu)
        cout << " " << d;
    cout << endl;
  }

  cout << endl;
  cout << "  STR structures summary:" << endl;
  for (auto c : stru)
  {
    cout << "   ";
    for (auto d : c.info)
        cout << " " << d;
    for (auto d : c.valu)
        cout << " " << d;
    cout << endl;
  }
  cout << endl;
  
  cout << "  ANC anchor position summary:" << endl;
  if (anchor.size() != 0)
  {
  
    cout << "   ";
    for (auto c : anchor)
      cout << " " << c;
  
  } else {
    cout << "    No Anchors";
  }
  cout << endl << endl;
  cout << endl;
  cout << "  PROCESSED: Structure Ids: stucture type, draw type" << endl;

  for( int i = 2; i < typeID.size(); i +=1 )
  {
      cout << "    " << typeID[i] << " " << drawID[i] << endl;
  }
  cout << endl;

  cout << "  PROCESSED: Initial Structure positions and velocities:" << endl;
//  calcStrucPositions();
  for (auto c : StrucPosVec) // Display results
  {
    cout << "   ";
    for (auto d : c)
    {
      cout << ' ' << d;
    } 
    cout << endl;  // to split lines
  }
  cout << endl;

  cout << "  PROCESSED: Combined Position Vector" << endl;
  cout << "   ";
    for (auto c : Position)
      cout << " " << c;
  cout << endl;  cout << endl;

  cout << "  PROCESSED: Combined Velocity Vector" << endl;
  cout << "   ";
    for (auto c : Velocity)
      cout << " " << c;
  cout << endl;  cout << endl;
  
  
  cout << "  PROCESSED: Anchor rotation converted to radians" << endl;
    for( int i = Dimension ; i < anchor.size(); i +=1 )
  {
      anchor[i]=anchor[i]*deg2rad;
  }
  cout << endl;
  cout << "   ";
    for (auto c : anchor)
      cout << " " << c;
  cout << endl;  cout << endl;

}


void StrucParams::calcStrucPositions()
{ 
  /* Place the first structure, converting degrees to radians */
  for( int k = k1; k <= k2 ; k +=1 )
  {
    pos0[k]=pos0[k]*deg2rad;
    pos0[k+Size]=pos0[k+Size]*deg2rad;
  }
  StrucPosVec.push_back(pos0);   

  /* Place the other structures*/
  for( int i = 1; i < stru.size(); i +=1 )
  {
    StrucPosVec.push_back(pos0);
    int L=0.5*(type[typeID[i]].valu[1]+type[typeID[i-1]].valu[1]);
    
    cout << "Size: " << Size << endl;
    vector <float> Cos = {0,0,0};

    switch (Dimension)
    {
    case 2:
      Cos={-cos(pos0[2]),-sin(pos0[2]),0}; // Update for 3D
cout << "  Case2: " << Size << endl;
    break;
    case 3:
      ycosines(pos0,Cos); // consider reference direction to +y
    break;
    }
    
    for( int j = 0; j <= j2 ; j +=1 )
    {
      cout << "j: " << j  << " Cos j: " << Cos[j] << endl;
      StrucPosVec[i][j]=StrucPosVec[i-1][j]
        +L*Cos[j];
    }
  }

  /* Rearrange vectors to transfer to fortran */
  for (auto c : StrucPosVec) // Display results
  {
    Position.insert(Position.end(), c.begin(), c.end()-Size);
    Velocity.insert(Velocity.end(), c.begin()+Size, c.end());
  }
}


/* Input Procesing Functions*/


void StrucParams::str()
{ 
 isKey="STR";
//  cout << "SIZE: " << line.size() << endl;
  if ( line.size() < 2)
  {
    cout << "str requires at least one entry after keyword" << endl;
    isKey="  INPUT";
    printline();
    cout << "  Exiting... " << endl;
    exit(1);
  }
  tempParam.info.push_back(line[1]); // write type name
  for( int i = 0; i < type.size(); i +=1 ) //search for type in types and store id.
  {
    if (type[i].info[0].compare(line[1]) == 0)
    {
      typeID.push_back(i);
      drawID.push_back(type[i].info[1]);
      break;
    }
  }
  tempParam.valu.resize(3,1.); // set default to white
  for( int i = 2; i < line.size(); i +=1 )
  {
    tempParam.valu[i-2]= stof(line[i]);
  }
  stru.push_back(tempParam);
  tempParam.info.clear();
  tempParam.valu.clear();
}


void StrucParams::typ()
{ 
  isKey="TYP";
/* The first two entries should be strings.  All the rest should be floats. 
  All entries are required. */
  
  if ( line.size() < 6)
  {
    cout << "typ requires at least 5 entries after keyword" << endl;
    isKey="  INPUT";
    printline();
    cout << "  Exiting... " << endl;
    exit(1);
  }
  
  tempParam.info.push_back(line[1]);
  tempParam.info.push_back(line[2]);
  for( int i = 3; i < line.size(); i +=1 )
  {
    tempParam.valu.push_back(stof(line[i]));
  }
  type.push_back(tempParam);
  tempParam.info.clear();
  tempParam.valu.clear();
}


void StrucParams::pos()
{ 
 isKey="POS";
  for( int i = 1; i < line.size(); i +=1 )
   {
     pos0[i-1]= stof(line[i]);
   }
}


void StrucParams::spr()
{ 
  isKey="SPR";
  if ( line.size() > 1)
  {
    Spring=stof(line[1]);
    if ( line.size() > 2) Damp=stof(line[2]);
  }
}


void StrucParams::anc()
{ 
 isKey="ANC";
  if ( line.size() > 1)
  {
    for( int i = 1; i < line.size(); i +=1 )
    {
      anchor.push_back(stof(line[i]));
    }
  }
}


void StrucParams::ycosines(std::vector<float>& X, std::vector<float>& C)
{ // calculates the y cosines of the rotation to determine initial direction
  float a=sqrt(pow(X[3],2.) + pow(X[4],2.) + pow(X[5],2.)); // theta
  float w[3];
  for (int i = 0; i < 3; i++) w[i]=X[i+3]/a;
  float c=cos(a);
  float s=-sin(a);
  float t=1.-cos(a);
  C[0]=t*w[0]*w[1]-s*w[2];
  C[1]=t*pow(w[1],2.)+c;
  C[2]=t*w[1]*w[2]+s*w[1];
}










