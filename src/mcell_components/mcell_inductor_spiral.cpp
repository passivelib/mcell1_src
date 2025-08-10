#include "mcell_error.h"
#include "gdstk.h"
#include "mcell_inductor_spiral.h"
#include "mcell_global.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <filesystem>
#include <sys/stat.h>
#include <thread> // std::thread
#include <chrono>
#include <array>

int InductorSpiral::drc(double d, double w, double wu, double s, double sp, double lp, double n, int geometry, string topM)
{
  // # geometry   : 0-> octagonal, else rectangular

  LayerMet topMId = this->l->getMet(topM);
  LayerMet bottomMId = this->l->t.getMet(topMId.metNum - 1);
  LayerVia viaId = this->l->getVia(topM, bottomMId.name);

  double dn = 2 * (0.5 * d - w - (floor(n)) * (w + s));

  int nInt = floor(n);
  double nMod = n - nInt;

  // double grid=2*this->l->t.getGrid();

  // ###############################################################################################
  // # metal width
  // ###############################################################################################

  if (w < topMId.minW)
  {
    throw SoftError("Warning: metal width < minW!");
  }

  if (wu * w < bottomMId.minW)
  {
    throw SoftError("Warning: underpass metal width < minW!");
  }

  // ###############################################################################################
  // # metal space
  // ###############################################################################################

  if (s < topMId.minS)
  {
    throw SoftError("Warning: metal space < minS!");
  }

  // ###############################################################################################
  // # turns
  // ###############################################################################################

  if (n < 1)
  {
    throw SoftError("Warning: n < 1!");
  }

  if (dn < topMId.minS)
  {
    throw SoftError("Warning: inner diameter too small!");
  }

  // ###############################################################################################
  // # check pins
  // ###############################################################################################

  if (sp < topMId.minS and nMod == 0)
  {
    throw SoftError("Warning: pin space < minS!");
  }

  if (lp < topMId.minS or lp < bottomMId.minS)
  {
    throw SoftError("Warning: pin length < minS!");
  }

  // ########################################################
  // #
  // #     k
  // #   -------
  // #   *******
  // #           *   2k
  // #             *
  // #               *
  // #                 * |
  // #                 * |
  // #     0.5*d       * | k
  // #   <------------>* |
  // #
  // #     k=(sqrt(2)-1)*0.5*d

  double dn_1 = 2 * (0.5 * d - w - (floor(n) - 1) * (w + s)); // d_(n-1)
  double kn_1 = this->l->k(dn_1);
  double kn = this->l->k(dn);
  // cout<<"dn="<<dn<<endl;

  if (nMod == 0) // pins at the same side
  {

    if (geometry == 1 and dn_1 < (sp + 2 * wu + 2 * topMId.minS)) // rect  uvek je wu>=w
    {
      throw SoftError("Warning: pin space too wide!");
    }

    if (geometry == 0 and 2 * kn_1 < (sp + 2 * wu + 2 * topMId.minS)) // oct
    {
      throw SoftError("Warning: pin space too wide!");
    }
  } // if
  else if (nMod == 0.75) // output pin at bottom
  // pin is placed from 0 to wu on x
  {
    //
    //                                   .---.                                           .---.
    // .---.               .             |   |            .---.             .            |   |
    // |   |               |             |   |            |   |             |            |   |
    // |   |               |      0.5dn  |   |            |   |             |            |   |
    // |   |               |<----------->|   |            |   |             |     kn     '   '
    // |   |               |             |   |            '   '             | <-------> /   /
    // |   | 0.5dn_1-w-s   .---.---------|   |             \   \            .---.------'   /
    // |   |<------------->| v |             |              \   \  kn_1-w-s | v |         /
    // |   |               '---'-------------'               \   \ <------> '---'---------
    // .   ----------------|   |--------------------.         \   '---------|   |---------------------.
    // |                   |   |                    |          \            |   |                     |
    // '-------------------|   |--------------------'           ------------|   |---------------------'
    //                     |   |                                            |   |
    //                     |   |                                            |   |
    //                     |---'                                            '---'
    //                     | ^       pin                                    | ^       pin
    //                     ' '---------                                     ' '---------
    //                    x=0                                              x=0

    if (geometry == 1 and 0.5 * dn - topMId.minS < (wu)) // rect wu>w
    {
      throw SoftError("Warning: underpass contact!");
    }

    if (geometry == 0 and kn - topMId.minS < (wu)) // oct wu>w
    {
      throw SoftError("Warning: underpass contact!");
    }
  }
  else
  {
    if (geometry == 1 and dn - 2 * topMId.minS < (wu)) // rect wu>w
    {
      throw SoftError("Warning: underpass contact!");
    }

    if (geometry == 0 and 2 * kn - 2 * topMId.minS < (wu)) // oct wu>w
    {
      throw SoftError("Warning: underpass contact!");
    }
  }

  // ###############################################################################################
  // # oct ind translate makes problem, left turns are shifted to right
  // ###############################################################################################
  // shift is done for w+s in right side

  // .---.          .                  .---.  shift:w+s  .
  // |   |          |                  |   |  -------->  |
  // |   | shift:w+s|                  |   |             |
  // |   | -------> |                  |   |             |
  // |   |          |                  '   '             |
  // |   |  0.5dn_1 |--------------.    \   \            |------.
  // |   |<-------->|              |     \   \    kn_1   |      |
  // |   |          |--------------'      \   \  <---->  |------'
  // .   -----------|--------------.       \   '---------|------.
  // |              |              |        \            |      |
  // '--------------|--------------'         ------------|------'
  //                |                                    |
  //                '                                    '
  //               x=0                                  x=0

  if ((geometry == 0) and kn_1 < w + s + topMId.minS)
  {

    throw SoftError("Warning: inner diameter too small!");
  }

  // ###############################################################################################
  // # rect ind translate makes problem
  // ###############################################################################################

  if ((geometry == 1) and 0.5 * dn_1 < w + s + topMId.minS)
  {

    throw SoftError("Warning: inner diameter too small!");
  }

  return 0;
}

