#include "mcell_error.h"
#include "gdstk.h"
#include "mcell_transformer_spiral.h"
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

int TransformerSpiral::threadNumber(Command *cm)
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

int TransformerSpiral::drc(double d, double w, double s, double sp, double lp, int np, int ns, double wtp, double wts, int geometry, string topM)
{

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
  LayerMet bottomMId = this->l->t.getMet(topMId.metNum - 1);
  LayerVia viaId = this->l->getVia(topM, bottomMId.name);

  // longer distance from top and bottom
  double minS = topMId.minS > bottomMId.minS ? topMId.minS : bottomMId.minS;

  double e2 = 2 * (w + s);
  double shift = (w + s) / (1 + sqrt(2));
  e2 = 2 * this->l->round2grid(0.5 * e2);
  shift = 2 * this->l->round2grid(0.5 * shift);

  // used to cut bridge45x2x2 on left side close to secondary pins distance between cuts is e2
  double b2x2 = e2 + w / (1 + sqrt(2)) + 2 * this->l->getGrid();
  b2x2 = 2 * this->l->round2grid(0.5 * b2x2);

  // used for bridge45x4
  double b2 = e2 + w / (1 + sqrt(2)) + minS;
  b2 = 2 * this->l->round2grid(0.5 * b2);

  // used to cut bridge45x4
  double b4 = b2 + shift;
  b4 = 2 * this->l->round2grid(0.5 * b4);

  double e = (w + s);
  e = 2 * this->l->round2grid(0.5 * e);
  // used to cut bridge45x2
  double b = e + w / (1 + sqrt(2)) + minS;
  b = 2 * this->l->round2grid(0.5 * b);

  // ###############################################################################################
  // # geometry check
  // ###############################################################################################

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

  if (np > 1 and ns > np - 1)
  {
    throw SoftError("Warning: ns > np-1!");
  }

  if (np == 1 and ns > 1)
  {
    throw SoftError("Warning: ns > np!");
  }

  if (np < 1 or ns < 1)
  {
    throw SoftError("Warning: ns or np < 1!");
  }

  // inner diameter
  double dn = d - (np + ns - 1) * 2 * (w + s) - 2 * w;

  if (dn < topMId.minS or dn < bottomMId.minS)
  {
    throw SoftError("Warning: inner diameter too small!");
  }

  // inner turn can not be connected with bridge45x4
  if (dn <= b4)
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

  if (np > 1 and sp < (b2x2 + 2 * w + 2 * bottomMId.minS))
  {
    throw SoftError("Warning: pin space too short!");
  }

  double d2 = d - 2 * (w + s) - 2 * w;
  double k2 = this->l->k(d2);

  if (geometry != 0 and d2 < (sp + 2 * w)) // rect
  {
    throw SoftError("Warning: pin space too wide!");
  }

  if (geometry == 0 and 2 * k2 < (sp + 2 * w)) // oct
  {
    throw SoftError("Warning: pin space too wide!");
  }

  // ###############################################################################################
  // # check bridge45x4
  // ###############################################################################################

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

  // inner diameter for bridge45x4 turn
  double dnBx4 = d - 2 * (2 * ns) * (w + s) - 2 * w;
  // b from picture
  double bnBx4 = this->l->k(dnBx4);

  if (geometry == 0 and ns > 1) // octagonal geometry
  {
    if (2 * bnBx4 < b4 + 2 * w)
    {
      throw SoftError("Warning: no enough space for bridge45x4!");
    }
  }

  if (geometry != 1 and ns > 1) // rect geometry
  {
    if (dnBx4 < b4 + 2 * w)
    {
      throw SoftError("Warning: no enough space for bridge45x4!");
    }
  }

  // ###############################################################################################
  // # check bridge45x2
  // ###############################################################################################
  // inner diameter for bridge45x2 turn
  double dnBx2 = d - (np + ns - 1) * 2 * (w + s) - 2 * w;
  double bnBx2 = (sqrt(2) - 1) * 0.5 * dnBx2;

  if (np > 1 and geometry == 0) // oct
  {
    if (2 * bnBx2 < b + 2 * w)
    {
      throw SoftError("Warning: no enough space for bridge45x2!");
    }
  }

  if (np > 1 and geometry != 0) // rect
  {
    if (dnBx2 < b + 2 * w)
    {
      throw SoftError("Warning: no enough space for bridge45x2!");
    }
  }

  // ###############################################################################################
  // # tapped primary
  // ###############################################################################################

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

  if (wtp > 0 and np == 1)
  {
    double k = this->l->k(d - 2 * w);

    if (wtp < topMId.minW)
    {
      throw SoftError("Warning: wtp<minw");
    }

    if (2 * k < wtp and geometry == 0)
    {
      throw SoftError("Warning: no enough space for primary tapping");
    }

    if (d < wtp and geometry != 0)
    {
      throw SoftError("Warning: no enough space for primary tapping");
    }
  }

  if (wtp > 0 and np == 2)
  {
    double kn = this->l->k(dn);

    if (2 * kn < wtp and geometry == 0)
    {
      throw SoftError("Warning: no enough space for primary tapping");
    }

    if (dn < wtp and geometry != 0)
    {
      throw SoftError("Warning: no enough space for primary tapping");
    }

    if (wts > 0) // secondary is tapped
    {

      LayerMet bBottomMId = this->l->t.getMet(bottomMId.metNum - 1);

      if (wtp < bBottomMId.minW)
      {
        throw SoftError("Warning: wtp<minw");
      }
    }
    else // secondary is not tapped
    {
      if (wtp < bottomMId.minW)
      {
        throw SoftError("Warning: wtp<minw");
      }
    }
  }

  if (wtp > 0 and np > 2)
  {
    double kn = this->l->k(dn);

    if (2 * kn < wtp and geometry == 0)
    {
      throw SoftError("Warning: no enough space for primary tapping");
    }

    if (dn < wtp and geometry != 0)
    {
      throw SoftError("Warning: no enough space for primary tapping");
    }

    LayerMet bBottomMId = this->l->t.getMet(bottomMId.metNum - 1);

    if (wts > 0)
    {
      if (((np - ns) % 2) == 0) // secondary and primary are not tapped at the same side
      {
        if (wtp < bBottomMId.minW)
        {
          throw SoftError("Warning: wtp<minw");
        }
      }
      else // secondary and primary are tapped at the same side
      {

        LayerMet bbBottomMId = this->l->t.getMet(bBottomMId.metNum - 1);

        if (wtp < bbBottomMId.minW)
        {
          throw SoftError("Warning: wtp<minw");
        }
      }
    }
    else
    {
      if (wtp < bBottomMId.minW)
      {
        throw SoftError("Warning: wtp<minw");
      }
    }
  }

  // ###############################################################################################
  // # tapped secondary
  // ###############################################################################################

  if (wts > 0 and ns == 1)
  {
    double ds = d - 2 * (w + s) - 2 * w;
    double kn = this->l->k(dn);

    if (2 * kn < wts and geometry == 0) // oct
    {
      throw SoftError("Warning: no enough space for secondary tapping");
    }

    if (ds < wts and geometry != 0) // rect
    {
      throw SoftError("Warning: no enough space for secondary tapping");
    }

    if (wts < topMId.minW)
    {
      throw SoftError("Warning: wtp<minw");
    }
  }

  if (wts > 0 and ns > 1)
  {
    double kn = this->l->k(dnBx4);

    if (2 * kn < wts and geometry == 0) // oct
    {
      throw SoftError("Warning: no enough space for secondary tapping");
    }

    if (dnBx4 < wts and geometry != 0) // rect
    {
      throw SoftError("Warning: no enough space for secondary tapping");
    }

    LayerMet bBottomMId = this->l->t.getMet(bottomMId.metNum - 1);

    if (wts < bBottomMId.minW)
    {
      throw SoftError("Warning: wts<minw");
    }
  }

  return 0;
}

