#include "mcell_technology.h"
#include "mcell_error.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <algorithm>       // std::sort
#include "yaml-cpp/yaml.h" // IWYU pragma: keep
using namespace std;

vector<char *> Tech::splitStr(char *str, char *delimiter)
{

  vector<char *> v;
  char *pch;
  pch = strtok(str, delimiter);
  while (pch != NULL)
  {
    v.push_back(pch);

    pch = strtok(NULL, delimiter);
  }

  return v;
}

int Tech::readFile(const char *fileName)
// write exception for not found file
// write exception for sintacs error
{
  ifstream inFile;
  inFile.exceptions(ifstream::badbit);

  inFile.open(fileName);

  if (!inFile)
  {

    string errMessage = "\nError: <Technology file>\nTechnology file: " + string(fileName) + " does not exist!\n";

    throw HardError(errMessage);
  }

  string line;
  char delimiter[] = " ="; // deliniter is with space and =
  vector<char *> v;

  int layerFleg = 0; // if in layer ... endLayer structure fleg is set to 1

  int metFlag = 0; // metFlag==1 -> metal
  int viaFlag = 0; // viaFlag==1 -> via
  int dieFlag = 0; // dieFlag==1 -> die
  int subFlag = 0; // subFlag==1 -> substrate
  char *comand;

  // Comands
  const char *comGrid = "grid";
  const char *comLayer = "layer";
  const char *comMetal = "metal";
  const char *comVia = "via";
  const char *comGdsNum = "gdsNum";
  const char *comGdsType = "gdsType";
  const char *comMinS = "minS";
  const char *comMinW = "minW";
  const char *comDSub = "dSub";
  const char *comMetT = "metThic";
  const char *comEndlayer = "endLayer";
  const char *comMetCond = "metCond";

  const char *comTopMet = "topMet";
  const char *comBotMet = "botMet";
  const char *comViaEnc = "viaEnc";
  const char *comViaSize = "viaSize";
  const char *comViaSpace = "viaSpace";
  const char *comViaCond = "viaCond";

  // dielectric
  const char *comDie = "dielectric";
  const char *comEr = "er";
  const char *comEi = "ei";

  // substrate
  const char *comSub = "substrate";
  const char *comSubEr = "er";
  const char *comSubSubCond = "subCond";

  // valid options
  std::string validCommands = " " + string(comGrid);
  validCommands = validCommands + ", " + string(comLayer);
  validCommands = validCommands + ", " + string(comEndlayer);
  validCommands = validCommands + ", " + string(comMetal);
  validCommands = validCommands + ", " + string(comVia);
  validCommands = validCommands + ", " + string(comGdsNum);
  validCommands = validCommands + ", " + string(comGdsType);
  validCommands = validCommands + ", " + string(comMinS);
  validCommands = validCommands + ", " + string(comMinW);
  validCommands = validCommands + ", " + string(comDSub);
  validCommands = validCommands + ", " + string(comMetT);
  validCommands = validCommands + ", " + string(comMetCond);
  validCommands = validCommands + ", " + string(comTopMet);
  validCommands = validCommands + ", " + string(comBotMet);
  validCommands = validCommands + ", " + string(comViaEnc);
  validCommands = validCommands + ", " + string(comViaSize);
  validCommands = validCommands + ", " + string(comViaSpace);
  validCommands = validCommands + ", " + string(comViaCond);
  validCommands = validCommands + ", " + string(comDie);
  validCommands = validCommands + ", " + string(comEr);
  validCommands = validCommands + ", " + string(comSub);
  validCommands = validCommands + ", " + string(comSubSubCond);

  // via and metal

  unsigned int lineNumber = 0;

  LayerMet tempMetal;
  LayerVia tempVia;
  LayerDielectric tempDie;
  LayerSub tempSub;

  while (getline(inFile, line))
  {
    lineNumber++;

    v = splitStr(&line[0], delimiter);

    if (v.size() > 0)
    {
      comand = v[0];
      // cout<<comand<<endl;
      // Coment
      if (comand[0] == '#')
      {
        continue;
      }

      if (strcmp(comand, comGrid) == 0) // set grid
      {

        layerFleg = 0;

        if (v.size() < 2)
        {
          string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
          errMessage = errMessage + "Setting grid is invalid!\n";
          inFile.close();
          throw HardError(errMessage);
        }

        try
        {
          this->grid = stod(v[1]);
        }
        catch (...)
        {
          string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
          errMessage = errMessage + "Setting grid is invalid!\n";
          errMessage = errMessage + v[1] + " is not a number!\n";
          inFile.close();
          throw HardError(errMessage);
        }

        continue;
      }

      if (layerFleg == 0)
      {
        if (strcmp(comand, comLayer) == 0)
        {
          layerFleg = 1;

          if (v.size() < 3)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + "Setting layer is invalid!\n";
            inFile.close();
            throw HardError(errMessage);
          }

          string name(v[1]);

          comand = v[2];

          if (strcmp(comand, comMetal) == 0)
          {
            metFlag = 1; // metal
            viaFlag = 0;
            dieFlag = 0;
            subFlag = 0;
            tempMetal.name = name;
          }
          else if (strcmp(comand, comVia) == 0)
          {
            metFlag = 0; // metal
            viaFlag = 1; // via
            dieFlag = 0;
            subFlag = 0;
            tempVia.name = name;
          }
          else if (strcmp(comand, comDie) == 0)
          {
            metFlag = 0; // metal
            viaFlag = 0; // via
            dieFlag = 1;
            subFlag = 0;
            tempDie.name = name;
          }
          else if (strcmp(comand, comSub) == 0)
          {
            metFlag = 0; // metal
            viaFlag = 0; // via
            dieFlag = 0;
            subFlag = 1;
            tempSub.name = name;
          }

          else
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + "Option: " + comand + " is invalid\n";
            errMessage = errMessage + "Valid options are:" + validCommands + "\n";
            inFile.close();
            throw HardError(errMessage);
          }

          continue;
        }
      }

      // metal layer

      if (layerFleg == 1 and metFlag == 1)
      {

        if (strcmp(comand, comEndlayer) == 0)
        {
          layerFleg = 0;
          metal.push_back(tempMetal);
          continue;
        }

        if (v.size() < 2)
        {
          string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
          errMessage = errMessage + "Setting metal layer is invalid!\n";
          inFile.close();
          throw HardError(errMessage);
        }

        comand = v[0];

        if (strcmp(comand, comGdsNum) == 0)
        {
          try
          {

            tempMetal.gdsNum = stoi(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comGdsType) == 0)
        {
          try
          {

            tempMetal.gdsType = stoi(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comMinS) == 0)
        {
          try
          {
            tempMetal.minS = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comMinW) == 0)
        {

          try
          {
            tempMetal.minW = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comDSub) == 0)
        {

          try
          {
            tempMetal.dSub = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comMetT) == 0)
        {

          try
          {
            tempMetal.metT = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comMetCond) == 0)
        {
          try
          {
            tempMetal.cond = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        // invalid option set
        string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
        errMessage = errMessage + "Option: " + comand + " is invalid\n";
        errMessage = errMessage + "Valid options are:" + validCommands + "\n";
        inFile.close();
        throw HardError(errMessage);
      }

      // via layer

      if (layerFleg == 1 and viaFlag == 1)
      {
        comand = v[0];

        if (strcmp(comand, comEndlayer) == 0)
        {
          layerFleg = 0;
          via.push_back(tempVia);
          continue;
        }

        if (v.size() < 2)
        {
          string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
          errMessage = errMessage + "Setting via layer is invalid!\n";
          inFile.close();
          throw HardError(errMessage);
        }

        if (strcmp(comand, comGdsNum) == 0)
        {

          try
          {

            tempVia.gdsNum = stoi(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }

          continue;
        }

        if (strcmp(comand, comGdsType) == 0)
        {

          try
          {

            tempVia.gdsType = stoi(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }

          continue;
        }

        if (strcmp(comand, comViaEnc) == 0)
        {

          try
          {

            tempVia.viaEnc = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comViaSize) == 0)
        {
          try
          {

            tempVia.viaSize = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comViaSpace) == 0)
        {
          try
          {

            tempVia.viaSpace = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }

          continue;
        }

        if (strcmp(comand, comTopMet) == 0)
        {
          tempVia.topMet = v[1];
          continue;
        }

        if (strcmp(comand, comBotMet) == 0)
        {
          tempVia.botMet = v[1];
          continue;
        }

        if (strcmp(comand, comViaCond) == 0)
        {
          try
          {
            tempVia.cond = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

      } // via

      // dielectric layer

      if (layerFleg == 1 and dieFlag == 1)
      {

        if (strcmp(comand, comEndlayer) == 0)
        {
          layerFleg = 0;
          this->dielectric.push_back(tempDie);
          continue;
        }

        if (v.size() < 2)
        {
          string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
          errMessage = errMessage + "Setting dielectric layer is invalid!\n";
          inFile.close();
          throw HardError(errMessage);
        }

        comand = v[0];

        if (strcmp(comand, comDSub) == 0)
        {

          try
          {
            tempDie.dSub = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comEr) == 0)
        {

          try
          {
            tempDie.er = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comEi) == 0)
        {

          try
          {
            tempDie.ei = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        // invalid option set
        string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
        errMessage = errMessage + "Option: " + comand + " is invalid\n";
        errMessage = errMessage + "Valid options are:" + validCommands + "\n";
        inFile.close();
        throw HardError(errMessage);

      } // dielectric

      // substrate layer

      if (layerFleg == 1 and subFlag == 1)
      {

        if (strcmp(comand, comEndlayer) == 0)
        {
          layerFleg = 0;
          this->substrate = tempSub;
          this->substrate.freq = -1;
          this->substrate.dSub = 0;
          this->substrate.ei = -1;
          continue;
        }

        if (v.size() < 2)
        {
          string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
          errMessage = errMessage + "Setting substrate layer is invalid!\n";
          inFile.close();
          throw HardError(errMessage);
        }

        comand = v[0];

        if (strcmp(comand, comSubEr) == 0)
        {

          try
          {
            tempSub.er = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        if (strcmp(comand, comSubSubCond) == 0)
        {

          try
          {
            tempSub.cond = stod(v[1]);
          }
          catch (...)
          {
            string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
            errMessage = errMessage + v[1] + " is not a number!\n";
            inFile.close();
            throw HardError(errMessage);
          }
          continue;
        }

        // invalid option set
        string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
        errMessage = errMessage + "Option: " + comand + " is invalid\n";
        errMessage = errMessage + "Valid options are:" + validCommands + "\n";
        inFile.close();
        throw HardError(errMessage);

      } // substrate

      // invalid option set
      string errMessage = "\nError: <Technology file> line: " + to_string(lineNumber) + "\n";
      errMessage = errMessage + "Option: " + comand + " is invalid\n";
      errMessage = errMessage + "Valid options are:" + validCommands + "\n";

      inFile.close();

      throw HardError(errMessage);
    }
  }

  inFile.close();
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LayerMet Tech::getMet(string nname)

{
  LayerMet ret;
  int fleg = 0;

  for (long unsigned int i = 0; i < metal.size(); i++)
  {
    if (metal[i].name == nname)
    {
      fleg = 1;
      ret = metal[i];
      break;
    }
  }

  if (fleg == 0)
  {
    string errMessage = "\nError: <Tech::getMet> :\n";
    errMessage = errMessage + "Metal layer: " + nname + " is not found!";
    throw HardError(errMessage);
  }

  return ret;
}

LayerMet Tech::getMet(int metNum)

{
  LayerMet ret;
  int fleg = 0;

  for (long unsigned int i = 0; i < metal.size(); i++)
  {
    if (metal[i].metNum == metNum)
    {
      fleg = 1;
      ret = metal[i];
      break;
    }
  }

  if (fleg == 0)
  {
    string errMessage = "\nError: <Tech::getMet> :\n";
    errMessage = errMessage + "Metal number: " + to_string(metNum) + " is not found!";
    throw HardError(errMessage);
  }

  return ret;
}

string Tech::getMetName(int metNum)

{
  string ret;
  int fleg = 0;

  for (long unsigned int i = 0; i < metal.size(); i++)
  {
    if (metal[i].metNum == metNum)
    {
      fleg = 1;
      ret = metal[i].name;
      break;
    }
  }

  if (fleg == 0)
  {
    string errMessage = "\nError: <Tech::getMet> :\n";
    errMessage = errMessage + "Metal number: " + to_string(metNum) + " is not found!";
    throw HardError(errMessage);
  }

  return ret;
}

string Tech::getMetNamePP(string nname)
{
  return this->getMetName((this->getMet(nname)).metNum + 1);
}

string Tech::getMetNameMM(string nname)
{
  return this->getMetName((this->getMet(nname)).metNum - 1);
}

LayerVia Tech::getVia(string ttopMet, string bbotMet)

{
  LayerVia ret;
  int fleg = 0;

  for (long unsigned int i = 0; i < via.size(); i++)
  {
    if (via[i].topMet == ttopMet and via[i].botMet == bbotMet)
    {
      fleg = 1;
      ret = via[i];
      break;
    }
  }

  if (fleg == 0)
  {
    string errMessage = "\nError: <Tech::getVia> :\n";
    errMessage = errMessage + "Via between top metal layer: " + ttopMet + " and bottom metal layer: " + bbotMet + " is not found\n";

    throw HardError(errMessage);
  }

  return ret;
}

double Tech::getGrid()
{

  return grid;
}

int Tech::checkTech() // checking if technology file is valid
{

  // check metal layers

  for (long unsigned int i = 0; i < metal.size(); i++)
  {
    if (this->metal[i].minW <= 0)
    {
      string errMessage = "\nError: <Tech::metal layer> :\n";
      errMessage = errMessage + "Metal layer: " + this->metal[i].name + " \n";
      errMessage = errMessage + "Minimal width: " + to_string(this->metal[i].minW) + " needs to be higher than 0!\n";

      throw HardError(errMessage);
    }

    if (this->metal[i].minS <= 0)
    {
      string errMessage = "\nError: <Tech::metal layer> :\n";
      errMessage = errMessage + "Metal layer: " + this->metal[i].name + " \n";
      errMessage = errMessage + "Minimal space: " + to_string(this->metal[i].minS) + " needs to be higher than 0!\n";

      throw HardError(errMessage);
    }

    if (this->metal[i].cond <= 0)
    {
      string errMessage = "\nError: <Tech::metal layer> :\n";
      errMessage = errMessage + "Metal layer: " + this->metal[i].name + " \n";
      errMessage = errMessage + "Conductivity: " + to_string(this->metal[i].cond) + " needs to be higher than 0!\n";

      throw HardError(errMessage);
    }

  } // for

  // check via layer

  for (long unsigned int i = 0; i < via.size(); i++)
  {
    if (this->via[i].viaSize <= 0)
    {
      string errMessage = "\nError: <Tech::via layer> :\n";
      errMessage = errMessage + "Via layer: " + this->via[i].name + " \n";
      errMessage = errMessage + "Via size: " + to_string(this->via[i].viaSize) + " needs to be higher than 0!\n";

      throw HardError(errMessage);
    }

    if (this->via[i].viaSpace <= 0)
    {
      string errMessage = "\nError: <Tech::via layer> :\n";
      errMessage = errMessage + "Via layer: " + this->via[i].name + " \n";
      errMessage = errMessage + "Via space: " + to_string(this->via[i].viaSpace) + " needs to be higher than 0!\n";

      throw HardError(errMessage);
    }

    if (this->via[i].viaEnc < 0)
    {
      string errMessage = "\nError: <Tech::via layer> :\n";
      errMessage = errMessage + "Via layer: " + this->via[i].name + " \n";
      errMessage = errMessage + "Via enclosure: " + to_string(this->via[i].viaEnc) + " needs to be higher or equal 0!\n";

      throw HardError(errMessage);
    }

    if (this->via[i].cond < 0)
    {
      string errMessage = "\nError: <Tech::via layer> :\n";
      errMessage = errMessage + "Via layer: " + this->via[i].name + " \n";
      errMessage = errMessage + "Conductivity: " + to_string(this->via[i].cond) + " needs to be higher than 0!\n";

      throw HardError(errMessage);
    }

  } // for

  // check dielectrics
  // if (this->dielectric.size() < 2)
  // {
  //   string errMessage = "\nError: <Tech::dielectric layer> :\n";
  //   errMessage = errMessage + "Minimum two dielectrics must be defined.\n";
  //   errMessage = errMessage + "First: bottom of the chip.\n";
  //   errMessage = errMessage + "Second: top of the chip.\n";
  //   errMessage = errMessage + "All metal and via layers must be between those two dielectrics.\n";
  //   throw HardError(errMessage);
  // }

  // if (this->dielectric[0].dSub > this->metal[0].dSub)
  // {
  //   string errMessage = "\nError: <Tech::dielectric layer> :\n";
  //   errMessage = errMessage + "Minimum two dielectrics must be defined.\n";
  //   errMessage = errMessage + "First: bottom of the chip.\n";
  //   errMessage = errMessage + "Second: top of the chip.\n";
  //   errMessage = errMessage + "All metal and via layers must be inbetween those two dielectrics.\n";
  //   errMessage = errMessage + "metal: " + this->metal[0].name + " is not inbetween dielectric layers.";
  //   throw HardError(errMessage);
  // }

  // if (this->dielectric[this->dielectric.size() - 1].dSub < this->metal[this->metal.size() - 1].dSub)
  // {
  //   string errMessage = "\nError: <Tech::dielectric layer> :\n";
  //   errMessage = errMessage + "Minimum two dielectrics must be defined.\n";
  //   errMessage = errMessage + "First: bottom of the chip.\n";
  //   errMessage = errMessage + "Second: top of the chip.\n";
  //   errMessage = errMessage + "All metal and via layers must be inbetween those two dielectrics.\n";
  //   errMessage = errMessage + "metal: " + this->metal[this->metal.size() - 1].name + " is not inbetween dielectric layers.";
  //   throw HardError(errMessage);
  // }

  return 0;
}

bool sortFunLayerMet(const LayerMet &m1, const LayerMet &m2)
{
  return m1.dSub < m2.dSub; // sort from bottom to top
}

bool sortFunLayerDielectric(const LayerDielectric &d1, const LayerDielectric &d2)
{
  return d1.dSub < d2.dSub; // sort from bottom to top
}

int Tech::sortMet()
{
  sort(this->metal.begin(), this->metal.end(), sortFunLayerMet);

  for (long unsigned int i = 0; i < this->metal.size(); i++)
  {
    this->metal[i].metNum = i + 1;
  }

  return 0;
}

int Tech::sortDielectric()
{
  sort(this->dielectric.begin(), this->dielectric.end(), sortFunLayerDielectric);

  // assign id numbers

  for (uint64_t i = 0; i < this->dielectric.size(); i++)
  {
    this->dielectric[i].id = i;
  }

  return 0;
}

int Tech::getMet(LayerMet &mmet, int ggdsNum, int ggdsType)
{

  for (long unsigned int i = 0; i < this->metal.size(); i++)
  {
    if (this->metal[i].gdsNum == ggdsNum and this->metal[i].gdsType == ggdsType)
    {
      mmet = this->metal[i];
      return 0;
    }
  }

  return 1;
}

int Tech::getVia(LayerVia &vvia, int ggdsNum, int ggdsType)
{

  for (long unsigned int i = 0; i < this->via.size(); i++)
  {
    if (this->via[i].gdsNum == ggdsNum and this->via[i].gdsType == ggdsType)
    {
      vvia = this->via[i];
      return 0;
    }
  }

  return 1;
}

bool Tech::isEqual(double x, double y)
{
  return abs(x - y) < 0.01 * this->grid ? true : false;
}

void Tech::readYamlFile(const std::string &fileName)
{

  try
  {

    YAML::Node node = YAML::LoadFile(fileName);

    // -----------------------------------------------------------
    // grid
    // -----------------------------------------------------------
    this->grid = node["grid"].as<double>(-1);

    if (this->grid < 0)
    {
      std::string massage = "Error: technology file, grid is not set!";
      throw HardError(massage);
    }

    // -----------------------------------------------------------
    // metal layers
    // -----------------------------------------------------------
    if (const YAML::Node &metal = node["metal"])
    {
      for (YAML::const_iterator it = metal.begin(); it != metal.end(); ++it)
      {
        LayerMet layerMetal;

        layerMetal.name = it->first.as<std::string>();
        const YAML::Node &layer = it->second;

        layerMetal.dSub = layer["h"].as<double>();
        layerMetal.metT = layer["t"].as<double>();
        layerMetal.cond = layer["sigma"].as<double>();
        layerMetal.gdsNum = layer["num"].as<int>();
        layerMetal.gdsType = layer["typ"].as<int>();
        layerMetal.minW = layer["w"].as<double>();
        layerMetal.minS = layer["s"].as<double>();

        this->metal.push_back(layerMetal);
      }
    }
    else
    {
      std::string massage = "Error: technology file, metal layers are not set!";
      throw HardError(massage);
    }

    // -----------------------------------------------------------
    // via layers
    // -----------------------------------------------------------
    if (const YAML::Node &via = node["via"])
    {
      for (YAML::const_iterator it = via.begin(); it != via.end(); ++it)
      {
        LayerVia layerVia;

        layerVia.name = it->first.as<std::string>();
        const YAML::Node &layer = it->second;

        layerVia.topMet = layer["top"].as<std::string>();
        layerVia.botMet = layer["bottom"].as<std::string>();
        layerVia.cond = layer["sigma"].as<double>();
        layerVia.gdsNum = layer["num"].as<int>();
        layerVia.gdsType = layer["typ"].as<int>();
        layerVia.viaSize = layer["w"].as<double>();
        layerVia.viaSpace = layer["s"].as<double>();
        layerVia.viaEnc = layer["e"].as<double>();

        this->via.push_back(layerVia);
      }
    }
    else
    {
      std::string massage = "Error: technology file, via layers are not set!";
      throw HardError(massage);
    }
  }
  catch (const std::exception &e)
  {
    std::string massage = e.what();
    massage = "Error in technology file!\n" + massage;
    throw HardError(massage);
  }
}
