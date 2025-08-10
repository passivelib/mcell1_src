// gdsInd.cpp
#include "mcell_inductor_symmetric.h"

#include "mcell_error.h"
#include "mcell_global.h"
#include "gdstk.h"

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
// using namespace std;

int InductorSymmetric::threadNumber(Command *cm)
{

  int numberOfThreads = 0;

  if (cm->parallel == 0)
  {
    numberOfThreads = std::thread::hardware_concurrency();
  }
  else if (cm->parallel == -1 or cm->parallel == 1)
  {
    numberOfThreads = 1;
  }
  else
  {
    numberOfThreads = cm->parallel;
  }

  return numberOfThreads > 0 ? numberOfThreads : 1;
}

int InductorSymmetric::drc(double d, double w, double s, double sp, double lp, int n, double wtp, int geometry, string topM)
{

  // #####################################################################################################
  // #        *   *
  // #        *   *
  // #        *   *
  // #     p1 ***** p2 ------------------         ******
  // #          |                 |               *  w *
  // #          |    e            |               *----*
  // #          <-------->|       | b             *      *
  // #                    |       |                 *      *
  // #               p3 ***** p4 --------             *      * | = w/(1+sqrt(2))
  // #                  *   *                           *----*
  // #                  *<->*                           *    *
  // #                  * w *                           ******
  // #####################################################################################################

  LayerMet topMId = this->l->getMet(topM);
  LayerMet bottomMId = this->l->t.getMet(topMId.metNum - 1);
  LayerVia viaId = this->l->getVia(topM, bottomMId.name);

  // ###############################################################################################
  // # geometry check
  // ###############################################################################################

  // add grid to some expressions to avoid drc problems due to round2grid()

  double grid = 2 * this->l->t.getGrid();

  double e = w + s;
  double b = e + w / (1 + sqrt(2)) + topMId.minS + grid;
  b = 2 * this->l->round2grid(0.5 * b);

  // ###############################################################################################
  // # metal width
  // ###############################################################################################

  if (w < topMId.minW or w < bottomMId.minW)
  {
    throw SoftError("Warning: metal width < minW!");
  }

  // ###############################################################################################
  // # metal space
  // ###############################################################################################

  if (s < topMId.minS or s < bottomMId.minS)
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

  // inner diameter
  double dn = d - 2 * (n - 1) * (w + s) - 2 * w;

  if (dn < topMId.minS or dn < bottomMId.minS)
  {
    throw SoftError("Warning: inner diameter too small!");
  }

  // ###############################################################################################
  // # check pins
  // ###############################################################################################

  if (sp < topMId.minS or sp < bottomMId.minS)
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

  double k = this->l->k(d - 2 * w);

  if (geometry == 1 and d - 2 * w < (sp + 2 * w)) // rect
  {
    throw SoftError("Warning: pin space too wide!");
  }

  if (geometry == 0 and 2 * k < (sp + 2 * w)) // oct
  {
    throw SoftError("Warning: pin space too wide!");
  }

  // ###############################################################################################
  // # check bridge45
  // ###############################################################################################

  double kn = this->l->k(dn);

  if (n > 1 and geometry == 0) // oct
  {
    if (2 * kn < b + 2 * w)
    {
      throw SoftError("Warning: no enough space for bridge45!");
    }
  }

  if (n > 1 and geometry == 1) // rect
  {
    if (dn < b + 2 * w)
    {
      throw SoftError("Warning: no enough space for bridge45!");
    }
  }

  // ###############################################################################################
  // # tapped inductor
  // ###############################################################################################

  if (wtp > 0)
  {

    double dnOut = d - 2 * (n - 1) * (w + s); // outer diameter
    double knOut = (sqrt(2) - 1) * 0.5 * dn;

    if (geometry == 0) //  octagonal inductor
    {
      if (n > 2) // bBottom metal layer is used
      {
        LayerMet bBottomMId = this->l->t.getMet(topMId.metNum - 2);

        if (bBottomMId.minW > wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }

        if (2 * knOut < wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }
      }
      else if (n == 1) // top metal layer is used for tapping
      {

        if (topMId.minW > wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }

        if (2 * knOut < wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }
      }
      else if (n == 2)
      {

        if (topMId.minW > wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }

        if (sp - 2 * topMId.minS < wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }
      }
    }
    else // rectangular inductor
    {

      if (n == 2)
      {

        if (topMId.minW > wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }

        if (sp - 2 * topMId.minS < wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }
      }

      else if (n == 1)
      {
        if (topMId.minW > wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }

        if (dnOut < wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }
      }

      else
      {
        LayerMet bBottomMId = this->l->t.getMet(topMId.metNum - 2);
        if (bBottomMId.minW > wtp)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }

        if (dnOut < wtp + grid)
        {
          throw SoftError("Warning: ind can not be tapped!");
        }
      }
    }

  } // tapped

  return 0;
}

