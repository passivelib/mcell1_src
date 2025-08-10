#include "mcell_error.h"
#include "gdstk.h"
#include "mcell_transformer2o2.h"
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

int Transformer2o2::threadNumber(Command *cm)
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

int Transformer2o2::drc(double dp, double ds, double wp, double ws, double sp, double ss, double spin, double lpin, double wtp, double wts, int geometry, string topM)
{

  // ###############################################################################################
  // # transformer2o2
  // #    : check if geometry is feasible
  // ###############################################################################################
  // # dp       : outer diameter of primary
  // # ds       : outer diameter of secondary
  // # wp       : primary line width
  // # ws       : secondary line width
  // # sp       : space between turns primary
  // # ss       : space between turns secondary
  // # spin       : space between pins
  // # lpin             : pin length (default should be 2w)
  // # wtp            : width of line for tapped primary if wtp<=0 no tapping
  // # wts            : width of line for tapped secondary if wts<=0 no tapping
  // # geometry   : 0-> octagonal, else rectangular
  // # topM     : top metal layer (metal name)
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

  double ep = (wp + sp);
  ep = 2 * this->l->round2grid(0.5 * ep);

  // cuting for bridge45 primary
  double bp = ep + wp / (1 + sqrt(2)) + minS;
  bp = 2 * this->l->round2grid(0.5 * bp);

  double es = (ws + ss);
  es = 2 * this->l->round2grid(0.5 * es);

  // cuting for bridge45 secondary
  double bs = es + ws / (1 + sqrt(2)) + minS;
  bs = 2 * this->l->round2grid(0.5 * bs);

  // cutting internal primary turn for tapping
  double bpCutForTapped = bs + 2 * ws + 2 * minS;
  bpCutForTapped = 2 * this->l->round2grid(0.5 * bpCutForTapped);

  // cutting internal secondary turn for tapping
  double bsCutForTapped = bp + 2 * wp + 2 * minS;
  bsCutForTapped = 2 * this->l->round2grid(0.5 * bsCutForTapped);

  // internal turn of primary external diameter
  double dp2 = dp - 2 * (wp + sp);
  dp2 = 2 * this->l->round2grid(0.5 * dp2);

  // internal turn of secondary external diameter
  double ds2 = ds - 2 * (ws + ss);
  ds2 = 2 * this->l->round2grid(0.5 * ds2);

  double rshS = 0.5 * dp + minS;
  rshS = 2 * this->l->round2grid(0.5 * rshS);

  bool tmpNotCutS = (rshS < 0.5 * ds2 - ws) or ((0.5 * ds2 + minS) < 0.5 * dp2 - wp and wts <= 0);

  double rshP = 0.5 * ds + minS;
  rshP = 2 * this->l->round2grid(0.5 * rshP);

  bool tmpNotCutP = (rshP < 0.5 * dp2 - wp) or ((0.5 * dp2 + minS) < 0.5 * ds2 - ws and wtp <= 0);

  // bool boolCutSorP=(not tmpNotCutP) or (not tmpNotCutS);

  //**************************************************************************************************
  // Metal shift must be > minW
  //**************************************************************************************************

  if (not tmpNotCutP and rshP - 0.5 * dp2 + wp < topMId.minW)
  {
    rshP = 0.5 * dp2 - wp + topMId.minW;
    rshP = 2 * this->l->round2grid(0.5 * rshP);
  }

  if (not tmpNotCutS and rshS - 0.5 * ds2 + ws < bottomMId.minW)
  {
    rshS = 0.5 * ds2 - ws + bottomMId.minW;
    rshS = 2 * this->l->round2grid(0.5 * rshS);
  }

  // ###############################################################################################
  // # metal width
  // ###############################################################################################

  if (wp < topMId.minW or wp < bottomMId.minW)
  {
    throw SoftError("Warning: metal width < minW!");
  }

  if (ws < topMId.minW or ws < bottomMId.minW)
  {
    throw SoftError("Warning: metal width < minW!");
  }

  // ###############################################################################################
  // # metal space
  // ###############################################################################################

  if (sp < topMId.minS or sp < bottomMId.minS)
  {
    throw SoftError("Warning: metal space < minS!");
  }

  if (ss < topMId.minS or ss < bottomMId.minS)
  {
    throw SoftError("Warning: metal space < minS!");
  }

  // ###############################################################################################
  // # check pins
  // ###############################################################################################

  if (spin < topMId.minS or spin < bottomMId.minS)
  {
    throw SoftError("Warning: pin space < minS!");
  }

  if (lpin < topMId.minW or lpin < bottomMId.minW)
  {
    throw SoftError("Warning: pin length < minW!");
  }

  if ((not tmpNotCutP) and spin < (bpCutForTapped + 2 * wp + 2 * minS))
  {
    throw SoftError("Warning: pin space too short!");
  }

  if ((not tmpNotCutS) and spin < (bsCutForTapped + 2 * ws + 2 * minS))
  {
    throw SoftError("Warning: pin space too short!");
  }

  if (tmpNotCutP and spin < bs + 2 * minS + 2 * ws)
  {
    throw SoftError("Warning: pin space too short!");
  }

  if (tmpNotCutS and spin < bp + 2 * minS + 2 * wp)
  {
    throw SoftError("Warning: pin space too short!");
  }

  if (wtp > 0 and spin < wtp + 2 * minS)
  {
    throw SoftError("Warning: pin space too short!");
  }

  if (wts > 0 and spin < wts + 2 * minS)
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

  if (geometry != 0 and (dp - 2 * wp < (spin + 2 * wp) or ds - 2 * ws < (spin + 2 * ws))) // rect
  {
    throw SoftError("Warning: pin space too wide!");
  }

  if (geometry == 0 and (2 * kp < (spin + 2 * wp) or 2 * ks < (spin + 2 * ws))) // oct
  {
    throw SoftError("Warning: pin space too wide!");
  }

  // ###############################################################################################
  // # check bridge45x2
  // ###############################################################################################

  double kp2 = this->l->k(dp2 - 2 * wp);
  double ks2 = this->l->k(ds2 - 2 * ws);

  if (geometry == 0) // oct
  {
    if (2 * kp2 < bp + 2 * wp or 2 * ks2 < bs + 2 * ws)
    {
      throw SoftError("Warning: no enough space for bridge45x2!");
    }
  }

  if (geometry != 0) // rect
  {
    if (dp2 - 2 * wp < bp + 2 * wp or ds2 - 2 * ws < bs + 2 * ws)
    {
      throw SoftError("Warning: no enough space for bridge45x2!");
    }
  }

  // ###############################################################################################
  // # tapping
  // ###############################################################################################

  if (wtp < topMId.minW and wtp > 0)
  {
    throw SoftError("Warning: wtp<minw");
  }

  if (wts < bottomMId.minW and wts > 0)
  {
    throw SoftError("Warning: wts<minw");
  }

  if (not tmpNotCutP)
  {

    if (geometry == 0 and 2 * kp2 < bpCutForTapped) // oct
    {
      throw SoftError("Warning: primary diameter too short!");
    }

    if (geometry != 0 and dp2 < bpCutForTapped)
    {
      throw SoftError("Warning: primary diameter too short!");
    }
  }

  if (not tmpNotCutS)
  {
    if (geometry == 0 and 2 * ks2 < bsCutForTapped) // oct
    {
      throw SoftError("Warning: secondary diameter too short!");
    }

    if (geometry != 0 and ds2 < bsCutForTapped)
    {
      throw SoftError("Warning: secondary diameter too short!");
    }
  }

  return 0;
}

