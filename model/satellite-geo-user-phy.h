/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SATELLITE_GEO_USER_PHY_H
#define SATELLITE_GEO_USER_PHY_H

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "satellite-phy.h"
#include "satellite-signal-parameters.h"

namespace ns3 {


/**
 * \ingroup satellite
 *
 * The SatGeoUserPhy models the physical layer of the satellite system (UT, GW, satellite)
 */
class SatGeoUserPhy : public SatPhy
{
public:

  /**
   * Default constructor
   */
  SatGeoUserPhy (void);

  SatGeoUserPhy (SatPhy::CreateParam_t& params, InterferenceModel ifModel,
                 CarrierBandwidthConverter converter, uint32_t carrierCount);

  virtual ~SatGeoUserPhy ();

  // inherited from Object
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  virtual void DoStart (void);
  virtual void DoDispose (void);

  /**
   * Geo User specific SINR calculator.
   * Calculate SINR with Geo User PHY specific parameters and given SINR.
   *
   * \param sinr Calculated (C/NI)
   */
  virtual double CalculateSinr (double sinr);

private:

  /**
   * Configured Adjacent Channel Interference (ACI) in dB.
   */
  double m_aciInterferenceCOverIDb;

  /**
   * Configured other system interference in dB.
   */
  double m_otherSysInterferenceCOverIDb;

  /**
   * Adjacent Channel Interference (ACI) in linear.
   */
  double m_aciInterferenceCOverI;

  /**
   * Other system interference in linear.
   */
  double m_otherSysInterferenceCOverI;

};

}

#endif /* SATELLITE_GEO_USER_PHY_H */