int InductorSymmetric::genCell(Cell &result, double d, double w, double s, double sp, double lp, int n, double wtp, int geometry, string topM, bool fasterCap)
{
  // ###############################################################################################
  // # symmetric inductor
  // ###############################################################################################
  // # d        : outer diameter
  // # w        : line width
  // # s        : space between lines
  // # sp       : space between pins
  // # lp             : pin length (default should be 2w)
  // # n        : number of turns
  // # wtp          : width of line for tapped inductor if wtp<=0 no tapping
  // # geometry   : 0-> octagonal, else rectangular
  // # topM     : top metal layer (metal name)
  // ###############################################################################################

  // round to grid

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
  {
    if (wtp <= 0)
    {
      sp = 2 * w;
    }
    else
    {
      sp = wtp + 2 * (topMId.minS + 2 * grid);
    }
  }
  if (lp <= 0)
    lp = 2 * w;

  sp = 2 * this->l->round2grid(0.5 * sp);
  lp = 2 * this->l->round2grid(0.5 * lp);
  wtp = 2 * this->l->round2grid(0.5 * wtp);

  // ###############################################################################################
  // # geometry check
  // ###############################################################################################

  this->drc(d, w, s, sp, lp, n, wtp, geometry, topM);

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

  double e = w + s;
  double b = e + w / (1 + sqrt(2)) + topMId.minS;
  b = 2 * this->l->round2grid(0.5 * b);

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

  Vec2 p1r = Vec2{0, 0.5 * b};
  Vec2 p2r = Vec2{0.5 * d + w, -0.5 * b};
  Vec2 p1l = Vec2{-0.5 * d - w, 0.5 * b};
  Vec2 p2l = Vec2{0, -0.5 * b};

  // fasterCap
  Vec2 p1c_w = Vec2{d + lp, -0.5 * w};
  Vec2 p2c_w = Vec2{-(d + lp), 0.5 * w};

  Array<Polygon *> path1Arr = {0};
  Array<Polygon *> path2Arr = {0};

  // ###############################################################################################
  // # right side 0 and 3
  // ###############################################################################################

  this->l->drawSegment(path0, d, w, 0, geometry, topM);
  this->l->drawSegment(path3, d, w, 3, geometry, topM);

  this->l->crop(path1Arr, path0, p1r, p2r, topM);
  this->l->crop(path2Arr, path3, p1r, p2r, topM);

  result.polygon_array.append(path1Arr[0]);
  // result.polygon_array.append(path2Arr[0]);

  path3 = path2Arr[0];

  path1Arr.clear();
  path2Arr.clear();

  // path0=path1Arr[0];

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # left side 1 and 2
  // ###############################################################################################

  this->l->drawSegment(path1, d, w, 1, geometry, topM);
  this->l->drawSegment(path2, d, w, 2, geometry, topM);

  Vec2 p1, p2;

  result.polygon_array.append(path3);

  p1 = Vec2{-0.5 * d, 0.5 * sp};
  p2 = Vec2{-0.5 * d + w, -0.5 * sp};

  this->l->crop(path1Arr, path1, p1, p2, topM);
  this->l->crop(path2Arr, path2, p1, p2, topM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # add contacts
  // ###############################################################################################

  p1 = Vec2{-0.5 * d - lp, w + 0.5 * sp};
  p2 = Vec2{-0.5 * d + w, 0.5 * sp};

  this->l->rectangle(path1, p1, p2, topM);
  result.polygon_array.append(path1);

  p1 = Vec2{-0.5 * d - lp, -w - 0.5 * sp};
  p2 = Vec2{-0.5 * d + w, -0.5 * sp};

  this->l->rectangle(path1, p1, p2, topM);
  result.polygon_array.append(path1);

  // ###############################################################################################
  // # add label
  // ###############################################################################################

  Label *label;

  this->l->label(label, "PLUS", Vec2{-0.5 * d - lp, 0.5 * (sp + w)}, topM);
  result.label_array.append(label);

  this->l->label(label, "MINUS", Vec2{-0.5 * d - lp, -0.5 * (sp + w)}, topM);
  result.label_array.append(label);

  Vec2 p3, p4;

  double dn_1, dn;

  int sideFlag = 1; // 1 is right, -1 is left

  // ###############################################################################################
  // # loop
  // ###############################################################################################

  for (int i = 0; i < n - 1; i++)
  {
    dn_1 = d - 2 * (w + s) * i;
    dn = d - 2 * (w + s) * (i + 1);

    // ###############################################################################################
    // # left side 1 and 2
    // ###############################################################################################

    this->l->drawSegment(path1, dn, w, 1, geometry, topM);
    this->l->drawSegment(path2, dn, w, 2, geometry, topM);

    this->l->crop(path1Arr, path1, p1l, p2l, topM);
    this->l->crop(path2Arr, path2, p1l, p2l, topM);

    result.polygon_array.append(path1Arr[0]);
    result.polygon_array.append(path2Arr[0]);

    path1Arr.clear();
    path2Arr.clear();

    path1Arr = {0};
    path2Arr = {0};

    // ###############################################################################################
    // # right side 0 and 3
    // ###############################################################################################

    this->l->drawSegment(path0, dn, w, 0, geometry, topM);
    this->l->drawSegment(path3, dn, w, 3, geometry, topM);

    this->l->crop(path1Arr, path0, p1r, p2r, topM);
    this->l->crop(path2Arr, path3, p1r, p2r, topM);

    result.polygon_array.append(path1Arr[0]);
    result.polygon_array.append(path2Arr[0]);

    path1Arr.clear();
    path2Arr.clear();

    path1Arr = {0};
    path2Arr = {0};

    // ###############################################################################################
    // # Bridge45
    // ###############################################################################################

    if (sideFlag == 1) // right
    {

      // #####################################################################################################
      // #        *   *
      // #        *   *
      // #        *   *
      // #     p1 ***** p2 ------------------
      // #          |                  |
      // #          |    e             |
      // #          <--------->|       | b
      // #                     |       |
      // #                 p3***** p4 --------
      // #                   *   *
      // #                   *<->*
      // #                   * w *
      // #####################################################################################################

      p1 = Vec2{0.5 * dn - w, 0.5 * b};
      p2 = Vec2{0.5 * dn, 0.5 * b};
      p3 = Vec2{0.5 * dn_1 - w, -0.5 * b};
      p4 = Vec2{0.5 * dn_1, -0.5 * b};

    } // if

    else // left
    {

      // #####################################################################################################
      // #                           *   *
      // #                           *   *
      // #                           *   *
      // #                        p1 ***** p2 -----
      // #                             |          |
      // #                   e         |          |
      // #           |<---------------->          | b
      // #           |                            |
      // #       p3***** p4 -----------------------
      // #         *   *
      // #         *<->*
      // #         * w *
      // #####################################################################################################

      p1 = Vec2{-0.5 * dn, 0.5 * b};
      p2 = Vec2{-0.5 * dn + w, 0.5 * b};
      p3 = Vec2{-0.5 * dn_1, -0.5 * b};
      p4 = Vec2{-0.5 * dn_1 + w, -0.5 * b};

      // cout<<"p1="<<"("<<p1.x<<","<<p1.y<<")"<<endl;
      // cout<<"p2="<<"("<<p2.x<<","<<p2.y<<")"<<endl;
      // cout<<"p3="<<"("<<p3.x<<","<<p3.y<<")"<<endl;
      // cout<<"p4="<<"("<<p4.x<<","<<p4.y<<")"<<endl;

    } // else

    this->l->bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);

    this->l->appendArrToCell(result, path1Arr);

    sideFlag = sideFlag * (-1);

  } // for

  dn = d - 2 * (w + s) * (n - 1);

  // line of simetry

  if (sideFlag == 1) // right
  {

    this->l->rectangle(path0, Vec2{0.5 * dn, 0.5 * b}, Vec2{0.5 * dn - w, -0.5 * b}, topM);
    // fasterCap
    if (fasterCap)
    {
      this->l->crop(path1Arr, path0, p1c_w, p2c_w, topM);

      result.polygon_array.append(path1Arr[0]);
      result.polygon_array.append(path1Arr[1]);

      path1Arr.clear();

      path1Arr = {0};
    }
    else
    {
      result.polygon_array.append(path0);
    }
  }

  else
  {
    this->l->rectangle(path0, Vec2{-0.5 * dn, 0.5 * b}, Vec2{-0.5 * dn + w, -0.5 * b}, topM);

    // fasterCap
    if (fasterCap)
    {
      this->l->crop(path1Arr, path0, p1c_w, p2c_w, topM);

      result.polygon_array.append(path1Arr[0]);
      result.polygon_array.append(path1Arr[1]);

      path1Arr.clear();

      path1Arr = {0};
    }
    else
    {
      result.polygon_array.append(path0);
    }
  }

  // Tapping
  //  needs to be check if wtp is too big in drc

  if (wtp > 0)
  {
    double ltp = w;

    // if (ltp>lp)
    // {
    //   ltp=lp;
    // }

    if (n > 2) // if n>2 bBottomM is used for tap
    {
      string bBottomM = this->l->t.getMetName(topMId.metNum - 2);

      if (sideFlag == 1) // right
      {
        this->l->fillVias(path1Arr, Vec2{0.5 * dn, 0.5 * wtp}, Vec2{0.5 * dn - w, -0.5 * wtp}, bottomM, bBottomM);
        this->l->fillVias(path2Arr, Vec2{0.5 * dn, 0.5 * wtp}, Vec2{0.5 * dn - w, -0.5 * wtp}, topM, bottomM);
        this->l->rectangle(path0, Vec2{0.5 * dn - w, -0.5 * wtp}, Vec2{0.5 * d + ltp, 0.5 * wtp}, bBottomM);
      }
      else // left
      {
        this->l->fillVias(path1Arr, Vec2{-0.5 * dn, 0.5 * wtp}, Vec2{-0.5 * dn + w, -0.5 * wtp}, bottomM, bBottomM);
        this->l->fillVias(path2Arr, Vec2{-0.5 * dn, 0.5 * wtp}, Vec2{-0.5 * dn + w, -0.5 * wtp}, topM, bottomM);
        this->l->rectangle(path0, Vec2{-0.5 * dn + w, -0.5 * wtp}, Vec2{-0.5 * d - ltp, 0.5 * wtp}, bBottomM);
      }

      result.polygon_array.append(path0);

      this->l->appendArrToCell(result, path1Arr);
      this->l->appendArrToCell(result, path2Arr);

      // add label rot tapping

      Label *tapLabel = NULL;

      if (sideFlag == 1) // right
      {

        this->l->label(tapLabel, string("TAP"), Vec2{0.5 * d + ltp, 0}, bBottomM);
      }
      else
      {

        this->l->label(tapLabel, string("TAP"), Vec2{-0.5 * d - ltp, 0}, bBottomM);
      }

      result.label_array.append(tapLabel);

    } // if n>2

    else // if n<2 topM is used for tapped port
    {
      if (sideFlag == 1) // right
      {

        this->l->rectangle(path0, Vec2{0.5 * dn - w, -0.5 * wtp}, Vec2{0.5 * d + ltp, 0.5 * wtp}, topM);
      }
      else // left
      {
        this->l->rectangle(path0, Vec2{-0.5 * dn + w, -0.5 * wtp}, Vec2{-0.5 * d - ltp, 0.5 * wtp}, topM);
      }

      result.polygon_array.append(path0);

      // add label rot tapping

      Label *tapLabel = NULL;

      if (sideFlag == 1) // right
      {

        this->l->label(tapLabel, string("TAP"), Vec2{0.5 * d + ltp, 0}, topM);
      }
      else
      {
        this->l->label(tapLabel, string("TAP"), Vec2{-0.5 * d - ltp, 0}, topM);
      }

      result.label_array.append(tapLabel);

    } // else n<2

  } // if wtp>0

  return 0;
}