int TransformerSpiral::genCell(Cell &result, double d, double w, double s, double sp, double lp, int np, int ns, double wtp, double wts, int geometry, string topM, bool fasterCap)
{
  // ###############################################################################################
  // # TransformerSpiral
  // ###############################################################################################
  // # d        : outer diameter
  // # w        : line width
  // # s        : space between lines
  // # sp       : space between pins
  // # lp       : pin length (default should be 2w)
  // # n        : number of turns
  // # wtp          : width of line for tapped primary of inductor if wtp<=0 no tapping
  // # wts          : width of line for tapped secondary of inductor if wtp<=0 no tapping
  // # geometry   : 0-> octagonal, else rectangular
  // # topM     : top metal layer (metal name)
  // # type      : 0-> standard pins on left side below bottom metal layer might be used for tapping
  // #          : 1-> pins are at bottom side, only bottom layer is used for tapping
  // ###############################################################################################

  LayerMet topMId = this->l->getMet(topM);
  string bottomM = this->l->t.getMetName(topMId.metNum - 1);
  LayerMet bottomMId = this->l->getMet(bottomM);

  // longer distance from top and bottom
  double minS = topMId.minS > bottomMId.minS ? topMId.minS : bottomMId.minS;

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

  // ###############################################################################################
  // # round to grid
  // ###############################################################################################

  double grid = this->l->t.getGrid();

  d = 2 * this->l->round2grid(0.5 * d);
  w = 2 * this->l->round2grid(0.5 * w);
  if (lp <= 0)
    lp = 2 * w;
  lp = 2 * this->l->round2grid(0.5 * lp);
  // leave a bit more space for 45 deg lines
  if (s >= topMId.minS and s <= topMId.minS + 2 * grid)
  {
    s = 2 * this->l->round2grid(0.5 * s + grid);
  }
  else
  {
    s = 2 * this->l->round2grid(0.5 * s);
  }

  wtp = 2 * this->l->round2grid(0.5 * wtp);
  wts = 2 * this->l->round2grid(0.5 * wts);

  double e2 = 2 * (w + s);
  double shift = (w + s) / (1 + sqrt(2));
  e2 = 2 * this->l->round2grid(0.5 * e2);
  shift = 2 * this->l->round2grid(0.5 * shift);

  // used to cut bridge45x2x2 on left side close to secondary pins distance between cuts is e2
  double b2x2 = e2 + w / (1 + sqrt(2)) + 2 * this->l->getGrid();
  b2x2 = 2 * this->l->round2grid(0.5 * b2x2);

  // used for bridge45x4
  double b2 = e2 + w / (1 + sqrt(2)) + minS;
  b2 = 2 * this->l->round2grid(0.5 * b2);

  // used to cut bridge45x4
  double b4 = b2 + shift;
  b4 = 2 * this->l->round2grid(0.5 * b4);

  double e = (w + s);
  e = 2 * this->l->round2grid(0.5 * e);
  // used to cut bridge45x2
  double b = e + w / (1 + sqrt(2)) + minS;
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

  // ###############################################################################################
  // # Set automatic space between pins
  // ###############################################################################################

  if (sp <= 0)
  {

    sp = b2x2 + 2 * w + 2 * bottomMId.minS + 2 * this->l->getGrid();
  }

  sp = 2 * this->l->round2grid(0.5 * sp);

  // ###############################################################################################
  // # geometry check DRC
  // ###############################################################################################

  this->drc(d, w, s, sp, lp, np, ns, wtp, wts, geometry, topM);

  Polygon *path0, *path1, *path2, *path3;

  // ###############################################################################################
  // # point for cutting at left and right side for bridge45x4
  // ###############################################################################################

  Vec2 p1cBx4 = Vec2{-0.5 * d - w, 0.5 * (b4)};
  Vec2 p2cBx4 = Vec2{0.5 * d + w, -0.5 * (b4)};

  // ###############################################################################################
  // # point for cutting at left and right side for bridge45x2
  // ###############################################################################################

  Vec2 p1cBx2 = Vec2{-0.5 * d - w, 0.5 * (b)};
  Vec2 p2cBx2 = Vec2{0.5 * d + w, -0.5 * (b)};

  // ###############################################################################################
  // # point for cutting at right side for bridge45x2 with distance e2 close to secondary pins
  // ###############################################################################################

  Vec2 p1cBx2x2 = Vec2{-0.5 * d - w, 0.5 * (b2x2)};
  Vec2 p2cBx2x2 = Vec2{0.5 * d + w, -0.5 * (b2x2)};

  // cut for faster cup
  Vec2 p1c_w = Vec2{-d - w - lp, -0.5 * w};
  Vec2 p2c_w = Vec2{d + w + lp, 0.5 * w};

  Array<Polygon *> path1Arr = {0};
  Array<Polygon *> path2Arr = {0};

  // ###############################################################################################
  // # right side 0 and 3 primary
  // ###############################################################################################

  this->l->drawSegment(path0, d, w, 0, geometry, topM);
  this->l->drawSegment(path3, d, w, 3, geometry, topM);

  if (np > 1) // if np==1 cut is not done
  {
    this->l->crop(path1Arr, path0, p1cBx2x2, p2cBx2x2, topM);
    this->l->crop(path2Arr, path3, p1cBx2x2, p2cBx2x2, topM);
    path0 = path1Arr[0];
    path3 = path2Arr[0];
  }
  else if (fasterCap)
  { // fasterCap

    this->l->crop(path1Arr, path0, p1c_w, p2c_w, topM);
    this->l->crop(path2Arr, path3, p1c_w, p2c_w, topM);
    path0 = path1Arr[0];
    path3 = path2Arr[0];
  }

  result.polygon_array.append(path0);
  result.polygon_array.append(path3);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # right side 0 and 3 secondary
  // ###############################################################################################
  Vec2 p1, p2;

  double ds = d - 2 * (s + w);

  p1 = Vec2{0.5 * d, 0.5 * sp};
  p2 = Vec2{0.5 * ds - w, -0.5 * sp};

  this->l->drawSegment(path0, ds, w, 0, geometry, topM);
  this->l->drawSegment(path3, ds, w, 3, geometry, topM);

  this->l->crop(path1Arr, path0, p1, p2, topM);
  this->l->crop(path2Arr, path3, p1, p2, topM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # add contacts
  // ###############################################################################################

  p1 = Vec2{0.5 * d + lp, w + 0.5 * sp};
  p2 = Vec2{0.5 * ds - w, 0.5 * sp};

  this->l->rectangle(path1, p1, p2, bottomM);
  result.polygon_array.append(path1);

  p1 = Vec2{0.5 * d + lp, -w - 0.5 * sp};
  p2 = Vec2{0.5 * ds - w, -0.5 * sp};

  this->l->rectangle(path1, p1, p2, bottomM);
  result.polygon_array.append(path1);

  // add vias
  p1 = Vec2{0.5 * ds - w, w + 0.5 * sp};
  p2 = Vec2{0.5 * ds, 0.5 * sp};

  this->l->fillVias(path1Arr, p1, p2, topM, bottomM);
  this->l->appendArrToCell(result, path1Arr);

  p1 = Vec2{0.5 * ds - w, -w - 0.5 * sp};
  p2 = Vec2{0.5 * ds, -0.5 * sp};

  this->l->fillVias(path1Arr, p1, p2, topM, bottomM);
  this->l->appendArrToCell(result, path1Arr);

  // ###############################################################################################
  // # add label
  // ###############################################################################################

  Label *label;

  this->l->label(label, "S1", Vec2{0.5 * d + lp, 0.5 * (sp + w)}, bottomM);
  result.label_array.append(label);

  this->l->label(label, "S2", Vec2{0.5 * d + lp, -0.5 * (sp + w)}, bottomM);
  result.label_array.append(label);

  // ###############################################################################################
  // # left side 1 and 2 primary
  // ###############################################################################################

  this->l->drawSegment(path1, d, w, 1, geometry, topM);
  this->l->drawSegment(path2, d, w, 2, geometry, topM);

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

  this->l->label(label, "P1", Vec2{-0.5 * d - lp, 0.5 * (sp + w)}, topM);
  result.label_array.append(label);

  this->l->label(label, "P2", Vec2{-0.5 * d - lp, -0.5 * (sp + w)}, topM);
  result.label_array.append(label);

  // ###############################################################################################
  // # left side 1 and 2 secondary
  // ###############################################################################################

  this->l->drawSegment(path1, ds, w, 1, geometry, topM);
  this->l->drawSegment(path2, ds, w, 2, geometry, topM);

  if (ns > 1) // if ns==1 cut is not done
  {
    this->l->crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
    this->l->crop(path2Arr, path2, p1cBx4, p2cBx4, topM);
    path1 = path1Arr[0];
    path2 = path2Arr[0];
  }
  else if (fasterCap)
  { // fasterCap

    this->l->crop(path1Arr, path1, p1c_w, p2c_w, topM);
    this->l->crop(path2Arr, path2, p1c_w, p2c_w, topM);
    path1 = path1Arr[0];
    path2 = path2Arr[0];
  }

  result.polygon_array.append(path1);
  result.polygon_array.append(path2);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # loop bridge 45 x 4
  // ###############################################################################################

  int firstTimeLoopFleg = 1;         // used to indicate first enter the loop. Need to put bridge45x2x2 on right side
  int sideRightFlag = 1;             // 1 is right, -1 is left
  Vec2 trans = Vec2{0, 0.5 * shift}; // translation vector
  double dn = d - 2 * 2 * (w + s);
  Vec2 p3, p4;

  // ##############################################################################################################################################################################################
  // # loop **************************************************************************************
  // ##############################################################################################################################################################################################

  for (int i = 0; i < ns; i++)
  {
    if (firstTimeLoopFleg == 1)
    {
      firstTimeLoopFleg = 0;

      if (np == 1)
      {
        continue;
      }

      // ###############################################################################################
      // # right side 0 and 3 primary
      // ###############################################################################################
      this->l->drawSegment(path0, dn, w, 0, geometry, topM);
      this->l->drawSegment(path3, dn, w, 3, geometry, topM);

      this->l->crop(path1Arr, path0, p1cBx2x2, p2cBx2x2, topM);
      this->l->crop(path2Arr, path3, p1cBx2x2, p2cBx2x2, topM);

      result.polygon_array.append(path1Arr[0]);
      result.polygon_array.append(path2Arr[0]);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

      // ###############################################################################################
      // # bridge 45 x 2 for primary left side close to secondary contacts distance e=e2, b=b2
      // ###############################################################################################

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
      p1 = Vec2{0.5 * d - 2 * (w + s) - w, 0.5 * (b2x2)};
      p2 = Vec2{0.5 * d - 2 * (w + s), 0.5 * (b2x2)};
      p3 = Vec2{0.5 * d - w, -0.5 * (b2x2)};
      p4 = Vec2{0.5 * d, -0.5 * (b2x2)};

      this->l->bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
      this->l->appendArrToCell(result, path1Arr);

      // ###############################################################################################
      // # left side 1 and 2 primary
      // ###############################################################################################

      this->l->drawSegment(path1, dn, w, 1, geometry, topM);
      this->l->drawSegment(path2, dn, w, 2, geometry, topM);

      if (ns > 1) // bridge45x4
      {
        this->l->crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
        this->l->crop(path2Arr, path2, p1cBx4, p2cBx4, topM);
        path1 = path1Arr[0];
        path2 = path2Arr[0];
      }
      else if (np > 2) // bridge45x2
      {
        this->l->crop(path1Arr, path1, p1cBx2, p2cBx2, topM);
        this->l->crop(path2Arr, path2, p1cBx2, p2cBx2, topM);
        path1 = path1Arr[0];
        path2 = path2Arr[0];
      }
      else if (fasterCap)
      { // fasterCap

        this->l->crop(path1Arr, path1, p1c_w, p2c_w, topM);
        this->l->crop(path2Arr, path2, p1c_w, p2c_w, topM);
        path1 = path1Arr[0];
        path2 = path2Arr[0];
      }

      result.polygon_array.append(path1);
      result.polygon_array.append(path2);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

      sideRightFlag = -sideRightFlag;

      dn = d - 2 * 3 * (w + s);

      continue;
    }

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

    if (sideRightFlag < 0) // left
    {

      // two left turns

      // first left turn
      this->l->drawSegment(path1, dn, w, 1, geometry, topM);
      this->l->drawSegment(path2, dn, w, 2, geometry, topM);

      this->l->crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
      this->l->crop(path2Arr, path2, p1cBx4, p2cBx4, topM);

      path1 = path1Arr[0];
      path2 = path2Arr[0];

      result.polygon_array.append(path1);
      result.polygon_array.append(path2);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

      // second left turn
      this->l->drawSegment(path1, dn - 2 * (w + s), w, 1, geometry, topM);
      this->l->drawSegment(path2, dn - 2 * (w + s), w, 2, geometry, topM);

      this->l->crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
      this->l->crop(path2Arr, path2, p1cBx4, p2cBx4, topM);

      path1 = path1Arr[0];
      path2 = path2Arr[0];

      result.polygon_array.append(path1);
      result.polygon_array.append(path2);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

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
      // double e2=2*(w+s);
      // //used to cut bridge45x2x2 on left side close to secondary pins distance between cuts is e2
      // //used to cut bridge45x4
      // double b2=e2+w/(1+sqrt(2))+2*this->l->getGrid();
      // double b4=b2+shift;
      // double shift=(w+s)/(1+sqrt(2));
      // Vec2 trans=Vec2{0, 0.5*shift}; //translation vector

      // bridge 45x4
      p1 = Vec2{-0.5 * dn, 0.5 * (b2)};
      p2 = Vec2{-0.5 * dn + w, 0.5 * (b2)};
      p3 = Vec2{-0.5 * dn - 2 * (w + s), -0.5 * (b2)};
      p4 = Vec2{-0.5 * dn - 2 * (w + s) + w, -0.5 * (b2)};

      // first turn
      this->l->bridge45x2(path0, path1, p1, p2, p3, p4, bottomM, topM);
      path0->translate(trans);
      result.polygon_array.append(path0);

      // extend contacts
      this->l->rectangle(path0, Vec2{p3.x, p3.y - trans.y - w}, Vec2{p4.x, p4.y + trans.y}, bottomM);
      result.polygon_array.append(path0);

      this->l->rectangle(path0, Vec2{p3.x, p1.y + trans.y + w}, Vec2{p4.x, p2.y - trans.y}, topM);
      result.polygon_array.append(path0);

      // add vias
      this->l->fillVias(path1Arr, Vec2{p3.x, p3.y - trans.y - w}, Vec2{p4.x, p3.y - trans.y}, topM, bottomM);
      this->l->appendArrToCell(result, path1Arr);
      this->l->fillVias(path1Arr, Vec2{p1.x, p1.y + trans.y}, Vec2{p2.x, p2.y + trans.y + w}, topM, bottomM);
      this->l->appendArrToCell(result, path1Arr);

      trans.y = trans.y * (-1);
      path1->translate(trans);
      result.polygon_array.append(path1);

      p1.x = p1.x - (w + s) * sideRightFlag;
      p2.x = p2.x - (w + s) * sideRightFlag;
      p3.x = p3.x - (w + s) * sideRightFlag;
      p4.x = p4.x - (w + s) * sideRightFlag;

      // second turn
      this->l->bridge45x2(path0, path1, p1, p2, p3, p4, bottomM, topM);

      path0->translate(trans);
      result.polygon_array.append(path0);

      trans.y = trans.y * (-1);

      // extend contacts
      this->l->rectangle(path0, Vec2{p1.x, p1.y + trans.y + w}, Vec2{p2.x, p2.y - trans.y}, bottomM);
      result.polygon_array.append(path0);

      this->l->rectangle(path0, Vec2{p1.x, p3.y - trans.y - w}, Vec2{p2.x, p3.y + trans.y}, topM);
      result.polygon_array.append(path0);

      // add vias
      this->l->fillVias(path1Arr, Vec2{p3.x, p3.y - trans.y - w}, Vec2{p4.x, p3.y - trans.y}, topM, bottomM);
      this->l->appendArrToCell(result, path1Arr);
      this->l->fillVias(path1Arr, Vec2{p1.x, p1.y + trans.y}, Vec2{p2.x, p2.y + trans.y + w}, topM, bottomM);
      this->l->appendArrToCell(result, path1Arr);

      path1->translate(trans);
      result.polygon_array.append(path1);

      // #####################################################################################################
      // two right turns
      // #####################################################################################################

      // ###############################################################################################
      // # right side 0 and 3 primary
      // ###############################################################################################

      // first turn
      this->l->drawSegment(path0, dn, w, 0, geometry, topM);
      this->l->drawSegment(path3, dn, w, 3, geometry, topM);

      if (i < ns - 1) // one more bridge45x4 is needed
      {
        this->l->crop(path1Arr, path0, p1cBx4, p2cBx4, topM);
        this->l->crop(path2Arr, path3, p1cBx4, p2cBx4, topM);
        path0 = path1Arr[0];
        path3 = path2Arr[0];
      }
      else // place for secondary tapped
      {
        // secondary tapping
        // this is only possible if ns>1
        // tapping for ns==1 needs to be done in other place

        if (fasterCap)
        { // fasterCap

          this->l->crop(path1Arr, path0, p1c_w, p2c_w, topM);
          this->l->crop(path2Arr, path3, p1c_w, p2c_w, topM);
          path0 = path1Arr[0];
          path3 = path2Arr[0];
        } // fasterCap

        if (wts > 0)
        {

          p1 = Vec2{-sideRightFlag * 0.5 * (dn - 2 * w), 0.5 * wts};
          p2 = Vec2{-sideRightFlag * (0.5 * d + wts), -0.5 * wts};

          if (wts > lp)
          {
            p2.x = -sideRightFlag * (0.5 * d + lp);
          }

          Polygon *tappedPath = NULL;

          string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
          this->l->rectangle(tappedPath, p1, p2, bBottomM);
          result.polygon_array.append(tappedPath);

          // fill vias

          Array<Polygon *> viaArr = {0};

          this->l->fillVias(viaArr, p1, Vec2{p1.x - sideRightFlag * w, p2.y}, topM, bottomM);
          this->l->appendArrToCell(result, viaArr);

          this->l->fillVias(viaArr, p1, Vec2{p1.x - sideRightFlag * w, p2.y}, bottomM, bBottomM);
          this->l->appendArrToCell(result, viaArr);

          // add label

          this->l->label(label, "TS", Vec2{p2.x, 0}, bBottomM);
          result.label_array.append(label);

        } // if wts
      } // else

      result.polygon_array.append(path0);
      result.polygon_array.append(path3);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

      // second turn
      this->l->drawSegment(path0, dn - 2 * (w + s), w, 0, geometry, topM);
      this->l->drawSegment(path3, dn - 2 * (w + s), w, 3, geometry, topM);

      if (i < ns - 1) // one more bridge45x4 is needed
      {
        this->l->crop(path1Arr, path0, p1cBx4, p2cBx4, topM);
        this->l->crop(path2Arr, path3, p1cBx4, p2cBx4, topM);
        path0 = path1Arr[0];
        path3 = path2Arr[0];
      }
      else if (np > ns + 1) // bridge42x2 is needed
      {
        this->l->crop(path1Arr, path0, p1cBx2, p2cBx2, topM);
        this->l->crop(path2Arr, path3, p1cBx2, p2cBx2, topM);
        path0 = path1Arr[0];
        path3 = path2Arr[0];
      }
      else // primary tapping
      {

        if (fasterCap)
        { // fasterCap

          this->l->crop(path1Arr, path0, p1c_w, p2c_w, topM);
          this->l->crop(path2Arr, path3, p1c_w, p2c_w, topM);
          path0 = path1Arr[0];
          path3 = path2Arr[0];
        } // fasterCap

        // add tapping for primary
        if (wtp > 0)
        {
          Array<Polygon *> viaArr = {0};
          Polygon *tappedPath = NULL;

          p1.y = 0.5 * wtp;
          p2.y = -0.5 * wtp;
          p2.x = -sideRightFlag * 0.5 * (dn - 2 * w - 2 * (w + s));

          if (wtp > lp)
          {
            p1.x = -sideRightFlag * (0.5 * d + lp);
          }
          else
          {
            p1.x = -sideRightFlag * (0.5 * d + wtp);
          }

          string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
          string bbBottomM = this->l->t.getMetName((this->l->t.getMet(bBottomM)).metNum - 1);
          this->l->rectangle(tappedPath, p1, p2, bbBottomM);
          result.polygon_array.append(tappedPath);

          // fill vias

          this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
          this->l->appendArrToCell(result, viaArr);

          this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
          this->l->appendArrToCell(result, viaArr);

          this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bBottomM, bbBottomM);
          this->l->appendArrToCell(result, viaArr);

          // add label

          this->l->label(label, "TP", Vec2{p1.x, 0}, bbBottomM);
          result.label_array.append(label);
        }

      } // else primary tapping

      result.polygon_array.append(path0);
      result.polygon_array.append(path3);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};
    }
    else // right
    {

      // tow right turns
      // first turn
      this->l->drawSegment(path0, dn, w, 0, geometry, topM);
      this->l->drawSegment(path3, dn, w, 3, geometry, topM);

      this->l->crop(path1Arr, path0, p1cBx4, p2cBx4, topM);
      this->l->crop(path2Arr, path3, p1cBx4, p2cBx4, topM);
      path0 = path1Arr[0];
      path3 = path2Arr[0];

      result.polygon_array.append(path0);
      result.polygon_array.append(path3);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

      // second turn
      this->l->drawSegment(path0, dn - 2 * (w + s), w, 0, geometry, topM);
      this->l->drawSegment(path3, dn - 2 * (w + s), w, 3, geometry, topM);

      this->l->crop(path1Arr, path0, p1cBx4, p2cBx4, topM);
      this->l->crop(path2Arr, path3, p1cBx4, p2cBx4, topM);
      path0 = path1Arr[0];
      path3 = path2Arr[0];

      result.polygon_array.append(path0);
      result.polygon_array.append(path3);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

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
      // double e2=2*(w+s);
      // //used to cut bridge45x2x2 on left side close to secondary pins distance between cuts is e2
      // //used to cut bridge45x4
      // double b2=e2+w/(1+sqrt(2))+2*this->l->getGrid();
      // double b4=b2+shift;
      // double shift=(w+s)/(1+sqrt(2));
      // Vec2 trans=Vec2{0, 0.5*shift}; //translation vector

      // bridge 45x4
      p1 = Vec2{0.5 * dn - w, 0.5 * (b2)};
      p2 = Vec2{0.5 * dn, 0.5 * (b2)};
      p3 = Vec2{0.5 * dn - w + 2 * (w + s), -0.5 * (b2)};
      p4 = Vec2{0.5 * dn + 2 * (w + s), -0.5 * (b2)};

      // first turn
      this->l->bridge45x2(path0, path1, p1, p2, p3, p4, bottomM, topM);
      path0->translate(trans);
      result.polygon_array.append(path0);

      // extend contacts
      this->l->rectangle(path0, Vec2{p3.x, p3.y - trans.y - w}, Vec2{p4.x, p4.y + trans.y}, bottomM);
      result.polygon_array.append(path0);

      this->l->rectangle(path0, Vec2{p3.x, p1.y + trans.y + w}, Vec2{p4.x, p2.y - trans.y}, topM);
      result.polygon_array.append(path0);

      // add vias
      this->l->fillVias(path1Arr, Vec2{p3.x, p3.y - trans.y - w}, Vec2{p4.x, p3.y - trans.y}, topM, bottomM);
      this->l->appendArrToCell(result, path1Arr);
      this->l->fillVias(path1Arr, Vec2{p1.x, p1.y + trans.y}, Vec2{p2.x, p2.y + trans.y + w}, topM, bottomM);
      this->l->appendArrToCell(result, path1Arr);

      trans.y = trans.y * (-1);
      path1->translate(trans);
      result.polygon_array.append(path1);

      p1.x = p1.x - (w + s) * sideRightFlag;
      p2.x = p2.x - (w + s) * sideRightFlag;
      p3.x = p3.x - (w + s) * sideRightFlag;
      p4.x = p4.x - (w + s) * sideRightFlag;

      // second turn
      this->l->bridge45x2(path0, path1, p1, p2, p3, p4, bottomM, topM);

      path0->translate(trans);
      result.polygon_array.append(path0);

      trans.y = trans.y * (-1);

      // extend contacts
      this->l->rectangle(path0, Vec2{p1.x, p1.y + trans.y + w}, Vec2{p2.x, p2.y - trans.y}, bottomM);
      result.polygon_array.append(path0);

      this->l->rectangle(path0, Vec2{p1.x, p3.y - trans.y - w}, Vec2{p2.x, p3.y + trans.y}, topM);
      result.polygon_array.append(path0);

      // add vias
      this->l->fillVias(path1Arr, Vec2{p3.x, p3.y - trans.y - w}, Vec2{p4.x, p3.y - trans.y}, topM, bottomM);
      this->l->appendArrToCell(result, path1Arr);
      this->l->fillVias(path1Arr, Vec2{p1.x, p1.y + trans.y}, Vec2{p2.x, p2.y + trans.y + w}, topM, bottomM);
      this->l->appendArrToCell(result, path1Arr);

      path1->translate(trans);
      result.polygon_array.append(path1);

      // #####################################################################################################
      // two left turns
      // #####################################################################################################

      // first turn
      this->l->drawSegment(path1, dn, w, 1, geometry, topM);
      this->l->drawSegment(path2, dn, w, 2, geometry, topM);

      if (i < ns - 1) // one more bridge45x4 is needed
      {
        this->l->crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
        this->l->crop(path2Arr, path2, p1cBx4, p2cBx4, topM);
        path1 = path1Arr[0];
        path2 = path2Arr[0];
      }
      else // place for secondary tapped
      {
        // secondary tapping
        // this is only possible if ns>1
        // tapping for ns==1 needs to be done in other place

        if (fasterCap)
        { // fasterCap

          this->l->crop(path1Arr, path1, p1c_w, p2c_w, topM);
          this->l->crop(path2Arr, path2, p1c_w, p2c_w, topM);
          path1 = path1Arr[0];
          path2 = path2Arr[0];
        } // fasterCap

        if (wts > 0)
        {

          p1 = Vec2{-sideRightFlag * 0.5 * (dn - 2 * w), 0.5 * wts};
          p2 = Vec2{-sideRightFlag * (0.5 * d + wts), -0.5 * wts};

          if (wts > lp)
          {
            p2.x = -sideRightFlag * (0.5 * d + lp);
          }

          Polygon *tappedPath = NULL;

          string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
          this->l->rectangle(tappedPath, p1, p2, bBottomM);
          result.polygon_array.append(tappedPath);

          // fill vias

          Array<Polygon *> viaArr = {0};

          this->l->fillVias(viaArr, p1, Vec2{p1.x - sideRightFlag * w, p2.y}, topM, bottomM);
          this->l->appendArrToCell(result, viaArr);

          this->l->fillVias(viaArr, p1, Vec2{p1.x - sideRightFlag * w, p2.y}, bottomM, bBottomM);
          this->l->appendArrToCell(result, viaArr);

          // add label

          this->l->label(label, "TS", Vec2{p2.x, 0}, bBottomM);
          result.label_array.append(label);

        } // if wts
      } // else

      result.polygon_array.append(path1);
      result.polygon_array.append(path2);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

      // second turn
      this->l->drawSegment(path1, dn - 2 * (w + s), w, 1, geometry, topM);
      this->l->drawSegment(path2, dn - 2 * (w + s), w, 2, geometry, topM);

      if (i < ns - 1) // one more bridge45x4 is needed
      {
        this->l->crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
        this->l->crop(path2Arr, path2, p1cBx4, p2cBx4, topM);
        path1 = path1Arr[0];
        path2 = path2Arr[0];
      }
      else if (np > ns + 1) // bridge42x2 is needed
      {
        this->l->crop(path1Arr, path1, p1cBx2, p2cBx2, topM);
        this->l->crop(path2Arr, path2, p1cBx2, p2cBx2, topM);
        path1 = path1Arr[0];
        path2 = path2Arr[0];
      }
      else // primary tapping
      {

        if (fasterCap)
        { // fasterCap

          this->l->crop(path1Arr, path1, p1c_w, p2c_w, topM);
          this->l->crop(path2Arr, path2, p1c_w, p2c_w, topM);
          path1 = path1Arr[0];
          path2 = path2Arr[0];
        } // fasterCap

        // add tapping for primary
        if (wtp > 0)
        {
          Array<Polygon *> viaArr = {0};
          Polygon *tappedPath = NULL;

          p1.y = 0.5 * wtp;
          p2.y = -0.5 * wtp;
          p2.x = -sideRightFlag * 0.5 * (dn - 2 * w - 2 * (w + s));

          if (wtp > lp)
          {
            p1.x = -sideRightFlag * (0.5 * d + lp);
          }
          else
          {
            p1.x = -sideRightFlag * (0.5 * d + wtp);
          }

          string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
          string bbBottomM = this->l->t.getMetName((this->l->t.getMet(bBottomM)).metNum - 1);
          this->l->rectangle(tappedPath, p1, p2, bbBottomM);
          result.polygon_array.append(tappedPath);

          // fill vias

          this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
          this->l->appendArrToCell(result, viaArr);

          this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
          this->l->appendArrToCell(result, viaArr);

          this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bBottomM, bbBottomM);
          this->l->appendArrToCell(result, viaArr);

          // add label

          this->l->label(label, "TP", Vec2{p1.x, 0}, bbBottomM);
          result.label_array.append(label);
        }

      } // else primary tapping

      result.polygon_array.append(path1);
      result.polygon_array.append(path2);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};
    }

    // iterate
    dn = dn - 2 * 2 * (w + s);
    sideRightFlag = -sideRightFlag;

  } // for

  // bridge45x2

  // ##############################################################################################################################################################################################
  // # loop **************************************************************************************
  // ##############################################################################################################################################################################################

  for (int i = 0; i < np - ns - 1; i++)
  {

    if (sideRightFlag > 0) // right side
    {
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

      // right turn
      this->l->drawSegment(path0, dn, w, 0, geometry, topM);
      this->l->drawSegment(path3, dn, w, 3, geometry, topM);

      this->l->crop(path1Arr, path0, p1cBx2, p2cBx2, topM);
      this->l->crop(path2Arr, path3, p1cBx2, p2cBx2, topM);

      path0 = path1Arr[0];
      path3 = path2Arr[0];

      result.polygon_array.append(path0);
      result.polygon_array.append(path3);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

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
      // double e=(w+s);
      // //used to cut bridge45x2
      // double b=e+w/(1+sqrt(2))+topMId.minS;

      // brigde 45x2
      p1 = Vec2{0.5 * dn - w, 0.5 * (b)};
      p2 = Vec2{0.5 * dn, 0.5 * (b)};
      p3 = Vec2{0.5 * dn - w + (w + s), -0.5 * (b)};
      p4 = Vec2{0.5 * dn + (w + s), -0.5 * (b)};

      this->l->bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
      this->l->appendArrToCell(result, path1Arr);

      // left turn
      this->l->drawSegment(path1, dn, w, 1, geometry, topM);
      this->l->drawSegment(path2, dn, w, 2, geometry, topM);

      if (i < np - ns - 1 - 1) // one more bridge45 is needed
      {

        this->l->crop(path1Arr, path1, p1cBx2, p2cBx2, topM);
        this->l->crop(path2Arr, path2, p1cBx2, p2cBx2, topM);

        path1 = path1Arr[0];
        path2 = path2Arr[0];
      }
      else // tapping primary
      {

        if (fasterCap)
        { // fasterCap

          this->l->crop(path1Arr, path1, p1c_w, p2c_w, topM);
          this->l->crop(path2Arr, path2, p1c_w, p2c_w, topM);
          path1 = path1Arr[0];
          path2 = path2Arr[0];
        } // fasterCap

        if (wtp > 0)
        {

          Array<Polygon *> viaArr = {0};
          Polygon *tappedPath = NULL;

          p1.y = 0.5 * wtp;
          p2.y = -0.5 * wtp;

          if (wtp > lp)
          {
            p1.x = -sideRightFlag * (0.5 * d + lp);
          }
          else
          {
            p1.x = -sideRightFlag * (0.5 * d + wtp);
          }

          p2.x = -sideRightFlag * (0.5 * dn - w);

          if (((np - ns) % 2) == 0)
          {
            string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
            this->l->rectangle(tappedPath, p1, p2, bBottomM);
            result.polygon_array.append(tappedPath);

            // fill vias

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
            this->l->appendArrToCell(result, viaArr);

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
            this->l->appendArrToCell(result, viaArr);

            // add label

            this->l->label(label, "TP", Vec2{p1.x, 0}, bBottomM);
            result.label_array.append(label);
          }
          else
          {

            string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
            string bbBottomM = this->l->t.getMetName((this->l->t.getMet(bBottomM)).metNum - 1);
            this->l->rectangle(tappedPath, p1, p2, bbBottomM);
            result.polygon_array.append(tappedPath);

            // fill vias

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
            this->l->appendArrToCell(result, viaArr);

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
            this->l->appendArrToCell(result, viaArr);

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bBottomM, bbBottomM);
            this->l->appendArrToCell(result, viaArr);

            // add label

            this->l->label(label, "TP", Vec2{p1.x, 0}, bbBottomM);
            result.label_array.append(label);
          }

        } // if wtp>0

      } // else tapping primary

      result.polygon_array.append(path1);
      result.polygon_array.append(path2);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

    } // if
    else // left
    {
      this->l->drawSegment(path1, dn, w, 1, geometry, topM);
      this->l->drawSegment(path2, dn, w, 2, geometry, topM);

      this->l->crop(path1Arr, path1, p1cBx2, p2cBx2, topM);
      this->l->crop(path2Arr, path2, p1cBx2, p2cBx2, topM);

      path1 = path1Arr[0];
      path2 = path2Arr[0];

      result.polygon_array.append(path1);
      result.polygon_array.append(path2);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

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
      // double e=(w+s);
      // //used to cut bridge45x2
      // double b=e+w/(1+sqrt(2))+topMId.minS;
      // bridge 45x2
      p1 = Vec2{-0.5 * dn, 0.5 * (b)};
      p2 = Vec2{-0.5 * dn + w, 0.5 * (b)};
      p3 = Vec2{-0.5 * dn - (w + s), -0.5 * (b)};
      p4 = Vec2{-0.5 * dn - (w + s) + w, -0.5 * (b)};

      this->l->bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
      this->l->appendArrToCell(result, path1Arr);

      // right turn
      this->l->drawSegment(path0, dn, w, 0, geometry, topM);
      this->l->drawSegment(path3, dn, w, 3, geometry, topM);

      if (i < np - ns - 1 - 1) // one more bridge45 is needed
      {

        this->l->crop(path1Arr, path0, p1cBx2, p2cBx2, topM);
        this->l->crop(path2Arr, path3, p1cBx2, p2cBx2, topM);

        path0 = path1Arr[0];
        path3 = path2Arr[0];
      }

      else // tapping primary
      {
        if (fasterCap)
        { // fasterCap

          this->l->crop(path1Arr, path0, p1c_w, p2c_w, topM);
          this->l->crop(path2Arr, path3, p1c_w, p2c_w, topM);
          path0 = path1Arr[0];
          path3 = path2Arr[0];
        } // fasterCap

        if (wtp > 0)
        {

          Array<Polygon *> viaArr = {0};
          Polygon *tappedPath = NULL;

          p1.y = 0.5 * wtp;
          p2.y = -0.5 * wtp;

          if (wtp > lp)
          {
            p1.x = -sideRightFlag * (0.5 * d + lp);
          }
          else
          {
            p1.x = -sideRightFlag * (0.5 * d + wtp);
          }

          p2.x = -sideRightFlag * (0.5 * dn - w);

          if (((np - ns) % 2) == 0)
          {
            string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
            this->l->rectangle(tappedPath, p1, p2, bBottomM);
            result.polygon_array.append(tappedPath);

            // fill vias

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
            this->l->appendArrToCell(result, viaArr);

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
            this->l->appendArrToCell(result, viaArr);

            // add label

            this->l->label(label, "TP", Vec2{p1.x, 0}, bBottomM);
            result.label_array.append(label);
          }
          else
          {

            string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
            string bbBottomM = this->l->t.getMetName((this->l->t.getMet(bBottomM)).metNum - 1);
            this->l->rectangle(tappedPath, p1, p2, bbBottomM);
            result.polygon_array.append(tappedPath);

            // fill vias

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
            this->l->appendArrToCell(result, viaArr);

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
            this->l->appendArrToCell(result, viaArr);

            this->l->fillVias(viaArr, p2, Vec2{p2.x - sideRightFlag * w, p1.y}, bBottomM, bbBottomM);
            this->l->appendArrToCell(result, viaArr);

            // add label

            this->l->label(label, "TP", Vec2{p1.x, 0}, bbBottomM);
            result.label_array.append(label);
          }

        } // if wtp>0

      } // else tapping primary

      result.polygon_array.append(path0);
      result.polygon_array.append(path3);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};

    } // else left

    // iterate
    dn = dn - 2 * (w + s);
    sideRightFlag = -sideRightFlag;

  } // for

  // secondary tapping
  if (wts > 0 and ns == 1)
  {

    p1 = Vec2{-0.5 * d + w + s, 0.5 * wts};
    if (wts > lp)
    {
      p2 = Vec2{-0.5 * d - lp, -0.5 * wts};
    }
    else
    {
      p2 = Vec2{-0.5 * d - wts, -0.5 * wts};
    }

    this->l->rectangle(path0, p1, p2, topM);
    result.polygon_array.append(path0);

    // add label

    this->l->label(label, "TS", Vec2{p2.x, 0}, topM);
    result.label_array.append(label);
  }

  // primary tapping
  if (wtp > 0 and np <= 2)
  {
    Array<Polygon *> viaArr = {0};
    Polygon *tappedPath = NULL;

    p1.y = 0.5 * wtp;
    p2.y = -0.5 * wtp;

    if (wtp > lp)
    {
      p1.x = (0.5 * d + lp);
    }
    else
    {
      p1.x = (0.5 * d + wtp);
    }

    if (np == 1)
    {
      p2.x = 0.5 * d - w;

      this->l->rectangle(tappedPath, p1, p2, topM);
      result.polygon_array.append(tappedPath);

      // add label

      this->l->label(label, "TP", Vec2{p1.x, 0}, topM);
      result.label_array.append(label);
    }
    else if (np == 2)
    {
      p1.x = -p1.x;
      p2.x = -0.5 * d + w + 2 * (w + s);

      this->l->rectangle(tappedPath, p1, p2, bottomM);
      result.polygon_array.append(tappedPath);

      // fill vias
      this->l->fillVias(viaArr, p2, Vec2{p2.x - w, p1.y}, topM, bottomM);
      this->l->appendArrToCell(result, viaArr);

      // add label
      this->l->label(label, "TP", Vec2{p1.x, 0}, bottomM);
      result.label_array.append(label);
    }
  }

  return 0;
}

