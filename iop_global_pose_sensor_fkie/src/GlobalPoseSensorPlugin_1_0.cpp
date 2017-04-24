/**
ROS/IOP Bridge
Copyright (c) 2017 Fraunhofer

This program is dual licensed; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation, or
enter into a proprietary license agreement with the copyright
holder.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; or you can read the full license at
<http://www.gnu.de/documents/gpl-2.0.html>
*/

/** \author Alexander Tiderko */

#include <pluginlib/class_list_macros.h>
#include <GlobalPoseSensorPlugin_1_0.h>

using namespace iop;
using namespace urn_jaus_jss_mobility_GlobalPoseSensor_1_0 ;
using namespace urn_jaus_jss_core_AccessControl_1_0;
using namespace urn_jaus_jss_core_Events_1_0;
using namespace urn_jaus_jss_core_Transport_1_0;


GlobalPoseSensorPlugin_1_0::GlobalPoseSensorPlugin_1_0()
{
	p_my_service = NULL;
	p_base_service = NULL;
	p_events_service = NULL;
	p_transport_service = NULL;
}

GlobalPoseSensorPlugin_1_0::~GlobalPoseSensorPlugin_1_0()
{

}

JTS::Service* GlobalPoseSensorPlugin_1_0::get_iop_service()
{
	return p_my_service;
}

const std::type_info & GlobalPoseSensorPlugin_1_0::get_iop_service_type()
{
	return typeid(GlobalPoseSensorService);
}

const std::type_info & GlobalPoseSensorPlugin_1_0::get_base_service_type()
{
	return typeid(AccessControlService);
}


void GlobalPoseSensorPlugin_1_0::create_jts_service(JTS::JausRouter* jaus_router)
{
	p_base_service = dynamic_cast<AccessControlService *>(get_base_service());
	p_events_service = dynamic_cast<EventsService *>(get_base_service(2));
	p_transport_service = dynamic_cast<TransportService *>(get_base_service(3));
	p_my_service = new GlobalPoseSensorService(jaus_router, p_transport_service, p_events_service, p_base_service);
}

PLUGINLIB_EXPORT_CLASS(iop::GlobalPoseSensorPlugin_1_0, iop::PluginInterface)