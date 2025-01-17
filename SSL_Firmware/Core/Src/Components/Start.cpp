/*
 * Start.cpp
 *
 *  Created on: 20 de mar de 2021
 *      Author: Moraes
 *  Edited on: 26/11/2022
 *  	Author: Isabel Chaves
 */

#include "Start.hpp"
#include "usbd_cdc_if.h"

//Component includes
#include "Encoder.hpp"
#include "Motor.hpp"
#include "Robo.hpp"
#include "CommunicationUSB.hpp"

#include "RoboIME_RF24.hpp"
#include "SerialDebug.hpp"
#include "CommunicationNRF.hpp"
#include "Defines.hpp"

//Protobuf includes
#include "grSim_Commands.pb.h"
#include "Feedback.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

//Constant definitions
#define NUM_ROBOTS 8

//Global variables
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim14;
extern UART_HandleTypeDef huart3;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

extern void (*usbRecvCallback)(uint8_t*, uint32_t*);
extern USBD_HandleTypeDef hUsbDeviceFS;
grSim_Robot_Command receivedPacket = grSim_Robot_Command_init_default;
Feedback sendPacket[NUM_ROBOTS];
bool transmitter;
nRF_Send_Packet_t nRF_Send_Packet[16];
nRF_Feedback_Packet_t nRF_Feedback_Packet;
nRF_Feedback_Packet_t nRF_FeedbackReceive_Packet[16];
uint8_t commCounter = 0;
uint32_t usbCounter = 0;

//Temporary (only for debug)
char serialBuf[64];

//Objects
Robo robo(1);
CommunicationUSB usb(&usbRecvCallback);
RoboIME_RF24 radio(nRF_CSn_GPIO_Port, nRF_CSn_Pin, nRF_CE_GPIO_Port, nRF_CE_Pin, nRF_IRQ_GPIO_Port, nRF_IRQ_Pin, &hspi1);
SerialDebug debug(&huart3);

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim6){
		if(!transmitter && radio.ready){
			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
			nRF_Feedback_Packet.packetId++;
			radio.UploadAckPayload((uint8_t*)&nRF_Feedback_Packet, sizeof(nRF_Feedback_Packet));
//			if(radio.getReceivedPayload((uint8_t*)nRF_Send_Packet)){
			if(true){
				HAL_GPIO_TogglePin(LD4_GPIO_Port, LD4_Pin);
				HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);
				commCounter = 0;
			}else{
				commCounter++;
			}
			if(commCounter < 100){	//Verifica se recebeu pacote no último 1s
				if(nRF_Send_Packet[0].wheelsspeed){
					robo.set_wheel_speed(nRF_Send_Packet[0].wheel1, nRF_Send_Packet[0].wheel2, nRF_Send_Packet[0].wheel3, nRF_Send_Packet[0].wheel4);

				}
				else{
					robo.set_robo_speed(nRF_Send_Packet[0].velnormal, nRF_Send_Packet[0].veltangent, nRF_Send_Packet[0].velangular);
				}
			}else{
				//Perdeu a comunicação
				robo.set_robo_speed(0, 0, 0);
				HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
				commCounter = 100;
			}
		}
	}
}

void USBpacketReceivedCallback(void){
	if(receivedPacket.id > 16){
		//debug.error("USB protobuf ID > 16");
	}else{
		/*char debugmessage[64];
		sprintf(debugmessage, "ID %lu", receivedPacket.id);
		debug.debug(debugmessage);*/
		usbCounter = 0;
		nRF_Send_Packet[receivedPacket.id].veltangent = receivedPacket.veltangent;
		nRF_Send_Packet[receivedPacket.id].velnormal = receivedPacket.velnormal;
		nRF_Send_Packet[receivedPacket.id].velangular = receivedPacket.velangular;
		nRF_Send_Packet[receivedPacket.id].wheelsspeed = receivedPacket.wheelsspeed;
		nRF_Send_Packet[receivedPacket.id].wheel1 = receivedPacket.wheel1;
		nRF_Send_Packet[receivedPacket.id].wheel2 = receivedPacket.wheel2;
		nRF_Send_Packet[receivedPacket.id].wheel3 = receivedPacket.wheel3;
		nRF_Send_Packet[receivedPacket.id].wheel4 = receivedPacket.wheel4;

	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == nRF_IRQ_Pin && HAL_GPIO_ReadPin(nRF_IRQ_GPIO_Port, nRF_IRQ_Pin) == GPIO_PIN_RESET){
		radio.extiCallback();
	}
}


void Flash_Write(uint8_t data, uint32_t adress, uint32_t sector_num){
	//Unlock the flash
	HAL_FLASH_Unlock();
	//Erase sector
	FLASH_Erase_Sector(sector_num, FLASH_VOLTAGE_RANGE_3);
	//Lock the flash
	HAL_FLASH_Lock();
	FLASH_WaitForLastOperation(1000);
	//Unlock the flash
	HAL_FLASH_Unlock();
	//Write to Flash
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, adress , data);
	//Lock the Flash space
	HAL_FLASH_Lock();

}