int Transformer2o2::genCell(Cell &result, double dp, double ds, double wp, double ws, double sp, double ss, double spin, double lpin, double wtp, double wts, int geometry, string topM, bool fasterCap)
{
  // ###############################################################################################
  // # transformer2o2
  // #    : generate cell
  // ###############################################################################################
  // # dp         : outer diameter of primary
  // # ds         : outer diameter of secondary
  // # wp         : primary line width
  // # ws         : secondary line width
  // # sp         : space between turns primary
  // # ss         : space between turns secondary
  // # spin       : space between pins
  // # lpin       : pin length (default should be 2w)
  // # wtp        : width of line for tapped primary if wtp<=0 no tapping
  // # wts        : width of line for tapped secondary if wts<=0 no tapping
  // # geometry   : 0-> octagonal, else rectangular
  // # topM       : top metal layer (metal name)
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

  wp = 2 * this->l->round2grid(0.5 * wp);
  ws = 2 * this->l->round2grid(0.5 * ws);
  sp = 2 * this->l->round2grid(0.5 * sp);
  ss = 2 * this->l->round2grid(0.5 * ss);
  dp = 2 * this->l->round2grid(0.5 * dp);
  ds = 2 * this->l->round2grid(0.5 * ds);

  double ep = (wp + sp);
  ep = 2 * this->l->round2grid(0.5 * ep);

  // cutting for bridge45 primary
  double bp = ep + wp / (1 + sqrt(2)) + minS;
  bp = 2 * this->l->round2grid(0.5 * bp);

  double es = (ws + ss);
  es = 2 * this->l->round2grid(0.5 * es);

  // cutting for bridge45 secondary
  double bs = es + ws / (1 + sqrt(2)) + minS;
  bs = 2 * this->l->round2grid(0.5 * bs);

  // cutting internal primary turn for tapping
  double bpCutForTapped = bs + 2 * ws + 2 * minS;
  bpCutForTapped = 2 * this->l->round2grid(0.5 * bpCutForTapped);

  // cutting internal secondary turn for tapping
  double bsCutForTapped = bp + 2 * wp + 2 * minS;
  bsCutForTapped = 2 * this->l->round2grid(0.5 * bsCutForTapped);

  double dmax;

  if (dp > ds)
  {
    dmax = dp;
  }
  else
  {
    dmax = ds;
  }
  dmax = 2 * this->l->round2grid(0.5 * dmax);

  // internal turn of primary external diameter
  double dp2 = dp - 2 * (wp + sp);
  dp2 = 2 * this->l->round2grid(0.5 * dp2);

  // internal turn of secondary external diameter
  double ds2 = ds - 2 * (ws + ss);
  ds2 = 2 * this->l->round2grid(0.5 * ds2);

  double rshS = 0.5 * dp + minS;
  rshS = 2 * this->l->round2grid(0.5 * rshS);

  bool tmpNotCutS = (rshS < 0.5 * ds2 - ws) or ((0.5 * ds2 + minS) < 0.5 * dp2 - wp and wts <= 0);

  double rshP = 0.5 * ds + minS;
  rshP = 2 * this->l->round2grid(0.5 * rshP);

  bool tmpNotCutP = (rshP < 0.5 * dp2 - wp) or ((0.5 * dp2 + minS) < 0.5 * ds2 - ws and wtp <= 0);

  //**************************************************************************************************
  // Metal shift must be > minW
  //**************************************************************************************************

  if (not tmpNotCutP and rshP - 0.5 * dp2 + wp < topMId.minW)
  {
    rshP = 0.5 * dp2 - wp + topMId.minW;
    rshP = 2 * this->l->round2grid(0.5 * rshP);
  }

  if (not tmpNotCutS and rshS - 0.5 * ds2 + ws < bottomMId.minW)
  {
    rshS = 0.5 * ds2 - ws + bottomMId.minW;
    rshS = 2 * this->l->round2grid(0.5 * rshS);
  }

  // bool boolCutSorP=(not tmpNotCutP) or (not tmpNotCutS);

  // space between pins
  if (spin <= 0)
  {
    double sppin = not tmpNotCutP ? bpCutForTapped + 2 * wp + 2 * minS : bs + 2 * minS + 2 * ws;

    if (wtp > 0)
    {
      sppin = sppin < wtp + 2 * minS ? wtp + 2 * minS : sppin;
    }

    double sspin = not tmpNotCutS ? bsCutForTapped + 2 * ws + 2 * minS : bp + 2 * minS + 2 * wp;

    if (wts > 0)
    {
      sspin = sspin < wts + 2 * minS ? wts + 2 * minS : sspin;
    }

    spin = sppin > sspin ? sppin : sspin;
  }

  // ###############################################################################################
  // # pin length
  // ###############################################################################################

  // lp is not distance from turns but from tapped part on left and right side between pins
  if (lpin <= 0)
    lpin = wp > ws ? 2 * wp : 2 * ws;

  // cutP or cutS is present
  if ((not tmpNotCutP) or (not tmpNotCutS))
  {

    if ((not tmpNotCutP) and (not tmpNotCutS))
    {
      lpin = rshP + wp > rshS + ws ? lpin + wp + minS : lpin + ws + minS;
    }
    else if ((not tmpNotCutP) and (0.5 * dmax < rshP))
    {
      lpin = lpin + rshP - 0.5 * dmax + wp;
    }
    else if ((not tmpNotCutS) and (0.5 * dmax < rshS))
    {
      lpin = lpin + rshS - 0.5 * dmax + ws;
    }
  }

  lpin = 2 * this->l->round2grid(0.5 * lpin);

  // ###############################################################################################
  // # round to grid
  // ###############################################################################################

  spin = 2 * this->l->round2grid(0.5 * spin);

  // wtp and wts can be <0 and than tapping is not done
  wtp = 2 * this->l->round2grid(0.5 * wtp);
  wts = 2 * this->l->round2grid(0.5 * wts);

  // ###############################################################################################
  // # geometry check
  // ###############################################################################################

  this->drc(dp, ds, wp, ws, sp, ss, spin, lpin, wtp, wts, geometry, topM);

  // cut for bridge45 primary
  Vec2 p1cBp = Vec2{-0.5 * dmax - wp, 0.5 * (bp)};
  Vec2 p2cBp = Vec2{0.5 * dmax + wp, -0.5 * (bp)};

  // cut for bridge45 secondary
  Vec2 p1cBs = Vec2{-0.5 * dmax - ws, 0.5 * (bs)};
  Vec2 p2cBs = Vec2{0.5 * dmax + ws, -0.5 * (bs)};

  // cut for internal primary turns
  Vec2 p1cTp = Vec2{-0.5 * dmax - wp, 0.5 * (bpCutForTapped)};
  Vec2 p2cTp = Vec2{0.5 * dmax + wp, -0.5 * (bpCutForTapped)};

  // cut for internal secondary turns
  Vec2 p1cTs = Vec2{-0.5 * dmax - ws, 0.5 * (bsCutForTapped)};
  Vec2 p2cTs = Vec2{0.5 * dmax + ws, -0.5 * (bsCutForTapped)};

  // cut for pins
  Vec2 p1cP = Vec2{-0.5 * dmax - wp, 0.5 * (spin)};
  Vec2 p2cP = Vec2{0.5 * dmax + wp, -0.5 * (spin)};

  // cut for faster cup
  Vec2 p1c_wp = Vec2{-dmax - wp - lpin, -0.5 * wp};
  Vec2 p2c_wp = Vec2{dmax + wp + lpin, 0.5 * wp};

  Vec2 p1c_ws = Vec2{-dmax - ws - lpin, -0.5 * ws};
  Vec2 p2c_ws = Vec2{dmax + ws + lpin, 0.5 * ws};

  Polygon *path0, *path1, *path2, *path3;

  Array<Polygon *> path1Arr = {0};
  Array<Polygon *> path2Arr = {0};

  // ###############################################################################################
  // # primary
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

  // ###############################################################################################
  // # external turn primary
  // ###############################################################################################

  this->l->drawSegment(path0, dp, wp, 0, geometry, topM);
  this->l->drawSegment(path1, dp, wp, 1, geometry, topM);
  this->l->drawSegment(path2, dp, wp, 2, geometry, topM);
  this->l->drawSegment(path3, dp, wp, 3, geometry, topM);

  this->l->crop(path1Arr, path1, p1cP, p2cP, topM);
  this->l->crop(path2Arr, path2, p1cP, p2cP, topM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  this->l->crop(path1Arr, path0, p1cBp, p2cBp, topM);
  this->l->crop(path2Arr, path3, p1cBp, p2cBp, topM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # primary contacts
  // ###############################################################################################

  Vec2 p1 = Vec2{-0.5 * dmax - lpin, wp + 0.5 * spin};
  Vec2 p2 = Vec2{-0.5 * dp + wp, 0.5 * spin};

  this->l->rectangle(path1, p1, p2, topM);
  result.polygon_array.append(path1);

  p1 = Vec2{-0.5 * dmax - lpin, -wp - 0.5 * spin};
  p2 = Vec2{-0.5 * dp + wp, -0.5 * spin};

  this->l->rectangle(path1, p1, p2, topM);
  result.polygon_array.append(path1);

  // ###############################################################################################
  // # primary label
  // ###############################################################################################
  Label *label;

  p1 = Vec2{-0.5 * dmax - lpin, 0.5 * (spin + wp)};
  p2 = Vec2{-0.5 * dmax - lpin, -0.5 * (spin + wp)};

  this->l->label(label, "P1", p1, topM);
  result.label_array.append(label);

  this->l->label(label, "P2", p2, topM);
  result.label_array.append(label);

  // ###############################################################################################
  // # internal turn primary
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

  this->l->drawSegment(path0, dp2, wp, 0, geometry, topM);
  this->l->drawSegment(path1, dp2, wp, 1, geometry, topM);
  this->l->drawSegment(path2, dp2, wp, 2, geometry, topM);
  this->l->drawSegment(path3, dp2, wp, 3, geometry, topM);

  // this->l->crop(path1Arr, path1, p1cT, p2cT, topM);
  // this->l->crop(path2Arr, path2, p1cT, p2cT, topM);

  this->l->crop(path1Arr, path0, p1cBp, p2cBp, topM);
  this->l->crop(path2Arr, path3, p1cBp, p2cBp, topM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # bridge45 right primary
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
  p1 = Vec2{0.5 * dp - wp, 0.5 * (bp)};
  p2 = Vec2{0.5 * dp, 0.5 * (bp)};
  Vec2 p3 = Vec2{0.5 * dp - wp - (wp + sp), -0.5 * (bp)};
  Vec2 p4 = Vec2{0.5 * dp - (wp + sp), -0.5 * (bp)};

  this->l->bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
  this->l->appendArrToCell(result, path1Arr);

  // ###############################################################################################
  // # primary for tapping
  // ###############################################################################################

  if (not tmpNotCutP)
  {

    this->l->crop(path1Arr, path1, p1cTp, p2cTp, topM);
    this->l->crop(path2Arr, path2, p1cTp, p2cTp, topM);

    result.polygon_array.append(path1Arr[0]);
    result.polygon_array.append(path2Arr[0]);

    path1Arr.clear();
    path2Arr.clear();

    path1Arr = {0};
    path2Arr = {0};

    p1 = Vec2{-0.5 * dp2 + wp, 0.5 * (bpCutForTapped)};
    p2 = Vec2{-rshP, 0.5 * (bpCutForTapped) + wp};

    this->l->rectangle(path1, p1, p2, topM);
    result.polygon_array.append(path1);

    p1 = Vec2{-0.5 * dp2 + wp, -0.5 * (bpCutForTapped)};
    p2 = Vec2{-rshP, -0.5 * (bpCutForTapped)-wp};

    this->l->rectangle(path1, p1, p2, topM);
    result.polygon_array.append(path1);

    p1 = Vec2{-rshP, 0.5 * (bpCutForTapped) + wp};
    p2 = Vec2{-rshP - wp, -0.5 * (bpCutForTapped)-wp};

    this->l->rectangle(path1, p1, p2, topM);

    // fasterCap to cut
    // fasterCap
    if (fasterCap)
    {
      this->l->crop(path1Arr, path1, p1c_wp, p2c_wp, topM);

      result.polygon_array.append(path1Arr[0]);
      result.polygon_array.append(path1Arr[1]);

      path1Arr.clear();

      path1Arr = {0};
    }
    else
    {
      result.polygon_array.append(path1);
    }
  }
  else
  {
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
  }

  // ###############################################################################################
  // # secondary
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

  // ###############################################################################################
  // # external turn secondary
  // ###############################################################################################

  this->l->drawSegment(path0, ds, ws, 0, geometry, bottomM);
  this->l->drawSegment(path1, ds, ws, 1, geometry, bottomM);
  this->l->drawSegment(path2, ds, ws, 2, geometry, bottomM);
  this->l->drawSegment(path3, ds, ws, 3, geometry, bottomM);

  this->l->crop(path1Arr, path0, p1cP, p2cP, bottomM);
  this->l->crop(path2Arr, path3, p1cP, p2cP, bottomM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  this->l->crop(path1Arr, path1, p1cBs, p2cBs, bottomM);
  this->l->crop(path2Arr, path2, p1cBs, p2cBs, bottomM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # secondary contacts
  // ###############################################################################################

  p1 = Vec2{0.5 * dmax + lpin, ws + 0.5 * spin};
  p2 = Vec2{0.5 * ds - ws, 0.5 * spin};

  this->l->rectangle(path1, p1, p2, bottomM);
  result.polygon_array.append(path1);

  p1 = Vec2{0.5 * dmax + lpin, -ws - 0.5 * spin};
  p2 = Vec2{0.5 * ds - ws, -0.5 * spin};

  this->l->rectangle(path1, p1, p2, bottomM);
  result.polygon_array.append(path1);

  // ###############################################################################################
  // # secondary label
  // ###############################################################################################

  p1 = Vec2{0.5 * dmax + lpin, 0.5 * (spin + ws)};
  p2 = Vec2{0.5 * dmax + lpin, -0.5 * (spin + ws)};

  this->l->label(label, "S1", p1, bottomM);
  result.label_array.append(label);

  this->l->label(label, "S2", p2, bottomM);
  result.label_array.append(label);

  // ###############################################################################################
  // # internal turn secondary
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

  this->l->drawSegment(path0, ds2, ws, 0, geometry, bottomM);
  this->l->drawSegment(path1, ds2, ws, 1, geometry, bottomM);
  this->l->drawSegment(path2, ds2, ws, 2, geometry, bottomM);
  this->l->drawSegment(path3, ds2, ws, 3, geometry, bottomM);

  this->l->crop(path1Arr, path1, p1cBs, p2cBs, bottomM);
  this->l->crop(path2Arr, path2, p1cBs, p2cBs, bottomM);

  result.polygon_array.append(path1Arr[0]);
  result.polygon_array.append(path2Arr[0]);

  path1Arr.clear();
  path2Arr.clear();

  path1Arr = {0};
  path2Arr = {0};

  // ###############################################################################################
  // # secondary for tapping
  // ###############################################################################################

  if (not tmpNotCutS)
  {

    this->l->crop(path1Arr, path0, p1cTs, p2cTs, bottomM);
    this->l->crop(path2Arr, path3, p1cTs, p2cTs, bottomM);

    result.polygon_array.append(path1Arr[0]);
    result.polygon_array.append(path2Arr[0]);

    path1Arr.clear();
    path2Arr.clear();

    path1Arr = {0};
    path2Arr = {0};

    p1 = Vec2{0.5 * ds2 - ws, 0.5 * (bsCutForTapped)};
    p2 = Vec2{rshS, 0.5 * (bsCutForTapped) + ws};

    this->l->rectangle(path1, p1, p2, bottomM);
    result.polygon_array.append(path1);

    p1 = Vec2{0.5 * ds2 - ws, -0.5 * (bsCutForTapped)};
    p2 = Vec2{rshS, -0.5 * (bsCutForTapped)-ws};

    this->l->rectangle(path1, p1, p2, bottomM);
    result.polygon_array.append(path1);

    p1 = Vec2{rshS, 0.5 * (bsCutForTapped) + ws};
    p2 = Vec2{rshS + ws, -0.5 * (bsCutForTapped)-ws};

    this->l->rectangle(path1, p1, p2, bottomM);

    // fasterCap

    if (fasterCap)
    {
      this->l->crop(path1Arr, path1, p1c_ws, p2c_ws, bottomM);

      result.polygon_array.append(path1Arr[0]);
      result.polygon_array.append(path1Arr[1]);

      path1Arr.clear();

      path1Arr = {0};
    }
    else
    {
      result.polygon_array.append(path1);
    }
  }
  else // no cutting
  {

    // fasterCap
    if (fasterCap)
    {
      this->l->crop(path1Arr, path0, p1c_wp, p2c_ws, bottomM);
      this->l->crop(path2Arr, path3, p1c_wp, p2c_ws, bottomM);

      result.polygon_array.append(path1Arr[0]);
      result.polygon_array.append(path2Arr[0]);

      path1Arr.clear();
      path2Arr.clear();

      path1Arr = {0};
      path2Arr = {0};
    }
    else
    {
      result.polygon_array.append(path0);
      result.polygon_array.append(path3);
    }
  }

  // ###############################################################################################
  // # bridge45 left secondary
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
  p1 = Vec2{-0.5 * ds, 0.5 * (bs)};
  p2 = Vec2{-0.5 * ds + ws, 0.5 * (bs)};
  p3 = Vec2{-0.5 * ds + (ws + ss), -0.5 * (bs)};
  p4 = Vec2{-0.5 * ds + (ws + ss) + ws, -0.5 * (bs)};

  this->l->bridge45x2(path0, path1, p1, p2, p3, p4, topM, bottomM);
  result.polygon_array.append(path0);
  result.polygon_array.append(path1);

  // add vias
  this->l->fillVias(path1Arr, p1, Vec2{p2.x, p2.y + ws}, topM, bottomM);
  this->l->appendArrToCell(result, path1Arr);
  this->l->fillVias(path1Arr, p3, Vec2{p4.x, p4.y - ws}, topM, bottomM);
  this->l->appendArrToCell(result, path1Arr);

  // ###############################################################################################
  // # primary tapped
  // ###############################################################################################

  if (wtp > 0)
  {

    double lmin = lpin;

    if (0.5 * dp2 - wp > rshP)
    {
      rshP = 0.5 * dp2;
    }

    p1 = Vec2{-0.5 * dmax - lmin, 0.5 * wtp};
    p2 = Vec2{-rshP, -0.5 * wtp};

    this->l->rectangle(path1, p1, p2, topM);
    result.polygon_array.append(path1);

    // ###############################################################################################
    // # label
    // ###############################################################################################
    p1 = Vec2{-0.5 * dmax - lmin, 0};

    this->l->label(label, "TP", p1, topM);
    result.label_array.append(label);
  }

  // ###############################################################################################
  // # secondary tapped
  // ###############################################################################################

  if (wts > 0)
  {

    double lmin = lpin;

    if (0.5 * ds2 - ws > rshS)
    {
      rshS = 0.5 * ds2;
    }

    p1 = Vec2{0.5 * dmax + lmin, 0.5 * wts};
    p2 = Vec2{rshS, -0.5 * wts};

    this->l->rectangle(path1, p1, p2, bottomM);
    result.polygon_array.append(path1);

    // ###############################################################################################
    // # label
    // ###############################################################################################
    p1 = Vec2{0.5 * dmax + lmin, 0};

    this->l->label(label, "TS", p1, bottomM);
    result.label_array.append(label);
  }

  return 0;
}

void Transformer2o2::printHelp()
{
  // ###############################################################################################
  // # Transformer2o2
  // #    : print command line options
  // #      this function is static so it can be called without any object Transformer2o2::printHelp
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
  std::cout << "       Set space between primary turns of transformer. Short option is -s." << endl;
  std::cout << "       This option is optional, default is ss=s." << endl;
  std::cout << "       --space=5 or -s 5" << endl;
  std::cout << "--space=smin:smax:sstep <-s>:" << endl;
  std::cout << "       Space between primary turns of transformer is swept from smin to smax with step sstep." << endl;
  std::cout << "       Short option is -s." << endl;
  std::cout << "       --space=2:7:1 or -s 2:7:1" << endl;
  std::cout << "********************************************************************************************" << endl;
  std::cout << "--space-secondary=number <--ss>:" << endl;
  std::cout << "       Set space between secondary turns of transformer. Short option is --ss." << endl;
  std::cout << "       This option is mandatory." << endl;
  std::cout << "       --space-secondary=5 or --ss=5" << endl;
  std::cout << "--space-secondary=smin:smax:sstep <--ss>:" << endl;
  std::cout << "       Space between secondary turns of transformer is swept from smin to smax with step sstep." << endl;
  std::cout << "       Short option is --ss." << endl;
  std::cout << "       --space-secondary=2:7:1 or --ss=2:7:1" << endl;
  std::cout << "********************************************************************************************" << endl;

  // exit(1);
}

int Transformer2o2::process(Command *cm)
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
      cm->cellName = "plTr2o2Oct";
    }
    else if (cm->geometry == 1) // rectangular
    {
      cm->cellName = "plTr2o2Rect";
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
    maestroFile << "description \"transformer2o2\"" << endl;

    // type
    maestroFile << strType << endl;

    maestroFile << "parameter outlength=d scaling 1e-6 rounding 0.1" << endl;
    maestroFile << "parameter metalw=w scaling 1e-6 rounding 0.1" << endl;
    maestroFile << "parameter metalws scaling 1e-6" << endl;
    maestroFile << "parameter space scaling 1e-6" << endl;
    maestroFile << "parameter sspace scaling 1e-6" << endl;
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
    // throw HardError("Error: Command line error. Cell name is not specified!");

    if (cm->gdsFile == "")
    {
      if (cm->geometry == 0) // octagonal
      {
        cm->cellName = "plTr2o2Oct";
      }
      else if (cm->geometry == 1) // rectangular
      {
        cm->cellName = "plTr2o2Rect";
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

  if (cm->w.max == cm->w.min and cm->d.max == cm->d.min and cm->s.max == cm->s.min and cm->ss.max == cm->ss.min and cm->sh.max == cm->sh.min and cm->ws.max == cm->ws.min)
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
  auto threadFunc = [this, &syncronization_flag, &counter, &runEmxShFile, &maestroFile, cm, sweepFlag](double d, double w, double ws, double s, double ss, double sh)
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

        ws = ws <= 0 ? w : ws; // if ws not set than equal to wp

        ss = ss <= 0 ? s : ss; // if ss not set than equal to s

        double ds = d - 2 * w - s + ss + 2 * ws - 2 * sh;
        double dmax = (d > ds ? d : ds); // if d>ds then d else ds

        // verbose
        string dStr = std::to_string(d);
        dStr = dStr.substr(0, dStr.find(".") + 3);

        string wStr = std::to_string(w);
        wStr = wStr.substr(0, wStr.find(".") + 3);

        string wsStr = std::to_string(ws);
        wsStr = wsStr.substr(0, wsStr.find(".") + 3);

        string ssStr = std::to_string(ss);
        ssStr = ssStr.substr(0, ssStr.find(".") + 3);

        string sStr = std::to_string(s);
        sStr = sStr.substr(0, sStr.find(".") + 3);

        string shStr = std::to_string(sh);
        shStr = shStr.substr(0, shStr.find(".") + 3);

        if (cm->tappedPrimaryWidth < -2)
        {
          cm->tappedPrimaryWidth = 2;
        }

        if (cm->tappedSecondaryWidth < -2)
        {
          cm->tappedSecondaryWidth = 2;
        }

        this->genCell(cell, d, ds, w, ws, s, ss, cm->ps, cm->pl, w * cm->tappedPrimaryWidth, ws * cm->tappedSecondaryWidth, cm->geometry, cm->topM, false);

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

          tmp = std::to_string(ss);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_sspace" + tmp;

          tmp = std::to_string(sh);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_sshift" + tmp;
          //******************************************************************************************
          // mutex protected: START
          //******************************************************************************************
          syncronization_flag.lock();
          std::cout << endl
                    << std::left;
          std::cout << std::setw(80) << "transformer2o2: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " ss=" + ssStr + " sh=" + shStr;
          counter++;
          std::cout << counter;

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

          tmp = std::to_string(ss);
          tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
          gdsFileName = gdsFileName + "_ss" + tmp;

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
          std::cout << std::setw(80) << "transformer2o2: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " ss=" + ssStr + " sh=" + shStr;
          counter++;
          std::cout << counter;

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

          string ssStr = std::to_string(ss);
          ssStr = ssStr.substr(0, ssStr.find(".") + 3);

          string sStr = std::to_string(s);
          sStr = sStr.substr(0, sStr.find(".") + 3);

          string shStr = std::to_string(sh);
          shStr = shStr.substr(0, shStr.find(".") + 3);

          syncronization_flag.lock();
          std::cout << endl
                    << std::left;
          std::cout << std::setw(80) << "transformer2o2: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " ss=" + ssStr + " sh=" + shStr;
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

  std::vector<std::array<double, 6>> paramArray;
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->ws.size() * cm->s.size() * cm->ss.size() * cm->sh.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double ws = cm->ws.min; ws <= cm->ws.max; ws = ws + cm->ws.step)
      {
        for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
        {

          for (double ss = cm->ss.min; ss <= cm->ss.max; ss = ss + cm->ss.step)
          {
            for (double sh = cm->sh.min; sh <= cm->sh.max; sh = sh + cm->sh.step)
            {

              paramArray.push_back({d, w, ws, s, ss, sh});

            } // for sh

          } // for ss

        } // for s

      } // for ws

    } // for w

  } // for d

  auto loopFunction = [this, &paramArray, &threadFunc](const uint64_t a, const uint64_t b)
  {
    for (size_t k = a; k < b; k++)
    {
      const std::array<double, 6> param = paramArray[k];
      threadFunc(param[0], param[1], param[2], param[3], param[4], param[5]);
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

int Transformer2o2::FastHenryGenCell(FastHenry *fh, double dp, double ds, double wp, double ws, double sp, double ss, double spin, double lpin, double wtp, double wts, int geometry, string topM, bool fasterCap)
{
  // ###############################################################################################
  // # transformer2o2
  // #    : generate cell
  // ###############################################################################################
  // # dp       : outer diameter of primary
  // # ds       : outer diameter of secondary
  // # wp       : primary line width
  // # ws       : secondary line width
  // # sp       : space between turns primary
  // # ss       : space between turns secondary
  // # spin       : space between pins
  // # lpin             : pin length (default should be 2w)
  // # wtp            : width of line for tapped primary if wtp<=0 no tapping
  // # wts            : width of line for tapped secondary if wts<=0 no tapping
  // # geometry   : 0-> octagonal, else rectangular
  // # topM     : top metal layer (metal name)
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

  wp = 2 * this->l->round2grid(0.5 * wp);
  ws = 2 * this->l->round2grid(0.5 * ws);
  sp = 2 * this->l->round2grid(0.5 * sp);
  ss = 2 * this->l->round2grid(0.5 * ss);
  dp = 2 * this->l->round2grid(0.5 * dp);
  ds = 2 * this->l->round2grid(0.5 * ds);

  double ep = (wp + sp);
  ep = 2 * this->l->round2grid(0.5 * ep);

  // cutting for bridge45 primary
  double bp = ep + wp / (1 + sqrt(2)) + minS;
  bp = 2 * this->l->round2grid(0.5 * bp);

  double es = (ws + ss);
  es = 2 * this->l->round2grid(0.5 * es);

  // cutting for bridge45 secondary
  double bs = es + ws / (1 + sqrt(2)) + minS;
  bs = 2 * this->l->round2grid(0.5 * bs);

  // cutting internal primary turn for tapping
  double bpCutForTapped = bs + 2 * ws + 2 * minS;
  bpCutForTapped = 2 * this->l->round2grid(0.5 * bpCutForTapped);

  // cutting internal secondary turn for tapping
  double bsCutForTapped = bp + 2 * wp + 2 * minS;
  bsCutForTapped = 2 * this->l->round2grid(0.5 * bsCutForTapped);

  double dmax;

  if (dp > ds)
  {
    dmax = dp;
  }
  else
  {
    dmax = ds;
  }
  dmax = 2 * this->l->round2grid(0.5 * dmax);

  // internal turn of primary external diameter
  double dp2 = dp - 2 * (wp + sp);
  dp2 = 2 * this->l->round2grid(0.5 * dp2);

  // internal turn of secondary external diameter
  double ds2 = ds - 2 * (ws + ss);
  ds2 = 2 * this->l->round2grid(0.5 * ds2);

  double rshS = 0.5 * dp + minS;
  rshS = 2 * this->l->round2grid(0.5 * rshS);

  bool tmpNotCutS = (rshS < 0.5 * ds2 - ws) or ((0.5 * ds2 + minS) < 0.5 * dp2 - wp and wts <= 0);

  double rshP = 0.5 * ds + minS;
  rshP = 2 * this->l->round2grid(0.5 * rshP);

  bool tmpNotCutP = (rshP < 0.5 * dp2 - wp) or ((0.5 * dp2 + minS) < 0.5 * ds2 - ws and wtp <= 0);

  //**************************************************************************************************
  // Metal shift must be > minW
  //**************************************************************************************************

  if (not tmpNotCutP and rshP - 0.5 * dp2 + wp < topMId.minW)
  {
    rshP = 0.5 * dp2 - wp + topMId.minW;
    rshP = 2 * this->l->round2grid(0.5 * rshP);
  }

  if (not tmpNotCutS and rshS - 0.5 * ds2 + ws < bottomMId.minW)
  {
    rshS = 0.5 * ds2 - ws + bottomMId.minW;
    rshS = 2 * this->l->round2grid(0.5 * rshS);
  }

  // bool boolCutSorP=(not tmpNotCutP) or (not tmpNotCutS);

  // space between pins
  if (spin <= 0)
  {
    double sppin = not tmpNotCutP ? bpCutForTapped + 2 * wp + 2 * minS : bs + 2 * minS + 2 * ws;

    if (wtp > 0)
    {
      sppin = sppin < wtp + 2 * minS ? wtp + 2 * minS : sppin;
    }

    double sspin = not tmpNotCutS ? bsCutForTapped + 2 * ws + 2 * minS : bp + 2 * minS + 2 * wp;

    if (wts > 0)
    {
      sspin = sspin < wts + 2 * minS ? wts + 2 * minS : sspin;
    }

    spin = sppin > sspin ? sppin : sspin;
  }

  // ###############################################################################################
  // # pin length
  // ###############################################################################################

  // lp is not distance from turns but from tapped part on left and right side between pins
  if (lpin <= 0)
    lpin = wp > ws ? 2 * wp : 2 * ws;

  // cutP or cutS is present
  if ((not tmpNotCutP) or (not tmpNotCutS))
  {

    if ((not tmpNotCutP) and (not tmpNotCutS))
    {
      lpin = rshP + wp > rshS + ws ? lpin + wp + minS : lpin + ws + minS;
    }
    else if ((not tmpNotCutP) and (0.5 * dmax < rshP))
    {
      lpin = lpin + rshP - 0.5 * dmax + wp;
    }
    else if ((not tmpNotCutS) and (0.5 * dmax < rshS))
    {
      lpin = lpin + rshS - 0.5 * dmax + ws;
    }
  }

  lpin = 2 * this->l->round2grid(0.5 * lpin);

  // ###############################################################################################
  // # round to grid
  // ###############################################################################################

  spin = 2 * this->l->round2grid(0.5 * spin);

  // wtp and wts can be <0 and than tapping is not done
  wtp = 2 * this->l->round2grid(0.5 * wtp);
  wts = 2 * this->l->round2grid(0.5 * wts);

  // ###############################################################################################
  // # geometry check
  // ###############################################################################################

  this->drc(dp, ds, wp, ws, sp, ss, spin, lpin, wtp, wts, geometry, topM);

  // cut for bridge45 primary
  Point2 p1cBp = Point2(-0.5 * dmax - wp, 0.5 * (bp));
  Point2 p2cBp = Point2(0.5 * dmax + wp, -0.5 * (bp));

  // cut for bridge45 secondary
  Point2 p1cBs = Point2(-0.5 * dmax - ws, 0.5 * (bs));
  Point2 p2cBs = Point2(0.5 * dmax + ws, -0.5 * (bs));

  // cut for internal primary turns
  Point2 p1cTp = Point2(-0.5 * dmax - wp, 0.5 * (bpCutForTapped));
  Point2 p2cTp = Point2(0.5 * dmax + wp, -0.5 * (bpCutForTapped));

  // cut for internal secondary turns
  Point2 p1cTs = Point2(-0.5 * dmax - ws, 0.5 * (bsCutForTapped));
  Point2 p2cTs = Point2(0.5 * dmax + ws, -0.5 * (bsCutForTapped));

  // cut for pins
  Point2 p1cP = Point2(-0.5 * dmax - wp, 0.5 * (spin));
  Point2 p2cP = Point2(0.5 * dmax + wp, -0.5 * (spin));

  // cut for fasterCap cup
  Point2 p1c_wp = Point2{-dmax - wp - lpin, -0.5 * wp};
  Point2 p2c_wp = Point2{dmax + wp + lpin, 0.5 * wp};

  Point2 p1c_ws = Point2{-dmax - ws - lpin, -0.5 * ws};
  Point2 p2c_ws = Point2{dmax + ws + lpin, 0.5 * ws};

  std::vector<Segment> path0, path1, path2, path3;

  std::vector<Segment> path1Arr;
  std::vector<Segment> path2Arr;

  path1Arr.clear();
  path2Arr.clear();

  path0.clear();
  path1.clear();
  path2.clear();
  path3.clear();

  // ###############################################################################################
  // # primary
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

  // ###############################################################################################
  // # external turn primary
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

  Point2 p1cPp = p1cP;
  Point2 p2cPp = p2cP;
  if (geometry == 1 and spin + wp < dp - 2 * wp)
  {
    p1cPp.y = -0.5 * (spin + wp);
    p2cPp.y = 0.5 * (spin + wp);
  }

  if (geometry == 0 and spin + wp < 2 * kp)
  {
    p1cPp.y = -0.5 * (spin + wp);
    p2cPp.y = 0.5 * (spin + wp);
  }

  fh->l.crop(path1Arr, path1, p1cPp, p2cPp, topM);
  fh->l.crop(path2Arr, path2, p1cPp, p2cPp, topM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  path1Arr.clear();
  path2Arr.clear();
  path1.clear();
  path2.clear();

  fh->l.crop(path1Arr, path0, p1cBp, p2cBp, topM);
  fh->l.crop(path2Arr, path3, p1cBp, p2cBp, topM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  path1Arr.clear();
  path2Arr.clear();

  path0.clear();
  path3.clear();

  // ###############################################################################################
  // # primary contacts
  // ###############################################################################################

  Point2 p1 = Point2(-0.5 * dmax - lpin, wp + 0.5 * spin);
  Point2 p2 = Point2(-0.5 * dp + 0.5 * wp, 0.5 * spin);

  fh->l.rectangle(path1, p1, p2, 0, topM); // 0 -> horizontal direction
  fh->addSegment(path1);
  path1.clear();

  p1 = Point2(-0.5 * dmax - lpin, -wp - 0.5 * spin);
  p2 = Point2(-0.5 * dp + 0.5 * wp, -0.5 * spin);

  fh->l.rectangle(path1, p1, p2, 0, topM); // 0 -> horizontal direction
  fh->addSegment(path1);
  path1.clear();

  // ###############################################################################################
  // # primary port
  // ###############################################################################################

  Point2 portP1 = Point2(-0.5 * dmax - lpin, 0.5 * (spin + wp));
  Point2 portP2 = Point2(-0.5 * dmax - lpin, -0.5 * (spin + wp));

  // ports
  Point2 portP1n;
  Point2 portP2n;
  Point2 portS1n;
  Point2 portS2n;

  // fh->addPort(p1, p2, topM,topM);

  // ###############################################################################################
  // # internal turn primary
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

  fh->l.drawSegment(path0, dp2, wp, 0, geometry, topM);
  fh->l.drawSegment(path1, dp2, wp, 1, geometry, topM);
  fh->l.drawSegment(path2, dp2, wp, 2, geometry, topM);
  fh->l.drawSegment(path3, dp2, wp, 3, geometry, topM);

  fh->l.crop(path1Arr, path0, p1cBp, p2cBp, topM);
  fh->l.crop(path2Arr, path3, p1cBp, p2cBp, topM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  path1Arr.clear();
  path2Arr.clear();

  path0.clear();
  path3.clear();

  // ###############################################################################################
  // # bridge45 right primary
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
  p1 = Point2(0.5 * dp - wp, 0.5 * (bp));
  p2 = Point2(0.5 * dp, 0.5 * (bp));
  Point2 p3 = Point2(0.5 * dp - wp - (wp + sp), -0.5 * (bp));
  Point2 p4 = Point2(0.5 * dp - (wp + sp), -0.5 * (bp));

  fh->l.bridge45Vias(path1Arr, p1, p2, p3, p4, bottomM, topM);
  fh->addSegment(path1Arr);
  path1Arr.clear();

  // ###############################################################################################
  // # primary for tapping
  // ###############################################################################################

  if (not tmpNotCutP)
  {

    fh->l.crop(path1Arr, path1, p1cTp, p2cTp, topM);
    fh->l.crop(path2Arr, path2, p1cTp, p2cTp, topM);

    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);

    path1Arr.clear();
    path2Arr.clear();
    path1.clear();
    path2.clear();

    p1 = Point2(-0.5 * dp2 + wp, 0.5 * (bpCutForTapped));
    p2 = Point2(-rshP, 0.5 * (bpCutForTapped) + wp);

    fh->l.rectangle(path1, p1, p2, 0, topM); // 0 -> horizontal direction
    fh->addSegment(path1);
    path1.clear();

    p1 = Point2(-0.5 * dp2 + wp, -0.5 * (bpCutForTapped));
    p2 = Point2(-rshP, -0.5 * (bpCutForTapped)-wp);

    fh->l.rectangle(path1, p1, p2, 0, topM); // 0 -> horizontal direction
    fh->addSegment(path1);
    path1.clear();

    // fasterCap to cut
    // fasterCap
    if (fasterCap)
    {

      p1 = Point2(-rshP, 0.5 * (bpCutForTapped) + wp);
      p2 = Point2(-rshP - wp, 0);

      fh->l.rectangle(path1, p1, p2, 1, topM); // 1 -> vertical direction

      fh->l.crop(path1Arr, path1, p1c_wp, p2c_wp, topM);
      fh->addSegment(path1Arr);
      path1Arr.clear();
      path1.clear();

      p1 = Point2(-rshP, 0);
      p2 = Point2(-rshP - wp, -0.5 * (bpCutForTapped)-wp);
      fh->l.rectangle(path1, p1, p2, 1, topM); // 1 -> vertical direction

      fh->l.crop(path1Arr, path1, p1c_wp, p2c_wp, topM);
      fh->addSegment(path1Arr);
      path1Arr.clear();
      path1.clear();

      // ports
      portP1n.x = -rshP - 0.5 * wp;
      portP1n.y = -0.5 * wp;

      portP2n.x = -rshP - 0.5 * wp;
      portP2n.y = 0.5 * wp;
    }
    else
    {
      p1 = Point2(-rshP, 0.5 * (bpCutForTapped) + wp);
      p2 = Point2(-rshP - wp, 0);

      fh->l.rectangle(path1, p1, p2, 1, topM); // 1 -> vertical direction
      fh->addSegment(path1);
      path1.clear();

      p1 = Point2(-rshP, 0);
      p2 = Point2(-rshP - wp, -0.5 * (bpCutForTapped)-wp);

      fh->l.rectangle(path1, p1, p2, 1, topM); // 1 -> vertical direction
      fh->addSegment(path1);
      path1.clear();
    }
  }
  else
  {
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
      portP1n.x = -0.5 * dp2 + 0.5 * wp;
      portP1n.y = -0.5 * wp;

      portP2n.x = -0.5 * dp2 + 0.5 * wp;
      portP2n.y = 0.5 * wp;
    }
    else
    {
      fh->addSegment(path1);
      fh->addSegment(path2);
    }

    path1.clear();
    path2.clear();
  }

  // ###############################################################################################
  // # secondary
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

  // ###############################################################################################
  // # external turn secondary
  // ###############################################################################################

  fh->l.drawSegment(path0, ds, ws, 0, geometry, bottomM);
  fh->l.drawSegment(path1, ds, ws, 1, geometry, bottomM);
  fh->l.drawSegment(path2, ds, ws, 2, geometry, bottomM);
  fh->l.drawSegment(path3, ds, ws, 3, geometry, bottomM);

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

  Point2 p1cPs = p1cP;
  Point2 p2cPs = p2cP;

  // rect geometry
  if (geometry == 1 and spin + ws < ds - 2 * ws)
  {
    p1cPs.y = -0.5 * (spin + ws);
    p2cPs.y = 0.5 * (spin + ws);
  }

  // oct geometry
  if (geometry == 0 and spin + ws < 2 * ks)
  {
    p1cPs.y = -0.5 * (spin + ws);
    p2cPs.y = 0.5 * (spin + ws);
  }

  fh->l.crop(path1Arr, path0, p1cPs, p2cPs, bottomM);
  fh->l.crop(path2Arr, path3, p1cPs, p2cPs, bottomM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  path1Arr.clear();
  path2Arr.clear();

  path0.clear();
  path3.clear();

  fh->l.crop(path1Arr, path1, p1cBs, p2cBs, bottomM);
  fh->l.crop(path2Arr, path2, p1cBs, p2cBs, bottomM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  path1Arr.clear();
  path2Arr.clear();

  path1.clear();
  path2.clear();

  // ###############################################################################################
  // # secondary contacts
  // ###############################################################################################

  p1 = Point2(0.5 * dmax + lpin, ws + 0.5 * spin);
  p2 = Point2(0.5 * ds - ws, 0.5 * spin);

  fh->l.rectangle(path1, p1, p2, 0, bottomM); // 0 -> horizontal direction
  fh->addSegment(path1);
  path1.clear();

  p1 = Point2(0.5 * dmax + lpin, -ws - 0.5 * spin);
  p2 = Point2(0.5 * ds - ws, -0.5 * spin);

  fh->l.rectangle(path1, p1, p2, 0, bottomM); // 0 -> horizontal direction
  fh->addSegment(path1);
  path1.clear();

  // ###############################################################################################
  // # secondary port
  // ###############################################################################################

  Point2 portS1 = Point2(0.5 * dmax + lpin, 0.5 * (spin + ws));
  Point2 portS2 = Point2(0.5 * dmax + lpin, -0.5 * (spin + ws));

  // fh->addPort(p1, p2, bottomM,bottomM);

  // ###############################################################################################
  // # internal turn secondary
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

  fh->l.drawSegment(path0, ds2, ws, 0, geometry, bottomM);
  fh->l.drawSegment(path1, ds2, ws, 1, geometry, bottomM);
  fh->l.drawSegment(path2, ds2, ws, 2, geometry, bottomM);
  fh->l.drawSegment(path3, ds2, ws, 3, geometry, bottomM);

  fh->l.crop(path1Arr, path1, p1cBs, p2cBs, bottomM);
  fh->l.crop(path2Arr, path2, p1cBs, p2cBs, bottomM);

  fh->addSegment(path1Arr);
  fh->addSegment(path2Arr);

  path1Arr.clear();
  path2Arr.clear();

  path1.clear();
  path2.clear();

  // ###############################################################################################
  // # secondary for tapping
  // ###############################################################################################

  if (not tmpNotCutS)
  {

    fh->l.crop(path1Arr, path0, p1cTs, p2cTs, bottomM);
    fh->l.crop(path2Arr, path3, p1cTs, p2cTs, bottomM);

    fh->addSegment(path1Arr);
    fh->addSegment(path2Arr);

    path1Arr.clear();
    path2Arr.clear();

    path0.clear();
    path3.clear();

    p1 = Point2(0.5 * ds2 - ws, 0.5 * (bsCutForTapped));
    p2 = Point2(rshS, 0.5 * (bsCutForTapped) + ws);

    fh->l.rectangle(path1, p1, p2, 0, bottomM); // o horizontal
    fh->addSegment(path1);
    path1.clear();

    p1 = Point2(0.5 * ds2 - ws, -0.5 * (bsCutForTapped));
    p2 = Point2(rshS, -0.5 * (bsCutForTapped)-ws);

    fh->l.rectangle(path1, p1, p2, 0, bottomM);
    fh->addSegment(path1);
    path1.clear();

    // fasterCap

    if (fasterCap)
    {
      p1 = Point2(rshS, 0.5 * (bsCutForTapped) + ws);
      p2 = Point2(rshS + ws, 0);
      fh->l.rectangle(path1, p1, p2, 1, bottomM);

      fh->l.crop(path1Arr, path1, p1c_ws, p2c_ws, bottomM);
      fh->addSegment(path1Arr);
      path1Arr.clear();
      path1.clear();

      p1 = Point2(rshS, 0);
      p2 = Point2(rshS + ws, -0.5 * (bsCutForTapped)-ws);
      fh->l.rectangle(path1, p1, p2, 1, bottomM);

      fh->l.crop(path1Arr, path1, p1c_ws, p2c_ws, bottomM);
      fh->addSegment(path1Arr);
      path1Arr.clear();
      path1.clear();

      // ports
      portS1n.x = rshS + 0.5 * ws;
      portS1n.y = -0.5 * ws;

      portS2n.x = rshS + 0.5 * ws;
      portS2n.y = 0.5 * ws;
    }
    else
    {

      p1 = Point2(rshS, 0.5 * (bsCutForTapped) + ws);
      p2 = Point2(rshS + ws, 0);

      fh->l.rectangle(path1, p1, p2, 1, bottomM);
      fh->addSegment(path1);
      path1.clear();

      p1 = Point2(rshS, 0);
      p2 = Point2(rshS + ws, -0.5 * (bsCutForTapped)-ws);

      fh->l.rectangle(path1, p1, p2, 1, bottomM);
      fh->addSegment(path1);
      path1.clear();
    }
  }
  else // no cutting
  {
    // fasterCap
    if (fasterCap)
    {
      fh->l.crop(path1Arr, path0, p1c_ws, p2c_ws, bottomM);
      fh->l.crop(path2Arr, path3, p1c_ws, p2c_ws, bottomM);

      fh->addSegment(path1Arr);
      fh->addSegment(path2Arr);

      path1Arr.clear();
      path2Arr.clear();

      // ports
      portS1n.x = 0.5 * ds2 - 0.5 * ws;
      portS1n.y = -0.5 * ws;

      portS2n.x = 0.5 * ds2 - 0.5 * ws;
      portS2n.y = 0.5 * ws;
    }
    else
    {
      fh->addSegment(path0);
      fh->addSegment(path3);
    }
    path0.clear();
    path3.clear();
  }

  // ###############################################################################################
  // # bridge45 left secondary
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
  p1 = Point2(-0.5 * ds, 0.5 * (bs));
  p2 = Point2(-0.5 * ds + ws, 0.5 * (bs));
  p3 = Point2(-0.5 * ds + (ws + ss), -0.5 * (bs));
  p4 = Point2(-0.5 * ds + (ws + ss) + ws, -0.5 * (bs));

  fh->l.bridge45x2(path0, path1, p1, p2, p3, p4, topM, bottomM);
  fh->addSegment(path0);
  fh->addSegment(path1);

  path0.clear();
  path1.clear();

  // add vias
  fh->l.fillVias(path1Arr, p1, Point2(p2.x, p2.y + ws), topM, bottomM);
  fh->addSegment(path1Arr);
  path1Arr.clear();

  fh->l.fillVias(path1Arr, p3, Point2(p4.x, p4.y - ws), topM, bottomM);
  fh->addSegment(path1Arr);
  path1Arr.clear();

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

    double lmin = lpin;

    if (0.5 * dp2 - wp > rshP)
    {
      rshP = 0.5 * dp2;
    }

    p1 = Point2(-0.5 * dmax - lmin, 0.5 * wtp);
    p2 = Point2(-rshP, -0.5 * wtp);

    fh->l.rectangle(path1, p1, p2, 0, topM);
    fh->addSegment(path1);
    path1.clear();
  }

  // ###############################################################################################
  // # secondary tapped
  // ###############################################################################################

  if (wts > 0)
  {

    double lmin = lpin;

    if (0.5 * ds2 - ws > rshS)
    {
      rshS = 0.5 * ds2;
    }

    p1 = Point2(0.5 * dmax + lmin, 0.5 * wts);
    p2 = Point2(rshS, -0.5 * wts);

    fh->l.rectangle(path1, p1, p2, 0, bottomM);
    fh->addSegment(path1);
    path1.clear();
  }

  return 0;
}

int Transformer2o2::FastHenryProcess(Command *cm)
{
  // ###############################################################################################
  // # FastHenryTransformer
  // #		: process command line options
  // ###############################################################################################

  ofstream runFastHenryShFile;

  if (cm->geometry == 0) // octagonal
  {
    cm->cellName = "plTr2o2Oct";
  }
  else if (cm->geometry == 1) // rectangular
  {
    cm->cellName = "plTr2o2Rect";
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
  auto threadFunc = [this, &syncronization_flag, &counter, &runFastHenryShFile, cm](double d, double w, double ws, double s, double ss, double sh)
  {
    try
    {

      FastHenry *fh = new FastHenry(*(this->fhl));

      // verbose
      string dStr = std::to_string(d);
      dStr = dStr.substr(0, dStr.find(".") + 3);

      string wStr = std::to_string(w);
      wStr = wStr.substr(0, wStr.find(".") + 3);

      string wsStr = std::to_string(ws);
      wsStr = wsStr.substr(0, wsStr.find(".") + 3);

      string ssStr = std::to_string(ss);
      ssStr = ssStr.substr(0, ssStr.find(".") + 3);

      string sStr = std::to_string(s);
      sStr = sStr.substr(0, sStr.find(".") + 3);

      string shStr = std::to_string(sh);
      shStr = shStr.substr(0, shStr.find(".") + 3);

      try
      {
        // check license
        if (not(*(cm->lv)))
        {

          d = std::rand() % 1000;
          w = std::rand() % 20;
        }

        ws = ws <= 0 ? w : ws; // if ws not set than equal to wp

        ss = ss <= 0 ? s : ss; // if ss not set than equal to s

        double ds = d - 2 * w - s + ss + 2 * ws - 2 * sh;
        // double dmax= (d>ds ? d:ds); //if d>ds then d else ds

        if (cm->tappedPrimaryWidth < 0)
        {
          cm->tappedPrimaryWidth = 2;
        }

        if (cm->tappedSecondaryWidth < 0)
        {
          cm->tappedSecondaryWidth = 2;
        }

        this->FastHenryGenCell(fh, d, ds, w, ws, s, ss, cm->ps, cm->pl, w * cm->tappedPrimaryWidth, ws * cm->tappedSecondaryWidth, cm->geometry, cm->topM, false);

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

        tmp = std::to_string(ss);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_ss" + tmp;

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
        std::cout << std::setw(80) << "transformer2o2: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " ss=" + ssStr + " sh=" + shStr;
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
        std::cout << std::setw(80) << "transformer2o2: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " ss=" + ssStr + " sh=" + shStr;
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

  std::vector<std::array<double, 6>> paramArray;
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->ws.size() * cm->s.size() * cm->ss.size() * cm->sh.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double ws = cm->ws.min; ws <= cm->ws.max; ws = ws + cm->ws.step)
      {
        for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
        {

          for (double ss = cm->ss.min; ss <= cm->ss.max; ss = ss + cm->ss.step)
          {
            for (double sh = cm->sh.min; sh <= cm->sh.max; sh = sh + cm->sh.step)
            {

              paramArray.push_back({d, w, ws, s, ss, sh});

            } // for sh

          } // for ss

        } // for s

      } // for ws

    } // for w

  } // for d

  auto loopFunction = [this, &paramArray, &threadFunc](const uint64_t a, const uint64_t b)
  {
    for (size_t k = a; k < b; k++)
    {
      const std::array<double, 6> param = paramArray[k];
      threadFunc(param[0], param[1], param[2], param[3], param[4], param[5]);
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

  std::cout << endl;
  runFastHenryShFile.close();
  chmod((cm->projectPath + "/runFastH.sh").c_str(), 0777);

  return 0;
}

int Transformer2o2::FastHenry2Process(Command *cm)
{
  // ###############################################################################################
  // # FastHenryTransformer
  // #		: process command line options
  // ###############################################################################################

  ofstream runFastHenryShFile;

  if (cm->geometry == 0) // octagonal
  {
    cm->cellName = "plTr2o2Oct";
  }
  else if (cm->geometry == 1) // rectangular
  {
    cm->cellName = "plTr2o2Rect";
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
  auto threadFunc = [this, &syncronization_flag, &counter, &runFastHenryShFile, cm](double d, double w, double ws, double s, double ss, double sh)
  {
    try
    {

      FastHenry *fh = new FastHenry(*(this->fhl));

      // verbose
      string dStr = std::to_string(d);
      dStr = dStr.substr(0, dStr.find(".") + 3);

      string wStr = std::to_string(w);
      wStr = wStr.substr(0, wStr.find(".") + 3);

      string wsStr = std::to_string(ws);
      wsStr = wsStr.substr(0, wsStr.find(".") + 3);

      string ssStr = std::to_string(ss);
      ssStr = ssStr.substr(0, ssStr.find(".") + 3);

      string sStr = std::to_string(s);
      sStr = sStr.substr(0, sStr.find(".") + 3);

      string shStr = std::to_string(sh);
      shStr = shStr.substr(0, shStr.find(".") + 3);

      try
      {
        // check license
        if (not(*(cm->lv)))
        {

          d = std::rand() % 1000;
          w = std::rand() % 20;
        }

        ws = ws <= 0 ? w : ws; // if ws not set than equal to wp

        ss = ss <= 0 ? s : ss; // if ss not set than equal to s

        double ds = d - 2 * w - s + ss + 2 * ws - 2 * sh;
        // double dmax= (d>ds ? d:ds); //if d>ds then d else ds

        if (cm->tappedPrimaryWidth < 0)
        {
          cm->tappedPrimaryWidth = 2;
        }

        if (cm->tappedSecondaryWidth < 0)
        {
          cm->tappedSecondaryWidth = 2;
        }

        this->FastHenryGenCell(fh, d, ds, w, ws, s, ss, cm->ps, cm->pl, w * cm->tappedPrimaryWidth, ws * cm->tappedSecondaryWidth, cm->geometry, cm->topM, true);

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

        tmp = std::to_string(ss);
        tmp = tmp.substr(0, tmp.find(".") + 3); // round to 2 digits
        gdsFileName = gdsFileName + "_ss" + tmp;

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
        std::cout << std::setw(80) << "transformer2o2: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " ss=" + ssStr + " sh=" + shStr;
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
        std::cout << std::setw(80) << "transformer2o2: d=" + dStr + " w=" + wStr + " ws=" + wsStr + " s=" + sStr + " ss=" + ssStr + " sh=" + shStr;
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

  std::vector<std::array<double, 6>> paramArray;
  paramArray.reserve(cm->d.size() * cm->w.size() * cm->ws.size() * cm->s.size() * cm->ss.size() * cm->sh.size());

  for (double d = cm->d.min; d <= cm->d.max; d = d + cm->d.step)
  {
    for (double w = cm->w.min; w <= cm->w.max; w = w + cm->w.step)
    {
      for (double ws = cm->ws.min; ws <= cm->ws.max; ws = ws + cm->ws.step)
      {
        for (double s = cm->s.min; s <= cm->s.max; s = s + cm->s.step)
        {

          for (double ss = cm->ss.min; ss <= cm->ss.max; ss = ss + cm->ss.step)
          {
            for (double sh = cm->sh.min; sh <= cm->sh.max; sh = sh + cm->sh.step)
            {

              paramArray.push_back({d, w, ws, s, ss, sh});

            } // for sh

          } // for ss

        } // for s

      } // for ws

    } // for w

  } // for d

  auto loopFunction = [this, &paramArray, &threadFunc](const uint64_t a, const uint64_t b)
  {
    for (size_t k = a; k < b; k++)
    {
      const std::array<double, 6> param = paramArray[k];
      threadFunc(param[0], param[1], param[2], param[3], param[4], param[5]);
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

  std::cout << endl;
  runFastHenryShFile.close();
  chmod((cm->projectPath + "/runFastH2.sh").c_str(), 0777);

  return 0;
}