void InductorSymmetric::printHelp()
{
  // ###############################################################################################
  // # symmetrical inductor
  // #    : print command line options for symind
  // #      this function is static so it can be called without any object SymInd::printHelp
  // ###############################################################################################
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--space=number <-s>:" << endl;
  std::cout << "       Set space between turns of inductor. Short option is -s." << endl;
  std::cout << "       --space=5 or -s 5" << endl;
  std::cout << "--space=smin:smax:sstep <-s>:" << endl;
  std::cout << "       Space between turns of inductor is swept from smin to smax with step sstep." << endl;
  std::cout << "       Short option is -s." << endl;
  std::cout << "       --space=2:7:1 or -s 2:7:1" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--number-of-turns=number <-n>:" << endl;
  std::cout << "       Set number of turns of inductor. Short option is -n." << endl;
  std::cout << "       --number-of-turns=3 or -n 3" << endl;
  std::cout << "--number-of-turns=nmin:nmax:nstep <-n>:" << endl;
  std::cout << "       Number of turns of inductor is swept from nmin to nmax with step nstep." << endl;
  std::cout << "       Short option is -n." << endl;
  std::cout << "       --number-of-turns=2:5:1 or -n 2:5:1" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--tapped=number:" << endl;
  std::cout << "--tapped:" << endl;
  std::cout << "       Tapped inductor provided argument is relative tapped width." << endl;
  std::cout << "       If argument is not provided, default is 2." << endl;
  std::cout << "       Absolute tapped width is relative times inductor width." << endl;
  std::cout << "       --tapped=4 //Absolute tapped width=4*w" << endl;
  std::cout << "       --tapped   //Absolute tapped width=2*w" << endl;
  std::cout << "********************************************************************************************" << endl;

  // exit(1);
}