void Start(){
	Robo robo(1);
	uint8_t id = *(uint8_t *)0x080E0000;
		if(id>15)
			id = 0;
	debug.setLevel(SerialDebug::DEBUG_LEVEL_DEBUG);
	debug.info("SSL firmware start");
	radio.ce(GPIO_PIN_SET);
	for (uint32_t i=0; i< NUM_ROBOTS; i++){
		sendPacket[i] = Feedback_init_default;
	}
	nRF_Send_Packet[0].velangular = 0;
	nRF_Send_Packet[0].veltangent = 0;
	nRF_Send_Packet[0].velnormal = 0;
	for(uint32_t i=0; i<2000; i++){
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, (GPIO_PinState)(id & 1));
		HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, (GPIO_PinState)((id>>1) & 1));
		HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, (GPIO_PinState)((id>>2) & 1));
		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, (GPIO_PinState)((id>>3) & 1));
		if(!HAL_GPIO_ReadPin(Btn_GPIO_Port, Btn_Pin)){
			id = (id+1) % 16;
			i=0;
			HAL_Delay(2);
			while(!HAL_GPIO_ReadPin(Btn_GPIO_Port, Btn_Pin));
		}
		HAL_Delay(1);
	}
	Flash_Write(id, 0x080E0000, 11);
	nRF_Feedback_Packet.status = id<<28;
#ifdef DEEPWEB
	nRF_Feedback_Packet.status |= 1<<2;		//Set bit 2
#else
	nRF_Feedback_Packet.status &= ~(1<<2);	//Reset bit 2
#endif
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
	radio.setup();
	if(HAL_GPIO_ReadPin(TX_Detect_GPIO_Port, TX_Detect_Pin) == GPIO_PIN_RESET){
		//TX (placa de COM)
		transmitter = true;
		debug.info("PD10 set as transmitter (computer)");
		radio.setDirection(PWRUP_TX);
	}else{
		//RX (Robô)
		transmitter = false;
		debug.info("PD10 set as receiver (robot)");
		radio.setDirection(PWRUP_RX);
	}
	if(!transmitter){
		radio.setRobotId(id);
	}
	debug.info("ID = 6");
	radio.ready = true;
	while(1){
		if(transmitter){
			for(uint8_t i=0; i<NUM_ROBOTS; i++){
				radio.setRobotId(i);
				nRF_Send_Packet[i].packetId++;
				usbCounter++;
#ifdef INTEL
				if(usbCounter > 500){	//Verifica se recebeu pacote do USb nos últimos Xs
					//Perdeu o USB
					for (uint8_t i=0; i<16; i++){
						nRF_Send_Packet[i].velangular = 0;
						nRF_Send_Packet[i].veltangent = 0;
						nRF_Send_Packet[i].velnormal = 0;
					}
					HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
					usbCounter = 500;
				}else{
					HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_RESET);
				}
#endif
				radio.sendPayload((uint8_t*)&nRF_Send_Packet[i], sizeof(nRF_Send_Packet[i]));	//Conversão do tipo do ponteiro
				HAL_Delay(1);
				if(radio.getReceivedPayload((uint8_t*)&nRF_FeedbackReceive_Packet[i])){
					HAL_GPIO_TogglePin(LD4_GPIO_Port, LD4_Pin);
					//debug.debug((char*)received);
					USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
						sendPacket[i].encoder1 = nRF_FeedbackReceive_Packet[i].encoder1;
						sendPacket[i].encoder2 = nRF_FeedbackReceive_Packet[i].encoder2;
						sendPacket[i].encoder3 = nRF_FeedbackReceive_Packet[i].encoder3;
						sendPacket[i].encoder4 = nRF_FeedbackReceive_Packet[i].encoder4;
						sendPacket[i].status = nRF_FeedbackReceive_Packet[i].status;
						sendPacket[i].id = nRF_FeedbackReceive_Packet[i].status>>28;
						usb.TransmitFeedbackPacket(i, id);
				}
			}
		}else{
			sendPacket[0].encoder1 = nRF_Feedback_Packet.encoder1;
			sendPacket[0].encoder2 = nRF_Feedback_Packet.encoder2;
			sendPacket[0].encoder3 = nRF_Feedback_Packet.encoder3;
			sendPacket[0].encoder4 = nRF_Feedback_Packet.encoder4;
			sendPacket[0].status = nRF_Feedback_Packet.status;
			sendPacket[0].id = 0;
			usb.TransmitFeedbackPacket(0, 0);
			HAL_Delay(10);
		}
	}
}