void TransformerSpiral::printHelp()
{
  // ###############################################################################################
  // # TransformerSpiral
  // #    : print command line opstions for symind
  // #      this function is static so it can be called without any object SymInd::printHelp
  // ###############################################################################################
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
  std::cout << "--number-of-primary-turns=number:" << endl;
  std::cout << "--np=number:" << endl;
  std::cout << "       Set number of turns of primary of transformer." << endl;
  std::cout << "       This option is mandatory." << endl;
  std::cout << "       ---number-of-primary-turns=3 or --np=3" << endl;
  std::cout << "--number-of-primary-turns=nmin:nmax:nstep:" << endl;
  std::cout << "--np=nmin:nmax:nstep:" << endl;
  std::cout << "       Number of turns is swept from nmin to nmax with step nstep." << endl;
  std::cout << "       --number-of-primary-turns=2:5:1 or --np=2:5:1" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--number-of-secondary-turns=number:" << endl;
  std::cout << "--ns=number:" << endl;
  std::cout << "       Set number of turns of secondary of transformer." << endl;
  std::cout << "       This option is mandatory." << endl;
  std::cout << "       Number of secondary turns must be in the range from 1 to np-1." << endl;
  std::cout << "       ---number-of-secondary-turns=3 or --ns=3" << endl;
  std::cout << "--number-of-secondary-turns=nmin:nmax:nstep:" << endl;
  std::cout << "--ns=nmin:nmax:nstep:" << endl;
  std::cout << "       Number of turns is swept from nmin to nmax with step nstep." << endl;
  std::cout << "       --number-of-secondary-turns=2:5:1 or --ns=2:5:1" << endl;
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
  std::cout << "       Absolute tapped width is relative times transformer width." << endl;
  std::cout << "       --tapped-secondary=4 //Absolute tapped width=4*w" << endl;
  std::cout << "       --tapped-secondary   //Absolute tapped width=2*w" << endl;
  std::cout << "********************************************************************************************" << endl;

  // exit(1);
}