int InductorSpiral::genCell(Cell &result, double d, double w, double wu, double s, double sp, double lp, double n, int geometry, string topM)
{
  // ###############################################################################################
  // # symmetrical inductor
  // ###############################################################################################
  // # d        : outer diameter
  // # w       : line width
  // # wu       : line width for underpass layer
  // # s        : space between lines
  // # sp       : space between pins
  // # lp             : pin length (default should be 2w)
  // # n        : number of turns, step 0.25
  // # geometry   : 0-> octagonal, else rectangular
  // # topM     : top metal layer (metal name)
  // ###############################################################################################

  LayerMet topMId = this->l->getMet(topM);
  string bottomM = this->l->t.getMetName(topMId.metNum - 1);

  double grid = this->l->t.getGrid();

  d = 2 * this->l->round2grid(0.5 * d);
  w = 2 * this->l->round2grid(0.5 * w);

  // leave a bit more space for 45 deg lines
  if (s >= topMId.minS and s <= topMId.minS + 2 * grid)
  {
    s = 2 * this->l->round2grid(0.5 * s + grid);
  }
  else
  {
    s = 2 * this->l->round2grid(0.5 * s);
  }

  if (sp <= 0)
    sp = 2 * w;
  if (lp <= 0)
    lp = 2 * w;

  sp = 2 * this->l->round2grid(0.5 * sp);
  lp = 2 * this->l->round2grid(0.5 * lp);
  wu = 2 * this->l->round2grid(0.5 * wu);

  // ###############################################################################################
  // # geometry check
  // ###############################################################################################

  n = round(n / 0.25) * 0.25;

  if (wu < w) // underpass layer can not be less wide than w
  {
    wu = w;
  }

  this->drc(d, w, wu, s, sp, lp, n, geometry, topM);

  // #####################################################################################################
  // #        *   *
  // #        *   *
  // #        *   *
  // #     p1 ***** p2 ------------------
  // #          |                 |
  // #          |    e            |
  // #          <-------->|       | b
  // #                    |       |
  // #               p3 ***** p4 --------
  // #                  *   *
  // #                  *<->*
  // #                  * w *
  // #####################################################################################################

  // double e=w+s;
  // double b=e+w/(1+sqrt(2))+2*topMId.minS;

  // ###############################################################################################
  // # 1. segment
  // ###############################################################################################

  //  # Parameters:
  //  #            w  : float
  //  #           segment width
  //  #            r  : float
  //  #                 outer radius
  //  #     quadrant: integer
  //  #               quadrant to draw segment (0 - 3)
  //  #       *****************
  //  #       * 1 * 0 *
  //  #       *****************
  //  #       * 2 * 3 *
  //  #       *****************
  //  #   geometry: integer
  //  #         0-> octagonal, else rectangular
  //  #     layerName: string
  //  #         metal name from tech file
  //  ########################################################

  Polygon *path0, *path1, *path2, *path3;

  // ###############################################################################################
  // # point for cutting at left and right side for bridge45
  // ###############################################################################################

  Vec2 p1l = Vec2{-0.5 * d - w, 0.5 * sp};
  Vec2 p2l = Vec2{0, -0.5 * sp};

  Array<Polygon *> path1Arr = {0};
  // Array<Polygon*> path2Arr={0};

  int nInt = floor(n);
  double nMod = n - nInt;

  int flegFirstTime = 1;

  double dn = d;

  // ###############################################################################################
  // # loop
  // ###############################################################################################

  for (int i = 0; i < nInt; i++)
  {
    this->l->drawSegment(path0, dn, w, 0, geometry, topM);
    this->l->drawSegment(path1, dn, w, 1, geometry, topM);
    this->l->drawSegment(path2, dn, w, 2, geometry, topM);
    this->l->drawSegment(path3, dn, w, 3, geometry, topM);

    if (flegFirstTime == 1 and nMod == 0) // pins are at the same side and they should be separated by sp
    {
      this->l->crop(path1Arr, path1, p1l, p2l, topM);
      path1 = path1Arr[0];
      path1Arr = {0};
    }
    flegFirstTime = 0;

    // path2 needs to be shifted

    path2->translate(Vec2{s + w, 0});

    this->l->crop(path1Arr, path2, Vec2{0, 0}, Vec2{0.5 * d, -0.5 * d}, topM);
    path2 = path1Arr[0];
    path1Arr = {0};

    if (nMod == 0 and nInt - 1 == i) // last segment if ports are at the same side
    {
      this->l->crop(path1Arr, path2, p1l, p2l, topM);
      path2 = path1Arr[0];
      path1Arr = {0};
    }

    result.polygon_array.append(path0);
    result.polygon_array.append(path1);
    result.polygon_array.append(path2);
    result.polygon_array.append(path3);

    dn = dn - 2 * (w + s);

  } // for

  Vec2 p1, p2;

  Label *label = NULL;

  if (nMod > 0)
  {
    this->l->drawSegment(path1, dn, w, 1, geometry, topM);
    result.polygon_array.append(path1);
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Vec2{-0.5 * d - lp, 0.5 * w};
    p2 = Vec2{-0.5 * d + w, -0.5 * w};

    this->l->rectangle(path1, p1, p2, topM);
    result.polygon_array.append(path1);

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    this->l->label(label, "PLUS", Vec2{-0.5 * d - lp, 0}, topM);
    result.label_array.append(label);
  }
  else
  {
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Vec2{-0.5 * d - lp, w + 0.5 * sp};
    p2 = Vec2{-0.5 * d + w, 0.5 * sp};

    this->l->rectangle(path1, p1, p2, topM);
    result.polygon_array.append(path1);

    p1 = Vec2{-0.5 * d - lp, -wu - 0.5 * sp};
    p2 = Vec2{-0.5 * dn + w, -0.5 * sp};

    this->l->rectangle(path1, p1, p2, bottomM);
    result.polygon_array.append(path1);

    // fill via
    path1Arr.clear();
    path1Arr = {0};

    this->l->fillVias(path1Arr, Vec2{-0.5 * dn + wu, -0.5 * sp}, Vec2{-0.5 * dn, -0.5 * sp - wu}, topM, bottomM);
    this->l->appendArrToCell(result, path1Arr);

    path1Arr.clear();
    path1Arr = {0};

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    this->l->label(label, "PLUS", Vec2{-0.5 * d - lp, 0.5 * (sp + w)}, topM);
    result.label_array.append(label);

    this->l->label(label, "MINUS", Vec2{-0.5 * d - lp, -0.5 * (sp + wu)}, bottomM);
    result.label_array.append(label);

    return 0;
  }

  if (nMod > 0.25)
  {
    this->l->drawSegment(path0, dn, w, 0, geometry, topM);
    result.polygon_array.append(path0);
  }
  else
  {
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Vec2{-0.5 * wu, 0.5 * dn - w};
    p2 = Vec2{0.5 * wu, 0.5 * d + lp};

    this->l->rectangle(path1, p1, p2, bottomM);
    result.polygon_array.append(path1);

    // fill via
    path1Arr.clear();
    path1Arr = {0};

    this->l->fillVias(path1Arr, Vec2{-0.5 * wu, 0.5 * dn - wu}, Vec2{0.5 * wu, 0.5 * dn}, topM, bottomM);

    this->l->appendArrToCell(result, path1Arr);

    path1Arr.clear();
    path1Arr = {0};

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    this->l->label(label, "MINUS", Vec2{0, 0.5 * d + lp}, bottomM);
    result.label_array.append(label);

    return 0;
  }

  if (nMod > 0.5)
  {
    this->l->drawSegment(path3, dn, w, 3, geometry, topM);
    result.polygon_array.append(path3);
  }
  else
  {
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Vec2{0.5 * dn - w, -0.5 * wu};
    p2 = Vec2{0.5 * d + lp, 0.5 * wu};

    this->l->rectangle(path1, p1, p2, bottomM);
    result.polygon_array.append(path1);

    // fill via
    path1Arr.clear();
    path1Arr = {0};

    this->l->fillVias(path1Arr, Vec2{0.5 * dn - wu, -0.5 * wu}, Vec2{0.5 * dn, 0.5 * wu}, topM, bottomM);

    this->l->appendArrToCell(result, path1Arr);

    path1Arr.clear();
    path1Arr = {0};

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    this->l->label(label, "MINUS", Vec2{0.5 * d + lp, 0}, bottomM);
    result.label_array.append(label);

    return 0;
  }

  if (nMod == 0.75)
  {
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Vec2{0, -0.5 * dn + wu};
    p2 = Vec2{wu, -0.5 * d - lp};

    this->l->rectangle(path1, p1, p2, bottomM);
    result.polygon_array.append(path1);

    // fill via
    path1Arr.clear();
    path1Arr = {0};

    this->l->fillVias(path1Arr, Vec2{0, -0.5 * dn + wu}, Vec2{wu, -0.5 * dn}, topM, bottomM);
    this->l->appendArrToCell(result, path1Arr);

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    this->l->label(label, "MINUS", Vec2{0.5 * wu, -(0.5 * d + lp)}, bottomM);
    result.label_array.append(label);
  }

  return 0;
}

