/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/config.h"
#include "../model/satellite-utils.h"
#include "../model/satellite-geo-net-device.h"
#include "../model/satellite-geo-feeder-phy.h"
#include "../model/satellite-geo-user-phy.h"
#include "../model/satellite-phy-tx.h"
#include "../model/satellite-phy-rx.h"
#include "../model/satellite-phy-rx-carrier-conf.h"
#include "satellite-geo-helper.h"
#include "satellite-helper.h"

NS_LOG_COMPONENT_DEFINE ("SatGeoHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGeoHelper);

TypeId
SatGeoHelper::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::SatGeoHelper")
      .SetParent<Object> ()
      .AddConstructor<SatGeoHelper> ()
      .AddAttribute ("FwdLinkInterferenceModel",
                     "Forward link interference model",
                     EnumValue (SatPhyRxCarrierConf::IF_CONSTANT),
                     MakeEnumAccessor (&SatGeoHelper::m_fwdLinkInterferenceModel),
                     MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::IF_TRACE, "Trace",
                                      SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket"))
      .AddAttribute ("RtnLinkInterferenceModel",
                     "Return link interference model",
                     EnumValue (SatPhyRxCarrierConf::IF_PER_PACKET),
                     MakeEnumAccessor (&SatGeoHelper::m_rtnLinkInterferenceModel),
                     MakeEnumChecker (SatPhyRxCarrierConf::IF_CONSTANT, "Constant",
                                      SatPhyRxCarrierConf::IF_TRACE, "Trace",
                                      SatPhyRxCarrierConf::IF_PER_PACKET, "PerPacket"))
     .AddTraceSource ("Creation", "Creation traces",
                       MakeTraceSourceAccessor (&SatGeoHelper::m_creation))
    ;
    return tid;
}

TypeId
SatGeoHelper::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this );

  return GetTypeId();
}

SatGeoHelper::SatGeoHelper()
{
  NS_LOG_FUNCTION (this );

  // this default constructor should be never called
  NS_ASSERT (false);
}

SatGeoHelper::SatGeoHelper (CarrierBandwidthConverter bandwidthConverterCb, uint32_t rtnLinkCarrierCount, uint32_t fwdLinkCarrierCount)
  : m_carrierBandwidthConverter (bandwidthConverterCb),
    m_fwdLinkCarrierCount (fwdLinkCarrierCount),
    m_rtnLinkCarrierCount (rtnLinkCarrierCount),
    m_deviceCount(0)
{
  NS_LOG_FUNCTION (this << rtnLinkCarrierCount << fwdLinkCarrierCount );

  m_deviceFactory.SetTypeId ("ns3::SatGeoNetDevice");
}

void 
SatGeoHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  m_deviceFactory.Set (n1, v1);
}

void
SatGeoHelper::SetUserPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatGeoUserPhy::" + n1, v1);
}

void
SatGeoHelper::SetFeederPhyAttribute (std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION (this << n1 );

  Config::SetDefault ("ns3::SatGeoFeederPhy::" + n1, v1);
}

NetDeviceContainer 
SatGeoHelper::Install (NodeContainer c)
{
  NS_LOG_FUNCTION (this );

  // currently only one node supported by helper
  NS_ASSERT (c.GetN () == 1);

  NetDeviceContainer devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
    devs.Add(Install(*i));
  }

  return devs;
}

Ptr<NetDevice>
SatGeoHelper::Install (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this << n);

  NS_ASSERT (m_deviceCount == 0);

  // Create SatGeoNetDevice
  Ptr<SatGeoNetDevice> satDev = m_deviceFactory.Create<SatGeoNetDevice> ();

  satDev->SetAddress (Mac48Address::Allocate ());
  n->AddDevice(satDev);
  m_deviceCount++;

  return satDev;
}

Ptr<NetDevice>
SatGeoHelper::Install (std::string aName)
{
  NS_LOG_FUNCTION (this << aName );

  Ptr<Node> n = Names::Find<Node> (aName);

  return Install (n);
}

void
SatGeoHelper::AttachChannels (Ptr<NetDevice> d, Ptr<SatChannel> ff, Ptr<SatChannel> fr, Ptr<SatChannel> uf, Ptr<SatChannel> ur, Ptr<SatAntennaGainPattern> userAgp, Ptr<SatAntennaGainPattern> feederAgp, uint32_t userBeamId )
{
  NS_LOG_FUNCTION (this << d << ff << fr << uf << ur << userAgp << feederAgp << userBeamId);

  Ptr<SatGeoNetDevice> dev = DynamicCast<SatGeoNetDevice> (d);
  //Ptr<MobilityModel> mobility = dev->GetNode()->GetObject<MobilityModel>();

  SatPhy::CreateParam_t params;
  params.m_beamId = userBeamId;
  params.m_device = d;
  params.m_txCh = uf;
  params.m_rxCh = ur;

  Ptr<SatGeoUserPhy> uPhy = CreateObject<SatGeoUserPhy> (params, m_rtnLinkInterferenceModel,
                                                         m_carrierBandwidthConverter, m_rtnLinkCarrierCount);

  params.m_txCh = fr;
  params.m_rxCh = ff;

  Ptr<SatGeoFeederPhy> fPhy = CreateObject<SatGeoFeederPhy> (params, m_fwdLinkInterferenceModel,
                                                             m_carrierBandwidthConverter, m_fwdLinkCarrierCount);

  SatPhy::ReceiveCallback uCb = MakeCallback (&SatGeoNetDevice::ReceiveUser, dev);
  SatPhy::ReceiveCallback fCb = MakeCallback (&SatGeoNetDevice::ReceiveFeeder, dev);

  uPhy->SetAttribute ("ReceiveCb", CallbackValue (uCb));
  fPhy->SetAttribute ("ReceiveCb", CallbackValue (fCb));

  // Note, that currently we have only one set of antenna patterns,
  // which are utilized in both in user link and feeder link, and
  // in both uplink and downlink directions.
  uPhy->SetTxAntennaGainPattern (userAgp);
  uPhy->SetRxAntennaGainPattern (userAgp);
  fPhy->SetTxAntennaGainPattern (feederAgp);
  fPhy->SetRxAntennaGainPattern (feederAgp);

  dev->AddUserPhy(uPhy, userBeamId);
  dev->AddFeederPhy(fPhy, userBeamId);

  uPhy->Initialize();
  fPhy->Initialize();
}

void
SatGeoHelper::EnableCreationTraces(Ptr<OutputStreamWrapper> stream, CallbackBase &cb)
{
  NS_LOG_FUNCTION (this);

  TraceConnect ("Creation", "SatGeoHelper",  cb);
}

} // namespace ns3