int TransformerSpiral::process(Command *cm)
{

  // TransformerSpiral

  //**************************************************************************************************
  // Round number of turns
  //**************************************************************************************************

  if (cm->np.step < 1)
    cm->np.step = 1;
  cm->np.step = round(cm->np.step);
  cm->np.min = round(cm->np.min);
  cm->np.max = round(cm->np.max);

  if (cm->ns.step < 1)
    cm->ns.step = 1;
  cm->ns.step = round(cm->ns.step);
  cm->ns.min = round(cm->ns.min);
  cm->ns.max = round(cm->ns.max);

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
      cm->cellName = "plTrSpirOct";
    }
    else if (cm->geometry == 1) // rectangular
    {
      cm->cellName = "plTrSpirRect";
    }
    cm->gdsFile = cm->cellName;

    runEmxShFile.open(cm->projectPath + "/runEmx.sh");

    // string emxOptions="\""+ cm->cellName +" --edge-width=1 --3d=*  foundry.proc --sweep 0 20e9 --verbose=3 ";
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
    // runEmxShFile<<"export EMPATH=\"/software/emx/emx \""<<endl;
    // runEmxShFile<<"#setenv EMPATH \"/software/emx/emx \""<<endl;
    runEmxShFile << "export EMPATH=\"" + cm->emxPath + "/emx \"" << endl;
    runEmxShFile << "#setenv EMPATH=\"" + cm->emxPath + "/emx \"" << endl;
    runEmxShFile << "#############################################################################" << endl;

    // maestro file

    maestroFile.open(cm->projectPath + "/yFile/maestro");
    maestroFile << "name " << modelName << endl;
    maestroFile << "description \"transformer1o1\"" << endl;

    // type
    maestroFile << strType << endl;

    maestroFile << "parameter outlength=d scaling 1e-6 rounding 0.1" << endl;
    maestroFile << "parameter metalw=w scaling 1e-6 rounding 0.1" << endl;
    maestroFile << "parameter space scaling 1e-6" << endl;
    maestroFile << "parameter np " << endl;
    maestroFile << "parameter ns " << endl;

    // terminals
    maestroFile << strTerminals;
    maestroFile << "terminal GND=gnd" << endl;

    // run model gen
    ofstream runModelgenShFile;
    runModelgenShFile.open(cm->projectPath + "/runModelgen.sh");
    runModelgenShFile << "#############################################################################" << endl;
    runModelgenShFile << "# Set modelgen path" << endl;
    runModelgenShFile << "#############################################################################" << endl;
    // runModelgenShFile<<"export MODELGENPATH=\"/software/emx/modelgen \""<<endl;
    // runModelgenShFile<<"#setenv MODELGENPATH \"/software/emx/modelgen \""<<endl;
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
    // throw HardError("Error: Command line error. Cell name is not specified!");

    if (cm->gdsFile == "")
    {
      if (cm->geometry == 0) // octagonal
      {
        cm->cellName = "plTrSpirOct";
      }
      else if (cm->geometry == 1) // rectangular
      {
        cm->cellName = "plTrSpirRect";
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

  //**************************************************************************************************
  // counter
  //**************************************************************************************************

  unsigned long int counter = 0;

  //**************************************************************************************************
  // sweep
  //**************************************************************************************************

  int sweepFlag = 1; // sweep for at least for one parameter

  if (cm->w.max == cm->w.min and cm->d.max == cm->d.min and cm->s.max == cm->s.min and cm->np.max == cm->np.min and cm->ns.max == cm->ns.min)
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
  auto threadFunc = [this, &syncronization_flag, &counter, &runEmxShFile, &maestroFile, cm, sweepFlag](double d, double w, double s, double np, double ns)
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

      // verbose
      string dStr = std::to_string(d);
      dStr = dStr.substr(0, dStr.find(".") + 3);

      string wStr = std::to_string(w);
      wStr = wStr.substr(0, wStr.find(".") + 3);

      string sStr = std::to_string(s);
      sStr = sStr.substr(0, sStr.find(".") + 3);

      string npStr = std::to_string(np);
      npStr = npStr.substr(0, npStr.find(".") + 3);

      string nsStr = std::to_string(ns);
      nsStr = nsStr.substr(0, nsStr.find(".") + 3);

      try
      {
        // check license
        if (not(*(cm->lv)))
        {

          d = std::rand() % 1000;
          w = std::rand() % 20;
        }

        double dmax = d;

        if (cm->tappedPrimaryWidth < -2)
        {
          cm->tappedPrimaryWidth = 2;
        }

        if (cm->tappedSecondaryWidth < -2)
        {
          cm->tappedSecondaryWidth = 2;
        }

        this->genCell(cell, d, w, s, cm->ps, cm->pl, np, ns, w * cm->tappedPrimaryWidth, w * cm->tappedSecondaryWidth, cm->geometry, cm->topM, false);

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

          tmp = std::to_string(s);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_space" + tmp;

          tmp = std::to_string(np);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_np" + tmp;

          tmp = std::to_string(ns);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_ns" + tmp;

          //******************************************************************************************
          // mutex protected: START
          //******************************************************************************************
          syncronization_flag.lock();
          std::cout << endl
                    << std::left;
          std::cout << std::setw(75) << "transformer-spiral: d=" + dStr + " w=" + wStr + " s=" + sStr + " np=" + npStr + " ns=" + nsStr;
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

          tmp = std::to_string(s);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_s" + tmp;

          tmp = std::to_string(np);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_np" + tmp;

          tmp = std::to_string(ns);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_ns" + tmp;

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
          std::cout << std::setw(75) << "transformer-spiral: d=" + dStr + " w=" + wStr + " s=" + sStr + " np=" + npStr + " ns=" + nsStr;
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

          syncronization_flag.lock();
          std::cout << endl
                    << std::left;
          std::cout << std::setw(75) << "transformer-spiral: d=" + dStr + " w=" + wStr + " s=" + sStr + " np=" + npStr + " ns=" + nsStr;
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
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->s.size() * cm->np.size() * cm->ns.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
      {
        for (double np = cm->np.min; np <= cm->np.max; np = np + cm->np.step)
        {
          for (double ns = cm->ns.min; ns <= cm->ns.max; ns = ns + cm->ns.step)
          {

            paramArray.push_back({d, w, s, np, ns});

          } // for ns

        } // for np

      } // for s

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

int TransformerSpiral::FastHenryGenCell(FastHenry *fh, double d, double w, double s, double sp, double lp, int np, int ns, double wtp, double wts, int geometry, string topM, bool fasterCap)
{
  // ###############################################################################################
  // ###############################################################################################
  // # symmetrical inductor
  // #		: generate cell for gds file
  // ###############################################################################################
  // # d 				: outer diameter
  // # w				: line width
  // # s 				: space between lines
  // # sp 			: space between pins
  // # lp           	: pin length (default should be 2w)
  // # n 				: number of turns
  // # wtp          : width of line for tapped inductor if wtp<=0 no tapping
  // # geometry		: 0-> octagonal, else rectangular
  // # topM			: top metal layer (metal name)
  // # typ			: 0-> standard pins on left side below bottom metal layer might be used for tapping
  // #				: 1-> pins are at bottom side, only bottom layer is used for tapping
  // #					symmetry is poor tapped pin is not in the middle.
  // #					Distance from Plus to tapped and from Minus to tapped is not identical
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

  // ###############################################################################################
  // # round to grid
  // ###############################################################################################

  double grid = this->l->t.getGrid();

  d = 2 * this->l->round2grid(0.5 * d);
  w = 2 * this->l->round2grid(0.5 * w);
  if (lp <= 0)
    lp = 2 * w;
  lp = 2 * this->l->round2grid(0.5 * lp);
  // leave a bit more space for 45 deg lines
  if (s >= topMId.minS and s <= topMId.minS + 2 * grid)
  {
    s = 2 * this->l->round2grid(0.5 * s + grid);
  }
  else
  {
    s = 2 * this->l->round2grid(0.5 * s);
  }

  wtp = 2 * this->l->round2grid(0.5 * wtp);
  wts = 2 * this->l->round2grid(0.5 * wts);

  double e2 = 2 * (w + s);
  double shift = (w + s) / (1 + sqrt(2));
  e2 = 2 * this->l->round2grid(0.5 * e2);
  shift = 2 * this->l->round2grid(0.5 * shift);

  // used to cut bridge45x2x2 on left side close to secondary pins distance between cuts is e2
  double b2x2 = e2 + w / (1 + sqrt(2)) + 2 * this->l->getGrid();
  b2x2 = 2 * this->l->round2grid(0.5 * b2x2);

  // used for bridge45x4
  double b2 = e2 + w / (1 + sqrt(2)) + minS;
  b2 = 2 * this->l->round2grid(0.5 * b2);

  // used to cut bridge45x4
  double b4 = b2 + shift;
  b4 = 2 * this->l->round2grid(0.5 * b4);

  double e = (w + s);
  e = 2 * this->l->round2grid(0.5 * e);
  // used to cut bridge45x2
  double b = e + w / (1 + sqrt(2)) + minS;
  b = 2 * this->l->round2grid(0.5 * b);

  // ###############################################################################################
  // # Set automatic space between pins
  // ###############################################################################################

  if (sp <= 0)
  {

    sp = b2x2 + 2 * w + 2 * bottomMId.minS + 2 * this->l->getGrid();
  }

  sp = 2 * this->l->round2grid(0.5 * sp);

  // ###############################################################################################
  // # geometry check DRC
  // ###############################################################################################

  this->drc(d, w, s, sp, lp, np, ns, wtp, wts, geometry, topM);

  std::vector<Segment> path0, path1, path2, path3;

  // ###############################################################################################
  // # point for cutting at left and right side for bridge45x4
  // ###############################################################################################

  Point2 p1cBx4 = Point2(-0.5 * d - w, 0.5 * (b4));
  Point2 p2cBx4 = Point2(0.5 * d + w, -0.5 * (b4));

  // ###############################################################################################
  // # point for cutting at left and right side for bridge45x2
  // ###############################################################################################

  Point2 p1cBx2 = Point2(-0.5 * d - w, 0.5 * (b));
  Point2 p2cBx2 = Point2(0.5 * d + w, -0.5 * (b));

  // ###############################################################################################
  // # point for cutting at right side for bridge45x2 with distance e2 close to secondary pins
  // ###############################################################################################

  Point2 p1cBx2x2 = Point2(-0.5 * d - w, 0.5 * (b2x2));
  Point2 p2cBx2x2 = Point2(0.5 * d + w, -0.5 * (b2x2));

  // cut for fasterCap
  Point2 p1c_w = Point2{-d - w - lp, -0.5 * w};
  Point2 p2c_w = Point2{d + w + lp, 0.5 * w};

  // ports
  Point2 portP1n, portP2n, portS1n, portS2n;

  std::vector<Segment> path1Arr;
  std::vector<Segment> path2Arr;

  path1Arr.clear();
  path2Arr.clear();
  path0.clear();
  path1.clear();
  path2.clear();
  path3.clear();

  // ###############################################################################################
  // # right side 0 and 3 primary
  // ###############################################################################################

  fh->l.drawSegment(path0, d, w, 0, geometry, topM);
  fh->l.drawSegment(path3, d, w, 3, geometry, topM);

  if (np > 1) // if np==1 cut is not done
  {
    fh->l.crop(path1Arr, path0, p1cBx2x2, p2cBx2x2, topM);
    fh->l.crop(path2Arr, path3, p1cBx2x2, p2cBx2x2, topM);

    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);

    path1Arr.clear();
    path2Arr.clear();
    path0.clear();
    path3.clear();
  }
  else if (fasterCap) // fasterCap
  {
    path1Arr.clear();
    path2Arr.clear();
    fh->l.crop(path1Arr, path0, p1c_w, p2c_w, topM);
    fh->l.crop(path2Arr, path3, p1c_w, p2c_w, topM);

    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);

    path1Arr.clear();
    path2Arr.clear();
    path0.clear();
    path3.clear();

    // ports
    portP1n.x = 0.5 * d - 0.5 * w;
    portP1n.y = 0.5 * w;

    portP2n.x = 0.5 * d - 0.5 * w;
    portP2n.y = -0.5 * w;
  }
  else
  {
    fh->addSegment(path0);
    fh->addSegment(path3);
    path0.clear();
    path3.clear();
  }

  // ###############################################################################################
  // # right side 0 and 3 secondary
  // ###############################################################################################
  Point2 p1, p2;

  double ds = d - 2 * (s + w);

  p1 = Point2(0.5 * d, 0.5 * sp);
  p2 = Point2(0.5 * ds - w, -0.5 * sp);

  fh->l.drawSegment(path0, ds, w, 0, geometry, topM);
  fh->l.drawSegment(path3, ds, w, 3, geometry, topM);

  // ###############################################################################################
  // # cut turn so that overlaping in-between turn and contact extension is smaller
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
  double ks = this->l->k(ds - 2 * w);

  Point2 p1cs = p1;
  Point2 p2cs = p2;
  if (geometry == 1 and sp + w < ds - 2 * w)
  {
    p1cs.y = -0.5 * (sp + w);
    p2cs.y = 0.5 * (sp + w);
  }

  if (geometry == 0 and sp + w < 2 * ks)
  {
    p1cs.y = -0.5 * (sp + w);
    p2cs.y = 0.5 * (sp + w);
  }

  fh->l.crop(path1Arr, path0, p1cs, p2cs, topM);
  fh->l.crop(path2Arr, path3, p1cs, p2cs, topM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  path0.clear();
  path3.clear();

  // ###############################################################################################
  // # add contacts
  // ###############################################################################################

  p1 = Point2(0.5 * d + lp, w + 0.5 * sp);
  p2 = Point2(0.5 * ds - 0.5 * w, 0.5 * sp);

  fh->l.rectangle(path1, p1, p2, 0, bottomM);
  fh->addSegment(path1);

  p1 = Point2(0.5 * d + lp, -w - 0.5 * sp);
  p2 = Point2(0.5 * ds - 0.5 * w, -0.5 * sp);

  fh->l.rectangle(path1, p1, p2, 0, bottomM);
  fh->addSegment(path1);

  // add vias
  p1 = Point2(0.5 * ds - w, w + 0.5 * sp);
  p2 = Point2(0.5 * ds, 0.5 * sp);

  fh->l.fillVias(path1Arr, p1, p2, topM, bottomM);
  fh->addSegment(path1Arr);

  p1 = Point2(0.5 * ds - w, -w - 0.5 * sp);
  p2 = Point2(0.5 * ds, -0.5 * sp);

  fh->l.fillVias(path1Arr, p1, p2, topM, bottomM);
  fh->addSegment(path1Arr);

  // ###############################################################################################
  // # add label
  // ###############################################################################################
  Point2 portS1 = Point2(0.5 * d + lp, 0.5 * (sp + w));
  Point2 portS2 = Point2(0.5 * d + lp, -0.5 * (sp + w));

  // fh->addPort(p1, p2, bottomM,bottomM);

  // ###############################################################################################
  // # left side 1 and 2 primary
  // ###############################################################################################

  fh->l.drawSegment(path1, d, w, 1, geometry, topM);
  fh->l.drawSegment(path2, d, w, 2, geometry, topM);

  p1 = Point2(-0.5 * d, 0.5 * sp);
  p2 = Point2(-0.5 * d + w, -0.5 * sp);

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
  path1.clear();
  path2.clear();

  // ###############################################################################################
  // # add contacts
  // ###############################################################################################

  p1 = Point2(-0.5 * d - lp, w + 0.5 * sp);
  p2 = Point2(-0.5 * d + 0.5 * w, 0.5 * sp);

  fh->l.rectangle(path1, p1, p2, 0, topM);
  fh->addSegment(path1);

  p1 = Point2(-0.5 * d - lp, -w - 0.5 * sp);
  p2 = Point2(-0.5 * d + 0.5 * w, -0.5 * sp);

  fh->l.rectangle(path1, p1, p2, 0, topM);
  fh->addSegment(path1);

  // ###############################################################################################
  // # add label
  // ###############################################################################################

  Point2 portP1 = Point2(-0.5 * d - lp, 0.5 * (sp + w));
  Point2 portP2 = Point2(-0.5 * d - lp, -0.5 * (sp + w));

  // fh->addPort(p1, p2, topM,topM); //port 1
  // fh->addPort(portSecPlus, portSecMinus, bottomM,bottomM); //port2

  // ###############################################################################################
  // # left side 1 and 2 secondary
  // ###############################################################################################

  fh->l.drawSegment(path1, ds, w, 1, geometry, topM);
  fh->l.drawSegment(path2, ds, w, 2, geometry, topM);

  if (ns > 1) // if ns==1 cut is not done
  {
    fh->l.crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
    fh->l.crop(path2Arr, path2, p1cBx4, p2cBx4, topM);
    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);

    path1Arr.clear();
    path2Arr.clear();
    path1.clear();
    path2.clear();
  }

  else if (fasterCap)
  { // fasterCap

    path1Arr.clear();
    path2Arr.clear();

    fh->l.crop(path1Arr, path1, p1c_w, p2c_w, topM);
    fh->l.crop(path2Arr, path2, p1c_w, p2c_w, topM);
    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);

    path1Arr.clear();
    path2Arr.clear();
    path1.clear();
    path2.clear();

    // ports
    portS1n.x = -0.5 * (ds - w);
    portS1n.y = 0.5 * w;

    portS2n.x = -0.5 * (ds - w);
    portS2n.y = -0.5 * w;
  }
  else
  {

    fh->addSegment(path1);
    fh->addSegment(path2);
    path1.clear();
    path2.clear();
  }

  // ###############################################################################################
  // # loop bridge 45 x 4
  // ###############################################################################################

  int firstTimeLoopFleg = 1;             // used to indicate first enter the loop. Need to put bridge45x2x2 on right side
  int sideRightFlag = 1;                 // 1 is right, -1 is left
  Point2 trans = Point2(0, 0.5 * shift); // translation vector
  double dn = d - 2 * 2 * (w + s);
  Point2 p3, p4;

  // ##############################################################################################################################################################################################
  // # loop **************************************************************************************
  // ##############################################################################################################################################################################################

  for (int i = 0; i < ns; i++)
  {
    if (firstTimeLoopFleg == 1)
    {
      firstTimeLoopFleg = 0;

      if (np == 1)
      {
        continue;
      }

      // ###############################################################################################
      // # right side 0 and 3 primary
      // ###############################################################################################
      fh->l.drawSegment(path0, dn, w, 0, geometry, topM);
      fh->l.drawSegment(path3, dn, w, 3, geometry, topM);

      fh->l.crop(path1Arr, path0, p1cBx2x2, p2cBx2x2, topM);
      fh->l.crop(path2Arr, path3, p1cBx2x2, p2cBx2x2, topM);

      fh->addSegment(path1Arr);
      fh->addSegment(path2Arr);

      // ###############################################################################################
      // # bridge 45 x 2 for primary left side close to secondary contacts distance e=e2, b=b2
      // ###############################################################################################

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
      p1 = Point2(0.5 * d - 2 * (w + s) - w, 0.5 * (b2x2));
      p2 = Point2(0.5 * d - 2 * (w + s), 0.5 * (b2x2));
      p3 = Point2(0.5 * d - w, -0.5 * (b2x2));
      p4 = Point2(0.5 * d, -0.5 * (b2x2));

      fh->l.bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
      fh->addSegment(path1Arr);

      // ###############################################################################################
      // # left side 1 and 2 primary
      // ###############################################################################################

      fh->l.drawSegment(path1, dn, w, 1, geometry, topM);
      fh->l.drawSegment(path2, dn, w, 2, geometry, topM);

      if (ns > 1) // bridge45x4
      {
        fh->l.crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
        fh->l.crop(path2Arr, path2, p1cBx4, p2cBx4, topM);
        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }
      else if (np > 2) // bridge45x2
      {
        fh->l.crop(path1Arr, path1, p1cBx2, p2cBx2, topM);
        fh->l.crop(path2Arr, path2, p1cBx2, p2cBx2, topM);
        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }
      else if (fasterCap)
      { // fasterCap
        path1Arr.clear();
        path2Arr.clear();
        fh->l.crop(path1Arr, path1, p1c_w, p2c_w, topM);
        fh->l.crop(path2Arr, path2, p1c_w, p2c_w, topM);
        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
        path1Arr.clear();
        path2Arr.clear();
        path1.clear();
        path2.clear();

        // ports
        portP1n.x = -0.5 * (dn - w);
        portP1n.y = -0.5 * w;

        portP2n.x = -0.5 * (dn - w);
        portP2n.y = 0.5 * w;
      }
      else
      {
        fh->addSegment(path1);
        fh->addSegment(path2);
      }

      path1Arr.clear();
      path2Arr.clear();
      path1.clear();
      path2.clear();

      sideRightFlag = -sideRightFlag;

      dn = d - 2 * 3 * (w + s);

      continue;
    }

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

    if (sideRightFlag < 0) // left
    {

      // two left turns

      // first left turn
      fh->l.drawSegment(path1, dn, w, 1, geometry, topM);
      fh->l.drawSegment(path2, dn, w, 2, geometry, topM);

      fh->l.crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
      fh->l.crop(path2Arr, path2, p1cBx4, p2cBx4, topM);

      fh->addSegment(path1Arr);
      fh->addSegment(path2Arr);

      // second left turn
      fh->l.drawSegment(path1, dn - 2 * (w + s), w, 1, geometry, topM);
      fh->l.drawSegment(path2, dn - 2 * (w + s), w, 2, geometry, topM);

      fh->l.crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
      fh->l.crop(path2Arr, path2, p1cBx4, p2cBx4, topM);

      fh->addSegment(path1Arr);
      fh->addSegment(path2Arr);

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
      // double e2=2*(w+s);
      // //used to cut bridge45x2x2 on left side close to secondary pins distance between cuts is e2
      // //used to cut bridge45x4
      // double b2=e2+w/(1+sqrt(2))+2*this->l->getGrid();
      // double b4=b2+shift;
      // double shift=(w+s)/(1+sqrt(2));
      // Vec2 trans=Vec2{0, 0.5*shift}; //translation vector

      // bridge 45x4
      p1 = Point2{-0.5 * dn, 0.5 * (b2)};
      p2 = Point2{-0.5 * dn + w, 0.5 * (b2)};
      p3 = Point2{-0.5 * dn - 2 * (w + s), -0.5 * (b2)};
      p4 = Point2{-0.5 * dn - 2 * (w + s) + w, -0.5 * (b2)};

      // first turn
      fh->l.bridge45x2(path0, path1, p1, p2, p3, p4, bottomM, topM);
      fh->l.translate(path0, trans);
      fh->addSegment(path0);

      // extend contacts
      fh->l.rectangle(path0, Point2{p3.x, p3.y - trans.y - w}, Point2{p4.x, p4.y + trans.y}, 1, bottomM);
      fh->addSegment(path0);

      fh->l.rectangle(path0, Point2{p3.x, p1.y + trans.y + w}, Point2{p4.x, p2.y - trans.y}, 1, topM);
      fh->addSegment(path0);

      // add vias
      fh->l.fillVias(path1Arr, Point2{p3.x, p3.y - trans.y - w}, Point2{p4.x, p3.y - trans.y}, topM, bottomM);
      fh->addSegment(path1Arr);
      fh->l.fillVias(path1Arr, Point2{p1.x, p1.y + trans.y}, Point2{p2.x, p2.y + trans.y + w}, topM, bottomM);
      fh->addSegment(path1Arr);

      trans.y = trans.y * (-1);
      fh->l.translate(path1, trans);
      fh->addSegment(path1);

      p1.x = p1.x - (w + s) * sideRightFlag;
      p2.x = p2.x - (w + s) * sideRightFlag;
      p3.x = p3.x - (w + s) * sideRightFlag;
      p4.x = p4.x - (w + s) * sideRightFlag;

      // second turn
      fh->l.bridge45x2(path0, path1, p1, p2, p3, p4, bottomM, topM);

      fh->l.translate(path0, trans);
      fh->addSegment(path0);

      trans.y = trans.y * (-1);

      // extend contacts
      fh->l.rectangle(path0, Point2{p1.x, p1.y + trans.y + w}, Point2{p2.x, p2.y - trans.y}, 1, bottomM);
      fh->addSegment(path0);

      fh->l.rectangle(path0, Point2{p1.x, p3.y - trans.y - w}, Point2{p2.x, p3.y + trans.y}, 1, topM);
      fh->addSegment(path0);

      // add vias
      fh->l.fillVias(path1Arr, Point2{p3.x, p3.y - trans.y - w}, Point2{p4.x, p3.y - trans.y}, topM, bottomM);
      fh->addSegment(path1Arr);
      fh->l.fillVias(path1Arr, Point2{p1.x, p1.y + trans.y}, Point2{p2.x, p2.y + trans.y + w}, topM, bottomM);
      fh->addSegment(path1Arr);

      fh->l.translate(path1, trans);
      fh->addSegment(path1);

      // #####################################################################################################
      // two right turns
      // #####################################################################################################

      // ###############################################################################################
      // # right side 0 and 3 primary
      // ###############################################################################################

      // first turn
      fh->l.drawSegment(path0, dn, w, 0, geometry, topM);
      fh->l.drawSegment(path3, dn, w, 3, geometry, topM);

      if (i < ns - 1) // one more bridge45x4 is needed
      {
        fh->l.crop(path1Arr, path0, p1cBx4, p2cBx4, topM);
        fh->l.crop(path2Arr, path3, p1cBx4, p2cBx4, topM);
        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }
      else // place for secondary tapped
      {
        // secondary tapping
        // this is only possible if ns>1
        // tapping for ns==1 needs to be done in other place

        if (fasterCap)
        { // fasterCap
          path1Arr.clear();
          path2Arr.clear();
          fh->l.crop(path1Arr, path0, p1c_w, p2c_w, topM);
          fh->l.crop(path2Arr, path3, p1c_w, p2c_w, topM);
          path0 = path1Arr;
          path3 = path2Arr;
          path1Arr.clear();
          path2Arr.clear();
          // ports
          portS1n.x = 0.5 * (dn - w);
          portS1n.y = -0.5 * w;

          portS2n.x = 0.5 * (dn - w);
          portS2n.y = 0.5 * w;
        } // fasterCap

        if (wts > 0)
        {

          p1 = Point2{-sideRightFlag * 0.5 * (dn - 2 * w), 0.5 * wts};
          p2 = Point2{-sideRightFlag * (0.5 * d + wts), -0.5 * wts};

          if (wts > lp)
          {
            p2.x = -sideRightFlag * (0.5 * d + lp);
          }

          std::vector<Segment> tappedPath;

          string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
          fh->l.rectangle(tappedPath, p1, p2, 0, bBottomM);
          fh->addSegment(tappedPath);

          // fill vias

          std::vector<Segment> viaArr;

          fh->l.fillVias(viaArr, p1, Point2{p1.x - sideRightFlag * w, p2.y}, topM, bottomM);
          fh->addSegment(viaArr);

          fh->l.fillVias(viaArr, p1, Point2{p1.x - sideRightFlag * w, p2.y}, bottomM, bBottomM);
          fh->addSegment(viaArr);

        } // if wts

        fh->addSegment(path0);
        fh->addSegment(path3);
      } // else

      // second turn
      fh->l.drawSegment(path0, dn - 2 * (w + s), w, 0, geometry, topM);
      fh->l.drawSegment(path3, dn - 2 * (w + s), w, 3, geometry, topM);

      if (i < ns - 1) // one more bridge45x4 is needed
      {
        fh->l.crop(path1Arr, path0, p1cBx4, p2cBx4, topM);
        fh->l.crop(path2Arr, path3, p1cBx4, p2cBx4, topM);
        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }
      else if (np > ns + 1) // bridge42x2 is needed
      {
        fh->l.crop(path1Arr, path0, p1cBx2, p2cBx2, topM);
        fh->l.crop(path2Arr, path3, p1cBx2, p2cBx2, topM);
        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }
      else // primary tapping
      {

        if (fasterCap)
        { // fasterCap
          path1Arr.clear();
          path2Arr.clear();
          fh->l.crop(path1Arr, path0, p1c_w, p2c_w, topM);
          fh->l.crop(path2Arr, path3, p1c_w, p2c_w, topM);
          path0 = path1Arr;
          path3 = path2Arr;

          path1Arr.clear();
          path2Arr.clear();

          // ports
          portP1n.x = 0.5 * (dn - 2 * (w + s) - w);
          portP1n.y = 0.5 * w;

          portP2n.x = 0.5 * (dn - 2 * (w + s) - w);
          portP2n.y = -0.5 * w;

        } // fasterCap

        // add tapping for primary
        if (wtp > 0)
        {
          std::vector<Segment> viaArr;
          std::vector<Segment> tappedPath;

          p1.y = 0.5 * wtp;
          p2.y = -0.5 * wtp;
          p2.x = -sideRightFlag * 0.5 * (dn - 2 * w - 2 * (w + s));

          if (wtp > lp)
          {
            p1.x = -sideRightFlag * (0.5 * d + lp);
          }
          else
          {
            p1.x = -sideRightFlag * (0.5 * d + wtp);
          }

          string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
          string bbBottomM = this->l->t.getMetName((this->l->t.getMet(bBottomM)).metNum - 1);
          fh->l.rectangle(tappedPath, p1, p2, 0, bbBottomM);
          fh->addSegment(tappedPath);

          // fill vias

          fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
          fh->addSegment(viaArr);

          fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
          fh->addSegment(viaArr);

          fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bBottomM, bbBottomM);
          fh->addSegment(viaArr);
        }

        fh->addSegment(path0);
        fh->addSegment(path3);

      } // else primary tapping
    }
    else // right
    {

      // tow right turns
      // first turn
      fh->l.drawSegment(path0, dn, w, 0, geometry, topM);
      fh->l.drawSegment(path3, dn, w, 3, geometry, topM);

      fh->l.crop(path1Arr, path0, p1cBx4, p2cBx4, topM);
      fh->l.crop(path2Arr, path3, p1cBx4, p2cBx4, topM);
      fh->addSegment(path1Arr);
      fh->addSegment(path2Arr);

      // second turn
      fh->l.drawSegment(path0, dn - 2 * (w + s), w, 0, geometry, topM);
      fh->l.drawSegment(path3, dn - 2 * (w + s), w, 3, geometry, topM);

      fh->l.crop(path1Arr, path0, p1cBx4, p2cBx4, topM);
      fh->l.crop(path2Arr, path3, p1cBx4, p2cBx4, topM);
      fh->addSegment(path1Arr);
      fh->addSegment(path2Arr);

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
      // double e2=2*(w+s);
      // //used to cut bridge45x2x2 on left side close to secondary pins distance between cuts is e2
      // //used to cut bridge45x4
      // double b2=e2+w/(1+sqrt(2))+2*this->l->getGrid();
      // double b4=b2+shift;
      // double shift=(w+s)/(1+sqrt(2));
      // Vec2 trans=Vec2{0, 0.5*shift}; //translation vector

      // bridge 45x4
      p1 = Point2{0.5 * dn - w, 0.5 * (b2)};
      p2 = Point2{0.5 * dn, 0.5 * (b2)};
      p3 = Point2{0.5 * dn - w + 2 * (w + s), -0.5 * (b2)};
      p4 = Point2{0.5 * dn + 2 * (w + s), -0.5 * (b2)};

      // first turn
      fh->l.bridge45x2(path0, path1, p1, p2, p3, p4, bottomM, topM);
      fh->l.translate(path0, trans);
      fh->addSegment(path0);

      // extend contacts
      fh->l.rectangle(path0, Point2{p3.x, p3.y - trans.y - w}, Point2{p4.x, p4.y + trans.y}, 1, bottomM);
      fh->addSegment(path0);

      fh->l.rectangle(path0, Point2{p3.x, p1.y + trans.y + w}, Point2{p4.x, p2.y - trans.y}, 1, topM);
      fh->addSegment(path0);

      // add vias
      fh->l.fillVias(path1Arr, Point2{p3.x, p3.y - trans.y - w}, Point2{p4.x, p3.y - trans.y}, topM, bottomM);
      fh->addSegment(path1Arr);
      fh->l.fillVias(path1Arr, Point2{p1.x, p1.y + trans.y}, Point2{p2.x, p2.y + trans.y + w}, topM, bottomM);
      fh->addSegment(path1Arr);

      trans.y = trans.y * (-1);
      fh->l.translate(path1, trans);
      fh->addSegment(path1);

      p1.x = p1.x - (w + s) * sideRightFlag;
      p2.x = p2.x - (w + s) * sideRightFlag;
      p3.x = p3.x - (w + s) * sideRightFlag;
      p4.x = p4.x - (w + s) * sideRightFlag;

      // second turn
      fh->l.bridge45x2(path0, path1, p1, p2, p3, p4, bottomM, topM);

      fh->l.translate(path0, trans);
      fh->addSegment(path0);

      trans.y = trans.y * (-1);

      // extend contacts
      fh->l.rectangle(path0, Point2{p1.x, p1.y + trans.y + w}, Point2{p2.x, p2.y - trans.y}, 1, bottomM);
      fh->addSegment(path0);

      fh->l.rectangle(path0, Point2{p1.x, p3.y - trans.y - w}, Point2{p2.x, p3.y + trans.y}, 1, topM);
      fh->addSegment(path0);

      // add vias
      fh->l.fillVias(path1Arr, Point2{p3.x, p3.y - trans.y - w}, Point2{p4.x, p3.y - trans.y}, topM, bottomM);
      fh->addSegment(path1Arr);
      fh->l.fillVias(path1Arr, Point2{p1.x, p1.y + trans.y}, Point2{p2.x, p2.y + trans.y + w}, topM, bottomM);
      fh->addSegment(path1Arr);

      fh->l.translate(path1, trans);
      fh->addSegment(path1);

      // #####################################################################################################
      // two left turns
      // #####################################################################################################

      // first turn
      fh->l.drawSegment(path1, dn, w, 1, geometry, topM);
      fh->l.drawSegment(path2, dn, w, 2, geometry, topM);

      if (i < ns - 1) // one more bridge45x4 is needed
      {
        fh->l.crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
        fh->l.crop(path2Arr, path2, p1cBx4, p2cBx4, topM);
        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }
      else // place for secondary tapped
      {
        // secondary tapping
        // this is only possible if ns>1
        // tapping for ns==1 needs to be done in other place

        if (fasterCap)
        { // fasterCap
          path1Arr.clear();
          path2Arr.clear();
          fh->l.crop(path1Arr, path1, p1c_w, p2c_w, topM);
          fh->l.crop(path2Arr, path2, p1c_w, p2c_w, topM);
          path1 = path1Arr;
          path2 = path2Arr;

          path1Arr.clear();
          path2Arr.clear();
          // ports
          portS1n.x = -0.5 * (dn - w);
          portS1n.y = 0.5 * w;

          portS2n.x = -0.5 * (dn - w);
          portS2n.y = -0.5 * w;
        } // fasterCap

        if (wts > 0)
        {

          p1 = Point2{-sideRightFlag * 0.5 * (dn - 2 * w), 0.5 * wts};
          p2 = Point2{-sideRightFlag * (0.5 * d + wts), -0.5 * wts};

          if (wts > lp)
          {
            p2.x = -sideRightFlag * (0.5 * d + lp);
          }

          std::vector<Segment> tappedPath;

          string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
          fh->l.rectangle(tappedPath, p1, p2, 0, bBottomM);
          fh->addSegment(tappedPath);

          // fill vias

          std::vector<Segment> viaArr;

          fh->l.fillVias(viaArr, p1, Point2{p1.x - sideRightFlag * w, p2.y}, topM, bottomM);
          fh->addSegment(viaArr);

          fh->l.fillVias(viaArr, p1, Point2{p1.x - sideRightFlag * w, p2.y}, bottomM, bBottomM);
          fh->addSegment(viaArr);

        } // if wts

        fh->addSegment(path1);
        fh->addSegment(path2);
      } // else

      // second turn
      fh->l.drawSegment(path1, dn - 2 * (w + s), w, 1, geometry, topM);
      fh->l.drawSegment(path2, dn - 2 * (w + s), w, 2, geometry, topM);

      if (i < ns - 1) // one more bridge45x4 is needed
      {
        fh->l.crop(path1Arr, path1, p1cBx4, p2cBx4, topM);
        fh->l.crop(path2Arr, path2, p1cBx4, p2cBx4, topM);
        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }
      else if (np > ns + 1) // bridge42x2 is needed
      {
        fh->l.crop(path1Arr, path1, p1cBx2, p2cBx2, topM);
        fh->l.crop(path2Arr, path2, p1cBx2, p2cBx2, topM);
        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }
      else // primary tapping
      {

        if (fasterCap)
        { // fasterCap
          path1Arr.clear();
          path2Arr.clear();
          fh->l.crop(path1Arr, path1, p1c_w, p2c_w, topM);
          fh->l.crop(path2Arr, path2, p1c_w, p2c_w, topM);
          path1 = path1Arr;
          path2 = path2Arr;

          path1Arr.clear();
          path2Arr.clear();

          // ports
          portP1n.x = -0.5 * (dn - 2 * (w + s) - w);
          portP1n.y = -0.5 * w;

          portP2n.x = -0.5 * (dn - 2 * (w + s) - w);
          portP2n.y = 0.5 * w;

        } // fasterCap

        // add tapping for primary
        // add tapping for primary
        if (wtp > 0)
        {
          std::vector<Segment> viaArr;
          std::vector<Segment> tappedPath;

          p1.y = 0.5 * wtp;
          p2.y = -0.5 * wtp;
          p2.x = -sideRightFlag * 0.5 * (dn - 2 * w - 2 * (w + s));

          if (wtp > lp)
          {
            p1.x = -sideRightFlag * (0.5 * d + lp);
          }
          else
          {
            p1.x = -sideRightFlag * (0.5 * d + wtp);
          }

          string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
          string bbBottomM = this->l->t.getMetName((this->l->t.getMet(bBottomM)).metNum - 1);
          fh->l.rectangle(tappedPath, p1, p2, 0, bbBottomM);
          fh->addSegment(tappedPath);

          // fill vias

          fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
          fh->addSegment(viaArr);

          fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
          fh->addSegment(viaArr);

          fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bBottomM, bbBottomM);
          fh->addSegment(viaArr);
          // add label
        }

        fh->addSegment(path1);
        fh->addSegment(path2);
      } // else primary tapping
    }

    // iterate
    dn = dn - 2 * 2 * (w + s);
    sideRightFlag = -sideRightFlag;

  } // for

  // bridge45x2

  // ##############################################################################################################################################################################################
  // # loop **************************************************************************************
  // ##############################################################################################################################################################################################

  for (int i = 0; i < np - ns - 1; i++)
  {

    if (sideRightFlag > 0) // right side
    {
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

      // right turn
      fh->l.drawSegment(path0, dn, w, 0, geometry, topM);
      fh->l.drawSegment(path3, dn, w, 3, geometry, topM);

      fh->l.crop(path1Arr, path0, p1cBx2, p2cBx2, topM);
      fh->l.crop(path2Arr, path3, p1cBx2, p2cBx2, topM);

      fh->addSegment(path1Arr);
      fh->addSegment(path2Arr);

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
      // double e=(w+s);
      // //used to cut bridge45x2
      // double b=e+w/(1+sqrt(2))+topMId.minS;

      // bridge 45x2
      p1 = Point2{0.5 * dn - w, 0.5 * (b)};
      p2 = Point2{0.5 * dn, 0.5 * (b)};
      p3 = Point2{0.5 * dn - w + (w + s), -0.5 * (b)};
      p4 = Point2{0.5 * dn + (w + s), -0.5 * (b)};

      fh->l.bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
      fh->addSegment(path1Arr);

      // left turn
      fh->l.drawSegment(path1, dn, w, 1, geometry, topM);
      fh->l.drawSegment(path2, dn, w, 2, geometry, topM);

      if (i < np - ns - 1 - 1) // one more bridge45 is needed
      {

        fh->l.crop(path1Arr, path1, p1cBx2, p2cBx2, topM);
        fh->l.crop(path2Arr, path2, p1cBx2, p2cBx2, topM);

        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }
      else // tapping primary
      {
        if (fasterCap)
        { // fasterCap
          path1Arr.clear();
          path2Arr.clear();
          fh->l.crop(path1Arr, path1, p1c_w, p2c_w, topM);
          fh->l.crop(path2Arr, path2, p1c_w, p2c_w, topM);
          path1 = path1Arr;
          path2 = path2Arr;

          path1Arr.clear();
          path2Arr.clear();

          // ports
          portP1n.x = -0.5 * (dn - w);
          portP1n.y = -0.5 * w;

          portP2n.x = -0.5 * (dn - w);
          portP2n.y = 0.5 * w;

        } // fasterCap

        if (wtp > 0)
        {

          std::vector<Segment> viaArr;
          std::vector<Segment> tappedPath;

          p1.y = 0.5 * wtp;
          p2.y = -0.5 * wtp;

          if (wtp > lp)
          {
            p1.x = -sideRightFlag * (0.5 * d + lp);
          }
          else
          {
            p1.x = -sideRightFlag * (0.5 * d + wtp);
          }

          p2.x = -sideRightFlag * (0.5 * dn - w);

          if (((np - ns) % 2) == 0)
          {
            string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
            fh->l.rectangle(tappedPath, p1, p2, 0, bBottomM);
            fh->addSegment(tappedPath);

            // fill vias

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
            fh->addSegment(viaArr);

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
            fh->addSegment(viaArr);
          }
          else
          {

            string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
            string bbBottomM = this->l->t.getMetName((this->l->t.getMet(bBottomM)).metNum - 1);
            fh->l.rectangle(tappedPath, p1, p2, 0, bbBottomM);
            fh->addSegment(tappedPath);

            // fill vias

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
            fh->addSegment(viaArr);

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
            fh->addSegment(viaArr);

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bBottomM, bbBottomM);
            fh->addSegment(viaArr);
          }

        } // if wtp>0

        fh->addSegment(path1);
        fh->addSegment(path2);

      } // else tapping primary

    } // if
    else // left
    {
      fh->l.drawSegment(path1, dn, w, 1, geometry, topM);
      fh->l.drawSegment(path2, dn, w, 2, geometry, topM);

      fh->l.crop(path1Arr, path1, p1cBx2, p2cBx2, topM);
      fh->l.crop(path2Arr, path2, p1cBx2, p2cBx2, topM);

      fh->addSegment(path1Arr);
      fh->addSegment(path2Arr);

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
      // double e=(w+s);
      // //used to cut bridge45x2
      // double b=e+w/(1+sqrt(2))+topMId.minS;
      // bridge 45x2
      p1 = Point2{-0.5 * dn, 0.5 * (b)};
      p2 = Point2{-0.5 * dn + w, 0.5 * (b)};
      p3 = Point2{-0.5 * dn - (w + s), -0.5 * (b)};
      p4 = Point2{-0.5 * dn - (w + s) + w, -0.5 * (b)};

      fh->l.bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
      fh->addSegment(path1Arr);

      // right turn
      fh->l.drawSegment(path0, dn, w, 0, geometry, topM);
      fh->l.drawSegment(path3, dn, w, 3, geometry, topM);

      if (i < np - ns - 1 - 1) // one more bridge45 is needed
      {

        fh->l.crop(path1Arr, path0, p1cBx2, p2cBx2, topM);
        fh->l.crop(path2Arr, path3, p1cBx2, p2cBx2, topM);

        fh->addSegment(path1Arr);
        fh->addSegment(path2Arr);
      }

      else // tapping primary
      {

        if (fasterCap)
        { // fasterCap
          path1Arr.clear();
          path2Arr.clear();
          fh->l.crop(path1Arr, path0, p1c_w, p2c_w, topM);
          fh->l.crop(path2Arr, path3, p1c_w, p2c_w, topM);
          path0 = path1Arr;
          path3 = path2Arr;

          path1Arr.clear();
          path2Arr.clear();

          // ports
          portP1n.x = 0.5 * (dn - w);
          portP1n.y = 0.5 * w;

          portP2n.x = 0.5 * (dn - w);
          portP2n.y = -0.5 * w;
        } // fasterCap

        if (wtp > 0)
        {

          std::vector<Segment> viaArr;
          std::vector<Segment> tappedPath;

          p1.y = 0.5 * wtp;
          p2.y = -0.5 * wtp;

          if (wtp > lp)
          {
            p1.x = -sideRightFlag * (0.5 * d + lp);
          }
          else
          {
            p1.x = -sideRightFlag * (0.5 * d + wtp);
          }

          p2.x = -sideRightFlag * (0.5 * dn - w);

          if (((np - ns) % 2) == 0)
          {
            string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
            fh->l.rectangle(tappedPath, p1, p2, 0, bBottomM);
            fh->addSegment(tappedPath);
            // fill vias

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
            fh->addSegment(viaArr);

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
            fh->addSegment(viaArr);
          }
          else
          {

            string bBottomM = this->l->t.getMetName((this->l->t.getMet(bottomM)).metNum - 1);
            string bbBottomM = this->l->t.getMetName((this->l->t.getMet(bBottomM)).metNum - 1);
            fh->l.rectangle(tappedPath, p1, p2, 0, bbBottomM);
            fh->addSegment(tappedPath);

            // fill vias

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, topM, bottomM);
            fh->addSegment(viaArr);

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bottomM, bBottomM);
            fh->addSegment(viaArr);

            fh->l.fillVias(viaArr, p2, Point2{p2.x - sideRightFlag * w, p1.y}, bBottomM, bbBottomM);
            fh->addSegment(viaArr);
          }

        } // if wtp>0

        fh->addSegment(path0);
        fh->addSegment(path3);

      } // else tapping primary

    } // else left

    // iterate
    dn = dn - 2 * (w + s);
    sideRightFlag = -sideRightFlag;

  } // for

  // secondary tapping
  if (wts > 0 and ns == 1)
  {

    p1 = Point2{-0.5 * d + w + s, 0.5 * wts};
    if (wts > lp)
    {
      p2 = Point2{-0.5 * d - lp, -0.5 * wts};
    }
    else
    {
      p2 = Point2{-0.5 * d - wts, -0.5 * wts};
    }

    fh->l.rectangle(path0, p1, p2, 0, topM);
    fh->addSegment(path0);
  }

  // primary tapping
  if (wtp > 0 and np <= 2)
  {
    std::vector<Segment> viaArr;
    std::vector<Segment> tappedPath;

    p1.y = 0.5 * wtp;
    p2.y = -0.5 * wtp;

    if (wtp > lp)
    {
      p1.x = (0.5 * d + lp);
    }
    else
    {
      p1.x = (0.5 * d + wtp);
    }

    if (np == 1)
    {
      p2.x = 0.5 * d - w;

      fh->l.rectangle(tappedPath, p1, p2, 0, topM);
      fh->addSegment(tappedPath);
    }
    else if (np == 2)
    {
      p1.x = -p1.x;
      p2.x = -0.5 * d + w + 2 * (w + s);

      fh->l.rectangle(tappedPath, p1, p2, 0, bottomM);
      fh->addSegment(tappedPath);

      // fill vias
      fh->l.fillVias(viaArr, p2, Point2{p2.x - w, p1.y}, topM, bottomM);
      fh->addSegment(viaArr);
    }
  }

  // ###############################################################################################
  // # ports
  // ###############################################################################################

  if (fasterCap)
  {

    fh->addPort(portP1, portP1n, topM, topM);
    fh->addPort(portP2n, portP2, topM, topM);
    fh->addPort(portS1, portS1n, bottomM, topM);
    fh->addPort(portS2n, portS2, topM, bottomM);
  }
  else
  {

    fh->addPort(portP1, portP2, topM, topM);
    fh->addPort(portS1, portS2, bottomM, bottomM);
  }

  return 0;
}