void InductorSpiral::printHelp()
{
  // ###############################################################################################
  // # common mode inductor
  // #    : print command line options for symind
  // #      this function is static so it can be called without any object SymInd::printHelp
  // ###############################################################################################
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--underpass-metal-width=number <--wu>:" << endl;
  std::cout << "       Set relative width for underpass layer." << endl;
  std::cout << "       Absolute width for underpass layer is --width times --underpass-metal-width." << endl;
  std::cout << "       This option is optional. If not set default value will be used." << endl;
  std::cout << "       Default option is 1." << endl;
  std::cout << "       Short option is --wu." << endl;
  std::cout << "       --underpass-metal-width=2 or --wu=2" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--space=number <-s>:" << endl;
  std::cout << "       Set space between turns of inductor. Short option is -s." << endl;
  std::cout << "       This option is mandatory." << endl;
  std::cout << "       --space=5 or -s 5" << endl;
  std::cout << "--space=smin:smax:sstep <-s>:" << endl;
  std::cout << "       Space between turns of inductor is swept from smin to smax with step sstep." << endl;
  std::cout << "       Short option is -s." << endl;
  std::cout << "       --space=2:7:1 or -s 2:7:1" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--number-of-turns=number <-n>:" << endl;
  std::cout << "       Set number of turns of inductor. Short option is -n." << endl;
  std::cout << "       This option is mandatory." << endl;
  std::cout << "       --number-of-turns=3.25 or -n 3.25" << endl;
  std::cout << "--number-of-turns=nmin:nmax:nstep <-n>:" << endl;
  std::cout << "       Number of turns of inductor is swept from nmin to nmax with step nstep." << endl;
  std::cout << "       Resolution for number of turns is 0.25." << endl;
  std::cout << "       Short option is -n." << endl;
  std::cout << "       --number-of-turns=2:5:1 or -n 2:5:1" << endl;
  std::cout << "********************************************************************************************" << endl;

  // exit(1);
}

