#include "mcell_error.h"
#include "gdstk.h"
#include "mcell_transformer2o1.h"
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

int Transformer2o1::threadNumber(Command *cm)
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

int Transformer2o1::drc(double dp, double ds, double wp, double ws, double s, double sp, double lp, double wtp, double wts, int geometry, string topM)
{

  // ###############################################################################################
  // # transformer1o2
  // # 		: check if geometry is feasible
  // ###############################################################################################
  // # dp 			: outer diameter of primary
  // # ds 			: outer diameter of secondary
  // # wp				: line width of primary
  // # ws				: secondary line width
  // # ss 			: shift between primary and secondary
  // # sp 			: space between pins
  // # lp           	: pin length (default should be 2w)
  // # wtp          	: width of line for tapped primary if wtp<=0 no tapping
  // # wts          	: width of line for tapped secondary if wts<=0 no tapping
  // # geometry		: 0-> octagonal, 1 -> rectangular
  // # topM			: top metal layer (metal name)
  // ###############################################################################################

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

  LayerMet topMId = this->l->getMet(topM);
  string bottomM = this->l->t.getMetName(topMId.metNum - 1);
  LayerMet bottomMId = this->l->getMet(bottomM);

  // longer distance from top and bottom
  double minS = topMId.minS > bottomMId.minS ? topMId.minS : bottomMId.minS;

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
  //  #         0-> octagonal, 1-> rectangular
  //  #     layerName: string
  //  #         metal name from tech file
  //  ########################################################

  // ###############################################################################################
  // # point for cutting at left and right side for bridge45
  // ###############################################################################################

  double dmax = dp; // max(dp,ds)

  if (dmax < ds)
  {
    dmax = ds;
  }
  // ###############################################################################################
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
  // #                  * ws*
  // ###############################################################################################
  double e = (wp + s);
  e = 2 * this->l->round2grid(0.5 * e);

  // cuting for bridge45
  double b = e + wp / (1 + sqrt(2)) + minS;
  b = 2 * this->l->round2grid(0.5 * b);

  // ###############################################################################################
  // # round to grid
  // ###############################################################################################

  dp = 2 * this->l->round2grid(0.5 * dp);
  ds = 2 * this->l->round2grid(0.5 * ds);
  dmax = 2 * this->l->round2grid(0.5 * dmax);
  wp = 2 * this->l->round2grid(0.5 * wp);
  ws = 2 * this->l->round2grid(0.5 * ws);
  s = 2 * this->l->round2grid(0.5 * s);

  if (sp <= 0)
    sp = 2 * (ws + minS) + b;
  if (lp <= 0)
    lp = 2 * wp;

  sp = 2 * this->l->round2grid(0.5 * sp);
  lp = 2 * this->l->round2grid(0.5 * lp);

  // wtp and wts can be <0 and than tapping is not done
  wtp = 2 * this->l->round2grid(0.5 * wtp);
  wts = 2 * this->l->round2grid(0.5 * wts);

  // ###############################################################################################
  // # metal width
  // ###############################################################################################

  if (wp < topMId.minW or wp < bottomMId.minW or ws < bottomMId.minW)
  {
    throw SoftError("Warning: metal width < minW!");
  }

  // ###############################################################################################
  // # metal space
  // ###############################################################################################

  if (s < topMId.minS)
  {
    throw SoftError("Warning: metal space < minS!");
  }

  // ###############################################################################################
  // # inner radius smaller than 2*mins
  // ###############################################################################################

  if (ds - 2 * ws < 2 * bottomMId.minS)
  {
    throw SoftError("Warning: inner diameter too small!");
  }
  double dp2 = dp - 4 * wp - 2 * s; // inner primary diameter

  if (dp2 < 2 * topMId.minS)
  {
    throw SoftError("Warning: inner diameter too small!");
  }

  // ###############################################################################################
  // # check bridge45x2
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

  double kp2 = this->l->k(dp2);

  if (geometry == 0) // oct
  {
    if (2 * kp2 < b + 2 * wp)
    {
      throw SoftError("Warning: no enough space for bridge45x2!");
    }
  }

  if (geometry != 0) // rect
  {
    if (dp2 < b + 2 * wp)
    {
      throw SoftError("Warning: no enough space for bridge45x2!");
    }
  }

  if (sp < b + 2 * wp + 2 * bottomMId.minS and 0.5 * dp > 0.5 * ds - ws - bottomMId.minS)
  {
    throw SoftError("Warning: pin space too short!");
  }

  // ###############################################################################################
  // # check pins
  // ###############################################################################################

  if (sp < topMId.minS or sp < bottomMId.minS)
  {
    throw SoftError("Warning: pin space < minS!");
  }

  if (lp < topMId.minW or lp < bottomMId.minW)
  {
    throw SoftError("Warning: pin length < minW!");
  }

  if (wtp > 0 and sp < wtp + 2 * topMId.minS)
  {
    throw SoftError("Warning: pin space too short!");
  }

  //  ########################################################
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

  // double d2=d-2*(w+s)-2*w;
  double kp = this->l->k(dp - 2 * wp);
  double ks = this->l->k(ds - 2 * ws);

  if (geometry != 0 and (dp - 2 * wp < (sp + 2 * wp) or ds - 2 * ws < (sp + 2 * ws))) // rect
  {
    throw SoftError("Warning: pin space too wide!");
  }

  if (geometry == 0 and (2 * kp < (sp + 2 * wp) or 2 * ks < (sp + 2 * ws))) // oct
  {
    throw SoftError("Warning: pin space too wide!");
  }

  // ###############################################################################################
  // # tapped transformer
  // ###############################################################################################

  if (wtp > 0)
  {
    if (wtp < topMId.minS)
    {
      throw SoftError("Warning: wtp too small!");
    }

    if ((geometry != 0) and (dp < wtp))
    {

      throw SoftError("Warning:  wtp too width!");
    }

    if ((geometry == 0) and (2 * kp2 < wtp))
    {

      throw SoftError("Warning:  wtp too width!");
    }
  }

  if (wts > 0)
  {
    if (wts < bottomMId.minS)
    {
      throw SoftError("Warning: wts too small!");
    }

    if ((geometry != 0) and (ds < wts))
    {

      throw SoftError("Warning:  wts too width!");
    }

    if ((geometry == 0) and (2 * ks < wts))
    {

      throw SoftError("Warning:  wts too width!");
    }
  }

  return 0;
}

