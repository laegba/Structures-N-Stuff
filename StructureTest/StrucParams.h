/*

  StrucParams.h
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

*/

#ifndef StrucParams_h
#define StrucParams_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>


class StrucParams
{
public:

  StrucParams(std::string filePath);

  /* variable */
  std::vector<std::vector<float>> StrucPosVec; // Structure Position and Velocity
  std::vector<float> Position;        // Position Vector
  std::vector<float> Velocity;        // Velocity Vector

  /* invariable after initial file read */
  int Dimension;                      // Dimension (2 or 3 for 2D or 3D)
  int Size;                           // Array size for each struc
  int StrucNumber;                    // Number of Structures
  std::vector<float> anchor;          // anchors
  std::vector<float> Mass;            // Mass-inertia Vector
  float Spring;                       // Spring constant
  float Damp;                         // Damping constant
  std::vector<int> typeID;            // Type ID
  std::vector<std::string> drawID;    // Draw ID
  
   
//  std::vector<int> drawID;          // Draw ID 
/*  Draw ID should be defined so that if shapes of different sizes are defined 
    it searches through the shortest possible list.
    
    This could be a string vector.
    The drawing class could then assign ids as appropriate
      upon transfer of data to match up with predefined drawing objects
  */
  // failure criteria, e.g. torque, bend angle, collision velocity.

  void Summary();                     // Summary of interpreted input


private:
  int j2,k1,k2;                                  // dimension array positions

  /* Should be put into a read file class */
// input variables
  std::string lineText;                          // input line text string
  std::string inp;                               // input string line element
  std::vector <std::string> line;                // input line string array
  std::vector <std::vector <std::string>> input; // input file array
  
  
// functions:
  std::string isKey;                             // input file array
  std::vector<float> pos0;                       // reference position and rotation
  
// functions:
  void parseline();
  void printline();
  void displayinput();
  void calcStrucPositions();
  void ycosines(std::vector<float>& X, std::vector<float>& C);

  
// process keywords:
  void str();
  void typ();
  void pos();
  void spr();
  void anc();

  struct Param
  {
    std::vector <std::string> info;
    std::vector <float> valu;
  };
  
  Param tempParam;

  std::vector <Param> type;                     // all structure types
  std::vector <Param> stru;                     // all structures
 
};

#endif /* #define StrucParams_h */