int InductorSpiral::process(Command *cm)
{

  //**************************************************************************************************
  // round parameters
  //**************************************************************************************************
  this->roundParameters(cm);

  //**************************************************************************************************
  // set gds and cell name
  //**************************************************************************************************
  this->setName(cm);

  //**************************************************************************************************
  // if any parameter is swept set cm->generateEm = 1
  //**************************************************************************************************
  this->ifSweep(cm);

  //**************************************************************************************************
  // maestro and emx files
  //**************************************************************************************************

  ofstream runEmxShFile;
  ofstream maestroFile;

  this->makeGdsYInitEmxMaestro(runEmxShFile, maestroFile, cm);

  //**************************************************************************************************
  // counter
  //**************************************************************************************************
  unsigned long int counter = 0;

  //**************************************************************************************************
  // mutex for shared memory
  //**************************************************************************************************
  std::mutex mutex;

  //**************************************************************************************************
  // lambda function for threads
  //**************************************************************************************************
  auto threadFunc = [this, &mutex, &counter, &runEmxShFile, &maestroFile, cm](double d, double w, double s, double n)
  {
    try
    {

      Library *pointerLib = (Library *)allocate_clear(sizeof(Library));
      pointerLib->name = &cm->cellName[0];
      pointerLib->unit = 1e-6;
      pointerLib->precision = 1e-9;

      Cell *PointerCell = (Cell *)allocate_clear(sizeof(Cell));
      PointerCell->name = &cm->cellName[0];
      pointerLib->cell_array.append(PointerCell);
      Cell &cell = *PointerCell; // reference to cell

      double parameters[] = {d,
                             w,
                             s,
                             n};

      try
      {

        // check license
        this->check(d, w, cm);
        // cell gen
        this->genCell(cell, d, w, (cm->wUnd) * w, s, cm->ps, cm->pl, n, cm->geometry, cm->topM);
        // merge
        this->merge(cell, cm);
        // ground shield
        this->patternedGroundShield(cell, cm, d);

        std::string gdsFileName = this->getGdsFileName(cm, parameters);
        std::string emxOptions = this->getEmxOptions(cm);
        counter++;

        if (cm->generateSpiceModel == 1 or cm->generateEm == 1) // model generator
        {

          //******************************************************************************************
          // mutex protected: START
          //******************************************************************************************
          mutex.lock();
          this->verbose(cm, std::to_string(counter), parameters);
          // emx
          this->writeEmxShFile(runEmxShFile, cm, gdsFileName, emxOptions);
          // maestro file
          if (cm->generateSpiceModel == 1)
          {
            this->writeMaestroFile(maestroFile, cm, gdsFileName);
          }

          mutex.unlock();
          //******************************************************************************************
          // mutex protected: END
          //******************************************************************************************
        }

        this->writeGdsFile(pointerLib, cm, gdsFileName);

      } // try
      catch (SoftError exception)
      {
        // ...code that handles SoftError...

        if (cm->generateEm == 0 and cm->generateSpiceModel == 1) // no sweep
        {
          mutex.lock();
          std::cout << exception.name << endl;
          mutex.unlock();
        }
        else
        {
          // verbose

          mutex.lock();
          this->verbose(cm, "Warning: not feasible!", parameters);
          mutex.unlock();
        }
      }

      // clear
      cell.clear();
      pointerLib->clear();
      free(pointerLib);
    } // try
    catch (HardError exception)
    {
      mutex.lock();
      // ...code that handles HardError...
      std::cout << exception.name << endl;
      // clear and exit
      mutex.unlock();
      exit(1);
    }
    catch (...)
    {
      mutex.lock();
      std::cout << "Unknown exception." << endl;
      mutex.unlock();
      exit(1);
    }
  }; // lambda

  std::vector<std::array<double, 4>> paramArray;
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->s.size() * cm->n.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
      {
        for (double n = cm->n.min; n <= cm->n.max; n = n + cm->n.step)
        {

          paramArray.push_back({d, w, s, n});

        } // for n

      } // for s

    } // for w

  } // for d

  auto loopFunction = [this, &paramArray, &threadFunc](const uint64_t a, const uint64_t b)
  {
    for (size_t k = a; k < b; k++)
    {
      const std::array<double, 4> param = paramArray[k];
      threadFunc(param[0], param[1], param[2], param[3]);
    }
  };

  //**************************************************************************************************
  // number of threads
  //**************************************************************************************************

  const size_t numThread = this->threadNumber(cm);

  if (numThread > 1)
  {
    size_t blockNum = std::ceil(paramArray.size() / this->numTasksPerThread);
    blockNum = (blockNum > numThread ? blockNum : numThread);

    BS::thread_pool pool(numThread);
    pool.detach_blocks<size_t>(0, paramArray.size(), loopFunction, blockNum);
    pool.wait();
  }
  else
  {
    loopFunction(0, paramArray.size());
  }

  // close file
  std::cout << endl;
  runEmxShFile.close();
  maestroFile.close();

  return 0;
}