int TransformerSpiral::FastHenryProcess(Command *cm)
{
  // ###############################################################################################
  // # FastHenryTransformer
  // #		: process command line options
  // ###############################################################################################

  ofstream runFastHenryShFile;

  if (cm->geometry == 0) // octagonal
  {
    cm->cellName = "plTrSpirOct";
  }
  else if (cm->geometry == 1) // rectangular
  {
    cm->cellName = "plTrSpirRect";
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
  auto threadFunc = [this, &syncronization_flag, &counter, &runFastHenryShFile, cm](double d, double w, double s, double np, double ns)
  {
    try
    {

      FastHenry *fh = new FastHenry(*(this->fhl));

      // verbose
      string dStr = std::to_string(d);
      dStr = dStr.substr(0, dStr.find(".") + 3);

      string wStr = std::to_string(w);
      wStr = wStr.substr(0, wStr.find(".") + 3);

      string sStr = std::to_string(s);
      sStr = sStr.substr(0, sStr.find(".") + 3);

      string npStr = std::to_string(np);
      npStr = npStr.substr(0, npStr.find(".") + 3);

      string nsStr = std::to_string(ns);
      nsStr = nsStr.substr(0, nsStr.find(".") + 3);

      try
      {
        // check license
        if (not(*(cm->lv)))
        {

          d = std::rand() % 1000;
          w = std::rand() % 20;
        }

        // genCell(Cell &result, double dp, double ds, double wp, double ws, double sp, double lp, double wtp, double wts, int geometry, string topM)
        this->FastHenryGenCell(fh, d, w, s, cm->ps, cm->pl, np, ns, w * cm->tappedPrimaryWidth, w * cm->tappedSecondaryWidth, cm->geometry, cm->topM, false);

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

        tmp = std::to_string(np);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_np" + tmp;

        tmp = std::to_string(ns);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_ns" + tmp;

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
        std::cout << std::setw(75) << "transformer-spiral: d=" + dStr + " w=" + wStr + " s=" + sStr + " np=" + npStr + " ns=" + nsStr;
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

        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(75) << "transformer-spiral: d=" + dStr + " w=" + wStr + " s=" + sStr + " np=" + npStr + " ns=" + nsStr;
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
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->s.size() * cm->np.size() * cm->ns.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
      {
        for (double np = cm->np.min; np <= cm->np.max; np = np + cm->np.step)
        {
          for (double ns = cm->ns.min; ns <= cm->ns.max; ns = ns + cm->ns.step)
          {

            paramArray.push_back({d, w, s, np, ns});

          } // for ns

        } // for np

      } // for s

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

int TransformerSpiral::FastHenry2Process(Command *cm)
{
  // ###############################################################################################
  // # FastHenryTransformer
  // #		: process command line options
  // ###############################################################################################

  ofstream runFastHenryShFile;

  if (cm->geometry == 0) // octagonal
  {
    cm->cellName = "plTrSpirOct";
  }
  else if (cm->geometry == 1) // rectangular
  {
    cm->cellName = "plTrSpirRect";
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
  auto threadFunc = [this, &syncronization_flag, &counter, &runFastHenryShFile, cm](double d, double w, double s, double np, double ns)
  {
    try
    {

      FastHenry *fh = new FastHenry(*(this->fhl));

      // verbose
      string dStr = std::to_string(d);
      dStr = dStr.substr(0, dStr.find(".") + 3);

      string wStr = std::to_string(w);
      wStr = wStr.substr(0, wStr.find(".") + 3);

      string sStr = std::to_string(s);
      sStr = sStr.substr(0, sStr.find(".") + 3);

      string npStr = std::to_string(np);
      npStr = npStr.substr(0, npStr.find(".") + 3);

      string nsStr = std::to_string(ns);
      nsStr = nsStr.substr(0, nsStr.find(".") + 3);

      try
      {
        // check license
        if (not(*(cm->lv)))
        {

          d = std::rand() % 1000;
          w = std::rand() % 20;
        }

        // genCell(Cell &result, double dp, double ds, double wp, double ws, double sp, double lp, double wtp, double wts, int geometry, string topM)
        this->FastHenryGenCell(fh, d, w, s, cm->ps, cm->pl, np, ns, w * cm->tappedPrimaryWidth, w * cm->tappedSecondaryWidth, cm->geometry, cm->topM, true);

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

        tmp = std::to_string(np);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_np" + tmp;

        tmp = std::to_string(ns);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_ns" + tmp;

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
        std::cout << std::setw(75) << "transformer-spiral: d=" + dStr + " w=" + wStr + " s=" + sStr + " np=" + npStr + " ns=" + nsStr;
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

        syncronization_flag.lock();
        std::cout << endl
                  << std::left;
        std::cout << std::setw(75) << "transformer-spiral: d=" + dStr + " w=" + wStr + " s=" + sStr + " np=" + npStr + " ns=" + nsStr;
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
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->s.size() * cm->np.size() * cm->ns.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
      {
        for (double np = cm->np.min; np <= cm->np.max; np = np + cm->np.step)
        {
          for (double ns = cm->ns.min; ns <= cm->ns.max; ns = ns + cm->ns.step)
          {

            paramArray.push_back({d, w, s, np, ns});

          } // for ns

        } // for np

      } // for s

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