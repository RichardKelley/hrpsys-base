// -*- C++ -*-
/*!
 * @file  ServoController.cpp
 * @brief servo controller component
 * $Date$
 *
 * $Id$
 */

#include <rtm/CorbaNaming.h>
#include <hrpModel/Link.h>
#include <hrpModel/ModelLoaderUtil.h>
#include "ServoController.h"
#include "util/VectorConvert.h"

#include "ServoSerial.h"

using namespace std;

// Module specification
// <rtc-template block="module_spec">
static const char* nullcomponent_spec[] =
  {
    "implementation_id", "ServoController",
    "type_name",         "ServoController",
    "description",       "null component",
    "version",           "1.0",
    "vendor",            "AIST",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    ""
  };
// </rtc-template>

ServoController::ServoController(RTC::Manager* manager)
  : RTC::DataFlowComponentBase(manager),
    // <rtc-template block="initializer">
    m_ServoControllerServicePort("ServoControllerService")
    // </rtc-template>
{
    m_service0.servo(this);
}

ServoController::~ServoController()
{
}



RTC::ReturnCode_t ServoController::onInitialize()
{
  std::cout << m_profile.instance_name << ": onInitialize()" << std::endl;
  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  
  // </rtc-template>

  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers

  // Set OutPort buffer
  
  // Set service provider to Ports
  m_ServoControllerServicePort.registerProvider("service0", "ServoControllerService", m_ServoControllerService);
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  addPort(m_ServoControllerServicePort);
  
  // </rtc-template>

  RTC::Properties& prop = getProperties();

  // get servo.devname
  std::string devname = prop["servo.devname"];
  if ( devname  == "" ) {
      std::cerr << "\e[1;31m[ERROR] " <<  m_profile.instance_name << ": needs servo.devname property\e[0m" << std::endl;
      return RTC::RTC_ERROR;
  }

  // get servo.id
  coil::vstring servo_ids = coil::split(prop["servo.id"], ",");
  if ( servo_ids.size() == 0 ) {
      std::cerr << "\e[1;31m[ERROR] " <<  m_profile.instance_name << ": needs servo.id property\e[0m" << std::endl;
      return RTC::RTC_ERROR;
  }
  servo_id.resize(servo_ids.size());
  for(int i = 0; i < servo_ids.size(); i++) {
      coil::stringTo(servo_id[i], servo_ids[i].c_str());
  }

  std::cout << m_profile.instance_name << ": servo_id : ";
  for(int i = 0; i < servo_id.size(); i++) {
      std::cerr << servo_id[i] << " ";
  }
  std::cerr << std::endl;

  // get servo.offset
  coil::vstring servo_offsets = coil::split(prop["servo.offset"], ",");
  if ( servo_offsets.size() == 0 ) {
      servo_offset.resize(servo_ids.size());
  } else {
      servo_offset.resize(servo_offsets.size());
  }
  if ( servo_ids.size() != servo_offset.size() ) {
      std::cerr << "\e[1;31m[ERROR] " <<  m_profile.instance_name << ": servo.id and servo.offset property must have same length\e[0m" << std::endl;
      return RTC::RTC_ERROR;
  }
  for(int i = 0; i < servo_offsets.size(); i++) {
      coil::stringTo(servo_offset[i], servo_offsets[i].c_str());
  }

  std::cout << m_profile.instance_name << ": servo_offset : ";
  for(int i = 0; i < servo_offset.size(); i++) {
      std::cerr << servo_offset[i] << " ";
  }
  std::cerr << std::endl;

  serial = new ServoSerial((char *)(devname.c_str()));

  m_robot = hrp::BodyPtr(new hrp::Body());

  RTC::Manager& rtcManager = RTC::Manager::instance();
  std::string nameServer = rtcManager.getConfig()["corba.nameservers"];
  int comPos = nameServer.find(",");
  if (comPos < 0){
      comPos = nameServer.length();
  }
  nameServer = nameServer.substr(0, comPos);
  RTC::CorbaNaming naming(rtcManager.getORB(), nameServer.c_str());
  if (!loadBodyFromModelLoader(m_robot, prop["model"].c_str(), 
                               CosNaming::NamingContext::_duplicate(naming.getRootContext())
                               )){
      std::cerr << "failed to load model[" << prop["model"] << "]" 
                << std::endl;
  }

  return RTC::RTC_OK;
}