int InductorSpiral::FastHenryGenCell(FastHenry *fh, double d, double w, double wu, double s, double sp, double lp, double n, int geometry, string topM)
{
  // ###############################################################################################
  // # common mode inductor
  // #		: generate cell for gds file
  // ###############################################################################################
  // # d 				: outer diameter
  // # w				: line width
  // # wu      		: line width for underpass layer
  // # s 				: space between lines
  // # sp 			: space between pins
  // # lp           	: pin length (default should be 2w)
  // # n 				: number of turns
  // # geometry		: 0-> octagonal, else rectangular
  // # topM			: top metal layer (metal name)
  // ###############################################################################################

  //  ########################################################
  //  # Parameters:
  //  #            w  : float
  //  #           segment width
  //  #            r  : float
  //  #                 outer radius
  //  #     quadrant: integer
  //  #               quadrant to draw segment (0 - 3)
  //  #       *****************
  //  #       * 1 * 0 *
  //  #       *****************
  //  #       * 2 * 3 *
  //  #       *****************
  //  #   geometry: integer
  //  #         0-> octagonal, else rectangular
  //  #     layerName: string
  //  #         metal name from tech file
  //  ########################################################

  LayerMet topMId = this->l->getMet(topM);
  string bottomM = this->l->t.getMetName(topMId.metNum - 1);

  double grid = this->l->t.getGrid();

  d = 2 * this->l->round2grid(0.5 * d);
  w = 2 * this->l->round2grid(0.5 * w);

  // leave a bit more space for 45 deg lines
  if (s >= topMId.minS and s <= topMId.minS + 2 * grid)
  {
    s = 2 * this->l->round2grid(0.5 * s + grid);
  }
  else
  {
    s = 2 * this->l->round2grid(0.5 * s);
  }

  if (sp <= 0)
    sp = 2 * w;
  if (lp <= 0)
    lp = 2 * w;

  sp = 2 * this->l->round2grid(0.5 * sp);
  lp = 2 * this->l->round2grid(0.5 * lp);
  wu = 2 * this->l->round2grid(0.5 * wu);

  // ###############################################################################################
  // # geometry check
  // ###############################################################################################

  n = round(n / 0.25) * 0.25;

  if (wu < w) // underpass layer can not be less wide than w
  {
    wu = w;
  }

  this->drc(d, w, wu, s, sp, lp, n, geometry, topM);

  // #####################################################################################################
  // #        *   *
  // #        *   *
  // #        *   *
  // #     p1 ***** p2 ------------------
  // #          |                 |
  // #          |    e            |
  // #          <-------->|       | b
  // #                    |       |
  // #               p3 ***** p4 --------
  // #                  *   *
  // #                  *<->*
  // #                  * w *
  // #####################################################################################################

  // double e=w+s;
  // double b=e+w/(1+sqrt(2))+2*topMId.minS;

  // ###############################################################################################
  // # 1. segment
  // ###############################################################################################

  //  # Parameters:
  //  #            w  : float
  //  #           segment width
  //  #            r  : float
  //  #                 outer radius
  //  #     quadrant: integer
  //  #               quadrant to draw segment (0 - 3)
  //  #       *****************
  //  #       * 1 * 0 *
  //  #       *****************
  //  #       * 2 * 3 *
  //  #       *****************
  //  #   geometry: integer
  //  #         0-> octagonal, else rectangular
  //  #     layerName: string
  //  #         metal name from tech file
  //  ########################################################

  std::vector<Segment> path0, path1, path2, path3;

  // ###############################################################################################
  // # point for cutting at left and right side for bridge45
  // ###############################################################################################

  Point2 p1l = Point2{-0.5 * d - w, 0.5 * sp};
  Point2 p2l = Point2{0, -0.5 * sp};

  std::vector<Segment> path1Arr;
  // Array<Polygon*> path2Arr={0};

  int nInt = floor(n);
  double nMod = n - nInt;

  int flegFirstTime = 1;

  double dn = d;

  // ###############################################################################################
  // # loop
  // ###############################################################################################

  for (int i = 0; i < nInt; i++)
  {
    fh->l.drawSegment(path0, dn, w, 0, geometry, topM);
    fh->l.drawSegment(path1, dn, w, 1, geometry, topM);
    fh->l.drawSegment(path2, dn, w, 2, geometry, topM);
    fh->l.drawSegment(path3, dn, w, 3, geometry, topM);

    if (flegFirstTime == 1 and nMod == 0) // pins are at the same side and they should be separated by sp
    {
      fh->l.crop(path1Arr, path1, p1l, p2l, topM);
      fh->addSegment(path1Arr);
    }
    else
    {
      fh->addSegment(path1);
    }
    flegFirstTime = 0;

    // path2 needs to be shifted
    fh->l.translate(path2, Point2{s + w, 0});

    fh->l.crop(path1Arr, path2, Point2{0, 0}, Point2{0.5 * d, -0.5 * d}, topM);

    if (nMod == 0 and nInt - 1 == i) // last segment if ports are at the same side
    {
      fh->l.crop(path2, path1Arr, p1l, p2l, topM);
      fh->addSegment(path2);
    }
    else
    {
      fh->addSegment(path1Arr);
    }

    fh->addSegment(path0);
    fh->addSegment(path3);

    dn = dn - 2 * (w + s);

  } // for

  Point2 p1, p2;
  Point2 pinPlus, pinMinus;

  if (nMod > 0)
  {
    fh->l.drawSegment(path1, dn, w, 1, geometry, topM);
    fh->addSegment(path1);
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Point2{-0.5 * d - lp, 0.5 * w};
    p2 = Point2{-0.5 * d + w, -0.5 * w};

    fh->l.rectangle(path1, p1, p2, 0, topM);
    fh->addSegment(path1);

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    pinPlus = Point2(-0.5 * d - lp, 0); // topM
  }
  else
  {
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Point2{-0.5 * d - lp, w + 0.5 * sp};
    p2 = Point2{-0.5 * d + w, 0.5 * sp};

    fh->l.rectangle(path1, p1, p2, 0, topM);
    fh->addSegment(path1);

    p1 = Point2{-0.5 * d - lp, -wu - 0.5 * sp};
    p2 = Point2{-0.5 * dn + w, -0.5 * sp};

    fh->l.rectangle(path1, p1, p2, 0, bottomM);
    fh->addSegment(path1);

    fh->l.fillVias(path1Arr, Point2{-0.5 * dn + wu, -0.5 * sp}, Point2{-0.5 * dn, -0.5 * sp - wu}, topM, bottomM);
    fh->addSegment(path1Arr);

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    pinPlus = Point2(-0.5 * d - lp, 0.5 * (sp + w));    // topM
    pinMinus = Point2(-0.5 * d - lp, -0.5 * (sp + wu)); // bottomM
    fh->addPort(pinPlus, pinMinus, topM, bottomM);

    return 0;
  }

  if (nMod > 0.25)
  {
    fh->l.drawSegment(path0, dn, w, 0, geometry, topM);
    fh->addSegment(path0);
  }
  else
  {
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Point2{-0.5 * wu, 0.5 * dn - w};
    p2 = Point2{0.5 * wu, 0.5 * d + lp};

    fh->l.rectangle(path1, p1, p2, 1, bottomM);
    fh->addSegment(path1);

    fh->l.fillVias(path1Arr, Point2{-0.5 * wu, 0.5 * dn - wu}, Point2{0.5 * wu, 0.5 * dn}, topM, bottomM);

    fh->addSegment(path1Arr);

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    pinMinus = Point2(0, 0.5 * d + lp); // bottomM
    fh->addPort(pinPlus, pinMinus, topM, bottomM);

    return 0;
  }

  if (nMod > 0.5)
  {
    fh->l.drawSegment(path3, dn, w, 3, geometry, topM);
    fh->addSegment(path3);
  }
  else
  {
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Point2{0.5 * dn - w, -0.5 * wu};
    p2 = Point2{0.5 * d + lp, 0.5 * wu};

    fh->l.rectangle(path1, p1, p2, 0, bottomM);
    fh->addSegment(path1);

    fh->l.fillVias(path1Arr, Point2{0.5 * dn - wu, -0.5 * wu}, Point2{0.5 * dn, 0.5 * wu}, topM, bottomM);

    fh->addSegment(path1Arr);

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    pinMinus = Point2(0.5 * d + lp, 0); // bottomM
    fh->addPort(pinPlus, pinMinus, topM, bottomM);

    return 0;
  }

  if (nMod == 0.75)
  {
    // ###############################################################################################
    // # add contacts
    // ###############################################################################################

    p1 = Point2{0, -0.5 * dn + wu};
    p2 = Point2{wu, -0.5 * d - lp};

    fh->l.rectangle(path1, p1, p2, 1, bottomM);
    fh->addSegment(path1);

    fh->l.fillVias(path1Arr, Point2{0, -0.5 * dn + wu}, Point2{wu, -0.5 * dn}, topM, bottomM);
    fh->addSegment(path1Arr);

    // ###############################################################################################
    // # add label
    // ###############################################################################################

    pinMinus = Point2(0.5 * wu, -(0.5 * d + lp)); // bottomM
    fh->addPort(pinPlus, pinMinus, topM, bottomM);
  }

  return 0;
}