int Transformer2o1::genCell(Cell &result, double dp, double ds, double wp, double ws, double s, double sp, double lp, double wtp, double wts, int geometry, string topM, bool fasterCap)
{
  // ###############################################################################################
  // # transformer1o2
  // # 		: generate cell
  // ###############################################################################################
  // # dp 			: outer diameter of primary
  // # ds 			: outer diameter of secondary
  // # wp				: line width of primary
  // # ws				: secondary line width
  // # ss 			: shift between primary and secondary
  // # sp 			: space between pins
  // # lp           	: pin length (default should be 2w)
  // # wtp          	: width of line for tapped primary if wtp<=0 no tapping
  // # wts          	: width of line for tapped secondary if wts<=0 no tapping
  // # geometry		: 0-> octagonal, 1 -> rectangular
  // # topM			: top metal layer (metal name)
  // ###############################################################################################

  LayerMet topMId = this->l->getMet(topM);
  string bottomM = this->l->t.getMetName(topMId.metNum - 1);
  LayerMet bottomMId = this->l->getMet(bottomM);

  // longer distance from top and bottom
  double minS = topMId.minS > bottomMId.minS ? topMId.minS : bottomMId.minS;

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
  //  #         0-> octagonal, 1-> rectangular
  //  #     layerName: string
  //  #         metal name from tech file
  //  ########################################################

  Polygon *path0, *path1, *path2, *path3;

  // ###############################################################################################
  // # point for cutting at left and right side for bridge45
  // ###############################################################################################

  // double ds = d-w+s+2*ws-2*ss; //secondary diameter

  double dmax = dp; // max(d,ds)

  if (dmax < ds)
  {
    dmax = ds;
  }
  // ###############################################################################################
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
  // #                  * ws*
  // ###############################################################################################

  // ###############################################################################################
  // # round to grid
  // ###############################################################################################

  dp = 2 * this->l->round2grid(0.5 * dp);
  ds = 2 * this->l->round2grid(0.5 * ds);
  dmax = 2 * this->l->round2grid(0.5 * dmax);
  wp = 2 * this->l->round2grid(0.5 * wp);
  ws = 2 * this->l->round2grid(0.5 * ws);
  s = 2 * this->l->round2grid(0.5 * s);

  double e = (wp + s);

  // cutting for bridge45
  double b = e + wp / (1 + sqrt(2)) + minS;
  b = 2 * this->l->round2grid(0.5 * b);

  if (sp <= 0)
    sp = 2 * (wp + minS) + b;
  if (lp <= 0)
    lp = 2 * wp;

  sp = 2 * this->l->round2grid(0.5 * sp);
  lp = 2 * this->l->round2grid(0.5 * lp);

  // wtp and wts can be <0 and than tapping is not done
  wtp = 2 * this->l->round2grid(0.5 * wtp);
  wts = 2 * this->l->round2grid(0.5 * wts);

  // ###############################################################################################
  // # geometry check DRC
  // ###############################################################################################

  this->drc(dp, ds, wp, ws, s, sp, lp, wtp, wts, geometry, topM);

  Vec2 p1c = Vec2{dp + ds, -0.5 * sp};
  Vec2 p2c = Vec2{-(dp + ds), 0.5 * sp};

  Vec2 p1c_wp = Vec2{dp + ds, -0.5 * wp};
  Vec2 p2c_wp = Vec2{-(dp + ds), 0.5 * wp};

  Vec2 p1c_ws = Vec2{dp + ds, -0.5 * ws};
  Vec2 p2c_ws = Vec2{-(dp + ds), 0.5 * ws};

  Array<Polygon *> path1Arr = {0};
  Array<Polygon *> path2Arr = {0};

  // ###############################################################################################
  // # primary outer
  // ###############################################################################################

  this->l->drawSegment(path0, dp, wp, 0, geometry, topM);
  this->l->drawSegment(path1, dp, wp, 1, geometry, topM);
  this->l->drawSegment(path2, dp, wp, 2, geometry, topM);
  this->l->drawSegment(path3, dp, wp, 3, geometry, topM);

  this->l->crop(path1Arr, path1, p1c, p2c, topM);
  this->l->crop(path2Arr, path2, p1c, p2c, topM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # cutout for bridge45
  // ###############################################################################################
  p1c.y = -0.5 * b;
  p2c.y = 0.5 * b;

  this->l->crop(path1Arr, path0, p1c, p2c, topM);
  this->l->crop(path2Arr, path3, p1c, p2c, topM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # primary contacts
  // ###############################################################################################

  Vec2 p1 = Vec2{-0.5 * dmax - lp, wp + 0.5 * sp};
  Vec2 p2 = Vec2{-0.5 * dp + wp, 0.5 * sp};

  this->l->rectangle(path1, p1, p2, topM);
  result.polygon_array.append(path1);

  p1 = Vec2{-0.5 * dmax - lp, -wp - 0.5 * sp};
  p2 = Vec2{-0.5 * dp + wp, -0.5 * sp};

  this->l->rectangle(path1, p1, p2, topM);
  result.polygon_array.append(path1);

  // ###############################################################################################
  // # primary label
  // ###############################################################################################
  Label *label;

  p1 = Vec2{-0.5 * dmax - lp, 0.5 * (sp + wp)};
  p2 = Vec2{-0.5 * dmax - lp, -0.5 * (sp + wp)};

  this->l->label(label, "P1", p1, topM);
  result.label_array.append(label);

  this->l->label(label, "P2", p2, topM);
  result.label_array.append(label);

  // ###############################################################################################
  // # primary inner turn
  // ###############################################################################################

  this->l->drawSegment(path0, dp - 2 * (wp + s), wp, 0, geometry, topM);
  this->l->drawSegment(path1, dp - 2 * (wp + s), wp, 1, geometry, topM);
  this->l->drawSegment(path2, dp - 2 * (wp + s), wp, 2, geometry, topM);
  this->l->drawSegment(path3, dp - 2 * (wp + s), wp, 3, geometry, topM);

  // fasterCap
  if (fasterCap)
  {
    this->l->crop(path1Arr, path1, p1c_wp, p2c_wp, topM);
    this->l->crop(path2Arr, path2, p1c_wp, p2c_wp, topM);

    result.polygon_array.append(path1Arr[0]);
    result.polygon_array.append(path2Arr[0]);

    path1Arr.clear();
    path2Arr.clear();

    path1Arr = {0};
    path2Arr = {0};
  }
  else
  {
    result.polygon_array.append(path1);
    result.polygon_array.append(path2);
  }

  // ###############################################################################################
  // # cutout for bridge45
  // ###############################################################################################

  this->l->crop(path1Arr, path0, p1c, p2c, topM);
  this->l->crop(path2Arr, path3, p1c, p2c, topM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # bridge45 secondary
  // ###############################################################################################
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
  p1 = Vec2{0.5 * dp - wp, 0.5 * (b)};
  p2 = Vec2{0.5 * dp, 0.5 * (b)};
  Vec2 p3 = Vec2{p1.x - e, -0.5 * (b)};
  Vec2 p4 = Vec2{p2.x - e, -0.5 * (b)};

  this->l->bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
  this->l->appendArrToCell(result, path1Arr);

  // ###############################################################################################
  // # secondary turn
  // ###############################################################################################

  this->l->drawSegment(path0, ds, ws, 0, geometry, bottomM);
  this->l->drawSegment(path1, ds, ws, 1, geometry, bottomM);
  this->l->drawSegment(path2, ds, ws, 2, geometry, bottomM);
  this->l->drawSegment(path3, ds, ws, 3, geometry, bottomM);

  // fasterCap
  if (fasterCap)
  {
    this->l->crop(path1Arr, path1, p1c_ws, p2c_ws, bottomM);
    this->l->crop(path2Arr, path2, p1c_ws, p2c_ws, bottomM);

    result.polygon_array.append(path1Arr[0]);
    result.polygon_array.append(path2Arr[0]);

    path1Arr.clear();
    path2Arr.clear();

    path1Arr = {0};
    path2Arr = {0};
  }
  else
  {
    result.polygon_array.append(path1);
    result.polygon_array.append(path2);
  }

  p1c.y = -0.5 * sp;
  p2c.y = 0.5 * sp;

  this->l->crop(path1Arr, path0, p1c, p2c, bottomM);
  this->l->crop(path2Arr, path3, p1c, p2c, bottomM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # secondary contacts
  // ###############################################################################################

  p1 = Vec2{0.5 * dmax + lp, ws + 0.5 * sp};
  p2 = Vec2{0.5 * ds - ws, 0.5 * sp};

  this->l->rectangle(path1, p1, p2, bottomM);
  result.polygon_array.append(path1);

  p1 = Vec2{0.5 * dmax + lp, -ws - 0.5 * sp};
  p2 = Vec2{0.5 * ds - ws, -0.5 * sp};

  this->l->rectangle(path1, p1, p2, bottomM);
  result.polygon_array.append(path1);

  // ###############################################################################################
  // # secondary label
  // ###############################################################################################

  p1 = Vec2{0.5 * dmax + lp, 0.5 * (sp + ws)};
  p2 = Vec2{0.5 * dmax + lp, -0.5 * (sp + ws)};

  this->l->label(label, "S1", p1, bottomM);
  result.label_array.append(label);

  this->l->label(label, "S2", p2, bottomM);
  result.label_array.append(label);

  // ###############################################################################################
  // # primary tapped
  // ###############################################################################################

  if (wtp > 0)
  {

    p1 = lp > wtp ? Vec2{-0.5 * dmax - wtp, 0.5 * wtp} : Vec2{-0.5 * dmax - lp, 0.5 * wtp};
    p2 = Vec2{-0.5 * dp + 2 * wp + s, -0.5 * wtp};

    this->l->rectangle(path1, p1, p2, topM);
    result.polygon_array.append(path1);

    // ###############################################################################################
    // # label
    // ###############################################################################################
    p1.y = 0;

    this->l->label(label, "TP", p1, topM);
    result.label_array.append(label);
  }

  // ###############################################################################################
  // # secondary tapped
  // ###############################################################################################

  if (wts > 0)
  {

    p1 = lp > wts ? Vec2{-0.5 * dmax - wts, 0.5 * wts} : Vec2{-0.5 * dmax - lp, 0.5 * wts};
    p2 = Vec2{-0.5 * ds + ws, -0.5 * wts};

    // string bBottomM = this->l->t.getMetName(bottomMId.metNum - 1);

    this->l->rectangle(path1, p1, p2, bottomM);
    result.polygon_array.append(path1);

    //  //add vias
    //   this->l->fillVias(path1Arr, p2, Vec2{0.5*ds-ws-s, 0.5*wts}, bottomM, bBottomM);
    //   this->l->appendArrToCell(result, path1Arr);

    // ###############################################################################################
    // # label
    // ###############################################################################################
    p1.y = 0;

    this->l->label(label, "TS", p1, bottomM);
    result.label_array.append(label);
  }

  return 0;
}

void Transformer2o1::printHelp()
{
  // ###############################################################################################
  // # Transformer1o2
  // #    : print command line options for symind
  // #      this function is static so it can be called without any object Transformer1o1::printHelp
  // ###############################################################################################
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--shift-secondary=number:" << endl;
  std::cout << "--sh=number:" << endl;
  std::cout << "       Set space between primary and secondary outer diameter of transformer." << endl;
  std::cout << "       If argument is not provided, default is 0." << endl;
  std::cout << "       --shift-secondary=5 or --sh=5" << endl;
  std::cout << "--sh=smin:smax:sstep:" << endl;
  std::cout << "       Space between primary and secondary diameter is swept from smin to smax with step sstep." << endl;
  std::cout << "       --shift-secondary=2:7:1 or --sh=2:7:1" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--tapped-primary=number:" << endl;
  std::cout << "--tapped-primary:" << endl;
  std::cout << "       Tapped primary of transformer, provided argument is relative tapped width." << endl;
  std::cout << "       If argument is not provided, default is 2." << endl;
  std::cout << "       Absolute tapped width is relative times transformer width." << endl;
  std::cout << "       --tapped-primary=4 //Absolute tapped width=4*w" << endl;
  std::cout << "       --tapped-primary   //Absolute tapped width=2*w" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--tapped-secondary=number:" << endl;
  std::cout << "--tapped-secondary:" << endl;
  std::cout << "       Tapped secondary of transformer, provided argument is relative tapped width." << endl;
  std::cout << "       If argument is not provided, default is 2." << endl;
  std::cout << "       Absolute tapped width is relative times transformer secondary width." << endl;
  std::cout << "       --tapped-secondary=4 //Absolute tapped width=4*ws" << endl;
  std::cout << "       --tapped-secondary   //Absolute tapped width=2*ws" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--width-secondary=number <--ws>:" << endl;
  std::cout << "       Set turns width of secondary transformer. Short option is --ws." << endl;
  std::cout << "       This option is optional, default is ws=w." << endl;
  std::cout << "       --width-secondary=5 or --ws=5" << endl;
  std::cout << "--width-secondary=wmin:wmax:wstep <--ws>:" << endl;
  std::cout << "       Secondary width of turns of transformer is swept from wmin to wmax with step wstep." << endl;
  std::cout << "       Short option is --ws." << endl;
  std::cout << "       --width-secondary=2:7:1 or --ws=2:7:1" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--space=number <-s>:" << endl;
  std::cout << "       Set space between turns of transformer. Short option is -s." << endl;
  std::cout << "       This option is mandatory." << endl;
  std::cout << "       --space=5 or -s 5" << endl;
  std::cout << "--space=smin:smax:sstep <-s>:" << endl;
  std::cout << "       Space between turns of transformer is swept from smin to smax with step sstep." << endl;
  std::cout << "       Short option is -s." << endl;
  std::cout << "       --space=2:7:1 or -s 2:7:1" << endl;
  std::cout << "********************************************************************************************" << endl;

  // exit(1);
}

int Transformer2o1::process(Command *cm)
{

  // Transformer1o1 inductor

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
      cm->cellName = "plTr2o1Oct";
    }
    else if (cm->geometry == 1) // rectangular
    {
      cm->cellName = "plTr2o1Rect";
    }
    cm->gdsFile = cm->cellName;

    runEmxShFile.open(cm->projectPath + "/runEmx.sh");

    string emxOptions = "\"" + cm->cellName + " " + cm->emxOptions + " ";

    emxOptions = emxOptions + " --port=p1=P1 --port=p2=P2 --port=p3=S1 --port=p4=S2  ";

    // strings for maestro file

    string strType, strTerminals, modelName;

    strTerminals = "terminal P1=p1\n";
    strTerminals = strTerminals + "terminal P2=p2\n";
    strTerminals = strTerminals + "terminal S1=p3\n";
    strTerminals = strTerminals + "terminal S2=p4\n";

    // check if tapped and if shield
    modelName = cm->cellName;

    if (cm->gndM == "" and cm->tappedPrimaryWidth == 0 and cm->tappedSecondaryWidth == 0) // there is no shield and no tapping
    {
      emxOptions = emxOptions + "\"";
      strType = "type transformer";
    }
    else if (cm->gndM == "" and cm->tappedSecondaryWidth == 0 and cm->tappedPrimaryWidth != 0) // primary tapped
    {
      emxOptions = emxOptions + " --port=p5=TP\"";
      modelName = modelName + "_ptapped";

      strTerminals = strTerminals + "terminal TP=p5\n";
      strType = "type center_tapped_primary_transformer";
    }
    else if (cm->gndM == "" and cm->tappedSecondaryWidth != 0 and cm->tappedPrimaryWidth == 0) // secondary tapped
    {
      emxOptions = emxOptions + " --port=p5=TS\"";
      modelName = modelName + "_stapped";

      strTerminals = strTerminals + "terminal TS=p5\n";
      strType = "type center_tapped_secondary_transformer";
    }
    else if (cm->gndM == "" and cm->tappedSecondaryWidth != 0 and cm->tappedPrimaryWidth != 0) // primary and secondary tapped
    {
      emxOptions = emxOptions + " --port=p5=TP --port=p6=TS\"";
      modelName = modelName + "_pstapped";

      strTerminals = strTerminals + "terminal TP=p5\n";
      strTerminals = strTerminals + "terminal TS=p6\n";

      strType = "type dual_center_tapped_transformer";
    }
    else if (cm->gndM != "" and cm->tappedSecondaryWidth == 0 and cm->tappedPrimaryWidth == 0) // shield
    {
      emxOptions = emxOptions + " --port=p5=SHIELD\"";
      modelName = modelName + "_shield";

      strTerminals = strTerminals + "terminal SHIELD=shield\n";
      strType = "type shield_complex_xformer";
    }
    else if (cm->gndM != "" and cm->tappedSecondaryWidth == 0 and cm->tappedPrimaryWidth != 0) // shield, primary tapped
    {
      emxOptions = emxOptions + " --port=p5=TP --port=p6=SHIELD\"";
      modelName = modelName + "_ptapped_shield";

      strTerminals = strTerminals + "terminal TP=p5\n";
      strTerminals = strTerminals + "terminal SHIELD=shield\n";
      strType = "type shield_complex_xformer_ptap";
    }
    else if (cm->gndM != "" and cm->tappedSecondaryWidth != 0 and cm->tappedPrimaryWidth == 0) // shield, secondary tapped
    {
      emxOptions = emxOptions + " --port=p5=TS --port=p6=SHIELD\"";
      modelName = modelName + "_stapped_shield";

      strTerminals = strTerminals + "terminal TS=p5\n";
      strTerminals = strTerminals + "terminal SHIELD=shield\n";
      strType = "type shield_complex_xformer_stap";
    }
    else // shield, primary and secondary tapped
    {
      emxOptions = emxOptions + " --port=p5=TP --port=p6=TS --port=p7=SHIELD\"";
      modelName = modelName + "_pstapped_shield";

      strTerminals = strTerminals + "terminal TP=p5\n";
      strTerminals = strTerminals + "terminal TS=p6\n";
      strTerminals = strTerminals + "terminal SHIELD=shield\n";
      strType = "type shield_complex_xformer_btap";
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
    maestroFile << "description \"transformer2o1\"" << endl;

    // type
    maestroFile << strType << endl;

    maestroFile << "parameter outlength=d scaling 1e-6 rounding 0.1" << endl;
    maestroFile << "parameter metalw=w scaling 1e-6 rounding 0.1" << endl;
    maestroFile << "parameter metalws scaling 1e-6" << endl;
    maestroFile << "parameter space scaling 1e-6" << endl;
    maestroFile << "parameter sshift scaling 1e-6" << endl;

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
        cm->cellName = "plTr2o1Oct";
      }
      else if (cm->geometry == 1) // rectangular
      {
        cm->cellName = "plTr2o1Rect";
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

  if (cm->w.max == cm->w.min and cm->d.max == cm->d.min and cm->s.max == cm->s.min and cm->ws.max == cm->ws.min and cm->sh.max == cm->sh.min)
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
  std::mutex syncronization_flag;

  //**************************************************************************************************
  // lambda function for threads
  //**************************************************************************************************
  auto threadFunc = [this, &syncronization_flag, &counter, &runEmxShFile, &maestroFile, cm, sweepFlag](double d, double w, double ws, double s, double sh)
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

        ws = ws <= 0 ? w : ws;
        double ds = d - 2 * w - s + ws - 2 * sh;
        double dmax = (d > ds ? d : ds); // if d>ds then d else ds

        // verbose
        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string wsStr = std::to_string(ws);
        wsStr = wsStr.substr(0, wsStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string ssStr = std::to_string(sh);
        ssStr = ssStr.substr(0, ssStr.find(".") + 3);

        if (cm->tappedPrimaryWidth < -2)
        {
          cm->tappedPrimaryWidth = 2;
        }

        if (cm->tappedSecondaryWidth < -2)
        {
          cm->tappedSecondaryWidth = 2;
        }

        this->genCell(cell, d, ds, w, ws, s, cm->ps, cm->pl, w * cm->tappedPrimaryWidth, ws * cm->tappedSecondaryWidth, cm->geometry, cm->topM, false);

        if (not(sweepFlag == 1 or cm->generateEm == 1))
        { // merge if no sweep
          this->l->merge(cell);
        }

        if (cm->gndM != "") // patterned ground shield
        {
          Array<Polygon *> gndShield = {0};

          this->l->patternedGroundShield(gndShield, cm->gndD + dmax, cm->gndW, cm->gndS, cm->gndGeometry, cm->gndM);
          this->l->appendArrToCell(cell, gndShield);

          // //add pin for ground shield
          // Polygon* tmpRec=NULL;
          // this->l->rectangle(tmpRec, Vec2{-0.5*(cm->gndD+dmax)-w, 0.5*w}, Vec2{-0.5*(cm->gndD+dmax), -0.5*w}, cm->gndM);
          // cell.polygon_array.append(tmpRec);

          // Label and pin

          Label *gndLabel = NULL;

          this->l->label(gndLabel, "SHIELD", Vec2{-0.5 * (cm->gndD + dmax), 0}, this->l->t.getMetNamePP(cm->gndM));

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

          tmp = std::to_string(ws);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_metalws" + tmp;

          tmp = std::to_string(s);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_space" + tmp;

          tmp = std::to_string(sh);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_sshift" + tmp;

          //******************************************************************************************
          // mutex protected: START
          //******************************************************************************************
          syncronization_flag.lock();
          std::cout << endl
                    << std::left;
          std::cout << std::setw(75) << "transformer2o1: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " sh=" + ssStr;
          counter++;
          std::cout << std::left << std::setw(30) << counter;

          runEmxShFile << "echo \"**************************************************************\"" << endl;
          runEmxShFile << "echo \"Iteration: " << counter << "\"" << endl;
          runEmxShFile << "echo \"**************************************************************\"" << endl;

          runEmxShFile << "$EMPATH " << cm->projectPath + "/gdsFile/" << gdsFileName << ".gds"
                       << " $EMOPTIONS --y-file=" + cm->projectPath + "/yFile/" << gdsFileName << ".y" << endl;

          // maestro file

          maestroFile << "file " + cm->projectPath + "/yFile/" << gdsFileName << ".y" << endl;
          syncronization_flag.unlock();
          //*********************************************************************************************
          // mutex unlock
          //*********************************************************************************************

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

          tmp = std::to_string(ws);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_ws" + tmp;

          tmp = std::to_string(s);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_s" + tmp;

          tmp = std::to_string(sh);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_sh" + tmp;

          tmp = std::to_string(cm->ps);
          tmp = tmp.substr(0, tmp.find(".") + 3);
          gdsFileName = gdsFileName + "_ps" + tmp;

          tmp = std::to_string(cm->pl);
          tmp = tmp.substr(0, tmp.find(".") + 3);
          gdsFileName = gdsFileName + "_pl" + tmp;

          string emxOptions = " --port=p1=P1 --port=p2=P2 --port=p3=S1 --port=p4=S2 ";

          if (cm->gndM == "" and cm->tappedSecondaryWidth == 0 and cm->tappedPrimaryWidth != 0) // primary tapped
          {
            emxOptions = emxOptions + " --port=p5=TP";
          }
          else if (cm->gndM == "" and cm->tappedSecondaryWidth != 0 and cm->tappedPrimaryWidth == 0) // secondary tapped
          {
            emxOptions = emxOptions + " --port=p5=TS";
          }
          else if (cm->gndM == "" and cm->tappedSecondaryWidth != 0 and cm->tappedPrimaryWidth != 0) // primary and secondary tapped
          {
            emxOptions = emxOptions + " --port=p5=TP --port=p6=TS";
          }
          else if (cm->gndM != "" and cm->tappedSecondaryWidth == 0 and cm->tappedPrimaryWidth == 0) // shield
          {
            emxOptions = emxOptions + " --port=p5=SHIELD";
          }
          else if (cm->gndM != "" and cm->tappedSecondaryWidth == 0 and cm->tappedPrimaryWidth != 0) // shield, primary tapped
          {
            emxOptions = emxOptions + " --port=p5=TP --port=p6=SHIELD";
          }
          else if (cm->gndM != "" and cm->tappedSecondaryWidth != 0 and cm->tappedPrimaryWidth == 0) // shield, secondary tapped
          {
            emxOptions = emxOptions + " --port=p5=TS --port=p6=SHIELD";
          }
          else if (cm->gndM != "" and cm->tappedSecondaryWidth != 0 and cm->tappedPrimaryWidth != 0)
          {
            emxOptions = emxOptions + " --port=p5=TP --port=p6=TS --port=p7=SHIELD";
          }

          //******************************************************************************************
          // mutex protected: START
          //******************************************************************************************
          syncronization_flag.lock();
          std::cout << endl
                    << std::left;
          std::cout << std::setw(75) << "transformer2o1: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " sh=" + ssStr;
          counter++;
          std::cout << std::left << std::setw(30) << counter;

          runEmxShFile << cm->emxPath + "/emx " + cm->projectPath + "/gdsFile/" << gdsFileName << ".gds"
                       << " " << cm->cellName + " " + cm->emxOptions + " ";
          runEmxShFile << " " << emxOptions + " ";
          runEmxShFile << "--y-file=" + cm->projectPath + "/yFile/" << gdsFileName << ".y" << endl;
          syncronization_flag.unlock();
          //*********************************************************************************************
          // mutex unlock
          //*********************************************************************************************

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
          syncronization_flag.lock();
          std::cout << exception.name << endl;
          syncronization_flag.unlock();
        }
        else
        {
          // verbose
          string dStr = std::to_string(d);
          dStr = dStr.substr(0, dStr.find(".") + 3);

          string wStr = std::to_string(w);
          wStr = wStr.substr(0, wStr.find(".") + 3);

          string wsStr = std::to_string(ws);
          wsStr = wsStr.substr(0, wsStr.find(".") + 3);

          string sStr = std::to_string(s);
          sStr = sStr.substr(0, sStr.find(".") + 3);

          string ssStr = std::to_string(sh);
          ssStr = ssStr.substr(0, ssStr.find(".") + 3);

          syncronization_flag.lock();
          std::cout << endl
                    << std::left;
          std::cout << std::setw(75) << "transformer2o1: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " sh=" + ssStr;
          std::cout << std::left << std::setw(30) << "Warning: not feasible!";
          syncronization_flag.unlock();
        }
      }
      catch (HardError exception)
      {
        syncronization_flag.lock();
        // ...code that handles HardError...
        std::cout << exception.name << endl;
        // clear and exit
        syncronization_flag.unlock();
        exit(1);
      }
      catch (...)
      {
        syncronization_flag.lock();
        std::cout << "Unknown exception." << endl;
        syncronization_flag.unlock();
        exit(1);
      }

      // clear
      cell.clear();
      pointerLib->clear();
      free(pointerLib);

    } // try
    catch (HardError exception)
    {
      syncronization_flag.lock();
      // ...code that handles HardError...
      std::cout << exception.name << endl;
      // clear and exit
      syncronization_flag.unlock();
      exit(1);
    }
    catch (...)
    {
      syncronization_flag.lock();
      std::cout << "Unknown exception." << endl;
      syncronization_flag.unlock();
      exit(1);
    }
  }; // lambda

  std::vector<std::array<double, 5>> paramArray;
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->ws.size() * cm->s.size() * cm->sh.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double ws = cm->ws.min; ws <= cm->ws.max; ws = ws + cm->ws.step)
      {
        for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
        {
          for (double sh = cm->sh.min; sh <= cm->sh.max; sh = sh + cm->sh.step)
          {
            paramArray.push_back({d, w, ws, s, sh});
          } // for sh

        } // for s

      } // for ws

    } // for w

  } // for d

  auto loopFunction = [this, &paramArray, &threadFunc](const uint64_t a, const uint64_t b)
  {
    for (size_t k = a; k < b; k++)
    {
      const std::array<double, 5> param = paramArray[k];
      threadFunc(param[0], param[1], param[2], param[3], param[4]);
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

int Transformer2o1::FastHenryGenCell(FastHenry *fh, double dp, double ds, double wp, double ws, double s, double sp, double lp, double wtp, double wts, int geometry, string topM, bool fasterCap)
{
  // ###############################################################################################
  // # transformer1o2
  // # 		: generate cell
  // ###############################################################################################
  // # dp 			: outer diameter of primary
  // # ds 			: outer diameter of secondary
  // # wp				: line width of primary
  // # ws				: secondary line width
  // # s 			  : shift between primary and secondary
  // # sp 			: space between pins
  // # lp           	: pin length (default should be 2w)
  // # wtp          	: width of line for tapped primary if wtp<=0 no tapping
  // # wts          	: width of line for tapped secondary if wts<=0 no tapping
  // # geometry		: 0-> octagonal, 1 -> rectangular
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
  LayerMet bottomMId = this->l->getMet(bottomM);

  // longer distance from top and bottom
  double minS = topMId.minS > bottomMId.minS ? topMId.minS : bottomMId.minS;

  std::vector<Segment> path0, path1, path2, path3;

  path0.clear();
  path1.clear();
  path2.clear();
  path3.clear();

  double dmax = dp; // max(d,ds)

  if (dmax < ds)
  {
    dmax = ds;
  }

  // ###############################################################################################
  // # round to grid
  // ###############################################################################################

  dp = 2 * this->l->round2grid(0.5 * dp);
  ds = 2 * this->l->round2grid(0.5 * ds);
  dmax = 2 * this->l->round2grid(0.5 * dmax);
  wp = 2 * this->l->round2grid(0.5 * wp);
  ws = 2 * this->l->round2grid(0.5 * ws);
  s = 2 * this->l->round2grid(0.5 * s);

  double e = (wp + s);

  // cutting for bridge45
  double b = e + wp / (1 + sqrt(2)) + minS;
  b = 2 * this->l->round2grid(0.5 * b);

  if (sp <= 0)
    sp = 2 * (wp + minS) + b;
  if (lp <= 0)
    lp = 2 * wp;

  sp = 2 * this->l->round2grid(0.5 * sp);
  lp = 2 * this->l->round2grid(0.5 * lp);

  // wtp and wts can be <0 and than tapping is not done
  wtp = 2 * this->l->round2grid(0.5 * wtp);
  wts = 2 * this->l->round2grid(0.5 * wts);

  // ###############################################################################################
  // # geometry check DRC
  // ###############################################################################################

  this->drc(dp, ds, wp, ws, s, sp, lp, wtp, wts, geometry, topM);

  Point2 p1c = Point2(dp + ds, -0.5 * sp);
  Point2 p2c = Point2(-(dp + ds), 0.5 * sp);

  // fasterCap
  Point2 p1c_wp = Point2{dp + ds, -0.5 * wp};
  Point2 p2c_wp = Point2{-(dp + ds), 0.5 * wp};

  Point2 p1c_ws = Point2{dp + ds, -0.5 * ws};
  Point2 p2c_ws = Point2{-(dp + ds), 0.5 * ws};

  std::vector<Segment> path1Arr;
  std::vector<Segment> path2Arr;

  path1Arr.clear();
  path2Arr.clear();

  // ###############################################################################################
  // # primary outer
  // ###############################################################################################

  fh->l.drawSegment(path0, dp, wp, 0, geometry, topM);
  fh->l.drawSegment(path1, dp, wp, 1, geometry, topM);
  fh->l.drawSegment(path2, dp, wp, 2, geometry, topM);
  fh->l.drawSegment(path3, dp, wp, 3, geometry, topM);

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
  double kp = this->l->k(dp - 2 * wp);

  Point2 p1cp = p1c;
  Point2 p2cp = p2c;
  if (geometry == 1 and sp + wp < dp - 2 * wp)
  {
    p1cp.y = -0.5 * (sp + wp);
    p2cp.y = 0.5 * (sp + wp);
  }

  if (geometry == 0 and sp + wp < 2 * kp)
  {
    p1cp.y = -0.5 * (sp + wp);
    p2cp.y = 0.5 * (sp + wp);
  }

  fh->l.crop(path1Arr, path1, p1cp, p2cp, topM);
  fh->l.crop(path2Arr, path2, p1cp, p2cp, topM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  path1Arr.clear();
  path2Arr.clear();

  // ###############################################################################################
  // # cutout for bridge45
  // ###############################################################################################
  p1c.y = -0.5 * b;
  p2c.y = 0.5 * b;

  fh->l.crop(path1Arr, path0, p1c, p2c, topM);
  fh->l.crop(path2Arr, path3, p1c, p2c, topM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  path1Arr.clear();
  path2Arr.clear();

  path0.clear();
  path1.clear();
  path2.clear();
  path3.clear();

  // ###############################################################################################
  // # primary contacts
  // ###############################################################################################

  Point2 p1 = Point2(-0.5 * dmax - lp, wp + 0.5 * sp);
  Point2 p2 = Point2(-0.5 * dp + 0.5 * wp, 0.5 * sp);

  fh->l.rectangle(path1, p1, p2, 0, topM);
  fh->addSegment(path1);
  path1.clear();

  p1 = Point2(-0.5 * dmax - lp, -wp - 0.5 * sp);
  p2 = Point2(-0.5 * dp + 0.5 * wp, -0.5 * sp);

  fh->l.rectangle(path1, p1, p2, 0, topM);
  fh->addSegment(path1);
  path1.clear();

  // ###############################################################################################
  // # primary label
  // ###############################################################################################

  Point2 portP1 = Point2(-0.5 * dmax - lp, 0.5 * (sp + wp));
  Point2 portP2 = Point2(-0.5 * dmax - lp, -0.5 * (sp + wp));

  // fh->addPort(p1, p2, topM,topM);

  // ###############################################################################################
  // # primary inner turn
  // ###############################################################################################

  fh->l.drawSegment(path0, dp - 2 * (wp + s), wp, 0, geometry, topM);
  fh->l.drawSegment(path1, dp - 2 * (wp + s), wp, 1, geometry, topM);
  fh->l.drawSegment(path2, dp - 2 * (wp + s), wp, 2, geometry, topM);
  fh->l.drawSegment(path3, dp - 2 * (wp + s), wp, 3, geometry, topM);

  // ###############################################################################################
  // # cutout for bridge45
  // ###############################################################################################

  fh->l.crop(path1Arr, path0, p1c, p2c, topM);
  fh->l.crop(path2Arr, path3, p1c, p2c, topM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);
  path1Arr.clear();
  path2Arr.clear();

  // ports
  Point2 portP1n;
  Point2 portP2n;
  Point2 portS1n;
  Point2 portS2n;

  // fasterCap
  if (fasterCap)
  {
    fh->l.crop(path1Arr, path1, p1c_wp, p2c_wp, topM);
    fh->l.crop(path2Arr, path2, p1c_wp, p2c_wp, topM);

    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);
    path1Arr.clear();
    path2Arr.clear();

    // ports
    portP1n.x = -0.5 * (dp - 2 * (wp + s) - wp);
    portP1n.y = -0.5 * wp;

    portP2n.x = -0.5 * (dp - 2 * (wp + s) - wp);
    portP2n.y = 0.5 * wp;
  }
  else
  {
    fh->addSegment(path1);
    fh->addSegment(path2);
  }

  path1.clear();
  path2.clear();

  // ###############################################################################################
  // # bridge45 secondary
  // ###############################################################################################
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
  p1 = Point2(0.5 * dp - wp, 0.5 * (b));
  p2 = Point2(0.5 * dp, 0.5 * (b));
  Point2 p3 = Point2(p1.x - e, -0.5 * (b));
  Point2 p4 = Point2(p2.x - e, -0.5 * (b));

  fh->l.bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
  fh->addSegment(path1Arr);

  // ###############################################################################################
  // # secondary turn
  // ###############################################################################################

  fh->l.drawSegment(path0, ds, ws, 0, geometry, bottomM);
  fh->l.drawSegment(path1, ds, ws, 1, geometry, bottomM);
  fh->l.drawSegment(path2, ds, ws, 2, geometry, bottomM);
  fh->l.drawSegment(path3, ds, ws, 3, geometry, bottomM);

  // fasterCap
  if (fasterCap)
  {
    fh->l.crop(path1Arr, path1, p1c_ws, p2c_ws, bottomM);
    fh->l.crop(path2Arr, path2, p1c_ws, p2c_ws, bottomM);

    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);

    path1Arr.clear();
    path2Arr.clear();

    // ports
    portS1n.x = -0.5 * (ds - ws);
    portS1n.y = 0.5 * ws;

    portS2n.x = -0.5 * (ds - ws);
    portS2n.y = -0.5 * ws;
  }
  else
  {
    fh->addSegment(path1);
    fh->addSegment(path2);
  }

  p1c.y = -0.5 * sp;
  p2c.y = 0.5 * sp;

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

  // internal k for secondary
  double ks = this->l->k(ds - 2 * ws);

  Point2 p1cs = p1c;
  Point2 p2cs = p2c;

  // rect geometry
  if (geometry == 1 and sp + ws < ds - 2 * ws)
  {
    p1cs.y = -0.5 * (sp + ws);
    p2cs.y = 0.5 * (sp + ws);
  }

  // oct geometry
  if (geometry == 0 and sp + ws < 2 * ks)
  {
    p1cs.y = -0.5 * (sp + ws);
    p2cs.y = 0.5 * (sp + ws);
  }

  fh->l.crop(path1Arr, path0, p1cs, p2cs, bottomM);
  fh->l.crop(path2Arr, path3, p1cs, p2cs, bottomM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  // ###############################################################################################
  // # secondary contacts
  // ###############################################################################################

  p1 = Point2(0.5 * dmax + lp, ws + 0.5 * sp);
  p2 = Point2(0.5 * ds - 0.5 * ws, 0.5 * sp);

  fh->l.rectangle(path1, p1, p2, 0, bottomM);
  fh->addSegment(path1);

  p1 = Point2(0.5 * dmax + lp, -ws - 0.5 * sp);
  p2 = Point2(0.5 * ds - 0.5 * ws, -0.5 * sp);

  fh->l.rectangle(path1, p1, p2, 0, bottomM);
  fh->addSegment(path1);

  // ###############################################################################################
  // # secondary label
  // ###############################################################################################

  Point2 portS1 = Point2(0.5 * dmax + lp, 0.5 * (sp + ws));
  Point2 portS2 = Point2(0.5 * dmax + lp, -0.5 * (sp + ws));

  // fh->addPort(p1, p2, bottomM,bottomM);

  // ###############################################################################################
  // # ports
  // ###############################################################################################

  if (fasterCap)
  {

    fh->addPort(portP1, portP1n, topM, topM);
    fh->addPort(portP2n, portP2, topM, topM);
    fh->addPort(portS1, portS1n, bottomM, bottomM);
    fh->addPort(portS2n, portS2, bottomM, bottomM);
  }
  else
  {

    fh->addPort(portP1, portP2, topM, topM);
    fh->addPort(portS1, portS2, bottomM, bottomM);
  }

  // ###############################################################################################
  // # primary tapped
  // ###############################################################################################

  if (wtp > 0)
  {

    p1 = lp > wtp ? Point2(-0.5 * dmax - wtp, 0.5 * wtp) : Point2(-0.5 * dmax - lp, 0.5 * wtp);
    p2 = Point2(-0.5 * dp + 2 * wp + s, -0.5 * wtp);

    fh->l.rectangle(path1, p1, p2, 0, topM);
    fh->addSegment(path1);
  }

  // ###############################################################################################
  // # secondary tapped
  // ###############################################################################################

  if (wts > 0)
  {

    p1 = lp > wts ? Point2(-0.5 * dmax - wts, 0.5 * wts) : Point2(-0.5 * dmax - lp, 0.5 * wts);
    p2 = Point2(-0.5 * ds + ws, -0.5 * wts);

    // string bBottomM = this->l->t.getMetName(bottomMId.metNum - 1);

    fh->l.rectangle(path1, p1, p2, 0, bottomM);
    fh->addSegment(path1);
  }

  return 0;
}

int Transformer2o1::FastHenryProcess(Command *cm)
{
  // ###############################################################################################
  // # FastHenryTransformer
  // #		: process command line options
  // ###############################################################################################

  ofstream runFastHenryShFile;

  if (cm->geometry == 0) // octagonal
  {
    cm->cellName = "plTr2o1Oct";
  }
  else if (cm->geometry == 1) // rectangular
  {
    cm->cellName = "plTr2o1Rect";
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
  auto threadFunc = [this, &syncronization_flag, &counter, &runFastHenryShFile, cm](double d, double w, double ws, double s, double sh)
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

        ws = ws <= 0 ? w : ws;
        double ds = d - 2 * w - s + ws - 2 * sh;

        // verbose
        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string wsStr = std::to_string(ws);
        wsStr = wsStr.substr(0, wsStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string ssStr = std::to_string(sh);
        ssStr = ssStr.substr(0, ssStr.find(".") + 3);

        this->FastHenryGenCell(fh, d, ds, w, ws, s, cm->ps, cm->pl, w * cm->tappedPrimaryWidth, ws * cm->tappedSecondaryWidth, cm->geometry, cm->topM, false);

        string gdsFileName = cm->gdsFile;

        string tmp = std::to_string(d);
        tmp = tmp.substr(0, tmp.find(".") + 3);
        gdsFileName = gdsFileName + "_d" + tmp;

        tmp = std::to_string(w);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_w" + tmp;

        tmp = std::to_string(ws);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_ws" + tmp;

        tmp = std::to_string(s);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_s" + tmp;

        tmp = std::to_string(sh);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_sh" + tmp;

        tmp = std::to_string(cm->ps);
        tmp = tmp.substr(0, tmp.find(".") + 3);
        gdsFileName = gdsFileName + "_ps" + tmp;

        tmp = std::to_string(cm->pl);
        tmp = tmp.substr(0, tmp.find(".") + 3);
        gdsFileName = gdsFileName + "_pl" + tmp;
        //******************************************************************************************
        // mutex protected: START
        //******************************************************************************************
        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(75) << "transformer2o1: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " sh=" + ssStr;
        counter++;
        cout << std::left << std::setw(30) << counter;
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

        // verbose
        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string wsStr = std::to_string(ws);
        wsStr = wsStr.substr(0, wsStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string ssStr = std::to_string(sh);
        ssStr = ssStr.substr(0, ssStr.find(".") + 3);

        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(75) << "transformer2o1: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " sh=" + ssStr;
        std::cout << std::left << std::setw(30) << "Warning: not feasible!";
        syncronization_flag.unlock();
      }
      catch (HardError exception)
      {
        syncronization_flag.lock();
        // ...code that handles HardError...
        std::cout << exception.name << endl;
        // clear and exit
        syncronization_flag.unlock();
        exit(1);
      }
      catch (...)
      {
        syncronization_flag.lock();
        std::cout << "Unknown exception." << endl;
        syncronization_flag.unlock();
        exit(1);
      }

      // clear
      delete fh;

    } // try
    catch (HardError exception)
    {
      syncronization_flag.lock();
      // ...code that handles HardError...
      std::cout << exception.name << endl;
      // clear and exit
      syncronization_flag.unlock();
      exit(1);
    }
    catch (...)
    {
      syncronization_flag.lock();
      std::cout << "Unknown exception." << endl;
      syncronization_flag.unlock();
      exit(1);
    }
  }; // lambda

  std::vector<std::array<double, 5>> paramArray;
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->ws.size() * cm->s.size() * cm->sh.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double ws = cm->ws.min; ws <= cm->ws.max; ws = ws + cm->ws.step)
      {
        for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
        {
          for (double sh = cm->sh.min; sh <= cm->sh.max; sh = sh + cm->sh.step)
          {
            paramArray.push_back({d, w, ws, s, sh});

          } // for ss

        } // for s

      } // for ws

    } // for w

  } // for d

  auto loopFunction = [this, &paramArray, &threadFunc](const uint64_t a, const uint64_t b)
  {
    for (size_t k = a; k < b; k++)
    {
      const std::array<double, 5> param = paramArray[k];
      threadFunc(param[0], param[1], param[2], param[3], param[4]);
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

int Transformer2o1::FastHenry2Process(Command *cm)
{
  // ###############################################################################################
  // # FastHenryTransformer
  // #		: process command line options
  // ###############################################################################################

  ofstream runFastHenryShFile;

  if (cm->geometry == 0) // octagonal
  {
    cm->cellName = "plTr2o1Oct";
  }
  else if (cm->geometry == 1) // rectangular
  {
    cm->cellName = "plTr2o1Rect";
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
  auto threadFunc = [this, &syncronization_flag, &counter, &runFastHenryShFile, cm](double d, double w, double ws, double s, double sh)
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

        ws = ws <= 0 ? w : ws;
        double ds = d - 2 * w - s + ws - 2 * sh;

        // verbose
        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string wsStr = std::to_string(ws);
        wsStr = wsStr.substr(0, wsStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string ssStr = std::to_string(sh);
        ssStr = ssStr.substr(0, ssStr.find(".") + 3);

        this->FastHenryGenCell(fh, d, ds, w, ws, s, cm->ps, cm->pl, w * cm->tappedPrimaryWidth, ws * cm->tappedSecondaryWidth, cm->geometry, cm->topM, true);

        string gdsFileName = cm->gdsFile;

        string tmp = std::to_string(d);
        tmp = tmp.substr(0, tmp.find(".") + 3);
        gdsFileName = gdsFileName + "_d" + tmp;

        tmp = std::to_string(w);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_w" + tmp;

        tmp = std::to_string(ws);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_ws" + tmp;

        tmp = std::to_string(s);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_s" + tmp;

        tmp = std::to_string(sh);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_sh" + tmp;

        tmp = std::to_string(cm->ps);
        tmp = tmp.substr(0, tmp.find(".") + 3);
        gdsFileName = gdsFileName + "_ps" + tmp;

        tmp = std::to_string(cm->pl);
        tmp = tmp.substr(0, tmp.find(".") + 3);
        gdsFileName = gdsFileName + "_pl" + tmp;
        //******************************************************************************************
        // mutex protected: START
        //******************************************************************************************
        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(75) << "transformer2o1: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " sh=" + ssStr;
        counter++;
        cout << std::left << std::setw(30) << counter;
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

        // verbose
        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string wsStr = std::to_string(ws);
        wsStr = wsStr.substr(0, wsStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string ssStr = std::to_string(sh);
        ssStr = ssStr.substr(0, ssStr.find(".") + 3);

        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(75) << "transformer2o1: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " sh=" + ssStr;
        std::cout << std::left << std::setw(30) << "Warning: not feasible!";
        syncronization_flag.unlock();
      }
      catch (HardError exception)
      {
        syncronization_flag.lock();
        // ...code that handles HardError...
        std::cout << exception.name << endl;
        // clear and exit
        syncronization_flag.unlock();
        exit(1);
      }
      catch (...)
      {
        syncronization_flag.lock();
        std::cout << "Unknown exception." << endl;
        syncronization_flag.unlock();
        exit(1);
      }

      // clear
      delete fh;

    } // try
    catch (HardError exception)
    {
      syncronization_flag.lock();
      // ...code that handles HardError...
      std::cout << exception.name << endl;
      // clear and exit
      syncronization_flag.unlock();
      exit(1);
    }
    catch (...)
    {
      syncronization_flag.lock();
      std::cout << "Unknown exception." << endl;
      syncronization_flag.unlock();
      exit(1);
    }
  }; // lambda

  std::vector<std::array<double, 5>> paramArray;
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->ws.size() * cm->s.size() * cm->sh.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double ws = cm->ws.min; ws <= cm->ws.max; ws = ws + cm->ws.step)
      {
        for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
        {
          for (double sh = cm->sh.min; sh <= cm->sh.max; sh = sh + cm->sh.step)
          {

            paramArray.push_back({d, w, ws, s, sh});

          } // for ss

        } // for s

      } // for ws

    } // for w

  } // for d

  auto loopFunction = [this, &paramArray, &threadFunc](const uint64_t a, const uint64_t b)
  {
    for (size_t k = a; k < b; k++)
    {
      const std::array<double, 5> param = paramArray[k];
      threadFunc(param[0], param[1], param[2], param[3], param[4]);
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