int InductorSymmetric::process(Command *cm)
{

  // symmetrical inductor

  //**************************************************************************************************
  // Round number of turns
  //**************************************************************************************************

  if (cm->n.step < 1)
    cm->n.step = 1;
  cm->n.step = round(cm->n.step);
  cm->n.min = round(cm->n.min);
  cm->n.max = round(cm->n.max);

  //**************************************************************************************************
  // Spice model generation
  //**************************************************************************************************

  ofstream runEmxShFile;
  ofstream maestroFile;

  if (cm->generateSpiceModel == 1)
  {
    system(std::string("mkdir -p " + cm->projectPath + "/gdsFile").c_str());
    system(std::string("rm -rf " + cm->projectPath + "/gdsFile/*").c_str());

    system(std::string("mkdir -p " + cm->projectPath + "/yFile").c_str());
    system(std::string("rm -rf " + cm->projectPath + "/yFile/*").c_str());

    if (cm->geometry == 0) // octagonal
    {
      cm->cellName = "plIndSymOct";
    }
    else if (cm->geometry == 1) // rectangular
    {
      cm->cellName = "plIndSymRect";
    }

    cm->gdsFile = cm->cellName;

    runEmxShFile.open(cm->projectPath + "/runEmx.sh");

    string emxOptions = "\"" + cm->cellName + " " + cm->emxOptions + " ";

    emxOptions = emxOptions + " --port=p1=PLUS --port=p2=MINUS ";

    // strings for maestro file

    string strType, strTerminals, modelName;

    strTerminals = "terminal PLUS=p1\n";
    strTerminals = strTerminals + "terminal MINUS=p2\n";

    // check if tapped and if shield
    modelName = cm->cellName;

    if (cm->gndM == "" and cm->tappedWidth == 0) // there is no shield and no tapping
    {
      emxOptions = emxOptions + "\"";
      strType = "type symmetric_inductor";
    }
    else if (cm->gndM != "" and cm->tappedWidth == 0) // ther is no tapping but there is shield
    {
      emxOptions = emxOptions + " --port=p3=SHIELD\"";
      strTerminals = strTerminals + "terminal SHIELD=shield\n";
      strType = "type symmetric_shield_inductor";
      modelName = modelName + "_shield";
    }
    else if (cm->gndM == "" and cm->tappedWidth != 0) // there is tapping and no shield
    {
      emxOptions = emxOptions + " --port=p3=TAP\"";
      strTerminals = strTerminals + "terminal TAP=p3\n";
      strType = "type center_tapped_inductor";
      modelName = modelName + "_tapped";
    }
    else // there is shield and tapping
    {
      emxOptions = emxOptions + " --port=p3=TAP --port=p4=SHIELD\"";

      strTerminals = strTerminals + "terminal TAP=p3\n";
      strTerminals = strTerminals + "terminal SHIELD=shield\n";
      strType = "type center_tapped_shield_inductor_3sect";
      modelName = modelName + "_tapped_shield";
    }

    runEmxShFile << "#############################################################################" << endl;
    runEmxShFile << "# Set EM options" << endl;
    runEmxShFile << "#############################################################################" << endl;
    runEmxShFile << "export EMOPTIONS=" << emxOptions << endl;
    runEmxShFile << "#setenv EMOPTIONS " << emxOptions << endl;
    runEmxShFile << "#############################################################################" << endl;
    runEmxShFile << "# Set EM path" << endl;
    runEmxShFile << "#############################################################################" << endl;
    runEmxShFile << "export EMPATH=\"" + cm->emxPath + "/emx \"" << endl;
    runEmxShFile << "#setenv EMPATH=\"" + cm->emxPath + "/emx \"" << endl;
    runEmxShFile << "#############################################################################" << endl;

    // maestro file

    maestroFile.open(cm->projectPath + "/yFile/maestro");

    maestroFile << "name " << modelName << endl;
    maestroFile << "description \"symmetrical inductor\"" << endl;

    // type
    maestroFile << strType << endl;

    maestroFile << "parameter outlength=d scaling 1e-6 rounding 0.1" << endl;
    maestroFile << "parameter metalw=w scaling 1e-6 rounding 0.1" << endl;
    maestroFile << "parameter space scaling 1e-6" << endl;
    maestroFile << "parameter turns" << endl;

    // terminals
    maestroFile << strTerminals;
    maestroFile << "terminal GND=gnd" << endl;

    // run model gen
    ofstream runModelgenShFile;
    runModelgenShFile.open(cm->projectPath + "/runModelgen.sh");
    runModelgenShFile << "#############################################################################" << endl;
    runModelgenShFile << "# Set modelgen path" << endl;
    runModelgenShFile << "#############################################################################" << endl;
    runModelgenShFile << "export MODELGENPATH=\"" + cm->emxPath + "/modelgen \"" << endl;
    runModelgenShFile << "#setenv MODELGENPATH \"" + cm->emxPath + "/modelgen \"" << endl;
    runModelgenShFile << "$MODELGENPATH --continuum " + cm->projectPath + "/yFile/maestro --spectre-file=" << modelName << ".scs --interpolate-responses --max-processes=2" << endl;
    runModelgenShFile.close();
    chmod((cm->projectPath + "/runModelgen.sh").c_str(), 0777);
  }

  //**************************************************************************************************
  // Define library
  //**************************************************************************************************

  if (cm->cellName == "")
  {

    if (cm->gdsFile == "")
    {
      if (cm->geometry == 0) // octagonal
      {
        cm->cellName = "plIndSymOct";
      }
      else if (cm->geometry == 1) // rectangular
      {
        cm->cellName = "plIndSymRect";
      }
    }
    else
    {
      cm->cellName = cm->gdsFile;
    }
  }

  //**************************************************************************************************
  // gds file
  //**************************************************************************************************

  if (cm->gdsFile == "")
  {

    cm->gdsFile = cm->cellName;
  }

  unsigned long int counter = 0;

  //**************************************************************************************************
  // sweep
  //**************************************************************************************************

  int sweepFlag = 1; // sweep for at least for one parameter

  if (cm->w.max == cm->w.min and cm->d.max == cm->d.min and cm->s.max == cm->s.min and cm->n.min == cm->n.max)
  {
    sweepFlag = 0; // no sweep
  }

  if ((sweepFlag == 1 or cm->generateEm == 1) and cm->generateSpiceModel == 0) // sweep but no generateSpiceModel
  {
    system(std::string("mkdir -p " + cm->projectPath + "/gdsFile").c_str());
    system(std::string("rm -rf " + cm->projectPath + "/gdsFile/*").c_str());

    system(std::string("mkdir -p " + cm->projectPath + "/yFile").c_str());
    system(std::string("rm -rf " + cm->projectPath + "/yFile/*").c_str());

    runEmxShFile.open(cm->projectPath + "/runEmx.sh");
  }

  //**************************************************************************************************
  // mutex for shared memory
  //**************************************************************************************************
  std::mutex mutex;

  //**************************************************************************************************
  // lambda function for threads
  //**************************************************************************************************
  auto threadFunc = [this, &mutex, &counter, &runEmxShFile, &maestroFile, cm, sweepFlag](double d, double w, double s, double n)
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

      try
      {

        // check license
        if (not(*(cm->lv)))
        {

          d = std::rand() % 1000;
          w = std::rand() % 20;
        }

        // verbose
        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string nStr = std::to_string(n);
        nStr = nStr.substr(0, nStr.find(".") + 3);

        if (cm->tappedWidth < -2)
        {
          cm->tappedWidth = 2;
        }

        this->genCell(cell, d, w, s, cm->ps, cm->pl, n, (cm->tappedWidth) * w, cm->geometry, cm->topM, false);

        if (not(sweepFlag == 1 or cm->generateEm == 1))
        { // merge if no sweep
          this->l->merge(cell);
        }

        if (cm->gndM != "") // patterned ground shield
        {
          Array<Polygon *> gndShield = {0};

          this->l->patternedGroundShield(gndShield, cm->gndD + d, cm->gndW, cm->gndS, cm->gndGeometry, cm->gndM);
          this->l->appendArrToCell(cell, gndShield);

          // Label and pin

          Label *gndLabel = NULL;

          this->l->label(gndLabel, "SHIELD", Vec2{-0.5 * (cm->gndD + d), 0}, this->l->t.getMetNamePP(cm->gndM));

          cell.label_array.append(gndLabel);
        }

        if (cm->generateSpiceModel == 1) // model generator
        {
          string gdsFileName = cm->gdsFile;
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

          //******************************************************************************************
          // mutex protected: START
          //******************************************************************************************
          mutex.lock();

          std::cout << endl
                    << std::left;
          std::cout << std::setw(70) << "inductor-symmetric: d=" + dStr + " w=" + wStr + " s=" + sStr + " n=" + nStr;
          counter++;
          std::cout << std::left << std::setw(30) << counter;

          runEmxShFile << "echo \"**************************************************************\"" << endl;
          runEmxShFile << "echo \"Iteration: " << counter << "\"" << endl;
          runEmxShFile << "echo \"**************************************************************\"" << endl;
          runEmxShFile << "$EMPATH " << cm->projectPath + "/gdsFile/" << gdsFileName << ".gds"
                       << " $EMOPTIONS --y-file=" + cm->projectPath + "/yFile/" << gdsFileName << ".y" << endl;
          // maestro file
          maestroFile << "file " + cm->projectPath + "/yFile/" << gdsFileName << ".y" << endl;
          mutex.unlock();
          //******************************************************************************************
          // mutex protected: END
          //******************************************************************************************

          gdsFileName = cm->projectPath + "/gdsFile/" + gdsFileName;

          gdsFileName = gdsFileName + ".gds";
          pointerLib->write_gds(&gdsFileName[0], 0, NULL);
        }

        else if (sweepFlag == 1 or cm->generateEm == 1)
        {
          string gdsFileName = cm->gdsFile;

          string tmp = std::to_string(d);
          tmp = tmp.substr(0, tmp.find(".") + 3);
          gdsFileName = gdsFileName + "_d" + tmp;

          tmp = std::to_string(w);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_w" + tmp;

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

          std::string emxOptions = " --port=p1=PLUS --port=p2=MINUS ";

          if (cm->gndM != "" and cm->tappedWidth == 0) // ther is no tapping but there is shield
          {
            emxOptions = emxOptions + " --port=p3=SHIELD";
          }
          else if (cm->gndM == "" and cm->tappedWidth != 0) // there is tapping and no shield
          {
            emxOptions = emxOptions + " --port=p3=TAP";
          }
          else if (cm->gndM != "" and cm->tappedWidth != 0) // there is shield and tapping
          {
            emxOptions = emxOptions + " --port=p3=TAP --port=p4=SHIELD";
          }

          //******************************************************************************************
          // mutex protected: START
          //******************************************************************************************
          mutex.lock();

          std::cout << endl
                    << std::left;
          std::cout << std::setw(70) << "inductor-symmetric: d=" + dStr + " w=" + wStr + " s=" + sStr + " n=" + nStr;
          counter++;
          std::cout << std::left << std::setw(30) << counter;

          runEmxShFile << cm->emxPath + "/emx " + cm->projectPath + "/gdsFile/" << gdsFileName << ".gds"
                       << " " << cm->cellName + " " + cm->emxOptions + " ";
          runEmxShFile << " " << emxOptions + " ";
          runEmxShFile << "--y-file=" + cm->projectPath + "/yFile/" << gdsFileName << ".y" << endl;

          mutex.unlock();
          //******************************************************************************************
          // mutex protected: END
          //******************************************************************************************

          gdsFileName = cm->projectPath + "/gdsFile/" + gdsFileName;
          gdsFileName = gdsFileName + ".gds";
          pointerLib->write_gds(&gdsFileName[0], 0, NULL);
        }
        else // no sweep, file name is different
        {
          string gdsFileName = cm->gdsFile + ".gds";
          pointerLib->write_gds(&gdsFileName[0], 0, NULL);
        }

      } // try
      catch (SoftError exception)
      {
        // ...code that handles SoftError...
        if (sweepFlag == 0) // no sweep
        {
          mutex.lock();
          std::cout << exception.name << endl;
          mutex.unlock();
        }
        else
        {
          // verbose
          string dStr = std::to_string(d);
          dStr = dStr.substr(0, dStr.find(".") + 3);

          string wStr = std::to_string(w);
          wStr = wStr.substr(0, wStr.find(".") + 3);

          string sStr = std::to_string(s);
          sStr = sStr.substr(0, sStr.find(".") + 3);

          string nStr = std::to_string(n);
          nStr = nStr.substr(0, nStr.find(".") + 3);

          mutex.lock();
          std::cout << endl
                    << std::left;
          std::cout << std::setw(70) << "inductor-symmetric: d=" + dStr + " w=" + wStr + " s=" + sStr + " n=" + nStr;
          std::cout << std::left << std::setw(30) << "Warning: not feasible!";
          mutex.unlock();
        }
      }
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
  chmod((cm->projectPath + "/runEmx.sh").c_str(), 0777);
  maestroFile.close();

  return 0;
}

