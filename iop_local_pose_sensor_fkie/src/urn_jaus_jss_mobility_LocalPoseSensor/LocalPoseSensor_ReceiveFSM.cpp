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


#include "urn_jaus_jss_mobility_LocalPoseSensor/LocalPoseSensor_ReceiveFSM.h"
#include <iop_component_fkie/iop_config.h>



using namespace JTS;

namespace urn_jaus_jss_mobility_LocalPoseSensor
{



LocalPoseSensor_ReceiveFSM::LocalPoseSensor_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_Events::Events_ReceiveFSM* pEvents_ReceiveFSM, urn_jaus_jss_core_AccessControl::AccessControl_ReceiveFSM* pAccessControl_ReceiveFSM)
{

	/*
	 * If there are other variables, context must be constructed last so that all
	 * class variables are available if an EntryAction of the InitialState of the
	 * statemachine needs them.
	 */
	context = new LocalPoseSensor_ReceiveFSMContext(*this);

	this->pTransport_ReceiveFSM = pTransport_ReceiveFSM;
	this->pEvents_ReceiveFSM = pEvents_ReceiveFSM;
	this->pAccessControl_ReceiveFSM = pAccessControl_ReceiveFSM;
	p_tf_frame_odom = "odom";
	p_tf_frame_robot = "base_link";
}



LocalPoseSensor_ReceiveFSM::~LocalPoseSensor_ReceiveFSM()
{
	delete context;
}

