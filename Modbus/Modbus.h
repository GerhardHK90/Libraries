/*
    Modbus.h - an Arduino library for a Modbus TCP slave.
*/


#include "Arduino.h"

#include <SPI.h>
#include <Ethernet.h>
#include <Discovery.h>
#include "CircularBuffer.h"

#ifndef Modbus_h
#define Modbus_h

#define ENABLE_DEBUG

// Debug functions
#ifdef ENABLE_DEBUG
#define DEBUG(x) (x)
#else 
#define DEBUG(x)
#endif
#define MTIMEOUT 1500
#define MBAP_PORT 502

#define ReadCoilStatus 				1
#define ReadInputStatus				2
#define ReadHoldingRegisters		3
#define ReadInputRegisters			4
#define WriteSingleCoilStatus		5
#define WriteSingleRegister			6
#define MultipleCoilWrite		   15
#define MultipleRegisterWrite	   16

//#define Offset
// Debug functions
#ifdef Offset
#define InputRegAddrOffSet 			10000
#define RegAddrOffSet 				30000
#define HoldingRegAddrOffSet 		40000
#else 
#define InputRegAddrOffSet 			0
#define RegAddrOffSet 				0
#define HoldingRegAddrOffSet 		0
#endif

#if defined(__SAM3X8E__) || defined(__MK20DX256__)
#define MODBUSPACKET 500
#else
#define MODBUSPACKET 250												   
#endif
typedef unsigned char uchar;
struct MBAPHeaderFields {
  short TransactionID;
  short ProtocolID; // should always be 0
  short Length;
  byte UnitID;
};
struct RequestHeaderFields {
  byte FunctionCode;
  short Address;
  short NumberOfPoints;
};


class Modbus
{
 
public:
  Modbus(unsigned short SizeOfRegisters, unsigned char _UnitType = 0);
  Modbus(unsigned short SizeOfRegisters,unsigned short SizeOfCoils ,unsigned char _UnitType = 0);
  ~Modbus();
  
  void Init(byte wiznetResetPin,byte Address);
  void Init();
  void Network_Setup(byte _address, byte _mac[6]);
  int Run();  
  
  bool ModbusMode;
  void setProccessTime(unsigned char _Slice);
  
  void setCoil(bool value, byte Index);
  bool getCoil(byte Index);
  
  void setInput(bool value, byte Index);
  bool getInput(byte Index);
  
  void setHoldingRegister(unsigned short value, byte Index);
  unsigned short getHoldingRegister(byte Index);
  
  void setRegister(unsigned short value, byte Index);
  unsigned short getRegister(byte Index);
  
  bool getTimeOut();
  unsigned short getSpeechPort();
  byte getAddress();
  uint8_t* getMac();
  static void swapHighLowByteOfCharArray(char* value,int size);
  
  bool *Coil;			//R/W
  bool *Input;
  unsigned short *HoldingRegister; //R/W
  unsigned short *Register;  
  bool CoilsWritten;
  bool HoldingRegistersWritten;
private: 
  EthernetServer ModbusProtocol;
  EthernetClient client;
  unsigned short modbusSize;
  byte address;
  byte mac[6];
  CircularBuffer *Data;
  byte *ByteArray;
  byte *Response;

  uint8_t UnitType;
  Dis *CardAttributes;

  unsigned short ResponseLength;
  MBAPHeaderFields MBAP;
  RequestHeaderFields ADU;
  unsigned short Reads;
  unsigned char Slice;
  int PacketSize;
  unsigned long Timer;
  unsigned long FunctionTimer;
  bool startUp;
  unsigned char UnitIdentifier;
};

#endif

// Speculations on Modbus message structure:
/**********Master***********/
// 0002 - Transaction ID
// 0000 - Protocol ID
// 0006 - Length 			= Unit identifier, Function code , Start address,# number of points	+ registers if writing data   
// 01 - Unit identifier

// 01 - Function code 
// 0000 - Start address (0x00001 - 1)
// 0005 - # number of points
// FFFF - Registers only if writing data
// /**********Master End*******/

// /**********Client***********/
// 0002 - Transaction ID - echo
// 0000 - Protocol ID    - echo
// 0004 - Length	     - New = Unit identifier, Function code ,Byte count,Data	     
// 01 - Unit identifier  - echo

// 01 - Function code    - echo
// 00 - Byte count	 	 - New = Data	     	
// 00 - Data	     	 - New
// /**********Client End*******/