int InductorSymmetric::FastHenryGenCell(FastHenry *fh, double d, double w, double s, double sp, double lp, int n, double wtp, int geometry, string topM, bool FastHenry2)
{
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
  {
    if (wtp <= 0)
    {
      sp = 2 * w;
    }
    else
    {
      sp = wtp + 2 * (topMId.minS + 2 * grid);
    }
  }
  if (lp <= 0)
    lp = 2 * w;

  sp = 2 * this->l->round2grid(0.5 * sp);
  lp = 2 * this->l->round2grid(0.5 * lp);
  wtp = 2 * this->l->round2grid(0.5 * wtp);

  // ###############################################################################################
  // # geometry check
  // ###############################################################################################

  this->drc(d, w, s, sp, lp, n, wtp, geometry, topM);

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

  double e = w + s;
  double b = e + w / (1 + sqrt(2)) + topMId.minS;
  b = 2 * this->l->round2grid(0.5 * b);

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

  Point2 p1r = Point2{0, 0.5 * b};
  Point2 p2r = Point2{0.5 * d + w, -0.5 * b};
  Point2 p1l = Point2{-0.5 * d - w, 0.5 * b};
  Point2 p2l = Point2{0, -0.5 * b};

  // fastHenry2
  Point2 p1c_w = Point2{d + lp, -0.5 * w};
  Point2 p2c_w = Point2{-(d + lp), 0.5 * w};

  std::vector<Segment> path1Arr;
  std::vector<Segment> path2Arr;

  // ###############################################################################################
  // # right side 0 and 3
  // ###############################################################################################

  fh->l.drawSegment(path0, d, w, 0, geometry, topM);
  fh->l.drawSegment(path3, d, w, 3, geometry, topM);

  fh->l.crop(path1Arr, path0, p1r, p2r, topM);
  fh->l.crop(path2Arr, path3, p1r, p2r, topM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  // ###############################################################################################
  // # left side 1 and 2
  // ###############################################################################################

  fh->l.drawSegment(path1, d, w, 1, geometry, topM);
  fh->l.drawSegment(path2, d, w, 2, geometry, topM);

  Point2 p1, p2;

  p1 = Point2{-0.5 * d, 0.5 * sp};
  p2 = Point2{-0.5 * d + w, -0.5 * sp};

  // ###############################################################################################
  // # cut turn so that overlapping in-between turn and contact extension is smaller
  // ###############################################################################################
  // ###############################################################################################
  //  #
  //  #     k
  //  #   -------
  //  #   *******
  //  #           *   2k
  //  #             *
  //  #               *
  //  #                 * |
  //  #                 * |
  //  #     0.5*d       * | k
  //  #   <------------>* |
  //  #
  //  #     k=(sqrt(2)-1)*0.5*d

  // internal k for primary
  double kp = this->l->k(d - 2 * w);

  Point2 p1cp = p1;
  Point2 p2cp = p2;
  if (geometry == 1 and sp + w < d - 2 * w)
  {
    p1cp.y = -0.5 * (sp + w);
    p2cp.y = 0.5 * (sp + w);
  }

  if (geometry == 0 and sp + w < 2 * kp)
  {
    p1cp.y = -0.5 * (sp + w);
    p2cp.y = 0.5 * (sp + w);
  }

  fh->l.crop(path1Arr, path1, p1cp, p2cp, topM);
  fh->l.crop(path2Arr, path2, p1cp, p2cp, topM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  // ###############################################################################################
  // # add contacts
  // ###############################################################################################

  p1 = Point2{-0.5 * d - lp, w + 0.5 * sp};
  p2 = Point2{-0.5 * d + w, 0.5 * sp};

  fh->l.rectangle(path1, p1, p2, 0, topM);
  fh->addSegment(path1);

  p1 = Point2{-0.5 * d - lp, -w - 0.5 * sp};
  p2 = Point2{-0.5 * d + w, -0.5 * sp};

  fh->l.rectangle(path1, p1, p2, 0, topM);
  fh->addSegment(path1);

  // ###############################################################################################
  // # add label
  // ###############################################################################################
  Point2 pinPlus = Point2{-0.5 * d - lp, 0.5 * (sp + w)};   // topM
  Point2 pinMinus = Point2{-0.5 * d - lp, -0.5 * (sp + w)}; // bottomM
  // fh->addPort(pinPlus, pinMinus, topM,topM);

  Point2 p3, p4;

  double dn_1, dn;

  int sideFlag = 1; // 1 is right, -1 is left

  // ###############################################################################################
  // # loop
  // ###############################################################################################

  for (int i = 0; i < n - 1; i++)
  {
    dn_1 = d - 2 * (w + s) * i;
    dn = d - 2 * (w + s) * (i + 1);

    // ###############################################################################################
    // # left side 1 and 2
    // ###############################################################################################

    fh->l.drawSegment(path1, dn, w, 1, geometry, topM);
    fh->l.drawSegment(path2, dn, w, 2, geometry, topM);

    fh->l.crop(path1Arr, path1, p1l, p2l, topM);
    fh->l.crop(path2Arr, path2, p1l, p2l, topM);

    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);

    // ###############################################################################################
    // # right side 0 and 3
    // ###############################################################################################

    fh->l.drawSegment(path0, dn, w, 0, geometry, topM);
    fh->l.drawSegment(path3, dn, w, 3, geometry, topM);

    fh->l.crop(path1Arr, path0, p1r, p2r, topM);
    fh->l.crop(path2Arr, path3, p1r, p2r, topM);

    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);

    // ###############################################################################################
    // # Bridge45
    // ###############################################################################################

    if (sideFlag == 1) // right
    {

      // #####################################################################################################
      // #        *   *
      // #        *   *
      // #        *   *
      // #     p1 ***** p2 ------------------
      // #          |                  |
      // #          |    e             |
      // #          <--------->|       | b
      // #                     |       |
      // #                 p3***** p4 --------
      // #                   *   *
      // #                   *<->*
      // #                   * w *
      // #####################################################################################################

      p1 = Point2{0.5 * dn - w, 0.5 * b};
      p2 = Point2{0.5 * dn, 0.5 * b};
      p3 = Point2{0.5 * dn_1 - w, -0.5 * b};
      p4 = Point2{0.5 * dn_1, -0.5 * b};

    } // if
    else // left
    {

      // #####################################################################################################
      // #                           *   *
      // #                           *   *
      // #                           *   *
      // #                        p1 ***** p2 -----
      // #                             |          |
      // #                   e         |          |
      // #           |<---------------->          | b
      // #           |                            |
      // #       p3***** p4 -----------------------
      // #         *   *
      // #         *<->*
      // #         * w *
      // #####################################################################################################

      p1 = Point2{-0.5 * dn, 0.5 * b};
      p2 = Point2{-0.5 * dn + w, 0.5 * b};
      p3 = Point2{-0.5 * dn_1, -0.5 * b};
      p4 = Point2{-0.5 * dn_1 + w, -0.5 * b};
    } // else

    fh->l.bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);

    fh->addSegment(path1Arr);

    sideFlag = sideFlag * (-1);

  } // for

  dn = d - 2 * (w + s) * (n - 1);

  Point2 pinPlusN;
  Point2 pinMinusN;

  if (sideFlag == 1) // right
  {

    // fastHenry2
    if (FastHenry2)
    {
      fh->l.rectangle(path0, Point2{0.5 * dn, 0.5 * b}, Point2{0.5 * dn - w, 0.5 * w}, 1, topM);
      fh->addSegment(path0);
      fh->l.rectangle(path0, Point2{0.5 * dn, -0.5 * w}, Point2{0.5 * dn - w, -0.5 * b}, 1, topM);
      fh->addSegment(path0);

      // port
      pinPlusN.x = 0.5 * (dn - w);
      pinPlusN.y = 0.5 * w;

      pinMinusN.x = 0.5 * (dn - w);
      pinMinusN.y = -0.5 * w;
    }
    else
    {
      fh->l.rectangle(path0, Point2{0.5 * dn, 0.5 * b}, Point2{0.5 * dn - w, -0.5 * b}, 1, topM);
      fh->addSegment(path0);
    }
  }

  else
  {

    // FastHenry2
    if (FastHenry2)
    {
      fh->l.rectangle(path0, Point2{-0.5 * dn, 0.5 * b}, Point2{-0.5 * dn + w, 0.5 * w}, 1, topM);
      fh->addSegment(path0);
      fh->l.rectangle(path0, Point2{-0.5 * dn, -0.5 * w}, Point2{-0.5 * dn + w, -0.5 * b}, 1, topM);
      fh->addSegment(path0);

      // port
      pinPlusN.x = -0.5 * (dn - w);
      pinPlusN.y = -0.5 * w;

      pinMinusN.x = -0.5 * (dn - w);
      pinMinusN.y = 0.5 * w;
    }
    else
    {
      fh->l.rectangle(path0, Point2{-0.5 * dn, 0.5 * b}, Point2{-0.5 * dn + w, -0.5 * b}, 1, topM);
      fh->addSegment(path0);
    }
  }

  // Add ports
  // Point2 pinPlus=Point2{-0.5*d-lp, 0.5*(sp+w)}; //topM
  // Point2 pinMinus=Point2{-0.5*d-lp, -0.5*(sp+w)}; //bottomM

  if (FastHenry2)
  {

    fh->addPort(pinPlus, pinPlusN, topM, topM);
    fh->addPort(pinMinusN, pinMinus, topM, topM);
  }
  else
  {
    fh->addPort(pinPlus, pinMinus, topM, topM);
  }

  // Tapping
  //  needs to be check if wtp is too big in drc

  if (wtp > 0)
  {
    double ltp = w;

    // if (ltp>lp)
    // {
    //   ltp=lp;
    // }

    if (n > 2) // if n>2 bBottomM is used for tap
    {
      string bBottomM = this->l->t.getMetName(topMId.metNum - 2);
      if (sideFlag == 1) // right
      {
        fh->l.fillVias(path1Arr, Point2{0.5 * dn, 0.5 * wtp}, Point2{0.5 * dn - w, -0.5 * wtp}, bottomM, bBottomM);
        fh->l.fillVias(path2Arr, Point2{0.5 * dn, 0.5 * wtp}, Point2{0.5 * dn - w, -0.5 * wtp}, topM, bottomM);
        fh->l.rectangle(path0, Point2{0.5 * dn - w, -0.5 * wtp}, Point2{0.5 * d + ltp, 0.5 * wtp}, 0, bBottomM);
      }
      else // left
      {
        fh->l.fillVias(path1Arr, Point2{-0.5 * dn, 0.5 * wtp}, Point2{-0.5 * dn + w, -0.5 * wtp}, bottomM, bBottomM);
        fh->l.fillVias(path2Arr, Point2{-0.5 * dn, 0.5 * wtp}, Point2{-0.5 * dn + w, -0.5 * wtp}, topM, bottomM);
        fh->l.rectangle(path0, Point2{-0.5 * dn + w, -0.5 * wtp}, Point2{-0.5 * d - ltp, 0.5 * wtp}, 0, bBottomM);
      }

      fh->addSegment(path0);
      fh->addSegment(path1Arr);
      fh->addSegment(path2Arr);

    } // if n>2

    else // if n<2 topM is used for tapped port
    {
      if (sideFlag == 1) // right
      {

        fh->l.rectangle(path0, Point2{0.5 * dn - w, -0.5 * wtp}, Point2{0.5 * d + ltp, 0.5 * wtp}, 0, topM);
      }
      else // left
      {
        fh->l.rectangle(path0, Point2{-0.5 * dn + w, -0.5 * wtp}, Point2{-0.5 * d - ltp, 0.5 * wtp}, 0, topM);
      }

      fh->addSegment(path0);

    } // else n<2

  } // if wtp>0

  return 0;
}