int InductorSpiral::FastHenryProcess(Command *cm)
{

  // ###############################################################################################
  // # FastHenryTransformer
  // #		: process command line options
  // ###############################################################################################

  ofstream runFastHenryShFile;
  //**************************************************************************************************
  // set gds and cell name
  //**************************************************************************************************
  this->setName(cm);
  //**************************************************************************************************
  // counter
  //**************************************************************************************************
  unsigned long int counter = 0;
  //**************************************************************************************************
  // make folders
  //**************************************************************************************************
  this->makeFastHenryFolders(cm);
  //**************************************************************************************************
  // open fastH.sh or fastH2.sh file
  //**************************************************************************************************
  this->openFastHenryShFile(runFastHenryShFile, cm);

  //**************************************************************************************************
  // mutex for shared memory
  //**************************************************************************************************
  std::mutex syncronization_flag;

  //**************************************************************************************************
  // lambda function for threads
  //**************************************************************************************************

  auto threadFunc = [this, &syncronization_flag, &counter, &runFastHenryShFile, cm](double d, double w, double s, double n)
  {
    try
    {

      FastHenry *fh = new FastHenry(*(this->fhl));
      double parameters[] = {d,
                             w,
                             s,
                             n};

      try
      {

        // check license
        this->check(d, w, cm);
        this->FastHenryGenCell(fh, d, w, (cm->wUnd) * w, s, cm->ps, cm->pl, n, cm->geometry, cm->topM);

        std::string gdsFileName = this->getGdsFileName(cm, parameters);
        counter++;
        //*********************************************************************************************
        // mutex lock
        //*********************************************************************************************
        syncronization_flag.lock();
        this->verbose(cm, std::to_string(counter), parameters);
        this->writeFastHenryShFile(runFastHenryShFile, cm, gdsFileName);
        syncronization_flag.unlock();
        //*********************************************************************************************
        // mutex unlock
        //*********************************************************************************************

        this->writeFastHenryMeshFile(fh, cm, gdsFileName);

      } // try
      catch (SoftError exception)
      {

        syncronization_flag.lock();
        this->verbose(cm, "Warning: not feasible!", parameters);

        syncronization_flag.unlock();
      }

      fh->clear();
      delete fh;
    }
    catch (HardError exception)
    {
      // ...code that handles HardError...
      syncronization_flag.lock();
      std::cout << exception.name << endl;
      syncronization_flag.unlock();
      // clear and exit
      exit(1);
    }
    catch (...)
    {
      syncronization_flag.lock();
      std::cout << "Unknown exception." << endl;
      syncronization_flag.unlock();
      exit(1);
    }
  };

  //**************************************************************************************************
  std::vector<std::array<double, 4>> paramArray;
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->s.size() * cm->n.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
      {
        for (double n = cm->n.min; n <= cm->n.max; n = n + cm->n.step)
        {

          paramArray.push_back({d, w, s, n});
          // threadFunc(d,w,s,n);

        } // for n

      } // for s

    } // for w

  } // for d

  auto loopFunction = [this, &paramArray, &threadFunc](const uint64_t a, const uint64_t b)
  {
    for (size_t k = a; k < b; k++)
    {
      const std::array<double, 4> param = paramArray[k];
      threadFunc(param[0], param[1], param[2], param[3]);
    }
  };

  //**************************************************************************************************
  // number of threads
  //**************************************************************************************************

  const size_t numThread = this->threadNumber(cm);

  if (numThread > 1)
  {
    size_t blockNum = std::ceil(paramArray.size() / this->numTasksPerThread);
    blockNum = (blockNum > numThread ? blockNum : numThread);

    BS::thread_pool pool(numThread);
    pool.detach_blocks<size_t>(0, paramArray.size(), loopFunction, blockNum);
    pool.wait();
  }
  else
  {
    loopFunction(0, paramArray.size());
  }
  // close file
  std::cout << endl;
  runFastHenryShFile.close();

  return 0;
}