void LocalPoseSensor_ReceiveFSM::setupNotifications()
{
	pAccessControl_ReceiveFSM->registerNotification("Receiving_Ready_NotControlled", ieHandler, "InternalStateChange_To_LocalPoseSensor_ReceiveFSM_Receiving_Ready_NotControlled", "AccessControl_ReceiveFSM");
	pAccessControl_ReceiveFSM->registerNotification("Receiving_Ready_Controlled", ieHandler, "InternalStateChange_To_LocalPoseSensor_ReceiveFSM_Receiving_Ready_Controlled", "AccessControl_ReceiveFSM");
	pAccessControl_ReceiveFSM->registerNotification("Receiving_Ready", ieHandler, "InternalStateChange_To_LocalPoseSensor_ReceiveFSM_Receiving_Ready_NotControlled", "AccessControl_ReceiveFSM");
	pAccessControl_ReceiveFSM->registerNotification("Receiving", ieHandler, "InternalStateChange_To_LocalPoseSensor_ReceiveFSM_Receiving_Ready_NotControlled", "AccessControl_ReceiveFSM");
	registerNotification("Receiving_Ready_NotControlled", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "LocalPoseSensor_ReceiveFSM");
	registerNotification("Receiving_Ready_Controlled", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_Controlled", "LocalPoseSensor_ReceiveFSM");
	registerNotification("Receiving_Ready", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready", "LocalPoseSensor_ReceiveFSM");
	registerNotification("Receiving", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving", "LocalPoseSensor_ReceiveFSM");
	iop::Config cfg("~LocalPoseSensor");
	int source = 0;
	std::map<int, std::string> source_map;
	source_map[0] = "tf";
	source_map[1] = "PoseStamped";
	source_map[2] = "Odometry";
	cfg.param("source_type", source, source, true, true, "", source_map);
	switch (source) {
		case 1 :
			p_pose_sub = cfg.subscribe<geometry_msgs::PoseStamped>("pose", 1, &LocalPoseSensor_ReceiveFSM::poseReceived, this);
			break;
		case 2 :
			p_odom_sub = cfg.subscribe<nav_msgs::Odometry>("odom", 1, &LocalPoseSensor_ReceiveFSM::odomReceived, this);
			break;
		default :
			cfg.param("tf_frame_odom", p_tf_frame_odom, p_tf_frame_odom);
			cfg.param("tf_frame_robot", p_tf_frame_robot, p_tf_frame_robot);
			double tf_hz = 10.0;
			cfg.param("tf_hz", tf_hz, tf_hz);
			if (tf_hz == 0.0) {
				tf_hz = 10.0;
			}
			ros::NodeHandle nh;
			p_tf_timer = nh.createTimer(ros::Duration(1.0 / tf_hz), &LocalPoseSensor_ReceiveFSM::tfCallback, this);
			break;
	}
	pEvents_ReceiveFSM->get_event_handler().register_query(QueryLocalPose::ID);
}

void LocalPoseSensor_ReceiveFSM::SendAction(std::string arg0, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	/// Insert User Code HERE
	ROS_DEBUG_NAMED("LocalPoseSensor", "request %s from %d.%d.%d", arg0.c_str(),
			  transportData.getSrcSubsystemID(), transportData.getSrcNodeID(), transportData.getSrcComponentID());
	JausAddress sender = JausAddress(transportData.getSrcSubsystemID(),
									 transportData.getSrcNodeID(),
									 transportData.getSrcComponentID());
	if (strcmp(arg0.c_str(), "ReportLocalPose") == 0) {
	  this->sendJausMessage(p_report_local_pose, sender);
	}
}

void LocalPoseSensor_ReceiveFSM::updateLocalPoseAction(SetLocalPose msg)
{
	/// Insert User Code HERE
	ROS_WARN_NAMED("LocalPoseSensor", "updateLocalPoseAction not implemented");
}



bool LocalPoseSensor_ReceiveFSM::isControllingClient(Receive::Body::ReceiveRec transportData)
{
	//// By default, inherited guards call the parent function.
	//// This can be replaced or modified as needed.
	return pAccessControl_ReceiveFSM->isControllingClient(transportData );
}

void LocalPoseSensor_ReceiveFSM::tfCallback(const ros::TimerEvent& e)
{
	try {
		tfListener.waitForTransform(p_tf_frame_odom, p_tf_frame_robot, ros::Time(0), ros::Duration(0.3));
		tf::StampedTransform transform;
		tfListener.lookupTransform(p_tf_frame_odom, p_tf_frame_robot, ros::Time(0), transform);
		p_report_local_pose.getBody()->getLocalPoseRec()->setX(transform.getOrigin().x());
		p_report_local_pose.getBody()->getLocalPoseRec()->setY(transform.getOrigin().y());
		p_report_local_pose.getBody()->getLocalPoseRec()->setZ(transform.getOrigin().z());
		try {
			tf::Quaternion q(transform.getRotation().getX(), transform.getRotation().getY(),
					transform.getRotation().getZ(), transform.getRotation().getW());
			tf::Matrix3x3 m(q);
			double yaw, pitch, roll;
			m.getRPY(roll, pitch, yaw);
			p_report_local_pose.getBody()->getLocalPoseRec()->setRoll(roll);
			p_report_local_pose.getBody()->getLocalPoseRec()->setPitch(pitch);
			p_report_local_pose.getBody()->getLocalPoseRec()->setYaw(yaw);
		} catch (const std::exception& e) {
			ROS_WARN_NAMED("LocalPoseSensor", "Error while get yaw, pitch, roll from quaternion: %s", e.what());
		}
		// set timestamp
		ReportLocalPose::Body::LocalPoseRec::TimeStamp ts;
		// current date/time based on current system
		iop::Timestamp stamp(ros::Time::now());
		ts.setDay(stamp.days);
		ts.setHour(stamp.hours);
		ts.setMinutes(stamp.minutes);
		ts.setSeconds(stamp.seconds);
		ts.setMilliseconds(stamp.milliseconds);
		p_report_local_pose.getBody()->getLocalPoseRec()->setTimeStamp(ts);
		pEvents_ReceiveFSM->get_event_handler().set_report(QueryLocalPose::ID, &p_report_local_pose);
	} catch (tf::TransformException &ex){
		ROS_WARN_STREAM_THROTTLE(1.0, "Could not lookup transform from " << p_tf_frame_robot << " to " << p_tf_frame_odom << ": " << ex.what());
	}

}

void LocalPoseSensor_ReceiveFSM::poseReceived(const geometry_msgs::PoseStamped::ConstPtr& pose)
{
	p_report_local_pose.getBody()->getLocalPoseRec()->setX(pose->pose.position.x);
	p_report_local_pose.getBody()->getLocalPoseRec()->setY(pose->pose.position.y);
	p_report_local_pose.getBody()->getLocalPoseRec()->setZ(pose->pose.position.z);
	try {
		tf::Quaternion q(pose->pose.orientation.x, pose->pose.orientation.y,
				pose->pose.orientation.z, pose->pose.orientation.w);
		tf::Matrix3x3 m(q);
		double yaw, pitch, roll;
		m.getRPY(roll, pitch, yaw);
		p_report_local_pose.getBody()->getLocalPoseRec()->setRoll(roll);
		p_report_local_pose.getBody()->getLocalPoseRec()->setPitch(pitch);
		p_report_local_pose.getBody()->getLocalPoseRec()->setYaw(yaw);
	} catch (const std::exception& e) {
		ROS_WARN_NAMED("LocalPoseSensor", "Error while get yaw, pitch, roll from quaternion: %s", e.what());
	}
	// set timestamp
	ReportLocalPose::Body::LocalPoseRec::TimeStamp ts;
	// current date/time based on current system
	iop::Timestamp stamp(pose->header.stamp);
	ts.setDay(stamp.days);
	ts.setHour(stamp.hours);
	ts.setMinutes(stamp.minutes);
	ts.setSeconds(stamp.seconds);
	ts.setMilliseconds(stamp.milliseconds);
	p_report_local_pose.getBody()->getLocalPoseRec()->setTimeStamp(ts);
	pEvents_ReceiveFSM->get_event_handler().set_report(QueryLocalPose::ID, &p_report_local_pose);
}


void LocalPoseSensor_ReceiveFSM::odomReceived(const nav_msgs::Odometry::ConstPtr& odom)
{
	p_report_local_pose.getBody()->getLocalPoseRec()->setX(odom->pose.pose.position.x);
	p_report_local_pose.getBody()->getLocalPoseRec()->setY(odom->pose.pose.position.y);
	p_report_local_pose.getBody()->getLocalPoseRec()->setZ(odom->pose.pose.position.z);
	try {
		tf::Quaternion q(odom->pose.pose.orientation.x, odom->pose.pose.orientation.y,
				odom->pose.pose.orientation.z, odom->pose.pose.orientation.w);
		tf::Matrix3x3 m(q);
		double yaw, pitch, roll;
		m.getRPY(roll, pitch, yaw);
		p_report_local_pose.getBody()->getLocalPoseRec()->setRoll(roll);
		p_report_local_pose.getBody()->getLocalPoseRec()->setPitch(pitch);
		p_report_local_pose.getBody()->getLocalPoseRec()->setYaw(yaw);
	} catch (const std::exception& e) {
		ROS_WARN_NAMED("LocalPoseSensor", "Error while get yaw, pitch, roll from quaternion: %s", e.what());
	}
	// set timestamp
	ReportLocalPose::Body::LocalPoseRec::TimeStamp ts;
	// current date/time based on current system
	iop::Timestamp stamp(odom->header.stamp);
	ts.setDay(stamp.days);
	ts.setHour(stamp.hours);
	ts.setMinutes(stamp.minutes);
	ts.setSeconds(stamp.seconds);
	ts.setMilliseconds(stamp.milliseconds);
	p_report_local_pose.getBody()->getLocalPoseRec()->setTimeStamp(ts);
	pEvents_ReceiveFSM->get_event_handler().set_report(QueryLocalPose::ID, &p_report_local_pose);
}

};