int InductorSymmetric::FastHenryProcess(Command *cm)
{
  if (cm->n.step < 1)
    cm->n.step = 1;
  cm->n.step = round(cm->n.step);
  cm->n.min = round(cm->n.min);
  cm->n.max = round(cm->n.max);

  ofstream runFastHenryShFile;

  if (cm->geometry == 0) // octagonal
  {
    cm->cellName = "plIndSymOct";
  }
  else if (cm->geometry == 1) // rectangular
  {
    cm->cellName = "plIndSymRect";
  }
  cm->gdsFile = cm->cellName;

  unsigned long int counter = 0;

  system(std::string("mkdir -p " + cm->projectPath + "/fastHenryFile").c_str());
  system(std::string("rm -rf " + cm->projectPath + "/fastHenryFile/*").c_str());

  system(std::string("mkdir -p " + cm->projectPath + "/zFile").c_str());
  system(std::string("rm -rf " + cm->projectPath + "/zFile/*").c_str());

  runFastHenryShFile.open(cm->projectPath + "/runFastH.sh");

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

      try
      {

        // check license
        if (not(*(cm->lv)))
        {

          d = std::rand() % 1000;
          w = std::rand() % 20;
        }

        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string nStr = std::to_string(n);
        nStr = nStr.substr(0, nStr.find(".") + 3);

        if (cm->tappedWidth < 0)
        {
          cm->tappedWidth = 2;
        }

        this->FastHenryGenCell(fh, d, w, s, cm->ps, cm->pl, n, (cm->tappedWidth) * w, cm->geometry, cm->topM, false);

        string gdsFileName = cm->gdsFile;

        string tmp = std::to_string(d);
        tmp = tmp.substr(0, tmp.find(".") + 3);
        gdsFileName = gdsFileName + "_d" + tmp;

        tmp = std::to_string(w);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_w" + tmp;

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

        //*********************************************************************************************
        // mutex lock
        //*********************************************************************************************
        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(70) << "inductor-symmetric: d=" + dStr + " w=" + wStr + " s=" + sStr + " n=" + nStr;
        counter++;
        std::cout << std::left << std::setw(30) << counter;
        runFastHenryShFile << cm->passiveLibPath + "/bin/" + std::string(PL_MACRO_FASTHENRY_NAME) + " " + cm->projectPath + "/fastHenryFile/" << gdsFileName << ".inp  -z " + cm->projectPath + "/zFile/" << gdsFileName << ".zp" << std::endl;
        syncronization_flag.unlock();
        //*********************************************************************************************
        // mutex unlock
        //*********************************************************************************************

        gdsFileName = cm->projectPath + "/fastHenryFile/" + gdsFileName;
        gdsFileName = gdsFileName + ".inp";

        fh->createEqualNodes();
        fh->writeToFile(gdsFileName.c_str(), cm->freq, cm->meshWidth);

      } // try
      catch (SoftError exception)
      {

        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string nStr = std::to_string(n);
        nStr = nStr.substr(0, nStr.find(".") + 3);

        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(70) << "inductor-symmetric: d=" + dStr + " w=" + wStr + " s=" + sStr + " n=" + nStr;
        std::cout << std::left << std::setw(30) << "Warning: not feasible!";
        syncronization_flag.unlock();
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

      delete fh;

    } // try
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
  runFastHenryShFile.close();
  chmod((cm->projectPath + "/runFastH.sh").c_str(), 0777);

  return 0;
}