// ###############################################################################################
// # get all emx options including ports name
// ###############################################################################################
std::string InductorSpiral::getEmxOptions(Command *cm)
{
  std::string emxOptions;

  emxOptions = " --port=p1=PLUS --port=p2=MINUS ";

  if (cm->gndM != "")
  {
    emxOptions = emxOptions + " --port=p3=SHIELD";
  }

  return emxOptions + " " + cm->emxOptions + " ";
}

void InductorSpiral::initMaestroFile(ofstream &maestroFile, Command *cm)
{

  maestroFile.open(cm->projectPath + "/yFile/maestro");

  string modelName = this->name;

  if (cm->gndM != "")
  {
    modelName = modelName + "_shield";
    maestroFile << "name " << modelName << endl;
    maestroFile << "description \"spiral inductor\"" << endl;
    maestroFile << "type shield_inductor" << endl;
  }
  else
  {
    maestroFile << "name " << modelName << endl;
    maestroFile << "description \"spiral inductor\"" << endl;
    maestroFile << "type inductor" << endl;
  }

  maestroFile << "parameter outlength=d scaling 1e-6 rounding 0.1" << endl;
  maestroFile << "parameter metalw=w scaling 1e-6 rounding 0.1" << endl;
  maestroFile << "parameter space scaling 1e-6" << endl;
  maestroFile << "parameter turns" << endl;
  maestroFile << "terminal PLUS=p1" << endl;
  maestroFile << "terminal MINUS=p2" << endl;

  if (cm->gndM != "")
  {
    maestroFile << "terminal SHIELD=shield" << endl;
  }

  maestroFile << "terminal GND=gnd" << endl;

  // modelgen run script

  this->makeModelGenSh(cm);
}

