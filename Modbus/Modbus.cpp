#include "Modbus.h"

//Remove ModbusProtocol(MBAP_PORT) as ons die connection moet maak
Modbus::Modbus(unsigned short SizeOfRegisters,unsigned char _UnitType):ModbusProtocol(MBAP_PORT){
    Coil = new bool[SizeOfRegisters];			//R/W
    Input = new bool[SizeOfRegisters];
    HoldingRegister = new unsigned short[SizeOfRegisters]; //R/W
    Register = new unsigned short[SizeOfRegisters];
	Data = new CircularBuffer(MODBUSPACKET);
	ByteArray = new byte[SizeOfRegisters*2+sizeof(MBAP)*2];
	Response = new byte[SizeOfRegisters*2+sizeof(MBAP)*2];
	modbusSize = SizeOfRegisters*2+sizeof(MBAP)*2;
    memset(Coil,0,SizeOfRegisters);
    memset(Input,0,SizeOfRegisters);
    memset(HoldingRegister,0,SizeOfRegisters*2);
    memset(Register,0,SizeOfRegisters*2);    
    ModbusMode = false;
    startUp = false;
	CoilsWritten = false;
    HoldingRegistersWritten = false;
	UnitType = 	_UnitType;
	Slice = 10;
	if(_UnitType != 0){
		CardAttributes = new Dis(_UnitType);
		UnitIdentifier = CardAttributes->info.address;
		Init();		
	}	
}
Modbus::Modbus(unsigned short SizeOfRegisters,unsigned short SizeOfCoils ,unsigned char _UnitType):ModbusProtocol(MBAP_PORT){
    Coil = new bool[SizeOfCoils];			//R/W
    Input = new bool[SizeOfCoils];
    HoldingRegister = new unsigned short[SizeOfRegisters]; //R/W
    Register = new unsigned short[SizeOfRegisters];
	Data = new CircularBuffer(MODBUSPACKET);
	if(SizeOfRegisters*2 > SizeOfCoils){
	ByteArray = new byte[SizeOfRegisters*2+sizeof(MBAP)*2];
	Response = new byte[SizeOfRegisters*2+sizeof(MBAP)*2];
	modbusSize = SizeOfRegisters*2+sizeof(MBAP)*2;	
	}
	else{
	ByteArray = new byte[SizeOfCoils+sizeof(MBAP)*2];
	Response = new byte[SizeOfCoils+sizeof(MBAP)*2];
	modbusSize = SizeOfCoils+sizeof(MBAP)*2;	
	}
    memset(Coil,0,SizeOfCoils);
    memset(Input,0,SizeOfCoils);
    memset(HoldingRegister,0,SizeOfRegisters*2);
    memset(Register,0,SizeOfRegisters*2);    
    ModbusMode = false;
    startUp = false;
	CoilsWritten = false;
    HoldingRegistersWritten = false;
	UnitType = 	_UnitType;
	Slice = 10;
	if(_UnitType != 0){
		CardAttributes = new Dis(_UnitType);
		UnitIdentifier = CardAttributes->info.address;
		Init();		
	}
}
Modbus::~Modbus(){
    delete[] Coil;
    delete[] Input;
    delete[] HoldingRegister;
    delete[] Register;
	delete[] ByteArray;
	delete[] Response;
	delete Data;
}
void Modbus::Init(byte wiznetResetPin,byte Address){
    byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
	address = Address;
    IPAddress ip(192,168,0, Address);
    // IPAddress ip(172,20,255, 177);
    pinMode(wiznetResetPin, OUTPUT);
    digitalWrite(wiznetResetPin, LOW);   // Resets Wiznet 5200
    delay(200);
    digitalWrite(wiznetResetPin, HIGH);   // Resets Wiznet 5200
    Ethernet.begin(mac, ip);
}
void Modbus::Init(){
	Serial.println("Modbus::Init()");
	#if defined (ARCH_TEENSY)
		Serial.println("defined((ARCH_TEENSY))");
		if (UnitType == LF_STANDALONE){
		pinMode(5, OUTPUT);
        digitalWrite(5, LOW);   // Resets Wiznet 5200
        delay(200);
        digitalWrite(5, HIGH);   // Resets Wiznet 5200
		Serial.println("Wiz reset");
		}
	#endif
	#if defined(ARCH_SAM)
    Serial.println("defined(ARCH_SAM)");
	if(UnitType == DIGITAL_UNIT)
    {
        pinMode(A0, OUTPUT);
        digitalWrite(A0, LOW);   // Resets Wiznet 5200
        delay(200);
        digitalWrite(A0, HIGH);   // Resets Wiznet 5200
    }
	else
    {
        pinMode(A1, OUTPUT);
        digitalWrite(A1, LOW);   // Resets Wiznet 5200
        delay(200);
        digitalWrite(A1, HIGH);   // Resets Wiznet 5200
    }
	#endif
    // pinMode(A1, OUTPUT);
    // digitalWrite(A1, LOW);   // Resets Wiznet 5200
    // delay(200);
    // digitalWrite(A1, HIGH);   // Resets Wiznet 5200
	#if defined(UNO)
	if((UnitType == MANAGEMENT_UNIT)||(UnitType == ANALOG)/*||(UnitType == ANALOG_4_INPUTS)*/){   
		
		pinMode(2, OUTPUT);
		//digitalWrite(2, HIGH);   // Resets Wiznet 5200
		//delay(10);
        digitalWrite(2, LOW);   // Resets Wiznet 5200
        delay(200);
        digitalWrite(2, HIGH);   // Resets Wiznet 5200
		//delay(250);
		Serial.println("Reset");
    }
    else{    
        pinMode(9, OUTPUT);
        digitalWrite(9, LOW);   // Resets Wiznet 5200
        delay(200);
        digitalWrite(9, HIGH);   // Resets Wiznet 5200
    }
	#endif 
	
	

    Network_Setup(CardAttributes->info.address,CardAttributes->info.mac);

}
unsigned short Modbus::getSpeechPort(){
	return CardAttributes->info.Speechport;
}
byte Modbus::getAddress(){
	return CardAttributes->info.address;
}
uint8_t* Modbus::getMac(){
	
	return CardAttributes->info.mac;
}
void Modbus::Network_Setup(byte _address, byte _mac[6]){
	uint32_t i = 0;
    
    address = _address;
    for (i=0;i<6;i++)
        mac[i] =_mac[i];
    
    IPAddress ip(192, 168, 0, address);
    Ethernet.begin(mac, ip);

}
void Modbus::setCoil(bool value, byte Index){
    Coil[Index] = value;
}
bool Modbus::getCoil(byte Index){
    return Coil[Index];
}
void Modbus::setInput(bool value, byte Index){
    Input[Index] = value;
}
bool Modbus::getInput(byte Index){
    return Input[Index];
}
void Modbus::setHoldingRegister(unsigned short value, byte Index){
    HoldingRegister[Index] = value;
}
unsigned short Modbus::getHoldingRegister(byte Index){
    return HoldingRegister[Index];
}
void Modbus::setRegister(unsigned short value, byte Index){
    Register[Index] = value;
}
unsigned short Modbus::getRegister(byte Index){
    return Register[Index];
}
bool Modbus::getTimeOut(){
	bool value = true;
		if((millis()-Timer) < MTIMEOUT)
			value = false;
			
    return value;
}
int Modbus::Run(){ 
    //Serial.println("Modbus::Run()");
    int RecievedData = 0;  
	if (client.available()){
		Serial.print("client.available() = ");
		Serial.println(client.available());
	}
    if(ModbusProtocol.available())	
		client = ModbusProtocol.available();	
	    if(client.available() > MODBUSPACKET){ 
			
            while(client.available()){
				if (client.available() < MODBUSPACKET)
					client.readBytes(ByteArray,client.available());
				else{
					client.readBytes(ByteArray,MODBUSPACKET);
				}
			}
			Serial.println("Overflow");
			Data->Clear();
            RecievedData = -3;
        }
	static int counter = 0;
    if((client.available()+Data->bytesWritten()) >= sizeof(MBAP)){		
		ModbusMode = true;
		FunctionTimer = micros();
		Timer = millis();
		
		while((client.available() > 0) && ((millis() - Timer)<Slice)){
			byte dataSample = client.read();
			Data->write(&dataSample);		
		}
		if(Data->bytesFree() == 0){
			Serial.println("Circular buffer overflow!!! line 189 Modbus.cpp");
			Data->Clear();
		}
		if(counter == 0)
		DEBUG(Serial.println(F("******************New Data**************************"))); 
		if ((micros() - FunctionTimer) > 1000){			
			DEBUG(Serial.print(F("!!!!!Function took : ")));
			DEBUG(Serial.println((micros() - FunctionTimer)));	
			counter++;
		}		    		
                          		
        
        Reads++; 		            
            DEBUG(Serial.println(F("Received RAW")));
			PacketSize = Data->bytesWritten();
            for (int y = 0; y < PacketSize-1; y++){
                DEBUG(Serial.print(Data->at(y),HEX));
                DEBUG(Serial.print("-"));
            }
            DEBUG(Serial.println(Data->at(PacketSize-1),HEX));
            
            //Populate MBAP Header (Siemens Standard)
			ResponseLength = 0;
            MBAP.TransactionID = (((short)Data->at(0))<<8);			//Transaction ID - Random number which client must reply with
            MBAP.TransactionID |= (((short)Data->at(1))&0x00FF);
            MBAP.ProtocolID = (((short)Data->at(2))<<8);				//Protocol ID - Should always be 0
            MBAP.ProtocolID |= (((short)Data->at(3))&0x00FF);			
            MBAP.Length = (((short)Data->at(4))<<8);					//Length of Packet excluding MBAP header, except include Unit identifier # of bytes
            MBAP.Length |= (((short)Data->at(5))&0x00FF);
            DEBUG(Serial.print(F("MBAP.Length : ")));
            DEBUG(Serial.println(MBAP.Length));
            MBAP.UnitID = Data->at(6);
            Response[ResponseLength++] = Data->at(0);				//Echo Transaction ID
            Response[ResponseLength++] = Data->at(1);		
            Response[ResponseLength++] = 0x00;						//Protocol ID always = 0
            Response[ResponseLength++] = 0x00;
            ResponseLength+=2;										//Length is calculated later
            Response[ResponseLength++] = MBAP.UnitID;				//Echo back UnitID
            if((UnitIdentifier == MBAP.UnitID)&&(MBAP.ProtocolID == 0)){
				unsigned short packetLength = (Data->at(4)<<8|(uchar)Data->at(5))+6;
				if(packetLength <= Data->bytesWritten()){			
						Data->read(ByteArray,packetLength);
						DEBUG(Serial.print(F("Iterations it Required to process 1 packet : ")));
						DEBUG(Serial.println(counter));
				counter = 0;
                switch(ByteArray[7]){									//Function Code
                case ReadCoilStatus : {
                    
                    ADU.FunctionCode = ByteArray[7];DEBUG(Serial.println(F("ReadCoilStatus")));
                    ADU.Address = (((short)ByteArray[8])<<8);DEBUG(Serial.print(F("ReadCoilStatus.Address : ")));
                    ADU.Address |= (((short)ByteArray[9])&0x00FF);DEBUG(Serial.println(ADU.Address,HEX)); 
                    ADU.NumberOfPoints = (((short)ByteArray[10])<<8);DEBUG(Serial.print(F("ReadCoilStatus.NumberOfPoints : ")));
                    ADU.NumberOfPoints |= (((short)ByteArray[11])&0x00FF);DEBUG(Serial.println(ADU.NumberOfPoints)); 
                    
                    byte NumberOfBytes = (ADU.NumberOfPoints/8); 	
                    if ((ADU.NumberOfPoints % 8) != 0)
                        NumberOfBytes++;
                    Response[4] = (byte)((3+NumberOfBytes)>>8);DEBUG(Serial.print(F("MBAP Length : ")));
                    Response[5] = (byte)(3+NumberOfBytes);DEBUG(Serial.println((3+NumberOfBytes)));
                    Response[ResponseLength++] = ADU.FunctionCode;DEBUG(Serial.print(F("NumberOfBytes : ")));									
                    Response[ResponseLength++] = NumberOfBytes;DEBUG(Serial.println(NumberOfBytes,HEX)); 								
                    for (int k = 0; k < NumberOfBytes; k++){
                        byte NumberOfCoils;
                        //Moet nog error goed by sit en sorg dat address nie verby die grote van my array gaan nie. ********************************************************************************
                        Response[ResponseLength] = 0;
                        if (ADU.NumberOfPoints >= 8){
                            NumberOfCoils = 8;
                        }
                        else
                            NumberOfCoils  = ADU.NumberOfPoints;
                        for (int y = 0; y < ADU.NumberOfPoints; y++){
                            Response[ResponseLength] |= (((byte)Coil[y+(k*8)+ADU.Address])<<y); 	
                        }
                        ADU.NumberOfPoints-=8;
                        DEBUG(Serial.print(F("ReadCoil Return : ")));
                        DEBUG(Serial.println(Response[ResponseLength],HEX)); 
                        ResponseLength++;
                        
                    }
                    break;
                }
                case ReadInputStatus : {
                    ADU.FunctionCode = ByteArray[7];DEBUG(Serial.println(F("ReadInputStatus")));
                    ADU.Address = (((short)ByteArray[8])<<8);DEBUG(Serial.print(F("ReadInputStatus.Address : ")));
                    ADU.Address |= (((short)ByteArray[9])&0x00FF);DEBUG(Serial.println(ADU.Address,HEX)); 
                    ADU.NumberOfPoints = (((short)ByteArray[10])<<8);DEBUG(Serial.print(F("ReadInputStatus.NumberOfPoints : ")));
                    ADU.NumberOfPoints |= (((short)ByteArray[11])&0x00FF);DEBUG(Serial.println(ADU.NumberOfPoints)); 
                    
                    byte NumberOfBytes = (ADU.NumberOfPoints/8); 	
                    if ((ADU.NumberOfPoints % 8) != 0)
                        NumberOfBytes++;
                    Response[4] = (byte)((3+NumberOfBytes)>>8);DEBUG(Serial.print(F("MBAP Length : ")));
                    Response[5] = (byte)(3+NumberOfBytes);DEBUG(Serial.println((3+NumberOfBytes)));
                    Response[ResponseLength++] = ADU.FunctionCode;DEBUG(Serial.print(F("NumberOfBytes : ")));									
                    Response[ResponseLength++] = NumberOfBytes;DEBUG(Serial.println(NumberOfBytes,HEX)); 	
                    ADU.Address-= InputRegAddrOffSet;
                    for (int k = 0; k < NumberOfBytes; k++){
                        byte NumberOfCoils;
                        //Moet nog error goed by sit en sorg dat address nie verby die grote van my array gaan nie. ********************************************************************************
                        Response[ResponseLength] = 0;
                        if (ADU.NumberOfPoints >= 8){
                            NumberOfCoils = 8;
                        }
                        else
                            NumberOfCoils  = ADU.NumberOfPoints;
                        for (int y = 0; y < ADU.NumberOfPoints; y++){
                            Response[ResponseLength] |= (((byte)Input[y+(k*8)+ADU.Address])<<y); 	
                        }
                        ADU.NumberOfPoints-=8;
                        DEBUG(Serial.print(F("Read Input Return : ")));
                        DEBUG(Serial.println(Response[ResponseLength],HEX)); 
                        ResponseLength++;
                        
                    }
                    break;
                }
                case ReadHoldingRegisters :{ 
                    ADU.FunctionCode = ByteArray[7];DEBUG(Serial.println(F("ReadHoldingRegisters")));
                    ADU.Address = (((short)ByteArray[8])<<8);DEBUG(Serial.print(F("ReadHoldingRegisters.Address : ")));
                    ADU.Address |= (((short)ByteArray[9])&0x00FF);DEBUG(Serial.println(ADU.Address,HEX)); 
                    ADU.NumberOfPoints = (((short)ByteArray[10])<<8);DEBUG(Serial.print(F("ReadHoldingRegisters.NumberOfPoints : ")));
                    ADU.NumberOfPoints |= (((short)ByteArray[11])&0x00FF);DEBUG(Serial.println(ADU.NumberOfPoints)); 
                    
                    byte NumberOfBytes = (ADU.NumberOfPoints)*2; 																		//Even if 0, stil 1 byte
                    Response[4] = (byte)((3+NumberOfBytes)>>8);DEBUG(Serial.print(F("MBAP Length : ")));
                    Response[5] = (byte)(3+NumberOfBytes);DEBUG(Serial.println((3+NumberOfBytes)));
                    Response[ResponseLength++] = ADU.FunctionCode;DEBUG(Serial.print(F("NumberOfBytes : ")));									
                    Response[ResponseLength++] = NumberOfBytes;DEBUG(Serial.println(NumberOfBytes,HEX)); 
                    //ADU.Address-= HoldingRegAddrOffSet;	
                    for (int k = 0; k < (ADU.NumberOfPoints); k++){
                        //Moet nog error goed by sit en sorg dat address nie verby die grote van my array gaan nie. ********************************************************************************
                        Response[ResponseLength++] = HoldingRegister[ADU.Address+k]>>8;DEBUG(Serial.print(F("Holding Register : ")));
                        Response[ResponseLength++] = HoldingRegister[ADU.Address+k];DEBUG(Serial.println(HoldingRegister[ADU.Address+k],HEX));
                    }
                    DEBUG(Serial.print(F("Registers Values : ")));
                    for (int k = 0; k < ADU.NumberOfPoints; k++){
                        DEBUG(Serial.print(HoldingRegister[ADU.Address + k],HEX));				
                        DEBUG(Serial.print(F("-")));
                    }
                    DEBUG(Serial.print(HoldingRegister[0],HEX));		
                    DEBUG(Serial.println());
                    break;
                }
                case ReadInputRegisters : { 
                    ADU.FunctionCode = ByteArray[7];DEBUG(Serial.println(F("ReadInputRegisters")));
                    ADU.Address = (((short)ByteArray[8])<<8);DEBUG(Serial.print(F("ReadInputRegisters.Address : ")));
                    ADU.Address |= (((short)ByteArray[9])&0x00FF);DEBUG(Serial.println(ADU.Address,HEX)); 
                    ADU.NumberOfPoints = (((short)ByteArray[10])<<8);DEBUG(Serial.print(F("ReadInputRegisters.NumberOfPoints : ")));
                    ADU.NumberOfPoints |= (((short)ByteArray[11])&0x00FF);DEBUG(Serial.println(ADU.NumberOfPoints)); 
                    
                    byte NumberOfBytes = (ADU.NumberOfPoints)*2; 																		//Even if 0, stil 1 byte
                    Response[4] = (byte)((3+NumberOfBytes)>>8);DEBUG(Serial.print(F("MBAP Length : ")));
                    Response[5] = (byte)(3+NumberOfBytes);DEBUG(Serial.println((3+NumberOfBytes)));
                    Response[ResponseLength++] = ADU.FunctionCode;DEBUG(Serial.print(F("NumberOfBytes : ")));									
                    Response[ResponseLength++] = NumberOfBytes;DEBUG(Serial.println(NumberOfBytes,HEX)); 
                    ADU.Address-= RegAddrOffSet;									
                    for (int k = 0; k < (ADU.NumberOfPoints); k++){
                        //Moet nog error goed by sit en sorg dat address nie verby die grote van my array gaan nie. ********************************************************************************
                        Response[ResponseLength++] = Register[ADU.Address+k]>>8;DEBUG(Serial.print(F("Register : ")));
                        Response[ResponseLength++] = Register[ADU.Address+k];DEBUG(Serial.println(Register[ADU.Address+k],HEX));
                        
                    }
                    
                    break;
                }
                case WriteSingleCoilStatus : { 
					CoilsWritten = true;
                    ADU.FunctionCode = ByteArray[7];DEBUG(Serial.println(F("WriteSingleCoilStatus")));
                    Response[ResponseLength++] = ByteArray[7];
                    ADU.Address = (((short)ByteArray[8])<<8);DEBUG(Serial.print(F("WriteSingleCoilStatus.Address : ")));
                    ADU.Address |= (((short)ByteArray[9])&0x00FF);DEBUG(Serial.println(ADU.Address,HEX)); 	
                    Response[ResponseLength++] = ByteArray[8];
                    Response[ResponseLength++] = ByteArray[9];
                    Response[4] = (byte)((MBAP.Length)>>8);DEBUG(Serial.print(F("MBAP Length : ")));
                    Response[5] = (byte)(MBAP.Length);DEBUG(Serial.println((MBAP.Length)));
                    Response[ResponseLength++] = ByteArray[10];		
                    Response[ResponseLength++] = ByteArray[11];	
                    if (ByteArray[10] == 0xff)
                        Coil[ADU.Address] = true;
                    else
                        Coil[ADU.Address] = false;
                    break;
                }
                case WriteSingleRegister : { 
					HoldingRegistersWritten = true;
                    ADU.FunctionCode = ByteArray[7];DEBUG(Serial.println(F("WriteSingleRegister")));
                    Response[ResponseLength++] = ByteArray[7];
                    ADU.Address = (((short)ByteArray[8])<<8);DEBUG(Serial.print(F("WriteSingleRegister.Address : ")));
                    ADU.Address |= (((short)ByteArray[9])&0x00FF);DEBUG(Serial.println(ADU.Address,HEX)); 									
                    Response[ResponseLength++] = ByteArray[8];
                    Response[ResponseLength++] = ByteArray[9];
                    Response[4] = (byte)((MBAP.Length)>>8);DEBUG(Serial.print(F("MBAP Length : ")));
                    Response[5] = (byte)(MBAP.Length);DEBUG(Serial.println((MBAP.Length)));
                    Response[ResponseLength++] = ByteArray[10];		
                    Response[ResponseLength++] = ByteArray[11];	
                    ADU.Address-= RegAddrOffSet;
                    HoldingRegister[ADU.Address] =	((ByteArray[10]<<8)&0xff00);
                    HoldingRegister[ADU.Address] |= ByteArray[11];									
                    break;
                }	
                case MultipleCoilWrite :{
                    short readptr = 13;
					CoilsWritten = true;
                    ADU.FunctionCode = ByteArray[7];DEBUG(Serial.println(F("MultipleCoilWrite")));
                    Response[ResponseLength++] = ByteArray[7];
                    ADU.Address = (((short)ByteArray[8])<<8);DEBUG(Serial.print(F("MultipleCoilWrite.Address : ")));
                    ADU.Address |= (((short)ByteArray[9])&0x00FF);DEBUG(Serial.println(ADU.Address,HEX)); 	
                    Response[ResponseLength++] = ByteArray[8];
                    Response[ResponseLength++] = ByteArray[9];									
                    ADU.NumberOfPoints = (((short)ByteArray[10])<<8);DEBUG(Serial.print(F("MultipleCoilWrite.NumberOfCoils : ")));
                    ADU.NumberOfPoints |= (((short)ByteArray[11])&0x00FF);DEBUG(Serial.println(ADU.NumberOfPoints)); 		
                    byte NumberOfbytes = ByteArray[12];DEBUG(Serial.print(F("MultipleCoilWrite.NumberOfbytes : ")));DEBUG(Serial.println(NumberOfbytes)); 	
                    
                    Response[4] = (byte)((MBAP.Length)>>8);DEBUG(Serial.print(F("MBAP Length : ")));
                    Response[5] = (byte)(MBAP.Length);DEBUG(Serial.println((MBAP.Length)));
                    Response[ResponseLength++] = ByteArray[10];		
                    Response[ResponseLength++] = ByteArray[11];	
					byte NumberOfCoils;
                    for (int k = 0; k < NumberOfbytes; k++){                        
                        //Moet nog error goed by sit en sorg dat address nie verby die grote van my array gaan nie. ********************************************************************************
                        //										Response[ResponseLength] = 0;
                        if (ADU.NumberOfPoints >= 8){
                            NumberOfCoils = 8;
                        }
                        else
                            NumberOfCoils = ADU.NumberOfPoints;
                        DEBUG(Serial.print("Byte :"));
						DEBUG(Serial.println(ByteArray[readptr],HEX));
                        for (int y = 0; y < NumberOfCoils; y++){							
                            if (ByteArray[readptr] & (1<<y)){
                                Coil[y+(k*8)+ADU.Address] = true;	
								DEBUG(Serial.print("Coil "));
                                DEBUG(Serial.print(y+(k*8)+ADU.Address));
								DEBUG(Serial.print(" :"));
								DEBUG(Serial.println(" true"));
                            }
                            else{ 
                                Coil[y+(k*8)+ADU.Address]=false;	
								DEBUG(Serial.print("Coil "));
                                DEBUG(Serial.print(y+(k*8)+ADU.Address));
								DEBUG(Serial.print(" :"));
								DEBUG(Serial.println(" false"));
							}
                        }
                        readptr++;
                        ADU.NumberOfPoints-=8;							
                    }
                    break;
                }
                case MultipleRegisterWrite : {
                    short readptr = 13;
					HoldingRegistersWritten = true;
                    ADU.FunctionCode = ByteArray[7];DEBUG(Serial.println(F("MultipleRegisterWrite")));
                    Response[ResponseLength++] = ByteArray[7];
                    ADU.Address = (((short)ByteArray[8])<<8);DEBUG(Serial.print(F("MultipleRegisterWrite.Address : ")));
                    ADU.Address |= (((short)ByteArray[9])&0x00FF);DEBUG(Serial.println(ADU.Address,HEX)); 	
                    Response[ResponseLength++] = ByteArray[8];
                    Response[ResponseLength++] = ByteArray[9];									
                    ADU.NumberOfPoints = (((short)ByteArray[10])<<8);DEBUG(Serial.print(F("MultipleRegisterWrite.NumberOfPoints: ")));
                    ADU.NumberOfPoints |= (((short)ByteArray[11])&0x00FF);DEBUG(Serial.println(ADU.NumberOfPoints)); 		
                    byte NumberOfBytes = ByteArray[12];DEBUG(Serial.print(F("MultipleRegisterWrite.NumberOfBytes : ")));DEBUG(Serial.println(NumberOfBytes)); 	
                    
                    Response[4] = (byte)((MBAP.Length)>>8);DEBUG(Serial.print(F("MBAP Length : ")));
                    Response[5] = (byte)(MBAP.Length);DEBUG(Serial.println((MBAP.Length)));
                    Response[ResponseLength++] = ByteArray[10];		
                    Response[ResponseLength++] = ByteArray[11];	
                    DEBUG(Serial.print(F("Registers Values : ")));
                    for (int k = 0; k < ADU.NumberOfPoints; k++){
                        //Moet nog error goed by sit en sorg dat address nie verby die grote van my array gaan nie. ********************************************************************************
                        unsigned short value = ByteArray[readptr++]<<8;
                        value |= ByteArray[readptr++];
                        this->setHoldingRegister(value,ADU.Address + k);	
                        DEBUG(Serial.print(this->getHoldingRegister(ADU.Address + k),HEX));				
                        DEBUG(Serial.print(F("-")));
                    }
                    break;
                }
                default  : {break;}
                }                

                client.write(Response,ResponseLength);
				client.flush();
                RecievedData = ByteArray[7]; 
				DEBUG(Serial.println());	
				DEBUG(Serial.print(F("MBAP.TransactionID :")));
				DEBUG(Serial.println(MBAP.TransactionID));
				DEBUG(Serial.print(F("MBAP.ProtocolID :")));
				DEBUG(Serial.println(MBAP.ProtocolID));
				DEBUG(Serial.print(F("MBAP.Length :")));
				DEBUG(Serial.println(MBAP.Length));				
				DEBUG(Serial.print(F("MBAP.UnitID :")));
				DEBUG(Serial.println(MBAP.UnitID));
				DEBUG(Serial.print(F("ADU.FunctionCode :")));
				DEBUG(Serial.println(ADU.FunctionCode));
				DEBUG(Serial.print(F("ADU.Address :")));
				DEBUG(Serial.println(ADU.Address));
				DEBUG(Serial.print(F("ADU.NumberOfPoints :")));
				DEBUG(Serial.println(ADU.NumberOfPoints));				
                DEBUG(Serial.println(F("Transmit RAW")));			
                for (int k = 0; k < ResponseLength-1; k++){            
                    DEBUG(Serial.print(Response[k],HEX));
                    DEBUG(Serial.print("-"));
                }
                DEBUG(Serial.println(Response[ResponseLength-1],HEX));
				memset(Response,0,modbusSize);
				memset(ByteArray,0,modbusSize);
				
	
            }
			else {
				DEBUG(Serial.print(F("Packet not ready Requires : ")));
				DEBUG(Serial.println((MBAP.Length+6)-Data->bytesWritten()));
			}
				
			}
			else{
                if(UnitIdentifier != MBAP.UnitID){
                    (Serial.println(F("Packet Failed because UnitIdentifier != MBAP.UnitID")));
                    (Serial.print(F("UnitIdentifier = ")));
                    (Serial.println(UnitIdentifier));
                    (Serial.print(F("MBAP.UnitID = ")));
                    (Serial.println(MBAP.UnitID));					
                }
                if(MBAP.ProtocolID != 0){
					(Serial.println(F("Packet Failed because MBAP.ProtocolID != 0")));
                    (Serial.print(F("MBAP.ProtocolID = ")));
                    (Serial.println(MBAP.ProtocolID));
                }
				byte shiftOne;
				(Serial.println(F("shiftOne")));
				Data->read(&shiftOne);
                RecievedData = -3;		                					
            }			
        				
    }
	
			// Serial.print(F("Proccessing Time : "));
			// Serial.println((micros()-Timer)/1000.0);	
    return RecievedData;
}
void Modbus::swapHighLowByteOfCharArray(char* value,int size){
    char tmp;
    for(int i = 0; i < size; i+=2){
        tmp = value[i];
        value[i] = value[i+1];
        value[i+1] = tmp;
    }
}
void Modbus::setProccessTime(unsigned char _Slice){
	Slice = _Slice;
}