int InductorSymmetric::FastHenry2Process(Command *cm)
{
  if (cm->n.step < 1)
    cm->n.step = 1;
  cm->n.step = round(cm->n.step);
  cm->n.min = round(cm->n.min);
  cm->n.max = round(cm->n.max);

  ofstream runFastHenryShFile;

  if (cm->geometry == 0) // octagonal
  {
    cm->cellName = "plIndSymOct";
  }
  else if (cm->geometry == 1) // rectangular
  {
    cm->cellName = "plIndSymRect";
  }
  cm->gdsFile = cm->cellName;

  unsigned long int counter = 0;

  system(std::string("mkdir -p " + cm->projectPath + "/fastHenry2File").c_str());
  system(std::string("rm -rf " + cm->projectPath + "/fastHenry2File/*").c_str());

  system(std::string("mkdir -p " + cm->projectPath + "/zFile2").c_str());
  system(std::string("rm -rf " + cm->projectPath + "/zFile2/*").c_str());

  runFastHenryShFile.open(cm->projectPath + "/runFastH2.sh");

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

      try
      {

        // check license
        if (not(*(cm->lv)))
        {

          d = std::rand() % 1000;
          w = std::rand() % 20;
        }

        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string nStr = std::to_string(n);
        nStr = nStr.substr(0, nStr.find(".") + 3);

        if (cm->tappedWidth < 0)
        {
          cm->tappedWidth = 2;
        }

        this->FastHenryGenCell(fh, d, w, s, cm->ps, cm->pl, n, (cm->tappedWidth) * w, cm->geometry, cm->topM, true);

        string gdsFileName = cm->gdsFile;

        string tmp = std::to_string(d);
        tmp = tmp.substr(0, tmp.find(".") + 3);
        gdsFileName = gdsFileName + "_d" + tmp;

        tmp = std::to_string(w);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_w" + tmp;

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

        //*********************************************************************************************
        // mutex lock
        //*********************************************************************************************
        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(70) << "inductor-symmetric: d=" + dStr + " w=" + wStr + " s=" + sStr + " n=" + nStr;
        counter++;
        std::cout << std::left << std::setw(30) << counter;
        runFastHenryShFile << cm->passiveLibPath + "/bin/" + std::string(PL_MACRO_FASTHENRY_NAME) + " " + cm->projectPath + "/fastHenry2File/" << gdsFileName << ".inp  -z " + cm->projectPath + "/zFile2/" << gdsFileName << ".zp" << std::endl;
        syncronization_flag.unlock();
        //*********************************************************************************************
        // mutex unlock
        //*********************************************************************************************

        gdsFileName = cm->projectPath + "/fastHenry2File/" + gdsFileName;
        gdsFileName = gdsFileName + ".inp";

        fh->createEqualNodes();
        fh->writeToFile(gdsFileName.c_str(), cm->freq, cm->meshWidth);

      } // try
      catch (SoftError exception)
      {

        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string nStr = std::to_string(n);
        nStr = nStr.substr(0, nStr.find(".") + 3);

        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(70) << "inductor-symmetric: d=" + dStr + " w=" + wStr + " s=" + sStr + " n=" + nStr;
        std::cout << std::left << std::setw(30) << "Warning: not feasible!";
        syncronization_flag.unlock();
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

      delete fh;

    } // try
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
  runFastHenryShFile.close();
  chmod((cm->projectPath + "/runFastH2.sh").c_str(), 0777);

  return 0;
}