void InductorSpiral::ifSweep(Command *cm)
{
  if (not(cm->w.max == cm->w.min and cm->d.max == cm->d.min and cm->s.max == cm->s.min and cm->n.min == cm->n.max))
  {
    cm->generateEm = 1; // sweep
  }
}

// ###############################################################################################
// # round all parameters and set default values if not set
// ###############################################################################################
void InductorSpiral::roundParameters(Command *cm)
{
  //**************************************************************************************************
  // round number of turns
  //**************************************************************************************************

  if (cm->n.step < 0.25)
    cm->n.step = 0.25;

  cm->n.step = round(cm->n.step / 0.25) * 0.25;
  cm->n.min = round(cm->n.min / 0.25) * 0.25;
  cm->n.max = round(cm->n.max / 0.25) * 0.25;

  //**************************************************************************************************
  // underpass width
  //**************************************************************************************************

  if (cm->wUnd < 1)
  {
    cm->wUnd = 1;
  }
}

// ###############################################################################################
// # write message to std::cout
// ##############################################################################################
void InductorSpiral::verbose(Command *cm, const std::string &message, double parameters[])
{
  double d = parameters[0];
  double w = parameters[1];
  double s = parameters[2];
  double n = parameters[3];

  string dStr = std::to_string(d);
  dStr = dStr.substr(0, dStr.find(".") + 3);

  string wStr = std::to_string(w);
  wStr = wStr.substr(0, wStr.find(".") + 3);

  string sStr = std::to_string(s);
  sStr = sStr.substr(0, sStr.find(".") + 3);

  string nStr = std::to_string(n);
  nStr = nStr.substr(0, nStr.find(".") + 3);

  std::cout << endl
            << std::left;
  std::cout << std::setw(70) << "inductor-spiral: d=" + dStr + " w=" + wStr + " s=" + sStr + " n=" + nStr;
  std::cout << std::left << std::setw(30) << message;
}

// ###############################################################################################
// # gds file name for modelgen
// ###############################################################################################
std::string InductorSpiral::getGdsFileName(Command *cm, double parameters[])
{
  double d = parameters[0];
  double w = parameters[1];
  double s = parameters[2];
  double n = parameters[3];

  string gdsFileName = cm->gdsFile;

  if (cm->generateSpiceModel == 1)
  {
    string tmp = std::to_string(d);
    tmp = tmp.substr(0, tmp.find(".") + 3);
    gdsFileName = gdsFileName + "_outlength" + tmp;

    tmp = std::to_string(w);
    tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
    gdsFileName = gdsFileName + "_metalw" + tmp;

    tmp = std::to_string(s);
    tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
    gdsFileName = gdsFileName + "_space" + tmp;

    tmp = std::to_string(n);
    tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
    gdsFileName = gdsFileName + "_turns" + tmp;
  }
  else if (cm->generateEm == 1 or cm->fastHenry > 0)
  {
    string tmp = std::to_string(d);
    tmp = tmp.substr(0, tmp.find(".") + 3);
    gdsFileName = gdsFileName + "_d" + tmp;

    tmp = std::to_string(w);
    tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
    gdsFileName = gdsFileName + "_w" + tmp;

    tmp = std::to_string(cm->wUnd);
    tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
    gdsFileName = gdsFileName + "_wu" + tmp;

    tmp = std::to_string(s);
    tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
    gdsFileName = gdsFileName + "_s" + tmp;

    tmp = std::to_string(n);
    tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
    gdsFileName = gdsFileName + "_n" + tmp;

    tmp = std::to_string(cm->ps);
    tmp = tmp.substr(0, tmp.find(".") + 3);
    gdsFileName = gdsFileName + "_ps" + tmp;

    tmp = std::to_string(cm->pl);
    tmp = tmp.substr(0, tmp.find(".") + 3);
    gdsFileName = gdsFileName + "_pl" + tmp;
  }

  return gdsFileName;
}