RTC::ReturnCode_t ServoController::onFinalize()
{
    if ( serial ) delete serial;
    return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t ServoController::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ServoController::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

RTC::ReturnCode_t ServoController::onActivated(RTC::UniqueId ec_id)
{
  std::cout << m_profile.instance_name<< ": onActivated(" << ec_id << ")" << std::endl;

  for (vector<int>::iterator it = servo_id.begin(); it != servo_id.end(); it++ ){
      serial->setTorqueOn(*it);
  }

  return RTC::RTC_OK;
}

RTC::ReturnCode_t ServoController::onDeactivated(RTC::UniqueId ec_id)
{
  std::cout << m_profile.instance_name<< ": onDeactivated(" << ec_id << ")" << std::endl;

  for (vector<int>::iterator it = servo_id.begin(); it != servo_id.end(); it++ ){
      serial->setTorqueOff(*it);
  }

  return RTC::RTC_OK;
}

RTC::ReturnCode_t ServoController::onExecute(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t ServoController::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ServoController::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ServoController::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ServoController::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ServoController::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

bool ServoController::setJointAngle(short id, double angle, double tm)
{
    serial->setPosition(id, angle, tm);
    return true;
}

bool ServoController::setJointAngles(const OpenHRP::ServoControllerService::dSequence angles, double tm)
{
    int id[servo_id.size()];
    double tms[servo_id.size()];
    double rad[servo_id.size()];
    for( int i = 0; i < servo_id.size(); i++ ) {
        id[i] = servo_id[i];
        tms[i] = tm;
        rad[i] = angles.get_buffer()[i]+servo_offset[i];
    }
    if ( angles.length() != servo_id.size() ) {
        std::cerr << "[ERROR] " <<  m_profile.instance_name << ": size of servo.id(" << angles.length() << ") is not correct, expected" << servo_id.size() << std::endl;
        return false;
    }
    serial->setPositions(servo_id.size(), id, rad, tms);
    return true;
}

bool ServoController::getJointAngle(short id, double &angle)
{
    int ret;
    ret = serial->getPosition(id, &angle);
    if (ret < 0) return false;
    return true;
}

bool ServoController::getJointAngles(OpenHRP::ServoControllerService::dSequence_out &angles)
{
    int ret;

    angles = new OpenHRP::ServoControllerService::dSequence();
    angles->length(servo_id.size());
    for(int i=0; i < servo_id.size(); i++){
        ret = serial->getPosition(servo_id[i], &(angles->get_buffer()[i]));
        if (ret < 0) return false;
    }
    return true;
}

bool ServoController::addJointGroup(const char *gname, const ::OpenHRP::ServoControllerService::StrSequence jnames)
{
    std::vector<int> indices;
    for (size_t i=0; i<jnames.length(); i++){
        hrp::Link *l = m_robot->link(std::string(jnames[i]));
        if (l){
            indices.push_back(l->jointId);
        }else{
            std::cerr << "[addJointGroup] link name " << jnames[i] << "is not found" << std::endl;
            return false;
        }
    }
    joint_groups[gname] = indices;

    return true;
}

bool ServoController::removeJointGroup(const char *gname)
{
    joint_groups.erase(gname);
}

bool ServoController::setJointAnglesOfGroup(const char *gname, const OpenHRP::ServoControllerService::dSequence angles, double tm)
{
    if ( joint_groups.find(gname) != joint_groups.end()) {
        int len = joint_groups[gname].size();
        if ( angles.length() != len ) {
            std::cerr << "[ERROR] " <<  m_profile.instance_name << ": size of servo.id(" << angles.length() << ") is not correct, expected" << len << std::endl;
            return false;
        }
        int id[len];
        double tms[len];
        double rad[len];
        for( int i = 0; i < len; i++ ) {
            id[i] = joint_groups[gname][i];
            tms[i] = tm;
            double offset;
            for( int j = 0; j < servo_id.size(); j++ ) {
                if ( servo_id[j] == id[i]) {
                    offset = servo_offset[j];
                }
            }
            rad[i] = angles.get_buffer()[i]+offset;
        }
        serial->setPositions(servo_id.size(), id, rad, tms);
    }
    return true;
}


extern "C"
{

  void ServoControllerInit(RTC::Manager* manager)
  {
    RTC::Properties profile(nullcomponent_spec);
    manager->registerFactory(profile,
                             RTC::Create<ServoController>,
                             RTC::Delete<ServoController>);
  }

};


